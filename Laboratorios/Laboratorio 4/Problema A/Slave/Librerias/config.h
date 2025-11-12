#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * Define la frecuencia del reloj del sistema.
 * Es crucial para <util/delay.h> y los cálculos de UART.
 */
#define F_CPU 16000000UL

/**
 * Interruptor de modo de compilación.
 * Definir esto como 1 para compilar para la simulación de Proteus
 * (usará ADC/potenciómetros en lugar de DHT22).
 *
 * Definir esto como 0 para compilar para el hardware físico real
 * (usará la biblioteca DHT22).
 */
#define SIMULATION_MODE 1

#endif /* CONFIG_H_ */
