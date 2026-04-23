# Programs for Nordic nrf52830dk

## Commands

### To build

```bash
cd <program>
west build -p always -b nrf52830dk/nrf52830dk <program>
west flash
```

## Programs

1. [Print to console](./print_console/src/main.c)
2. [Blinky](./blinky/README.md)
3. Button
    1. [Button Polling](./button_polling/README.md)
    2. [Button Interrupts](./button_interrupt/README.md)
4. Threads and Semaphores
    1. [One Button](./threads_semaphores/one_button_thread_semaphore/src/)
    2. All Buttons
