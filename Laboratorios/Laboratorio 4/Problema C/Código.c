#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>

#define NUM_LEDS 256
#define DATA_PIN PD6
#define BAUD 9600
#define MATRIZ_TAMANIO 16
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1)
#define NUM_COLORES_INICIALIZACION 5

typedef struct { 
	uint8_t r,g,b; 
	} Color;
	
Color leds[NUM_LEDS];

// Frames 
const uint8_t frame1[NUM_LEDS] PROGMEM = { 
	#include "frame1.txt" 
	};
	
const uint8_t frame2[NUM_LEDS] PROGMEM = { 
	#include "frame2.txt" 
	};
	
const uint8_t frame3[NUM_LEDS] PROGMEM = { 
	#include "frame3.txt" 
	};
	
const uint8_t frame4[NUM_LEDS] PROGMEM = { 
	#include "frame4.txt" 
	};
	
const uint8_t frame5[NUM_LEDS] PROGMEM = { 
	#include "frame5.txt" 
	};
	
const uint8_t frame6[NUM_LEDS] PROGMEM = { 
	#include "frame6.txt" 
	};
	
const uint8_t frameA[NUM_LEDS] PROGMEM = {
	#include "frameA.txt"
	};
	
const uint8_t frameB[NUM_LEDS] PROGMEM = {
	#include "frameB.txt"
	};
	
const uint8_t frameC[NUM_LEDS] PROGMEM = {
	#include "frameC.txt"
	};
	
const uint8_t frameD[NUM_LEDS] PROGMEM = {
	#include "frameD.txt"
	};
	
const uint8_t frameE[NUM_LEDS] PROGMEM = {
	#include "frameE.txt"
	};
	
const uint8_t frameF[NUM_LEDS] PROGMEM = {
	#include "frameF.txt"
	};

// UART
void UART_send(char c){
	while(!(UCSR0A&(1<<UDRE0)));UDR0=c;
	}
	
void UART_print(char* s){
	while(*s) UART_send(*s++);
	}
	
void UART_print_P(const char *s){
	char c;while((c=pgm_read_byte(s++))) UART_send(c);
	}
	
char UART_check_receive(){
	if(UCSR0A&(1<<RXC0)) 
	return UDR0; 
	return 0;
	}
	
void UART_init(){
	UBRR0H=UBRR_VALUE>>8; 
	UBRR0L=UBRR_VALUE; 
	UCSR0B=(1<<TXEN0)|(1<<RXEN0); 
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);
	}

// WS2812
void ws2812_send(Color *leds,uint16_t n){
	cli();
	for(uint16_t i=0;i<n;i++){
		uint8_t colors[3]={leds[i].g,leds[i].r,leds[i].b};
		for(uint8_t k=0;k<3;k++){
			uint8_t mask=0x80;
			while(mask){
				if(colors[k]&mask){
					PORTD|=(1<<DATA_PIN);
					__asm__ __volatile__("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
					PORTD&=~(1<<DATA_PIN);
					__asm__ __volatile__("nop\nnop\nnop\nnop\nnop\n");
					}else{
					PORTD|=(1<<DATA_PIN);
					__asm__ __volatile__("nop\nnop\nnop\n");
					PORTD&=~(1<<DATA_PIN);
					__asm__ __volatile__("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
				}
				mask>>=1;
			}
		}
	}
	sei();
	for(volatile uint16_t i=0;i<50;i++);
}

uint16_t unmapSerpentine(uint16_t i) {
	if (i >= NUM_LEDS) return 0;

	uint8_t fila = i / MATRIZ_TAMANIO;
	uint8_t columna = i % MATRIZ_TAMANIO;
	uint16_t indice_logico;

	if (fila % 2 != 0) {
		columna = (MATRIZ_TAMANIO - 1) - columna;
	}
	
	indice_logico = (fila * MATRIZ_TAMANIO) + columna;

	return indice_logico;
}

// Mostrar Frame (a cada variable/numero se le asigna un color)
void mostrarFrameColor(const uint8_t *frame){
	uint16_t indice_logico;
	for(uint16_t i=0;i<NUM_LEDS;i++){
		indice_logico = unmapSerpentine(i);
		uint8_t val=pgm_read_byte(&(frame[indice_logico]));
		
		switch(val){
			case 0: leds[i].r=0;   leds[i].g=0;   leds[i].b=0; break;
			case 1: leds[i].r=15;  leds[i].g=15;  leds[i].b=15; break;
			case 2: leds[i].r=30; leds[i].g=30; leds[i].b=30; break;
			case 3: leds[i].r=255; leds[i].g=255; leds[i].b=255; break;
			case 4: leds[i].r=128; leds[i].g=0;   leds[i].b=32; break;
			case 10: leds[i].r=255;   leds[i].g=0;   leds[i].b=0; break;
			case 6: leds[i].r=0;  leds[i].g=0;  leds[i].b=255; break;
			case 7: leds[i].r=0; leds[i].g=150; leds[i].b=255; break;
			case 8: leds[i].r=253; leds[i].g=166; leds[i].b=0; break;
			case 9: leds[i].r=128; leds[i].g=64;   leds[i].b=0; break;
		    case 11: leds[i].r=0; leds[i].g=255;   leds[i].b=0; break;
		}
	}
	ws2812_send(leds, NUM_LEDS);
}

void mostrarColor(uint8_t r, uint8_t g, uint8_t b){
	for(uint16_t i=0;i<NUM_LEDS;i++){
		leds[i].r=r; leds[i].g=g; leds[i].b=b;
	}
	ws2812_send(leds, NUM_LEDS);
}

uint8_t init_colores[NUM_COLORES_INICIALIZACION][3] = {
	{255, 0, 0},     // Rojo
	{0, 255, 0},     // Verde
	{0, 0, 255},     // Azul
	{255, 255, 255}, // Blanco
	{0, 0, 0}        // Apagado
};

uint16_t init_tiempos[NUM_COLORES_INICIALIZACION] = {500, 500, 500, 500, 300}; // ms

// Setup 
void setup(){ 
	DDRD|=(1<<DATA_PIN); UART_init(); 
	}

void show_menu(){
	UART_print_P(PSTR("\n============================================\n"));
	UART_print_P(PSTR("   Animaciones en Matriz RGB \n"));
	UART_print_P(PSTR("============================================\n"));
	UART_print_P(PSTR("  1. Perrito\n"));
	UART_print_P(PSTR("  2. Fantasma\n"));
	UART_print_P(PSTR("  3. Inicializacion\n"));
	UART_print_P(PSTR("  M/m. Visualizar menu.\n"));
	UART_print_P(PSTR("============================================\n"));
}
int main(void){
	setup();
	_delay_ms(1000);
	while(UCSR0A & (1<<RXC0)) (void)UDR0; // limpiar buffer (mejor impresión UART)

	// Variables de control
	uint8_t modo = 3;             // Se comienza con la inicialización
	uint8_t frame_index = 0;      // para animaciones normales
	uint32_t contador = 0;
	uint8_t init_frame_index = 0;
	uint32_t init_contador = 0;

	// Secuencias de animaciones
	const uint8_t *secuencia_perrito[] = {
		frame1, frame2, frame1, frame2, frame1, frame2, frame1, frame2,
		frame3, frame2, frame3, frame2, frame3, frame4, frame5, frame4,
		frame5, frame4, frame4, frame5, frame4, frame3, frame2, frame6
	};
	
	// Tiempo de cada frame de la animación
	const uint16_t duracion_perrito[] = {
		200,200,200,200,200,200,200,200,
		400,400,400,400,400,400,400,400,
		400,400,400,400,400,400,400,800
	};

	const uint8_t *secuencia_fantasma[] = {
		frameA, frameB, frameC, frameD, frameE, frameF, frameE, frameD
	};
	const uint16_t duracion_fantasma[] = {
		250, 250, 250, 250, 250, 250, 250, 250
	};

	show_menu(); // muestra el menú al inicio

	while(1){
		char rx = UART_check_receive();

		// Menú rápido
		if(rx=='m'||rx=='M') show_menu();

		// Selección modo
		if(rx=='1'){
			modo = 1;
			frame_index = 0;
			contador = 0;
			UART_print_P(PSTR("\nAnimacion: Perrito\n"));
			mostrarFrameColor(secuencia_perrito[frame_index]); // muestra primer frame 
		}
		if(rx=='2'){
			modo = 2;
			frame_index = 0;
			contador = 0;
			UART_print_P(PSTR("\nAnimacion: Fantasma\n"));
			mostrarFrameColor(secuencia_fantasma[frame_index]); 
		}
		if(rx=='3'){
			modo = 3;
			init_frame_index = 0;
			init_contador = 0;
		}

		// Ejecutar animaciones
		if(modo==1){ // Perrito
			contador++;
			if(contador >= duracion_perrito[frame_index]){
				contador = 0;
				mostrarFrameColor(secuencia_perrito[frame_index]);
				frame_index++;
				if(frame_index >= (sizeof(secuencia_perrito)/sizeof(secuencia_perrito[0]))) frame_index = 0;
			}
		}
		else if(modo==2){ // Fantasma
			contador++;
			if(contador >= duracion_fantasma[frame_index]){
				contador = 0;
				mostrarFrameColor(secuencia_fantasma[frame_index]);
				frame_index++;
				if(frame_index >= (sizeof(secuencia_fantasma)/sizeof(secuencia_fantasma[0]))) frame_index = 0;
			}
		}
		else if(modo==3){ // Inicialización no bloqueante
			// Al iniciar, mostrar mensaje solo en el primer frame
			if(init_frame_index == 0 && init_contador == 0){
				UART_print_P(PSTR("\nInicializacion de matriz LED RGB.\n"));
			}

			init_contador++;
			if(init_contador >= init_tiempos[init_frame_index]){
				init_contador = 0;
				mostrarColor(init_colores[init_frame_index][0],
				init_colores[init_frame_index][1],
				init_colores[init_frame_index][2]);
				init_frame_index++;

				if(init_frame_index >= NUM_COLORES_INICIALIZACION){
					UART_print_P(PSTR("Inicializacion correcta.\n")); // mensaje al final
					modo = 0; 
				}
			}
		}
		_delay_ms(1); // avanza contador 
	}
}
