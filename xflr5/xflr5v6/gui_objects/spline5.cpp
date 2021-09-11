/****************************************************************************

    Spline  Class
    Copyright (C) 1996 Paul Bourke    http://astronomy.swin.edu.au/~pbourke/curves/spline/
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

#include <QPolygon>

#include <xflcore/xflcore.h>
#include "spline5.h"



/**
*The public constructor
*/
Spline5::Spline5() :Spline()
{
    m_theStyle.m_Symbol = Line::NOSYMBOL; //no points to start with
    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 2;
    m_theStyle.m_bIsVisible    = true;

    m_theStyle.m_Color = QColor(70, 200, 120);
}


/**
*Draws the control points on a QPainter.
*/
void Spline5::drawCtrlPoints(QPainter &painter, double const &scalex, double const &scaley, QPointF const &Offset)
{
    painter.save();

    QPointF pt;

    int width  = 3;

    QPen PointPen;
    QBrush NoBrush(Qt::NoBrush);
    PointPen.setWidth(1);

    painter.setPen(PointPen);
    painter.setBrush(NoBrush);

    for (int i=0; i<m_CtrlPt.size(); i++)
    {
        pt.rx() =  m_CtrlPt[i].x*scalex + Offset.x();
        pt.ry() = -m_CtrlPt[i].y*scaley + Offset.y();

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
            PointPen.setColor(m_theStyle.m_Color);
        }
        painter.setPen(PointPen);
        painter.drawEllipse(int(pt.x()-width), int(pt.y()-width), 2*width, 2*width);
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
    QPen OutPen;

    int width = 2;

    OutPen.setColor(m_theStyle.m_Color);
    OutPen.setStyle(Qt::SolidLine);
    OutPen.setWidth(1);
    painter.setPen(OutPen);

    for (int i=0; i<m_iRes;i++)
    {
        pt.rx() =  m_Output[i].x*scalex + Offset.x();
        pt.ry() = -m_Output[i].y*scaley + Offset.y();

        painter.drawRect(int(pt.x()-width), int(pt.y()-width), 2*width, 2*width);
    }

    painter.restore();
}


/**
*Draws the spline curve on a QPainter.
*/
void Spline5::drawSpline(QPainter & painter, double const &scalex, double const &scaley, QPointF const &Offset)
{
    painter.save();

    QPen SplinePen;

    SplinePen.setColor(m_theStyle.m_Color);
    SplinePen.setStyle(xfl::getStyle(m_theStyle.m_Stipple));
    SplinePen.setWidth(m_theStyle.m_Width);
    painter.setPen(SplinePen);

    QPolygonF poly;

    if(m_CtrlPt.size()>=3)
    { 

        for(int k=0; k<m_iRes;k++)
        {
            poly.append({m_Output[k].x * scalex + Offset.x(), -m_Output[k].y * scaley + Offset.y()});
        }
        painter.drawPolyline(poly);
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



