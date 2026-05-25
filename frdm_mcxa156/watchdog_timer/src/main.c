#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wdt_logger, LOG_LEVEL_DBG);

#define WDT_NODE  DT_NODELABEL(wwdt0)  

int main(void)
{
  int err;
  int wdt_channel_id;
  const struct device *const wdt = DEVICE_DT_GET(WDT_NODE);

  LOG_INF("Starting hardware watchdog example");

  if (!device_is_ready(wdt)) {
    LOG_ERR("Device %s is not ready", wdt -> name);
    return 0;
  }

  // Configure the watchdog to trigger a full system reset after 5 seconds
  struct wdt_timeout_cfg wdt_config = {
    .window.min = 0U,
    .window.max = 5000U,            // 5000 ms = 5s
    .callback = NULL,               // No software warning
    .flags = WDT_FLAG_RESET_SOC     // Reset the entire System-on-chip
  };

  // Install the Configuration 
  wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);

  if (wdt_channel_id < 0) {
    LOG_ERR("Watchdog install failed: %d", wdt_channel_id);
    return 0;
  }

  // Start the hardware timer
  err = wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG);
  if (err < 0) {
    LOG_ERR("Watchdog setup failed: %d", err);
    return 0;
  }

  LOG_INF("Watchdog started! It will reset the board if not fed within 5 seconds");

  for (int i = 0; i <= 5; i++) {
    LOG_INF("Feeding the watchdog.... (Feed %d/5)", i);
    wdt_feed(wdt, wdt_channel_id);

    // Sleep for 3 seconds
    k_msleep(3000);
  }

  // Simulating a system crash hang
  LOG_WRN("Software freeze! The main thread is stuck");
  LOG_WRN("The watchdog will reboot the MCU in 5 seconds");

  while (1) {
    /* *The thread is trapped here and no longer calling wdt_feed()
     * The hardware timer ill hit 5000ms and the MCU will reset
    */
    k_msleep(100);
  }

  return 0;
}
