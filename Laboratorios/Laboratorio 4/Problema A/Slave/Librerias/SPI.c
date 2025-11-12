#include "spi.h"
#include "uart.h" // Para debug printing
#include <avr/interrupt.h>
#include <stdint.h> // Usar uint8_t para claridad

/**
 * @brief Initializes SPI hardware in Slave mode.
 * MISO (PB4) is set to output.
 * SS (PB2), MOSI (PB3), SCK (PB5) are inputs.
 * Enables SPI (SPE) and SPI Interrupt (SPIE).
 */
void SPI_SlaveInit(void) {
    // Set MISO as output, all other SPI pins (MOSI, SCK, SS) as input
    DDRB |= (1 << PB4); // MISO output
    DDRB &= ~((1 << PB3) | (1 << PB5) | (1 << PB2)); // MOSI, SCK, SS input
    
    // Enable SPI (SPE) and SPI Interrupt (SPIE).
    // MSTR bit is 0, so this configures as Slave.
    SPCR = (1 << SPE) | (1 << SPIE);
    
    // Pre-load SPDR with an initial "ready" value.
    // The master will receive this on its *first* transfer.
 //   SPDR = 0xFF; // 0xFF puede significar "idle/listo"
	SPDR = 0x00;  // En lugar de 0xFF

}

/**
 * @brief SPI Transfer Complete Interrupt Service Routine.
 * This function is called automatically when a full byte
 * has been received from the Master.
 */
ISR(SPI_STC_vect) {
    // 1. Leer el byte de comando del Master
    uint8_t comando = SPDR;
    uint8_t response = comando; // Por defecto, hacer eco del comando como ACK
    
	
	uart_print("Recibiendo SPI: ");
	uart_tx(comando);
	uart_print("\r\n");

    // 2. Imprimir por UART el comando recibido (para debug)
    uart_print("CMD Recibido: '");
    uart_tx(comando);
    uart_print("' -> ");
    
	
    // 3. Procesar el comando
   switch (comando) {
	   case 'L': // Alarma de GAS -> solo LED encendido
	   PORTB |= (1 << PB0);    // LED ON
	   PORTD &= ~(1 << PD3);   // BUZZER OFF
	   PORTB &= ~(1 << PB1);   // RELE OFF
	   uart_print("ALERTA GAS -> LED ON, BUZZER OFF, RELE OFF\r\n");
	   break;

	   case 'B': // Alarma de FUEGO -> solo BUZZER encendido
	   PORTD |= (1 << PD3);    // BUZZER ON
	   PORTB &= ~(1 << PB0);   // LED OFF
	   PORTB &= ~(1 << PB1);   // RELE OFF
	   uart_print("ALERTA FUEGO -> BUZZER ON, LED OFF, RELE OFF\r\n");
	   break;

	   case 'R': // Alarma de TEMPERATURA -> solo RELE encendido
	   PORTB |= (1 << PB1);    // RELE ON
	   PORTB &= ~(1 << PB0);   // LED OFF
	   PORTD &= ~(1 << PD3);   // BUZZER OFF
	   uart_print("ALERTA TEMPERATURA -> RELE ON, LED OFF, BUZZER OFF\r\n");
	   break;

	   case 'N': // Todo normal -> todo apagado
	   PORTB &= ~(1 << PB0);   // LED OFF
	   PORTD &= ~(1 << PD3);   // BUZZER OFF
	   PORTB &= ~(1 << PB1);   // RELE OFF
	   uart_print("TODO NORMAL -> Todo apagado\r\n");
	   break;

case 'x': // Apagar todo
PORTB &= ~((1 << PB0) | (1 << PB1)); // LED y RELÉ off
PORTD &= ~(1 << PD3);                // Buzzer off
uart_print("Todo OFF\r\n");
break;

	   default:
	   uart_print("Comando DESCONOCIDO\r\n");
	   response = '?';
	   break;
   }

    
    // 4. Cargar el byte de respuesta en el registro SPDR.
    // Este byte será enviado de vuelta al Master durante
    // la *siguiente* transacción SPI.
    SPDR = response;
}