#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>


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
    SPDR = data ;
    while( !( SPSR & ( 1 << SPIF ) )) ;
    return SPDR ;
}


static void oled_cmd( uint8_t cmd )
{
	PORTB &= ~( 1 << OLED_DC );
	PORTB &= ~( 1 << OLED_CS );
	spi_transfer( cmd );
	PORTB |= ( 1 << OLED_CS );
}


static void oled_data( const uint8_t * buf , uint16_t len )
{
	PORTB |= ( 1 << OLED_DC ) ;
	PORTB &= ~( 1 << OLED_CS ) ;
	for( uint16_t i = 0 ; i < len ; i++ ){
		spi_transfer(buf[i]);	
	}	
	PORTB |= ( 1 << OLED_CS );
}


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
    {0x7E, 0x11, 0x11, 0x11, 0x7E},// A
    {0x7F, 0x49, 0x49, 0x49, 0x36},// B
    {0x3E, 0x41, 0x41, 0x41, 0x22},// C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},// D
    {0x7F, 0x49, 0x49, 0x49, 0x41},// E
    {0x7F, 0x09, 0x09, 0x01, 0x01},// F
    {0x3E, 0x41, 0x41, 0x51, 0x32},// G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},// H
    {0x00, 0x41, 0x7F, 0x41, 0x00},// I
    {0x20, 0x40, 0x41, 0x3F, 0x01},// J
    {0x7F, 0x08, 0x14, 0x22, 0x41},// K
    {0x7F, 0x40, 0x40, 0x40, 0x40},// L
    {0x7F, 0x02, 0x04, 0x02, 0x7F},// M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},// N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},// O
    {0x7F, 0x09, 0x09, 0x09, 0x06},// P
    {0x3E, 0x41, 0x51, 0x21, 0x5E},// Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},// R
    {0x46, 0x49, 0x49, 0x49, 0x31},// S
    {0x01, 0x01, 0x7F, 0x01, 0x01},// T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},// U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},// V
    {0x7F, 0x20, 0x18, 0x20, 0x7F},// W
    {0x63, 0x14, 0x08, 0x14, 0x63},// X
    {0x03, 0x04, 0x78, 0x04, 0x03},// Y
    {0x61, 0x51, 0x49, 0x45, 0x43}// Z
};


static void fb_draw_bar( uint8_t col , uint8_t page , uint8_t width )
{
    uint16_t offset = page * 128 + col ;
    for ( uint8_t i = 0 ; i < width && ( offset + i ) < 1024 ; i++ )
    {
        framebuf[ offset + i ] = 0x7E ;
    }
}


static void fb_draw_arrow( uint8_t col , uint8_t page , char dir)
{
    uint16_t offset = page *  128 + col ;
    if( dir == '<'){
        uint8_t arrow[] = {0x00, 0x08, 0x14, 0x22, 0x41} ;// <
        for( uint8_t i = 0 ; i< 5 &&(offset+i) < 1024 ; i++ ){
            framebuf[offset + i ] = arrow[i] ;
        }
    }else if( dir == '>'){
        uint8_t arrow[] = { 0x41, 0x22, 0x14, 0x08, 0x00 };// >
        //uint8_t arrow[] = { 0x08, 0x2A, 0x1C, 0x08 } ;// ->

        for( uint8_t i = 0 ; i < 5 && ( offset+i) < 1024 ; i++){
            //framebuf[ offset+i ] = arrow[ 6 - i ];
            framebuf[ offset+i ] = arrow[ i ];
        }
    }else if( dir == '^' )
    {
        uint8_t arrow[] ={ 0x04, 0x02, 0x01, 0x02, 0x04 } ;// ^
        for( uint8_t i = 0 ; i < 5 && ( offset+i) < 1024 ; i++){
        framebuf[ offset+i ] = arrow[ i ] ;
        };

    }else{
        uint8_t eq[] = {0x00, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00};
        for( uint8_t i = 0 ; i < 7 && ( offset+i) < 1024 ; i++ ){
            framebuf[ offset + i ] = eq[i] ;
        }
    }
}


static uint8_t get8bitfont( uint16_t fontnum , uint8_t row )
{
    return ( row == 1 ) ? fontnum & ~( 0xFF00 ) :
        fontnum >> 8 ;
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
    else if( c > '9' && c < 'Z' ) idx = c - 54 ;
    else if ( c == ':' ) idx = 10 ;
    else return ;

    uint16_t offset = page * 128 + col ;

    for (uint8_t i = 0 ; i < 5 ; i++ ) {
        if ( offset + i < 1024 ) {
            framebuf[ offset + i ] = font_5x7[idx][i] ;
        }
    }
}


static void fb_draw_string( uint8_t col, uint8_t page, const char *s )
{
    while (*s) {
        fb_draw_char( col, page, *s++  );
        col += 6 ;  /* set 6 for 5 pixels + 1 space */
    }
}


static void oled_flush(void)
{
    oled_cmd(0x21); oled_cmd(0); oled_cmd(127) ;  /* Column range */
    oled_cmd(0x22); oled_cmd(0); oled_cmd(7) ;    /* Page range */
    //oled_data( framebuf, 1024 );
    oled_data( framebuf , 1024 ) ;
}
