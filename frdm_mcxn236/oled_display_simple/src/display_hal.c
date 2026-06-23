#include "display_hal.h"
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(display_hal, LOG_LEVEL_INF);

static const struct device *display_dev = DEVICE_DT_GET(DT_NODELABEL(sh1106));

int display_hal_init(void) {
  if (!device_is_ready(display_dev)) {
    LOG_ERR("Display is not ready");
    return -ENODEV;
  }

  if (display_set_pixel_format(display_dev, PIXEL_FORMAT_MONO10) != 0) {
    if (display_set_pixel_format(display_dev, PIXEL_FORMAT_MONO01) != 0) {
      LOG_ERR("Failed to set required pixel format");
      return -ENOTSUP;
    }
  }

  if (cfb_framebuffer_init(display_dev)) {
    LOG_ERR("Frame buffer initialization failed");
    return -EIO;
  }

  cfb_framebuffer_clear(display_dev, true);
  display_blanking_off(display_dev);

  return 0;
}

void display_hal_print(const char *text, uint8_t x, uint8_t y) {
  if (display_dev == NULL) return;

  cfb_framebuffer_clear(display_dev, false);
  cfb_print(display_dev, text, x, y);
  cfb_framebuffer_finalize(display_dev);
}
