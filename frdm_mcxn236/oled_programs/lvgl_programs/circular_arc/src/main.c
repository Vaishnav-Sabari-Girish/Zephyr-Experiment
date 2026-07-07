#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(lvgl_arc, LOG_LEVEL_INF);

int main(void)
{
  LOG_INF("Starting LVGL Circular Arc Demo");

  /* 1. Hardware and LVGL Safety Checks */
  const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display_dev)) {
    LOG_ERR("Display not ready! Unplug USB to reset I2C bus.");
    return -1;
  }
  display_blanking_off(display_dev);

  if (lv_disp_get_default() == NULL) {
    LOG_ERR("LVGL failed to bind to the OLED.");
    return -1;
  }

  /* 2. Grab the screen and force 1-bit high-contrast colors */
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_color(scr, lv_color_white(), LV_PART_MAIN);

  /* 3. Create the Arc Widget (Shifted to the right) */
  lv_obj_t *arc = lv_arc_create(scr);
  lv_obj_set_size(arc, 54, 54);                  /* Sized to fit 64px height */
  lv_obj_align(arc, LV_ALIGN_RIGHT_MID, -10, 0); /* Place on the right side */

  lv_arc_set_bg_angles(arc, 135, 45);
  lv_arc_set_range(arc, 0, 100);

  /* 1-Bit Widget Styling */
  /* Destroy the white bounding box */
  lv_obj_set_style_bg_color(arc, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_width(arc, 0, LV_PART_MAIN);

  /* Style the empty track (thin line) */
  lv_obj_set_style_arc_color(arc, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_arc_width(arc, 1, LV_PART_MAIN);

  /* Style the filled indicator (thick line) */
  lv_obj_set_style_arc_color(arc, lv_color_white(), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc, 4, LV_PART_INDICATOR);

  /* Obliterate the touchscreen Knob */
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);

  /* 4. Create the percentage label */
  lv_obj_t *val_label = lv_label_create(scr);
  lv_label_set_text(val_label, "0%");
  /* Lock the text specifically to the center of the ARC widget */
  lv_obj_align_to(val_label, arc, LV_ALIGN_CENTER, 0, 0);

  /* 5. Create a Title Label (Shifted to the left) */
  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "CPU\nLOAD");         /* \n splits it into stacked lines */
  lv_obj_align(title, LV_ALIGN_LEFT_MID, 15, 0); /* Place on the left side */

  /* 6. The Animation Loop */
  int value = 0;
  int step = 2;
  char val_str[8];

  while (1) {
    /* Bounce the value back and forth between 0 and 100 */
    value += step;
    if (value >= 100 || value <= 0) {
      step = -step;
    }

    /* Update the Arc's fill value */
    lv_arc_set_value(arc, value);

    /* Format and update the text label inside the Arc */
    snprintf(val_str, sizeof(val_str), "%d%%", value);
    lv_label_set_text(val_label, val_str);

    /* Tell LVGL to render the new frame */
    lv_task_handler();

    /* Update at a smooth framerate */
    k_msleep(40);
  }

  return 0;
}
