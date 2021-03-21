/****************************************************************************

    Curve Class
    Copyright (C) 2003-2016 Andre Deperrois

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

/** @file
 *
 * This file implements the Curve object for the graphs.
 *
 */



#ifndef CURVE_H
#define CURVE_H


#include <QVarLengthArray>
#include <QColor>


#include "linestyle.h"

class Graph;

/**
* @class Curve
* This class defines the curve object used by the Graph class.
*/
class Curve
{
    friend class Graph;

public:
    Curve();

    int     appendPoint(double xn, double yn);


    /**
     * Resets the content of the curve.
     */
    void    clear()
    {
        x.clear();
        y.clear();
    }

    int     closestPoint(double xs, double ys, double &dist);
    void    closestPoint(double xs, double ys, double &dist, int &n);
    void    closestPoint(double const &xs, double const &ys, double &xSel, double &ySel, double &dist, int &nSel);
    void    copyData(Curve *pCurve);
    void    duplicate(Curve *pCurve);


    /**
     * Return the index of the currently selected point
     *@param the index of the currently selected point
    */
    int selected(){return m_iSelected;}



    /**
     * Sets the visibility of the curve in the graphs
     *@param bVisible true if the curve is to be displayed, false otherwise
     */
    void setVisible(bool bVisible){m_curveStyle.m_bIsVisible = bVisible;}

    /**
     * Sets the curve's color
     * @param clr the new QColor value for the curve
     */
    void setColor(QColor clr){m_curveStyle.m_Color = clr;}

    /**
     * Sets the curve's style
     * @param nStyle the index of the new curve's style
     */
    void setStyle(int nStyle){ m_curveStyle.m_Style = nStyle;}

    void setLineStyle(LineStyle lineStyle) { m_curveStyle = lineStyle; }

    void setLineStyle(int Style, int Width, QColor color, int PointStyle, bool bVisible);

    void setPoints(int points) {m_curveStyle.m_PointStyle=points;}

    /**
     * Sets the index of the currently selected point of this curve
     * @param n the point to select
     */
    void setSelected(int n){m_iSelected = n;}

    /**
     *Sets the curve's width
     *@param nWidth the new curve's width in pixels
     **/
    void setWidth(int nWidth){m_curveStyle.m_Width = nWidth;}

    /**
     * Sets the curve title
     *@param Title the new curve's title
     */
    void setCurveName(QString Title){ m_CurveName = Title;}


    /** Return the visibility of the curve as a boolean. */
    bool isVisible() {return m_curveStyle.m_bIsVisible;}

    /** Return the visibility of the points as a boolean. */
    bool pointsVisible() {return m_curveStyle.m_PointStyle>0; }

    /** Returns the Curve's number of points. */
    int size() {return x.count();}

    /** Returns the Curve's number of points. */
    int count() {return x.size();}

    /** Returns the Curve style*/
    int style() {return m_curveStyle.m_Style;}

    /** Returns the Curve width*/
    int width() {return m_curveStyle.m_Width;}

    /** Returns the Curve color*/
    QColor  color() {return m_curveStyle.m_Color;}

    int pointStyle() {return m_curveStyle.m_PointStyle;}

    /** Returns the Curve's title */
    void curveName(QString &string) {string =  m_CurveName;}

    /** Returns the Curve's title */
    QString curveName(){ return m_CurveName;}

    double  xMin();
    double  xMax();
    double  yMin();
    double  yMax();


public:
    //    Curve Data
    QVarLengthArray<double,  1024> x;          /**< the array of the points x-coordinates */
    QVarLengthArray<double,  1024> y;          /**< the array of the points y-coordinates */


private:    
    QString m_CurveName;                       /**< the curves's name */
    int m_iSelected;                           /**< the index of the curve's currently selected point, or -1 if none is selected */
    Graph *m_pParentGraph;                      /**< a pointer to the parent graph to which this curve belongs */
    LineStyle m_curveStyle;
};


#endif
