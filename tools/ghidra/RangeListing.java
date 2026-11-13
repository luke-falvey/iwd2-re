// Write a gap-free listing of an address range: every instruction, whether
// or not it belongs to a function, defined data as `db`/`dd` lines, and
// bytes the analysis left undefined disassembled where possible. Use it for
// a function whose per-function listing has holes (code reached only
// through a jump table the analysis did not resolve).
//
// Headless (use -readOnly: the extra disassembly is not saved):
//   -postScript RangeListing.java <out-dir> <start> <end> [<start> <end> ...]
// Writes <out-dir>/ranges/0xSTART-0xEND.asm.
//@category iwd2

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;

public class RangeListing extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Path dir = Path.of(args[0]).resolve("ranges");
        Files.createDirectories(dir);
        Listing listing = currentProgram.getListing();

        for (int i = 1; i + 1 < args.length; i += 2) {
            long start = Long.decode(args[i]);
            long endOff = Long.decode(args[i + 1]);
            Address p = toAddr(start);
            Address end = toAddr(endOff);
            List<String> lines = new ArrayList<>();
            lines.add(String.format("; range 0x%X-0x%X (gap-free; '*' marks code that is in no function,", start, endOff));
            lines.add(";  '?' code disassembled only for this listing)");

            while (p.compareTo(end) < 0) {
                monitor.checkCancelled();
                Instruction ins = listing.getInstructionAt(p);
                boolean forced = false;
                if (ins == null && listing.getDefinedDataAt(p) == null
                    && listing.getCodeUnitContaining(p) != null
                    && listing.getCodeUnitContaining(p).getMinAddress().equals(p)) {
                    byte b = getByte(p);
                    if (b != (byte) 0xCC) {
                        disassemble(p);
                        ins = listing.getInstructionAt(p);
                        forced = ins != null;
                    }
                }
                if (ins != null) {
                    Function f = getFunctionContaining(p);
                    String mark = forced ? "?" : (f == null ? "*" : " ");
                    lines.add(String.format("%08X %s%s", p.getOffset(), mark, ins.toString()));
                    p = p.add(ins.getLength());
                    continue;
                }
                Data d = listing.getDefinedDataAt(p);
                if (d != null && d.getLength() == 4) {
                    lines.add(String.format("%08X  dd 0x%08X", p.getOffset(), getInt(p)));
                    p = p.add(4);
                } else if (d != null && d.getLength() > 0) {
                    StringBuilder sb = new StringBuilder();
                    int n = Math.min(d.getLength(), 64);
                    for (byte b : getBytes(p, n)) {
                        sb.append(String.format("%02x ", b & 0xff));
                    }
                    lines.add(String.format("%08X  db %s%s", p.getOffset(), sb.toString().trim(),
                        d.getLength() > n ? " ..." : ""));
                    p = p.add(d.getLength());
                } else {
                    lines.add(String.format("%08X  db %02x", p.getOffset(), getByte(p) & 0xff));
                    p = p.add(1);
                }
            }
            Files.write(dir.resolve(String.format("0x%X-0x%X.asm", start, endOff)), lines);
            println("RangeListing: " + lines.size() + " lines for " + String.format("0x%X-0x%X", start, endOff));
        }
    }
}
