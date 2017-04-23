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
// Given an array of n+1 pairs (x[i], y[i]), with i ranging from 0 to n,
// this function calculates the 3rd order cubic spline which interpolate the pairs.
//
// The spline is defined for each interval [x[j], x[j+1]) by n third order polynomial functions
//              p_j(x) = ax3 + bx2 + cx + d
//
// The equations to determine the coefficients a,b,c,d are
//
// Interpolation : 2n conditions
//    p_j(x[j])   = y[j];
//    p_j(x[j+1]) = y[j+1];
//
// Continuity of 1st and 2nd order derivatives at internal points: 2(n-1) conditions
//    p_j'(x[j]) = p_j+1'(x[j])
//    p_j"(x[j]) = p_j+1"(x[j])
//
// Second order derivative is zero at the end points : 2 conditions
//    p_j"(x[0]) =  p_j"(x[n]) =0
//
//
// This sets a linear system of size 4n which is solved by the Gauss algorithm for coefs a,b,c and d
// The RHS vector is
//	  a[0]
//	  b[0]
//	  c[0]
//	  d[0]
//	  a[1]
//    ...
//	  d[n-1]
*/
bool CubicSplineInterpolation(int n, double *x, double *y, double *a, double *b, double *c, double *d)
{
	if(n>50) return false;
	int i,size;

	double *M = new double[16*n*n];
	double *RHS = new double[4*n*n];

	memset(M, 0, 16*n*n*sizeof(double));
	memset(RHS, 0, 4*n*sizeof(double));

	size = 4*n;
//	Interpolation conditions
	for (i=0; i<n; i++)
	{
		//pj(x[i]) = y[i]
		M[2*i*size +4*i]     = x[i]*x[i]*x[i];
		M[2*i*size +4*i + 1] = x[i]*x[i];
		M[2*i*size +4*i + 2] = x[i];
		M[2*i*size +4*i + 3] = 1.0;

		//pj(x[i+1]) = y[i+1]
		M[(2*i+1)*size +4*i]     = x[i+1]*x[i+1]*x[i+1];
		M[(2*i+1)*size +4*i + 1] = x[i+1]*x[i+1];
		M[(2*i+1)*size +4*i + 2] = x[i+1];
		M[(2*i+1)*size +4*i + 3] = 1.0;

		RHS[2*i]   = y[i];
		RHS[2*i+1] = y[i+1];
	}

//  Derivation conditions
	for (i=1; i<n; i++)
	{
		//continuity of 1st order derivatives

		M[(2*n+i)*size + 4*(i-1)]     =  3.0*x[i]*x[i];
		M[(2*n+i)*size + 4*(i-1)+1]   =  2.0     *x[i];
		M[(2*n+i)*size + 4*(i-1)+2]   =  1.0;

		M[(2*n+i)*size + 4*i]   = -3.0*x[i]*x[i];
		M[(2*n+i)*size + 4*i+1] = -2.0     *x[i];
		M[(2*n+i)*size + 4*i+2] = -1.0;

		RHS[2*n+i]   = 0.0;

		//continuity of 2nd order derivatives
		M[(3*n+i)*size + 4*(i-1)]     =  6.0*x[i];
		M[(3*n+i)*size + 4*(i-1)+1]   =  2.0     ;

		M[(3*n+i)*size + 4*i]   = -6.0*x[i];
		M[(3*n+i)*size + 4*i+1] = -2.0     ;

		RHS[3*n+i]   = 0.0;
	}

//	second order derivative is zero at end points = "natural spline"
	M[2*n*size]     = 6.0*x[0];
	M[2*n*size+1]   = 2.0;
	RHS[2*n]        = 0.0;

	M[3*n*size + size-4]   = 6.0*x[n];
	M[3*n*size + size-3]   = 2.0;
	RHS[3*n+1]             = 0.0;

	bool bCancel = false;
	if(!Gauss(M, 4*n, RHS, 1, &bCancel)) return false;

	for(i=0; i<n; i++)
	{
		a[i] = RHS[4*i];
		b[i] = RHS[4*i+1];
		c[i] = RHS[4*i+2];
		d[i] = RHS[4*i+3];
	}

	delete [] M;
	delete [] RHS;
	return true;
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

















