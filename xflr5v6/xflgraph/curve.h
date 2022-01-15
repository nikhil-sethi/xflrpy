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


#pragma once

#include <QVector>
#include <QColor>


#include <xflcore/linestyle.h>

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

        int  appendPoint(double xn, double yn);
        void setPoint(int k, double xc, double yc) {if(k<0||k>m_x.size())return; m_x[k]=xc; m_y[k]=yc;}
        void setPointStyle(QVector<double> const &xc, QVector<double> const&yc) {m_x=xc; m_y=yc;}

        /**
         * Resets the content of the curve.
         */
        void clear() {m_x.clear(); m_y.clear();}
        void reset() {clear();}
        void resizePoints(int n) {m_x.resize(n); m_y.resize(n);}

        int     closestPoint(double xs, double ys, double &dist) const;
        void    closestPoint(double xs, double ys, double &dist, int &n) const;
        void    closestPoint(double const &xs, double const &ys, double &xSel, double &ySel, double &dist, int &nSel) const;
        void    copyData(const Curve *pCurve);
        void    duplicate(const Curve *pCurve);

        double x(int i) const {return m_x.at(i);}
        double y(int i) const {return m_y.at(i);}

        /**
         * Return the index of the currently selected point
         *@param the index of the currently selected point
        */
        int selected() const {return m_iSelected;}



        /**
         * Sets the visibility of the curve in the graphs
         *@param bVisible true if the curve is to be displayed, false otherwise
         */
        void setVisible(bool bVisible){m_theStyle.m_bIsVisible = bVisible;}

        /**
         * Sets the curve's color
         * @param clr the new QColor value for the curve
         */
        void setColor(QColor const &clr) {m_theStyle.m_Color = clr;}

        /**
         * Sets the curve's style
         * @param nStyle the index of the new curve's style
         */
        void setStipple(int nStyle){ m_theStyle.setStipple(nStyle);}

        void setLineStyle(LineStyle const &ls2) {m_theStyle=ls2;}

        void setPointStyle(int pointstyle) {m_theStyle.setPointStyle(pointstyle);}

        /**
         * Sets the index of the currently selected point of this curve
         * @param n the point to select
         */
        void setSelected(int n){m_iSelected = n;}

        /**
         *Sets the curve's width
         *@param nWidth the new curve's width in pixels
         **/
        void setWidth(int nWidth){m_theStyle.m_Width = nWidth;}

        /**
         * Sets the curve title
         *@param Title the new curve's title
         */
        void setName(QString const &Title){ m_CurveName = Title;}


        /** Return the visibility of the curve as a boolean. */
        bool isVisible() const {return m_theStyle.m_bIsVisible;}

        /** Return the visibility of the points as a boolean. */
        bool pointsVisible() const {return m_theStyle.m_Symbol>0; }

        /** Returns the Curve's number of points. */
        int size() const {return m_x.count();}

        /** Returns the Curve's number of points. */
        int count() const {return m_x.size();}

        /** Returns the Curve style*/
        Line::enumLineStipple stipple() const {return m_theStyle.m_Stipple;}

        /** Returns the Curve width*/
        int width() const {return m_theStyle.m_Width;}

        /** Returns the Curve color*/
        QColor const& color() const {return m_theStyle.m_Color;}

        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}

        /** Returns the Curve's title */
        void curveName(QString &string) const {string =  m_CurveName;}

        /** Returns the Curve's title */
        QString const &curveName() const {return m_CurveName;}

        double  xMin() const;
        double  xMax() const;
        double  yMin() const;
        double  yMax() const;


    public:
        //    Curve Data
        QVector<double> m_x;          /**< the array of the points x-coordinates */
        QVector<double> m_y;          /**< the array of the points y-coordinates */


    private:
        QString m_CurveName;                       /**< the curves's name */
        int m_iSelected;                           /**< the index of the curve's currently selected point, or -1 if none is selected */
        Graph *m_pParentGraph;                      /**< a pointer to the parent graph to which this curve belongs */
        LineStyle m_theStyle;
};


