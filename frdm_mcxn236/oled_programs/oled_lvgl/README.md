# FRDM-MCXN236 Zephyr LVGL Monochrome OLED

A robust Zephyr RTOS application demonstrating how to configure and run the
Light and Versatile Graphics Library (LVGL) on a 1-bit monochrome I2C OLED
(SH1106). This project completely bypasses the basic Character Framebuffer (CFB)
subsystem in favor of LVGL's advanced, widget-based rendering engine.

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
oled_lvgl_basic/
├── CMakeLists.txt
├── prj.conf                    # Crucial LVGL memory and 1-bit alignment fixes
├── boards/
│   └── frdm_mcxn236.overlay    # Devicetree mapping with zephyr,display chosen node
└── src/
    └── main.c                  # Defensive initialization and UI rendering

```

## The Monochrome Traps (Why we configured it this way)

Running a massive graphics library like LVGL on a tiny black-and-white screen
presents several unique hardware challenges. Here is exactly how this project
solves them:

### 1. The "Grainy Screen" Memory Corruption

By default, LVGL tries to save CPU power by only redrawing the exact pixels that
change (e.g., a tiny 5x5 box). However, monochrome OLED drivers (like the SH1106
or ST7567) stack their pixels in 8-bit vertical bytes. If LVGL sends an
unaligned 5-pixel update, the driver's memory math breaks, resulting in a
display full of grainy static.

* **The Fix:** `CONFIG_LV_Z_VDB_SIZE=100` and `CONFIG_LV_Z_FULL_REFRESH=y`
  forces LVGL to draw the entire 128x64 screen in RAM, and push a mathematically
  perfect, 100% aligned frame to the driver every time.

### 2. The Broken/Alien Text

LVGL defaults to the `Montserrat` font, which uses anti-aliasing (grey pixels)
to make curved letters look smooth. Because a 1-bit OLED only understands solid
white or solid black, it deletes the grey pixels entirely, resulting in
hollowed-out, unreadable letters.

* **The Fix:** We explicitly disable Montserrat and enable
  `CONFIG_LV_FONT_UNSCII_8=y`. This is a strict "pixel-art" font with zero
  smoothing, meaning the OLED can render it perfectly.

### 3. The Watchdog Boot Loop

Giving LVGL enough RAM to hold the entire screen requires allocating massive
memory pools. If you turn on `DEBUG` level logging to watch this memory
allocate, the slow serial console delays the boot process. The board's Hardware
Watchdog Timer assumes the system is frozen and forces a reboot before `main()`
can even run!

* **The Fix:** `CONFIG_LOG_DEFAULT_LEVEL=3` keeps the boot process
  lightning-fast, bypassing the Watchdog, while `CONFIG_MAIN_STACK_SIZE=4096`
  ensures the OS has enough breathing room to boot the display thread.

### 4. I2C Bus Lockups

If you restart the microcontroller while it is in the middle of talking to the
screen, the screen will hold the I2C data line hostage. When the board boots
back up, it thinks the display is dead.

* **The Fix:** `main.c` contains defensive `NULL` pointer checks. If the
  hardware is locked, it safely prints an error to the terminal instead of
  triggering a fatal CPU Hard Fault. (Fix physically by unplugging the USB for 5
  seconds).

## Building and Flashing

Because of the strict memory limits and font changes, a pristine build is always
recommended when altering LVGL Kconfigs.

1. **Build the application:**

```bash
west build -b frdm_mcxn236 -p pristine

```

2. **Flash to the target:**

```bash
west flash

```
