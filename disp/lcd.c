/*
lcdpcf8574 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <arch/soc.h>

#include "lcd.h"


//#define LCD_PCF8574_DEVICEID 7 //device id, addr = pcf8574 base addr + LCD_PCF8574_DEVICEID

/** 
 *  @name  Definitions for Display Size 
 *  Change these definitions to adapt setting to your display
 */
#define LCD_LINES           4     /**< number of visible lines of the display */
#define LCD_DISP_LENGTH    20     /**< visibles characters per line of the display */
#define LCD_LINE_LENGTH  0x40     /**< internal line length of the display    */
#define LCD_START_LINE1  0x00     /**< DDRAM address of first char of line 1 */
#define LCD_START_LINE2  0x40     /**< DDRAM address of first char of line 2 */
#define LCD_START_LINE3  0x14     /**< DDRAM address of first char of line 3 */
#define LCD_START_LINE4  0x54     /**< DDRAM address of first char of line 4 */
#define LCD_WRAP_LINES      1     /**< 0: no wrap, 1: wrap at end of visibile line */



#define LCD_DATA0_PIN    4            /**< pin for 4bit data bit 0  */
#define LCD_DATA1_PIN    5            /**< pin for 4bit data bit 1  */
#define LCD_DATA2_PIN    6            /**< pin for 4bit data bit 2  */
#define LCD_DATA3_PIN    7            /**< pin for 4bit data bit 3  */
#define LCD_RS_PIN       0            /**< pin  for RS line         */
#define LCD_RW_PIN       1            /**< pin  for RW line         */
#define LCD_E_PIN        2            /**< pin  for Enable line     */
#define LCD_LED_PIN      3            /**< pin  for Led             */

/**
 *  @name Definitions for LCD command instructions
 *  The constants define the various LCD controller instructions which can be passed to the 
 *  function lcd_command(), see HD44780 data sheet for a complete description.
 */

/* instruction register bit positions, see HD44780U data sheet */
#define LCD_CLR               0      /* DB0: clear display                  */
#define LCD_HOME              1      /* DB1: return to home position        */
#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */
#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */
#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */
#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */
#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */

/* function set: set interface data length and number of display lines */
#define LCD_FUNCTION_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */
#define LCD_FUNCTION_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */


#define LCD_MODE_DEFAULT     ((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) )

#define lcd_e_delay_us()   {for(int c = 0; c < 1; c++);}//__asm__ __volatile__( "rjmp 1f\n 1:" );
#define lcd_e_toggle()  toggle_e(self)

#if LCD_LINES==1
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_1LINE 
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES 
#endif

#define set_pin(pin) pio_write_pin(self->port, pin, 1)
#define clear_pin(pin) pio_write_pin(self->port, pin, 0)
#define write_byte(byte) pio_write_word(self->port, 0, (byte) & 0xff)
#define read_pin(pin) pio_read_pin(self->port, (pin))

/* toggle Enable Pin to initiate write */
static void toggle_e(struct lcd *self)
{
	set_pin(LCD_E_PIN);
	lcd_e_delay_us();
	clear_pin(LCD_E_PIN);
}


/*************************************************************************
Low-level function to write byte to LCD controller
Input:    data   byte to write to LCD
          rs     1: write data    
                 0: write instruction
Returns:  none
*************************************************************************/
static void lcd_write(struct lcd *self, uint8_t data,uint8_t rs) 
{
	if (rs) /* write data        (RS=1, RW=0) */
		self->dataport |= _BV(LCD_RS_PIN);
	else /* write instruction (RS=0, RW=0) */
		self->dataport &= ~_BV(LCD_RS_PIN);
	self->dataport &= ~_BV(LCD_RW_PIN);
	//self->dataport |= _BV(LCD_LED_PIN); 
	write_byte(self->dataport);

	self->dataport &= ~0xf0; 
	self->dataport |= data & 0xf0; 
	write_byte(self->dataport);
	lcd_e_toggle();

	/* output low nibble */
	self->dataport &= ~0xf0; 
	self->dataport |= data << 4; 
	write_byte(self->dataport);
	lcd_e_toggle();

	self->dataport |= 0xf0; 
	write_byte(self->dataport);
}


/*************************************************************************
Low-level function to read byte from LCD controller
Input:    rs     1: read data    
                 0: read busy flag / address counter
Returns:  byte read from LCD controller
*************************************************************************/

static uint8_t lcd_read(struct lcd *self, uint8_t rs) 
{
  uint8_t data = 0;
	if (rs) // data
		self->dataport |= _BV(LCD_RS_PIN);
	else // instruction
		self->dataport &= ~_BV(LCD_RS_PIN);
	self->dataport |= _BV(LCD_RW_PIN);
	self->dataport &= ~_BV(LCD_E_PIN); 
	write_byte(self->dataport);

	
	set_pin(LCD_E_PIN);
	lcd_e_delay_us();
	data = read_pin(LCD_DATA3_PIN) << 4; /* read high nibble first */
	clear_pin(LCD_E_PIN);
	lcd_e_delay_us(); /* Enable 500ns low */
	set_pin(LCD_E_PIN);
	lcd_e_delay_us();
	data |= read_pin(LCD_DATA3_PIN) &0x0F; /* read low nibble */
	clear_pin(LCD_E_PIN);
	
	//uart_printf("%02x ", data); 
	return data;
}


/*************************************************************************
loops while lcd is busy, returns address counter
*************************************************************************/
static uint8_t lcd_waitbusy(struct lcd *self)

{
	register uint8_t c;
	
	/* wait until busy flag is cleared */
	while ( (c=lcd_read(self, 0)) & (1<<LCD_BUSY)) {}
	
	//delay_us(2000); 
	/* the address counter is updated 4us after the busy flag is cleared */
	delay_us(2);

	/* now read the address counter */
	return (lcd_read(self, 0));  // return address counter
}/* lcd_waitbusy */


/*************************************************************************
Move cursor to the start of next line or to the first line if the cursor 
is already on the last line.
*************************************************************************/
static inline void lcd_newline(struct lcd *self, uint8_t pos)
{
    register uint8_t addressCounter;


#if LCD_LINES==1
    addressCounter = 0;
#endif
#if LCD_LINES==2
    if ( pos < (LCD_START_LINE2) )
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
#endif
#if LCD_LINES==4
    if ( pos < LCD_START_LINE3 )
        addressCounter = LCD_START_LINE2;
    else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4) )
        addressCounter = LCD_START_LINE3;
    else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2) )
        addressCounter = LCD_START_LINE4;
    else 
        addressCounter = LCD_START_LINE1;
#endif
    lcd_command(self, (1<<LCD_DDRAM)+addressCounter);

}/* lcd_newline */


/*
** PUBLIC FUNCTIONS 
*/

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(struct lcd *self, uint8_t cmd)
{
    lcd_waitbusy(self);
    lcd_write(self, cmd,0);
}


/*************************************************************************
Send data byte to LCD controller 
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(struct lcd *self, uint8_t data)
{
    lcd_waitbusy(self);
    lcd_write(self, data,1);
}



/*************************************************************************
Set cursor to specified position
Input:    x  horizontal position  (0: left most position)
          y  vertical position    (0: first line)
Returns:  none
*************************************************************************/
void lcd_gotoxy(struct lcd *self, uint8_t x, uint8_t y)
{
#if LCD_LINES==1
    lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE1+x);
#endif
#if LCD_LINES==2
    if ( y==0 ) 
        lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else
        lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE2+x);
#endif
#if LCD_LINES==4
    if ( y==0 )
        lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else if ( y==1)
        lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE2+x);
    else if ( y==2)
        lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE3+x);
    else /* y==3 */
        lcd_command(self, (1<<LCD_DDRAM)+LCD_START_LINE4+x);
#endif

}/* lcd_gotoxy */


/*************************************************************************
*************************************************************************/
/*static int lcd_getxy(struct lcd *self)
{
    return lcd_waitbusy(self);
}*/

/*************************************************************************
Clear display and set cursor to home position
*************************************************************************/
void lcd_clrscr(struct lcd *self)
{
    lcd_command(self, 1<<LCD_CLR);
}


/*************************************************************************
Set illumination pin
*************************************************************************/
void lcd_led(struct lcd *self, uint8_t onoff)
{
	if(onoff)
		self->dataport |= _BV(LCD_LED_PIN);
	else
		self->dataport &= ~_BV(LCD_LED_PIN);
	write_byte(self->dataport);
}


/*************************************************************************
Set cursor to home position
*************************************************************************/
void lcd_home(struct lcd *self)
{
    lcd_command(self, 1<<LCD_HOME);
}


/*************************************************************************
Display character at current cursor position 
Input:    character to be displayed                                       
Returns:  none
*************************************************************************/
void lcd_putc(struct lcd *self, char c)
{
    uint8_t pos;

    pos = lcd_waitbusy(self);   // read busy-flag and address counter
    if (c=='\n')
    {
        lcd_newline(self, pos);
    }
    else
    {
#if LCD_WRAP_LINES==1
#if LCD_LINES==1
        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }
#elif LCD_LINES==2
        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        }else if ( pos == LCD_START_LINE2+LCD_DISP_LENGTH ){
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }
#elif LCD_LINES==4
        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        }else if ( pos == LCD_START_LINE2+LCD_DISP_LENGTH ) {
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE3,0);
        }else if ( pos == LCD_START_LINE3+LCD_DISP_LENGTH ) {
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE4,0);
        }else if ( pos == LCD_START_LINE4+LCD_DISP_LENGTH ) {
            lcd_write(self, (1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }
#endif
        lcd_waitbusy(self);
#endif
        lcd_write(self, c, 1);
    }

}/* lcd_putc */


/*************************************************************************
Display string without auto linefeed 
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(struct lcd *self, const char *s)
/* print string on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = *s++) ) {
        lcd_putc(self, c);
    }

}/* lcd_puts */


/*************************************************************************
Display string from program memory without auto linefeed 
Input:     string from program memory be be displayed                                        
Returns:   none
*************************************************************************/
void lcd_puts_p(struct lcd *self, const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_putc(self, c);
    }

}/* lcd_puts_p */

/*
uint16_t lcd_printf(struct lcd *self, const char *fmt, ...){
	char buf[32]; 
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf(buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	lcd_puts(self, buf);
	return n; 
}
*/

/*************************************************************************
Initialize display and select type of cursor 
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void lcd_init(struct lcd *self, pio_dev_t port, uint8_t dispAttr)
{
	(void)(dispAttr); 
	self->port = port;
	
	#if LCD_PCF8574_INIT == 1
	//init pcf8574
	pcf8574_init();
	#endif
	self->dataport = 0;
	write_byte(self->dataport);
	
	// wait for display to power up
	delay_us(16000); 
	
	/* initial write to lcd is 8bit */
	self->dataport |= 0x30; 
	write_byte(self->dataport);
	lcd_e_toggle();
	
	delay_us(4992); // delay, busy flag can't be checked here 
	lcd_e_toggle();
	
	delay_us(64); 
	// repeat last command third time
	lcd_e_toggle();
	delay_us(64); 
	
	/* now configure for 4bit mode */
	self->dataport &= ~0xf0;
	self->dataport |= 0x20; 
	write_byte(self->dataport);
	lcd_e_toggle();
	delay_us(64); // some displays need this additional delay
	
	lcd_command(self, LCD_FUNCTION_DEFAULT); 
	lcd_command(self, LCD_DISP_OFF); 
	lcd_clrscr(self);
	lcd_command(self, LCD_MODE_DEFAULT); 
	//lcd_command(dispAttr); 
	
	lcd_command(self, 0x0c); 
	lcd_command(self, 0x06); 
	lcd_data(self, 0x48); 

}/* lcd_init */
