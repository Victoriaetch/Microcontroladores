#ifndef MQ135_H
#define MQ135_H

#include <stdint.h>

void mq135_init(uint8_t adc_channel);
uint16_t mq135_read_raw(void); // 0..1023

#endif

