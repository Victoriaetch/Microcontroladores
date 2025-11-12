#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>

/**
 * @brief Inicializa el ATmega328P en modo Maestro SPI.
 * Configura MOSI, SCK, y PB2 (SS hardware) como salidas.
 * Configura MISO como entrada.
 * Configura PD1 como el pin de control SS (salida).
 */
void SPI_MasterInit(void);

/**
 * @brief Transmite un byte de datos al esclavo Y RECIBE un byte.
 * Controla automáticamente el pin PD1 (SS).
 *
 * @param data El byte a transmitir.
 * @return char El byte recibido del esclavo (leído de SPDR).
 */
char SPI_Transfer(char data);

#endif /* SPI_H_ */