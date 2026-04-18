#!/usr/bin/env dub

/+ dub.sdl:
       name "batch_clean"
       description "Zephyr Batch Cleaner"
       dependency "colored" version="~>0.0.32"
+/ 

import std.stdio : writeln, writefln;
import std.file : dirEntries, SpanMode, isDir, exists, remove, rmdirRecurse;
import std.path : buildPath, baseName;
import std.algorithm : filter;
import colored;

void main() {
    string[] targetDirs = ["nucleo-l433rc-p", "nrf52840dk"];
    string rootBuildDir = "build";

    writeln("🧹 Starting Zephyr Batch Cleaner...".blue.bold);

    int cleanedCount = 0;

    foreach (targetDir; targetDirs) {
        writeln("\nScanning for artifacts in: ".cyan, targetDir.white.bold);
        
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
                bool cleanedSomething = false;
                string compileCmdsTarget = buildPath(appPath, "compile_commands.json");
                string localBuildDir = buildPath(appPath, "build"); 

                if (exists(compileCmdsTarget)) {
                    remove(compileCmdsTarget);
                    cleanedSomething = true;
                }
                
                if (exists(localBuildDir) && isDir(localBuildDir)) {
                    rmdirRecurse(localBuildDir);
                    cleanedSomething = true;
                }

                if (cleanedSomething) {
                    writeln("🗑️  Cleaned locals: ".green, buildPath(targetDir, appName).white.bold);
                    cleanedCount++;
                }
            }
        }
    }
    
    // Nuke the segregated root build directory
    if (exists(rootBuildDir) && isDir(rootBuildDir)) {
         rmdirRecurse(rootBuildDir);
         writeln("\n🗑️  Wiped root build directory: ".green, rootBuildDir.white.bold);
    }

    writeln("==================================================".blue.bold);
    writeln("✨ Batch Clean Complete!".white.bold);
    writeln("Projects checked/cleaned: ".cyan, cleanedCount);
    writeln("==================================================".blue.bold);
}
