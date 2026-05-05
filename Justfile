# List all available commands by default
default:
    @just --list

# Build all Zephyr projects across all configured boards
build:
    @./scripts/batch_build.d

# Clean all build artifacts, local folders, and symlinks
clean:
    @./scripts/batch_clean.d
    @find . -type d -name ".ccls-cache" -exec rm -rf {} +
    @find . -type d -name ".cache" -exec rm -rf {} +

# Display a memory profiling table for all compiled binaries
size:
    @./scripts/batch_size.d

# Inspect generated KConfig files 
check config-name:
    @./scripts/batch_check.d {{config-name}}

dts dts_node:
    @./scripts/batch_dts.d {{dts_node}}
