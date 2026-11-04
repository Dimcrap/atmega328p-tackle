#include <stdint.h>
#define F_CPU 16000000UL
#include <avr/io.h>


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

}
