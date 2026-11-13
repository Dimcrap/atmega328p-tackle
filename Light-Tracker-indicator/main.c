#include <stdint.h>
#define F_CPU 16000000UL
#include <avr/io.h>
#include "1306spi.h"
#include <string.h>
#include "ADCdriver.h"
#include <string.h>
#include <stdlib.h>


int main(void)
{
    spi_init() ;
    oled_init() ;
    adc_intit() ;

    while( 1) {
        uint16_t left = adc_read_averaged(0) ;
        uint16_t right = adc_read_averaged(1) ;
        uint16_t up = adc_read_averaged(2) ;

        uint8_t bar_l = (uint8_t)( (uint32_t)left * BAR_MAX_WIDTH / 1023 ) ;
        uint8_t bar_r = (uint8_t)( (uint32_t)right * BAR_MAX_WIDTH / 1023) ;
        uint8_t bar_third = (uint8_t)( (uint32_t)up * BAR_MAX_WIDTH / 1023) ;

        fb_clear() ;

        fb_draw_string( 0 , 0  ,"LEFT" ) ;
        fb_draw_string( 93 ,0 , "RIGHT") ;
        fb_draw_string( 0 , 3 , "UP" );

        fb_draw_bar( 0 , 2 , bar_l ) ;
        fb_draw_bar( 128 - bar_r , 3 , bar_r ) ;
        fb_draw_bar( 0 , 5 , bar_third );

        char dir = '=' ;
        /*int16_t diff = ( int16_t )left - ( int16_t ) right ;
        if( diff > 50) dir = '<' ;
        else if( diff >  ) dir = '^' ;
        else if ( diff < -50 ) dir = '>' ;*/
        if( left > right && left > up ) dir = '<' ;
        else if( right > left && right > up ) dir = '>' ;
        else if( up > left && up > right ) dir = '^' ;

        fb_draw_arrow( 60, 6 , dir ) ;

        char buf[5] ;
        utoa( left , buf , 10 ) ;
        fb_draw_string( 30 , 0 , buf ) ;
        utoa( right , buf , 10 ) ;
        fb_draw_string( 70 , 0 , buf ) ;
        utoa( up , buf , 10 ) ;
        fb_draw_string( 15 , 3 , buf ) ;

        oled_flush() ;
        _delay_ms(50) ;

    }

}




