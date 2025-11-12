#include "config.h" // AÑADIDO: Para F_CPU
#include <avr/io.h>
#include "MQ135.h"

static uint8_t mq135_ch = 0;

void mq135_init(uint8_t adc_channel){
	mq135_ch = adc_channel & 0x0F;
	
	// NOTA: La inicialización del ADC (ADMUX, ADCSRA)
	// se movió a la función ADC_Init() en main.c,
	// ya que el ADC es compartido por 3 sensores.
	// Solo guardamos el canal aquí.
}

uint16_t mq135_read_raw(void){
	// Seleccionar el canal del MQ135
	ADMUX = (ADMUX & 0xF0) | (mq135_ch & 0x0F);
	
	// Iniciar conversión
	ADCSRA |= (1 << ADSC);
	
	// Esperar a que termine
	while(ADCSRA & (1 << ADSC));
	
	return (uint16_t)ADC;
}
