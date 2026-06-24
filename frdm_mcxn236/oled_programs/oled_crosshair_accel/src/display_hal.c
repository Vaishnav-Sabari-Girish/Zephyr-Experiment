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

void display_hal_draw_background(void) {
  if (display_dev == NULL) return;

  /* A thin vertical and horizontal line making a center cross */
  struct cfb_position v_start = {64, 28}, v_end = {64, 38};
  struct cfb_position h_start = {60, 32}, h_end = {68, 32};

  cfb_draw_line(display_dev, &v_start, &v_end);
  cfb_draw_line(display_dev, &h_start, &h_end);
}

/* Draws a rectangle which acts as a bubble */
void display_hal_draw_bubble(int x, int y) {
  if (display_dev == NULL) return;

  /* Prevent the box from drawing off the edge of the screen and crashing it */
  if (x < 2) x = 2;
  if (x > 125) x = 125;
  if (y < 2) y = 2;
  if (y > 61) x = 61;

  /* Top-Left and Bottom-right corners of the rectangle */
  struct cfb_position top_left = {x - 2, y - 2};
  struct cfb_position bottom_right = {x + 2, y + 2};

  cfb_draw_rect(display_dev, &top_left, &bottom_right);
}
