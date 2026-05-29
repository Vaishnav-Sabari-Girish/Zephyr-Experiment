# Omen Project Context — Zephyr Programs

## Overview

Multi-board Zephyr RTOS example programs targeting three MCU platforms. The
repo is organized as a flat collection of per-board directories, each
containing self-contained Zephyr application folders.

## Supported Boards

| Directory          | Board Target            | MCU / SoC              |
| ------------------ | ----------------------- | ---------------------- |
| `nucleo-l433rc-p/` | `nucleo_l433rc_p`       | STM32L433RC (Cortex-M4) |
| `nrf52840dk/`      | `nrf52840dk/nrf52840`   | nRF52840 (Cortex-M4)    |
| `frdm_mcxa156/`    | `frdm_mcxa156`          | NXP MCXA156 (Cortex-M33) |

## Environment

- **Zephyr SDK**: Installed per
  [official docs](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html).
- **Zephyr source**: Expected at `$HOME/zephyrproject/zephyr/`.
- **Env var**: `ZEPHYR_BASE=$HOME/zephyrproject/zephyr/` (set in `.bashrc` or
  `.zshrc`).
- **Python venv**: `west` and Zephyr requirements installed in an activated
  virtual environment.

## Project Structure

Each program is a self-contained Zephyr application:

```text
<board>/<program>/
  ├── CMakeLists.txt   # find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
  ├── prj.conf         # KConfig overrides
  ├── src/
  │   └── main.c       # Application entry point
  └── (optional) Justfile, README.md, .gitignore
```

### CMakeLists.txt convention

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(<name>)
# Optional: set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_sources(app PRIVATE src/main.c)
```

## Build Commands

### Individual program

```bash
cd <board>/<program>
west build -b <board_target> .
```

Examples:

```bash
# STM32 Nucleo
west build -b nucleo_l433rc_p nucleo-l433rc-p/blinky

# nRF52840 DK
west build -b nrf52840dk/nrf52840 nrf52840dk/blinky/led_blink

# NXP FRDM-MCXA156
west build -b frdm_mcxa156 frdm_mcxa156/led_blink
```

### Batch build (all boards, all programs)

```bash
just build
# or
./scripts/batch_build.d
```

Build output goes to `build/<board>/<program>/` (segregated to prevent
collisions). A `compile_commands.json` symlink is created in each program
directory for clangd/LSP integration.

### Pristine rebuild

```bash
west build -p always -b <board_target> <program_path>
```

## Utility Scripts

All scripts are written in **D** and invoked via dub shebang
(`#!/usr/bin/env dub`). The `colored` dub dependency is resolved at runtime.

| Script                     | Purpose                                     |
| -------------------------- | ------------------------------------------- |
| `scripts/batch_build.d`    | Build all programs across all boards        |
| `scripts/batch_clean.d`    | Remove all build artifacts and symlinks     |
| `scripts/batch_size.d`     | Report `.text/.data/.bss` sizes via `size`  |
| `scripts/batch_check.d <C>`| Search generated `.config` for KConfig symbol |
| `scripts/batch_dts.d <N>`  | Search generated `zephyr.dts` for DTS node  |

`batch_check.d` accepts a config name with or without the `CONFIG_` prefix
(e.g., `just check GPIO` or `just check CONFIG_GPIO`).

`batch_dts.d` searches the final devicetree output (e.g.,
`just dts button0`).

## Justfile Shortcuts

| Command                  | Action                                      |
| ------------------------ | ------------------------------------------- |
| `just build`             | Batch build all programs                    |
| `just clean`             | Clean all artifacts + `.ccls-cache`, `.cache` |
| `just size`              | Binary size table                           |
| `just check <config>`    | KConfig search across all built projects    |
| `just dts <node>`        | Devicetree search across all built projects |

## Flashing

Board-specific flashing commands. The batch scripts do **not** flash.

```bash
# STM32 Nucleo L433RC-P (ST-Link)
west flash

# nRF52840 DK (J-Link on-board, or nrfjprog)
west flash

# FRDM-MCXA156 (J-Link)
west flash --runner jlink
```

## Pre-commit Hooks

`.githooks/pre-commit` runs:

1. `typos` — spell check
2. `rumdl check --fix` — auto-format all `*.md` files (except `CHANGELOG.md`)

Install with: `git config core.hooksPath .githooks`

## Tooling Config

- **typos**: `_typos.toml` — spell-check exceptions for "Zephyr", "RTOS",
  "CMake"
- **rumdl**: `rumdl.toml` — markdown lint, line-length 80, no reflow in code
  blocks
- **gitignore**: Ignores `build/`, `compile_commands.json`, `.ccls-cache/`,
  `omen.md`, `.omen.debug.log`

## Programs Per Board

### nucleo-l433rc-p (7 programs)

`blinky`, `button_interrupts`, `button_polling`, `msg_queue`, `print_console`,
`thread_sync`, `uart_shell`

### nrf52840dk (12 programs)

`blinky/led_blink`, `blinky/led_blink_all`, `button_interrupt`,
`button_polling`, `led_control_serial_comm`, `logging`, `message_queue`,
`print_console`, `sensors`, `serial_communication`,
`threads_semaphores/*` (2 variants), `time_slicing`, `uart_shell`,
`workqueue_program`

### frdm_mcxa156 (11 programs)

`button_interrupt`, `button_polling`, `led_blink`, `message_queue`, `nvs`,
`pwm_led`, `sensors`, `state_machines`, `threads_semaphores`, `uart_shell`,
`watchdog_timer`

## Notes

- No datasheets are currently stored in `./datasheets/`.
- No `.omen/` directory exists yet (created on first Omen session).
- The `nrf52840dk` board README says `nrf52830dk` in the build command — this
  appears to be a typo; the correct target is `nrf52840dk/nrf52840` (confirmed
  by `batch_build.d`).
