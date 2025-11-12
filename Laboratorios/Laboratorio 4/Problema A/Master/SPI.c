#include "spi.h"
#include <avr/io.h>

void SPI_MasterInit(void) {
	// Configurar MOSI(PB3), SCK(PB5) y PB2(SS hardware) como SALIDA
	// PB2 debe ser salida para forzar el modo Maestro, aunque no lo usemos.
	DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2);
	
	// Configurar MISO(PB4) como ENTRADA
	DDRB &= ~(1 << PB4);
	
	// Configurar nuestro pin SS personalizado (PD1) como SALIDA
	DDRD |= (1 << PD1);
	
	// Poner PD1 en ALTO (inactivo) por defecto
	PORTD |= (1 << PD1);
	
	// Habilitar SPI, Modo Maestro, y reloj Fosc/16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

/**
 * @brief Envía un byte y recibe un byte del esclavo.
 */
char SPI_Transfer(char data) {
	// 1. Seleccionar esclavo (PD1 BAJO)
	PORTD &= ~(1 << PD1);
	
	// 2. Iniciar transmisión (cargar datos en el registro)
	SPDR = data;
	
	// 3. Esperar que la transmisión termine (SPIF flag se pone a 1)
	while(!(SPSR & (1 << SPIF)));
	
	// 4. *** CORRECCIÓN CRÍTICA ***
	// Leer SPDR. Esto devuelve el byte que el Esclavo envió
	// Y lo que es más importante, limpia el flag SPIF.
	char response = SPDR;
	
	// 5. Deseleccionar esclavo (PD1 ALTO)
	PORTD |= (1 << PD1);
	
	// 6. Devolver la respuesta del esclavo (aunque no la usemos)
	return response;
}