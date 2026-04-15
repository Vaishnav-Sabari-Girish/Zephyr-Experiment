#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SW0_NODE    DT_ALIAS(sw0)
#define LED0_NODE   DT_ALIAS(led0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static struct gpio_callback button_cb_data;

/* Define a Semaphore
* Name: button_sem 
* Initial count: 0 (Not Ready)
* Max count: 1 (Binary Semaphore)
*/ 

K_SEM_DEFINE(button_sem, 0, 1);

/* The ISR (Interrupt Service Routine)
 * This must be incredibly fast. It does no heavy lifting
 * It just gives the semaphore and exits
 */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  k_sem_give(&button_sem);
}

// The worker thread
// This is a completely separate execution context from main() 
// It runs infinitely, but spends 99.9% of it's life safely suspended
void led_worker_thread(void) {
  while (1) {
    k_sem_take(&button_sem, K_FOREVER);

    printk("Worker thread: Received Semaphore! Toggling LED\n");
    gpio_pin_toggle_dt(&led);
  }
}

/* Register the thread with the kernel
 * Name: led_worker_id 
 * Stack Size: 1024 bytes
 * Entry function: led_worker_thread
 * Priority: 7 (Lower numbers are higher priority in Zephyr)
*/ 

K_THREAD_DEFINE(led_worker_id, 1024, led_worker_thread, NULL, NULL, NULL, 7, 0, 0);

// Main thread
int main(void)
{
  printk("Starting Thread and Semaphore app on Nucleo\n");

  if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&led)) {
    printk("ERROR: Hardware is not ready\n");
    return 0;
  }

  gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button, GPIO_INPUT);

  gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

  gpio_add_callback_dt(&button, &button_cb_data);

  printk("Hardware Initialized. Main thread going to sleep\n");

  k_sleep(K_FOREVER);

  return 0;
}
