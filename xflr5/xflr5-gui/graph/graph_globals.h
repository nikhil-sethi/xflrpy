/****************************************************************************

    Graph globals
        Copyright (C) 2019 Andre Deperrois

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

#ifndef GRAPH_GLOBALS_H
#define GRAPH_GLOBALS_H

#include <QPainter>


void drawPoint(QPainter &painter, int pointStyle, QPoint pt, QColor backColor);
Qt::PenStyle getStyle(int s);


#endif // GRAPH_GLOBALS_H
