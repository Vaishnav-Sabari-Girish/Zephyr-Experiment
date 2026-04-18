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
2. Blinky
    1. [Blink LED](./led_blink/src/main.c)
    2. [Blink all LED's](./led_blink_all/src/main.c)
