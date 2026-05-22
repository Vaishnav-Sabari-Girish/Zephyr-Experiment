#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE       DT_ALIAS(led0)
#define SW0_NODE        DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

static struct gpio_callback button_cb_data;
LOG_MODULE_REGISTER(interrupt_logger, LOG_LEVEL_DBG);

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  LOG_INF("Interrupt triggered! Toggling LED");
  gpio_pin_toggle_dt(&led);
}

int main(void)
{
  LOG_INF("Button Interrupt");

  if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
    LOG_ERR("Hardware not ready");
    return 0;
  }

  gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button, GPIO_INPUT);

  gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

  gpio_add_callback_dt(&button, &button_cb_data);

  LOG_INF("Interrupts configured. Main thread is now gonna sleep forever");

  k_sleep(K_FOREVER);

  return 0;
}
