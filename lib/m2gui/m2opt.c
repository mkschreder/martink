/*
  
  mnuopt.c
  
  A parser for the string OPTions

  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2011  olikraus@gmail.com

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


  Syntax:
  options := {<cmd>}
  cmd:= <char> <arg>
  char:= single character
  arg:= <value>

  value
    a value from 0 to 255
    value 255 has a special meaning: it means: argument is not present, so "w255" is the same as ""
    
    
  Code size status: 
    Proccedure attributes: Optimization completed
    Arithmetic optimization: Done

*/

#include "m2.h"

m2_rom_char_p m2_parser_str;

static void m2_parser_inc_str(void) M2_NOINLINE;
static char m2_parser_get_c(void) M2_NOINLINE;

static void m2_parser_inc_str(void)
{
  m2_parser_str++;
}

static char m2_parser_get_c(void)
{
#if defined(M2_AVR_OPT_ROM)
  return m2_rom_low_level_get_byte(m2_parser_str);
#else
  return *m2_parser_str;
#endif
}

static uint8_t m2_parser_is_digit(void)
{
  register char c;
  c = m2_parser_get_c();
  if ( c >= '0' && c <= '9' )
    return 1;
  return 0;
}

static void m2_parser_skip_space(void)
{
  register char c;
  for(;;)
  {
    c = m2_parser_get_c();
    if ( c == '\0' )
      break;
    if ( c > ' ')
      break;
    m2_parser_inc_str();
  }
}

static uint8_t m2_parser_get_val(void)
{
  register uint8_t val = 0;
  register int8_t c;
  for(;;)
  {
    c = m2_parser_get_c();
    if ( c >= '0' && c <= '9' )
    {
      val *= 10;
      val += c;
      val -= '0';
    }
    else
      break;
    m2_parser_inc_str();
  }
  m2_parser_skip_space();
  return val;
}

static void m2_parser_skip_cmd(void)
{
  /* skip command byte */
  m2_parser_inc_str();
  m2_parser_skip_space();
  if ( m2_parser_is_digit() )
    m2_parser_get_val();
}

static uint8_t m2_parser_find_cmd(char cmd)
{
  register char c;
  m2_parser_skip_space();  
  for(;;)
  {
    c = m2_parser_get_c();
    if ( c == '\0' )
      return 0;
    if ( c == cmd )
      return 1;
    m2_parser_skip_cmd();
  }
}

/*==============================================================*/

/* only used for m2tklab */
static uint8_t m2_calc_cmd_cnt(void)
{
  register char c;
  uint8_t cnt = 0;
  m2_parser_skip_space();  
  for(;;)
  {
    c = m2_parser_get_c();
    if ( c == '\0' )
      break;
    cnt++;
    m2_parser_skip_cmd();
  }
  return cnt;
}

/* only used for m2tklab */
static char m2_calc_nth_cmd(uint8_t n)
{
  register char c;
  uint8_t cnt = 0;
  m2_parser_skip_space();  
  for(;;)
  {
    c = m2_parser_get_c();
    if ( c == '\0' )
      break;
    if ( cnt == n )
      return c;
    cnt++;    
    m2_parser_skip_cmd();
  }
  return '\0';
}

/*==============================================================*/

static void m2_parser_set_str(m2_rom_void_p str)
{
  m2_parser_str = str;
}

static uint8_t m2_parser_get_cmd_val(char cmd, uint8_t not_found_val)
{
  if ( m2_parser_find_cmd(cmd) != 0 )
  {
    m2_parser_inc_str();
    m2_parser_skip_space();
    return m2_parser_get_val();
  }
  return not_found_val;
}

/*==============================================================*/

uint8_t m2_get_cmd_cnt(m2_rom_char_p str)
{
  m2_parser_set_str(str);
  return m2_calc_cmd_cnt();
}

char m2_get_nth_cmd(m2_rom_char_p str, uint8_t n)
{
  m2_parser_set_str(str);
  return m2_calc_nth_cmd(n);
}


/*==============================================================*/

uint8_t m2_opt_get_val_any_default(m2_rom_char_p str, char cmd, uint8_t default_value)
{
  if ( str == NULL )
    return default_value;
  m2_parser_set_str(str);
  return m2_parser_get_cmd_val(cmd, default_value);
}


uint8_t m2_opt_get_val(m2_rom_char_p str, char cmd)
{
  return m2_opt_get_val_any_default(str, cmd, M2_OPT_NOT_FOUND);
}

uint8_t m2_opt_get_val_zero_default(m2_rom_char_p str, char cmd)
{
  return m2_opt_get_val_any_default(str, cmd, 0);
}


/*==============================================================*/

