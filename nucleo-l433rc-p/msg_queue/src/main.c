#include "zephyr/sys/util_macro.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SW0_NODE     DT_ALIAS(sw0)
#define LED0_NODE    DT_ALIAS(led0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static struct gpio_callback button_cb_data;

/* Define the data package
* This is the payload that will be sent from the ISR to the thread
*/ 
struct button_data {
  uint32_t press_count;
  uint32_t uptime_ms;
};

/* Define the message queue
  * Name: button_msgq
  * Data size: The size of the struct
  * Max items: 10 (It can hold 10 button presses in the backlog if the thread is slow)
  * Alignment: 4 bytes (Standard for 32-bit ARM chips)
  */ 
K_MSGQ_DEFINE(button_msgq, sizeof(struct button_data), 10, 4);

// ISR
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  static uint32_t counter = 0;
  counter++;

  // Pack the data
  struct button_data payload;
  payload.press_count = counter;

  // k_uptime_get_32() grabs the OS tick time in ms
  payload.uptime_ms = k_uptime_get_32();

  // Put the data into the queue
  // K_NO_WAIT is critical. An ISR is never allowed to wait
  k_msgq_put(&button_msgq, &payload, K_NO_WAIT);
}

// Worker thread
void led_worker_thread(void) {
  struct button_data received_data;

  while (1) {
    // Wait forever until a message arrives in the queue
    k_msgq_get(&button_msgq, &received_data, K_FOREVER);

    // Process the data
    printk("-------Button Event Received-----\n");
    printk("Press #: %d\n", received_data.press_count);
    printk("OS Uptime: %d ms\n", received_data.uptime_ms);

    // Give visual feedback
    gpio_pin_set_dt(&led, 1);   // Turn ON the LED
    k_msleep(100);
    gpio_pin_set_dt(&led, 0);   // Turn OFF the LED
  }
}

K_THREAD_DEFINE(led_worker_id, 1024, led_worker_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
  printk("Starting Message Queue Application\n");

  if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&led)) {
    printk("ERROR: Hardware is not ready\n");
    return 0;
  }

  gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button, GPIO_INPUT);

  gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
  gpio_add_callback_dt(&button, &button_cb_data);

  k_sleep(K_FOREVER);

  return 0;
}
