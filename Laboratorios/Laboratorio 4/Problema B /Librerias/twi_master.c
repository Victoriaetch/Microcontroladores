#include "twi_master.h"
#include <avr/io.h>

void TWI_MasterInit(void) {
	TWSR = 0;        // Prescaler = 1
	TWBR = 32;       // ~100kHz con F_CPU = 16 MHz
}

void TWI_Start(void) {
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

void TWI_Stop(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

void TWI_Write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

void TWI_Send(uint8_t address, uint8_t data) {
	TWI_Start();
	TWI_Write(address << 1);   // Dirección en modo escritura
	TWI_Write(data);           // Enviar comando
	TWI_Stop();
}

