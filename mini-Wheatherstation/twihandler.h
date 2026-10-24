#pragma once

#define TWI_FREQ 100000UL

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

static void twi_init(void)
{
    TWSR = 0 ;  //prescaler = 1
    TWBR = ( ( F_CPU  / TWI_FREQ ) - 16 ) / 2 ;
    TWCR = ( 1 << TWEN );

}

static uint8_t twi_start(void)
{
    TWCR = ( 1 << TWINT ) | ( 1 << TWSTA ) | ( 1 << TWEN );
    while( !( TWCR & (TWINT) ) ) ;
    return ( TWCR & 0xF8 ) ;
}

static void twi_stop(void)
{
        TWCR = ( 1 << TWINT ) | ( 1 << TWSTO ) | ( 1 << TWEN ) ;
        while(  TWCR & ( 1 << TWSTO )) ;
}

static uint8_t twi_write(uint8_t data)
{
    TWDR = data ;
    TWCR = ( 1 << TWINT ) | ( 1 << TWEN );
    while( !(TWCR & ( 1 << TWINT ))) ;
    return ( TWSR & 0xF8 ) ;
}

static uint8_t twi_read_ack(void)
{
        TWCR = ( 1 << TWINT ) | ( 1<<TWEA) | ( 1<< TWEN);
        while( !( TWCR & ( 1 << TWINT )) );
        return TWDR ;
}


static uint8_t twi_read_nack(void)
{
        TWCR = ( 1 << TWINT ) | ( 1<< TWEN );
        while( !( TWCR & ( 1<<TWINT )));
        return TWDR ;
}
















