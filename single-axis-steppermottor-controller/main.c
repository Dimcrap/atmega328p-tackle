#include <stdint.h>
#define F_CPU 16000000UL
#define BAUD 115200
#define UBRR_VAL ((F_CPU / (8UL * BAUD)) - 1 )

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <util/delay.h>

#define STEP_PIN    PD4
#define DIR_PIN     PD5
#define LIMIT_PIN   PD2 // INT0

#define EE_STEPS_MM ((float *) 0 ) //eeprom address


static volatile int32_t current_pos = 0 ;
static volatile int32_t target_pos  = 0 ;
static volatile int8_t  moving      = 0 ;
static volatile int8_t limit_hit    = 0 ;
static volatile uint8_t homing      = 0 ;

static float steps_per_mm = 25.0 ;//default 200 stpes/rev/ 8mm pitch
static float feedrate = 300.0 ;
static float rapid_rate = 1000.0 ;

static void uart_init(void)
{
    UBRR0H = (uint8_t)(UBRR_VAL >> 8 );
    UBRR0L = (int8_t)(UBRR_VAL);
    UCSR0A = (1 << U2X0 ) ;
    UCSR0B = ( 1 << TXEN0 ) | ( 1 << RXEN0 );
    UCSR0C = ( 1 << UCSZ01 ) | ( 1 << UCSZ00 );
}

static void uart_putc( char c )
{
    while( !(UCSR0A & ( 1 << UDRE0 ))) ;
    UDR0 = c ;
}

static void uart_puts(const char *s )
{
    while ( *s ) uart_putc( *s++ );
}

static void uart_put_i32( int32_t val)
{
    char buf[12] ;
    ltoa( val , buf ,10 );
    uart_puts( buf );
}


static uint8_t uart_available(void)
{
    return ( UCSR0A & ( 1 << RXC0)) ? 1 : 0 ;
}


static char uart_getc(void)
{
    while( !(UCSR0A & (1 << RXC0)));
    return UDR0 ;
}


static uint8_t uart_readline(char * buf , uint8_t maxlen)
{
    uint8_t i = 0 ;
    while( 1 < maxlen - 1 ){
        char c = uart_getc() ;
        if( c == '\n' || c == '\r' )
        {
            if( i > 0) break ;
            continue ;
        }
        buf[i++] = toupper(c) ;
    }
    buf[i] = '\0' ;
    return i ;
}

//================ADC=======

static void adc_init(void)
{
    ADMUX = ( 1 << REFS0 ); //chanell 0
    ADCSRA = ( 1 << ADEN ) | ( 1 << ADPS2) | (1 << ADPS1) | ( 1 << ADPS0 );
}

static uint16_t adc_read(void)
{
    ADCSRA |=  ( 1 << ADSC );
    while( ADCSRA & (1 << ADSC)) ;
    return ADC ;
}

//=============timer pulse====
static void timer1_start( float steps_sec)
{
    if( steps_sec < 1.0) steps_sec = 1.0 ;

    uint16_t ocr = (uint16_t)(F_CPU / (2UL * 64 * (uint32_t)steps_sec) -1 ) ;
    if( ocr < 1) ocr = 1 ;

    TCCR1A = 0 ;
    TCCR1B = ( 1 << WGM12 ) | ( 1 << CS11 ) | ( 1<< CS10) ;//ctc ,prescaler 64
    OCR1A = ocr ;
    TCNT1 = 0 ;
    TIMSK1 = ( 1 << OCIE1A );
    moving = 1 ;
}

static void timer1_stop(void)
{
    TIMSK1 = 0 ;
    TCCR1B = 0 ;
    moving = 0 ;
}

ISR(TIMER1_COMPA_vect)
{
    if(homing){
        if( limit_hit ){
            timer1_stop() ;
            current_pos = 0 ;
            target_pos = 0 ;
            homing = 0 ;
            return ;
        }
        //step toward home
        PORTD |= ( 1 << STEP_PIN );
        PORTD &= ~( 1 << STEP_PIN );
        current_pos--;
        return ;
    }
    if( current_pos == target_pos ){
        timer1_stop() ;
        return ;
    }

    PORTD |= ( 1 << STEP_PIN );
    PORTD &= ~( 1 << STEP_PIN );

    if( current_pos < target_pos ) current_pos++;
    else current_pos-- ;

}

ISR( INT0_vect )
{
    limit_hit = 1 ;
}


static void move_to( float pos_mm , float rate_mm_min )
{
    int32_t target = ( int32_t)(pos_mm * steps_per_mm );
    int32_t diff;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        target_pos = target ;
        diff = target_pos - current_pos ;
    }

    if( diff == 0 ) return ;

    if( diff > 0 ) PORTD |= ( 1 << DIR_PIN );
    else { PORTD &= ~( 1 << DIR_PIN); diff = -diff; }

    float steps_sec = ( rate_mm_min / 60.0 ) * steps_per_mm ;
    timer1_start(steps_sec) ;

    while(moving){
        if( limit_hit && !homing ){
            timer1_stop() ;
            uart_puts("!! Limit hit during move\n");
            break ;
        }
    }
}


static void do_home(void)
{
    uart_puts("Homing...\n");
    limit_hit = 0 ;
    homing = 1 ;

    PORTD &= ~(1 << DIR_PIN );

    float home_speed = 200.0 ;
    float steps_sec = ( home_speed / 60.0 ) * steps_per_mm ;
    timer1_start(steps_sec);

    while(homing);

    uart_puts("Home found . Position reset to 0.\n");
}

static void jog_check(void)
{
    uint16_t adc = adc_read();

    /* Dead zone in center (480-544): no movement */
    if (adc > 480 && adc < 544) return;

    /* Map ADC to speed: further from center = faster */
    float speed;
    if (adc >= 544) {
        PORTD |= (1 << DIR_PIN);   /* Forward */
        speed = (float)(adc - 544) / 480.0 * 600.0;  /* 0-600 mm/min */
    } else {
        PORTD &= ~(1 << DIR_PIN);  /* Reverse */
        speed = (float)(480 - adc) / 480.0 * 600.0;
    }

    if (speed < 10.0) return;

    /* Single step at computed rate */
    float steps_sec = (speed / 60.0) * steps_per_mm;
    uint16_t delay_us = (uint16_t)(1000000.0 / steps_sec);
    if (delay_us < 100) delay_us = 100;

    PORTD |= (1 << STEP_PIN);
    PORTD &= ~(1 << STEP_PIN);

    if (PORTD & (1 << DIR_PIN)) current_pos++;
    else current_pos--;

    /* Simple delay for step rate */
    for (uint16_t i = 0; i < (delay_us / 10) ; i++) {
        _delay_us(10);
    }
}


/* ================================
 *  G-code parser
 *  ================================ */
static float parse_value(const char *line, char letter)
{
    const char *p = line;
    while (*p) {
        if (*p == letter) {
            return atof(p + 1);
        }
        p++;
    }
    return -1.0;
}

static void process_line(const char *line)
{
    if (line[0] == 'G') {
        int cmd = atoi(line + 1);
        float x, f;

        switch (cmd) {
            case 0:  /* G0: rapid move */
                x = parse_value(line, 'X');
                if (x >= 0) move_to(x, rapid_rate);
                break;

            case 1:  /* G1: linear move */
                f = parse_value(line, 'F');
                if (f > 0) feedrate = f;
                x = parse_value(line, 'X');
            if (x >= 0) move_to(x, feedrate);
            break;

            case 28: /* G28: home */
                do_home();
                break;

            default:
                uart_puts("Unknown G command\n");
        }
    } else if (line[0] == 'M') {
        int cmd = atoi(line + 1);

        switch (cmd) {
            case 114: /* M114: report position */
                uart_puts("X:");
                uart_put_i32(current_pos);
                uart_puts(" steps (");
                {
                    int32_t um = (int32_t)(current_pos * 1000.0 / steps_per_mm);
                    uart_put_i32(um / 1000);
                    uart_putc('.');
                    int32_t frac = um % 1000;
                    if (frac < 0) frac = -frac;
                    if (frac < 100) uart_putc('0');
                    if (frac < 10) uart_putc('0');
                    uart_put_i32(frac);
                }
                uart_puts(" mm)\n");
                break;

            case 500: /* M500: save calibration */
                eeprom_update_float(EE_STEPS_MM, steps_per_mm);
                uart_puts("Saved steps/mm: ");
                uart_put_i32((int32_t)steps_per_mm);
                uart_putc('\n');
                break;

            case 501: /* M501: load calibration */
            {
                float val = eeprom_read_float(EE_STEPS_MM);
                if (val > 0.0 && val < 10000.0) {
                    steps_per_mm = val;
                    uart_puts("Loaded steps/mm: ");
                    uart_put_i32((int32_t)steps_per_mm);
                    uart_putc('\n');
                } else {
                    uart_puts("No valid calibration in EEPROM\n");
                }
            }
            break;

            default:
                uart_puts("Unknown M command\n");
        }
    } else if (line[0] == 'J') {
        /* J1: enter jog mode, J0: exit */
        if (line[1] == '1') {
            uart_puts("Jog mode (pot controls speed). Send J0 to exit.\n");
            while (1) {
                jog_check();
                if (uart_available()) {
                    char c = uart_getc();
                    if (c == 'J' || c == 'j') break;
                }
            }
            uart_puts("Jog mode exited.\n");
        }
    } else if (line[0] != '\0') {
        uart_puts("? ");
        uart_puts(line);
        uart_putc('\n');
    }
}


int main(void)
{
    /* GPIO setup */
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN);

    /* Limit switch on PD2 (INT0): input with pull-up */
    DDRD &= ~(1 << LIMIT_PIN);
    PORTD |= (1 << LIMIT_PIN);

    /* INT0: falling edge (switch closes to GND) */
    EICRA = (1 << ISC01);
    EIMSK = (1 << INT0);

    uart_init();
    adc_init();
    sei();

    /* Load calibration from EEPROM */
    float saved = eeprom_read_float(EE_STEPS_MM);
    if (saved > 0.0 && saved < 10000.0) {
        steps_per_mm = saved;
    }

    uart_puts("\n--- ATmega328P Stepper Controller ---\n");
    uart_puts("Commands: G0 X50, G1 X25 F300, G28, M114, M500, M501, J1\n");
    uart_puts("Steps/mm: ");
    uart_put_i32((int32_t)steps_per_mm);
    uart_puts("\n> ");

    char line[64];

    while (1) {
        uart_readline(line, sizeof(line));
        process_line(line);
        uart_puts("> ");
    }
}

