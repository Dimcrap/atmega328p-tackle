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

#include "28BYjmod.h"


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
    UBRR0H = ( uint8_t )(UBRR_VAL >> 8 ) ;
    UBRR0L = ( int8_t )(UBRR_VAL);
    UCSR0A = ( 1 << U2X0 ) ;
    UCSR0B = ( 1 << TXEN0 ) | ( 1 << RXEN0 ) ;
    UCSR0C = ( 1 << UCSZ01 ) | ( 1 << UCSZ00 ) ;
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
    ltoa( val , buf ,10 ) ;
    uart_puts( buf ) ;
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

//================ADC=========

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


ISR(TIMER1_COMPA_vect)
{

}


ISR( INT0_vect )
{
    limit_hit = 1 ;
}


/*  ================================
 *       G-code parser
 *  ================================ */
static float parse_value(const char *line, char letter)
{
    const char *p = line ;
    while ( *p ) {
        if ( *p == letter ) {
            return atof( p + 1 ) ;
        }
        p++ ;
    }
    return -1.0 ;
}



static void primary_processline( const char * line );


int main(void)
{

    uart_init() ;
    adc_init() ;
    sei() ;
    BYJ48_Init() ;

    /* Load calibration from EEPROM */
    /*
    float saved = eeprom_read_float( EE_STEPS_MM );
    if ( saved > 0.0 && saved < 10000.0 ) {
        steps_per_mm = saved ;
    }*/

    EICRA = (1 << ISC01);
    EIMSK = (1 << INT0);

    uart_puts("\n--- ATmega328P Stepper Controller ---\n");
    uart_puts("Commands: G0 X50, G1 X25 F300, G3 x b, M114, M500, M501, J1\n");
    uart_puts("Steps/mm: ");
    uart_put_i32( (int32_t)steps_per_mm );
    uart_puts("\n> ");

    char line[64];

    while (1) {
        uart_readline(line, sizeof(line)) ;
        primary_processline( line ) ;
        uart_puts("> ") ;
    }

}

static void primary_processline( const char * line )
{
    if ( line[0] == 'G' ) {
        int cmd = atoi( line + 1 );

        switch ( cmd ) {
            case 0 :
                uart_puts("turning clockwise ...->");
                BYJ48_OneTurnClockwise() ;
                break;
            case 1 :
                uart_puts("turning anticlockwise <-...");
                BYJ48_OneTurnantiClockwise() ;
                break;
            case 2 :
                BYJ48_TurnOff() ;
                uart_puts("turned OFF\n") ;
                break;
            case 3 :
                BYJ48_Step( atoi( line+3 ) , atoi(line+5) ) ;
                uart_puts( "stepping . . . " );
                break ;

            default:
                uart_puts("Undefined command");
        }
    }else if (line[0] != '\0') {
         uart_puts("? ");
         uart_puts(line);
         uart_putc('\n');
     }
};

