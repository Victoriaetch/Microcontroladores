#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * Defina la frecuencia del reloj del sistema.
 * Es crucial para <util/delay.h> y los c?lculos de UART.
 */
#define F_CPU 16000000UL

/**
 * Interruptor de modo de compilaci?n.
 * Defina esto como 1 para compilar para la simulaci?n de Proteus
 * (usar? ADC/potenci?metros en lugar de DHT22).
 *
 * Defina esto como 0 para compilar para el hardware f?sico real
 * (usar? la biblioteca DHT22).
 */
#define SIMULATION_MODE 0


#endif /* CONFIG_H_ */