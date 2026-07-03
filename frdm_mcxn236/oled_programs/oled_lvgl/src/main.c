#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(lvgl_main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Starting LVGL - Monochrome Optimized");

    /* 1. Verify the OLED is physically communicating */
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Display not ready! Unplug USB to reset I2C bus.");
        return -1;
    }

    display_blanking_off(display_dev);

    /* 2. Verify Zephyr's LVGL wrapper successfully booted in the background */
    if (lv_disp_get_default() == NULL) {
        LOG_ERR("LVGL failed to bind to the OLED. Check memory allocations.");
        return -1;
    }

    /* 3. Grab the active screen pointer */
    lv_obj_t *scr = lv_scr_act();
    
    /* 4. Force high-contrast colors (Required for 1-bit displays) */
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(scr, lv_color_white(), LV_PART_MAIN);

    /* 5. Create the User Interface */
    lv_obj_t *label1 = lv_label_create(scr);
    lv_label_set_text(label1, "LVGL SUCCESS!");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *label2 = lv_label_create(scr);
    lv_label_set_text(label2, "OLED Running");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 10);

    /* 6. The LVGL Tick Loop */
    while (1) {
        lv_task_handler();
        k_msleep(10);
    }

    return 0;
}
