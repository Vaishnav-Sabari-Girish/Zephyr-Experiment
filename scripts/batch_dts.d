#!/usr/bin/env dub

/+ dub.sdl:
       name "batch_dts"
       description "Zephyr Devicetree Inspector"
       dependency "colored" version="~>0.0.32"
+/ 

import std.stdio : writeln, writefln, writef, write; 
import std.file : dirEntries, SpanMode, isDir, exists, readText;
import std.path : buildPath, baseName, dirName;
import std.algorithm : filter, canFind;
import std.string : splitLines, toLower, strip, endsWith, startsWith, indexOf;
import std.regex : ctRegex, replaceAll;
import std.conv : to; 
import colored;

void main(string[] args) {
    if (args.length < 2) {
        writeln("❌ Error: Please provide a hardware node or pin to search for.".red);
        writeln("Usage: ".cyan, "./scripts/batch_dts.d <SEARCH_TERM>");
        writeln("Example: ".cyan, "./scripts/batch_dts.d uart0\n");
        return;
    }

    string searchTerm = args[1].toLower();
    string[] targetDirs = ["nucleo-l433rc-p", "nrf52840dk"];

    writeln("🌳 Searching hardware tree for ".blue.bold, searchTerm.magenta.bold, "...\n".blue.bold);

    int projectsChecked = 0;
    int matchesFound = 0;

    foreach (targetDir; targetDirs) {
        if (!exists(targetDir) || !isDir(targetDir)) continue;

        auto entries = dirEntries(targetDir, SpanMode.depth)
            .filter!(e => e.isFile && baseName(e.name) == "CMakeLists.txt" && !e.name.canFind("/build/"));

        foreach (entry; entries) {
            string appPath = dirName(entry.name);
            string appName = baseName(appPath);
            
            string dtsFile = buildPath("build", targetDir, appName, "zephyr", "zephyr.dts");

            if (exists(dtsFile)) {
                projectsChecked++;
                string content = readText(dtsFile);
                string[] lines = content.splitLines();
                
                bool foundInProject = false;

                // Simple Block Parser State Machine
                string currentNode = "";
                string[] keys;
                string[] vals;
                bool nodeMatches = false;

                foreach (line; lines) {
                    string s = line.strip();
                    if (s.length == 0) continue;

                    // Node opens
                    if (s.endsWith("{")) {
                        currentNode = s[0 .. $-1].strip();
                        keys.length = 0;
                        vals.length = 0;
                        nodeMatches = currentNode.toLower().canFind(searchTerm);
                    } 
                    // Node closes
                    else if (s.startsWith("}")) { 
                        if (nodeMatches && keys.length > 0) {
                            if (!foundInProject) {
                                writeln("📦 ", targetDir.cyan, " / ", appName.white.bold);
                                foundInProject = true;
                            }

                            writeln("📌 Node: ".cyan, currentNode.magenta.bold);
                            
                            auto table = Table(["Property", "Value"]);
                            foreach (i; 0 .. keys.length) {
                                table.addRow([
                                    keys[i].green.to!string, 
                                    vals[i].yellow.to!string
                                ]);
                            }
                            table.print();
                            writeln();
                            matchesFound++;
                        }
                        currentNode = "";
                        nodeMatches = false;
                    } 
                    // Inside a node, grabbing properties
                    else if (currentNode != "") {
                        string key, val;
                        auto eqIdx = s.indexOf("=");
                        
                        if (eqIdx != -1) {
                            key = s[0 .. eqIdx].strip();
                            val = s[eqIdx + 1 .. $].strip();
                            // Remove trailing semicolons from values
                            if (val.endsWith(";")) val = val[0 .. $-1];
                        } else {
                            key = s;
                            // Remove trailing semicolons from boolean/empty properties
                            if (key.endsWith(";")) key = key[0 .. $-1];
                            val = "-";
                        }
                        
                        keys ~= key;
                        vals ~= val;
                        
                        if (s.toLower().canFind(searchTerm)) {
                            nodeMatches = true;
                        }
                    }
                }
                
                if (foundInProject) {
                    writeln("--------------------------------------------------");
                }
            }
        }
    }

    if (projectsChecked == 0) {
        writeln("⚠️  No compiled zephyr.dts files found. Run your build script first!".yellow);
    } else if (matchesFound == 0) {
        writeln("👻 No hardware matches found for ".yellow, searchTerm.white.bold, ".".yellow);
    }
}

// ==========================================
// Reusable Table Struct
// ==========================================
struct Table {
    string[] headers;
    string[][] rows;
    int[] colWidths;
    
    enum ansiRegex = ctRegex!`\x1B\[[0-9;]*[mK]`;

    this(string[] h) {
        headers = h;
        colWidths = new int[headers.length];
        foreach (i, header; headers) {
            colWidths[i] = visibleLength(header);
        }
    }

    void addRow(string[] row) {
        rows ~= row;
        foreach (i, item; row) {
            int len = visibleLength(item);
            if (len > colWidths[i]) {
                colWidths[i] = len;
            }
        }
    }

    void print() {
        printSeparator();
        
        foreach (i, header; headers) {
            int padding = colWidths[i] - visibleLength(header);
            writef("| %s%*s ", header.cyan.bold, padding, "");
        }
        writeln("|");
        
        printSeparator();

        foreach (row; rows) {
            foreach (i, item; row) {
                int padding = colWidths[i] - visibleLength(item);
                writef("| %s%*s ", item, padding, "");
            }
            writeln("|");
        }
        
        printSeparator();
    }

    private void printSeparator() {
        foreach (width; colWidths) {
            write("+");
            for(int i = 0; i < width + 2; i++) write("-");
        }
        writeln("+");
    }

    private int visibleLength(string s) {
        return cast(int)(s.replaceAll(ansiRegex, "").length);
    }
}
