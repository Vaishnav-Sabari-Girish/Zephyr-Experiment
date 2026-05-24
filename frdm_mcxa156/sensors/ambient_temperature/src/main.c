#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(sensor_logger, LOG_LEVEL_DBG);
int main(void)
{
  const struct device *const temp_dev = DEVICE_DT_GET_ANY(nxp_p3t1755);

  if (temp_dev == NULL) {
    LOG_ERR("No device found for nxp,p3t1755");
    return 0;
  }

  if (!device_is_ready(temp_dev)) {
    LOG_ERR("Device is not ready");
    return 0;
  }

  LOG_INF("Successfully initialized %s", temp_dev -> name);

  while (1) {
    struct sensor_value temp_val;

    if (sensor_sample_fetch(temp_dev) < 0) {
      LOG_ERR("Failed to fetch sensor data");
      continue;
    }

    if (sensor_channel_get(temp_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val) < 0) {
      LOG_ERR("Failed to get sensor channel");
      continue;
    }

    /*
     * Zephyr sensor_value uses 2 integers 
     * val1: integer part 
     * val2: Fractional part (in milliseconds)
    */ 

    LOG_INF("Ambient Temperature: %d.%06d C", temp_val.val1, temp_val.val2);

    k_msleep(1000);
  }

  return 0;
}
