#include "twi_slave.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint8_t twi_last_cmd = 0;
volatile uint8_t twi_new_cmd = 0;

// Inicializa TWI (I2C) en modo esclavo
void TWI_SlaveInit(uint8_t address) {
	TWAR = (address << 1);
	TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
}

ISR(TWI_vect) {
	uint8_t status = TWSR & 0xF8;

	switch(status) {
		case 0x60:  // SLA+W recibido
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
		break;

		case 0x80:  // DATA recibido
		twi_last_cmd = TWDR;
		twi_new_cmd = 1;
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
		break;

		default:
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
		break;
	}
}
