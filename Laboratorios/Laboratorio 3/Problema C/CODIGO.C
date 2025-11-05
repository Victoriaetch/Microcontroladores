#define F_CPU 16000000UL // Frecuencia del cristal (16MHz)
#define BAUD 9600
#define UBRR_VAL ((F_CPU / 16 / BAUD) - 1)
#define DEADBAND 10 // Zona muerta para evitar vibraciones
#define KP 0.5      // Ganancia Proporcional

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>      // Para sprintf
#include <stdlib.h>     // Para abs()

// --- Prototipos de Funciones ---
void uart_init(unsigned int ubrr);
void uart_transmit_string(char* str);
void adc_init(void);
uint16_t adc_read(uint8_t channel);
void pwm_init(void);
void motor_control(int direction, uint8_t speed);

// --- Función de Transmisión UART (para printf) ---
static int uart_putchar_stream(char c, FILE *stream);
static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar_stream, NULL, _FDEV_SETUP_WRITE);

static int uart_putchar_stream(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar_stream('\r', stream);
    }
    // Espera a que el buffer de transmisión esté vacío
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c; // Envía el carácter
    return 0;
}

// --- Función Principal ---
int main(void) {
    uint16_t ref_val = 0;
    uint16_t actual_val = 0;
    int error = 0;
    int pwm_calc = 0;
    uint8_t pwm_out = 0;
    int direction = 0; // 0=Stop, 1=Horario, -1=Antihorario

    // Inicialización
    uart_init(UBRR_VAL);
    adc_init();
    pwm_init();
    
    // Configura pines de dirección del motor (PD2, PD3) como salida
    DDRD |= (1 << DDD2) | (1 << DDD3);

    // Redirige stdout a la función de transmisión UART
    stdout = &uart_output;

    while (1) {
        // Leer Potenciómetros
        ref_val = adc_read(0);    // Referencia (0-1023)
        actual_val = adc_read(1); // Actual (0-1023)

        // Calcular Error (Control Proporcional)
        error = (int)ref_val - (int)actual_val;

        // Aplicar Zona Muerta
        if (abs(error) < DEADBAND) {
            // Detener
            direction = 0;
            pwm_out = 0;
        } else {
            // Calcular PWM basado en el error (Control P)
            pwm_calc = (int)(abs(error) * KP);

            // Saturar (limitar) el valor PWM a 255
            if (pwm_calc > 255) {
                pwm_calc = 255;
            }
            pwm_out = (uint8_t)pwm_calc;

            // Determinar Dirección
            if (error > 0) {
                direction = 1; // Mover en sentido horario
            } else {
                direction = -1; // Mover en sentido antihorario
            }
        }

        // Aplicar control al motor
        motor_control(direction, pwm_out);

        // Informar por Puerto Serial
        // Formato: Ref,Actual,PWM,Dir
        printf("%u,%u,%u,%d\n", ref_val, actual_val, pwm_out, direction);

        // Retardo para estabilizar el loop y la comunicación
        _delay_ms(50); // 20 actualizaciones por segundo
    }
}

// --- Implementación de Funciones ---
void uart_init(unsigned int ubrr) {
    // Configurar baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    // Habilitar transmisor
    UCSR0B = (1 << TXEN0);
    // Configurar formato: 8 bits de datos, 1 bit de stop
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void adc_init(void) {
    // Referencia AVcc (5V)
    ADMUX = (1 << REFS0);
    // Habilitar ADC y Prescaler de 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel) {
    // Seleccionar canal (0-5)
    channel &= 0x07; // Asegurarse que el canal esté entre 0 y 7
    ADMUX = (ADMUX & 0xF8) | channel;

    // Iniciar conversión
    ADCSRA |= (1 << ADSC);

    // Esperar a que termine la conversión
    while (ADCSRA & (1 << ADSC));

    return (ADC); // Retorna el valor de 10 bits
}

void pwm_init(void) {
    // Configurar PD6 (OC0A) como salida
    DDRD |= (1 << DDD6);
    // Modo Fast PWM de 8 bits (WGM01, WGM00)
    // Salida no-invertida en OC0A (COM0A1)
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    // Prescaler de 64 (CS01, CS00)
    // F_PWM = 16MHz / (64 * 256) = ~976 Hz
    TCCR0B = (1 << CS01) | (1 << CS00);  
    // Iniciar PWM con 0% de ciclo de trabajo
    OCR0A = 0;
}

void motor_control(int direction, uint8_t speed) {
    switch (direction) {
        case 1: // Horario
            PORTD |= (1 << PD2);  // IN1 = HIGH
            PORTD &= ~(1 << PD3); // IN2 = LOW
            break;
        case -1: // Antihorario
            PORTD &= ~(1 << PD2); // IN1 = LOW
            PORTD |= (1 << PD3);  // IN2 = HIGH
            break;
        default: // Stop (Freno)
            PORTD &= ~(1 << PD2); // IN1 = LOW
            PORTD &= ~(1 << PD3); // IN2 = LOW
            break;
    }
    // Aplicar velocidad (Duty Cycle)
    OCR0A = speed;
}
