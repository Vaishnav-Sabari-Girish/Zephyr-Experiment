#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios); 

LOG_MODULE_REGISTER(led_logger, LOG_LEVEL_INF);

int main(void)
{
  int ret = 0;

  if (!gpio_is_ready_dt(&led)) {
    LOG_ERR("Hardware not ready");
    return 0;
  }

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

  if (ret < 0) {
    LOG_ERR("Failed to configure LED pin");
    return 0;
  }

  while (1) {
    ret = gpio_pin_toggle_dt(&led);

    if (ret < 0) {
      LOG_ERR("Failed to toggle LED pin");
      return 0;
    }

    k_msleep(1000);
  }

  return 0;
}
