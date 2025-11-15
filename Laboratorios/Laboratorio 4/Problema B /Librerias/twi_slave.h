#ifndef TWI_SLAVE_H
#define TWI_SLAVE_H

#include <stdint.h>

extern volatile uint8_t twi_last_cmd;
extern volatile uint8_t twi_new_cmd;

void TWI_SlaveInit(uint8_t address);

#endif


