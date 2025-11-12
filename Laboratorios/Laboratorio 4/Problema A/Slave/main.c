#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"
#include "uart.h"

void setup_actuators(void) {
	DDRB |= (1 << PB0) | (1 << PB1);
	DDRD |= (1 << PD3);
	PORTB &= ~((1 << PB0) | (1 << PB1));
	PORTD &= ~(1 << PD3);
}

int main(void) {
	setup_actuators();
	uart_init(9600);
	SPI_SlaveInit();
	sei();  // habilitar interrupciones globales
	uart_print("\r\n--- ESCLAVO INICIADO ---\r\n");
	uart_print("Esperando comandos SPI MASTER...\r\n");
	while (1);
}
