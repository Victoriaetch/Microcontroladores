#ifndef UART_H
#define UART_H
#include <stdint.h>

void uart_init(uint32_t baud);
void uart_tx(uint8_t c);
void uart_print(const char *s);
void uart_print_num(int num);

#endif

