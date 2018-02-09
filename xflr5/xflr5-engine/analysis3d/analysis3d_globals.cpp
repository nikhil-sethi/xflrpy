/****************************************************************************

	Globals Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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

//Global functions

/**@file This file contains the definitions of methods used throughout the program and not specific to one application. */

#include <analysis3d_globals.h>

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <math.h>
#include <qopengl.h>
#include <QtCore>

#include <matrix.h>

/**
* Tests if a given integer is between two other integers
*@param f the integer to test
*@param f1 the first bound
*@param f2 the second bound
*@return true if f1<f<f2 or f2<f<f1
*/
bool isBetween(int f, int f1, int f2)
{
	if (f2 < f1)
	{
		int tmp = f2;
		f2 = f1;
		f1 = tmp;
	}
	if(f<f1) return false;
	else if(f>f2) return false;
	return true;
}


/**
* Tests if a given integer is between two double values
*@param f the integer to test
*@param f1 the first bound
*@param f2 the second bound
*@return true if f1<f<f2 or f2<f<f1
*/
bool isBetween(int f, double f1, double f2)
{
	double ff = f;
	if (f2 < f1)
	{
		double tmp = f2;
		f2 = f1;
		f1 = tmp;
	}
	if(ff<f1)       return false;
	else if(ff>f2) return false;
	return true;
}


/**
* Tests if a given integer is an even number
*@param n the integer to test
*@return true if n is an even number
*/
bool isEven(int n)
{
	return n%2==0;
}




/**
* Calculates the blending value of a point on a BSpline. This is done recursively.
* If the numerator and denominator are 0 the expression is 0.
* If the denominator is 0 the expression is 0
*
* @param index   the control point's index
* @param p       the spline's degree
* @param t       the spline parameter
* @param knots   a pointer to the vector of knots
* @return the spline function value
*/
#define EPS 0.0001
double splineBlend(int const &index, int const &p, double const &t, double *knots)
{
	if(p==0)
	{
		if ((knots[index] <= t) && (t < knots[index+1]) ) return 1.0;
		else                                              return 0.0;
	}
	else
	{
		if (fabs(knots[index+p] - knots[index])<EPS && fabs(knots[index+p+1] - knots[index+1])<EPS)
			return 0.0;
		else if (fabs(knots[index+p] - knots[index])<EPS)
			return (knots[index+p+1]-t) / (knots[index+p+1]-knots[index+1])  * splineBlend(index+1, p-1, t, knots);
		else if (fabs(knots[index+p+1] - knots[index+1])<EPS)
			return (t-knots[index])     / (knots[index+p] - knots[index])    * splineBlend(index,   p-1, t, knots);
		else
			return (t-knots[index])     / (knots[index+p] - knots[index])    * splineBlend(index,   p-1, t, knots) +
					(knots[index+p+1]-t) / (knots[index+p+1]-knots[index+1]) * splineBlend(index+1 ,p-1, t, knots);
	}
}





//-----------------------------------------------------------------------------------------------





typedef enum {MAINWING, SECONDWING, ELEVATOR, FIN, OTHERWING} enumWingType;



QString boolToString(bool b)
{
	return b ? "true" : "false";
}

















