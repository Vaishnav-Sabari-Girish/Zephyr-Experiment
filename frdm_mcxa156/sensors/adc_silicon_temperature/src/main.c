#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>


LOG_MODULE_REGISTER(silicon_logger, LOG_LEVEL_DBG);

int main(void)
{
  const struct device *const temp_dev = DEVICE_DT_GET_ANY(nxp_lpadc_temp40);

  if (temp_dev == NULL) {
    LOG_ERR("No nxp,lpadc-temp40 device found in devicetree");
    return 0;
  }

  if (!device_is_ready(temp_dev)) {
    LOG_ERR("Device is not ready %s", temp_dev -> name);
    return 0;
  }

  LOG_INF("Device %s initialized successfully", temp_dev -> name);

  while (1) {
    struct sensor_value temp_val;

    if (sensor_sample_fetch(temp_dev) < 0) {
      LOG_ERR("Failed to fetch sensor data");
      k_msleep(1000);
      continue;
    }

    if (sensor_channel_get(temp_dev, SENSOR_CHAN_DIE_TEMP, &temp_val) < 0) {
      LOG_ERR("Failed to get sensor channel");
      continue;
    }

    LOG_INF("Silicon Die Temperature: %d.%06d C", temp_val.val1, temp_val.val2);

    k_msleep(1000);
  }

  return 0;
}
