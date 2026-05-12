#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dht22_log, LOG_LEVEL_DBG);

int main(void)
{
  const struct device *const dht22 = DEVICE_DT_GET_ONE(aosong_dht);

  if (!device_is_ready(dht22)) {
    LOG_ERR("Device %s is not ready", dht22 -> name);
    return -1;
  }

  LOG_INF("DHT22 is ready");

  while (1) {
    int rc = sensor_sample_fetch(dht22);

    if (rc == 0) {
      struct sensor_value temp, hum;

      sensor_channel_get(dht22, SENSOR_CHAN_AMBIENT_TEMP, &temp);
      sensor_channel_get(dht22, SENSOR_CHAN_HUMIDITY, &hum);

      double temperature = sensor_value_to_double(&temp);
      double humidity = sensor_value_to_double(&hum);

      LOG_INF("Temperature: %.1f C, Humidity: %.1f %%", temperature, humidity);

    } else {
      LOG_ERR("Failed to fetch sensor data. Error code: %d", rc);
    }

    k_sleep(K_MSEC(2500));
  }

  return 0;
}
