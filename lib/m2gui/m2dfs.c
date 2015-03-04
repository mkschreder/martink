/*

  m2dfs.c

  m2tklib = Mini Interative Interface Toolkit Library
  
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

  Code size status: 

*/

#include "m2.h"

/* use the temp object from m2draw.c */
extern m2_nav_t m2_draw_current;

static void m2_draw_sub(m2_dfs_fnptr cb)
{
  cb(&m2_draw_current);
  if ( m2_nav_down(&m2_draw_current, 0) != 0 )
  {
    do
    {
      m2_draw_sub(cb);
    } while ( m2_nav_next(&m2_draw_current) != 0 );
    m2_nav_up(&m2_draw_current);
  }
}

void m2_nav_dfs(m2_nav_p nav, m2_dfs_fnptr cb)
{
  m2_draw_current.element_list[0] = nav->element_list[0];
  m2_draw_current.depth = 1;
  m2_draw_sub(cb);
}

