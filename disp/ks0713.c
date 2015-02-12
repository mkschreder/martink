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
	self->port_state &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
	self->port_state |= (KS0713_RD | (uint16_t)cmd);
	self->putn(self, &self->port_state, 1); 
	
	self->port_state |= KS0713_CS1;
	self->port_state &= ~KS0713_RD;
	self->putn(self, &self->port_state, 1); 
	
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
  * @brief  Initialise the lcd panel.
  * @note   Sets up the controller and displays our logo.
  * @param  None
  * @retval None
  */
void ks0713_init(struct ks0713 *self, void (*putn)(struct ks0713 *self, uint16_t *data, size_t size))
{
	self->putn = putn; 
	self->contrast = 0x28; 
	self->cursor_x = self->cursor_y = 0; 
	
	// set the pins high
	self->port_state = KS0713_PIN_MASK; 
	self->port_state |= KS0713_BACKLIGHT; 
	self->port_state &= (KS0713_RD | KS0713_WR); 
	
	self->putn(self, &self->port_state, 1); 
	
	self->port_state &= ~KS0713_RES; 
	self->putn(self, &self->port_state, 1); 
	delay_us(5); 
	// now pull reset high
	self->port_state |= KS0713_RES; 
	self->putn(self, &self->port_state, 1); 
	
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

	memset(self->lcd_buffer, 0, sizeof(self->lcd_buffer)); 
	
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
	self->putn(self, &self->port_state, 1); 
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

#ifdef CONFIG_SAVE_MEMORY
	// do not use large command buffer if we have to save memory
	// instead send each word by itself (3ms on 24mhz stm32)
	void ks0713_commit(struct ks0713 *self)
	{
		uint16_t word = self->port_state; 
		
		for (unsigned row = 0; row < KS0713_HEIGHT / 8; ++row){
			word &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
			word |= (KS0713_RD | (uint16_t)(KS0713_SET_PAGE_ADDR | row));
			self->putn(self, &word, 1); 
			
			word |= KS0713_CS1;
			word &= ~KS0713_RD;
			self->putn(self, &word, 1); 
				
			word &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
			word |= (KS0713_RD | (uint16_t)(KS0713_SET_COL_ADDR_LSB | 0x04));
			self->putn(self, &word, 1); 
				
			word |= KS0713_CS1;
			word &= ~KS0713_RD;
			self->putn(self, &word, 1); 
				
			word &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
			word |= (KS0713_RD | (uint16_t)(KS0713_SET_COL_ADDR_MSB | 0x00));
			self->putn(self, &word, 1); 
			
			word |= KS0713_CS1;
			word &= ~KS0713_RD;
			self->putn(self, &word, 1); 
				
			word &= ~(KS0713_WR | KS0713_DATA ); 
			word |= KS0713_A0; 
			
			uint8_t *data = self->lcd_buffer + (KS0713_WIDTH * row); 
			data += (KS0713_WIDTH-1);
			
			for (uint16_t i=0; i<KS0713_WIDTH; ++i)
			{
				word &= ~(KS0713_CS1 | KS0713_DATA);
				word |= (KS0713_RD | (uint16_t)(*data--));
				self->putn(self, &word, 1); 
			
				word |= KS0713_CS1;
				word &= ~KS0713_RD;
				self->putn(self, &word, 1); 
			}
		}
	}
#else 
	// this one takes ~1ms on 24mhz stm32 without dma. 
	// construction of command buffer takes ~400us. 
	void ks0713_commit(struct ks0713 *self)
	{
		const int cmd_buf_size = ((KS0713_HEIGHT / 8) * (KS0713_WIDTH * 2 + 6)); 
		uint16_t cmd_buffer[cmd_buf_size]; 
		uint16_t *cmd = cmd_buffer; 
		uint16_t word = self->port_state; 
		
		for (unsigned row = 0; row < KS0713_HEIGHT / 8; ++row){
			word &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
			word |= (KS0713_RD | (uint16_t)(KS0713_SET_PAGE_ADDR | row));
			*cmd++ = word; 
			
			word |= KS0713_CS1;
			word &= ~KS0713_RD;
			*cmd++ = word; 
				
			word &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
			word |= (KS0713_RD | (uint16_t)(KS0713_SET_COL_ADDR_LSB | 0x04));
			//self->putn(self, &self->port_state, 1); 
			*cmd++ = word; 
				
			word |= KS0713_CS1;
			word &= ~KS0713_RD;
			*cmd++ = word; 
				
			word &= ~(KS0713_CS1 | KS0713_WR | KS0713_DATA | KS0713_A0);
			word |= (KS0713_RD | (uint16_t)(KS0713_SET_COL_ADDR_MSB | 0x00));
			*cmd++ = word; 
			
			word |= KS0713_CS1;
			word &= ~KS0713_RD;
			*cmd++ = word; 
				
			word &= ~(KS0713_WR | KS0713_DATA ); 
			word |= KS0713_A0; 
			
			uint8_t *data = self->lcd_buffer + (KS0713_WIDTH * row); 
			data += (KS0713_WIDTH-1);
			
			for (uint16_t i=0; i<KS0713_WIDTH; ++i)
			{
				word &= ~(KS0713_CS1 | KS0713_DATA);
				word |= (KS0713_RD | (uint16_t)(*data--));
				*cmd++ = word; 
				
				word |= KS0713_CS1;
				word &= ~KS0713_RD;
				*cmd++ = word; 
			}
			
			//self->putn(self, cmd_buffer, KS0713_WIDTH * 2);
			//ks0713_send_data(self, self->lcd_buffer + (KS0713_WIDTH * row), KS0713_WIDTH);
		}
		// commit data to the lcd 
		self->putn(self, cmd_buffer, cmd_buf_size); 
	}
#endif // save memory

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
