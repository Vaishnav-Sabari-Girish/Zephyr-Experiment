#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
  printk("Hello Zephyr\n");
  printk("From STM32 Nucleo L433RC-P\n");

  int counter = 0;

  while (1) {
    printk("Counter Loop: %d\n", counter);
    counter++;

    k_msleep(1000);
  }

  return 0;
}
