#ifndef TWI_MASTER_H
#define TWI_MASTER_H

#include <stdint.h>

void TWI_MasterInit(void);
void TWI_Start(void);
void TWI_Stop(void);
void TWI_Write(uint8_t data);
void TWI_Send(uint8_t address, uint8_t data);

#endif
