#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * @brief Defina la frecuencia del reloj del sistema.
 * Es crucial para <util/delay.h> y los cálculos de UART.
 */
#define F_CPU 16000000UL

/**
 * @brief Interruptor de modo de compilación.
 * Defina esto como 1 para compilar para la simulación de Proteus
 * (usará ADC/potenciómetros en lugar de DHT22).
 *
 * Defina esto como 0 para compilar para el hardware físico real
 * (usará la biblioteca DHT22).
 */
#define SIMULATION_MODE 1


#endif /* CONFIG_H_ */