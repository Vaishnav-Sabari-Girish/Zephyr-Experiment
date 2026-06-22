#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(die_logger, LOG_LEVEL_INF);

int main(void)
{
  const struct device *const die_temp = DEVICE_DT_GET(DT_ALIAS(die_temp0));

  if (die_temp == NULL) {
    LOG_ERR("No nxp,lpadc-temp40 device found in devicetree");
    return -1;
  }

  if (!device_is_ready(die_temp)) {
    LOG_ERR("Device is not ready");
    return -1;
  }

  LOG_INF("Device %s is initialized successfully", die_temp -> name);

  while (1) {
    struct sensor_value silicon_temperature;

    if (sensor_sample_fetch(die_temp) < 0) {
      LOG_ERR("Failed to fetch sensor data");
      k_msleep(1000);
      continue;
    }

    if (sensor_channel_get(die_temp, SENSOR_CHAN_DIE_TEMP, &silicon_temperature) < 0) {
      LOG_ERR("Failed to get sensor channel");
      return -1;
    }

    double temp_c = sensor_value_to_double(&silicon_temperature);

    LOG_INF("Silicon Die Temperature: %.2f", temp_c);

    k_msleep(1000);
  }

  return 0;
}
