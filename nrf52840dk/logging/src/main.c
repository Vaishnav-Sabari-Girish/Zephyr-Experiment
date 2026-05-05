#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(basic_logger, LOG_LEVEL_DBG);

int main(void)
{
  uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 
                    0x04, 0x05, 0x06, 0x07,
                    'H', 'e', 'l', 'l', 'o'};

  LOG_INF("Logging Application for nrd52840dk");
  LOG_DBG("Debug Level Log");
  LOG_INF("Info Level Log");
  LOG_WRN("Warning Level Log");
  LOG_ERR("Error Level Log");

  // Hexdump data 
  LOG_HEXDUMP_INF(data, sizeof(data), "Sample Data!");
  return 0;
}
