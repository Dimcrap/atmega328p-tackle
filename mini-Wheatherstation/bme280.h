#include "twihandler.h"
#include <avr/io.h>
#include <util/delay.h>

#define BME280_ADDR 0x76


static void bme280_write_reg( uint8_t reg , uint8_t val )
{
    twi_start() ;
    twi_write( ( BME280_ADDR << 1 ) | 0 ) ;
    twi_write( reg ) ;
    twi_stop() ;
}


static uint8_t bme280_read_reg( uint8_t reg )
{
    twi_start() ;
    twi_write( (BME280_ADDR << 1 ) | 0 ) ;
    twi_write ( reg ) ;
    twi_start() ;
    twi_write( (BME280_ADDR << 1 ) | 1 ) ;
    uint8_t val = twi_read_nack() ;
    twi_stop() ;
    return val ;
}

static void bme280_read_burst( uint8_t reg , uint8_t *buff , uint8_t len )
{
    twi_start();
    twi_write( (BME280_ADDR << 1 ) | 0  );
    twi_write( reg ) ;
    twi_start() ;
    twi_write( ( BME280_ADDR << 1) | 1 ) ;
    for( uint8_t i = 0 ; i < len - 1 ; i++ )
    {
        buff[i] = twi_read_ack() ;
    }
    buff[ len -1 ] = twi_read_nack() ;
    twi_stop() ;
}

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5;
    int16_t  dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t  dig_H1, dig_H3;
    int16_t  dig_H2, dig_H4, dig_H5;
    int8_t   dig_H6;
} bme280_cal_t;

static bme280_cal_t cal ;


static void bme280_read_calibration( void )
{
        uint8_t buf[ 26 ];
        bme280_read_burst( 0x88  , buf , 26 ) ;


        cal.dig_T1 = (uint16_t)(buf[1] << 8) | buf[0];
        cal.dig_T2 = (int16_t)(buf[3] << 8) | buf[2];
        cal.dig_T3 = (int16_t)(buf[5] << 8) | buf[4];
        cal.dig_P1 = (uint16_t)(buf[7] << 8) | buf[6];
        cal.dig_P2 = (int16_t)(buf[9] << 8) | buf[8];
        cal.dig_P3 = (int16_t)(buf[11] << 8) | buf[10];
        cal.dig_P4 = (int16_t)(buf[13] << 8) | buf[12];
        cal.dig_P5 = (int16_t)(buf[15] << 8) | buf[14];
        cal.dig_P6 = (int16_t)(buf[17] << 8) | buf[16];
        cal.dig_P7 = (int16_t)(buf[19] << 8) | buf[18];
        cal.dig_P8 = (int16_t)(buf[21] << 8) | buf[20];
        cal.dig_P9 = (int16_t)(buf[23] << 8) | buf[22];
        //cal.dig_H1 = bme280_read_reg( 0xA1 );

        uint8_t hbuf[ 7 ] ;
        //bme280_read_burst( 0xE1 , hbuf , 7 );
        cal.dig_H2 = ( int16_t )( hbuf[1] << 8 ) | hbuf[ 0 ] ;
        cal.dig_H3 = hbuf[ 2 ] ;
        cal.dig_H4 = ( int16_t )( hbuf[ 3 ] << 4 ) | ( hbuf[ 4 ] & 0x0F );
        cal.dig_H5 = ( int16_t )( hbuf[5] << 4 ) | ( hbuf[ 4 ] >> 4 );
        cal.dig_H6 = ( int8_t )hbuf[ 6 ] ;
}


static void bme280_init( void )
{
    bme280_write_reg( 0xE0 , 0xB6 ) ;
    _delay_ms( 10 );

    bme280_read_calibration() ;

    /* humididy oversampling x1 ( must be set before ctrl_meas) */
    bme280_write_reg( 0xF2 , 0x01 ) ;

    //temprature x1 , pressure x1 , normal mode
    bme280_write_reg( 0xF4 , 0x27 ) ;

    //standby 1000ms , filter off
    bme280_write_reg( 0xF5 , 0xA0 );
}

static int32_t t_fine;

static int32_t compensate_temp(int32_t adc_T)
{
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1)))
    * ((int32_t)cal.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)cal.dig_T1))
    * ((adc_T >> 4) - ((int32_t)cal.dig_T1))) >> 12)
    * ((int32_t)cal.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;  /* Result in 0.01 C */
}


static uint32_t compensate_press(int32_t adc_P)
{
    int32_t var1 = (t_fine >> 1) - 64000;
    int32_t var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)cal.dig_P6);
    var2 = var2 + ((var1 * ((int32_t)cal.dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)cal.dig_P4) << 16);
    var1 = (((cal.dig_P3 * (((var1>>2)*(var1>>2)) >> 13)) >> 3)
    + ((((int32_t)cal.dig_P2) * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * ((int32_t)cal.dig_P1)) >> 15;
    if (var1 == 0) return 0;
    uint32_t p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000) p = (p << 1) / ((uint32_t)var1);
    else                p = (p / (uint32_t)var1) * 2;
    var1 = (((int32_t)cal.dig_P9) * ((int32_t)(((p>>3)*(p>>3))>>13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)cal.dig_P8)) >> 13;
    return (uint32_t)((int32_t)p + ((var1 + var2 + cal.dig_P7) >> 4));
    /* Result in Pa with Q24.8 format (divide by 256 for Pa) */
}


static uint32_t compensate_humid(int32_t adc_H)
{
    int32_t v = t_fine - 76800;
    v = (((((adc_H << 14) - (((int32_t)cal.dig_H4) << 20)
    - (((int32_t)cal.dig_H5) * v)) + 16384) >> 15)
    * (((((((v * ((int32_t)cal.dig_H6)) >> 10)
    * (((v * ((int32_t)cal.dig_H3)) >> 11) + 32768)) >> 10)
    + 2097152) * ((int32_t)cal.dig_H2) + 8192) >> 14));
    v = v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)cal.dig_H1)) >> 4);
    v = (v < 0) ? 0 : v;
    v = (v > 419430400) ? 419430400 : v;
    return (uint32_t)(v >> 12);  /* Result in Q22.10 format (divide by 1024 for %) */
}

