#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>

#include "twi_master.h"    
#include "LCD_4bits.h"
#include "uart.h" 


#if SIMULATION_MODE == 0
#include "DHT22.h"
#include "MQ135.h"
#endif

#define MQ135_ADC_CHANNEL 0
#define FLAME_ADC_CHANNEL 1
#define TEMP_ADC_CHANNEL  2

#define STOP_BUTTON_PIN   PD4
#define STOP_BUTTON_PORT  PIND

#define GAS_ALARM_THRESHOLD   900
#define FLAME_ALARM_THRESHOLD 255
#define TEMP_ALARM_THRESHOLD  26.5

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
	TWI_MasterInit();   
	ADC_Init();
	uart_init(9600);   


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
	lcd_print("MODO REAL");
	#endif
	_delay_ms(1000);

	// Sincronización inicial
	lcd_clear();
	lcd_print("Sincronizando...");
	TWI_Send(0x20, 'x');   
	_delay_ms(1000);
	lcd_goto(1, 0);
	lcd_print("Sistema OK");
	_delay_ms(800);

	while (1) {

		uint16_t gas_level;
		uint16_t flame_level;
		int16_t  temperature;

		#if SIMULATION_MODE == 1
		gas_level   = ADC_Read(MQ135_ADC_CHANNEL);
		flame_level = ADC_Read(FLAME_ADC_CHANNEL);
		temperature = (int16_t)(ADC_Read(TEMP_ADC_CHANNEL) / 20.46);
		#else
		int16_t temp_x10 = 0;
		uint16_t hum_x10 = 0;
		gas_level   = mq135_read_raw();
		flame_level = ADC_Read(FLAME_ADC_CHANNEL);
		if (dht22_read(&temp_x10, &hum_x10))
		temperature = temp_x10/10;
		#endif
		
		uart_print("G:");
		uart_print_num(gas_level);

		uart_print(" F:");
		uart_print_num(flame_level);

		uart_print(" T:");
		uart_print_num(temperature);

		uart_print("\r\n");


		bool stop_button_pressed = !(STOP_BUTTON_PORT & (1 << STOP_BUTTON_PIN));
		bool gas_alarm   = (gas_level   > GAS_ALARM_THRESHOLD);
		bool flame_alarm = (flame_level > FLAME_ALARM_THRESHOLD);
		bool temp_alarm  = (temperature > TEMP_ALARM_THRESHOLD);


		char command = 'x';

		if (stop_button_pressed) {
			command = 'x';
		}
		else if (gas_alarm) {
			command = 'L';
		}	
		else if (flame_alarm) {
			command = 'B';
		}
		else if (temp_alarm) {
			command = 'R';
		}
		else {
			command = 'x';
		}
		
		uart_print("CMD -> ");
		uart_tx(command);
		uart_print("\r\n");


		// ----------- ENVÍO AL ESCLAVO (I2C) -----------
		TWI_Send(0x20, command);
		_delay_ms(500);
		uart_print("I2C envió: ");
		uart_tx(command);
		uart_print("\r\n");


		// ----------- LCD -----------
		lcd_clear();
		if (flame_alarm && !stop_button_pressed)
		lcd_print("! FUEGO !");
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
        snprintf(buffer, 17, "T:%d G:%u F:%u", (int)temperature, gas_level, flame_level);
		#else
		snprintf(buffer, 17, "Temp: %dC", (int)temperature);
		#endif

		lcd_print(buffer);

		_delay_ms(500);
	}
}


