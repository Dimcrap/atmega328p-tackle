#include "28byjmod.h"



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


void BYJ48_TurnOff(void)
{
    rappiding = 0 ;
    BYJ48_PORT_IN &= TURNOFF ;
}


void BYJ48_HalfStepClockwise(void){
    byte i = 0 ;
    while( i < HALFSTEPCOUNT )
    {
        BYJ48_PORT_IN =  (BYJ48_PORT_IN & 0xF0 ) | g_halfstep[i++] ;
        _delay_ms(2) ;
    }
     ( currentpos +1 >= 512 ) ? currentpos = 0 : currentpos+=1 ;
}


void BYJ48_HalfStepAntiClockwise(void)
{
    byte i = HALFSTEPCOUNT ;
    // one step = 1/8*64
    while (i--) {
        // send to INs
        BYJ48_PORT_IN = (BYJ48_PORT_IN & 0xF0) | g_halfstep[i];

        _delay_ms (2);
    }
    ( currentpos - 1 < 0 ) ? currentpos = 0 : currentpos-=1 ;
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
    // one turn
    for (j=0; j<512; j++) {
        // one step
        BYJ48_HalfStepAntiClockwise ();
    }
}

void BYJ48_Step(uint16_t stepcount , int8_t dir) {
    stepcount*=5;
    uint16_t count = 500 ;
    if( dir > 0 )
    {
        while( stepcount-- ){
            BYJ48_HalfStepClockwise() ;
        }
    }else {
        while( stepcount-- ){
            BYJ48_HalfStepAntiClockwise() ;
        }
    }
};


void BYJ48_Rapid(byte clockwise)
{
    rappiding = 1 ;
    if( clockwise )
    {   while( rappiding ) BYJ48_OneTurnClockwise() ;
    }else{
        while( rappiding ) BYJ48_OneTurnantiClockwise() ;
    }
}

void BYJ48_Savepos( float * address){
    eeprom_()
}

static void updatecurrentpos(void){
    //512 > x
    eeprom_update_i16( EE_POS_VAL , newvalue ) ;

};


/*
 * eprom_read_float( EE_STEPS_MM );
 * eeprom_update_float( EE_STEPS_MM , steps_per_mm );
 *
 * */
