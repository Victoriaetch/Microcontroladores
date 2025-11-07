#define F_CPU 16000000UL // Frecuencia micro
// Librerías utilizadas
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1)
#define MAX_DATOS 100 

float datos_temp[MAX_DATOS];  // Arreglo para almacenar las temperaturas medidas
uint8_t datos_heat[MAX_DATOS]; // Arreglo para almacenar estados del calefactor
uint8_t datos_fan[MAX_DATOS];  // Arreglo para almacenar estados del ventilador
uint8_t datos_pm[MAX_DATOS]; // Arreglo para almacenar el punto medio (PM) de cada medición
uint8_t indice_datos = 0; // índice de la posición actual para almacenar datos

// Funciones UART
void UART_send(char c) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void UART_print(char* str) {
	while(*str) UART_send(*str++);
}

void UART_print_P(const char *progmem_str) {
	char c;
	while ((c = pgm_read_byte(progmem_str++))) UART_send(c);
}

char UART_check_receive() {
	if (UCSR0A & (1<<RXC0)) return UDR0;
	return 0;
}

void UART_init() {
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)UBRR_VALUE;
	UCSR0B = (1<<TXEN0) | (1<<RXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

// Lectura de sensor (ADC)
void ADC_init() {
	ADMUX = (1<<REFS0); // AVCC como referencia
	ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // Prescaler 128
}

uint16_t ADC_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Selecciona el canal
	ADCSRA |= (1<<ADSC); // Inicia la conversión
	while(ADCSRA & (1<<ADSC)); 
	return ADC; // Devuelve el valor leído
}

// Funciones PWM 

void PWM_init_fan() { //PWM para ventilador
	DDRB |= (1<<PB3); // Pin PB3 como salida
    TCCR2A = (1<<COM2A1)|(1<<WGM20)|(1<<WGM21); // Modo Fast PWM
    TCCR2B = (1<<CS21); // Prescaler 8
}

void set_fan_speed(uint8_t speed) { // Ajusta la velocidad del ventilador
	OCR2A = speed; // Modifica el duty cycle
}

void PWM_init_heater() { // PWM para calefactor
	DDRD |= (1<<PD5); // Pin PD5 como salida
	TCCR0A |= (1<<COM0B1)|(1<<WGM00)|(1<<WGM01); // Fast PWM
	TCCR0B |= (1<<CS01); // Prescaler 8
	OCR0B = 0;
}

void set_heater_power(uint8_t power) { // Ajusta potencia del calefactor
    if (power > 255) power = 255;
    OCR0B = power;
}

// Menú interactivo 
void show_menu(uint8_t current_pm) {
	UART_print_P(PSTR("\n============================================\n"));
	UART_print_P(PSTR("    Sistema de Control de Temperatura \n"));
	UART_print_P(PSTR("============================================\n"));
	UART_print_P(PSTR("  Para ajustar el Punto Medio (PM):\n"));
	UART_print_P(PSTR("  1. Ingrese un nuevo valor (0-99).\n"));
	UART_print_P(PSTR("  2. Presione ENTER.\n"));
	char buffer[40];
	sprintf(buffer, "  > PM Actual: %dC\n", current_pm);
	UART_print(buffer);
	UART_print_P(PSTR("============================================\n"));
}

int main(void) {
	// Inicializaciones
	UART_init();
	ADC_init();
	PWM_init_fan();
	PWM_init_heater();

	DDRD |= (1<<PD7); // Dirección motor (para la simulación exclusivamente)
	PORTD |= (1<<PD7); // Sentido fijo

	float tempC; // Variable para temperatura
	char buffer[80]; // Buffer para imprimir UART
	uint16_t adc_val;
	uint8_t punto_medio = 27; // Punto medio inicial
	char new_pm_buffer[4] = {0}; // Buffer para ingresar nuevo PM
	uint8_t idx = 0; // Índice de ingreso de dígitos
	char received_char;
	uint8_t heater_state = 0;
	uint8_t fan_state = 0;
	static char comando_buffer[10]; // Buffer de comandos
	static uint8_t comando_idx = 0; // Índice de comando

	show_menu(punto_medio); // Muestra menú inicial

	while (1) {

		for (uint8_t t = 0; t < 20; t++) { // Ciclo de 20 lecturas para UART

			received_char = UART_check_receive(); // Revisa si hay datos UART
			if (received_char != 0) {
				if (received_char != '\r' && received_char != '\n') {
					if (comando_idx < sizeof(comando_buffer) - 1)
					comando_buffer[comando_idx++] = received_char; // Guarda comando

					if (received_char >= '0' && received_char <= '9' && idx < 3)
					new_pm_buffer[idx++] = received_char; // Guarda dígitos PM
					} else {
					comando_buffer[comando_idx] = '\0';
					new_pm_buffer[idx] = '\0';

					if (strcmp(comando_buffer, "Datos") == 0 || strcmp(comando_buffer, "datos") == 0) {
						UART_print_P(PSTR("\n=== DATOS GUARDADOS ===\n"));
						char linea[32];
						for (uint8_t i = 0; i < indice_datos; i++) {
							sprintf(linea, "%.2f,%d,%d,%d\r\n", datos_temp[i], datos_heat[i], datos_fan[i], datos_pm[i]);
							UART_print(linea); // Imprime los datos guardados
						}
						UART_print_P(PSTR("=======================\n"));
					}

					else if (idx > 0) {
						uint8_t nuevo_pm = (uint8_t)atoi(new_pm_buffer); // Convierte a entero
						if (nuevo_pm <= 99) {
							punto_medio = nuevo_pm; // Actualiza PM
							sprintf(buffer, "\n*** PM actualizado a %dC ***\n", punto_medio);
							UART_print(buffer);
							show_menu(punto_medio);
						}
					}

					comando_idx = 0; // Reinicia buffers
					idx = 0;
					comando_buffer[0] = '\0';
					new_pm_buffer[0] = '\0';
				}
			}

			_delay_ms(100);  
		}

		adc_val = ADC_read(0); // Lee el ADC
		tempC = (adc_val * 5.0 / 1023.0) * 100.0; // Convierte a temperatura Celsius

		// Rangos para controlar calefactor y ventilador
		uint8_t lim1 = punto_medio - 8;
		uint8_t lim2 = punto_medio - 1;
		uint8_t lim3 = punto_medio + 10;
		uint8_t lim4 = punto_medio + 20;

		// Control del calefactor y ventilador según temperatura
		if (tempC <= lim1) {
			set_heater_power(254); set_fan_speed(0);
			heater_state = 2; fan_state = 0;
			sprintf(buffer, "T:%.2fC | Calefactor encendido (ALTO)\n", tempC);
		}
		else if (tempC > lim1 && tempC <= lim2) {
			set_heater_power(150); set_fan_speed(0);
			heater_state = 1; fan_state = 0;
			sprintf(buffer, "T:%.2fC | Calefactor encendido (MEDIO)\n", tempC);
		}
		else if (tempC > lim2 && tempC <= punto_medio + 3) {
			set_heater_power(0); set_fan_speed(0);
			heater_state = 0; fan_state = 0;
			sprintf(buffer, "T:%.2fC | Reposo (Punto medio)\n", tempC);
		}
		else if (tempC > punto_medio + 3 && tempC <= lim3) {
			set_heater_power(0); set_fan_speed(150);
			heater_state = 0; fan_state = 1;
			sprintf(buffer, "T:%.2fC | Ventilador encendido (BAJO)\n", tempC);
		}
		else if (tempC > lim3 && tempC <= lim4) {
			set_heater_power(0); set_fan_speed(190);
			heater_state = 0; fan_state = 2;
			sprintf(buffer, "T:%.2fC | Ventilador encendido (MEDIO)\n", tempC);
		}
		else if (tempC > lim4) {
			set_heater_power(0); set_fan_speed(254);
			heater_state = 0; fan_state = 3;
			sprintf(buffer, "T:%.2fC | Ventilador encendido (ALTO)\n", tempC);
		}

		UART_print(buffer);  // Muestra estado actual
		UART_print_P(PSTR("(Ingrese nuevo PM o 'datos' para listar)\n"));

		// Guarda datos en arreglos para gráficar déspues
		if (indice_datos < MAX_DATOS) {
			datos_temp[indice_datos] = tempC;
			datos_heat[indice_datos] = heater_state;
			datos_fan[indice_datos] = fan_state;
			datos_pm[indice_datos] = punto_medio; 
			indice_datos++;
		}
	}
}
