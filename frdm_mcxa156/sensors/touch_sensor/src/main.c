#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(touch_logger, LOG_LEVEL_DBG);

/* LED: red_led on gpio3 pin 12, active low */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* Touch sensor: gpio1 pin 4, active high */
static const struct gpio_dt_spec touch = GPIO_DT_SPEC_GET(DT_ALIAS(touch0), gpios);

static struct gpio_callback touch_cb_data;

void touched(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("Touched. Toggling LED");
	gpio_pin_toggle_dt(&led);
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED device not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED (err %d)", ret);
		return 0;
	}

	if (!gpio_is_ready_dt(&touch)) {
		LOG_ERR("Touch sensor device not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&touch, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure touch sensor (err %d)", ret);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&touch, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure touch interrupt (err %d)", ret);
		return 0;
	}

	gpio_init_callback(&touch_cb_data, touched, BIT(touch.pin));
	ret = gpio_add_callback(touch.port, &touch_cb_data);
	if (ret < 0) {
		LOG_ERR("Failed to add touch callback (err %d)", ret);
		return 0;
	}

	LOG_INF("Touch sensor ready on %s pin %d", touch.port->name, touch.pin);
	LOG_INF("Touch the sensor to toggle the LED");

	return 0;
}

