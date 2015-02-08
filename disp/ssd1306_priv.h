#pragma once


#define U8G_ESC_CS(x) 255, (0xd0 | ((x)&0x0f))
#define U8G_ESC_ADR(x) 255, (0xe0 | ((x)&0x0f))
#define U8G_ESC_RST(x) 255, (0xc0 | ((x)&0x0f))
#define U8G_ESC_END 255, 254

static const uint8_t cmd_init[] PROGMEM = {
	U8G_ESC_CS(0),             //disable chip
	U8G_ESC_ADR(0),           /* instruction mode */
	U8G_ESC_RST(1),           /* do reset low pulse with (1*16)+2 milliseconds */
	U8G_ESC_CS(1),             /* enable chip */
	
	0x0ae,				/* display off, sleep mode */
	0x0d5, 0x081,		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
	0x0a8, 0x03f,		/* multiplex ratio */
	0x0d3, 0x000,	0x00,	/* display offset */
	//0x040,				/* start line */
	0x08d, 0x14,		/* charge pump setting (p62): 0x014 enable, 0x010 disable */
	0x020, 0x00, // memory addr mode
	0x0a1,				/* segment remap a0/a1*/
	0x0a5, // display on
	0x0c8,				/* c0: scan dir normal, c8: reverse */
	0x0da, 0x012,		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
	0x081, 0x09f,		/* set contrast control */
	0x0d9, 0x011,		/* pre-charge period */
	0x0db, 0x020,		/* vcomh deselect level */
	0x021, 0x00, 0x7f, // (7d for 125 col screen!) column addressing mode
	0x022, 0x00, 0x07,		/* page addressing mode WRONG: 3 byte cmd! */
	0x0a4,				/* output ram to display */
	0x0a6,				/* none inverted normal display mode */
	0x0af,				/* display on */

	U8G_ESC_CS(0),             /* disable chip */
	U8G_ESC_END                /* end of sequence */
};

static const unsigned char cmd_home[] PROGMEM = {
	U8G_ESC_ADR(0),           // instruction mode 
	U8G_ESC_CS(1),             // enable chip 
	0x000, // | (col & 0x0f),		// set lower 4 bit of the col adr to 0 
	0x010, // | ((col >> 4) & 0x0f),		// set higher 4 bit of the col adr to 0 
	0x0b0, // | row & 0x0f,  	// page address
	U8G_ESC_END,                // end of sequence 
};
