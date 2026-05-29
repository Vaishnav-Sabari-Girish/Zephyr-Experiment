#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gy91_logger, LOG_LEVEL_DBG);

#define MPU9250_NODE    DT_COMPAT_GET_ANY_STATUS_OKAY(invensense_mpu9250)
#define BMP280_NODE    DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280)

static const struct device *const mpu = DEVICE_DT_GET(MPU9250_NODE);
static const struct device *const bmp = DEVICE_DT_GET(BMP280_NODE);

static int sensor_check(const struct device *dev, const char *label) {
  if (dev == NULL) {
    LOG_ERR("%s: No device found in devicetree", label);
    return -ENODEV;
  }

  if (!device_is_ready(dev)) {
    LOG_ERR("%s: Device is not ready", label);
    return -ENODEV;
  }

  LOG_INF("%s: Device is ready (%s)", label, dev -> name);
  return 0;
}

// MPU9250 
static int mpu9250_read(void) {
  struct sensor_value accel[3], gyro[3], magn[3], die_temp;
  int rc;

  rc = sensor_sample_fetch(mpu);
  if (rc < 0) {
    LOG_ERR("MPU9250 fetch failed: %d", rc);
    return rc;
  }

  rc = sensor_channel_get(mpu, SENSOR_CHAN_ACCEL_XYZ, accel);
  rc |= sensor_channel_get(mpu, SENSOR_CHAN_GYRO_XYZ, gyro);
  rc |= sensor_channel_get(mpu, SENSOR_CHAN_MAGN_XYZ, magn);
  rc |= sensor_channel_get(mpu, SENSOR_CHAN_DIE_TEMP, &die_temp);

  if (rc < 0) {
    LOG_ERR("MPU9250 channel get failed: %d", rc);
    return rc;
  }

  LOG_INF("Accel: %.2f  %.2f  %.2f ms^2  "
          "Gyro:  %.2f  %.2f  %.2f rad/s "
          "Mag:   %.2f  %.2f  %.2f uT "
          "DieT:  %.2f C",
          sensor_value_to_double(&accel[0]),
          sensor_value_to_double(&accel[1]),
          sensor_value_to_double(&accel[2]),
          sensor_value_to_double(&gyro[0]),
          sensor_value_to_double(&gyro[1]),
          sensor_value_to_double(&gyro[2]),
          sensor_value_to_double(&magn[0]),
          sensor_value_to_double(&magn[1]),
          sensor_value_to_double(&magn[2]),
          sensor_value_to_double(&die_temp)
          );

  return 0;
}

// BMP280 
static int bmp280_read(void) {
  struct sensor_value temp, press;
  int rc;

  rc = sensor_sample_fetch(bmp);

  if (rc < 0) {
    LOG_ERR("BMP280 fetch failed: %d", rc);
    return rc;
  }

  rc = sensor_channel_get(bmp, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  rc |= sensor_channel_get(bmp, SENSOR_CHAN_PRESS, &press);

  if (rc < 0) {
    LOG_ERR("BMP280 channel get failed: %d", rc);
    return rc;
  }

  LOG_INF("Temp: %.2f C, Pressure: %.2f hPa",
          sensor_value_to_double(&temp),
          sensor_value_to_double(&press) / 100.0
          );

  return 0;
}

int main(void)
{
  if (sensor_check(mpu, "MPU9250") < 0) return -1;
  if (sensor_check(bmp, "BMP280") < 0) return -1;

  while (1) {
    mpu9250_read();
    bmp280_read();
    k_msleep(1000);
  }

  return 0;
}
