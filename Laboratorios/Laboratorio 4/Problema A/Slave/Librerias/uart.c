#include "config.h" // AÑADIDO: Para F_CPU
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h> // <-- *** ARREGLO CRÍTICO: Añadido para itoa() ***
#include "uart.h"

void uart_init(uint32_t baud){
	// Calculate UBRR value
	uint16_t ubrr = (F_CPU / 16 / baud) - 1;
	
	// Set baud rate
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr);
	
	// Enable Transmitter (TX)
	// Add (1<<RXEN0) if you need to receive data
	UCSR0B = (1 << TXEN0);
	
	// Set frame format: 8data, 1stop bit (8N1)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_tx(uint8_t c){
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0)));
	
	// Put data into buffer, sends the data
	UDR0 = c;
}

void uart_print(const char *s){
	while(*s) {
		uart_tx((uint8_t)*s++);
	}
}

void uart_print_num(int num){
	char buf[12];
	itoa(num, buf, 10); // Esta función requiere stdlib.h
	uart_print(buf);
}