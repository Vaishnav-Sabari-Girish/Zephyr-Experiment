# FRDM-MCXN236 Zephyr LVGL Circular Arc (Gauge) Demo

A Zephyr RTOS application demonstrating how to create and style a professional
circular arc (gauge) widget using the Light and Versatile Graphics Library
(LVGL). This project renders a dynamic, bouncing "CPU LOAD" dashboard on a 1.3"
Monochrome I2C OLED (SH1106), optimized specifically for 1-bit display
limitations.

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
oled_lvgl_arc/
├── CMakeLists.txt
├── prj.conf                    # Contains 1-bit memory, font, and alignment fixes
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree mapping for the OLED display
└── src/
    └── main.c                  # Arc widget creation, split layout, and animation loop

```

## Key Engineering Concepts

Building complex widgets on a low-resolution, 1-bit screen requires specific
styling and layout techniques to prevent visual artifacting:

* **Bounding Box Annihilation:** By default, LVGL draws a background box behind
  widgets. On color screens, it is transparent, but on a 1-bit screen, LVGL
  rounds the transparency to solid white. We force the background to black and
  the border to `0` to keep the arc floating cleanly on the screen.
* **Obliterating the "Knob":** Arcs are typically used as touchscreen sliders
  and feature a circular "knob" at the end of the line. Because
  opacity/transparency fails on 1-bit screens, we use `lv_obj_remove_style()` to
  completely strip the knob from memory, leaving only the gauge line.
* **Contrast via Thickness:** Since we cannot use colors (e.g., green for good,
  red for bad), we use line thickness to create visual contrast. The empty track
  (`LV_PART_MAIN`) is a thin 1-pixel line, while the active fill
  (`LV_PART_INDICATOR`) is a thick 4-pixel line.
* **Side-by-Side Spatial Layout:** An OLED that is 128x64 is twice as wide as it
  is tall. Because our arc takes up 54 pixels vertically, stacking a title above
  it would exceed the 64-pixel limit and cause overlapping. We utilized
  `lv_obj_align()` to shift the arc to the right, and `lv_obj_align_to()` to
  lock the percentage text precisely inside the center of the arc.

## Building and Flashing

Since this project utilizes the exact same underlying LVGL memory configuration
and Devicetree overlay as the previous project setups, a standard build will
work perfectly.

1. **Build the application:**

```bash
west build -b frdm_mcxn236

```

2. **Flash to the target:**

```bash
west flash

```

## Expected Output

Upon booting, the OLED will display a clean, dual-pane layout. On the left, a
static "CPU LOAD" title will appear. On the right, a circular gauge will
smoothly bounce back and forth between 0% and 100%, with the exact percentage
text dynamically updating perfectly centered inside the gauge.
