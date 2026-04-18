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
    // Map the folder name to the required Zephyr board target
    string[string] targetBoards = [
        "nucleo-l433rc-p": "nucleo_l433rc_p",
        "nrf52840dk": "nrf52840dk/nrf52840"
    ];

    writeln("🚀 Starting Zephyr Multi-Board Batch Builder...".blue.bold);

    int successCount = 0;
    int failCount = 0;

    foreach (targetDir, boardName; targetBoards) {
        writeln("\n📁 Scanning directory: ".cyan, targetDir.white.bold, " for target: ".cyan, boardName.white.bold);

        if (!exists(targetDir) || !isDir(targetDir)) {
            writeln("⚠️  Directory not found, skipping: ".yellow, targetDir);
            continue;
        }

        auto entries = dirEntries(targetDir, SpanMode.shallow).filter!(e => e.isDir);

        foreach (entry; entries) {
            string appPath = entry.name;
            string appName = baseName(appPath);
            string cmakeFile = buildPath(appPath, "CMakeLists.txt");

            if (exists(cmakeFile)) {
                writeln("--------------------------------------------------");
                writeln("🔨 Building: ".magenta, appName.white.bold);
                writeln("--------------------------------------------------");

                // Segregate builds to prevent collisions (e.g. build/nrf52840dk/print_console)
                string buildDir = buildPath("build", targetDir, appName);

                string[] cmd = [
                    "west", "build", "-b", boardName, "-d", buildDir, appPath,
                    "--", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
                ];

                auto pid = spawnProcess(cmd);
                auto exitCode = wait(pid);

                if (exitCode == 0) {
                    writeln("✅ Success: ".green.bold, buildPath(targetDir, appName).green);
                    successCount++;

                    string compileCmdsSource = buildPath(buildDir, "compile_commands.json").absolutePath;
                    string compileCmdsTarget = buildPath(appPath, "compile_commands.json").absolutePath;

                    if (exists(compileCmdsSource)) {
                        if (exists(compileCmdsTarget)) {
                            remove(compileCmdsTarget);
                        }
                        
                        symlink(compileCmdsSource, compileCmdsTarget);
                        writeln("🔗 Symlinked compile_commands.json for Neovim (clangd)".cyan);
                    } else {
                        writeln("⚠️ Warning: compile_commands.json not found in build directory.".yellow);
                    }
                    writeln(); 
                    
                } else {
                    writeln("❌ Failed: ".red.bold, buildPath(targetDir, appName).red, " (Exit Code: ".red, exitCode, ")\n".red);
                    failCount++;
                }
            }
        }
    }

    writeln("==================================================".blue.bold);
    writeln("🏁 Batch Build Complete!".white.bold);
    writeln("Successful: ".green, successCount, " | Failed: ".red, failCount);
    writeln("==================================================".blue.bold);
}
