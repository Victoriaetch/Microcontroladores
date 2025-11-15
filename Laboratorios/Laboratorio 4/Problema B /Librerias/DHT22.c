#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "DHT22.h"

#define DHT_PORT PORTD
#define DHT_DDR  DDRD
#define DHT_PIN  PIND
#define DHT_DATA_PIN PD2

void dht22_init(void) {
	DHT_DDR |= (1 << DHT_DATA_PIN);
	DHT_PORT |= (1 << DHT_DATA_PIN); // pull-up
}

static uint8_t dht_read_bit(void) {
	uint16_t count = 0;

	while (!(DHT_PIN & (1 << DHT_DATA_PIN))) {
		if (++count > 1000) return 0;
		_delay_us(1);
	}

	_delay_us(35); // antes 41

	uint8_t bit = (DHT_PIN & (1 << DHT_DATA_PIN)) ? 1 : 0;

	count = 0;
	while (DHT_PIN & (1 << DHT_DATA_PIN)) {
		if (++count > 1000) break;
		_delay_us(1); // antes 10
	}

	return bit;
}

bool dht22_read(int16_t *temp_x10, uint16_t *hum_x10) {
	uint8_t data[5] = {0};

	DHT_DDR |= (1 << DHT_DATA_PIN);
	DHT_PORT &= ~(1 << DHT_DATA_PIN);
	_delay_ms(20);
	DHT_PORT |= (1 << DHT_DATA_PIN);
	_delay_us(30);
	DHT_DDR &= ~(1 << DHT_DATA_PIN);
	DHT_PORT |= (1 << DHT_DATA_PIN);

	uint16_t count = 0;
	while (DHT_PIN & (1 << DHT_DATA_PIN)) { if (++count > 10000) return false; _delay_us(1); }
	count = 0;
	while (!(DHT_PIN & (1 << DHT_DATA_PIN))) { if (++count > 10000) return false; _delay_us(1); }
	count = 0;
	while (DHT_PIN & (1 << DHT_DATA_PIN)) { if (++count > 10000) return false; _delay_us(1); }

	for (uint8_t i = 0; i < 5; i++) {
		for (uint8_t j = 0; j < 8; j++) {
			data[i] <<= 1;
			data[i] |= dht_read_bit();
		}
	}

	uint8_t chk = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	if (chk != data[4]) return false;

	*hum_x10  = data[0] * 10;
	*temp_x10 = data[2] * 10;

	return true;
}
