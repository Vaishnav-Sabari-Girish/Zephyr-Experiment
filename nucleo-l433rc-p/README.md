# Programs for STM32 Nucleo L433RC-P

## Commands

### To build

```bash
cd <program>
west build -p always -b nucleo_l433rc_p <program>
west flash
```

## Programs

1. [Printing to console](./print_console/src/main.c)
2. [Blink LED](./blinky/src/main.c)
3. [Button Polling](./button_polling/src/main.c)
4. [Button Interrupts](./button_interrupts/src/main.c)
5. [Threads and Semaphores](./thread_sync/src/main.c)
6. [Message Queue](./msg_queue/src/main.c)
7. [UART Shell](./uart_shell/src/main.c)
