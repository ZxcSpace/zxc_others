#ifndef __DISPLAY_FOR_LAOWANG_H__
#define __DISPLAY_FOR_LAOWANG_H__

#include "display_ist3931.h"

uint8_t display_for_laowang_init();

void clear_screen(uint8_t val);

uint8_t screen_write_by_pix( const uint8_t x, const uint8_t y,
                            uint8_t width, uint8_t height, const void *buf);

void write_num11(uint16_t num, uint8_t x, uint8_t y);

#endif

