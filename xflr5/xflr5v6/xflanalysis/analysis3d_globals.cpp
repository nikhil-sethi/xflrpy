/****************************************************************************

    Globals Class
    Copyright (C) 2008-2016 André Deperrois 

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


#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <QtCore>

#include <xflcore/matrix.h>
#include <xflanalysis/analysis3d_globals.h>



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

















