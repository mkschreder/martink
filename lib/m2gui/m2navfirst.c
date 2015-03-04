/*

  m2navfirst.c

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

  Code size status: 
    Proccedure attributes: Optimization completed
    Arithmetic optimization: Done

*/

#include "m2.h"

/*
  Description:
    Navigate to first element.
  Return value:
    0, if max depth is 0 or 1.
  Implementation Status
    Could be moved to different file
*/
uint8_t m2_nav_first(m2_nav_p nav)
{
  if ( nav->depth <= 1 )
    return 0;
  m2_nav_load_child(nav, 0);
  return 1;  
}

