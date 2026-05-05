#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

// LED's 
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

// RX buffer 
static uint8_t rx_buf[10] = {0};

// UART callback 
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
  switch (evt->type) {
    case UART_RX_RDY:
      // Loop through all received bytes (just in case the terminal sends '1' + '\n')
      for (int i = 0; i < evt->data.rx.len; i++) {
        uint8_t received_char = evt->data.rx.buf[evt->data.rx.offset + i];

        switch (received_char) {
          case '1': // Notice the single quotes!
            gpio_pin_toggle_dt(&led0);
            break;
          case '2':
            gpio_pin_toggle_dt(&led1);
            break;
          case '3':
            gpio_pin_toggle_dt(&led2);
            break;
          case '4':
            gpio_pin_toggle_dt(&led3);
            break;
          default:
            // Ignore other characters like Enter (\r or \n)
            break;
        }
      }
      break;

    case UART_RX_DISABLED:
      uart_rx_enable(dev, rx_buf, sizeof(rx_buf), 100);
      break;

    default:
      break;
  }
}

int main(void)
{
  int ret;
  if (!device_is_ready(led0.port) ||
      !device_is_ready(led1.port) ||
      !device_is_ready(led2.port) ||
      !device_is_ready(led3.port) ||
      !device_is_ready(uart)
     ) {
    printk("Device not ready");
    return -1;
  }

  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    return 1;
  }

  ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    return 1;
  }

  ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    return 1;
  }

  ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    return 1;
  }

  ret = uart_callback_set(uart, uart_cb, NULL);
  if (ret < 0) {
    return 1;
  }

  static uint8_t tx_buf[] = {"Press 1-4 to toggle the 4 LED's\r\n"};

  ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
  if (ret < 0) {
    return 1;
  }

  // Receive data 
  ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 100);
  if (ret < 0) {
    return 1;
  }

  while (1) {
    k_sleep(K_MSEC(1000));
  }
  
  return 0;
}
