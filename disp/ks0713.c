/*
 *                  Copyright 2014 ARTaylor.co.uk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Richard Taylor (richard@artaylor.co.uk)
 */

/* Description:
 *
 * This is an LCD driver for the lcd found on FS-T6 series RC transmitters. 
 * There are low, medium and high level functions for drawing text
 * and basic graphics to the display buffer.
 * lcd_update() must be called to send the buffer to the LCD.
 *
 */

#include <string.h>

#include <arch/soc.h>

#include "ks0713.h"

#define KS0713_CHAR_WIDTH 6
#define KS0713_CHAR_HEIGHT 8

#define KS0713_CONTRAST_MIN	25
#define KS0713_CONTRAST_MAX	60

//#define KS0713_HEIGHT				7
//#define KS0713_WIDTH				5

#define KS0713_PIN_MASK	(0x1FFF)

#define KS0713_DATA		(0xFF)		// D0-D7
#define KS0713_RD			(1 << 8)	// RD / E
#define KS0713_WR			(1 << 9)	// RD / #WR
#define KS0713_A0			(1 << 10)	// A0 / RS / Data / #CMD
#define KS0713_RES			(1 << 11)	// Reset
#define KS0713_CS1			(1 << 12)	// Chip Select 1

//#define KS0713_BACKLIGHT	(1 << 2)
#define KS0713_BACKLIGHT	(1 << 13)

#define KS0713_DISP_ON_OFF		(0xAE)
#define KS0713_DISPLAY_LINE		(0x40)
#define KS0713_SET_REF_VOLTAGE	(0x81)	// 2-byte cmd
#define KS0713_SET_PAGE_ADDR	(0xB0)
#define KS0713_SET_COL_ADDR_MSB	(0x10)
#define KS0713_SET_COL_ADDR_LSB	(0x00)
#define KS0713_ADC_SELECT		(0xA0)
#define KS0713_REVERSE_DISP		(0xA6)
#define KS0713_ENTIRE_DISP		(0xA4)
#define KS0713_LCD_BIAS			(0xA2)
#define KS0713_SET_MOD_READ		(0xE0)
#define KS0713_UNSET_MOD_READ	(0xEE)
#define KS0713_RESET			(0xE2)
#define KS0713_SHL_SELECT		(0xC0)
#define KS0713_POWER_CTRL		(0x28)
#define KS0713_REG_RES_SEL		(0x20)
#define KS0713_STATIC_IND_MODE	(0xAC)	// 2-byte cmd
//#define KS0713_POWER_SAVE		Display off, Entire display ON.

#define FONT_STRIDE				(16 * 5)

#include "lcd_font_5x7.h"

//static const unsigned char *font = font_medium;

/**
  * @brief  Send a command to the LCD.
  * @note	Switch the MPU interface to command mode and send
  * @param  cmd: Command to send
  * @retval None
  */
static void ks0713_send_command(struct ks0713 *self, uint8_t cmd)
{
	self->port_state &= ~(KS0713_WR | KS0713_DATA | KS0713_A0); 
	self->port_state |= cmd; 
	self->write_word(self, self->port_state); 
	
	self->port_state &= ~KS0713_CS1;
	self->port_state |= KS0713_RD;
	self->write_word(self, self->port_state); 
	
	self->port_state |= KS0713_CS1;
	self->port_state &= ~KS0713_RD;
	self->write_word(self, self->port_state); 
	
	/*uint16_t gpio = GPIO_ReadOutputData(GPIOC);

	gpio &= ~(LCD_WR | LCD_DATA | LCD_A0);
	gpio |= cmd;
	GPIO_Write(GPIOC, gpio);

	// Toggle the Enable lines.
	gpio &= ~LCD_CS1;
	gpio |= LCD_RD;
	GPIO_Write(GPIOC, gpio);
	gpio |= LCD_CS1;
	gpio &= ~LCD_RD;
	GPIO_Write(GPIOC, gpio);*/
}

/**
  * @brief  Send data to the LCD.
  * @note	Switch the MPU interface to data mode and send
  * @param  data: pointer to LCD data
  * @param  len: Number of bytes of data to send
  * @retval None
  */
static void ks0713_send_data(struct ks0713 *self, uint8_t *data, uint16_t len)
{
	self->port_state &= ~(KS0713_WR | KS0713_DATA ); 
	self->port_state |= KS0713_A0; 
	self->write_word(self, self->port_state); 
	
	data += (len-1);
	
	for (uint16_t i=0; i<len; ++i)
	{
		self->port_state &= ~KS0713_DATA;
		self->port_state |= *data--;
		self->write_word(self, self->port_state); 
	
		// Toggle the Enable lines.
		self->port_state &= ~KS0713_CS1;
		self->port_state |= KS0713_RD;
		self->write_word(self, self->port_state); 
		self->port_state |= KS0713_CS1;
		self->port_state &= ~KS0713_RD;
		self->write_word(self, self->port_state); 
	}
	/*uint16_t i;
	uint16_t gpio = GPIO_ReadOutputData(GPIOC);

	gpio &= ~(LCD_WR | LCD_DATA);
	gpio |= LCD_A0;

	data += (len-1);

	for (i=0; i<len; ++i)
	{
		gpio &= ~LCD_DATA;
		gpio |= *data--;
		GPIO_Write(GPIOC, gpio);

		// Toggle the Enable lines.
		gpio &= ~LCD_CS1;
		gpio |= LCD_RD;
		GPIO_Write(GPIOC, gpio);
		gpio |= LCD_CS1;
		gpio &= ~LCD_RD;
		GPIO_Write(GPIOC, gpio);
	}*/
}

/**
  * @brief  Initialise the lcd panel.
  * @note   Sets up the controller and displays our logo.
  * @param  None
  * @retval None
  */
void ks0713_init(struct ks0713 *self, void (*write_word)(struct ks0713 *self, uint16_t word))
{
	self->write_word = write_word; 
	self->contrast = 0x28; 
	self->cursor_x = self->cursor_y = 0; 
	
	// set the pins high
	self->port_state = KS0713_PIN_MASK; 
	self->port_state |= KS0713_BACKLIGHT; 
	self->port_state &= (KS0713_RD | KS0713_WR); 
	
	self->write_word(self, self->port_state); 
	
	self->port_state &= ~KS0713_RES; 
	self->write_word(self, self->port_state); 
	delay_us(5); 
	// now pull reset high
	self->port_state |= KS0713_RES; 
	self->write_word(self, self->port_state); 
	
	delay_us(50); 
	
	ks0713_send_command(self, KS0713_RESET); 				// Initialize the internal functions (Reset)
	ks0713_send_command(self, KS0713_DISP_ON_OFF | 0x00); 	// Turn off LCD panel (DON = 0)
	ks0713_send_command(self, KS0713_ADC_SELECT | 0x01);  	// Select SEG output direction reversed (ADC = 1)
	ks0713_send_command(self, KS0713_REVERSE_DISP | 0x00); 	// Select normal / reverse display (REV = 0)
	ks0713_send_command(self, KS0713_ENTIRE_DISP | 0x00); 	// Select normal display ON (EON = 0)
	ks0713_send_command(self, KS0713_LCD_BIAS | 0x00); 		// Select LCD bias (0)
	ks0713_send_command(self, KS0713_SHL_SELECT | 0x08); 	// Select COM output direction normal (SHL = 0)
	ks0713_send_command(self, KS0713_POWER_CTRL | 0x07); 	// Control power circuit operation (VC,VR,VF on)
	ks0713_send_command(self, KS0713_REG_RES_SEL | 0x04); 	// Select internal resistance ratio (0x05)
	ks0713_send_command(self, KS0713_SET_REF_VOLTAGE); 		// Set reference voltage Mode (2-part cmd)
	ks0713_send_command(self, self->contrast); 					// Set reference voltage register
	ks0713_send_command(self, KS0713_DISP_ON_OFF | 0x01); 	// Turn on LCD panel (DON = 1)

	ks0713_commit(self);
	ks0713_set_backlight(self, 1);
	
	/*GPIO_InitTypeDef gpioInit;

	// Enable the GPIO block clocks and setup the pins.
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_SetBits(GPIOC, LCD_PIN_MASK);
	GPIO_ResetBits(GPIOC, LCD_RD | LCD_WR);

	// Configure the LCD pins.
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = LCD_PIN_MASK;
	GPIO_Init(GPIOC, &gpioInit);

	gpioInit.GPIO_Pin = LCD_BACKLIGHT;
	GPIO_Init(GPIOD, &gpioInit);

	// Reset LCD
	GPIO_ResetBits(GPIOC, LCD_RES);
	delay_us(5);
	GPIO_SetBits(GPIOC, LCD_RES);

	// Wait for reset to complete.
	delay_us(50);

	lcd_send_command(KS0713_RESET); 				// Initialize the internal functions (Reset)
	lcd_send_command(KS0713_DISP_ON_OFF | 0x00); 	// Turn off LCD panel (DON = 0)
	lcd_send_command(KS0713_ADC_SELECT | 0x01);  	// Select SEG output direction reversed (ADC = 1)
	lcd_send_command(KS0713_REVERSE_DISP | 0x00); 	// Select normal / reverse display (REV = 0)
	lcd_send_command(KS0713_ENTIRE_DISP | 0x00); 	// Select normal display ON (EON = 0)
	lcd_send_command(KS0713_LCD_BIAS | 0x00); 		// Select LCD bias (0)
	lcd_send_command(KS0713_SHL_SELECT | 0x08); 	// Select COM output direction normal (SHL = 0)
	lcd_send_command(KS0713_POWER_CTRL | 0x07); 	// Control power circuit operation (VC,VR,VF on)
	lcd_send_command(KS0713_REG_RES_SEL | 0x04); 	// Select internal resistance ratio (0x05)
	lcd_send_command(KS0713_SET_REF_VOLTAGE); 		// Set reference voltage Mode (2-part cmd)
	lcd_send_command(contrast); 					// Set reference voltage register
	lcd_send_command(KS0713_DISP_ON_OFF | 0x01); 	// Turn on LCD panel (DON = 1)

	lcd_update();
	lcd_backlight(TRUE);
	*/
}

/**
  * @brief  Turn the backlight on / off.
  * @note
  * @param  on: Whether the backlight should be on or off.
  * @retval None
  */
void ks0713_set_backlight(struct ks0713 *self, uint8_t on)
{
	if (on)
		self->port_state |= KS0713_BACKLIGHT; 
	else
		self->port_state &= ~KS0713_BACKLIGHT; 
	self->write_word(self, self->port_state); 
}


/**
  * @brief  Set the LCD contrast.
  * @note
  * @param  val: Set contrast to this value.
  * @retval None
  */
void ks0713_set_contrast(struct ks0713 *self, uint8_t val)
{
	self->contrast = val;
	if ((self->contrast + val) > 0xff) self->contrast = 0xff;
	else if (self->contrast + val < 0) self->contrast = 0;

	ks0713_send_command(self, KS0713_SET_REF_VOLTAGE); 		// Set reference voltage Mode (2-part cmd)
	ks0713_send_command(self, self->contrast); 						// Set reference voltage register
}

/**
  * @brief  Transfer frame buffer to LCD.
  * @note
  * @param  None
  * @retval None
  */
void ks0713_commit(struct ks0713 *self)
{
	for (unsigned row = 0; row < KS0713_HEIGHT / 8; ++row){
		ks0713_send_command(self, KS0713_SET_PAGE_ADDR | row);
		ks0713_send_command(self, KS0713_SET_COL_ADDR_LSB | 0x04); // low col
		ks0713_send_command(self, KS0713_SET_COL_ADDR_MSB | 0x00);
		ks0713_send_data(self, self->lcd_buffer + (KS0713_WIDTH * row), KS0713_WIDTH);
	}
}

/**
  * @brief  Set / Clean a specific pixel.
  * @note	Top left is (0,0)
  * @param  x: horizontal pixel location
  * @param  y: vertical pixel location
  * @param  operation: 0 - off, 1 - on, 2 - xor
  * @retval None
  */
void ks0713_write_pixel(struct ks0713 *self, uint8_t x, uint8_t y, ks0713_pixel_op_t op)
{
	if(x > KS0713_WIDTH || y > KS0713_HEIGHT) return; 
	
	switch (op)
	{
	case KS0713_OP_NONE:
		break;
	case KS0713_OP_CLR:
		self->lcd_buffer[x+ (y/8)*KS0713_WIDTH] &= ~(1 << (y%8));
		break;
	case KS0713_OP_SET:
		self->lcd_buffer[x+(y/8)*KS0713_WIDTH] |= (1 << (y%8));
		break;
	case KS0713_OP_XOR:
		self->lcd_buffer[x+ (y/8)*KS0713_WIDTH] ^= (1 << (y%8));
		break;
	}
}

/**
  * @brief  Set cursor position in pixels.
  * @note	Top left is (0,0)
  * @param  x: horizontal cursor location
  * @param  y: vertical cusros location
  * @retval None
  */
void ks0713_move_cursor(struct ks0713 *self, uint8_t x, uint8_t y)
{
	if (((y * KS0713_CHAR_HEIGHT)+KS0713_CHAR_HEIGHT) > KS0713_HEIGHT) return;
	if (((x * KS0713_CHAR_WIDTH) +KS0713_CHAR_WIDTH) > KS0713_WIDTH) return;

	self->cursor_x = x;
	self->cursor_y = y;
}

void ks0713_clear(struct ks0713 *self){
	for(unsigned c = 0; c < sizeof(self->lcd_buffer); c++){
		self->lcd_buffer[c] = 0; 
	}
}

#include "lcd_font_5x7.h"

static void _ks0713_put(tty_dev_t self, uint8_t ch, color_t fg, color_t bg){
	struct ks0713 *dev = container_of(self, struct ks0713, tty); 
	(void)(fg); 
	(void)(bg); 
	const unsigned char *glyph = &lcd_font_5x7[ch * 5]; 
	uint16_t x = dev->cursor_x * KS0713_CHAR_WIDTH; 
	uint16_t y = dev->cursor_y * KS0713_CHAR_HEIGHT; 
	for(unsigned j = 0; j < 5; j++){
		dev->lcd_buffer[x + j + (y/8)*KS0713_WIDTH] = glyph[j];
	}
	dev->lcd_buffer[x + 6 + (y/8)*KS0713_WIDTH + 1] = 0; 
}

static void _ks0713_move_cursor(tty_dev_t self, uint16_t x, uint16_t y){
	struct ks0713 *dev = container_of(self, struct ks0713, tty); 
	ks0713_move_cursor(dev, x, y); 
}

static void _ks0713_get_size(tty_dev_t self, uint16_t *w, uint16_t *h){
	//struct ks0713 *dev = container_of(self, struct tty_device, tty); 
	(void)(self); 
	*w = KS0713_WIDTH / KS0713_CHAR_WIDTH; 
	*h = KS0713_HEIGHT / KS0713_CHAR_HEIGHT; 
}

static void _ks0713_clear(tty_dev_t self){
	struct ks0713 *dev = container_of(self, struct ks0713, tty); 
	ks0713_clear(dev); 
}

tty_dev_t ks0713_get_tty_interface(struct ks0713 *self){
	static struct tty_device _if; 
	_if = (struct tty_device) {
		.put = _ks0713_put, 
		.move_cursor = _ks0713_move_cursor, 
		.get_size = _ks0713_get_size, 
		.clear = _ks0713_clear
	}; 
	self->tty = &_if; 
	return &self->tty; 
}

/**
  * @brief  Write a character.
  * @note
  * @param  c: ASCII character to write
  * @param  op: ks0713_pixel_op_t
  * @param  flags: LCD_FLAGS (CHAR_*)
  * @retval None
  */
  /*
void ks0713_write_char(struct ks0713 *self, uint8_t ch, ks0713_pixel_op_t op)
{
	
	uint8_t x, y;
	uint8_t x1, y1, divX = 1, divY = 1;
	uint8_t height = CHAR_HEIGHT;
	uint8_t width = CHAR_WIDTH;
	uint8_t row = 0;
	ks0713_pixel_op_t op_set = (op==ks0713_pixel_op_t_SET)?ks0713_pixel_op_t_SET:ks0713_pixel_op_t_CLR;
	ks0713_pixel_op_t op_clr = (op==ks0713_pixel_op_t_SET)?ks0713_pixel_op_t_CLR:ks0713_pixel_op_t_SET;

	if ((flags & CHAR_2X) != 0)
	{
		height *= 2;
		width *= 1;
		divY = 2;
	}
	else if ((flags & CHAR_4X) != 0)
	{
		height *=2;
		width *=2;
		divX = 2;
		divY = 2;
	}

	if (op == ks0713_pixel_op_t_XOR)
	{
		op_set = ks0713_pixel_op_t_XOR;
		op_clr = ks0713_pixel_op_t_NONE;
	}

	// condenced chars overlap so do not erase or prev char gets damaged, use CHAR_NOSPACE alternatively
	if (flags & CHAR_CONDENSED)
		op_clr = ks0713_pixel_op_t_NONE;

	if ((cursor_y+height) >= LCD_HEIGHT) return;
	else if ((cursor_x+width) >= LCD_WIDTH) return;

	for (x=0; x<width; x++ )
	{
		row = 0;
		for (y=0; y<height + 1; y++)
		{
			uint8_t d;
			x1 = x / divX;
			y1 = y / divY;
			d = font[ (c*CHAR_WIDTH) + x1 + row*FONT_STRIDE ];
			if (flags & CHAR_UNDERLINE)
				d |= 0x80;

			if (d & (1 << y1%8))
				lcd_set_pixel(cursor_x+x, cursor_y+y, op_set);
			else
				lcd_set_pixel(cursor_x+x, cursor_y+y, op_clr);

			if (y1%8 == 7)
				row++;
		}

		if ((flags & CHAR_CONDENSED) != 0 && x%CHAR_WIDTH==4)
			cursor_x--;
	}
	for (y=0; y<height+1; y++) lcd_set_pixel(cursor_x+width, cursor_y+y, op_clr);

	cursor_x += width + 1;
	if ((flags & (CHAR_CONDENSED | CHAR_NOSPACE)) != 0)
		cursor_x--;

	if (cursor_x >= LCD_WIDTH)
	cursor_y += height + 1;
}
*/
/**
  * @brief  Write a string.
  * @note	Iterate through a null terminated string.
  * @param  s: ASCII string to write
  * @param  op: ks0713_pixel_op_t
  * @param  flags: LCD_FLAGS (CHAR_*)
  * @retval None
  */
  /*
void ks0713_write_string(struct ks0713 *self, const char *s, ks0713_pixel_op_t op)
{
	const char *ptr = s;
	uint8_t n = strlen(s);

	if (flags & ALIGN_RIGHT)
	{
		cursor_x -= n * ((flags & CHAR_CONDENSED) ? CHAR_WIDTH : (CHAR_WIDTH + 1));
	}

	for (ptr = s; n > 0; ptr++, n--)
		lcd_write_char(*ptr, op, flags);

	if( flags & TRAILING_SPACE )
		lcd_write_char(' ', op, flags);
		
}*/


// Reduce stack usage.
/*
static uint8_t tth;
static uint8_t th;
static uint8_t h;
static uint8_t t;
static int16_t u;

void lcd_write_int(int32_t val, ks0713_pixel_op_t op, uint16_t flags)
{
	int count = 0;

	if (val < 0) u = -val;
	else u = val;
	tth = u / 10000;
	u -= tth * 10000;
	th = u / 1000;
	u -= th * 1000;
	h = u / 100;
	u -= h * 100;
	t = u / 10;
	u -= t * 10;

	if (flags & ALIGN_RIGHT)
	{
		if (val < 0) count++;
		//if (val >= 0) lcd_write_char('+', op);

		if (tth > 0) count++;
		if (tth > 0 || th > 0) count++;
		if (tth > 0 || th > 0 || h > 0) count++;
		if (tth > 0 || th > 0 || h > 0 || t > 0 || (flags & INT_DIV10) || (flags & INT_PAD10)) count++;
		if (flags & INT_DIV10) count++;

		if (flags & CHAR_CONDENSED)
			cursor_x -= count * CHAR_WIDTH;
		else
			cursor_x -= count * (CHAR_WIDTH + 1);
	}

	if (val < 0) lcd_write_char('-', op, flags);
	//if (val >= 0) lcd_write_char('+', op);

	if (tth > 0)
		lcd_write_char(tth + '0', op, flags);

	if (tth > 0 || th > 0)
		lcd_write_char(th + '0', op, flags);

	if (tth > 0 || th > 0 || h > 0)
		lcd_write_char(h + '0', op, flags);

	if (tth > 0 || th > 0 || h > 0 || t > 0 || (flags & INT_DIV10) || (flags & INT_PAD10))
		lcd_write_char(t + '0', op, flags);
	if (flags & INT_DIV10)
		lcd_write_char('.', op, flags);

	lcd_write_char(u + '0', op, flags);
	if( flags & TRAILING_SPACE )
		lcd_write_char(' ', op, flags);
}

void lcd_write_hex(uint32_t val, ks0713_pixel_op_t op, uint16_t flags)
{
	int digits = 4;
	int i;
	if (val > 0xFFFF)
		digits = 8;

	for (i=0; i<digits; ++i)
	{
		int digit = (val >> (4*(digits-i-1))) & 0xF;

		if (digit > 9)
			lcd_write_char(digit - 10 + 'A', op, flags);
		else
			lcd_write_char(digit + '0', op, flags);
	}
}
*/
void ks0713_draw_line(struct ks0713 *self, int8_t x0, int8_t y0, int8_t x1, int8_t y1, ks0713_pixel_op_t op)
{
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;){
		ks0713_write_pixel(self, x0, y0, op);
		
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void ks0713_draw_rect(struct ks0713 *self, int8_t x1, int8_t y1, int8_t x2, int8_t y2, ks0713_pixel_op_t op)
{
	int8_t x, y;
	
	if (x1 > x2 || y1 > y2)
		return;

	for (y = y1; y <= y2; ++y)
	{
		for (x = x1; x <= x2; ++x)
		{
			//if ((flags & RECT_FILL) || y == y1 || y == y2 || x == x1 || x == x2)
			if (y == y1 || y == y2 || x == x1 || x == x2)
			{
				/*
				if (flags & RECT_ROUNDED)
				{
					if (!( (x == x1 && y == y1) ||
						   (x == x2 && y == y1) ||
						   (x == x1 && y == y2) ||
						   (x == x2 && y == y2) ))
					{
						ks0713_write_pixel(self, x, y, op);
					}

				}
				else*/
					ks0713_write_pixel(self, x, y, op);
			}
		}
	}
}
/*
char lcd_draw_message(const char *msg, ks0713_pixel_op_t op, ks0713_pixel_op_t op2, char selectedLine)
{
	char line = 1;
	const int width = (LCD_WIDTH - 2*cursor_x) / (CHAR_WIDTH + 1);
	char line_buffer[LCD_WIDTH / (CHAR_WIDTH + 1)];
	int nchars;
	const char *ptr = msg;
	int x = cursor_x;

	// Iterate through the string to print it
	while (*ptr)
	{
		const char *p;
		const char *pspace = 0;
		// find next wrap point
		for (p = ptr; *p ; ++p )
		{
			// remember last place with space
			if( *p == ' ' ) pspace = p;
			// out of width so it's a break place
			if( p-ptr > width )
			{
				p = pspace ? pspace : p;
				break;
			}
			// new line - must break
			if( *p == '\n' )
			{
				break;
			}
		}
		nchars = p - ptr;
		memcpy(line_buffer, ptr, nchars);
		line_buffer[nchars] = 0;
		cursor_x = x + (width - nchars) * (CHAR_WIDTH + 1) / 2;
		lcd_write_string(line_buffer, line == selectedLine ? op2 : op, FLAGS_NONE);
		cursor_y += (CHAR_HEIGHT + 1);
		line++;
		ptr = *p ? p+1 : p;
	}
	return line-1;
}

*/
