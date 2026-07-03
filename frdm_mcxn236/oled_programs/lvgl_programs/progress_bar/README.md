# FRDM-MCXN236 Zephyr LVGL Progress Bar Demo

A Zephyr RTOS application demonstrating how to create and dynamically update
graphical widgets using the Light and Versatile Graphics Library (LVGL). This
project renders a live, looping progress bar and a dynamically updating
percentage text label on a 1.3" Monochrome I2C OLED (SH1106).

* [`CMakeLists.txt`](./CMakeLists.txt)
* [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)
* [`prj.conf`](./prj.conf)
* [`src/main.c`](./src/main.c)

## Hardware Requirements

* **Development Board:** NXP FRDM-MCXN236
* **Display:** 1.3" I2C OLED (SH1106 controller)
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
oled_lvgl_progress_bar/
├── CMakeLists.txt
├── prj.conf                    # Contains the 1-bit memory and alignment fixes
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree mapping for the OLED
└── src/
    └── main.c                  # Widget creation and animation loop

```

## Key Engineering Concepts

* **Widget Parts and Styling:** LVGL widgets are often composed of multiple
  sub-elements. To make the progress bar visible on a 1-bit screen, we must
  explicitly style `LV_PART_MAIN` (the background track) to be black with a
  white border, and `LV_PART_INDICATOR` (the moving fill) to be solid white.
* **Dynamic UI Updates:** Unlike simpler graphics libraries where you must
  manually erase and redraw text, LVGL is state-driven. We simply call
  `lv_bar_set_value()` and `lv_label_set_text()` inside the main loop, and LVGL
  automatically calculates the pixel changes.
* **String Formatting in C:** The standard C `snprintf` function is utilized to
  safely convert our raw integer `progress` variable into a formatted string
  (e.g., `"42%"`) that the LVGL label widget can render.
* **The Task Handler:** The `lv_task_handler()` function sits inside the
  `while(1)` loop and acts as the heartbeat of the graphics engine, processing
  all changes made to the widgets and flushing the final calculated frame to the
  display driver.

## Building and Flashing

Since this project uses the exact same Kconfig (`prj.conf`) and Devicetree
overlay as the basic LVGL setup, a pristine build is not strictly required if
you are just modifying `main.c`.

1. **Build the application:**

```bash
west build -b frdm_mcxn236

```

2. **Flash to the target:**

```bash
west flash

```

## Expected Output

Upon booting, the OLED will display "System Boot" at the top center. Below it, a
progress bar will smoothly fill from left to right, accompanied by a percentage
counter ("0%" to "100%") at the bottom of the screen. Once it reaches 100%, it
pauses for half a second before looping back to 0%.
