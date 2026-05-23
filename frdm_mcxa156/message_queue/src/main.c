#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#define SW0_NODE     DT_ALIAS(sw0)
#define LED0_NODE    DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

static struct gpio_callback button_cb_data;

LOG_MODULE_REGISTER(msgq_logger, LOG_LEVEL_INF);

// Data package 
struct button_data {
  uint32_t press_count;
  uint32_t uptime_ms;
};

K_MSGQ_DEFINE(button_msgq, sizeof(struct button_data), 10, 4);

// ISR 
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  static uint32_t counter = 0;
  counter++;

  struct button_data payload;
  payload.press_count = counter;

  payload.uptime_ms = k_uptime_get_32();

  k_msgq_put(&button_msgq, &payload, K_NO_WAIT);
}

// Worker thread 
void led_worker_thread(void) {
  struct button_data rx_data;

  while (1) {
    k_msgq_get(&button_msgq, &rx_data, K_FOREVER);

    LOG_INF("Button Event Received");
    LOG_INF("Press #: %d", rx_data.press_count);
    LOG_INF("Uptime (ms): %d", rx_data.uptime_ms);

    gpio_pin_set_dt(&led, 1);
    k_msleep(100);
    gpio_pin_set_dt(&led, 0);
  }
}

K_THREAD_DEFINE(led_worker_id, 1024, led_worker_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
  LOG_INF("Message Queue Application");

  if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
    LOG_ERR("Hardware not ready");
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
