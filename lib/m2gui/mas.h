/*

  mas.h
  
  mass storage interface subsystem

  part of m2tklib (Mini Interative Interface Toolkit Library)
  
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

*/



#ifndef _MAS_H
#define _MAS_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif



#define MAS_PATH_MAX 64


/*
  This must be '/' for the  Petit FAT File System Module (http://elm-chan.org/fsw/ff/00index_p.html)
*/
#define MAS_DIR_SEP '/'


typedef uint8_t mas_device_fn(uint8_t msg, void *arg);

/* INIT: arg is ptr to mas_arg_init_t */
#define MAS_MSG_INIT 0
/* GET_DIR_ENTRY_AT_POS: arg is ptr to mas_arg_get_dir_entry_at_pos_t */
#define MAS_MSG_GET_DIR_ENTRY_AT_POS 1
/* GET_DIR_ENTRY_CNT: arg is ptr to mas_arg_get_dir_entry_cnt_t */
#define MAS_MSG_GET_DIR_ENTRY_CNT 2


#ifdef MAS_SD_INTERFACE
/* OPEN_READ: arg is pointer to const char *pathname */
#define MAS_MSG_OPEN_READ 10
/* READ: arg is pointer to mas_arg_read_t */
#define MAS_MSG_READ 11
/* READ_BYTE: arg is NULL */
#define MAS_MSG_READ_BYTE 12
/* GET_POS: arg is mas_arg_file_pos */
#define MAS_MSG_GET_POS 30
/* SET_POS: arg is mas_arg_file_pos */
#define MAS_MSG_SET_POS 31
#endif


struct _mas_arg_init
{
  /* provided input */
  uint8_t cs_pin;
};
typedef struct _mas_arg_init mas_arg_init_t;

struct _mas_arg_get_dir_entry_at_pos
{
  /* provided input */
  const char *path;
  uint16_t n;
  /* expected output */
  char *buf; 
  uint8_t is_dir;
};
typedef struct _mas_arg_get_dir_entry_at_pos mas_arg_get_dir_entry_at_pos_t;

struct _mas_arg_get_dir_entry_cnt
{
  /* provided input */
  const char *path;
  /* expected output */
  uint16_t cnt;
};
typedef struct _mas_arg_get_dir_entry_cnt mas_arg_get_dir_entry_cnt_t;

#ifdef MAS_SD_INTERFACE
struct _mas_arg_read
{
  /* provided input */
  uint8_t cnt;
  /* expected output */
  const uint8_t *buf;
};
typedef struct _mas_arg_read mas_arg_read_t;

struct _mas_arg_file_pos
{
  uint32_t pos;
};
typedef struct _mas_arg_file_pos mas_arg_file_pos_t;
#endif

/*======================================================*/

extern char mas_entry_name[];   /* 12 chars for the name, 1 for the terminating '\0' */
extern uint8_t mas_entry_is_dir;
extern char mas_pwd[MAS_PATH_MAX+12+1];	/* add 12 to allow concat of entry_name (including '/') */


/*======================================================*/


/* devices, first argument to mas_init() */
uint8_t mas_device_sim(uint8_t msg, void *arg);    	/* internal simulation of some dirs and files, always possible */

extern uint8_t pff_arduino_chip_select_pin;		/* contains the Arduino pin number of the chip select signal for pff */
uint8_t mas_device_pff(uint8_t msg, void *arg);   	/* http://code.google.com/p/sdfatlib/ */
uint8_t mas_device_sdfat(uint8_t msg, void *arg);	/* http://elm-chan.org/fsw/ff/00index_p.html   Petit FAT File System Module, arg is pointer to SdFat */
uint8_t mas_device_sd(uint8_t msg, void *arg);	/* build in lib */


/* user api */
uint8_t mas_ChDir(const char *subdir);
uint8_t mas_ChDirUp(void);
void mas_ChDirRoot(void);
uint8_t mas_GetDirEntry(uint16_t n);
uint16_t mas_GetDirEntryCnt(void);
const char *mas_GetPathFilename(void);
const char *mas_GetPath(void);
const char *mas_GetFilename(void);
uint8_t mas_IsDir(void);
void mas_ClearFilename(void);
uint8_t mas_Init(mas_device_fn *device, void *arg);

#ifdef __cplusplus
}
#endif

#endif

