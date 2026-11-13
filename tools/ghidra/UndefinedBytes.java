// Disassemble the byte runs that OrphanCode.java reported as "undefined"
// (neither instructions nor data), so they can be told apart: real code the
// analysis never reached, or tables and padding.
//
// Headless (use -readOnly: the disassembly is not saved to the project):
//   -postScript UndefinedBytes.java <out-dir> [min-size]
// Reads <out-dir>/orphan_code.json; writes <out-dir>/orphans_undef/0xADDR.asm
// for every undefined run of at least min-size bytes (default 6): a header
// line with the raw bytes, then `ADDRESS  instruction` lines for whatever
// disassembles from the start of the run.
//@category iwd2

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;

public class UndefinedBytes extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Path out = Path.of(args.length > 0 ? args[0] : ".");
        long minSize = args.length > 1 ? Long.parseLong(args[1]) : 6;
        Path dir = out.resolve("orphans_undef");
        Files.createDirectories(dir);

        Pattern row = Pattern.compile("\"addr\":\"0x([0-9A-Fa-f]+)\",\"size\":(\\d+),\"kind\":\"undefined\"");
        Listing listing = currentProgram.getListing();
        int written = 0;

        for (String line : Files.readAllLines(out.resolve("orphan_code.json"))) {
            Matcher m = row.matcher(line);
            if (!m.find()) {
                continue;
            }
            long start = Long.parseLong(m.group(1), 16);
            long size = Long.parseLong(m.group(2));
            if (size < minSize) {
                continue;
            }
            monitor.checkCancelled();

            Address a = toAddr(start);
            Address end = a.add(size);
            byte[] raw = getBytes(a, (int) Math.min(size, 4096));
            StringBuilder hex = new StringBuilder();
            for (byte b : raw) {
                hex.append(String.format("%02x", b & 0xff));
            }

            List<String> lines = new ArrayList<>();
            lines.add(String.format("; undefined bytes @ 0x%X size %d", start, size));
            lines.add("; bytes " + hex);

            disassemble(a);
            Address p = a;
            long decoded = 0;
            while (p.compareTo(end) < 0) {
                Instruction ins = listing.getInstructionAt(p);
                if (ins == null) {
                    // Try to pick the flow up again after a gap.
                    lines.add(String.format("%08X  ??", p.getOffset()));
                    p = p.add(1);
                    if (p.compareTo(end) < 0 && listing.getInstructionAt(p) == null
                        && listing.getDefinedDataAt(p) == null) {
                        disassemble(p);
                    }
                    continue;
                }
                lines.add(String.format("%08X  %s", p.getOffset(), ins.toString()));
                decoded += ins.getLength();
                p = p.add(ins.getLength());
            }
            lines.set(0, lines.get(0) + String.format(" decoded %d", decoded));
            Files.write(dir.resolve(String.format("0x%X.asm", start)), lines);
            written++;
        }
        println("UndefinedBytes: " + written + " listings -> " + dir);
    }
}
