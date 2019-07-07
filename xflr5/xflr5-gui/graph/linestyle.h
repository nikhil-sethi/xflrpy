/****************************************************************************

    LineStyle Class
    Copyright (C) 2009-2018 Andre Deperrois

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

#include <QColor>


struct LineStyle
{
    bool m_bIsVisible;       /**< true if the curve is visible in the active view >*/
    int m_Style;             /**< the index of the style with which to draw the curve >*/
    int m_Width;             /**< the width with which to draw the curve >*/
    QColor m_Color;          /**< the color with which to draw the curve >*/

    int m_PointStyle;        /**< defines the point display. O = no points, 1 = small circles, 2 = large circles,3 = small squares, 4 = large squares >*/
};


