#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>
#include <stdbool.h>

// El pin se define en el .c
void dht22_init(void);

bool dht22_read(int16_t *temperature_c_x10, uint16_t *humidity_x10);


#endif
