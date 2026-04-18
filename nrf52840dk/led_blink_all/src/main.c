#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

int main(void)
{
  int ret1, ret2, ret3, ret4;

  printk("Blinky App\n\n");

  if (!gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3) || !gpio_is_ready_dt(&led4)) {
    printk("ERROR: LED not ready\n");
    return 0;
  }

  ret1 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
  ret3 = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
  ret4 = gpio_pin_configure_dt(&led4, GPIO_OUTPUT_INACTIVE);

  if ((ret1 < 0) || (ret2 < 0) || (ret3 < 0) || (ret4 < 0)) {
    printk("ERROR: Failed to configure LED pin \n");
    return 0;
  }

  while (1) {
    ret1 = gpio_pin_toggle_dt(&led1);
    ret2 = gpio_pin_toggle_dt(&led2);
    ret3 = gpio_pin_toggle_dt(&led3);
    ret4 = gpio_pin_toggle_dt(&led4);

  if ((ret1 < 0) || (ret2 < 0) || (ret3 < 0) || (ret4 < 0)) {
      printk("ERROR: Failed to toggle LED pin\n");
      return 0;
    }

    k_msleep(1000);
  }

  return 0;
}
