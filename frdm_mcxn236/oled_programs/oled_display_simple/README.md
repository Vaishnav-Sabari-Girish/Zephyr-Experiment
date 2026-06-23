# FRDM-MCXN236 Zephyr OLED Display Interface

A modular Zephyr RTOS application demonstrating how to drive a 1.3" Monochrome
OLED display (SH1106 controller) using the I2C bus and the Character Framebuffer
(CFB) subsystem on the NXP FRDM-MCXN236 development board.

* [`src/main.c`](./src/main.c)
* [`src/display_hal.h`](./src/display_hal.h)
* [`src/display_hal.c`](./src/display_hal.c)
* [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)

## Hardware Requirements

* **Development Board:** NXP FRDM-MCXN236
* **Display:** 1.3" I2C OLED (SH1106 controller)
* **Jumper Wires**

### Wiring Guide

The display is wired to the standard Arduino-compatible header on the
FRDM-MCXN236, which shares the `flexcomm2_lpi2c2` bus with the internal IMU.

| OLED Pin | FRDM-MCXN236 Pin | Notes |
| :--- | :--- | :--- |
| **SCL** | `P3_27` | I2C Clock |
| **SDA** | `P3_28` | I2C Data |
| **VCC** | `3.3V` | **Must use 3.3V logic level** |
| **GND** | `GND` | Common Ground |

## Project Structure

```text
oled_display_simple/
├── CMakeLists.txt
├── prj.conf
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree overlay for SH1106 on flexcomm2_lpi2c2
└── src/
    ├── main.c                  # Application entry point
    ├── display_hal.c           # Hardware Abstraction Layer for CFB routines
    └── display_hal.h           # HAL interface definitions

```

## Configuration Highlights

This project utilizes the unified Zephyr display driver framework. Key
configurations in `prj.conf` include:

* `CONFIG_SSD1306=y`: Enables the unified driver supporting the SH1106 hardware.
* `CONFIG_CHARACTER_FRAMEBUFFER=y`: Enables the CFB subsystem for rendering text
  geometry.
* `CONFIG_HEAP_MEM_POOL_SIZE=16384`: Required for `k_malloc` dynamic memory
  allocation utilized by the CFB subsystem to initialize the framebuffer.

## Building and Flashing

Ensure your Zephyr environment and `west` workspace are properly sourced.

1. **Build the application:**

```bash
west build -b frdm_mcxn236 -p pristine

```

2. **Flash to the target:**

```bash
west flash

```

## Expected Output

Upon successful flashing, the firmware will initialize the display and draw
"Hello, Zephyr!" at coordinates (10, 20).

Serial terminal output will confirm initialization:

```text
[00:00:00.000,000] <inf> main: Starting OLED application for FRDM-MCXN236
[00:00:00.005,000] <inf> display_hal: SH1106 Display initialized successfully.
[00:00:00.005,000] <inf> main: Drawing to screen...

```
