# FRDM-MCXN236 Zephyr LVGL Live Chart Demo

A Zephyr RTOS application demonstrating how to plot dynamic, real-time data
using the Light and Versatile Graphics Library (LVGL) Chart widget. This project
renders a live-scrolling line graph on a 1.3" Monochrome I2C OLED (SH1106),
simulating real sensor telemetry.

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
oled_lvgl_chart/
├── CMakeLists.txt
├── prj.conf                    # Contains the 1-bit memory and alignment fixes
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree mapping for the OLED
└── src/
    └── main.c                  # Chart widget creation and data simulation loop

```

## Key Engineering Concepts

* **Data Series (`lv_chart_series_t`):** LVGL charts manage data through
  "series." You can attach multiple lines to a single chart (e.g., plotting both
  temperature and humidity simultaneously) by simply adding another series.
* **Auto-Scrolling FIFO Buffer:** The function `lv_chart_set_next_value()`
  automatically handles the array logic. Because we set
  `lv_chart_set_point_count()` to 25, LVGL maintains exactly 25 data points in
  memory. When a new value is pushed, it drops the oldest point and shifts the
  entire graph to the left automatically.
* **Monochrome Styling Tweak (Markers):** By default, LVGL places a circular
  marker (dot) on every data point on a line graph. On a low-resolution 128x64
  black-and-white screen, these dots cluster together and create a messy,
  unreadable blob. By setting the `LV_PART_INDICATOR` size to `0`, we hide the
  dots and leave only the clean, crisp connecting line.
* **Simulating Telemetry:** To demonstrate the chart without wiring up a
  physical sensor, the `while(1)` loop uses a simple math algorithm (a bouncing
  triangle wave) to continuously generate data that moves smoothly between 10
  and 90.

## Building and Flashing

Since this project utilizes the same underlying LVGL memory configuration and
Devicetree overlay as the previous setups, a standard build will work perfectly.

1. **Build the application:**

```bash
west build -b frdm_mcxn236

```

2. **Flash to the target:**

```bash
west flash

```

## Expected Output

When the board boots, the OLED will display a "Live Sensor Data" title at the
top. Below it, a line graph will continuously scroll from right to left,
plotting a smooth, bouncing zig-zag line to simulate real-time telemetry updates
happening at 10 frames per second.
