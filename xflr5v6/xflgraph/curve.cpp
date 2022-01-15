/****************************************************************************

    Curve Class
    Copyright (C) 2003-2016 André Deperrois 

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



#include <xflgraph/curve.h>
#include <xflgraph/graph.h>

/**
 * The public constructor
 */
Curve::Curve()
{
    m_theStyle.m_Color = QColor(255,0,0);
    m_theStyle.m_bIsVisible = true;
    m_theStyle.m_Symbol = Line::NOSYMBOL;
    m_theStyle.m_Width = 1;
    m_theStyle.m_Stipple = Line::SOLID;
    m_iSelected = -1;
}


/**
 * Appends a point to the end of the data
 * @param xn: x-coordinate
 * @param yn: y-coordinate
 * @return the new number of points in the curve
 */
int Curve::appendPoint(double xn, double yn)
{
    m_x.append(xn);
    m_y.append(yn);
    return size();
}


/**
 * Copies the data and settings from an existing curve
 * @param pCurve: a pointer to the input curve
 */
void Curve::duplicate(Curve const*pCurve)
{
    if(!pCurve) return;

    copyData(pCurve);

    m_theStyle = pCurve->m_theStyle;
    m_CurveName  = pCurve->m_CurveName;
}


/**
 * Copies the data and settings from an existing curve
 * @param pCurve: a pointer to the input curve
 */
void Curve::copyData(Curve const*pCurve)
{
    if(!pCurve) return;
    clear();
    m_x = pCurve->m_x;
    m_y = pCurve->m_y;
}


/**
 * Returns the index of the curve's point closest to the input coordinates
 * @param xs x coordinate
 * @param ys y coordinate
 * @param &dist distance to the return point
 * @return the index of the closest point
 */
int Curve::closestPoint(double xs, double ys, double &dist) const
{
    int ref;
    double d2;
    ref = -1;
    dist = 1.e10;
    if (size()<1) return -1;
    for(int i=0; i<size(); i++)
    {
        d2 =   (xs-m_x[i])*(xs-m_x[i])/m_pParentGraph->xScale()/m_pParentGraph->xScale()
             + (ys-m_y[i])*(ys-m_y[i])/m_pParentGraph->yScale()/m_pParentGraph->yScale();
        if (d2<dist)
        {
            dist = d2;
            ref = i;
        }
    }
    return ref;
}



/**
 * Returns the index of the curve's point closest to the input coordinates
 * @overload overloaded function
 * @param xs x coordinate
 * @param ys y coordinate
 * @param &dist distance to the return point
 * @param &n the index of the closest point
 */
void Curve::closestPoint(double xs, double ys, double &dist, int &n) const
{
    dist = 1.e10;
    if (n<1) return;
    for(int i=0; i<n; i++)
    {
        double d2 =   (xs-m_x[i])*(xs-m_x[i])/m_pParentGraph->xScale()/m_pParentGraph->xScale()
                    + (ys-m_y[i])*(ys-m_y[i])/m_pParentGraph->yScale()/m_pParentGraph->yScale();
        if (d2<dist)
        {
            dist = d2;
            n = i;
        }
    }
}


/**
 * Returns the index and the coordinates of the curve's point closest to the input coordinates
 * @param xs x input coordinate
 * @param ys y input coordinate
 * @param xSel x output coordinate
 * @param ySel y output coordinate
 * @param &dist distance to the return point
 * @param &nSel the index of the closest point
 */
void Curve::closestPoint(double const &xs, double const &ys, double &xSel, double &ySel, double &dist, int &nSel) const
{
    double d2;
    dist = 1.e40;

    for(int i=0; i<size(); i++)
    {
        d2 =   (xs-m_x[i])*(xs-m_x[i]) + (ys-m_y[i])*(ys-m_y[i]);
        if (d2<dist)
        {
            dist = d2;
            xSel = m_x.at(i);
            ySel = m_y.at(i);
            nSel = i;
        }
    }
}


/**
 * Returns the minimum x value of this curve
 *@return the x min value, or +99999999.0 if the curve has no points
 */
double Curve::xMin() const
{
    double xMin = 99999999.0;
//    if(n==0) xmin = .0; 
//    else
        for(int i=0; i<size();i++)
            xMin = qMin(xMin, m_x.at(i));
    return xMin;
}


/**
 * Returns the maximum x value of this curve
 *@return the x max value, or -99999999.0 if the curve has no points
 */
double Curve::xMax() const
{
    double xMax = -99999999.0;
//    if(n==0) xmax = 1.0; 
//    else
    for(int i=0; i<size();i++)
            xMax = qMax(xMax, m_x.at(i));
    return xMax;
}


/**
 * Returns the minimum y value of this curve
 *@return the y min value, or +99999999.0 if the curve has no points
 */
double Curve::yMin()  const
{
    double yMin = 99999999.0;
//    if(n==0) ymin = .0; 
//    else
    for(int i=0; i<size();i++)
            yMin = qMin(yMin, m_y.at(i));
    return yMin;
}


/**
 * Returns the maximum y value of this curve
 *@return the y max value, or -99999999.0 if the curve has no points
 */
double Curve::yMax() const
{
    double yMax = -99999999.0;
//    if(n==0) ymax = 1.0; 
//    else
        for(int i=0; i<size();i++)
            yMax = qMax(yMax, m_y.at(i));
    return yMax;
}


