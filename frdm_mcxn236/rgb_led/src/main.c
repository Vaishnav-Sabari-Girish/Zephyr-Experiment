#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define LED0_NODE  DT_ALIAS(led0)
#define LED1_NODE  DT_ALIAS(led1)
#define LED2_NODE  DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

LOG_MODULE_REGISTER(rgb_logger, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("RGB blinky program");

  int ret1 = 0, ret2 = 0, ret3 = 0;

  if (!gpio_is_ready_dt(&led0) ||
      !gpio_is_ready_dt(&led1) ||
      !gpio_is_ready_dt(&led2)
     ) 
  {
    LOG_ERR("LED's are not ready");
    return -1;
  }

  ret1 = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
  ret2 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  ret3 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

  if ((ret1 < 0) || (ret2 < 0) || (ret3 < 0)) {
    LOG_ERR("Failed to configure LED pin");
    return -1;
  }

  while (1) {
    LOG_INF("Toggling LED's");

    LOG_INF("LED 0 toggled");
    ret1 = gpio_pin_set_dt(&led0, 1);
    if (ret1 < 0) {
      LOG_ERR("Failed to set LED 0 high");
      return -1;
    }

    k_msleep(1000);
    ret1 = gpio_pin_set_dt(&led0, 0);

    ret2 = gpio_pin_set_dt(&led1, 1);
    if (ret2 < 0) {
      LOG_ERR("Failed to set LED 1 high");
      return -1;
    }

    k_msleep(1000);
    ret2 = gpio_pin_set_dt(&led1, 0);

    ret3 = gpio_pin_set_dt(&led2, 1);
    if (ret3 < 0) {
      LOG_ERR("Failed to set LED 2 high");
      return -1;
    }

    k_msleep(1000);
    ret3 = gpio_pin_set_dt(&led2, 0);
  }

  return 0;
}
