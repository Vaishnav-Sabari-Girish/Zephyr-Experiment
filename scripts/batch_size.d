#!/usr/bin/env dub

/+ dub.sdl:
       name "batch_size"
       description "Zephyr Firmware Size Reporter"
       dependency "colored" version="~>0.0.32"
+/ 

import std.stdio : writeln, writefln, writef, write; 
import std.file : dirEntries, SpanMode, isDir, exists;
import std.path : buildPath, baseName, dirName;
import std.process : execute;
import std.algorithm : filter, canFind;
import std.string : splitLines, split, strip;
import std.regex : ctRegex, replaceAll;
import std.conv : to; 
import std.format : format;
import colored;

void main() {
    string[] targetDirs = ["nucleo-l433rc-p", "nrf52840dk"];

    writeln("📊 Starting Multi-Board Firmware Size Profiler...".blue.bold);
    
    auto table = Table(["Board", "Application", "Text/ROM (KB)", "Data/RAM (KB)", "BSS (KB)", "Total (KB)"]);

    int foundCount = 0;

    foreach (targetDir; targetDirs) {
        if (!exists(targetDir) || !isDir(targetDir)) continue;

        // DEEP SEARCH
        auto entries = dirEntries(targetDir, SpanMode.depth)
            .filter!(e => e.isFile && baseName(e.name) == "CMakeLists.txt" && !e.name.canFind("/build/"));

        foreach (entry; entries) {
            string appPath = dirName(entry.name);
            string appName = baseName(appPath);
            
            // Check local build directory first
            string elfFile = buildPath(appPath, "build", "zephyr", "zephyr.elf");
            
            // Fallback to our segregated batch_builder directory
            if (!exists(elfFile)) {
                elfFile = buildPath("build", targetDir, appName, "zephyr", "zephyr.elf");
            }

            if (exists(elfFile)) {
                foundCount++;
                auto sizeResult = execute(["size", elfFile]);
                
                if (sizeResult.status == 0) {
                    string[] lines = sizeResult.output.strip().splitLines();
                    
                    if (lines.length > 1) {
                        string[] columns = lines[1].split();
                        if (columns.length >= 4) {
                            try {
                                float textKB  = columns[0].to!float / 1024.0;
                                float dataKB  = columns[1].to!float / 1024.0;
                                float bssKB   = columns[2].to!float / 1024.0;
                                float totalKB = columns[3].to!float / 1024.0; 

                                string textStr  = format("%.2f", textKB);
                                string dataStr  = format("%.2f", dataKB);
                                string bssStr   = format("%.2f", bssKB);
                                string totalStr = format("%.2f", totalKB);

                                table.addRow([
                                    targetDir.cyan.to!string,
                                    appName.white.bold.to!string, 
                                    textStr.green.to!string, 
                                    dataStr.yellow.to!string, 
                                    bssStr.magenta.to!string, 
                                    totalStr.cyan.to!string
                                ]);
                            } catch (Exception e) {
                                table.addRow([
                                    targetDir.cyan.to!string,
                                    appName.white.bold.to!string, 
                                    "Parse Error".red.to!string, 
                                    "-", "-", "-"
                                ]);
                            }
                        }
                    }
                } else {
                    table.addRow([
                        targetDir.cyan.to!string,
                        appName.white.bold.to!string, 
                        "Error".red.to!string, 
                        "-", "-", "-"
                    ]);
                }
            }
        }
    }

    if (foundCount > 0) {
        writeln(); 
        table.print();
    } else {
        writeln("\nNo compiled binaries found. Run your build script first!".yellow);
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
