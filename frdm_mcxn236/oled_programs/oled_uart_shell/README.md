# FRDM-MCXN236 Zephyr OLED Shell Terminal

A highly interactive Zephyr RTOS application that bridges the serial console and
a physical display. This project leverages the Zephyr Shell Subsystem to create
a command-line interface (CLI) over UART, allowing users to type custom text
commands from their computer and instantly render them on a 1.3" Monochrome OLED
(SH1106).

* [`CMakeLists.txt`](./CMakeLists.txt)
* [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)
* [`prj.conf`](./prj.conf)
* [`src/main.c`](./src/main.c)
* [`src/display_hal.c`](./src/display_hal.c)
* [`src/display_hal.h`](./src/display_hal.h)

## Hardware Requirements

* **Development Board:** NXP FRDM-MCXN236
* **Display:** 1.3" I2C OLED (SH1106 controller)
* **Jumper Wires**

### Wiring Guide

The OLED is connected to the standard I2C header (`flexcomm2_lpi2c2`).

| OLED Pin | FRDM-MCXN236 Pin | Notes |
| :--- | :--- | :--- |
| **SCL** | `P3_27` | I2C Clock |
| **SDA** | `P3_28` | I2C Data |
| **VCC** | `3.3V` | **Must use 3.3V logic level** |
| **GND** | `GND` | Common Ground |

## Project Structure

```text
oled_shell_terminal/
├── CMakeLists.txt
├── prj.conf                    # Enables Shell, I2C, Display, and CFB
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree overlay for the SH1106 OLED
└── src/
    ├── main.c                  # Shell command registration and parsing logic
    ├── display_hal.c           # Hardware Abstraction Layer with CFB graphics
    └── display_hal.h           

```

## Key Engineering Concepts

* **Zephyr Shell Subsystem:** Instead of writing custom UART byte-parsing state
  machines, Zephyr provides a fully featured, POSIX-like shell. It runs
  automatically in a background thread and handles backspaces, arrow keys, and
  tab-completion.
* **Shell Command Registration:** Using Zephyr macros (`SHELL_CMD_REGISTER` and
  `SHELL_STATIC_SUBCMD_SET_CREATE`), custom functions are mapped to terminal
  keywords.
* **Argument Parsing:** The shell automatically splits user input into an
  argument count (`argc`) and an array of string pointers (`argv`), making it
  incredibly easy to parse multi-word messages.
* **Threadless Main Loop:** Because the Shell subsystem operates on its own
  dedicated thread, the `main()` function in this project only handles hardware
  initialization and then simply returns 0. No `while(1)` loop is required!

## Configuration Highlights

* `CONFIG_SHELL=y`: Transforms the standard serial logging console into an
  interactive command prompt (`uart:~$`).
* `CONFIG_SHELL_LOG_BACKEND=y`: Ensures that standard `LOG_INF` and `LOG_ERR`
  messages coexist peacefully with the shell prompt without mangling text.
* `CONFIG_CHARACTER_FRAMEBUFFER=y`: Provides the underlying text-rendering
  engine for the OLED.
* `CONFIG_SHELL_STACK_SIZE=4096`: Increases the memory allocated to the shell
  thread to safely handle large string buffers.

## Building and Flashing

1. **Build the application:**

```bash
   west build -b frdm_mcxn236 -p pristine

```

2. **Flash to the target:**

```bash
   west flash

```

## Expected Output & Usage Guide

When the board boots, the OLED will display a "ZEPHYR Shell Ready" welcome
screen.

Open your preferred serial terminal (e.g., PuTTY, minicom, or standard screen)
configured to `115200` baud and press **Enter** to wake the shell. You will be
greeted by the command prompt:

```text
uart:~$

```

You can now use the `oled` root command. Use tab-completion or type `help` to
explore!

**1. Displaying a Message:**
Type the `oled msg` command followed by any text you want:

```text
uart:~$ oled msg Hello Zephyr RTOS!
Success! Sent to OLED: Hello Zephyr RTOS! 

```

*The OLED will instantly clear and display "Hello Zephyr RTOS!" on the screen.*

**2. Clearing the Screen:**

```text
uart:~$ oled clear
OLED Screen Cleared.

```

*The OLED will wipe completely blank.*
