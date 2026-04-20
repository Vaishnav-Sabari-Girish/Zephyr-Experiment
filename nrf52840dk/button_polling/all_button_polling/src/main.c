#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SW0_NODE      DT_ALIAS(sw0)
#define LED0_NODE     DT_ALIAS(led0)
#define SW1_NODE      DT_ALIAS(sw1)
#define LED1_NODE     DT_ALIAS(led1)
#define SW2_NODE      DT_ALIAS(sw2)
#define LED2_NODE     DT_ALIAS(led2)
#define SW3_NODE      DT_ALIAS(sw3)
#define LED3_NODE     DT_ALIAS(led3)

static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios, {0});
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios, {0});
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET_OR(SW3_NODE, gpios, {0});
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

int main(void)
{
  printk("Button Polling");

  if (
    !gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&button1) ||
    !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&button2) ||
    !gpio_is_ready_dt(&led3) || !gpio_is_ready_dt(&button3) ||
    !gpio_is_ready_dt(&led4) || !gpio_is_ready_dt(&button4)
  ) {
    printk("ERROR: Hardware not ready\n");
    return 0;
  }

  gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&led4, GPIO_OUTPUT_ACTIVE);

  gpio_pin_configure_dt(&button1, GPIO_INPUT);
  gpio_pin_configure_dt(&button2, GPIO_INPUT);
  gpio_pin_configure_dt(&button3, GPIO_INPUT);
  gpio_pin_configure_dt(&button4, GPIO_INPUT);

  while (1) {
    int button1_state = gpio_pin_get_dt(&button1);
    int button2_state = gpio_pin_get_dt(&button2);
    int button3_state = gpio_pin_get_dt(&button3);
    int button4_state = gpio_pin_get_dt(&button4);

    if (button1_state > 0) {
      gpio_pin_set_dt(&led1, 1);
    } else {
      gpio_pin_set_dt(&led1, 0);
    }

    if (button2_state > 0) {
      gpio_pin_set_dt(&led2, 1);
    } else {
      gpio_pin_set_dt(&led2, 0);
    }
    if (button3_state > 0) {
      gpio_pin_set_dt(&led3, 1);
    } else {
      gpio_pin_set_dt(&led3, 0);
    }
    if (button4_state > 0) {
      gpio_pin_set_dt(&led4, 1);
    } else {
      gpio_pin_set_dt(&led4, 0);
    }

    k_msleep(10);
  }

  return 0;
}
