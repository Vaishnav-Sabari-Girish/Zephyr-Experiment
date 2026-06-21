#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(imu_internal_logger, LOG_LEVEL_DBG);

#define ACCEL_NODE DT_NODELABEL(fxls8974)

#define LED0_NODE  DT_ALIAS(led0)
#define LED1_NODE  DT_ALIAS(led1)
#define LED2_NODE  DT_ALIAS(led2)

static const struct device *accel = DEVICE_DT_GET(ACCEL_NODE);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

/* RGB helper */
static void set_rgb(bool r, bool g, bool b) {
  gpio_pin_set_dt(&led0, r);
  gpio_pin_set_dt(&led1, g);
  gpio_pin_set_dt(&led2, b);
}

int main(void)
{
  struct sensor_value x, y, z;

  LOG_INF("IMU Tilt indicator (Using on-board FXLS8974)");

  if (!device_is_ready(accel)) {
    LOG_ERR("FXLS8974 is not ready");
    return -1;
  }

  gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

  while (1) {
    sensor_sample_fetch(accel);

    sensor_channel_get(accel, SENSOR_CHAN_ACCEL_X, &x);
    sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Y, &y);
    sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Z, &z);

    double xg = sensor_value_to_double(&x);
    double yg = sensor_value_to_double(&y);
    double zg = sensor_value_to_double(&z);

    LOG_INF("X=%.2f  Y=%.2f  Z=%.2f", xg, yg, zg);

    /* Upside Down */
    if (zg < -8.0) {
      set_rgb(1, 1, 1);           // White
    }

    /* Right Tilt */
    else if (xg > 3.0) {
      set_rgb(0, 0, 1);          // Blue
    }

    /* Left Tilt */
    else if (xg < -3.0) {
      set_rgb(1, 0, 0);          // Red
    }

    /* Right Tilt (Y)*/
    else if (yg > 3.0) {
      set_rgb(1, 1, 0);          // Cyan
    }

    /* Left Tilt (Y) */
    else if (yg < -3.0) {
      set_rgb(0, 1, 1);          // Yellow
    }

    /* Flat */
    else {
      set_rgb(0, 1, 0);
    }

    k_msleep(100);
  }

  return 0;
}
