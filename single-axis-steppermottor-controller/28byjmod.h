#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

typedef uint8_t byte ;


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



static volatile int16_t currentpos = 0 ;
static volatile byte rappiding = 0 ;


static const uint8_t g_halfstep[] = {
     0b00000001
    ,0b00000011
    ,0b00000010
    ,0b00000110
    ,0b00000100
    ,0b00001100
    ,0b00001000
    ,0b00001001
};


void BYJ48_SetDDR_INs(void);

void BYJ48_SetPORT_INs();

void BYJ48_Init (void);

void BYJ48_TurnOff(void);

void BYJ48_HalfStepClockwise(void);

void BYJ48_HalfStepAntiClockwise(void) ;

void BYJ48_OneTurnClockwise(void ) ;

void BYJ48_OneTurnantiClockwise(void) ;

void BYJ48_Step(uint16_t stepcount , int8_t dir) ;

void BYJ48_Rapid(byte clockwise);

void BYJ48_Savepos( float * address) ;


static void updatecurrentpos(void);
