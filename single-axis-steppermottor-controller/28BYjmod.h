#include <avr/io.h>
#include <util/delay.h>
#include <cstdint>


#define COIL1
#define COIL2
#define COIL3
#define COIL4

typedef uint8_t byte  ;

#ifndef BYJ48_DDR_IN
#define BYJ48_DDR_IN    DDRC
#endif
#ifndef BYJ48_PORT_IN
#define BYJ48_PORT_IN   PORTC
#endif
#ifndef BYJ48_PIN_IN
#define BYJ48_PIN_IN    PINC
#endif

#define SETBIT(REG,BIT) { REG |= (1 << BIT ); }
#define CLEARBIT (REG , BIT) { REG &= ~( 1 << BIT ); }

#ifndef BYJ48_IN1
#define BYJ48_IN1       0
#endif
#ifndef BYJ48_IN2
#define BYJ48_IN2       1
#endif
#ifndef BYJ48_IN3
#define BYJ48_IN3       2
#endif
#ifndef BYJ48_IN4
#define BYJ48_IN4       3
#endif

#define TURNOFF 0xF0
#define HALFSTEPCOUNT 8

const uint8_t g_halfstep[] = {
     0b00000001
    ,0b00000011
    ,0b00000010
    ,0b00000110
    ,0b00000100
    ,0b00001100
    ,0b00001000
    ,0b00001001
};


void BYJ48_SetDDR_INs(void)
{
    SETBIT( BYJ48_DDR_IN, BYJ48_IN1 );
    SETBIT( BYJ48_DDR_IN , BYJ48_IN2 );
    SETBIT( BYJ48_DDR_IN , BYJ48_IN3 );
    SETBIT( BYJ48_DDR_IN , BYJ48_IN4 );
}


void BYJ48_SetPORT_INs()
{
    SETBIT( BYJ48_PORT_IN , BYJ48_IN1 ) ;
    SETBIT( BYJ48_PORT_IN , BYJ48_IN2 ) ;
    SETBIT( BYJ48_PORT_IN , BYJ48_IN3 ) ;
    SETBIT( BYJ48_PORT_IN , BYJ48_IN4 ) ;
}


void BYJ48_Init (void)
{
    // set IN1-4 as output
    BYJ48_SetDDR_INs ();
    // set IN1-4 high
    BYJ48_SetPORT_INs ();
}

void BYJ48_HalfStepClockwise(void){
    byte i = 0 ;
    while( i < HALFSTEPCOUNT ){
        BYJ48_PORT_IN =  (BYJ48_PORT_IN & 0xF0 ) | g_halfstep[i++] ;
        _delay_ms(2) ;
    }
}

void BYJ48_HalfStepAntiClockwise(void)
{
    byte i = HALFSTEPCOUNT ;
    while ( i > 0) {
        BYJ48_PORT_IN = ( BYJ48_PORT_IN & 0xF0) | g_halfstep[i++] ;
        _delay_ms(2) ;
    }
}

void BYJ48_OneTurnClockwise(void )
{
    uint16_t k ;
    for ( k = 0 ; k < 512 ; k++){
        BYJ48_HalfStepClockwise() ;
    }
}

void BYJ48_OneTurnantiClockwise(void)
{
    uint16_t j ;
    for( j = 0 ; j < 512 ; j++ )
    {
        BYJ48_HalfStepAntiClockwise() ;
    }
}

void BYJ48_TurnOff(void)
{
    BYJ48_PORT_IN &= TURNOFF ;
}



