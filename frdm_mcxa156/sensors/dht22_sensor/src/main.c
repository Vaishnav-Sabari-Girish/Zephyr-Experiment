#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dht22_logger, LOG_LEVEL_DBG);

int main(void)
{
  const struct device *const dht = DEVICE_DT_GET_ONE(aosong_dht);

  if (!device_is_ready(dht)) {
    LOG_ERR("Device %s is not ready.", dht -> name);
    return 0;
  }

  LOG_INF("Device %s found", dht -> name);

  struct sensor_value temp, humidity;

  while (1) {
    k_sleep(K_SECONDS(2));

    int ret = sensor_sample_fetch(dht);

    if (ret == 0) {
      sensor_channel_get(dht, SENSOR_CHAN_AMBIENT_TEMP, &temp);

      sensor_channel_get(dht, SENSOR_CHAN_HUMIDITY, &humidity);

      LOG_INF("Temperature (C): %.2f , Humidity(%%): %.2f",
               sensor_value_to_double(&temp),
               sensor_value_to_double(&humidity)
              );
    } else {
      LOG_ERR("Failed to fet data from sensor. Error code: %d", ret);
      LOG_WRN("Check wirings");
    }
  }
  return 0;
}
