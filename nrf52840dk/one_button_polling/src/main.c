#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SW0_NODE      DT_ALIAS(sw0)
#define LED0_NODE     DT_ALIAS(led0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
  printk("Button Polling");

  if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
    printk("ERROR: Hardware not ready\n");
    return 0;
  }

  gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

  gpio_pin_configure_dt(&button, GPIO_INPUT);

  while (1) {
    int button_state = gpio_pin_get_dt(&button);

    if (button_state > 0) {
      gpio_pin_set_dt(&led, 1);
    } else {
      gpio_pin_set_dt(&led, 0);
    }

    k_msleep(10);
  }

  return 0;
}
