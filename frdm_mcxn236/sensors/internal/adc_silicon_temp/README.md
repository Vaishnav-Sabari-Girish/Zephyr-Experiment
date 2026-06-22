# Internal Silicon Die Temperature — FRDM-MCXN236

Utilizes the built-in `nxp,lpadc-temp40` driver to sample the SoC's internal die
temperature. Maps the physical hardware channel to a software command buffer to
comply with Zephyr's modern NXP ADC architecture.

[Code File](./src/main.c)

[Overlay File](./boards/frdm_mcxn236.overlay)

## Output format (using Zephyr Logging)

```text
*** Booting Zephyr OS build v4.4.0-313-g4e7fcbb2d84d ***
[00:00:00.002,000] <inf> die_logger: Device temp0 is initialized successfully
[00:00:01.003,000] <inf> die_logger: Silicon Die Temperature: 32.404785 C
[00:00:02.004,000] <inf> die_logger: Silicon Die Temperature: 32.411020 C
[00:00:03.005,000] <inf> die_logger: Silicon Die Temperature: 32.409550 C

```

The device initializes on boot and the main loop fetches and prints the sensor
data at a 1 Hz interval.

---

## Devicetree Architecture (Command Buffers vs Hardware Channels)

In modern Zephyr versions (3.4+), NXP MCUX LPADC drivers no longer map `reg`
directly to the hardware multiplexer channel. This implementation
**replaces direct channel mapping** with a software command buffer approach.

### Key overlay properties

| Property | Why it is required |
| -------- | ------------------ |
| `reg = <0>` | Targets software command buffer 0 instead of the physical channel |
| `zephyr,input-positive` | Explicitly routes the 'A' side of the hardware multiplexer (`MCUX_LPADC_CH26A`)|
| `zephyr,vref-mv` | Provides the baseline voltage (3300mV) needed for the driver's Celsius conversion math |
| `zephyr,oversampling` | Crushes electrical noise from the high-impedance internal diode |
| `zephyr,acquisition-time` | Lengthens the capacitor charge time (131 ticks) required for internal sensors |

### Overlay Configuration

```devicetree
/* ── Include NXP specific bindings ── */
#include <zephyr/dt-bindings/adc/adc.h>
#include <zephyr/dt-bindings/adc/mcux-lpadc.h>

/ {
	aliases {
		die-temp0 = &temp0;
	};
};

```

**LPADC Command Buffer Setup:**

```devicetree
&lpadc0 {
	status = "okay";
	#address-cells = <1>;
	#size-cells = <0>;

	channel@0 {
		reg = <0>;
		zephyr,gain = "ADC_GAIN_1";
		zephyr,vref-mv = <3300>; 
		zephyr,reference = "ADC_REF_EXTERNAL0";
		zephyr,acquisition-time = <ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131)>;
		zephyr,oversampling = <4>; 
		zephyr,input-positive = <MCUX_LPADC_CH26A>;
		zephyr,resolution = <16>;
	};
};

```

**Sensor Target:**

```devicetree
&temp0 {
	status = "okay";
	io-channels = <&lpadc0 0>;
};

```

---

## Data flow

```text
┌─────────────────┐   Hardware   ┌───────────────┐   Zephyr API   ┌────────────┐
│ Internal Diode  │─────────────►│ LPADC0        │───────────────►│ LOG_INF()  │
│ (Channel 26A)   │  multiplex   │ (Cmd Buffer 0)│  sensor_value  │ serial     │
└─────────────────┘              └──────┬────────┘                └────────────┘
                                        │
                             ┌──────────┴──────────┐
                             │ VREF Math (3300mV)  │
                             │ Oversampling (x4)   │
                             └─────────────────────┘

```

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| Error `-22` (`-EINVAL`) on boot | The driver rejected a parameter. Ensure `zephyr,input-positive = <MCUX_LPADC_CH26A>` is used and `reg` is set to `<0>`, not `<26>`. |
| `channel_26_P_zephyr_gain... undeclared` | Check that your overlay explicitly defines `zephyr,gain = "ADC_GAIN_1";` inside the `channel@0` subnode. |
| Temperatures reading `2933 C` or `-297 C` | The driver math is underflowing/overflowing. Add `zephyr,vref-mv = <3300>;` to the channel config. |
| Values swing wildly every second | Internal diodes have high source impedance. Add `zephyr,oversampling = <4>;` to smooth out the noise. |

## Wiring

| Sensor | FRDM-MCXN236 |
| --- | --- |
| N/A | **Internal to SoC** |

## Build & Flash

```bash
# Clean build directory
west build -p always -b frdm_mcxn236

# Flash to board
west flash

```
