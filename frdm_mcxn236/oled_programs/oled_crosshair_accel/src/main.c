#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include "display_hal.h"

LOG_MODULE_REGISTER(display_main, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("Starting IMU bubble Level");

  if (display_hal_init() != 0) {
    LOG_ERR("Display init failed");
    return -1;
  }

  const struct device *const accel_dev = DEVICE_DT_GET(DT_ALIAS(accel0));

  if (!device_is_ready(accel_dev)) {
    LOG_ERR("Device is not ready");
    return -1;
  }

  while (1) {
    struct sensor_value accel[3];

    if (sensor_sample_fetch(accel_dev) < 0) {
      LOG_ERR("Failed to fetch IMU data");
      k_msleep(1000);
      continue;
    }

    sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_XYZ, accel);

    double ax = sensor_value_to_double(&accel[0]);
    double ay = sensor_value_to_double(&accel[1]);

    /*
     * MAPPING MATH
     * Screen is 128x64. Center is (64, 32)
     * Max tilt is -9.8 m/s^2
     * X-Axis: 64 pixels from center to edge. (64 / 9.8 = ~6.5 multiplier)
     * Y-Axis: 32 pixels from center to edge. (32 / 9.8 = ~3.2 multiplier)
     * We invert the math (-) so the bubble moves "up"
    */  
    int bubble_x = 64 - (int)(ax * 6.5);
    int bubble_y = 32 + (int)(ay * 3.2);

    display_hal_clear();
    display_hal_draw_background();
    display_hal_draw_bubble(bubble_x, bubble_y);
    display_hal_finalize();

    k_msleep(50);
  }

  return 0;
}
