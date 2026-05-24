# ADC Silicon Die Temperature — FRDM-MCXA156

Reads the on-chip **silicon die temperature** using the NXP **LPADC**
peripheral and Zephyr's generic sensor API.

[`src/main.c`](./src/main.c)

[`boards/frdm_mcxa156.overlay`](./boards/frdm_mcxa156.overlay)

## Hardware

| Board | SoC | Core |
|---|---|---|
| FRDM-MCXA156 | NXP MCX-A156 | ARM Cortex-M33 @ 96 MHz |

The MCXA156 has an internal temperature sensor (TEMP40) wired to LPADC0
via a dedicated internal mux channel — no external pin or component needed.

## Project Structure

```text
adc_silicon_temperature/
├── CMakeLists.txt
├── prj.conf
├── Justfile
├── src/
│   └── main.c
└── boards/
    └── frdm_mcxa156.overlay
```

---

## How It Was Built — Step by Step

### Step 1 — Understand the driver and binding

The temperature sensor is exposed via the `nxp,lpadc-temp40` compatible.
Its binding (`dts/bindings/sensor/nxp,lpadc-temp40.yaml`) requires one
mandatory property:

```yaml
properties:
  io-channels:
    required: true
    description: This should point to an ADC channel (e.g., <&lpadc0 0>).
      That will be used for reading from the internal temperature sensor.
```

The driver source (`drivers/sensor/nxp/nxp_lpadc_temp40/lpadc_temp40.c`)
constructs its ADC channel config using:

```c
.ch_cfg = ADC_CHANNEL_CFG_DT(
    DT_CHILD(DT_INST_IO_CHANNELS_CTLR(inst),
             UTIL_CAT(channel_, DT_INST_IO_CHANNELS_INPUT(inst)))),
```

`DT_INST_IO_CHANNELS_INPUT` extracts the **specifier cell** from
`io-channels` and `UTIL_CAT` appends it to `channel_` to form the child
node name. This means:

- `io-channels = <&lpadc0 0>` → driver looks for `channel_0` → matches
  `channel@0`
- `io-channels = <&lpadc0 0x1a>` → driver looks for `channel_0x1a` →
  **no match**
- `io-channels = <&lpadc0 26>` → driver looks for `channel_26` → **no match**

The specifier cell must be a **plain decimal integer** whose stringification
matches the `channel@N` node name exactly.

### Step 2 — Find the correct ADC channel for the temperature sensor

The DTSI (`dts/arm/nxp/mcx/nxp_mcxa156.dtsi`) defines `temp0` at line 554
but leaves `io-channels` absent — the overlay must supply it. The internal
temperature sensor is routed through LPADC0's hardware mux input
`MCUX_LPADC_CH26A` (channel 26A). The Zephyr sample for this board at:

```text
samples/sensor/die_temp_polling/boards/frdm_mcxa156.overlay
```

confirms the correct setup: a logical `channel@0` node with
`zephyr,input-positive = <MCUX_LPADC_CH26A>` routes LPADC0 to the temp
sensor. The `channel@0` / `reg = <0>` is a **logical slot index** — the
hardware mux routing is done by `zephyr,input-positive`, not by `reg`.

### Step 3 — The overlay pitfalls

Several approaches that look correct but fail at compile time:

| Attempt | Result |
|---|---|
| `channel@1a` + `io-channels = <&lpadc0 0x1a>` | `channel_0x1a` not found |
| `channel@1a` + `io-channels = <&lpadc0 26>` | `channel_26` not found |
| `channel@0` + `io-channels = <&lpadc0 0>` | ✅ `channel_0` found |

The correct overlay:

```dts
#include <zephyr/dt-bindings/adc/adc.h>
#include <zephyr/dt-bindings/adc/mcux-lpadc.h>

&lpadc0 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;

    channel@0 {
        reg = <0>;
        zephyr,gain = "ADC_GAIN_1";
        zephyr,reference = "ADC_REF_EXTERNAL0";
        zephyr,acquisition-time = <ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131)>;
        zephyr,input-positive = <MCUX_LPADC_CH26A>;
    };
};

&temp0 {
    status = "okay";
    io-channels = <&lpadc0 0>;
};
```

Both `#include` headers are required:

- `adc.h` — provides `ADC_ACQ_TIME` and `ADC_ACQ_TIME_TICKS`
- `mcux-lpadc.h` — provides `MCUX_LPADC_CH26A`

The acquisition time of **131 ticks** is the value NXP specifies for the
TEMP40 sensor to settle correctly. Using `ADC_ACQ_TIME_DEFAULT` will not
give accurate readings.

### Step 4 — The application (`src/main.c`)

The app is fully devicetree-driven using `DEVICE_DT_GET_ANY`:

```c
const struct device *const temp_dev = DEVICE_DT_GET_ANY(nxp_lpadc_temp40);
```

The read loop fetches and prints temperature once per second:

```c
sensor_sample_fetch(temp_dev);
sensor_channel_get(temp_dev, SENSOR_CHAN_DIE_TEMP, &temp_val);
LOG_INF("Silicon Die Temperature: %d.%06d C", temp_val.val1, temp_val.val2);
k_msleep(1000);
```

`sensor_value` stores the reading split into integer and fractional parts
(`val1` and `val2`), giving microsecond-precision formatting with `%d.%06d`.

### Step 5 — `prj.conf`

```kconfig
CONFIG_ADC=y
CONFIG_SENSOR=y
CONFIG_LOG=y
CONFIG_GPIO=y
```

---

## Build and Flash

```bash
west build -b frdm_mcxa156 .
west flash
```

Default runner is **LinkServer** (on-board MCU-Link CMSIS-DAP). To use
J-Link instead:

```bash
west flash -r jlink
```

Console output on LPUART0 (J21, 115200 8N1):

```text
*** Booting Zephyr OS build v4.4.0-... ***
[00:00:00.001,000] <inf> silicon_logger: Device temp0 initialized successfully
[00:00:00.064,000] <inf> silicon_logger: Silicon Die Temperature: 34.500000 C
[00:00:01.064,000] <inf> silicon_logger: Silicon Die Temperature: 34.625000 C
```
