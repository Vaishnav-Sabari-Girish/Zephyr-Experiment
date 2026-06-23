# FRDM-MCXN236 Zephyr Die Temperature & OLED Display

A modular Zephyr RTOS application that fetches real-time telemetry from the
FRDM-MCXN236's internal silicon die temperature sensor (LPADC) and displays it
on a 1.3" Monochrome OLED (SH1106 controller). This project demonstrates I2C
display integration, ADC sensor polling, and advanced use of the Character
Framebuffer (CFB) subsystem, including multi-font rendering and custom font
generation.

* [`src/main.c`](./src/main.c)
* [`src/display_hal.h`](./src/display_hal.h)
* [`src/display_hal.c`](./src/display_hal.c)
* [`boards/frdm_mcxn236.overlay`](./boards/frdm_mcxn236.overlay)
* [`CMakeLists.txt`](./CMakeLists.txt)

## Hardware Requirements

* **Development Board:** NXP FRDM-MCXN236 (Utilizing internal `temp40` die
  sensor)
* **Display:** 1.3" I2C OLED (SH1106 controller)
* **Jumper Wires**

### Wiring Guide

The display is wired to the standard Arduino-compatible header on the
FRDM-MCXN236, communicating over the `flexcomm2_lpi2c2` bus.

| OLED Pin | FRDM-MCXN236 Pin | Notes |
| :--- | :--- | :--- |
| **SCL** | `P3_27` | I2C Clock |
| **SDA** | `P3_28` | I2C Data |
| **VCC** | `3.3V` | **Must use 3.3V logic level** |
| **GND** | `GND` | Common Ground |

## Project Structure

```text
oled_die_temp/
├── CMakeLists.txt              # Build instructions, including custom font generation
├── prj.conf
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree overlay for SH1106 and LPADC temperature node
└── src/
    ├── main.c                  # Application loop: Sensor polling & display layering
    ├── display_hal.c           # Hardware Abstraction Layer for CFB routines
    └── display_hal.h           # HAL interface definitions

```

## Configuration Highlights

This project utilizes several Zephyr subsystems across sensors and displays. Key
configurations in `prj.conf` include:

* **Sensor & ADC Integration:**
* `CONFIG_ADC=y` & `CONFIG_SENSOR=y`: Enables the LPADC subsystem to read the
  internal die temperature.
* `CONFIG_CBPRINTF_FP_SUPPORT=y`: Required to format floating-point values
  (`%.2f`) for the temperature output.
* `CONFIG_MAIN_STACK_SIZE=4096`: Expanded thread memory to handle floating-point
  string formatting safely.
* **Display Subsystem:**
* `CONFIG_SSD1306=y`: Enables the unified driver supporting the SH1106 hardware.
* `CONFIG_CHARACTER_FRAMEBUFFER=y`: Enables the CFB subsystem.
* `CONFIG_HEAP_MEM_POOL_SIZE=16384`: Provides dynamic memory allocation space
  for the CFB framebuffer (`k_malloc`).

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

Upon successful flashing, the firmware will initialize the display and begin
polling the sensor every second. The OLED will display a static large 24px title
("SYS TEMP:") with a dynamically updating custom 8px text string below it
showing the live die temperature.

Serial terminal output will confirm initialization and stream the live data:

```log
*** Booting Zephyr OS build v4.4.0-313-g4e7fcbb2d84d ***
[00:00:00.011,000] <inf> die_oled_logger: Starting the Die Temperature and OLED application
[00:00:00.011,000] <inf> die_oled_logger: Sensor temp0 initialized successfully
[00:00:00.012,000] <inf> die_oled_logger: Silicon Die temperature: 23.80 C
[00:00:00.012,000] <inf> display_hal: Font changed successfully to 15x24
[00:00:00.012,000] <inf> display_hal: Font changed successfully to 10x16
[00:00:01.039,000] <inf> die_oled_logger: Silicon Die temperature: 23.26 C
[00:00:01.039,000] <inf> display_hal: Font changed successfully to 15x24
[00:00:01.040,000] <inf> display_hal: Font changed successfully to 10x16
[00:00:02.067,000] <inf> die_oled_logger: Silicon Die temperature: 22.02 C
[00:00:02.067,000] <inf> display_hal: Font changed successfully to 15x24
[00:00:02.067,000] <inf> display_hal: Font changed successfully to 10x16
[00:00:03.095,000] <inf> die_oled_logger: Silicon Die temperature: 23.69 C
```
