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
