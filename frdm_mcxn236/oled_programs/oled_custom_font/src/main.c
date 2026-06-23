#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include "display_hal.h"

LOG_MODULE_REGISTER(die_oled_logger, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("Starting the Die Temperature and OLED application");

  if (display_hal_init() != 0) {
    LOG_ERR("Display Initialization failed.");
    return -1;
  }

  const struct device *const die_temp = DEVICE_DT_GET(DT_ALIAS(die_temp0));

  if (die_temp == NULL) {
    LOG_ERR("No device nxp,lpadc-temp40 device found in devictree");
  }

  if (!device_is_ready(die_temp)) {
    LOG_ERR("Die Temperature sensor is not ready");
    return -1;
  }

  LOG_INF("Sensor %s initialized successfully", die_temp -> name);

  char display_buffer[32];

  while (1) {
    struct sensor_value silicon_temperature;

    if (sensor_sample_fetch(die_temp) < 0) {
      LOG_ERR("Failed to fetch sensor data");
      k_msleep(1000);
      continue;
    }

    if (sensor_channel_get(die_temp, SENSOR_CHAN_DIE_TEMP, &silicon_temperature) < 0) {
      LOG_ERR("Failed to get sensor channel");
      k_msleep(1000);
      continue;
    }

    double temp_c = sensor_value_to_double(&silicon_temperature);

    LOG_INF("Silicon Die temperature: %.2f C", temp_c);

    display_hal_clear();

    /* Title */
    display_hal_set_font_size(15, 24);
    display_hal_print("DIE TEMP", 0, 0);

    /* Actual Value */
    display_hal_set_font_size(9, 16);

    /* Format into string and print the output in display */
    snprintf(display_buffer, sizeof(display_buffer), "Temp: %.2f C", temp_c);
    display_hal_print(display_buffer, 0, 30);

    display_hal_finalize();

    k_msleep(1000);
  }

  return 0;
}
