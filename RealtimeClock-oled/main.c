#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "1306spi.h"


volatile uint32_t seconds = 0 ;
volatile uint32_t milliseconds = 0 ;

ISR(TIMER1_COMPA_vect)
{
    static uint16_t subsec = 0 ;
    //static uint16_t submill = 0 ;

    subsec++ ;
    //submill++ ;

    /*if( subsec % 1000 == 0 ){
       milliseconds++ ;
    }*/


    if( subsec >= 1000 ){
        subsec = 0 ;
        seconds++ ;
    }else{
        milliseconds++ ;
    }
}

static void timer1_init(void){
    TCCR1A = 0 ;
    TCCR1B = ( 1 << WGM12 ) | ( 1 << CS11 ) | ( 1 <<CS10);
    OCR1A = 249 ;
    TIMSK1 = ( 1 << OCIE1A );
}

static void format_time(uint32_t sec , char *buf)
{
        uint8_t h = ( sec / 3600 ) % 24 ;
        uint8_t m = ( sec / 60 ) % 60 ;
        uint8_t s = sec % 60 ;

        buf[0] = '0' + h / 10 ;
        buf[1] = '0' + h % 10 ;
        buf[2] = ':' ;
        buf[3] = '0' +  m / 10 ;
        buf[4] = '0' + m % 10 ;
        buf[5] = ':' ;
        buf[6] = '0' +  s / 10 ;
        buf[7] = '0' + s % 10 ;
        buf[8] = '\0' ;
}



int main(void)
{
    spi_init() ;
    oled_init() ;
	timer1_init() ;
	sei() ;

	uint32_t last_sec = 0xFFFFFFFF ;
    uint32_t last_millsec =0xFFFFFFFF ;


	while(1){
		uint32_t now ;
        uint32_t nowmill ;
		//adding conditin for if stop button presses or not
		ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
			now = seconds ;
            nowmill = milliseconds ;
		}


	        if ( now != last_sec ) {
            last_sec = now ;
            char time_str[9] ;
            format_time( now, time_str ) ;

            fb_clear();
            fb_draw_string( 20, 3, time_str );  /* Centered on page 3 */
            //fb_draw_char(3,6 , '3') ;
            //fb_draw_char2x7( 60 , 7 , '4' );
            oled_flush();
        }

        if( nowmill != last_millsec )
        {
            last_millsec = nowmill  ;
            //fb_clear();
            fb_draw_bar( nowmill ) ;
            oled_flush() ;
        }

	}

}







