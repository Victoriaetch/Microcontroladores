#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay_basic.h>
#include <util/delay.h>

#define STEP_X   PB3
#define DIR_X    PB4
#define ENABLE_X PB5

#define STEP_Y   PC3
#define DIR_Y    PC4
#define ENABLE_Y PC5

#define PEN_PIN  PC0

#define PASOS_POR_CM 100
#define DIAG_CORR 1.1

static inline void custom_delay(uint16_t cycles) { _delay_loop_2(cycles); }

void setup_plotter(void) {
	DDRB |= (1 << STEP_X) | (1 << DIR_X) | (1 << ENABLE_X);
	DDRC |= (1 << STEP_Y) | (1 << DIR_Y) | (1 << ENABLE_Y) | (1 << PEN_PIN);
	PORTB |= (1 << ENABLE_X);
	PORTC |= (1 << ENABLE_Y);
	PORTC |= (1 << PEN_PIN);
}

void move_motor(volatile uint8_t *port_dir, uint8_t dir_pin,
volatile uint8_t *port_step, uint8_t step_pin,
uint8_t direction, uint16_t steps_count) {
	if (direction) *port_dir |= (1 << dir_pin);
	else *port_dir &= ~(1 << dir_pin);
	custom_delay(50);
	for (uint16_t i = 0; i < steps_count; i++) {
		*port_step |= (1 << step_pin);
		custom_delay(300);
		*port_step &= ~(1 << step_pin);
		custom_delay(300);
	}
}

void move_axis(uint8_t axis, uint8_t direction, uint16_t steps) {
	if (axis == 0) move_motor(&PORTB, DIR_X, &PORTB, STEP_X, direction, steps);
	else move_motor(&PORTC, DIR_Y, &PORTC, STEP_Y, direction, steps);
}

void lower_pen(void) { PORTC &= ~(1 << PEN_PIN); _delay_ms(100); }
void lift_pen(void)  { PORTC |= (1 << PEN_PIN); _delay_ms(100); }

// -------------------- MOVIMIENTOS BÁSICOS --------------------
void mover_derecha(float cm)  { move_axis(0, 1, (uint16_t)(cm * PASOS_POR_CM)); }
void mover_izquierda(float cm){ move_axis(0, 0, (uint16_t)(cm * PASOS_POR_CM)); }
void mover_arriba(float cm)   { move_axis(1, 1, (uint16_t)(cm * PASOS_POR_CM)); }
void mover_abajo(float cm)    { move_axis(1, 0, (uint16_t)(cm * PASOS_POR_CM)); }

// -------------------- DIAGONALES --------------------
void mover_diag_arriba_der(float cm) {
	uint16_t pasos = (uint16_t)(cm * PASOS_POR_CM * DIAG_CORR);
	for(uint16_t i=0;i<pasos;i++){ move_axis(0,1,1); move_axis(1,1,1); }
}
void mover_diag_arriba_izq(float cm) {
	uint16_t pasos = (uint16_t)(cm * PASOS_POR_CM * DIAG_CORR);
	for(uint16_t i=0;i<pasos;i++){ move_axis(0,0,1); move_axis(1,1,1); }
}
void mover_diag_abajo_der(float cm) {
	uint16_t pasos = (uint16_t)(cm * PASOS_POR_CM * DIAG_CORR);
	for(uint16_t i=0;i<pasos;i++){ move_axis(0,1,1); move_axis(1,0,1); }
}
void mover_diag_abajo_izq(float cm) {
	uint16_t pasos = (uint16_t)(cm * PASOS_POR_CM * DIAG_CORR);
	for(uint16_t i=0;i<pasos;i++){ move_axis(0,0,1); move_axis(1,0,1); }
}

// -------------------- FIGURAS GEOMÉTRICAS --------------------

void dibujar_triangulo(float lado){
	lower_pen();
	for(int i=0;i<1;i++){
		mover_derecha(lado);
		mover_diag_arriba_izq(lado);
		mover_abajo(lado);
	}
	
	lift_pen();
}

void dibujar_cruz(float size){
	lower_pen();
	mover_arriba(size); 
	mover_abajo(size/2); 
	mover_izquierda(size/2); 
	mover_derecha(size); 
	lift_pen();
}

void dibujar_circulo(float radio){
	lower_pen();
	for(int i=0;i<360;i++){
	mover_diag_arriba_der(0.01745*radio);
	mover_diag_arriba_izq(0.01745*radio);
}
  lift_pen();
}

// -------------------- DIBUJOS --------------------
void dibujar_gato(void) {
	lower_pen();

	// CUERPO Y COLA
	mover_izquierda(3);
	mover_diag_arriba_izq(2);
	mover_arriba(6.5);
	mover_diag_arriba_der(3.5);
	mover_derecha(1);
	mover_diag_abajo_der(1);
	mover_abajo(1.5);
	mover_izquierda(1.5);
	mover_arriba(1);
	mover_izquierda(0.5);
	mover_diag_abajo_izq(2);
	mover_abajo(5);
	mover_diag_abajo_der(2);
	mover_derecha(2);
	mover_abajo(2);

	mover_arriba(5);
	mover_diag_arriba_der(1);
	mover_arriba(1);
	mover_diag_arriba_der(2);
	mover_diag_arriba_izq(2);
	mover_arriba(4);
	mover_diag_arriba_der(1);
	mover_arriba(4);
	mover_diag_abajo_der(3);
	mover_diag_arriba_der(1);
	mover_derecha(2);
	mover_diag_abajo_der(1);
	mover_diag_arriba_der(3);
	mover_abajo(4);
	mover_diag_abajo_der(1);
	mover_abajo(4);
	mover_diag_abajo_izq(2);
	mover_diag_abajo_der(2);
	mover_abajo(1);
	mover_diag_abajo_der(1);
	mover_abajo(6);
	mover_diag_abajo_izq(1);
	mover_arriba(1);
	mover_abajo(1);
	mover_izquierda(1);
	mover_arriba(1);
	mover_abajo(1);
	mover_izquierda(1);
	mover_arriba(1);
	mover_abajo(1);
	mover_diag_arriba_izq(1);
	mover_izquierda(3.7);
	mover_diag_abajo_izq(1);
	mover_arriba(1);
	mover_abajo(1);
	mover_izquierda(1);
	mover_arriba(1);
	mover_abajo(1);
	mover_izquierda(1);
	mover_arriba(1);
	mover_abajo(1);
	mover_diag_arriba_izq(1);
	mover_arriba(1);
	mover_diag_arriba_der(1);
	mover_arriba(5);

	// PATITAS
	lift_pen();
	mover_derecha(2);
	lower_pen();
	mover_abajo(5);
	mover_diag_abajo_der(1);
	mover_abajo(1);
	mover_derecha(3.7);
	mover_arriba(1);
	mover_diag_arriba_der(1);
	mover_arriba(5);

	lift_pen();
	mover_derecha(2);
	lower_pen();
	mover_abajo(5);
	mover_diag_abajo_der(1);

	// BOCA
	lift_pen();
	mover_arriba(10.5);
	mover_izquierda(3.5);
	lower_pen();
	mover_diag_abajo_izq(1);
	mover_izquierda(2);
	mover_diag_arriba_izq(1);

	// NARIZ
	lift_pen();
	mover_derecha(1);
	lower_pen();
	mover_derecha(1);
	mover_arriba(1);
	mover_derecha(3);
	mover_izquierda(2);
	mover_diag_abajo_der(2);
	mover_diag_arriba_izq(2);
	mover_arriba(1);
	mover_derecha(2);
	mover_izquierda(7);
	mover_derecha(2);
	mover_abajo(1);
	mover_izquierda(2);
	mover_derecha(2);
	mover_diag_abajo_izq(2);
	mover_diag_arriba_der(2);
	mover_derecha(1);
	mover_abajo(1);

	// OJOS
	lift_pen();
	mover_arriba(3);
	mover_izquierda(0.5);
	lower_pen();
	mover_izquierda(1);
	mover_arriba(1);
	mover_derecha(1);
	mover_abajo(1);
	mover_izquierda(2);
	mover_arriba(2);
	mover_derecha(2);
	mover_abajo(2);

	lift_pen();
	mover_derecha(2);
	lower_pen();
	mover_arriba(2);
	mover_derecha(2);
	mover_abajo(1);
	mover_izquierda(1);
	mover_abajo(1);
	mover_izquierda(1);
	mover_derecha(2);
	mover_arriba(1);

	lift_pen();
}

void dibujar_rana(void) {
	lower_pen();

	// Cabeza y parte superior
	mover_diag_arriba_der(0.5);
	mover_arriba(1.5);
	mover_diag_arriba_der(0.5);
	mover_diag_abajo_der(0.5);
	mover_arriba(1.5);
	mover_diag_arriba_der(0.5);
	mover_diag_arriba_izq(0.5);
	mover_arriba(1);
	mover_diag_arriba_der(0.5);
	mover_diag_arriba_der(0.5);
	mover_diag_abajo_der(0.5);
	mover_diag_abajo_izq(0.5);
	mover_diag_arriba_izq(0.5);

	// Ojo izquierdo
	lift_pen();
	mover_derecha(0.7);
	lower_pen();
	mover_derecha(1);
	mover_diag_arriba_der(0.5);
	mover_diag_abajo_der(0.5);
	mover_diag_abajo_izq(0.5);
	mover_diag_arriba_izq(0.5);

	// Ojo derecho
	lift_pen();
	mover_derecha(0.7);
	lower_pen();
	mover_diag_abajo_der(0.5);
	mover_abajo(1);
	mover_diag_abajo_izq(0.5);
	mover_diag_abajo_der(0.5);
	mover_abajo(1.5);
	mover_diag_arriba_der(0.5);
	mover_diag_abajo_der(0.5);
	mover_abajo(1.5);
	mover_diag_abajo_der(0.5);
	mover_izquierda(1);
	mover_arriba(2);
	mover_abajo(2);
	mover_diag_abajo_der(0.5);
	mover_izquierda(1.5);
	mover_diag_arriba_der(0.5);
	mover_arriba(2);
	mover_abajo(2);
	mover_izquierda(1.5);
	mover_arriba(2);
	mover_abajo(2);
	mover_diag_abajo_der(0.5);
	mover_izquierda(1.6);
	mover_diag_arriba_der(0.5);
	mover_izquierda(1);
	mover_derecha(1);
	mover_arriba(2);

	// Boca
	lift_pen();
	mover_arriba(1.7);
	mover_derecha(1.5);
	lower_pen();
	mover_derecha(2.5);

	lift_pen();
	mover_derecha(0.3);
	mover_arriba(0.3);
	lower_pen();
	mover_izquierda(3);

	lift_pen();
}

// -------------------- MAIN --------------------
int main(void){
	setup_plotter();
	_delay_ms(1000);

	while(1){
		dibujar_gato();  
	  lift_pen();
	  _delay_ms(2000);
		dibujar_rana(); 
		lift_pen(); 
    _delay_ms(2000);
		dibujar_triangulo(5); 
    _delay_ms(2000);
		dibujar_cruz(5);     
    _delay_ms(2000);
		dibujar_circulo(3); 
    _delay_ms(2000);
	}
}
