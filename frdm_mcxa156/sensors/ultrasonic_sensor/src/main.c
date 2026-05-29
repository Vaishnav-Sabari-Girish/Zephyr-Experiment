#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>

LOG_MODULE_REGISTER(ultrasonic_logger, LOG_LEVEL_DBG);

int main(void)
{
  const struct device *const hcsr04 = DEVICE_DT_GET(DT_ALIAS(sonic0)); 

  if (!device_is_ready(hcsr04)) {
    LOG_ERR("Device %s is not ready", hcsr04 -> name);
    return 0;
  }

  LOG_INF("Device %s is ready", hcsr04 -> name);

  struct sensor_value distance;

  while (1) {
    k_msleep(500);

    int ret = sensor_sample_fetch(hcsr04);

    if (ret == 0) {
      sensor_channel_get(hcsr04, SENSOR_CHAN_DISTANCE, &distance);

      double dist_meters = sensor_value_to_double(&distance);
      double dist_cm = dist_meters * 100.0;

      LOG_INF("Distance: %.3f m (%.1f cm): ", dist_meters, dist_cm);
    } else {
      LOG_ERR("Failed to fetch data from sensor. Error code: %d", ret);
    }
  }

  return 0;
}
