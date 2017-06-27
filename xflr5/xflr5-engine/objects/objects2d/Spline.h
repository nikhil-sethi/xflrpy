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
 *@file This file implements the spline class used in foil direct and inverse design.
 */


#ifndef SPLINE_H
#define SPLINE_H


#include <QFile>
#include <QList>
#include <QTextStream>
#include <objects2d/Vector3d.h>
#include "xfoil_params.h"


/**
*@class Spline
*@brief  The class which defines the 2D spline object.

The spline is used in direct foil design to represent upper and lower surfaces, and in XInverse foil design to define the specification curve.

Based on the code provided by Paul Bourke.
*/
class Spline
{
//	friend class SplineFoil;
//	friend class QXInverse;
//	friend class InverseOptionsDlg;

public:
	Spline();


	bool insertPoint(double const &x, double const &y);
	bool removePoint(int const &k);
	int isControlPoint(Vector3d const &Real);
	int isControlPoint(double const &x, double const &y, double const &zx, double const &zy);
	int isControlPoint(Vector3d const &Real, double const &ZoomFactor);
	double splineBlend(int const &i, int const &p, double const &t);
	double getY(double const &x);

	void copy(Spline *pSpline);
	void copySymetric(Spline *pSpline);
	void splineCurve();
	void splineKnots();
	
	void setStyle(int style){m_Style = style;}
	void setWidth(int width){m_Width = width;}
	void getColor(int &r, int &g, int &b, int &a);
	void setColor(int r, int g, int b, int a=255);
	int red() {return m_red;}
	int green() {return m_green;}
	int blue() {return m_blue;}
	int alphaChannel(){return m_alphaChannel;}


	int m_iHighlight;                /**< the index of the currently highlighted control point, i.e. the point over which the mouse hovers, or -1 of none. */
	int m_iSelect;                   /**< the index of the currently selected control point, i.e. the point on which the user has last click, or -1 if none. */
	int m_iRes;                      /**< the number of output points to draw the spline */
	int m_iDegree;                   /**< the spline's degree */


	QList<double> m_knot;            /**< the array of the values of the spline's knot */
	QList<Vector3d> m_CtrlPoint;      /**< the array of the positions of the spline's control points */
	Vector3d m_Output[IQX2];          /**< the array of output points, size of which is m_iRes @todo use a QVarLengthArray or a QList*/


	int m_Style, m_Width, m_PointStyle;
	bool m_bIsVisible;
	int m_red, m_blue, m_green, m_alphaChannel;    /**< the color with which to draw the Foil */

};


#endif
