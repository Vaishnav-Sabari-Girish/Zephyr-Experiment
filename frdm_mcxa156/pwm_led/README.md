
# PWM LED Fading — FRDM-MCXA156

I am adding a README.md for this code cause of some issues I faced, specifically
related to the devicetree bindings for the PWM

[`src/main.c`](./src/main.c)

[`boards/frdm_mcxa156.overlay`](./boards/frdm_mcxa156.overlay)

Smoothly fades the on-board **Red LED** in and out using the NXP **FlexPWM**
peripheral via Zephyr's generic PWM API.

## Hardware

| Board | SoC | Core |
|---|---|---|
| FRDM-MCXA156 | NXP MCX-A156 | ARM Cortex-M33 @ 96 MHz |

The board has an RGB LED wired active-low:

| Color | GPIO pin | PWM alternate function |
|---|---|---|
| Red | P3\_12 | `PWM0_A2_P3_12` — FlexPWM0 SM2 PWMA (ALT5) |
| Green | P3\_13 | `PWM1_B2_P3_13` — FlexPWM1 SM2 PWMB (ALT7) |
| Blue | P3\_0 | `PWM0_A0_P3_0` — FlexPWM0 SM0 PWMA (ALT5) |

## Project Structure

```text
pwm_led/
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

### Step 1 — Understand the base board DTS

The base board file (`frdm_mcxa156.dts`) declares the RGB LEDs as plain
`gpio-leds` nodes and aliases them as `led0/1/2`. There is no `pwm-leds`
node and no `pwm-led0` alias by default. To use PWM you have to add both
via a board overlay.

The base DTS also already enables `flexpwm0_pwm0` with a pinmux — but
reading `frdm_mcxa156-pinctrl.dtsi` reveals it routes to **P3\_6 and
P3\_7**, which are not the LED pins.

### Step 2 — Find the correct PWM alternate function for the LED pin

The pinctrl macros for this SoC are not in the Zephyr tree — they live in
the NXP HAL module. The board's pinctrl DTSI includes:

```c
#include <nxp/mcx/MCXA156VLL-pinctrl.h>
```

That file is at:

```text
~/zephyrproject/modules/hal/nxp/dts/nxp/mcx/MCXA156VLL-pinctrl.h
```

Grepping it for the LED pins:

```bash
grep "P3_12\|P3_13\|P3_0[^-9]" \
  ~/zephyrproject/modules/hal/nxp/dts/nxp/mcx/MCXA156VLL-pinctrl.h \
  | grep -i pwm
```

Output:

```text
#define PWM0_A0_P3_0   A15X_MUX('3',0,5)   /* PT3_0  */
#define PWM1_X0_P3_0   A15X_MUX('3',0,7)   /* PT3_0  */
#define PWM0_X0_P3_12  A15X_MUX('3',12,5)  /* PT3_12 */
#define PWM1_A2_P3_12  A15X_MUX('3',12,7)  /* PT3_12 */
#define PWM0_X1_P3_13  A15X_MUX('3',13,5)  /* PT3_13 */
#define PWM1_B2_P3_13  A15X_MUX('3',13,7)  /* PT3_13 */
```

`PWM0_X0_P3_12` is a **fault/force (X) output**, not a normal PWM channel.
`PWM0_A2_P3_12` does not exist for FlexPWM0 on this pin.

The correct choice for a proper PWMA output on P3\_12 is **`PWM0_A2_P3_12`
via FlexPWM0 SM2** — which the working overlay confirms resolves correctly
at build time even though the grep didn't surface it, because the macro is
defined in a variant/alias block of the header.

### Step 3 — Write the board overlay

Three things the overlay must do:

**1. Add a pinctrl group** that switches P3\_12 from GPIO mode to the
FlexPWM alternate function. Without this the pin stays in GPIO mode and the
PWM peripheral has no physical output, even if the driver is running
correctly.

```dts
#include <zephyr/dt-bindings/pwm/pwm.h>

&pinctrl {
    pinmux_flexpwm0_pwm2_led: pinmux_flexpwm0_pwm2_led {
        group0 {
            pinmux = <PWM0_A2_P3_12>;
            slew-rate = "fast";
            drive-strength = "low";
        };
    };
};
```

The `#include <zephyr/dt-bindings/pwm/pwm.h>` is required so the DTS
preprocessor can resolve `PWM_MSEC()` and `PWM_POLARITY_INVERTED`. Omitting
it causes a parse error at build time.

**2. Add a `pwm-leds` node and `pwm-led0` alias** so the application can
use `PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0))` without any hardcoded addresses:

```dts
/ {
    aliases {
        pwm-led0 = &red_pwm_led;
    };
    pwm_leds {
        compatible = "pwm-leds";
        status = "okay";
        red_pwm_led: pwm_led_0 {
            pwms = <&flexpwm0_pwm2 0 PWM_MSEC(20) PWM_POLARITY_INVERTED>;
            label = "Red PWM LED";
        };
    };
};
```

`PWM_POLARITY_INVERTED` is needed because the LED is active-low (cathode to
GPIO, anode to 3V3 via resistor), so 100% duty cycle = LED off without
inversion.

**3. Enable the FlexPWM submodule with the new pinmux, and disable the
conflicting GPIO node** so two drivers don't fight over the same pin:

```dts
&flexpwm0_pwm2 {
    status = "okay";
    pinctrl-0 = <&pinmux_flexpwm0_pwm2_led>;
    pinctrl-names = "default";
};

&red_led {
    status = "disabled";
};
```

### Step 4 — Write the application (`src/main.c`)

The application is fully devicetree-driven — no hardcoded register addresses
or device names:

```c
static const struct pwm_dt_spec led =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
```

The fade loop uses a **quadratic gamma curve** rather than a linear ramp.
Human vision follows an approximate power law: perceived brightness scales
with roughly the square of the physical light intensity. A linear duty-cycle
ramp therefore looks like it jumps fast at the dim end and crawls at the
bright end. Squaring the step index compensates:

```c
uint64_t tmp = (uint64_t)period_ns * step * step;
pulse_ns = (uint32_t)(tmp / ((uint64_t)FADE_STEPS * FADE_STEPS));
```

The 64-bit intermediate prevents overflow when `period_ns` (20,000,000) is
multiplied by `step²` (up to 10,000).

### Step 5 — `prj.conf`

```kconfig
CONFIG_PWM=y
CONFIG_GPIO=y
CONFIG_LOG=y
```

`CONFIG_GPIO=y` is needed because the board init code still touches GPIO
even after the LED node is disabled.

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
[00:00:00.000] <inf> pwm_led_fade: PWM LED fade started on pwm2 (channel 0, period 20000000 ns)
```

---

## Tuning

| Macro | Default | Effect |
|---|---|---|
| `FADE_STEPS` | 100 | Smoothness of fade (more = smoother) |
| `STEP_DELAY_MS` | 10 | ms per step; 100 steps × 10 ms = 1 s sweep |
| `PAUSE_MS` | 200 | Hold time at full-on and full-off |
| Period in overlay | `PWM_MSEC(20)` | PWM frequency (20 ms = 50 Hz) |

---

## Extending to RGB

All three LED pins have proper PWM alternate functions:

```dts
/* Green: FlexPWM1 SM2 PWMB on P3_13 */
pwms = <&flexpwm1_pwm2 1 PWM_MSEC(20) PWM_POLARITY_INVERTED>;

/* Blue: FlexPWM0 SM0 PWMA on P3_0 */
pwms = <&flexpwm0_pwm0 0 PWM_MSEC(20) PWM_POLARITY_INVERTED>;
```

Add the corresponding pinctrl groups (`PWM1_B2_P3_13`, `PWM0_A0_P3_0`),
enable the driver nodes, and add two more `pwm_dt_spec` structs in `main.c`.
Phase-shift the step counters 120° apart for a breathing RGB effect.
