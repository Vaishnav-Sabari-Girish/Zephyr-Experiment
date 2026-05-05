#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#define I2C0_NODE DT_NODELABEL(mysensor)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

// Essential registers for MPU9250 
#define MPU9250_REG_PWR_MGMT_1     0x6B
#define MPU9250_REG_WHO_AM_I       0x75
#define MPU9250_REG_ACCEL_START    0x3B 

// Register the logging module
LOG_MODULE_REGISTER(i2c_log, LOG_LEVEL_DBG);
int main(void)
{
  int ret;
  if (!device_is_ready(dev_i2c.bus)) {
    LOG_ERR("I2C bus %s is not ready", dev_i2c.bus -> name);
    return -1;
  }

  LOG_INF("I2C bus ready. Talking to device at 0x%02X", dev_i2c.addr);

  // Wake the sensor up 
  uint8_t wake_config[2] = {MPU9250_REG_PWR_MGMT_1, 0x00};

  ret = i2c_write_dt(&dev_i2c, wake_config, sizeof(wake_config));

  if (ret != 0) {
    LOG_ERR("Failed t wake MPU9250 (Reg %02X)", wake_config[0]);
    return -1;
  }

  LOG_INF("Sensor Awake");

  k_msleep(100);

  // Verify connection 
  // Read the WHO_AM_I register, should return 0x71 or 0x73 
  uint8_t who_am_i_reg = MPU9250_REG_WHO_AM_I;
  uint8_t who_am_i_val = 0;

  ret = i2c_write_read_dt(&dev_i2c, &who_am_i_reg, 1, &who_am_i_val, 1);
  if (ret != 0) {
    LOG_ERR("Failed to read WHO_AM_I register");
  } else {
    LOG_INF("WHO_AM_I reading: 0x%02X", who_am_i_val);
  }

  while (1) {
    uint8_t accel_data[14] = {0};

    ret = i2c_burst_read_dt(&dev_i2c, MPU9250_REG_ACCEL_START, accel_data, sizeof(accel_data));

    if (ret != 0) {
      LOG_ERR("Failed to burst read accelerometer");
    } else {
      // Raw values (accelerometer)
      int16_t accel_x = (accel_data[0] << 8) | accel_data[1];
      int16_t accel_y = (accel_data[2] << 8) | accel_data[3];
      int16_t accel_z = (accel_data[4] << 8) | accel_data[5];

      // Raw values (Gyroscope)
      
      int16_t gyro_x = (accel_data[8] << 8) | accel_data[9];
      int16_t gyro_y = (accel_data[10] << 8) | accel_data[11];
      int16_t gyro_z = (accel_data[12] << 8) | accel_data[13];

      // Actual values [G]
      float a_x = (float)accel_x / 16384.0f;
      float a_y = (float)accel_y / 16384.0f;
      float a_z = (float)accel_z / 16384.0f;

      // Actual values +/- 250 DPS (131.0 LSB/DPS)
      float g_x = (float)gyro_x / 131.0f;
      float g_y = (float)gyro_y / 131.0f;
      float g_z = (float)gyro_z / 131.0f;

      LOG_INF("Accel X: %.2f  | Y: %.2f  | Z: %.2f", a_x, a_y, a_z);
      LOG_INF("Gyro X: %.2f  | Y: %.2f  | Z: %.2f", g_x, g_y, g_z);
    }
    k_msleep(500);
  }

  return 0;
}
