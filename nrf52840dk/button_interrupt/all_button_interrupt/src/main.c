#include "zephyr/sys/util_macro.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE   DT_ALIAS(led0)
#define SW0_NODE    DT_ALIAS(sw0) 
#define LED1_NODE   DT_ALIAS(led1)
#define SW1_NODE    DT_ALIAS(sw1) 
#define LED2_NODE   DT_ALIAS(led2)
#define SW2_NODE    DT_ALIAS(sw2) 
#define LED3_NODE   DT_ALIAS(led3)
#define SW3_NODE    DT_ALIAS(sw3) 

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios, {0});
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios, {0});
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET_OR(SW3_NODE, gpios, {0});

static struct gpio_callback button1_cb_data;
static struct gpio_callback button2_cb_data;
static struct gpio_callback button3_cb_data;
static struct gpio_callback button4_cb_data;

void button1_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Interrupt Triggered! Toggling LED\n");
  gpio_pin_toggle_dt(&led1);
}

void button2_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Interrupt Triggered! Toggling LED\n");
  gpio_pin_toggle_dt(&led2);
}
void button3_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Interrupt Triggered! Toggling LED\n");
  gpio_pin_toggle_dt(&led3);
}
void button4_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Interrupt Triggered! Toggling LED\n");
  gpio_pin_toggle_dt(&led4);
}

int main(void)
{
  if (
    !gpio_is_ready_dt(&button1) || !gpio_is_ready_dt(&led1) ||
    !gpio_is_ready_dt(&button2) || !gpio_is_ready_dt(&led2) ||
    !gpio_is_ready_dt(&button3) || !gpio_is_ready_dt(&led3) ||
    !gpio_is_ready_dt(&button4) || !gpio_is_ready_dt(&led4)
  ) {
    printk("ERROR: Hardware Not Ready\n");
    return 0;
  }
  gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button1, GPIO_INPUT);

  gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button2, GPIO_INPUT);

  gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button3, GPIO_INPUT);
  
  gpio_pin_configure_dt(&led4, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button4, GPIO_INPUT);
  
  gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
  gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
  gpio_pin_interrupt_configure_dt(&button3, GPIO_INT_EDGE_TO_ACTIVE);
  gpio_pin_interrupt_configure_dt(&button4, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&button1_cb_data, button1_pressed, BIT(button1.pin));
  gpio_init_callback(&button2_cb_data, button2_pressed, BIT(button2.pin));
  gpio_init_callback(&button3_cb_data, button3_pressed, BIT(button3.pin));
  gpio_init_callback(&button4_cb_data, button4_pressed, BIT(button4.pin));

  gpio_add_callback_dt(&button1, &button1_cb_data);
  gpio_add_callback_dt(&button2, &button2_cb_data);
  gpio_add_callback_dt(&button3, &button3_cb_data);
  gpio_add_callback_dt(&button4, &button4_cb_data);

  printk("Interrupts configured. Main thread is now gonna sleep forever\n");

  k_sleep(K_FOREVER);

  return 0;
}
