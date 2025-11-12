#include "config.h" //Para F_CPU
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h> // Añadido para itoa() 
#include "uart.h"

void uart_init(uint32_t baud){

	uint16_t ubrr = (F_CPU / 16 / baud) - 1;
	
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr);
	
	UCSR0B = (1 << TXEN0);
	
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_tx(uint8_t c){
	while (!(UCSR0A & (1 << UDRE0)));
	
	UDR0 = c;
}

void uart_print(const char *s){
	while(*s) {
		uart_tx((uint8_t)*s++);
	}
}

void uart_print_num(int num){
	char buf[12];
	itoa(num, buf, 10); 
	uart_print(buf);

}
