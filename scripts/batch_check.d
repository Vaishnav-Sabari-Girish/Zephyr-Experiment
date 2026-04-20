#!/usr/bin/env dub

/+ dub.sdl:
       name "batch_check"
       description "Zephyr Kconfig Inspector"
       dependency "colored" version="~>0.0.32"
+/ 

import std.stdio : writeln, writefln;
import std.file : dirEntries, SpanMode, isDir, exists, readText;
import std.path : buildPath, baseName, dirName;
import std.algorithm : filter, canFind;
import std.string : splitLines, toUpper, strip;
import colored;

void main(string[] args) {
    if (args.length < 2) {
        writeln("❌ Error: Please provide a configuration to search for.".red);
        writeln("Usage: ".cyan, "./scripts/batch_check.d <CONFIG_NAME>");
        writeln("Example: ".cyan, "./scripts/batch_check.d I2C\n");
        return;
    }

    // Convert the search term to uppercase (e.g., "uart" -> "UART")
    string searchTerm = args[1].toUpper();
    // Prepend CONFIG_ if the user forgot it, so we match exactly
    if (!searchTerm.canFind("CONFIG_")) {
        searchTerm = "CONFIG_" ~ searchTerm;
    }

    string[] targetDirs = ["nucleo-l433rc-p", "nrf52840dk"];

    writeln("🔍 Searching for ".blue.bold, searchTerm.magenta.bold, " across built projects...\n".blue.bold);

    int projectsChecked = 0;
    int matchesFound = 0;

    foreach (targetDir; targetDirs) {
        if (!exists(targetDir) || !isDir(targetDir)) continue;

        auto entries = dirEntries(targetDir, SpanMode.depth)
            .filter!(e => e.isFile && baseName(e.name) == "CMakeLists.txt" && !e.name.canFind("/build/"));

        foreach (entry; entries) {
            string appPath = dirName(entry.name);
            string appName = baseName(appPath);
            
            // Look for the generated Zephyr config file in our segregated build folder
            string configFile = buildPath("build", targetDir, appName, "zephyr", ".config");

            if (exists(configFile)) {
                projectsChecked++;
                string content = readText(configFile);
                string[] lines = content.splitLines();
                
                bool foundInProject = false;

                foreach (line; lines) {
                    line = line.strip();
                    // Ignore empty lines and standard comments (unless it's a "is not set" comment)
                    if (line.length == 0) continue;
                    
                    // Check if the line contains our search term
                    if (line.canFind(searchTerm)) {
                        if (!foundInProject) {
                            writeln("📦 ", targetDir.cyan, " / ", appName.white.bold);
                            foundInProject = true;
                        }

                        // Color code the output based on whether it's enabled or disabled
                        if (line.canFind("=y")) {
                            writeln("   ✅ ".green, line.green);
                        } else if (line.canFind("is not set")) {
                            writeln("   ❌ ".red, line.red);
                        } else {
                            // Catch-all for string/hex values (e.g., CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC=12000000)
                            writeln("   🔹 ".yellow, line.yellow);
                        }
                        matchesFound++;
                    }
                }
                
                if (foundInProject) {
                    writeln("--------------------------------------------------");
                }
            }
        }
    }

    if (projectsChecked == 0) {
        writeln("⚠️  No compiled .config files found. Run your build script first!".yellow);
    } else if (matchesFound == 0) {
        writeln("👻 No matches found for ".yellow, searchTerm.white.bold, " in any compiled project.".yellow);
    }
}
