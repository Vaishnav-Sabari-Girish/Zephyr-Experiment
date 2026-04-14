#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

// Get the device tree node identifier for led0
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
  int ret;

  printk("Blinky app\n\n");

  if (!gpio_is_ready_dt(&led)) {
    printk("ERROR: LED device %s is not ready\n", led.port->name);
    return 0;
  }

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    printk("ERROR: Failed to configure LED pin\n");
    return 0;
  }

  while (1) {
    ret = gpio_pin_toggle_dt(&led);

    if (ret < 0) {
      printk("ERROR: Failed to toggle LED pin\n");
      return 0;
    }

    k_msleep(1000);
  }

  return 0;
}
