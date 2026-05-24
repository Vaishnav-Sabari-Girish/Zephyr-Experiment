/*
 * PWM LED Fading – FRDM-MCXA156 (NXP MCX-A156, Cortex-M33)
 *
 * Smoothly fades the on-board Red LED in and out using the FlexPWM
 * peripheral via Zephyr's generic PWM API.
 *
 * Build:
 *   west build -b frdm_mcxa156 .
 * Flash:
 *   west flash
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <stdint.h>

LOG_MODULE_REGISTER(pwm_led_fade, LOG_LEVEL_INF);

/* ------------------------------------------------------------------
 * Devicetree binding – resolve the pwm-led0 alias added in the overlay
 * ------------------------------------------------------------------ */
static const struct pwm_dt_spec led =
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

/* ------------------------------------------------------------------
 * Fade parameters
 * ------------------------------------------------------------------ */
#define FADE_STEPS      100          /* brightness levels per direction  */
#define STEP_DELAY_MS   10           /* ms per step  → 1 s full sweep    */
#define PAUSE_MS        200          /* pause at min/max brightness       */

int main(void)
{
	if (!pwm_is_ready_dt(&led)) {
		LOG_ERR("PWM device %s is not ready", led.dev->name);
		return -ENODEV;
	}

	LOG_INF("PWM LED fade started on %s (channel %u, period %u ns)",
		led.dev->name, led.channel, led.period);

	uint32_t period_ns = led.period;
	uint32_t pulse_ns;
	int step;

	while (1) {
		/* Fade IN: 0 % → 100 % duty cycle */
		for (step = 0; step <= FADE_STEPS; step++) {
			/*
			 * Quadratic gamma curve: perceived brightness is
			 * roughly proportional to duty^2, so a linear ramp
			 * in duty² space looks perceptually smooth.
			 *
			 *   pulse = period × (step / FADE_STEPS)²
			 */
			uint64_t tmp = (uint64_t)period_ns * step * step;
			pulse_ns = (uint32_t)(tmp / ((uint64_t)FADE_STEPS * FADE_STEPS));

			int ret = pwm_set_dt(&led, period_ns, pulse_ns);
			if (ret < 0) {
				LOG_ERR("pwm_set_dt failed: %d", ret);
				return ret;
			}
			k_msleep(STEP_DELAY_MS);
		}

		k_msleep(PAUSE_MS);

		/* Fade OUT: 100 % → 0 % duty cycle */
		for (step = FADE_STEPS; step >= 0; step--) {
			uint64_t tmp = (uint64_t)period_ns * step * step;
			pulse_ns = (uint32_t)(tmp / ((uint64_t)FADE_STEPS * FADE_STEPS));

			int ret = pwm_set_dt(&led, period_ns, pulse_ns);
			if (ret < 0) {
				LOG_ERR("pwm_set_dt failed: %d", ret);
				return ret;
			}
			k_msleep(STEP_DELAY_MS);
		}

		k_msleep(PAUSE_MS);
	}

	return 0;
}
