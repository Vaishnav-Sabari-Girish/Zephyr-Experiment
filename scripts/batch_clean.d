#!/usr/bin/env dub

/+ dub.sdl:
       name "batch_clean"
       description "Zephyr Batch Cleaner"
       dependency "colored" version="~>0.0.32"
+/ 

import std.stdio : writeln, writefln;
import std.file : dirEntries, SpanMode, isDir, exists, remove, rmdirRecurse;
import std.path : buildPath, baseName, dirName;
import std.algorithm : canFind;
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

        // PHASE 1: GATHER
        // Collect all project paths into a static array first so we don't 
        // mutate the file system while the iterator is actively crawling it.
        string[] appPaths;
        foreach (entry; dirEntries(targetDir, SpanMode.depth)) {
            if (entry.isFile && baseName(entry.name) == "CMakeLists.txt" && !entry.name.canFind("/build/")) {
                appPaths ~= dirName(entry.name);
            }
        }

        // PHASE 2: DESTROY
        // Now it is perfectly safe to delete the build folders
        foreach (appPath; appPaths) {
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
                writeln("🗑️  Cleaned locals: ".green, appPath.white.bold);
                cleanedCount++;
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
