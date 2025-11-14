#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include "spi.h"
#include "LCD_4bits.h"

#if SIMULATION_MODE == 0
#include "DHT22.h"
#include "MQ135.h"
#endif

#define MQ135_ADC_CHANNEL 0
#define FLAME_ADC_CHANNEL 1
#define TEMP_ADC_CHANNEL  2

#define STOP_BUTTON_PIN   PD4
#define STOP_BUTTON_PORT  PIND

#define GAS_ALARM_THRESHOLD   500
#define FLAME_ALARM_THRESHOLD 60
#define TEMP_ALARM_THRESHOLD  30

void ADC_Init(void) {
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read(uint8_t channel) {
	channel &= 0x07;
	ADMUX = (ADMUX & 0xF8) | channel;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

int main(void) {
	lcd_init();
	SPI_MasterInit();
	ADC_Init();

	DDRD &= ~(1 << STOP_BUTTON_PIN);
	PORTD |= (1 << STOP_BUTTON_PIN);

	#if SIMULATION_MODE == 0
	dht22_init();
	mq135_init(MQ135_ADC_CHANNEL);
	#endif

	lcd_clear();
	lcd_print("Sistema Inicio");
	lcd_goto(1, 0);
	#if SIMULATION_MODE == 1
	lcd_print("MODO SIMULACION");
	#else
	lcd_print("MODO REAL (DHT)");
	#endif
	_delay_ms(1000);

	// Sincronización inicial
	lcd_clear();
	lcd_print("Sincronizando...");
	SPI_Transfer('x'); // apaga todo
	_delay_ms(100);
	lcd_goto(1, 0);
	lcd_print("Sistema OK");
	_delay_ms(500);

	while (1) {
		uint16_t gas_level;
		uint16_t flame_level;
		int16_t temperature;

		#if SIMULATION_MODE == 1
		gas_level = ADC_Read(MQ135_ADC_CHANNEL);
		flame_level = ADC_Read(FLAME_ADC_CHANNEL);
		temperature = (int16_t)(ADC_Read(TEMP_ADC_CHANNEL) / 20.46);
		#else
		int16_t temp_x10 = 0;
		uint16_t hum_x10 = 0;
		gas_level = mq135_read_raw();
		flame_level = ADC_Read(FLAME_ADC_CHANNEL);
		if (dht22_read(&temp_x10, &hum_x10))
		temperature = temp_x10 / 10;
		#endif

		bool stop_button_pressed = !(STOP_BUTTON_PORT & (1 << STOP_BUTTON_PIN));
		bool gas_alarm   = (gas_level > GAS_ALARM_THRESHOLD);
		bool flame_alarm = (flame_level > FLAME_ALARM_THRESHOLD);
		bool temp_alarm  = (temperature > TEMP_ALARM_THRESHOLD);

		char command = 'x'; // valor por defecto: todo apagado

		// -----------------------------------------------------------
		// Lógica de prioridad + botón de silencio
		// -----------------------------------------------------------
		if (stop_button_pressed) {
			// Si se presiona el botón, apaga todo inmediatamente
			command = 'x';
			} else if (flame_alarm) {
			command = 'B';  // Fuego tiene máxima prioridad
			} else if (gas_alarm) {
			command = 'L';  // Gas
			} else if (temp_alarm) {
			command = 'R';  // Temperatura
			} else {
			command = 'x';  // Sistema OK ? todo apagado
		}

		SPI_Transfer(command);
		_delay_ms(50);

		// -----------------------------------------------------------
		// Mostrar estado en la LCD
		// -----------------------------------------------------------
		lcd_clear();
		if (flame_alarm && !stop_button_pressed)
		lcd_print("!!! FUEGO !!!");
		else if (gas_alarm && !stop_button_pressed)
		lcd_print("Alerta Gas");
		else if (stop_button_pressed && (gas_alarm || flame_alarm))
		lcd_print("Alarma Silenciada");
		else if (temp_alarm)
		lcd_print("Alta Temperatura");
		else
		lcd_print("Sistema OK");

		lcd_goto(1, 0);
		char buffer[17];
		#if SIMULATION_MODE == 1
		snprintf(buffer, 17, "T:%d G:%-4u F:%-4u", (int)temperature, gas_level, flame_level);
		#else
		snprintf(buffer, 17, "Temp: %dC", (int)temperature);
		#endif
		lcd_print(buffer);

		_delay_ms(500);
	}
}


