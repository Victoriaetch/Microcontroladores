#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "twi_slave.h"
#include "uart.h"

void setup_actuators(void) {
	DDRB |= (1 << PB0) | (1 << PB1);  
	DDRD |= (1 << PD3);               

	PORTB &= ~((1 << PB0) | (1 << PB1)); 
	PORTD &= ~(1 << PD3);              
}

static void process_command(uint8_t cmd)
{
	switch (cmd) {

		case 'L':   // ALERTA GAS = LED
		PORTB |=  (1 << PB0);   // LED ON
		PORTB &= ~(1 << PB1);   // RELÉ OFF
		PORTD &= ~(1 << PD3);   // BUZZER OFF
		uart_print("ALERTA GAS\r\n");
		break;

		case 'B':   // ALERTA FUEGO = BUZZER
		PORTD |=  (1 << PD3);   // BUZZER ON
		PORTB &= ~(1 << PB0);   // LED OFF
		PORTB &= ~(1 << PB1);   // RELÉ OFF
		uart_print("ALERTA FUEGO\r\n");
		break;

		case 'R':   // ALERTA TEMPERATURA = RELÉ
		PORTB |=  (1 << PB1);   // RELÉ ON
		PORTB &= ~(1 << PB0);   // LED OFF
		PORTD &= ~(1 << PD3);   // BUZZER OFF
		uart_print("ALERTA TEMP\r\n");
		break;

		case 'x':   // APAGAR TODO
		PORTB &= ~((1 << PB0) | (1 << PB1)); // LED OFF, RELÉ OFF
		PORTD &= ~(1 << PD3);               // BUZZER OFF
		uart_print("TODO OFF\r\n");
		break;

		default:
		uart_print("CMD DESCONOCIDO\r\n");
		PORTB &= ~((1 << PB0) | (1 << PB1));
		PORTD &= ~(1 << PD3);
		break;
	}
}


int main(void)
{
	setup_actuators();
	uart_init(9600);

	TWI_SlaveInit(0x20);   // Dirección del esclavo
	sei();

	uart_print("SLAVE READY\r\n");

	while (1) {

		// si el master envió algo:
		if (twi_new_cmd) {
			uint8_t cmd = twi_last_cmd;
			twi_new_cmd = 0;   // reset flag

			process_command(cmd);
		}
		_delay_ms(5);
	}
}
