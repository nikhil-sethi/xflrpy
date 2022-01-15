/****************************************************************************

    Frame Class
    Copyright (C) 2007-2016 André Deperrois 

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
 
/** @file This file implements the Frame class used in the definition of Body objects */
 
#ifndef FRAME_H
#define FRAME_H

#include <xflgeom/geom3d/vector3d.h>
#include <QDataStream>
#include <QVector>




/**
*@class Frame This class defines a frame in the yz plane, on which the body surface is built. 
 * This is similar to the way a real life body is designed and built.
 * The Frame's points may be used indiferently by a spline-type or a flat-panel-type body.
 * The Frmae's points are defined from bottom to top, i.e. for crescending z values, and for the body's left (port) side. 
  * The x-value of the control points is unused, the frame's position is defined by the variable m_Position.
*/
class Frame
{
public:
    Frame(int nCtrlPts=0);

    void    appendPoint(Vector3d const& Pt);
    void    copyFrame(Frame *pFrame);
    void    copyPoints(QVector<Vector3d> *pPointList);
    double  height() const;
    int     isPoint(Vector3d const &point, double const &ZoomFactor) const;
    void    insertPoint(int n);
    void    insertPoint(int n, const Vector3d &Pt);
    int     insertPoint(Vector3d const &Real, int iAxis);
    Vector3d point(int iPt) const {return m_CtrlPoint.at(iPt);}
    int     pointCount() const {return m_CtrlPoint.size();}
    bool    removePoint(int n);
    void    rotateFrameY(double Angle);
    bool    serializeFrame(QDataStream &ar, bool bIsStoring);
    void    setPosition(Vector3d Pos);
    void    setuPosition(double u);
    void    setvPosition(double v);
    void    setwPosition(double w);
    double  zPos() const;

    Vector3d position() const {return m_Position;}
    Vector3d const &selectedPoint() const {return m_CtrlPoint[s_iSelect];}
    void setSelectedPoint(Vector3d pt) {if(s_iSelect>=0 && s_iSelect<m_CtrlPoint.size()) m_CtrlPoint[s_iSelect]=pt;}

    const Vector3d &ctrlPointAt(int idx) const {return m_CtrlPoint.at(idx);}


    static int selectedIndex()            {return s_iSelect;}
    static int highlightedIndex()         {return s_iHighlight;}
    static void setSelected(int index)    {s_iSelect=index;}
    static void setHighlighted(int index) {s_iHighlight=index;}

public:
    QVector <Vector3d> m_CtrlPoint;    /**< the array of points which define the frame.  */
    Vector3d m_Position;             /**< the translation vector for the Frame's origin */

private:
    static int s_iHighlight;               /**< the point over which the mouse hovers, or -1 if none */
    static int s_iSelect;                  /**< the selected pointed, i.e. the last point on which the user has clicked, or -1 if none */
};


#endif

