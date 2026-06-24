# FRDM-MCXN236 Zephyr IMU Bubble Level

A real-time Zephyr RTOS application that combines hardware sensor polling with
display graphics. It utilizes the FRDM-MCXN236's internal FXLS8974 accelerometer
to measure physical tilt and renders a dynamic, moving "bubble" over a crosshair
on a 1.3" Monochrome OLED (SH1106) using the Character Framebuffer (CFB)
graphics primitives.

* [`CMakeLists.txt`](./CMakeLists.txt)
* [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)
* [`prj.conf`](./prj.conf)
* [`src/main.c`](./src/main.c)
* [`src/display_hal.c`](./src/display_hal.c)
* [`src/display_hal.h`](./src/display_hal.h)

## Hardware Requirements

* **Development Board:** NXP FRDM-MCXN236 (Utilizing internal FXLS8974 IMU)
* **Display:** 1.3" I2C OLED (SH1106 controller)
* **Jumper Wires**

### Wiring Guide

Both the OLED and the internal IMU operate on the exact same I2C bus
(`flexcomm2_lpi2c2`). They coexist peacefully by using different I2C addresses
(OLED: `0x3C`, IMU: `0x18`).

| OLED Pin | FRDM-MCXN236 Pin | Notes |
| :--- | :--- | :--- |
| **SCL** | `P3_27` | I2C Clock |
| **SDA** | `P3_28` | I2C Data |
| **VCC** | `3.3V` | **Must use 3.3V logic level** |
| **GND** | `GND` | Common Ground |

## Project Structure

```text
oled_bubble_level/
├── CMakeLists.txt
├── prj.conf                    # Enables Sensor, IMU, and CFB subsystems
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree overlay for SH1106 and FXLS8974
└── src/
    ├── main.c                  # Acceleration math and rendering loop (~20 FPS)
    ├── display_hal.c           # Hardware Abstraction Layer with CFB graphics functions
    └── display_hal.h           

```

## Configuration Highlights

* **Sensor Subsystem:**
* `CONFIG_SENSOR=y` & `CONFIG_FXLS8974=y`: Activates the driver for the on-board
  accelerometer.
* `CONFIG_CBPRINTF_FP_SUPPORT=y`: Required to extract standard `double` values
  from the sensor struct.
* **Display Graphics:**
* `CONFIG_CHARACTER_FRAMEBUFFER=y`: Exposes CFB's underlying 2D geometry
  functions (`cfb_draw_line`, `cfb_draw_rect`).
* `CONFIG_HEAP_MEM_POOL_SIZE=16384`: Required to allocate RAM for the
  framebuffer.

## Building and Flashing

1. **Build the application:**

```bash
   west build -b frdm_mcxn236 -p pristine

```

2. **Flash to the target:**

```bash
   west flash

```

## Troubleshooting: I2C Bus Lockup

If you flash the board and immediately see `Failed to write pixel data` or
`Could not get WHOAMI value` in the serial terminal, the I2C bus is locked.

**Cause:** The microcontroller restarted mid-transmission, leaving the OLED
holding the data line down and deafening the bus. **Fix:** Perform a hard reset
by completely unplugging the board's USB power for 5 seconds to power-cycle the
OLED screen, then plug it back in.

## Expected Output

When laid flat, the OLED will display a static center crosshair with a small 5x5
pixel square perfectly centered. As you physically tilt the FRDM-MCXN236 board,
the accelerometer reads the gravitational shifts on the X and Y axes, and the
square will smoothly glide across the screen acting like a physical spirit
level.
