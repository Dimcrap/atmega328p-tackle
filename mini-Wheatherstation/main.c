#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "1306spi.h"
#include "twihandler.h"
#include "bme280.h"




int main(void)
{
    spi_init() ;
    oled_init() ;
    twi_init() ;
    bme280_init() ;


    while( 1 ){

            fb_clear();
            //fb_draw_string( 20, 3, time_str );  /* Centered on page 3 */
            //fb_draw_char(3,6 , '3') ;
            //fb_draw_char2x7( 60 , 7 , '4' );
            fb_draw_char( 0 , 0 ,'4') ;
            fb_draw_char( 1 , 2 ,'g') ;
            fb_draw_string( 2 , 4 , "2350" ) ;
            fb_draw_string( 10, 5, "whusp a eyoo" );
            oled_flush() ;

            _delay_ms(2000) ;
    }

}







