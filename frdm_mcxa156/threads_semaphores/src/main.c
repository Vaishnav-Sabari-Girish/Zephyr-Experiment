#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define LED0_NODE     DT_ALIAS(led0)
#define SW0_NODE      DT_ALIAS(sw0)

LOG_MODULE_REGISTER(thread_logger, LOG_LEVEL_DBG);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

static struct gpio_callback button_cb_data;

// Define the semaphore 
K_SEM_DEFINE(button_sem, 0, 1);

// ISR
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  k_sem_give(&button_sem);
}

// Worker thread 
void led_worker_thread(void) {
  while (1) {
    k_sem_take(&button_sem, K_FOREVER);

    LOG_INF("Worker thread: Received Semaphore! Toggling LED");
    gpio_pin_toggle_dt(&led);
  }
}

// Thread registration 
K_THREAD_DEFINE(led_worker_id, 1024, led_worker_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
  LOG_INF("Thread Semaphore app");

  if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
    LOG_ERR("Hardware is not ready");
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
