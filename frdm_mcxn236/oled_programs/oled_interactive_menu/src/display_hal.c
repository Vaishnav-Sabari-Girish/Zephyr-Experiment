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

  display_hal_clear();
  display_blanking_off(display_dev);

  /* Set a default fallback font upon initialization */
  cfb_framebuffer_set_font(display_dev, 0);

  return 0;
}

void display_hal_set_font_size(uint8_t target_height) {
  if (display_dev == NULL) return;

  uint8_t num_fonts = cfb_get_numof_fonts(display_dev);
  uint8_t f_width, f_height;

  for (int i = 0; i < num_fonts; i++) {
    cfb_get_font_size(display_dev, i, &f_width, &f_height);

    if (f_height == target_height) {
      cfb_framebuffer_set_font(display_dev, i);
      LOG_INF("Font changed successfully to %dx%d", f_width, f_height);
      return;
    }
  }

  LOG_ERR("Requested font size %d not found in compiled fonts!", target_height);
}

void display_hal_clear() {
  if (display_dev != NULL) {
    cfb_framebuffer_clear(display_dev, false);
  }
}

void display_hal_finalize() {
  if (display_dev != NULL) {
    cfb_framebuffer_finalize(display_dev);
  }
}

void display_hal_print(const char *text, uint8_t x, uint8_t y) {
  if (display_dev != NULL) {
    cfb_print(display_dev, text, x, y);
  }
}

void display_hal_draw_divider() {
  struct cfb_position start = {0, 26};
  struct cfb_position end = {128, 26};
  cfb_draw_line(display_dev, &start, &end);
}
