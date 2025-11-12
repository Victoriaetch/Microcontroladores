#include "config.h" // AÑADIDO: Para F_CPU
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "DHT22.h"

// --- Configuración de pines ---
// Según tu PDF: "Sensor DHT22" -> "PD2"
#define DHT_PORT PORTD
#define DHT_DDR  DDRD
#define DHT_PIN  PIND
#define DHT_DATA_PIN PD2 // <-- Corregido al pin PD2

void dht22_init(void) {
	// Línea en alto por defecto
	DHT_DDR |= (1 << DHT_DATA_PIN);
	DHT_PORT |= (1 << DHT_DATA_PIN);
}

// Función interna para leer un bit
static uint8_t dht_read_bit(uint16_t *timeout_counter) {
	while (!(DHT_PIN & (1 << DHT_DATA_PIN))) { // Espera inicio de bit (HIGH)
		_delay_us(1);
		if (++(*timeout_counter) > 500) return 0; // 500us timeout
	}
	_delay_us(30); // Esperar 30us
	
	if (DHT_PIN & (1 << DHT_DATA_PIN)) { // Si sigue en HIGH, es un '1'
		return 1;
		} else {
		return 0; // Si bajó, es un '0'
	}
}

bool dht22_read(int16_t *temp_x10, uint16_t *hum_x10) {
	uint8_t data[5] = {0};
	uint16_t timeout = 0;

	// --- 1. Señal de inicio (Start) ---
	DHT_DDR |= (1 << DHT_DATA_PIN);  // Pin como salida
	DHT_PORT &= ~(1 << DHT_DATA_PIN); // Pin en BAJO
	_delay_ms(20);                  // Mantener bajo por 20ms
	
	DHT_PORT |= (1 << DHT_DATA_PIN);  // Pin en ALTO
	_delay_us(30);                  // Mantener alto por 30us
	
	DHT_DDR &= ~(1 << DHT_DATA_PIN);  // Pin como entrada
	DHT_PORT &= ~(1 << DHT_DATA_PIN); // Desactivar pull-up

	// --- 2. Esperar respuesta del sensor ---
	timeout = 0;
	while (DHT_PIN & (1 << DHT_DATA_PIN)) {   // Espera que DHT baje el pin
		_delay_us(1);
		if (++timeout > 200) return false; // Falla: Sin respuesta
	}
	timeout = 0;
	while (!(DHT_PIN & (1 << DHT_DATA_PIN))) { // Espera que DHT suba el pin
		_delay_us(1);
		if (++timeout > 200) return false; // Falla: Sin respuesta
	}
	timeout = 0;
	while (DHT_PIN & (1 << DHT_DATA_PIN)) {   // Espera que DHT baje el pin (fin de la respuesta)
		_delay_us(1);
		if (++timeout > 200) return false; // Falla: Sin respuesta
	}

	// --- 3. Leer los 40 bits de datos ---
	for (uint8_t i = 0; i < 5; i++) { // Para cada uno de los 5 bytes
		for (uint8_t j = 0; j < 8; j++) { // Para cada uno de los 8 bits
			timeout = 0;
			data[i] <<= 1; // Mover bits a la izquierda
			if (dht_read_bit(&timeout)) {
				data[i] |= 1;
			}
			if (timeout > 0) return false; // Timeout durante la lectura de bits
			
			// Esperar a que el pin baje (fin del bit)
			while(DHT_PIN & (1 << DHT_DATA_PIN)) {
				_delay_us(1);
				if(++timeout > 500) return false;
			}
		}
	}
	
	// --- 4. Verificar Checksum ---
	uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	if (checksum != data[4]) {
		return false; // Falla: Checksum no coincide
	}

	// --- 5. Decodificar datos ---
	*hum_x10 = (data[0] << 8) | data[1];
	*temp_x10 = (data[2] << 8) | data[3];

	// Manejar temperaturas negativas (Bit 15 es el signo)
	if (*temp_x10 & 0x8000) {
		*temp_x10 = -(*temp_x10 & 0x7FFF);
	}

	return true; // Éxito
}