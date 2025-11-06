#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "UART.h"
#include "TWI.h"
#include "i2c_lcd.h"

#define BAUD 9600
#define MY_UBRR F_CPU/16/BAUD-1

#define LCD_BACKLIGHT_ON  1

// --- Pines ---
#define LED_VERDE PB0
#define LED_ROJO  PB1
#define BTN_BORRAR PD2   // INT0
#define BTN_UPDATE PD3   // INT1

// EEPROM almacena UID de tarjeta autorizada
uint8_t eeprom_uid[10] EEMEM;

// --- Prototipos de funciones ---
void leds_init(void);
void botones_init(void);
void interrupciones_init(void);
void read_stored_uid(uint8_t *buf);
void write_stored_uid(const uint8_t *buf);
void clear_stored_uid(void);
uint8_t uid_vacio(const uint8_t *buf);
uint8_t simulate_rfid(uint8_t *uid);
void lcd_show(const char *l1, const char *l2);

// --- Inicializaciones ---
void leds_init(void) {
	DDRB |= (1<<LED_VERDE) | (1<<LED_ROJO);
	PORTB &= ~((1<<LED_VERDE) | (1<<LED_ROJO));
}

void botones_init(void) {
	DDRD &= ~((1<<BTN_BORRAR) | (1<<BTN_UPDATE));
	PORTD |= (1<<BTN_BORRAR) | (1<<BTN_UPDATE);
}

void interrupciones_init(void) {
	EICRA |= (1<<ISC01) | (1<<ISC11);
	EICRA &= ~((1<<ISC00) | (1<<ISC10));
	EIMSK |= (1<<INT0) | (1<<INT1);
	sei();
}

void read_stored_uid(uint8_t *buf) {
	eeprom_read_block((void*)buf, (const void*)eeprom_uid, 10);
}

void write_stored_uid(const uint8_t *buf) {
	eeprom_update_block((const void*)buf, (void*)eeprom_uid, 10);
}

void clear_stored_uid(void) {
	uint8_t blank[10];
	for (uint8_t i=0;i<10;i++) blank[i]=0xFF;
	write_stored_uid(blank);
}

uint8_t uid_vacio(const uint8_t *buf) {
	for (uint8_t i=0;i<10;i++) {
		if (buf[i]!=0xFF && buf[i]!=0x00) return 0;
	}
	return 1;
}

static int hexval(char c) {
	if (c>='0' && c<='9') return c-'0';
	if (c>='A' && c<='F') return 10+c-'A';
	if (c>='a' && c<='f') return 10+c-'a';
	return -1;
}

// Recibe por UART una línea tipo UID:AA BB CC DD EE
uint8_t simulate_rfid(uint8_t *uid) {
	static char buffer[64];
	static uint8_t i = 0;

	while (UCSR0A & (1<<RXC0)) {
		char c = UDR0;
		if (c == '\r') continue;
		if (c == '\n' || i >= sizeof(buffer)-1) {
			buffer[i] = '\0';
			if (strncmp(buffer,"UID:",4)==0 || strncmp(buffer,"uid:",4)==0) {
				memset(uid,0,10);
				char *p = buffer+4;
				uint8_t n=0;
				while (*p && n<5) {
					while (*p==' ') p++;
					int h1 = hexval(p[0]);
					int h2 = hexval(p[1]);
					if (h1>=0 && h2>=0) {
						uid[n++] = (h1<<4)|h2;
						p+=2;
					} else break;
				}
				i=0; buffer[0]='\0';
				if (n==5) return 1;
			}
			i=0; buffer[0]='\0';
		} else buffer[i++]=c;
	}
	return 0;
}

void lcd_show(const char *l1, const char *l2) {
	twi_lcd_clear();
	twi_lcd_cmd(0x80);
	twi_lcd_msg(l1);
	twi_lcd_cmd(0xC0);
	twi_lcd_msg(l2);
}

// --- ISR BOTONES ---
ISR(INT0_vect) { // borrar tarjeta
	_delay_ms(100);
	clear_stored_uid();
	lcd_show("EEPROM borrada","                ");
	uart_print("[EEPROM] Tarjeta borrada\r\n");
	PORTB |= (1<<LED_ROJO);
	_delay_ms(600);
	PORTB &= ~(1<<LED_ROJO);
	lcd_show("Acerque tarjeta","para registrar");
}

ISR(INT1_vect) { // registrar nueva tarjeta
	_delay_ms(100);
	lcd_show("Modo registro","Acerque tarjeta");
	uart_print("[REGISTRO] Enviar UID:AA BB CC DD EE\r\n");
	uint8_t uid[10]={0};
	uint16_t t=0;
	uint8_t ok=0;
	while (t<10000) {
		if (simulate_rfid(uid)) {
			write_stored_uid(uid);
			lcd_show("Tarjeta guardada","                ");
			uart_print("[REGISTRO] Nueva tarjeta guardada\r\n");
			ok=1;
			break;
		}
		_delay_ms(200);
		t+=200;
	}
	if (!ok) {
		lcd_show("Registro cancelado","Timeout");
		uart_print("[REGISTRO] Timeout\r\n");
	}
	_delay_ms(1000);
	lcd_show("Acerque tarjeta","para acceso");
}

// --- MAIN ---
int main(void) {
	uart_init(MY_UBRR);
	leds_init();
	botones_init();
	interrupciones_init();
	twi_init();
	twi_lcd_init(LCD_BACKLIGHT_ON);

	uart_print("=== Cerradura RFID (Simulacion PicSimLab) ===\r\n");
	lcd_show("Bienvenido","Sistema RFID");
	_delay_ms(1500);
	lcd_show("Acerque tarjeta","para acceso");

	uint8_t stored[10];
	uint8_t card[10];
	read_stored_uid(stored);

	while (1) {
		memset(card,0,sizeof(card));
		if (simulate_rfid(card)) {
			uart_print("Tarjeta: ");
			uart_print_hex_array(card,5);

        read_stored_uid(stored);

			if (uid_vacio(stored)) {
				lcd_show("No hay tarjeta","registrada");
				uart_print("Sin tarjeta registrada\r\n");
				PORTB |= (1<<LED_ROJO);
				_delay_ms(800);
				PORTB &= ~(1<<LED_ROJO);
				lcd_show("Acerque tarjeta","para registro");
				continue;
			}

			if (memcmp(card,stored,5)==0) {
				lcd_show("Acceso","PERMITIDO");
				uart_print("Acceso permitido\r\n");
				PORTB |= (1<<LED_VERDE);
				_delay_ms(1500);
				PORTB &= ~(1<<LED_VERDE);
				} else {
				lcd_show("Acceso","DENEGADO");
				uart_print("Acceso denegado\r\n");
				PORTB |= (1<<LED_ROJO);
				_delay_ms(1200);
				PORTB &= ~(1<<LED_ROJO);
			}
			lcd_show("Acerque tarjeta","para acceso");
		}
		_delay_ms(200);
	}
}


