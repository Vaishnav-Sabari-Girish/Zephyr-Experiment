#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led[] = {
  GPIO_DT_SPEC_GET(LED0_NODE, gpios),
  GPIO_DT_SPEC_GET(LED1_NODE, gpios),
  GPIO_DT_SPEC_GET(LED2_NODE, gpios)
};

static int cmd_led_control(const struct shell *sh, size_t argc, char **argv) {
  int led_num = atoi(argv[1]);
  const char *state = argv[2];

  if (led_num < 1 || led_num > 3) {
    shell_error(sh, "Invalid LED number. Use only 1, 2 or 3");
    return -EINVAL;
  }

  int val = (strcmp(state, "on") == 0) ? 1 : 0;

  gpio_pin_set_dt(&led[led_num - 1], val);
  shell_print(sh, "LED %d is now %s", led_num, (val ? "ON" : "OFF"));

  return 0;
}

SHELL_CMD_ARG_REGISTER(led, NULL,
                       "Control LED's: led <1-3> <on|off>",
                       cmd_led_control, 3, 0);

int main(void)
{
  for (int i = 0; i < 3; i++) {
    if (!gpio_is_ready_dt(&led[i])) {
      return -1;
    }
    gpio_pin_configure_dt(&led[i], GPIO_OUTPUT_INACTIVE);
  }

  k_sleep(K_FOREVER);

  return 0;
}
