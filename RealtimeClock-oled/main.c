#include <stdint.h>
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "1306spi.h"


volatile uint32_t seconds = 0 ;


ISR(TIMER1_COMPA_vect)
{
    static uint16_t subsec = 0 ;

    subsec++ ;
    if( subsec >= 1000 ){
        subsec = 0 ;
        seconds++ ;
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
    char test[5] = "555" ;

	while(1){
		uint32_t now ;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			now = seconds ;
		}
		
	        if (now != last_sec) {
       	     last_sec = now;
            char time_str[9];
            format_time( now, time_str );

            fb_clear();
            //fb_draw_string(30, 3, time_str);  /* Centered on page 3 */
            //fb_draw_string(32, 4, test );
            fb_draw_string(0, 0,"143" );
            fb_draw_string(0, 1,"342" );
            fb_draw_string(0, 2,"342" );
            //fb_draw_string(1, 2,"2123" );
            //fb_draw_string(1, 3,"2123" );
            //fb_draw_string(0, 5,"1123" );
            //fb_draw_string(0, 7,"3" );
            //fb_test_draw_char( 15 ,4 ,'0' );
            oled_flush();
        }
	}

}







