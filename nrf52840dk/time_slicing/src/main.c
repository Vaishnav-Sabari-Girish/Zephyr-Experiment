#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define STACK_SIZE       1024
#define SHARED_PRIORITY     5

LOG_MODULE_REGISTER(time_slice_demo, LOG_LEVEL_INF);

/* 
 * 1. Define a Mutex (Think of it as a "talking stick") 
 */
K_MUTEX_DEFINE(uart_mutex);

/*
  * Thread A
*/ 
void thread_a_entry(void) {
  while (1) {
    /* 2. Grab the talking stick. If Thread B has it, wait here until they drop it. */
    k_mutex_lock(&uart_mutex, K_FOREVER);
    
    LOG_WRN("Thread A is hoarding the CPU");
    
    /* 3. Drop the talking stick so others can use the console. */
    k_mutex_unlock(&uart_mutex);

    k_busy_wait(20000);
  }
}

/*
  * Thread B 
*/
void thread_b_entry(void) {
  while (1) {
    /* Thread B must also ask for the talking stick */
    k_mutex_lock(&uart_mutex, K_FOREVER);
    
    LOG_WRN("Thread B is hoarding the CPU");
    
    k_mutex_unlock(&uart_mutex);

    k_busy_wait(20000);
  }
}

K_THREAD_DEFINE(thread_a_id, STACK_SIZE, thread_a_entry, NULL, NULL, NULL, SHARED_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread_b_id, STACK_SIZE, thread_b_entry, NULL, NULL, NULL, SHARED_PRIORITY, 0, 0);

int main(void)
{
  LOG_INF("Starting the Time slicing demo with Mutexes!");

  while (1) {
    k_sleep(K_FOREVER);
  }

  return 0;
}
