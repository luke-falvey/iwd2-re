// Define functions Ghidra's analysis missed (typically reached only through a
// vtable) and export them like ExportFunctions does: <out>/decomp/0xADDR.asm
// and .c, plus <out>/functions.extra.json with their entries (merge these into
// functions.json by address).
// Headless: -postScript DefineFunctions.java <out-dir> 0xADDR [0xADDR ...]
// Output is derived from the copyrighted binary: keep it outside the repo.
// @category iwd2-re

import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

public class DefineFunctions extends GhidraScript {
    private static String hex(Address a) {
        return String.format("0x%X", a.getOffset());
    }

    private static String json(String s) {
        return "\"" + s.replace("\\", "\\\\").replace("\"", "\\\"") + "\"";
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Path out = Path.of(args[0]);
        Path decompDir = out.resolve("decomp");
        Files.createDirectories(decompDir);

        DecompInterface ifc = new DecompInterface();
        ifc.setOptions(new DecompileOptions());
        ifc.toggleCCode(true);
        ifc.toggleSyntaxTree(false);
        ifc.openProgram(currentProgram);

        List<String> entries = new ArrayList<>();
        for (int i = 1; i < args.length; i++) {
            Address entry = toAddr(Long.decode(args[i]));
            Function f = getFunctionAt(entry);
            if (f == null) {
                if (getInstructionAt(entry) == null) {
                    new DisassembleCommand(entry, null, true).applyTo(currentProgram, monitor);
                }
                new CreateFunctionCmd(entry).applyTo(currentProgram, monitor);
                f = getFunctionAt(entry);
            }
            if (f == null) {
                println("DefineFunctions: FAILED at " + hex(entry));
                continue;
            }
            String addr = hex(entry);

            Set<String> callers = new TreeSet<>();
            for (Function c : f.getCallingFunctions(monitor)) callers.add(hex(c.getEntryPoint()));
            Set<String> callees = new TreeSet<>();
            for (Function c : f.getCalledFunctions(monitor)) callees.add(hex(c.getEntryPoint()));

            long size = f.getBody().getNumAddresses();
            entries.add("{\"addr\":" + json(addr)
                + ",\"name\":" + json(f.getName(true))
                + ",\"size\":" + size
                + ",\"is_thunk\":" + f.isThunk()
                + ",\"signature\":" + json(f.getPrototypeString(false, false))
                + ",\"callers\":[" + String.join(",", callers.stream().map(DefineFunctions::json).toList()) + "]"
                + ",\"callees\":[" + String.join(",", callees.stream().map(DefineFunctions::json).toList()) + "]}");

            try (PrintWriter w = new PrintWriter(Files.newBufferedWriter(decompDir.resolve(addr + ".asm"), StandardCharsets.UTF_8))) {
                w.println("; " + f.getName(true) + " @ " + addr + " size " + size);
                InstructionIterator ii = currentProgram.getListing().getInstructions(f.getBody(), true);
                while (ii.hasNext()) {
                    Instruction ins = ii.next();
                    w.println(String.format("%08X  %s", ins.getAddress().getOffset(), ins.toString()));
                }
            }

            DecompileResults r = ifc.decompileFunction(f, 120, monitor);
            String c = r.decompileCompleted() ? r.getDecompiledFunction().getC() : ("// decompile failed: " + r.getErrorMessage());
            Files.writeString(decompDir.resolve(addr + ".c"), "// " + f.getName(true) + " @ " + addr + "\n" + c, StandardCharsets.UTF_8);
            println("DefineFunctions: " + addr + " size " + size);
        }
        Files.writeString(out.resolve("functions.extra.json"), "[\n" + String.join(",\n", entries) + "\n]\n", StandardCharsets.UTF_8);
    }
}
