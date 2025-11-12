#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>

/**
 * Inicializa el ATmega328P en modo Esclavo SPI.
 * Configura MISO (PB4) como salida.
 * Configura MOSI, SCK y SS (PB2) como entradas.
 * Habilita el SPI y la Interrupción de SPI (SPIE).
 */
void SPI_SlaveInit(void);


#endif /* SPI_H_ */
