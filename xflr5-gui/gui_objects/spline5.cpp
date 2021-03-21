/****************************************************************************

    Spline  Class
    Copyright (C) 1996 Paul Bourke    http://astronomy.swin.edu.au/~pbourke/curves/spline/
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

#include <globals/globals.h>
#include <graph_globals.h>
#include "spline5.h"
#include <math.h>


/**
*The public constructor
*/
Spline5::Spline5() :Spline()
{
    m_Style = 0;
    m_Width = 1;
    m_Color = QColor(70, 200, 120);
}


/**
*Draws the control points on a QPainter.
*/
void Spline5::drawCtrlPoints(QPainter &painter, double const &scalex, double const &scaley, QPointF const &Offset)
{
    painter.save();

    QPointF pt;
    int i, width;

    width  = 3;

    QPen PointPen;
    QBrush NoBrush(Qt::NoBrush);
    PointPen.setWidth(1);

    painter.setPen(PointPen);
    painter.setBrush(NoBrush);

    for (i=0; i<m_CtrlPoint.size(); i++)
    {
        pt.rx() =  m_CtrlPoint[i].x*scalex + Offset.x();
        pt.ry() = -m_CtrlPoint[i].y*scaley + Offset.y();

        if (m_iSelect==i)
        {
            PointPen.setWidth(2);
            PointPen.setColor(QColor(100,100,255));
        }
        else if(m_iHighlight==i)
        {
            PointPen.setWidth(2);
            PointPen.setColor(QColor(255,0,0));
        }
        else
        {
            PointPen.setWidth(1);
            PointPen.setColor(m_Color);
        }
        painter.setPen(PointPen);
        painter.drawEllipse(pt.x()-width, pt.y()-width, 2*width, 2*width);
    }
    painter.restore();
}



/**
*Draws the output points on a QPainter.
*/
void Spline5::drawOutputPoints(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset)
{
    painter.save();

    QPointF pt;
    int i, width;
    QPen OutPen;

    width = 2;

    OutPen.setColor(m_Color);
    OutPen.setStyle(Qt::SolidLine);
    OutPen.setWidth(1);
    painter.setPen(OutPen);

    for (i=0; i<m_iRes;i++)
    {
        pt.rx() =  m_Output[i].x*scalex + Offset.x();
        pt.ry() = -m_Output[i].y*scaley + Offset.y();

        painter.drawRect(pt.x()-width, pt.y()-width, 2*width, 2*width);
    }

    painter.restore();
}


/**
*Draws the spline curve on a QPainter.
*/
void Spline5::drawSpline(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset)
{
    painter.save();

    QPointF From, To;
    int k;
    QPen SplinePen;

    SplinePen.setColor(m_Color);
    SplinePen.setStyle(getStyle(m_Style));
    SplinePen.setWidth(m_Width);
    painter.setPen(SplinePen);


    if(m_CtrlPoint.size()>=3)
    { 
        From.rx() =  m_Output[0].x * scalex + Offset.x();
        From.ry() = -m_Output[0].y * scaley + Offset.y();

        for(k=1; k<m_iRes;k++) 
        {
            To.rx() =  m_Output[k].x * scalex + Offset.x();
            To.ry() = -m_Output[k].y * scaley + Offset.y();

            painter.drawLine(From, To);

            From = To;
        }
    }
    painter.restore();
}



/**
* Exports the spline output points to a text file
* @param out the stream to which the data is directed
* @param bExtrados true if the data should be written from end to beginning, false if written from beginning to end. This is the order required by foil files.
*/
void Spline5::exportSpline(QTextStream &out, bool bExtrados)
{
    int k;
    QString strOut;

    if(bExtrados)
    {
        for (k=m_iRes-1;k>=0; k--)
        {
            strOut= QString(" %1  %2\n").arg(m_Output[k].x,7,'f',4).arg( m_Output[k].y,7,'f',4);
            out << strOut;
        }
    }
    else
    {
        for (k=1;k<m_iRes; k++)
        {
            strOut=QString(" %1  %2\n").arg(m_Output[k].x,7,'f',4).arg( m_Output[k].y,7,'f',4);
            out << strOut;
        }
    }
}



