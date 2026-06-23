#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <stdint.h>

/* Initialize the display and the framebuffer */
int display_hal_init(void);

/* Clears the display, writes text at (x, y) and pushes the frame */
void display_hal_print(const char *text, uint8_t x, uint8_t y);

#endif // !DISPLAY_HAL_H
