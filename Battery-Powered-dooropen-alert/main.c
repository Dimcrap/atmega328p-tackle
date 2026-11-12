#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <sys/types.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define LEDPIN PB0
#define BUZZPIN PB1
#define SWITCH PD2

#define EE_SLEEP_COUNT ((uint16_t *) 0) ;
typedef uint8_t byte;

volatile uint8_t wdt_fired = 0 ;
volatile uint8_t door_opened = 0 ;

ISR(WDT_vect)
{
        wdt_fired = 1 ;
}

ISR(INT0_vect)
{
    door_opened = 1 ;
}


static void delay_ms( uint16_t ms)
{
    while( ms-- ) _delay_ms(1) ;
}


static void led_blink( byte count, uint16_t on_ms , uint16_t off_ms )
{
        DDRB |= ( 1 << LED_PIN );
        for( byte i = 0 ; i < count ; i++ )
        {
            PORTB |= ( 1 << LEDPIN) ;
            delay_ms( on_ms ) ;
            PORTB &= ~( 1 << LEDPIN );
            delay_ms( off_ms ) ;
        };
}


static void beepBuzz( uint16_t duration )
{
    DDRB |= ( 1 << BUZZ ) ;
    PRR &= ~( 1 << PRTIME1 ); //time1 enable

    TCCR1A = ( 1 << COM1A0 ); //toggle OC1A
    TCCR1B = ( 1 << WGM12) | ( 1 << CS11 ); //prescaler 8
    OCR1A = 499 ; // 8MHz / ( 2 * 8 * 500 ) = 1kHz

    delay_ms( duration ) ;

    TCCR1A = 0 ;
    TCCR1B = 0 ;
    PORTB &= ~( 1 << BUZZPIN ) ;

    PRR |= ( 1 << PRTIME1 ) ;
}

static void eneter_deep_sleep(void)
{
    ADCSRA &= ~(1 << ADEN); //ADC
    ACSR |= ( 1 <<‌ ACD ); //analog comparator

    PRR = (1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM0)
    | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0)
    | (1 << PRADC);

    DDRB &= ~( (1 << LEDPIN) | ( 1 << BUZZPIN) );
    PORTB &= ~( (1 << LEDPIN) | ( 1 << BUZZER_PIN));

    WDTCSR |= ( 1 << WDIE) ;

    set_sleep_mode( SLEEP_MODE_PWR_DOWN );
    sleep_enable();

    MCUCR |= (1 << BODS) | (1 << BODSE);
    MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);
    sei() ;
    sleep_cpu() ;  /* Must enter sleep within 3 clock cycles */
    sleep_disable() ;
}



int main(void)
{
    MCUSR &= ~( 1 << WDRF );
    wdt_disable();

    DDRD &= ~( 1 << REED_PIN );
    PORTD |= ( 1 << REED_PIN );

    EICRA = ( 1 << ISC01) | ( 1 << ISC00 ) ;
    EIMSK = ( 1 << INT0 ) ;

    cli();
    wdt_reset() ;
    WDTCSR |= ( 1 << WDCE ) | ( 1 << WDE ) ;
    WDTCSR = ( 1<< WDIE ) | ( 1 << WDP3 ) | ( 1 << WDP0 );
    sei();

    uint16_t door_count = eeprom_read_word( EE_DOOR_COUNT ) ;
    if( door_count = 0xFFFF ) door_count = 0 ;

    byte show = door_count % 10 ;
    if( show > 0 ) led_blink( show , 150 , 150 );
    else led_blink( 3 , 100 , 100 );

    while( 1 ){
        eneter_deep_sleep() ;

        if( door_opened ){
            door_opened = 0 ;

            door_count++ ;
            eeprom_read_word( EE_DOOR_COUNT,door_count ) ;

            beepBuzz(2000) ;
            led_blink( 3 , 200 , 200 );

            _delay_ms(400) ;
        }

        if( wdt_fired ){
            wdt_fired  = 0 ;

            DDRB |= ( 1 << LED_PIN );
            PORTB |= ( 1 << LED_PIN );
            _delay_ms(20) ;
            PORTB &= ~( 1 << LED_PIN );
            DDRB &= ~( 1 << LED_PIN );
        }
    }
}


