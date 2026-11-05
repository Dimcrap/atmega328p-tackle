#include <stdint.h>
#define F_CPU 16000000UL
#include <avr/io.h>
#include "1306spi.h"
#include <string.h>

static void adc_intit(void)
{
    ADMUX = ( 1 << REFS0 ) ;

    //enabling adc prescaller 128 (125 khz ADC clock)
    ADCSRA = ( 1 << ADEN ) | ( 1 << ADPS2) | ( 1 << ADPS1) | ( 1 << ADPS0 ) ;

    //first conversion (25 clock cycles)
    ADCSRA |= ( 1 << ADSC );
    while( ADCSRA & ( 1 << ADSC )) ;
}


static uint16_t adc_read( uint8_t channel )
{
    ADMUX = ( ADMUX & 0xF0 ) | ( channel & 0x0F ) ;

    ADCSRA |= ( 1<< ADSC );
    while( ADCSRA & ( 1 << ADSC )) ;

    return ADC ; // Read 10-bit result
}


static void adc_init_freerun(uint8_t channel)
{
        ADMUX = ( 1 << REFS0 ) |( channel & 0x0F );
        ADCSRA = ( 1 << ADEN ) | ( 1 << ADSC ) | ( 1 << ADATE ) | ( 1 << ADPS2 ) | ( 1 <<  ADPS1 ) | ( 1 << ADPS0 ) ;
        ADCSRB = 0 ;
}

static uint16_t adc_read_freerun(void)
{
    return ADC ;
}

#define OVERSAMPLE_COUNT 16
#define BAR_MAX_WIDTH 50

static uint16_t adc_read_averaged(uint8_t channel )
{
    uint32_t sum = 0 ;
    for( uint8_t i = 0 ; i < OVERSAMPLE_COUNT; i++)
    {
        sum += adc_read(channel) ;
    }
    return (uint16_t)( sum / OVERSAMPLE_COUNT );
}



int main(void)
{
    spi_init() ;
    oled_init() ;
    adc_intit() ;

    while( 1) {
        uint16_t left = adc_read_averaged(0) ;
        uint16_t right = adc_read_averaged(1) ;

        uint8_t bar_l = (uint8_t)((uint32_t)left * BAR_MAX_WIDTH / 1023 );
        uint8_t bar_r = (uint8_t)((uint32_t)right * BAR_MAX_WIDTH / 1023) ;

        fb_clear() ;

        fb_draw_string( 0 , 0 , "LEFT" ) ;

        fb_draw_string( 0 , 0  ,"LEFT" ) ;
        fb_draw_string( 90 ,0 , "RIGHT") ;

        fb_draw_bar( 0 , 2 , bar_l) ;
        fb_draw_bar( 128 - bar_r , 4 ,bar_r ) ;

        char dir = '=' ;
        int16_t diff = (int16_t)left - ( int16_t)right ;
        if( diff > 50) dir = '<' ;
        else if ( diff < -50) dir = '>' ;
        fb_draw_arrow( 60, 6 , dir ) ;

        char buf[5] ;
        utoa( left , buf ,10 );
        fb_draw_string( 30 , 0 , buf ) ;
        utoa( right , buf , 10 );
        fb_draw_string( 70 , 0 , buf ) ;

        oled_flush() ;
        _delay_ms(50) ;

    }
}




