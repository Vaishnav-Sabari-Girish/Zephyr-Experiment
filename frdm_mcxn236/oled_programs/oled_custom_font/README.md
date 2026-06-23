# FRDM-MCXN236 Zephyr Custom Font & Die Temperature Display

A custom Zephyr RTOS application for the NXP FRDM-MCXN236 that fetches the
internal silicon die temperature and renders it on a 1.3" Monochrome OLED
(SH1106) using a dynamically generated TrueType pixel font.

This project demonstrates advanced utilization of the Character Framebuffer
(CFB) subsystem, specifically bypassing default fonts to generate and utilize
custom `.ttf` files during the CMake build process.

- [`CMakeLists.txt`](./CMakeLists.txt)
- [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)
- [`prj.conf`](./prj.conf)
- [`src/main.c`](./src/main.c)
- [`src/display_hal.c`](./src/display_hal.c)
- [`src/display_hal.h`](./src/display_hal.h)

## Hardware Requirements

- **Development Board:** NXP FRDM-MCXN236 (Utilizing internal `temp40` die
  sensor)
- **Display:** 1.3" I2C OLED (SH1106 controller)
- **Jumper Wires**

### Wiring Guide

The OLED shares the `flexcomm2_lpi2c2` I2C bus with the board's internal IMU.

| OLED Pin | FRDM-MCXN236 Pin | Notes |
| :--- | :--- | :--- |
| **SCL** | `P3_27` | I2C Clock |
| **SDA** | `P3_28` | I2C Data |
| **VCC** | `3.3V` | **Must use 3.3V logic level** |
| **GND** | `GND` | Common Ground |

## Project Structure

```text
oled_custom_font/
├── CMakeLists.txt              # Configured to run Zephyr's gen_cfb_font_header.py
├── prj.conf
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree overlay for SH1106 and LPADC
├── fonts/
│   └── silkscreen.ttf          # Source vector pixel font 
└── src/
    ├── main.c                  # Main polling and rendering loop
    ├── display_hal.c           # Hardware Abstraction Layer with custom font logic
    └── display_hal.h           

```

## The TrueType Font Vector Trap (Why 9x16 instead of 8x8?)

This project uses the "Silkscreen" font. While visually the letters appear as
tiny 8x8 pixel blocks, the font generation in `CMakeLists.txt` is strictly set
to **9 width** and **16 height**.

**The Explanation:** `.ttf` files are vector graphics, not raw pixel arrays.
TrueType fonts contain invisible vertical padding (bounding boxes) to
accommodate ascenders, descenders, and natural line spacing. When Zephyr's
`gen_cfb_font_header.py` script mathematical rasterizes the 8px visual letters,
it includes this invisible padding, resulting in an actual compiled height of 16
pixels and a natural kerning width of 9 pixels.

Attempting to force the script to `8 8` results in a build exception
(`Exception: text height 16 mismatch with -y 8`).

**The Solution Implemented:**

1. **CMake:** We accommodate the mathematical reality of the `.ttf` file by
   setting the generation target to `9 16`.
2. **HAL Search Logic:** Because Zephyr now contains *two* 16px high fonts (the
   default 10x16 and our custom 9x16), `display_hal_set_font()` was upgraded to
   require exact matches for **both** width and height to prevent the display
   driver from picking the wrong font.

## Technical Notes & Configurations

- **Font Include Pathing:** Zephyr automatically adds the CMake build's
  `generated/` folder to the compiler's include path. Therefore, the font is
  included in `display_hal.c` simply as `#include <cfb_font_9x16.h>`, omitting
  the directory prefix.
- **Stack Size:** `CONFIG_MAIN_STACK_SIZE=4096` is enabled to support safe
  `snprintf` floating-point conversions for the temperature data.
- **Heap Memory:** `CONFIG_HEAP_MEM_POOL_SIZE=16384` provides the dynamic memory
  required for the CFB framebuffer (`k_malloc`).

## Building and Flashing

Ensure your Zephyr environment and `west` workspace are properly sourced.

1. **Build the application:**
*(Note: A pristine build is recommended when changing font generation
parameters)*

```bash
west build -b frdm_mcxn236 -p pristine

```

2. **Flash to the target:**

```bash
west flash
```

## Expected Output

The display will initialize, wipe the RAM buffer, and draw a multi-layered frame
every second:

1. **"DIE TEMP"** rendered at coordinates (0,0) using the default large font
   (15x24).
2. **"Temp: XX.XX C"** rendered at coordinates (0,40) utilizing the dynamically
   generated custom Silkscreen font (9x16).
