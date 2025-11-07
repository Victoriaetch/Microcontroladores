// --- DEFINICIONES GLOBALES Y CONFIGURACIÓN ---
#define F_CPU 16000000UL

// --- LIBRERÍAS ESTÁNDAR DE AVR ---
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#include <avr/eeprom.h>
#include <string.h>
#include <avr/interrupt.h>  

// --- LIBRERÍAS PERSONALIZADAS (DRIVERS) ---
// (Estos archivos .h estan en la carpeta del problema)
#include "UART.h"     // Funciones para comunicación serie
#include "SPI.h"      // Funciones para comunicación SPI
#include "RC522.h"    // Funciones específicas para el lector RFID MFRC522
#include "TWI.h"      // Funciones para comunicación I2C
#include "i2c_lcd.h"  // Funciones para controlar el LCD por I2C

// --- CONSTANTES DE CONFIGURACIÓN ---
#define BAUD 9600
#define MY_UBRR F_CPU/16/BAUD-1

#define LCD_BACKLIGHT_ON  1   // Constante para encender el backlight del LCD
#define LCD_BACKLIGHT_OFF 0   // Constante para apagar el backlight del LCD

// --- MAPEADO DE PINES ---
#define LED_VERDE PD6
#define LED_ROJO PD7
#define BTN_BORRAR PD2
#define BTN_UPDATE PD3   

#define UID_LEN 10         // Define la longitud del UID que se va a almacenar

// --- VARIABLE EN EEPROM ---
// Declara un array de 10 bytes que se ubicará en la memoria EEPROM
// Esta variable persistirá aunque se apague el microcontrolador
uint8_t eeprom_uid[UID_LEN] EEMEM;

// --- PROTOTIPOS DE FUNCIONES AUXILIARES ---
void leds_init (void);
void botones_init(void);
void interrupciones_init(void);

// Funciones de ayuda para la EEPROM
void read_stored_uid(uint8_t *buf);
void write_stored_uid(const uint8_t *buf);
void clear_stored_uid(void);

// Funciones de lógica y UI
uint8_t uid_vacio (const uint8_t *buf);
void display_and_log(const char *l1, const char *l2, const char *msg);

// Funciones de manejo de estado (activadas por botones)
void handle_registration(void);
void handle_erase(void) ;

// --- IMPLEMENTACIONES DE FUNCIONES AUXILIARES ---

/**
 *  Configura los pines de los LEDs (Verde y Rojo) como salidas y los apaga.
 */
void leds_init (void) {
	DDRD |= (1<<LED_VERDE) | (1<<LED_ROJO);
	PORTD &= ~((1<<LED_VERDE) | (1<<LED_ROJO));
}

/**
 * Configura los pines de los botones (Borrar y Update) como entradas
 * y activa las resistencias de pull-up internas.
 */
void botones_init(void) {
	DDRD &= ~((1<<BTN_BORRAR) | (1<<BTN_UPDATE));
	PORTD |= (1<<BTN_BORRAR) | (1<<BTN_UPDATE); // pull-ups internos
}


/**
 * Configura las interrupciones externas INT0 (PD2) e INT1 (PD3).
 */
void interrupciones_init(void) {
	// Configura INT0 e INT1 para flanco descendente (cuando se presiona el botón)
	EICRA |= (1 << ISC01) | (1 << ISC11);
	EICRA &= ~((1 << ISC00) | (1 << ISC10));
	
	// Habilita interrupciones INT0 e INT1
	EIMSK |= (1 << INT0) | (1 << INT1);
	
	// Habilita interrupciones globales
	sei();
}


/**
 *  Lee el bloque de UID (UID_LEN bytes) desde la EEPROM.
 *  buf = Puntero a un buffer en RAM donde se guardarán los datos.
 */
void read_stored_uid(uint8_t *buf) {
	eeprom_read_block((void*)buf, (const void*)eeprom_uid, UID_LEN);
}

/**
 * Escribe (actualiza) un bloque de UID en la EEPROM.
 * Nota: eeprom_update_block() es inteligente; solo escribe si los datos
 * nuevos son diferentes a los existentes, ahorrando ciclos de escritura.
 * buf = Puntero al buffer en RAM que contiene los datos a escribir.
 */
void write_stored_uid(const uint8_t *buf) {
	eeprom_update_block((const void*)buf, (void*)eeprom_uid, UID_LEN);
}


/**
 * Borra el UID almacenado en la EEPROM.
 * Lo hace escribiendo 0xFF en todas las posiciones.
 */
void clear_stored_uid(void) {
	uint8_t clear[UID_LEN];
	for (uint8_t i = 0; i < UID_LEN; i++) 
	clear[i] = 0xFF;
	write_stored_uid(clear);
}

/**
 * Comprueba si un buffer de UID está "vacío".
 * Un UID se considera vacío si todos sus bytes son 0xFF (borrado)
 * o 0x00 (valor por defecto si nunca se escribió).
 * buf = Puntero al buffer de UID a comprobar.
 * return 1 (true) si está vacío, 0 (false) si no lo está.
 */
uint8_t uid_vacio(const uint8_t *buf) {
for (uint8_t i = 0; i < UID_LEN; i++) {
		if (buf[i] != 0xFF && buf[i] != 0x00) 
			return 0;
	}
	return 1;
}

/**
 * Función de ayuda para mostrar mensajes en el LCD (2 líneas)
 * y opcionalmente enviar un mensaje de log por UART.
 * l1 = Mensaje para la línea 1 del LCD.
 * l2 = Mensaje para la línea 2 del LCD.
 * msg = Mensaje para el log UART (si es NULL, no se envía nada).
 */
void display_and_log(const char *l1, const char *l2, const char *msg) { 
	twi_lcd_clear();
	twi_lcd_cmd(0x00); // Cursor fila 0, col 0
	twi_lcd_msg(l1);
	twi_lcd_cmd(0xC0); // Cursor fila 1, col 0 
	twi_lcd_msg(l2);
	if (msg) {
		uart_print(msg);
		uart_print("\r\n");
	}
}

/**
 * Maneja el proceso de registro de una nueva tarjeta maestra.
 * Se llama desde el bucle principal cuando se detecta el flag 'flag_actualizar'.
 */
void handle_registration(void) {
	display_and_log("Modo registro", "Acerque tarjeta", "Modo registro");
	uint8_t card_uid[UID_LEN] = {0};
	uint8_t nueva = 0;
	uint32_t timeout_ms = 0;
	const uint32_t max_timeout_ms = 10000; // 10 s

	while (timeout_ms < max_timeout_ms) {
		memset(card_uid, 0, UID_LEN);
		mfrc522_standard(card_uid); // llama SPI OK desde main
		if (card_uid[0] != 0) {
			write_stored_uid(card_uid);
			uart_print("Nueva tarjeta guardada: ");
			uart_print_hex_array(card_uid, UID_LEN);
			display_and_log("Nueva tarjeta","registrada","Tarjeta registrada");
			nueva = 1;
			break;
		}
		_delay_ms(200);
		timeout_ms += 200;
	}

	if (!nueva) {
		display_and_log("Registro","Cancelado","Registro: timeout");
	}
	_delay_ms(800);
	display_and_log("Acerque tarjeta","para acceso", NULL);
}

/**
 * Maneja el proceso de borrado de la tarjeta maestra.
 * Se llama desde el bucle principal cuando se detecta el flag 'flag_borrar'.
 */
void handle_erase(void) {
	clear_stored_uid();
	display_and_log("Tarjeta", "borrada", "Tarjeta borrada");
	PORTD |= (1<<LED_ROJO);
	_delay_ms(5000);
	PORTD &= ~(1<<LED_ROJO);
	display_and_log("Acerque tarjeta","para acceso","");
}

// --- FUNCIÓN PRINCIPAL ---
int main(void) {
	// --- SECCIÓN DE INICIALIZACIÓN (Setup) ---
	uart_init(MY_UBRR);	
	spi_init();
	leds_init();
	botones_init();
	interrupciones_init();   
	twi_lcd_init(LCD_BACKLIGHT_ON); 
	mfrc522_resetPinInit();
	_delay_ms(100); 
	mfrc522_init();

	// --- Mensajes de bienvenida ---
	uart_print("Sistema Cerradura RFID iniciado\r\n");
	display_and_log("Bienvenido al", "sistema RFID", "Bienvenido al sistema RFID"); //REVISAR
	_delay_ms(3000);

	// --- Preparación del bucle ---
	uint8_t stored_uid[UID_LEN];
	uint8_t card_uid[UID_LEN] = {0};
	read_stored_uid(stored_uid);

    display_and_log("Acerque tarjeta", "para acceder", NULL);

	// --- BUCLE PRINCIPAL ---
    while (1) 
    {
        // --- 1. Atención a los flags de interrupción ---
		// Las ISR (ver al final) solo ponen estos flags a 1.
		// El trabajo pesado se hace aquí en el main loop para no bloquear las interrupciones.

		// Si se presionó el botón ACTUALIZAR...	
		if (flag_actualizar) {
	    flag_actualizar = 0;
	    handle_registration();
		read_stored_uid(stored_uid); // actualizar copia en RAM
	}
		// Si se presionó el botón BORRAR...
	    if (flag_borrar) {
		flag_borrar = 0;
		handle_erase();
		read_stored_uid(stored_uid);
	}

// --- 2. Lógica normal de lectura ---
	memset(card_uid,0,sizeof(card_uid));
	mfrc522_standard(card_uid);

	if (card_uid[0]!=0) {
		uart_print("Tarjeta detectada: ");
		uart_print_hex_array(card_uid, UID_LEN);
		_delay_ms(1500);

		// --- 3. Lógica de decisión ---
			
		// Caso 1: No hay ninguna tarjeta registrada en el sistema
		if (uid_vacio(stored_uid)) {
			display_and_log("Sin tarjeta","registrada","No hay tarjeta registrada");
			PORTD |= (1<<LED_ROJO);
			_delay_ms(1500);
			PORTD &= ~(1<<LED_ROJO);
		}

		// Caso 2: La tarjeta leída (card_uid) COINCIDE con la maestra (stored_uid)
		else if (memcmp(card_uid, stored_uid, UID_LEN) == 0) {
			  PORTD |= (1<<LED_VERDE);
			  display_and_log("Acceso","permitido!","Acceso permitido!!");
			  _delay_ms(5000);
			  PORTD &= ~(1<<LED_VERDE);

			  } 
		// Caso 3: La tarjeta leída NO COINCIDE con la maestra
		else {
			  PORTD |= (1<<LED_ROJO);
			  display_and_log("Acceso","denegado!","Acceso denegado");
			  _delay_ms(5000);
			  PORTD &= ~(1<<LED_ROJO);
		  }

        // --- 4. Esperar a que la tarjeta se retire ---
		// Este bucle evita que la misma tarjeta sea leída múltiples veces
		// si se deja sobre el lector.		  
		while (1) {
			  memset(card_uid, 0, UID_LEN);
			  mfrc522_standard(card_uid);
			  if (card_uid[0] == 0) break;
			  _delay_ms(500);
		  }
	  }

	  _delay_ms(1000); // pausa entre lecturas normal
		  display_and_log("Acerque tarjeta", "para acceder", NULL);

  }
}

// --- RUTINAS DE SERVICIO DE INTERRUPCIÓN (ISR) ---

/**
 * ISR (Rutina de Servicio de Interrupción) para INT0 (Pin PD2).
 * Se ejecuta cuando se presiona el botón BTN_BORRAR.
 */
ISR(INT0_vect) { // BORRAR
	// Simple debounce y set flag
	_delay_ms(200);
	if (!(PIND & (1<<BTN_BORRAR))) { // SI sigue presionado
		flag_borrar = 1;
	}
}

/**
 * ISR para INT1 (Pin PD3).
 * Se ejecuta cuando se presiona el botón BTN_UPDATE.
 */
ISR(INT1_vect) { // ACTUALIZAR (entrar en modo registro desde el main)
	_delay_ms(200);
	if (!(PIND & (1<<BTN_UPDATE))) {
		flag_actualizar = 1;
	}
}
