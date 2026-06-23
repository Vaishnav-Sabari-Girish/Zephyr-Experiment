#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "display_hal.h"

LOG_MODULE_REGISTER(display_main, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("OLED application for FRDM-MCXN236");

  if (display_hal_init() != 0) {
    LOG_ERR("Failed to initialize Display HAL");
    return -1;
  }

  LOG_INF("Drawing to screen");

  display_hal_print("Hello World", 10, 20);

  while (1) {
    k_sleep(K_SECONDS(1));
  }

  return 0;
}
