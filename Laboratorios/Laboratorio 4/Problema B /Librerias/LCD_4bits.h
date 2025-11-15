#ifndef LCD_4BITS_H
#define LCD_4BITS_H

#include <stdint.h>

void lcd_init(void);
void lcd_clear(void);
void lcd_goto(uint8_t row, uint8_t col);
void lcd_print(const char *s);
void lcd_print_num(int num);

#endif

