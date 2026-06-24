# FRDM-MCXN236 Zephyr Interactive OLED Menu

A Zephyr RTOS application that implements a multi-page interactive user
interface. It demonstrates how to combine hardware GPIO interrupts (via the
on-board user button) with an I2C OLED display (SH1106) and the internal LPADC
die temperature sensor.

* [`CMakeLists.txt`](./CMakeLists.txt)
* [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)
* [`prj.conf`](./prj.conf)
* [`src/main.c`](./src/main.c)
* [`src/display_hal.c`](./src/display_hal.c)
* [`src/display_hal.h`](./src/display_hal.h)

## Hardware Requirements

* **Development Board:** NXP FRDM-MCXN236
* **Display:** 1.3" I2C OLED (SH1106 controller)
* **Input:** On-board user button (mapped to `sw0` in Zephyr, usually SW2 or SW3
  physically)
* **Jumper Wires**

### Wiring Guide

| OLED Pin | FRDM-MCXN236 Pin | Notes |
| :--- | :--- | :--- |
| **SCL** | `P3_27` | I2C Clock |
| **SDA** | `P3_28` | I2C Data |
| **VCC** | `3.3V` | **Must use 3.3V logic level** |
| **GND** | `GND` | Common Ground |

## Project Structure

```text
oled_interactive_menu/
├── CMakeLists.txt
├── prj.conf                    # Enables GPIO, I2C, Display, Sensor, and ADC
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree overlay for OLED and LPADC
└── src/
    ├── main.c                  # Interrupt Service Routine (ISR) and UI state machine
    ├── display_hal.c           # Hardware Abstraction Layer with CFB graphics
    └── display_hal.h           

```

## Key Engineering Concepts

* **Hardware Interrupts (ISRs):** Instead of constantly checking if the button
  is pressed (polling), the GPIO pin triggers an interrupt
  (`button_pressed_isr`) the exact microsecond the voltage drops. This halts the
  main loop, updates the UI state, and immediately resumes.
* **Software Debouncing:** Physical buttons vibrate (bounce) microscopically
  when pressed, which can trigger dozens of interrupts in milliseconds. The ISR
  utilizes `k_uptime_get()` to enforce a 250ms ignore-window after the initial
  press, ensuring clean single-page turns.
* **UI State Machine:** A `switch/case` block driven by a `current_screen`
  variable renders entirely different layouts (Uptime, Temperature, System Info)
  dynamically based on the user's navigation.

## Configuration Highlights

* `CONFIG_GPIO=y`: Enables the GPIO subsystem required to read the physical
  button state and attach hardware interrupts.
* `CONFIG_CHARACTER_FRAMEBUFFER=y`: Enables drawing text and the static 2D UI
  dividing line.
* `CONFIG_CBPRINTF_FP_SUPPORT=y`: Allows `snprintf` to format the floating-point
  die temperature.

## Building and Flashing

1. **Build the application:**

```bash
   west build -b frdm_mcxn236 -p pristine

```

2. **Flash to the target:**

```bash
   west flash

```

## Expected Output

When the board boots, the display will show **Screen 0 (Uptime)**, rendering a
live clock ticking up the seconds since boot.

Press the on-board user button to instantly cycle through the menu:

* **Screen 0:** Live System Uptime (`HH:MM:SS`)
* **Screen 1:** Live Silicon Die Temperature

Each screen features a horizontal 2D dividing line separating the title from the
data payload.
