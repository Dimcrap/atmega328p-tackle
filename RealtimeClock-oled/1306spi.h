#include <avr/io.h>
#include <stdint.h>

#define OLED_DC PB1 
#define OLED_RST PB0
#define OLED_CS PB2


static void spi_init(void)
{
    DDRB |= ( 1 << PB3 ) | ( 1 << PB5 ) | ( 1 << PB2 );

    SPCR = ( 1 << SPE ) | ( 1 << MSTR );

}


static uint8_t spi_transfer(uint8_t data)
{
    SPDR = data;
    while( !( SPSR & ( 1 << SPIF ) ));
    return SPDR;

}
/*
static uint8_t spi_transfer(uint16_t data)
{
    SPDR = data;
    while( !( SPSR & ( 1 << SPIF ) ));
    return SPDR;

}*/

static void oled_cmd( uint8_t cmd )
{
	PORTB &= ~( 1 << OLED_DC );
	PORTB &= ~( 1 << OLED_CS );
	spi_transfer( cmd );
	PORTB |= ( 1 << OLED_CS );
}


static void oled_data( const uint8_t * buf , uint16_t len )
{
	PORTB |= ( 1 << OLED_DC );
	PORTB &= ~( 1 << OLED_CS );
	for( uint16_t i = 0 ; i < len ; i++ ){
		spi_transfer(buf[i]);	
	}	
	PORTB |= ( 1 << OLED_CS );
}

/*
static void oled_data( const uint16_t * buf , uint16_t len )
{
    PORTB |= ( 1 << OLED_DC );
    PORTB &= ~( 1 << OLED_CS );
    for( uint16_t i = 0 ; i < len ; i++ ){
        spi_transfer(buf[i]);
    }
    PORTB |= ( 1 << OLED_CS );
}*/


static void oled_init(void)
{
	DDRB |= ( 1 << OLED_DC) | ( 1 << OLED_RST ) | ( 1 << OLED_CS );
	
	PORTB &= ~( 1 << OLED_RST );
	_delay_ms( 10 );
	PORTB |= ( 1 << OLED_RST );	
	_delay_ms( 10 );

	oled_cmd(0xAE) ;  /* Display off */
    oled_cmd(0xD5);  /* Set clock divide */
    oled_cmd(0x80);
    oled_cmd(0xA8);  /* Set multiplex ratio */
    oled_cmd(0x3F);  /* 64 rows */
    oled_cmd(0xD3);  /* Display offset */
    oled_cmd(0x00);
    oled_cmd(0x40);  /* Start line 0 */
    oled_cmd(0x8D);  /* Charge pump */
    oled_cmd(0x14);  /* Enable */
    oled_cmd(0x20);  /* Memory addressing mode */
    oled_cmd(0x00);  /* Horizontal */
    oled_cmd(0xA1);  /* Segment remap */
    oled_cmd(0xC8);  /* COM scan direction */
    oled_cmd(0xDA);  /* COM pins config */
    oled_cmd(0x12);
    oled_cmd(0x81);  /* Contrast */
    oled_cmd(0xCF);
    oled_cmd(0xD9);  /* Pre-charge period */
    oled_cmd(0xF1);
    oled_cmd(0xDB);  /* VCOMH deselect */
    oled_cmd(0x40);
    oled_cmd(0xA4);  /* Display from RAM */
    oled_cmd(0xA6);  /* Normal (not inverted) */
    oled_cmd(0xAF);  /* Display on */
}


static uint8_t framebuf[ 1024 ] ;
static uint16_t tstframebuf[ 512 ] ;


static const uint8_t font_5x7[][5] = {
    {0x3E,0x51,0x49,0x45,0x3E},  /* 0 */
    {0x00,0x42,0x7F,0x40,0x00},  /* 1 */
    {0x42,0x61,0x51,0x49,0x46},  /* 2 */
    {0x21,0x41,0x45,0x4B,0x31},  /* 3 */
    {0x18,0x14,0x12,0x7F,0x10},  /* 4 */
    {0x27,0x45,0x45,0x45,0x39},  /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30},  /* 6 */
    {0x01,0x71,0x09,0x05,0x03},  /* 7 */
    {0x36,0x49,0x49,0x49,0x36},  /* 8 */
    {0x06,0x49,0x49,0x29,0x1E},  /* 9 */
    {0x00,0x36,0x36,0x00,0x00},  /* : */
};


static uint16_t font_ [][8] ={
    //{0x0F, 0xFC , 0x33 , 0x03 , 0x3F , 0xF0, 0x0F, 0xFC } /* 0 */
    { 0x0000,0x0ffc,0x3303,0x30c3,0x30c3,0x3033,0x0ffc,0x0000 },  // 0
    { 0x0000,0x0000,0x0000,0x300c,0x3fff,0x3000,0x0000,0x0000 },  // 1
    { 0x0000,0x3c0c,0x3303,0x30c3,0x30c3,0x30c3,0x303c,0x0000 },  // 2
    { 0x0000,0x0c03,0x3003,0x30c3,0x30c3,0x30f3,0x0f0f,0x0000 },  // 3
    { 0x0000,0x0300,0x03c0,0x0330,0x030c,0x3fff,0x0300,0x0000 },  // 4
    { 0x0000,0x0c3f,0x3033,0x3033,0x3033,0x3033,0x0fc3,0x0000 },  // 5
    { 0x0000,0x0ff0,0x30cc,0x30c3,0x30c3,0x30c3,0x0f03,0x0000 },  // 6
    { 0x0000,0x0003,0x0003,0x3f03,0x00c3,0x0033,0x000f,0x0000 },  // 7
    { 0x0000,0x0f3c,0x30c3,0x30c3,0x30c3,0x30c3,0x0f3c,0x0000 },  // 8
    { 0x0000,0x303c,0x30c3,0x30c3,0x30c3,0x0cc3,0x03fc,0x0000 },  // 9
    { 0x0000,0x0000,0x0000,0x0330,0x0000,0x0000,0x0000,0x0000 },  // :
};


static void fb_clear(void)
{
    for (uint16_t i = 0; i < 1024; i++) framebuf[i] = 0 ;
    for ( uint16_t i = 0 ; i< 512 ; i++ ) tstframebuf[i] = 0 ;
}



static void fb_draw_char(uint8_t col, uint8_t page, char c)
{
    uint8_t idx;
    if (c >= '0' && c <= '9') idx = c - '0';
    else if (c == ':') idx = 10;
    else return;

    uint16_t offset = page * 128 + col ;
    for (uint8_t i = 0 ; i < 5 ; i++ ) {
        if (offset + i < 1024) {
            //framebuf[ offset + i ] = font_5x7[ idx ][ i ] ;
            framebuf[ offset + i ] = 0xFF ;
            //framebuf[ offset + i + i ] = font_5x7[idx][i] ;
        }
    }

    offset = (page+1) * 128 + col ;
    //framebuf[ offset ] = 0xFF ;

}


static void fb_test_draw_char(uint16_t col, uint16_t page, char c)
{
    uint8_t idx = 0 ;
    //if (c >= '0' && c <= '9') idx = c - '0' ;
    //else if (c == ':') idx = 10 ;
    //else return ;

    uint16_t offset = page * 128 + col ;
    for ( uint8_t i = 0 ; i < 8 ; i++ ) {
        if ( offset + i < 512 ) {
            tstframebuf[ offset + i ] = font_ [ idx ][ i ] ;
            //framebuf[ offset + i + i ] = font_5x7[idx][i] ;
        }
    }
    offset =  page  * 128 + (col+1) ;
    tstframebuf[offset+5] = 0xFF ;
}



static void fb_draw_string(uint8_t col, uint8_t page, const char *s)
{
    while (*s) {
        fb_draw_char( col, page, *s++  );
        col += 7;  /* set 6 for 5 pixels + 1 space */
    }
}

static void oled_flush(void)
{
    oled_cmd(0x21); oled_cmd(0); oled_cmd(127) ;  /* Column range */
    oled_cmd(0x22); oled_cmd(0); oled_cmd(7) ;    /* Page range */
    //oled_data( framebuf, 1024 );
    oled_data( framebuf , 1024 );
}
