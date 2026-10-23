#include <avr/io.h>


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


static const uint16_t font_11x16[][12] = {
    {0x7efc,0xbc7a,0xc006,0xc006,0xc006,0xc006,0xc006,0xc006,0xbc7a,0x7efc,0x0000},  // 0
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x3c78,0x7efc,0x0000},  // 1
    {0x7e00,0xbd02,0xc386,0xc386,0xc386,0xc386,0xc386,0xc386,0x817a,0x00fc,0x0000},  // 2
    {0x0000,0x8102,0xc386,0xc386,0xc386,0xc386,0xc386,0xc386,0xbd7a,0x7efc,0x0000},  // 3
    {0x00fc,0x0178,0x0380,0x0380,0x0380,0x0380,0x0380,0x0380,0x3d78,0x7efc,0x0000},  // 4
    {0x00fc,0x817a,0xc386,0xc386,0xc386,0xc386,0xc386,0xc386,0xbd02,0x7e00,0x0000},  // 5
    {0x7efc,0xbd7a,0xc386,0xc386,0xc386,0xc386,0xc386,0xc386,0xbd02,0x7e00,0x0000},  // 6
    {0x0000,0x0002,0x0006,0x0006,0x0006,0x0006,0x0006,0x0006,0x3c7a,0x7efc,0x0000},  // 7
    {0x7efc,0xbd7a,0xc386,0xc386,0xc386,0xc386,0xc386,0xc386,0xbd7a,0x7efc,0x0000},  // 8
    {0x00fc,0x817a,0xc386,0xc386,0xc386,0xc386,0xc386,0xc386,0xbd7a,0x7efc,0x0000},  // 9
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0F0F,0x0000,0x0000,0x0000,0x0000,0x0000}   // :
};


static uint8_t get8bitfont( uint16_t fontnum , uint8_t row ){
    return ( row == 1 ) ? fontnum & ~( 0xFF00 ) :
        fontnum >> 8 ;
};


static const uint8_t getscale2( uint8_t fontnum , uint8_t row )
{
    uint8_t extracted ;
    uint8_t doubled ;

    if( row == 1 )
    {
        int rightfour = fontnum & ~( 0xF << 4 ) ;

        for ( int i = 0,k=0 ; i < 4 ; i++,k++ )
        {
            unsigned int mask = 1 << i ;
            extracted = ( rightfour  & mask ) >> i ;
            doubled = doubled | ( extracted << k++ ) ;
            doubled = doubled | ( extracted << k ) ;
        }

    }else{
        int leftfour = fontnum >> 4  ;

        for ( int i = 0,k=0 ; i < 4 ; i++,k++ )
        {
            unsigned int mask = 1 << i ;
            extracted = ( leftfour  & mask ) >> i ;
            doubled = doubled | ( extracted << k++ ) ;
            doubled = doubled | ( extracted << k ) ;
        }
    }

    return doubled ;
};


static void fb_clear(void)
{
    for ( uint16_t i = 0; i < 1024; i++) framebuf[i] = 0 ;
    for ( uint16_t i = 0 ; i< 512 ; i++ ) tstframebuf[i] = 0 ;
}


static void fb_draw_char(uint8_t col, uint8_t page, char c)
{
    uint8_t idx;
    if (c >= '0' && c <= '9') idx = c - '0' ;
    else if (c == ':') idx = 10 ;
    else return ;

    uint16_t offset = page * 128 + col ;
    uint16_t offsetrow2 = ( page+1 ) * 128 + col ;
    for (uint8_t i = 0 ; i < 12 ; i++ ) {
        if (offset + i < 1024) {
            framebuf[ offset + i ] = get8bitfont( font_11x16[idx][i] , 1 ) ;
            framebuf[ offsetrow2 + i ] = get8bitfont( font_11x16[ idx ][ i ] , 2 ) ;
            //framebuf[ offsetrow2 + i ] = getscale2( font_5x7[ idx ][ i ] , 2 ) ;
            //framebuf[ offset + i ] = font_5x7[idx][i] ;
        }
    }

}


static void fb_draw_char2x7( uint8_t col , uint8_t page , char c )
{
    uint8_t idx;
    if (c >= '0' && c <= '9') idx = c - '0' ;
    else if (c == ':') idx = 10 ;
    else return ;

    uint16_t offset = 896+40 ;//page *128 + 90 ;

    for (uint8_t i = 0 ; i < 5 ; i++ ) {
        if (offset + i < 1024) {
            framebuf[ offset + i ] = 0Xff ;//font_5x7[idx][i] ;
        }
    }

}


/*
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
    tstframebuf[offset] = 0xFF ;
}*/



static void fb_draw_string( uint8_t col, uint8_t page, const char *s )
{
    while (*s) {
        fb_draw_char( col, page, *s++  );
        col += 11 ;  /* set 6 for 5 pixels + 1 space */
    }
}

static void fb_draw_bar( uint32_t seconduint )
{
        uint16_t offset = 896 ;
        static uint16_t col ;

        for( uint8_t i = 0 ; i < col ; i++ )
        {
            if( offset + i < 1024 )
                framebuf[ offset + i ] = 0xf ;
        }
        col++ ;

        if ( col > 127)
            col=0 ;
        //framebuf[ offsetrow2 + i ] = get8bitfont( font_11x16[ idx ][ i ] , 2 ) ;
}

static void oled_flush(void)
{
    oled_cmd(0x21); oled_cmd(0); oled_cmd(127) ;  /* Column range */
    oled_cmd(0x22); oled_cmd(0); oled_cmd(7) ;    /* Page range */
    //oled_data( framebuf, 1024 );
    oled_data( framebuf , 1024 ) ;
}
