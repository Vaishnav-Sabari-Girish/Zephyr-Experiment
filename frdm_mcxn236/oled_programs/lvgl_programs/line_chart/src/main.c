#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(line_chart, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("Starting LVGL Line chart demo");

  const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display)) {
    LOG_ERR("Display is not ready");
    return -1;
  }

  display_blanking_off(display);

  if (lv_disp_get_default() == NULL) {
    LOG_ERR("LVGL failed to bind to the OLED");
    return -1;
  }

  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_color(scr, lv_color_white(), LV_PART_MAIN);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "Line Chart");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 2);

  lv_obj_t *chart = lv_chart_create(scr);
  lv_obj_set_size(chart, 120, 45);
  lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, -2);

  // Remove background grid lines
  lv_chart_set_div_line_count(chart, 0, 0);

  lv_obj_set_style_bg_color(chart, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_color(chart, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(chart, 1, LV_PART_MAIN);
  lv_obj_set_style_line_color(chart, lv_color_white(), LV_PART_ITEMS);
  lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);

  lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);

  // Data series 
  lv_chart_series_t * ser = lv_chart_add_series(chart, lv_color_white(), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(chart, 25);      // Show the last 25 points on the screen

  int value = 50;
  int step = 5;

  while (1) {
    value += step;

    if (value >= 90 || value <= 10) {
      step = -step;  // Reverse direction
    }
    
    /*
     * Push the value to the chart 
     * LVGL will automatically shift all the old points to the left by 1
    */
    lv_chart_set_next_value(chart, ser, value);

    lv_task_handler();

    k_msleep(100);
  }

  return 0;
}
