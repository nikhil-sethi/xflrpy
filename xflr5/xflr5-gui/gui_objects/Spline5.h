/****************************************************************************

    Spline  Class
	Copyright (C) 1996 Paul Bourke	http://astronomy.swin.edu.au/~pbourke/curves/spline/
	Copyright (C) 2003 Andre Deperrois adeperrois@xflr5.com

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


#ifndef SPLINE5_H
#define SPLINE5_H


#include <QPainter>
#include <QFile>
#include <QList>
#include <QTextStream>
#include <objects2d/Spline.h>
#include <objects2d/Vector3d.h>
#include <gui_params.h>


class Spline5 : public Spline
{

public:
	Spline5();

	void drawSpline(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset);
	void drawCtrlPoints(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset);
	void drawOutputPoints(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset);

	void exportSpline(QTextStream &out, bool bExtrados);


	/** Sets the spline's drawing style
	*@param style the index of the spline's style
	*/
	void setStyle(int style){m_Style = style;}


	/** Sets the spline's drawing width
	*@param width the width of the spline
	*/
	void setWidth(int width) {m_Width = width;}


	/**
	*Sets the spline drawing color
	*@param color the spline's color
	*/
	void setColor(QColor color) {m_Color = color;}


	/**
	*Sets the spline's style, width and color
	*@param style the spline's drawing style
	*@param width the width of the spline
	*@param color the spline's color
	*/
	void setSplineParams(int style, int width, QColor color)
	{
		m_Width = width;
		m_Style = style;
		m_Color = color;
	}

	QColor color() {return m_Color;}
	int style() {return m_Style;}
	int width() {return m_Width;}

private:
	QColor m_Color;                  /**< the spline's display color */
	int m_Style;                     /**< the index of the spline's style */
	int m_Width;                     /**< the width of the spline */
};


#endif
