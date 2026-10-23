#include <cstdint>
#define F_CPU 16000000UL
#define TWI_FREQ 100000UL

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include "1306spi.h"
#include "twihandler.h"
#include "bme280.h"


static void twi_init(void){

}






int main(void)
{
    twi_init() ;
    spi_init() ;
    oled_init() ;
    bme280_init() ;

    while( 1 )
    {
        uint8_t data[ 8 ];

    }


}
