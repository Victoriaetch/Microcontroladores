#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "LCD_4bits.h"

// Control pins
#define LCD_RS_PORT PORTD
#define LCD_RS_DDR  DDRD
#define LCD_RS_PIN  PD5

#define LCD_EN_PORT PORTD
#define LCD_EN_DDR  DDRD
#define LCD_EN_PIN  PD6

// Data lines (4-bit)
#define LCD_D4_PORT PORTD
#define LCD_D4_DDR  DDRD
#define LCD_D4_PIN  PD7

#define LCD_D5_PORT PORTB
#define LCD_D5_DDR  DDRB
#define LCD_D5_PIN  PB0

#define LCD_D6_PORT PORTB
#define LCD_D6_DDR  DDRB
#define LCD_D6_PIN  PB1

#define LCD_D7_PORT PORTB
#define LCD_D7_DDR  DDRB
#define LCD_D7_PIN  PB2  

static inline void lcd_pulse_en(void){
	LCD_EN_PORT |= (1<<LCD_EN_PIN);
	_delay_us(1);
	LCD_EN_PORT &= ~(1<<LCD_EN_PIN);
	_delay_us(50);
}

static void lcd_write_nibble(uint8_t nibble){
	if (nibble & 0x01) LCD_D4_PORT |= (1<<LCD_D4_PIN);
	else LCD_D4_PORT &= ~(1<<LCD_D4_PIN);
	
	if (nibble & 0x02) LCD_D5_PORT |= (1<<LCD_D5_PIN);
	else LCD_D5_PORT &= ~(1<<LCD_D5_PIN);

	if (nibble & 0x04) LCD_D6_PORT |= (1<<LCD_D6_PIN);
	else LCD_D6_PORT &= ~(1<<LCD_D6_PIN);

	if (nibble & 0x08) LCD_D7_PORT |= (1<<LCD_D7_PIN);
	else LCD_D7_PORT &= ~(1<<LCD_D7_PIN);
	
	lcd_pulse_en();
}

static void lcd_cmd(uint8_t cmd){
	LCD_RS_PORT &= ~(1<<LCD_RS_PIN);
	_delay_us(1);
	lcd_write_nibble((cmd>>4) & 0x0F);
	lcd_write_nibble(cmd & 0x0F);
}

static void lcd_data(uint8_t data){
	LCD_RS_PORT |= (1<<LCD_RS_PIN);
	_delay_us(1);
	lcd_write_nibble((data>>4) & 0x0F);
	lcd_write_nibble(data & 0x0F);
}

void lcd_init(void){
	LCD_RS_DDR |= (1<<LCD_RS_PIN);
	LCD_EN_DDR |= (1<<LCD_EN_PIN);

	LCD_D4_DDR |= (1<<LCD_D4_PIN);
	LCD_D5_DDR |= (1<<LCD_D5_PIN);
	LCD_D6_DDR |= (1<<LCD_D6_PIN);
	LCD_D7_DDR |= (1<<LCD_D7_PIN);

	LCD_RS_PORT &= ~(1<<LCD_RS_PIN);
	LCD_EN_PORT &= ~(1<<LCD_EN_PIN);
	LCD_D4_PORT &= ~(1<<LCD_D4_PIN);
	LCD_D5_PORT &= ~(1<<LCD_D5_PIN);
	LCD_D6_PORT &= ~(1<<LCD_D6_PIN);
	LCD_D7_PORT &= ~(1<<LCD_D7_PIN);

	_delay_ms(15);

	lcd_write_nibble(0x03);
	_delay_ms(5);
	lcd_write_nibble(0x03);
	_delay_us(100);
	lcd_write_nibble(0x03);
	_delay_us(100);
	lcd_write_nibble(0x02);
	_delay_us(100);

	lcd_cmd(0x28);
	_delay_us(50);
	lcd_cmd(0x0C);
	_delay_us(50);
	lcd_cmd(0x06);
	_delay_us(50);
	lcd_cmd(0x01);
	_delay_ms(2);
}

void lcd_clear(void){
	lcd_cmd(0x01);
	_delay_ms(2);
}

void lcd_goto(uint8_t row, uint8_t col){
	uint8_t addr = (row == 0) ? 0x80 : 0xC0;
	lcd_cmd(addr + col);
}

void lcd_print(const char *s){
	while(*s){
		lcd_data((uint8_t)*s++);
	}
}

void lcd_print_num(int num){
	char buf[12];
	itoa(num, buf, 10);
	lcd_print(buf);
}
