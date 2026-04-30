#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Command handler for turning LED ON
static int cmd_led_on(const struct shell *sh, size_t argc, char **argv) {
  gpio_pin_set_dt(&led, 1);

  shell_print(sh, "LED is now ON");

  return 0;
}

// Command handler for turning LED ON
static int cmd_led_off(const struct shell *sh, size_t argc, char **argv) {
  gpio_pin_set_dt(&led, 0);

  shell_print(sh, "LED is now OFF");

  return 0;
}

// Create subcommand set for led
// This creates groups "on" and "off" under parent command "led"
SHELL_STATIC_SUBCMD_SET_CREATE(sub_led,
                               SHELL_CMD(on, NULL, "Turn the onboard LED on", cmd_led_on),
                               SHELL_CMD(off, NULL, "Turn the onboard LED off", cmd_led_off),
                               SHELL_SUBCMD_SET_END
);

// Register the root command
SHELL_CMD_REGISTER(led, &sub_led, "Control the onboard LED", NULL);

int main(void)
{
  if (!gpio_is_ready_dt(&led)) {
    return -1;
  }

  gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

  k_sleep(K_FOREVER);

  return 0;
}
