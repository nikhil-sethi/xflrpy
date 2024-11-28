/****************************************************************************

    Line_enums functions

    Copyright (C) 2008-2019 André Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/


#pragma once

namespace Line
{
    /** @enum The different applications available to the user */
    typedef enum {SOLID, DASH, DOT, DASHDOT, DASHDOTDOT, NOLINE} 	enumLineStipple;

    /**< @enum The different symbols available for line plots*/
    typedef enum {NOSYMBOL,
                  LITTLECIRCLE,   BIGCIRCLE, LITTLESQUARE, BIGSQUARE, TRIANGLE, TRIANGLE_INV,
                  LITTLECIRCLE_F, BIGCIRCLE_F, LITTLESQUARE_F, BIGSQUARE_F, TRIANGLE_F, TRIANGLE_INV_F,
                  LITTLECROSS, BIGCROSS}	enumPointStyle;
}


