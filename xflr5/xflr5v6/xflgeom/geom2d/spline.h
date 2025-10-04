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
 *@file This file implements the spline class used in foil direct and inverse design.
 */


#pragma once


#include <QFile>
#include <QVector>
#include <QTextStream>
#include <QColor>

#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/xflobject.h>

/**
*@class Spline
*@brief  The class which defines the 2D spline object.

The spline is used in direct foil design to represent upper and lower surfaces, and in XInverse foil design to define the specification curve.

Based on the code provided by Paul Bourke.
*/
class Spline : public XflObject
{
    public:
        Spline();

        void clearControlPoints() {m_CtrlPt.clear();}
        int ctrlPointCount() const {return m_CtrlPt.size();}
        void appendControlPoint(double x, double y);
        void appendControlPoint(const Vector3d &Pt);
        bool insertPoint(double const &x, double const &y);
        bool removePoint(int const &k);
        int isControlPoint(Vector3d const &Real) const;
        int isControlPoint(double const &x, double const &y, double const &zx, double const &zy) const;
        int isControlPoint(Vector3d const &Real, double const &ZoomFactor) const;

        Vector3d &controlPoint(int i) {return m_CtrlPt[i];}
        Vector3d const &controlPoint(int i) const {return m_CtrlPt.at(i);}
        Vector3d const &lastCtrlPoint()     const {return m_CtrlPt.last();}
        Vector3d const &firstCtrlPoint()    const {return m_CtrlPt.first();}
        void setCtrlPoint(int n, double x, double y) {m_CtrlPt[n]=Vector3d(x,y,0);}
        void setCtrlPoint(int n, Vector3d const &pt) {m_CtrlPt[n]=pt;}
        void setFirstCtrlPoint(Vector3d const&pt) {if(m_CtrlPt.size()>0) m_CtrlPt.first()=pt;}
        void setLastCtrlPoint(double x, double y) {if(m_CtrlPt.size()>0) m_CtrlPt.back()=Vector3d(x,y,0);}
        void setLastCtrlPoint(Vector3d const&pt)  {if(m_CtrlPt.size()>0) m_CtrlPt.last()=pt;}


        double splineBlend(int const &i, int const &p, double const &t);
        double getY(double const &x) const;

        void clearPoints() {m_CtrlPt.clear();}

        void copy(Spline *pSpline);
        void copySymetric(Spline *pSpline);
        void splineCurve();
        void splineKnots();


        double xMin() const;
        double xMax() const;
        double yMin() const;
        double yMax() const;

    public:
        int m_iHighlight;                /**< the index of the currently highlighted control point, i.e. the point over which the mouse hovers, or -1 of none. */
        int m_iSelect;                   /**< the index of the currently selected control point, i.e. the point on which the user has last click, or -1 if none. */
        int m_iRes;                      /**< the number of output points to draw the spline */
        int m_iDegree;                   /**< the spline's degree */


        QVector<double> m_knot;            /**< the array of the values of the spline's knot */
        QVector<Vector3d> m_CtrlPt;      /**< the array of the positions of the spline's control points */
        QVector<Vector2d> m_Output;          /**< the array of output points, size of which is m_iRes */


};


