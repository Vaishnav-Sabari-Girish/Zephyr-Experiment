#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SW0_NODE     DT_ALIAS(sw0)
#define LED0_NODE    DT_ALIAS(led0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Allocate memory for the callback data structure
static struct gpio_callback button_cb_data;

// Callback function (ISR)
// Function only executes when hardware triggers the interrupt 
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Interrupt Triggered! Toggling LED!\n");
  gpio_pin_toggle_dt(&led);
}

int main(void)
{
  printk("Interrupt on Nucleo L433RC-P\n");

  if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&led)) {
    printk("ERROR: Hardware not ready\n");
    return 0;
  }

  gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

  gpio_pin_configure_dt(&button, GPIO_INPUT);

  // Interrupt activates in rising edge
  gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

  gpio_add_callback_dt(&button, &button_cb_data);

  printk("Interrupts configured. Main thread is now gonna sleep forever\n");

  k_sleep(K_FOREVER);

  return 0;
}
