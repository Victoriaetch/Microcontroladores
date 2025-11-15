#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>
#include <stdbool.h>

// El pin se define en el .c
void dht22_init(void);

/**
 * @brief Lee la temperatura (x10) y la humedad (x10) del sensor.
 * @param temperature_c_x10 Puntero para almacenar la temperatura (ej: 25.6°C se guarda como 256)
 * @param humidity_x10 Puntero para almacenar la humedad (ej: 50.5% se guarda como 505)
 * @return true si la lectura y el checksum son exitosos, false si falla.
 */
bool dht22_read(int16_t *temperature_c_x10, uint16_t *humidity_x10);

#endif