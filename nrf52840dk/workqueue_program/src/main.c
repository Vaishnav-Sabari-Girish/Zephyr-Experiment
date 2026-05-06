#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(workqueue_demo, LOG_LEVEL_INF);

/* 
 * 1. Define Priorities 
 * Lower number = Higher priority in Zephyr.
 * Threads A and B will preempt the Workqueue.
 */
#define STACK_SIZE       1024
#define SHARED_PRIORITY  2 
#define WORKQ_PRIORITY   4

/* 2. Define the Console Mutex */
K_MUTEX_DEFINE(uart_mutex);

/* 3. Define Custom Workqueue Stack and Queue Object */
K_THREAD_STACK_DEFINE(my_stack_area, STACK_SIZE);
struct k_work_q offload_work_q;

/* 
 * 4. Define the Custom Work Item 
 * We wrap the standard k_work struct inside our own custom struct
 * so we can pass data (like a name/string) into the background task.
 */
struct work_info {
    struct k_work work;
    char name[25];
} my_work;

/* Emulate heavy mathematical work */
static inline void emulate_work(void) {
    for(volatile int count_out = 0; count_out < 300000; count_out ++);
}

/* 
 * 5. The Workqueue Execution Function
 * This is what the background thread actually runs when work is submitted.
 */
void offload_function(struct k_work *item) {
    /* 
     * MAGIC MACRO: CONTAINER_OF
     * Zephyr only passes the `k_work` pointer. We use this macro to calculate 
     * where the parent `work_info` struct is in memory so we can read the `name`.
     */
    struct work_info *the_work = CONTAINER_OF(item, struct work_info, work);
    
    k_mutex_lock(&uart_mutex, K_FOREVER);
    LOG_WRN(">>> Workqueue executing task from: %s", the_work->name);
    k_mutex_unlock(&uart_mutex);
    
    emulate_work();
}

/* 
 * THREAD A (Priority 2)
 */
void thread_a_entry(void) {
    while (1) {
        k_mutex_lock(&uart_mutex, K_FOREVER);
        LOG_INF("Thread A running. Submitting a ticket to the Workqueue...");
        k_mutex_unlock(&uart_mutex);

        /* Submit the work to our custom background queue */
        k_work_submit_to_queue(&offload_work_q, &my_work.work);

        /* 
         * Thread A MUST sleep. If it used k_busy_wait() here, 
         * the Priority 4 Workqueue would starve and never run!
         */
        k_msleep(1000); 
    }
}

/* 
 * THREAD B (Priority 2)
 */
void thread_b_entry(void) {
    while (1) {
        k_mutex_lock(&uart_mutex, K_FOREVER);
        LOG_INF("Thread B running independently.");
        k_mutex_unlock(&uart_mutex);

        k_msleep(1000); 
    }
}

/* Initialize the threads statically */
K_THREAD_DEFINE(thread_a_id, STACK_SIZE, thread_a_entry, NULL, NULL, NULL, SHARED_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread_b_id, STACK_SIZE, thread_b_entry, NULL, NULL, NULL, SHARED_PRIORITY, 0, 0);

/* 
 * MAIN THREAD
 */
int main(void) {
    k_mutex_lock(&uart_mutex, K_FOREVER);
    LOG_INF("Starting Workqueue and Mutex Demo");
    k_mutex_unlock(&uart_mutex);

    /* Start the Custom Workqueue in the background */
    k_work_queue_start(&offload_work_q, my_stack_area,
                       K_THREAD_STACK_SIZEOF(my_stack_area), WORKQ_PRIORITY,
                       NULL);

    /* Initialize the Work Item and package our custom string into it */
    strcpy(my_work.name, "Thread A");
    k_work_init(&my_work.work, offload_function);

    /* Let the main thread sleep forever so A, B, and the Workqueue can take over */
    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}
