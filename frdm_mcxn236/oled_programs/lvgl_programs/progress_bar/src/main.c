#include "misc/lv_area.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(progress_bar, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("Starting LVGL Progress Bar demo");

  const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

  if (!device_is_ready(display)) {
    LOG_ERR("Display not ready.");
    return -1;
  }

  display_blanking_off(display);

  if (lv_display_get_default() == NULL) {
    LOG_ERR("LVGL failed to bind to the OLED");
    return -1;
  }

  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_color(scr, lv_color_white(), LV_PART_MAIN);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "System Boot");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

  lv_obj_t *bar = lv_bar_create(scr);
  lv_obj_set_size(bar, 100, 12);
  lv_obj_align(bar, LV_ALIGN_CENTER, 0, 5);
  lv_bar_set_range(bar, 0, 100);

  /* 
     * 1-Bit Widget Styling: 
     * By default, LVGL uses grey colors for widgets. We must force it 
     * to draw a white border, a black background, and a white indicator fill.
  */
  lv_obj_set_style_border_color(bar, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
  lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_INDICATOR);

  lv_obj_t *pct_label = lv_label_create(scr);
  lv_label_set_text(pct_label, "0%");
  lv_obj_align(pct_label, LV_ALIGN_BOTTOM_MID, 0, -5);

  int progress = 0;
  char pct_str[8];

  while (1) {

    lv_bar_set_value(bar, progress, LV_ANIM_OFF);

    snprintf(pct_str, sizeof(pct_str), "%d%%", progress);

    lv_label_set_text(pct_label, pct_str);

    progress += 2;
    if (progress > 100) {
      progress = 0;
      k_msleep(500);
    }

    lv_task_handler();

    k_msleep(50);
  }

  return 0;
}
