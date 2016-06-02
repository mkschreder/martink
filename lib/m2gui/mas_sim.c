/*

  mas_sim.c
  
  mass storage device for file system simulation

  part of m2tklib (Mini Interative Interface Toolkit Library)
  
  Copyright (C) 2012  olikraus@gmail.com

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

#include "mas.h"

/*
  Description:
    Requrest for a specific file at a specified position within the directory
*/

static uint8_t mas_sim_get_nth_file(const char *path, uint8_t n, char *buf, uint8_t *is_dir)
{
  uint8_t i;
  uint8_t subdir_level = 0;
  
  if ( n > 9 )
    return 0;

  i = 0;
  for(;;)
  {
    if ( path[i] == '\0' )
      break;
    if ( path[i] == MAS_DIR_SEP )
      subdir_level ++;
    i++;
  }
  
  if ( n == 0 )
  {
    buf[0] = 'd';
    buf[1] = 'i';
    buf[2] = 'r';
    buf[3] = '_';
    buf[4] = subdir_level+'0';
    buf[5] = '\0';
    *is_dir = 1;
  }
  else
  {
    buf[0] = 'f';
    buf[1] = 'i';
    buf[2] = 'l';
    buf[3] = 'e';
    buf[4] = '_';
    buf[5] = subdir_level+'0';
    buf[6] = '_';
    buf[7] = n-1+'1';
    buf[8] = '\0';
    *is_dir = 0;
  }

  return 1;
}


uint8_t mas_device_sim(uint8_t msg, void *arg)
{
  if ( msg == MAS_MSG_GET_DIR_ENTRY_AT_POS )
  {
    mas_arg_get_dir_entry_at_pos_t *a = ((mas_arg_get_dir_entry_at_pos_t *)arg);
    return mas_sim_get_nth_file(a->path, a->n, a->buf, &(a->is_dir));
  }
  else if ( msg == MAS_MSG_GET_DIR_ENTRY_CNT )
  {
    mas_arg_get_dir_entry_cnt_t *a = ((mas_arg_get_dir_entry_cnt_t *)arg);    
    a->cnt = 9;
    return 1;
  }
  else if ( msg == MAS_MSG_INIT )
  {
    return 1;
  }
  return 0;
}

