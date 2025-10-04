/****************************************************************************

    Spline  Class
    Copyright (C) 1996 Paul Bourke    http://astronomy.swin.edu.au/~pbourke/curves/spline/
    Copyright (C) 2003 André Deperrois 

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



/**
 *@file This file subclasses the spline class with methods and variables for the gui.
 */


#pragma once


#include <QPainter>
#include <QFile>
#include <QVector>
#include <QTextStream>
#include <xflgeom/geom2d/spline.h>
#include <xflgeom/geom3d/vector3d.h>



class Spline5 : public Spline
{

public:
    Spline5();

    void drawSpline(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset);
    void drawCtrlPoints(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset);
    void drawOutputPoints(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset);

    void exportSpline(QTextStream &out, bool bExtrados);

};


