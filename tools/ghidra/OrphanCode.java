// List code in the game range that belongs to no function, and bytes that
// are neither instructions nor defined data. The function export only sees
// what Ghidra made into a function, so anything listed here is invisible to
// the inventory and the worklist.
//
// Headless: -postScript OrphanCode.java <out-dir>
// Writes <out-dir>/orphan_code.json:
//   [{"addr":"0x...","size":N,"kind":"code"|"undefined","first":"MNEMONIC ..."}]
// and, for every code run, a listing <out-dir>/orphans/0xADDR.asm in the same
// format as the per-function listings.
//@category iwd2

import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;

public class OrphanCode extends GhidraScript {
    private static final long GAME_START = 0x401000L;
    private static final long GAME_END = 0x7DA200L;

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Path out = Path.of(args.length > 0 ? args[0] : ".");
        Path listings = out.resolve("orphans");
        Files.createDirectories(listings);
        List<String> runLines = new ArrayList<>();
        Listing listing = currentProgram.getListing();
        Address a = toAddr(GAME_START);
        Address end = toAddr(GAME_END);

        try (PrintWriter w = new PrintWriter(out.resolve("orphan_code.json").toFile(), "UTF-8")) {
            w.println("[");
            boolean firstRow = true;
            long runStart = -1;
            long runSize = 0;
            String runKind = null;
            String runFirst = null;
            int runs = 0;

            while (a.compareTo(end) < 0) {
                monitor.checkCancelled();
                String kind = null;
                String text = "";
                long len = 1;
                Instruction ins = listing.getInstructionAt(a);
                if (ins != null) {
                    len = ins.getLength();
                    if (getFunctionContaining(a) == null) {
                        String m = ins.getMnemonicString();
                        // Alignment between functions is not code.
                        boolean pad = m.equals("INT3") || m.equals("NOP");
                        if (!pad) {
                            kind = "code";
                            text = ins.toString();
                        }
                    }
                } else {
                    Data d = listing.getDefinedDataAt(a);
                    if (d != null) {
                        len = Math.max(1, d.getLength());
                    } else {
                        CodeUnit cu = listing.getCodeUnitContaining(a);
                        if (cu != null && !cu.getMinAddress().equals(a)) {
                            // Inside a defined unit that started earlier.
                            len = cu.getMaxAddress().subtract(a) + 1;
                        } else {
                            byte b = getByte(a);
                            if (b != (byte) 0xCC && b != (byte) 0x90 && b != 0) {
                                kind = "undefined";
                                text = String.format("%02x", b & 0xff);
                            }
                        }
                    }
                }

                long off = a.getOffset();
                if (kind != null && kind.equals(runKind) && off == runStart + runSize) {
                    runSize += len;
                    if (kind.equals("code")) {
                        runLines.add(String.format("%08X  %s", off, text));
                    }
                } else {
                    if (runKind != null) {
                        w.println((firstRow ? "" : ",") + row(runStart, runSize, runKind, runFirst));
                        firstRow = false;
                        runs++;
                        flush(listings, runStart, runSize, runKind, runLines);
                    }
                    runLines.clear();
                    if (kind != null) {
                        runStart = off;
                        runSize = len;
                        runKind = kind;
                        runFirst = text;
                        if (kind.equals("code")) {
                            runLines.add(String.format("%08X  %s", off, text));
                        }
                    } else {
                        runKind = null;
                    }
                }
                a = a.add(len);
            }
            if (runKind != null) {
                w.println((firstRow ? "" : ",") + row(runStart, runSize, runKind, runFirst));
                runs++;
                flush(listings, runStart, runSize, runKind, runLines);
            }
            w.println("]");
            println("OrphanCode: " + runs + " runs -> " + out.resolve("orphan_code.json"));
        }
    }

    private static void flush(Path dir, long start, long size, String kind, List<String> lines) throws Exception {
        if (!kind.equals("code") || lines.isEmpty()) {
            return;
        }
        List<String> out = new ArrayList<>();
        out.add(String.format("; code in no function @ 0x%X size %d", start, size));
        out.addAll(lines);
        Files.write(dir.resolve(String.format("0x%X.asm", start)), out);
    }

    private static String row(long start, long size, String kind, String first) {
        return String.format("{\"addr\":\"0x%X\",\"size\":%d,\"kind\":\"%s\",\"first\":\"%s\"}",
            start, size, kind, first.replace("\\", "\\\\").replace("\"", "'"));
    }
}
