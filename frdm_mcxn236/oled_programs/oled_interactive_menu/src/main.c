#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include "display_hal.h"

LOG_MODULE_REGISTER(menu_system, LOG_LEVEL_INF);

#define SW0_NODE    DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

/* State Machine */
volatile uint8_t current_screen = 0;
#define TOTAL_SCREENS 2

/* Button Debounce variables */
static int64_t last_button_press_time = 0;

// ISR 
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  int64_t now = k_uptime_get();

  /* Ignore the press if it happened within 250ms of the last one */
  if (now - last_button_press_time < 250) {
    return;
  }

  last_button_press_time = now;

  current_screen++;
  if (current_screen >= TOTAL_SCREENS) {
    current_screen = 0;
  }

  LOG_INF("Button Pressed, switched to screen %d", current_screen);
}

int main(void)
{
  LOG_INF("Starting interactive menu");

  if (display_hal_init() != 0) return -1;

  if (!gpio_is_ready_dt(&button)) {
    LOG_ERR("Button is not ready");
    return -1;
  }

  gpio_pin_configure_dt(&button, GPIO_INPUT);
  gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
  gpio_add_callback(button.port, &button_cb_data);

  const struct device *const die_temp = DEVICE_DT_GET(DT_ALIAS(die_temp0));
  if (!device_is_ready(die_temp)) return -1;

  char buffer[32];

  while (1) {
    display_hal_clear();

    // Draw a UI divider for all screens
    display_hal_draw_divider();

    switch (current_screen) {
      case 0:
        /* SCREEN 0: System Uptime */
        display_hal_set_font_size(24);
        display_hal_print("UPTIME", 0, 0);
        
        uint32_t uptime_s = k_uptime_get_32() / 1000;
        uint8_t hours = uptime_s / 3600;
        uint8_t mins = (uptime_s % 3600) / 60;
        uint8_t secs = uptime_s % 60;

        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, mins, secs);
        display_hal_set_font_size(16);
        display_hal_print(buffer, 0, 35);
        break;

      case 1: 
        display_hal_set_font_size(24);
        display_hal_print("DIE TEMP", 0, 0);
        
        struct sensor_value silicon_temperature;
        if (sensor_sample_fetch(die_temp) == 0) {
          sensor_channel_get(die_temp, SENSOR_CHAN_DIE_TEMP, &silicon_temperature);
          double temp_c = sensor_value_to_double(&silicon_temperature);
          snprintf(buffer, sizeof(buffer), "%.2f C", temp_c);
        } else {
          snprintf(buffer, sizeof(buffer), "ERROR");
        }
        
        display_hal_set_font_size(16);
        display_hal_print(buffer, 0, 35);
        break;
    }

    display_hal_finalize();
    k_msleep(100);
  }

  return 0;
}
