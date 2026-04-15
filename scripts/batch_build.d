#!/usr/bin/env dub

/+ dub.sdl:
       name "batch_build"
       description "Zephyr Batch Building"
       dependency "colored" version="~>0.0.32"
+/ 

import std.stdio : writeln, writefln;
import std.file : dirEntries, SpanMode, isDir, exists, symlink, remove;
import std.path : buildPath, baseName, absolutePath;
import std.process : spawnProcess, wait;
import std.algorithm : filter;
import colored;

void main() {
    string boardName = "nucleo_l433rc_p"; 
    string targetDir = "nucleo-l433rc-p";

    writeln("🚀 Starting Zephyr Batch Builder...".blue.bold);
    writeln("Scanning directory: ".cyan, targetDir.white.bold, " for target: ".cyan, boardName.white.bold, "\n");

    if (!exists(targetDir) || !isDir(targetDir)) {
        writeln("Error: Target directory does not exist.".red.bold);
        return;
    }

    auto entries = dirEntries(targetDir, SpanMode.shallow).filter!(e => e.isDir);
    
    int successCount = 0;
    int failCount = 0;

    foreach (entry; entries) {
        string appPath = entry.name;
        string appName = baseName(appPath);
        string cmakeFile = buildPath(appPath, "CMakeLists.txt");

        if (exists(cmakeFile)) {
            writeln("--------------------------------------------------");
            writeln("🔨 Building: ".magenta, appName.white.bold);
            writeln("--------------------------------------------------");

            string buildDir = buildPath("build", appName);

            // Pass the CMake flag to ensure compile_commands.json is always generated
            string[] cmd = [
                "west", "build", "-b", boardName, "-d", buildDir, appPath,
                "--", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
            ];

            auto pid = spawnProcess(cmd);
            auto exitCode = wait(pid);

            if (exitCode == 0) {
                writeln("✅ Success: ".green.bold, appName.green);
                successCount++;

                // --- NEOVIM LSP SYMLINKING STEP ---
                string compileCmdsSource = buildPath(buildDir, "compile_commands.json").absolutePath;
                string compileCmdsTarget = buildPath(appPath, "compile_commands.json").absolutePath;

                if (exists(compileCmdsSource)) {
                    // Remove existing symlink/file to prevent FileExists errors on rebuilds
                    if (exists(compileCmdsTarget)) {
                        remove(compileCmdsTarget);
                    }
                    
                    // Create the symlink
                    symlink(compileCmdsSource, compileCmdsTarget);
                    writeln("🔗 Symlinked compile_commands.json for Neovim (clangd)".cyan);
                } else {
                    writeln("⚠️ Warning: compile_commands.json not found in build directory.".yellow);
                }
                writeln(); // Blank line for spacing
                
            } else {
                writeln("❌ Failed: ".red.bold, appName.red, " (Exit Code: ".red, exitCode, ")\n".red);
                failCount++;
            }
        }
    }

    writeln("==================================================".blue.bold);
    writeln("🏁 Batch Build Complete!".white.bold);
    writeln("Successful: ".green, successCount, " | Failed: ".red, failCount);
    writeln("==================================================".blue.bold);
}
