#include <stdio.h>          // FOr snprintf
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/random/random.h>

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t rx_buf[10] = {0};
static uint8_t tx_buf[] = {"UART basic program \r\n"};

// UART callback 
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
  switch (evt -> type) {
    case UART_TX_DONE:
      break;

    case UART_TX_ABORTED:
      break;

    case UART_RX_RDY:
      break;

    case UART_RX_BUF_REQUEST:
      break;

    case  UART_RX_BUF_RELEASED:
      break;

    case UART_RX_DISABLED:
      break;

    case UART_RX_STOPPED:
      break;

    default:
      break;
  }
}

int main(void)
{
  if (!device_is_ready(uart)) {
    return -1;
  }

  const struct uart_config uart_cfg = {
    .baudrate = 115200,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
    .data_bits = UART_CFG_DATA_BITS_8,
    .flow_ctrl = UART_CFG_FLOW_CTRL_NONE
  };

  int err = uart_configure(uart, &uart_cfg);

  if (err == -ENOSYS) {
    return -ENOSYS;
  }

  // Register the callback function 
  err = uart_callback_set(uart, uart_cb, NULL);

  if (err) {
    return err;
  }

  /*
  * Start Listening for incoming data 
  * The '100' is a timeout in microseconds
  * If someone types a few characters and stops,
  * the UART will wait 100us before firing UART_RX_RDY anyway,
  * even if the 10-byte buffer is not full yet
  */ 
  uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 100);

  // Startup message sent 
  err = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);

  if (err) {
    return err;
  }

  /*
  * The main loop 
  * UART is handled asynchronously in the background, this main thread can now do other things
  * So here we will generate random sensor data to send via UART
  */

  static char sensor_buf[50];

  while (1) {
    uint32_t raw_rand = sys_rand32_get();

    int temp_integer = 20 + (raw_rand % 11);
    int temp_decimal = (raw_rand >> 8) % 100;

    int msg_len = snprintf(sensor_buf, sizeof(sensor_buf),
                           "Sensor Temp: %d.%02d C\r\n",
                           temp_integer, temp_decimal);

    uart_tx(uart, (const uint8_t *)sensor_buf, msg_len, SYS_FOREVER_US);

    k_sleep(K_MSEC(2000));
  }

  return 0;
}
