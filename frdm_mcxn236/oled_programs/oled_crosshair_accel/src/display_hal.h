#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <stdint.h>

/* Initialize the display and the framebuffer */
int display_hal_init(void);

/* Clears the display, writes text at (x, y) and pushes the frame */
void display_hal_clear(void);

void display_hal_finalize(void);

void display_hal_draw_background(void);
void display_hal_draw_bubble(int x, int y);

#endif // !DISPLAY_HAL_H
