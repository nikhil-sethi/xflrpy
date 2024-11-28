/****************************************************************************

    Graph Classes
    Copyright (C) 2003-2019 André Deperrois

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


#include <cmath>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QTextStream>

#include <xflgraph/graph.h>
#include <xflgraph/curve.h>
#include <xflcore/xflcore.h>

#define MININTERVAL  0.000000001


bool Graph::s_bHighlightPoint(true);

QColor Graph::s_CurveColors[] = {QColor(255,   0,   0), QColor(  0,   0, 255), QColor(  0, 255,   0), QColor(255, 255,   0),
                                 QColor(  0, 255, 255), QColor(255,   0, 255), QColor(255, 125,  70), QColor( 70, 125, 255),
                                 QColor(125, 255,  70), QColor(255, 70,  200)};


Graph::Graph()
{
    m_rCltRect.setRect(0,0, 200, 300);
    m_GraphType = GRAPH::OTHERGRAPH;

    setGraphDefaults();

    //Type is used to determine automatic scales
    m_AutoScaleType = 1;

    m_X = 0;
    m_Y = 1;

    m_xmin         = .0;
    m_xmax         = .10;
    m_ymin         = .0;
    m_ymax         = .10;
    m_xunit        = 0.1;
    m_yunit        = 0.1;
    m_xo           = 0.0;
    m_yo           = 0.0;

    m_scalex     = 0.1;
    m_scaley     = 0.1;
    Cxmin        = 0.0;
    Cxmax        = 1.0;
    Cymin        = 0.0;
    Cymax        = 1.0;

    exp_x = 0;
    exp_y = 0;

    m_bYInverted  = false;

    m_bAutoX        = true;
    m_bAutoY        = true;

    m_BorderStyle.m_bIsVisible = true;

    m_ptoffset.rx() = 0;
    m_ptoffset.ry() = 0;

    m_BorderStyle.m_Stipple = Line::SOLID;
    m_BorderStyle.m_Width = 2;

    m_iMargin = 51;

    m_h       = 0;
    m_w       = 0;

    setGraphDefaults();
}


Graph::~Graph()
{
    deleteCurves();
}



void Graph::drawGraph(QRect const &rect, QPainter &painter)
{
    m_rCltRect = rect;
    drawGraph(painter);
}


void Graph::drawGraph(QPainter &painter)
{
    QColor color;
    painter.save();

    //    Draw Border
    if(m_BorderStyle.m_bIsVisible) color = m_BorderStyle.m_Color;
    else                           color = m_BackColor;
    QPen BorderPen(color);
    BorderPen.setStyle(xfl::getStyle(m_BorderStyle.m_Stipple));
    BorderPen.setWidth(m_BorderStyle.m_Width);

    painter.setPen(BorderPen);
    painter.fillRect(m_rCltRect, m_BackColor);
    painter.drawRect(m_rCltRect);
    initializeGraph();

    painter.setClipRect(m_rCltRect);

    painter.setBackgroundMode(Qt::TransparentMode);

    if(m_Grid.bXMajGrid())  drawXMajGrid(painter);
    if(m_Grid.bYMajGrid(0)) drawYMajGrid(painter);
    if(m_Grid.bXMinGrid())  drawXMinGrid(painter);
    if(m_Grid.bYMinGrid(0)) drawYMinGrid(painter);

    drawAxes(painter);

    drawXTicks(painter);

    drawYTicks(painter);

    for (int nc=0; nc < m_oaCurves.size(); nc++) drawCurve(nc,painter);

    drawTitles(painter);

    painter.setClipping(false);
    painter.restore();
}


void Graph::drawCurve(int nIndex, QPainter &painter)
{
    painter.save();

    QPoint To, Min, Max;
    QRect rViewRect;

    int ptside = 5;
    Curve* pCurve = curve(nIndex);

    double scaley = m_scaley;

    QBrush FillBrush(m_BackColor);
    painter.setBrush(FillBrush);

    QPen CurvePen(pCurve->color());
    CurvePen.setStyle(xfl::getStyle(pCurve->stipple()));
    CurvePen.setWidth(std::max(pCurve->width(), 1));
    painter.setPen(CurvePen);

    Min.setX(int(m_xmin/m_scalex) +m_ptoffset.x());
    Min.setY(int(m_ymin/scaley)   +m_ptoffset.y());
    Max.setX(int(m_xmax/m_scalex) +m_ptoffset.x());
    Max.setY(int(m_ymax/scaley)   +m_ptoffset.y());
    rViewRect.setTopLeft(Min);
    rViewRect.setBottomRight(Max);

    QPolygonF polycurve(pCurve->size());
    if(pCurve->size()>=1 && pCurve->isVisible())
    {
        for (int i=0; i<pCurve->size();i++)
        {
            polycurve[i] = {pCurve->x(i)/m_scalex+m_ptoffset.x(), pCurve->y(i)/  scaley+m_ptoffset.y()};
        }
        if(pCurve->width()>=1) painter.drawPolyline(polycurve);

        CurvePen.setStyle(Qt::SolidLine);
        painter.setPen(CurvePen);
        for (int i=0; i<polycurve.size(); i++)
        {
            xfl::drawSymbol(painter, pCurve->pointStyle(), m_BackColor, pCurve->color(), polycurve.at(i).x(), polycurve.at(i).y());
        }
    }

    if(s_bHighlightPoint)
    {
        int point = pCurve->selected();
        if(point>=0)
        {
            //highlight
            CurvePen.setWidth(pCurve->width()+2);
            CurvePen.setColor(Qt::red);
            painter.setPen(CurvePen);
            To.setX(int(pCurve->x(point)/m_scalex+m_ptoffset.x()));
            To.setY(int(pCurve->y(point)/scaley  +m_ptoffset.y()));
            painter.drawRect(To.x()-ptside,To.y()-ptside, 2*ptside,2*ptside);
        }
    }
    painter.restore();
}


void Graph::drawAxes(QPainter &painter)
{
    double xp(0), yp(0);
    QPen AxesPen;
    double scaley = m_scaley;
    painter.save();

    AxesPen.setColor(m_Grid.xAxisColor());
    AxesPen.setStyle(xfl::getStyle(m_Grid.xAxisStipple()));
    AxesPen.setWidth(m_Grid.xAxisWidth());
    painter.setPen(AxesPen);

    //vertical axis
    if     (m_xo>=m_xmin && m_xo<=m_xmax) xp = m_xo;
    else if(m_xo>m_xmax)                  xp = m_xmax;
    else                                  xp = m_xmin;

    painter.drawLine(int(xp/m_scalex) + m_ptoffset.x(), int(m_ymin/scaley) + m_ptoffset.y(),
                     int(xp/m_scalex) + m_ptoffset.x(), int(m_ymax/scaley) + m_ptoffset.y());

    //horizontal axis
    if(m_yo>=m_ymin && m_yo<=m_ymax) yp = m_yo;
    else if(m_yo>m_ymax)         yp = m_ymax;
    else                     yp = m_ymin;


    painter.drawLine(int(m_xmin/m_scalex) +m_ptoffset.x(), int(yp/scaley) + m_ptoffset.y(),
                     int(m_xmax/m_scalex) +m_ptoffset.x(), int( yp/scaley) + m_ptoffset.y());

    painter.restore();
}


void Graph::drawTitles(QPainter &painter)
{
    //draws the x & y axis name

    double xp(0), yp(0);

    double scaley = m_scaley;
    painter.save();
    int XPosXTitle = 5;
    int YPosXTitle = -10;
    int XPosYTitle = -5;
    int YPosYTitle =  5;

    if(m_xo>=m_xmin && m_xo<=m_xmax) xp = m_xo;
    else if(m_xo>m_xmax)             xp = m_xmax;
    else                             xp = m_xmin;

    if(m_yo>=m_ymin && m_yo<=m_ymax) yp = m_yo;
    else if(m_yo>m_ymax)             yp = m_ymax;
    else                             yp = m_ymin;

    painter.setFont(m_TitleFont);

    QPen TitlePen(m_TitleColor);
    painter.setPen(TitlePen);

    painter.drawText(  int(m_xmax/m_scalex) + m_ptoffset.x() + XPosXTitle,
                       int(yp  /scaley)   + m_ptoffset.y() + YPosXTitle, m_XTitle);

    painter.drawText(  m_ptoffset.x() + int(xp/m_scalex)   + XPosYTitle,
                       m_rCltRect.top() + m_iMargin - YPosYTitle, m_YTitle);
    painter.restore();
}


void Graph::drawXTicks(QPainter &painter)
{
    int exp=0;

    if(fabs(m_xunit)<0.00000001) return;
    if(fabs(m_xmax-m_xmin)/m_xunit>30.0) return;

    double scaley = m_scaley;
    painter.save();
    QString strLabel, strLabelExp;

    QFontMetrics fm(m_LabelFont);

    painter.setFont(m_LabelFont);


    int TickSize = 5;
    int height  = fm.height()/2;
    int yExpOff = height/2;


    QPen LabelPen(m_Grid.xAxisColor());

    LabelPen.setStyle(xfl::getStyle(m_Grid.xAxisStipple()));
    LabelPen.setWidth(m_Grid.xAxisWidth());
    painter.setPen(LabelPen);

    int nx = int((m_xo-m_xmin)/m_xunit);
    double xt = m_xo - nx*m_xunit;

    double yp = 0;
    if(m_yo>=m_ymin && m_yo<=m_ymax) yp = m_yo;
    else if(m_yo>m_ymax)         yp = m_ymax;
    else                     yp = m_ymin;

    int iTick = 0;

    while(fabs(m_xunit)>MININTERVAL && xt<=m_xmax*1.0001 && iTick<100)
    {
        //Draw ticks
        if(xt>=m_xmin)
        {
            painter.setPen(LabelPen);
            painter.drawLine(int(xt/m_scalex) + m_ptoffset.x(),int(yp/scaley) +TickSize + m_ptoffset.y(),
                             int(xt/m_scalex) + m_ptoffset.x(),int(yp/scaley)           + m_ptoffset.y());
            painter.setPen(m_LabelColor);

            if(fabs(xt)<MININTERVAL)
            {
                strLabel = "0";
                painter.drawText(int(xt/m_scalex) - fm.horizontalAdvance(strLabel)/2 + m_ptoffset.x(),
                                 int(yp/scaley)   + TickSize*2 +height   + m_ptoffset.y(),
                                 strLabel);
            }
            else if(exp_x>=4 || exp_x<=-4)
            {
                double main = xt;
                expFormat(main, exp);

                strLabel = QString("%1 10").arg(main,5,'f',1);
                painter.drawText(int(xt/m_scalex) - fm.horizontalAdvance(strLabel)/2  +m_ptoffset.x(),
                                 int(yp/scaley)   + TickSize*2 +height    +m_ptoffset.y(),
                                 strLabel);
                strLabelExp = QString("%1").arg(exp);

                painter.drawText(int(xt/m_scalex) + fm.horizontalAdvance(strLabel)/2       +m_ptoffset.x(),
                                 int(yp/scaley)   + TickSize*2 +height-yExpOff +m_ptoffset.y(),
                                 strLabelExp);
            }
            else
            {
                if(exp_x>0)         strLabel = QString("%1").arg(xt,0,'f',0);
                else if (exp_x>=-1) strLabel = QString("%1").arg(xt,6,'f',1);
                else if (exp_x>=-2) strLabel = QString("%1").arg(xt,6,'f',2);
                else if (exp_x>=-3) strLabel = QString("%1").arg(xt,6,'f',3);
                painter.drawText(int(xt/m_scalex) - fm.horizontalAdvance(strLabel)/2 + m_ptoffset.x(),
                                 int(yp/scaley)   + TickSize*2 +height   + m_ptoffset.y(),
                                 strLabel);
            }
        }
        xt += m_xunit ;
        iTick++;
    }
    painter.restore();
}


void Graph::drawYTicks(QPainter &painter)
{
    if(fabs(m_xunit)<0.00000001) return;
    if(fabs(m_ymax-m_ymin)/m_yunit>30.0) return;
    double scaley = m_scaley;
    painter.save();
    QString strLabel, strLabelExp;
    int exp = 0;
    QFontMetrics fm(m_LabelFont);
    painter.setFont(m_LabelFont);

    int fmheight  = fm.height();
    int fmheight4 = int(double(fmheight)/4.0);

    int TickSize = 5;

    QPen LabelPen(m_Grid.yAxisColor());
    LabelPen.setStyle(xfl::getStyle(m_Grid.yAxisStipple()));
    LabelPen.setWidth(m_Grid.yAxisWidth());

    double xp=0;
    if(m_xo>=m_xmin && m_xo<=m_xmax) xp = m_xo;
    else if(m_xo>m_xmax)         xp = m_xmax;
    else                     xp = m_xmin;

    double yt = m_yo-int((m_yo-m_ymin)*1.0001/m_yunit)*m_yunit;//one tick at the origin

    int iTick=0;

    while(fabs(m_yunit)>MININTERVAL && yt<=m_ymax*1.0001 && iTick<100)
    {
        //Draw ticks
        if(yt>=m_ymin)
        {
            painter.setPen(LabelPen);
            painter.drawLine(int(xp/m_scalex)          + m_ptoffset.x(), int(yt/scaley) + m_ptoffset.y(),
                             int(xp/m_scalex)-TickSize + m_ptoffset.x(), int(yt/scaley) + m_ptoffset.y());

            painter.setPen(m_LabelColor);


            if(fabs(yt)<MININTERVAL)
            {
                strLabel = "0";
                painter.drawText(int(xp/m_scalex) - fm.horizontalAdvance(strLabel)-TickSize*2 +m_ptoffset.x(),
                                 int(yt/scaley)   + fmheight4 +m_ptoffset.y(),
                                 strLabel);
            }
            else if(abs(exp_y)>=4)
            {
                double main = yt;
                expFormat(main, exp);

                strLabel    = QString("%1 10").arg(main,5,'f',1);
                strLabelExp = QString("%1").arg(exp);

                painter.drawText(int(xp/m_scalex) - fm.horizontalAdvance(strLabel)-TickSize*3 + m_ptoffset.x(),
                                 int(yt/scaley)   + fmheight4                     + m_ptoffset.y(),
                                 strLabel);

                if(exp_y>=4)
                {
                    painter.drawText(int(xp/m_scalex)   - TickSize*3 + m_ptoffset.x(),
                                     int(yt/scaley)                  + m_ptoffset.y(),
                                     strLabelExp);
                }
                else
                {
                    painter.drawText(int(xp/m_scalex)   - TickSize*3 + 2 + m_ptoffset.x(),
                                     int(yt/scaley)                  + m_ptoffset.y(),
                                     strLabelExp);
                }
            }
            else
            {
                if(exp_y>=0)        strLabel = QString("%1").arg(yt,6,'f',0);
                else if (exp_y>=-1) strLabel = QString("%1").arg(yt,6,'f',1);
                else if (exp_y>=-2) strLabel = QString("%1").arg(yt,6,'f',2);
                else if (exp_y>=-3) strLabel = QString("%1").arg(yt,6,'f',3);

                painter.drawText(int(xp/m_scalex)   - fm.horizontalAdvance(strLabel)-TickSize*2 +m_ptoffset.x(),
                                 int(yt/scaley) + fmheight4 +m_ptoffset.y(),
                                 strLabel);
            }
        }
        yt += m_yunit ;
        iTick++;
    }
    painter.restore();
}


void Graph::drawXMajGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(m_xunit)<0.00000001)     return;
    if(fabs(m_xmax-m_xmin)/m_xunit>30.0) return;

    painter.save();
    int YMin, YMax;

    QPen GridPen(m_Grid.xMajColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    GridPen.setWidth(m_Grid.xMajWidth());
    painter.setPen(GridPen);

    YMin = int(m_ymin/scaley) + m_ptoffset.y();
    YMax = int(m_ymax/scaley) + m_ptoffset.y();


    double xt = m_xo-int((m_xo-m_xmin)*1.0001/m_xunit)*m_xunit;//one tick at the origin
    while(xt<=m_xmax*1.001)
    {
        if(xt>=m_xmin)
        {
            painter.drawLine(int(xt/m_scalex) + m_ptoffset.x(), YMin, int(xt/m_scalex) + m_ptoffset.x(), YMax);
        }
        xt += m_xunit ;
    }
    painter.restore();
}


void Graph::drawYMajGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(m_yunit)<0.00000001) return;
    if(fabs(m_ymax-m_ymin)/m_yunit>30.0) return;

    painter.save();

    QPen GridPen(m_Grid.yMajColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMajStipple(0)));
    GridPen.setWidth(m_Grid.yMajWidth(0));
    painter.setPen(GridPen);


    double yt = m_yo-int((m_yo-m_ymin)*1.0001/m_yunit)*m_yunit;//one tick at the origin

    int XMin = qMax(int(m_xmin/m_scalex + m_ptoffset.x()), m_rCltRect.left());
    int XMax = qMin(int(m_xmax/m_scalex + m_ptoffset.x()), m_rCltRect.right());

    while(yt<=m_ymax*1.0001)
    {
        if(yt>=m_ymin)
        {
            painter.drawLine(XMin, int(yt/scaley)   + m_ptoffset.y(), XMax, int(yt/scaley)   + m_ptoffset.y());
        }
        yt += m_yunit ;
    }
    painter.restore();
}

void Graph::drawXMinGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(m_xunit)<0.00000001) return;
//    if(fabs(m_XMinorUnit)<0.00000001) return;
    if(fabs(m_xmax-m_xmin)/m_xunit>30.0) return;
//    if(fabs(xmax-xmin)/m_XMinorUnit>100.0) return;
    int YMin(0), YMax(0);

    painter.save();
    QPen GridPen(m_Grid.xMinColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMinStipple()));
    GridPen.setWidth(m_Grid.xMinWidth());
    painter.setPen(GridPen);


    YMin = int(m_ymin/scaley)+ m_ptoffset.y();
    YMax = int(m_ymax/scaley)+ m_ptoffset.y();

    double m_XMinorUnit = m_xunit/5.0;
    double xDelta = m_XMinorUnit;
    double xt = m_xo-int((m_xo-m_xmin)*1.0001/xDelta)*xDelta;//one tick at the origin


    while(xt<=m_xmax*1.001)
    {
        if(xt>=m_xmin)
        {
            painter.drawLine(int(xt/m_scalex) + m_ptoffset.x(), YMin, int(xt/m_scalex) + m_ptoffset.x(), YMax);
        }
        xt += xDelta;
    }
    painter.restore();
}

void Graph::drawYMinGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(m_yunit)<0.00000001) return;
//    if(fabs(m_YMinorUnit)<0.00000001) return;
    if(fabs(m_ymax-m_ymin)/m_yunit>30.0) return;
//    if(fabs(ymax-ymin)/m_YMinorUnit>100.0) return;

    double m_YMinorUnit = m_xunit/5.0;

    painter.save();
    QPen GridPen(m_Grid.yMinColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMinStipple(0)));
    GridPen.setWidth(m_Grid.yMinWidth(0));
    painter.setPen(GridPen);

    double yDelta = m_YMinorUnit;
    double yt = m_yo-int((m_yo-m_ymin)*1.0001/yDelta)*yDelta;//one tick at the origin
    int XMin = qMax(int(m_xmin/m_scalex + m_ptoffset.x()), m_rCltRect.left());
    int XMax = qMin(int(m_xmax/m_scalex + m_ptoffset.x()), m_rCltRect.right());

    while(yt<=m_ymax*1.0001)
    {
        if(yt>=m_ymin)
        {
            painter.drawLine(XMin, int(yt/scaley)   + m_ptoffset.y(), XMax, int(yt/scaley)   + m_ptoffset.y());
        }
        yt += yDelta ;
    }
    painter.restore();
}


void Graph::drawLegend(QPainter &painter, QPoint const &Place, QFont const&LegendFont, QColor const &LegendColor, QColor const &backColor)
{
    painter.save();
    int LegendSize(0), ypos(0);
    QString strong;

    LegendSize = 30;
    //    ypos = 12;
    QFontMetrics fm(LegendFont);
    ypos = fm.height();

    painter.setFont(LegendFont);

    painter.setBackgroundMode(Qt::TransparentMode);

    QPen TextPen(LegendColor);
    QPen LegendPen(Qt::gray);
    QBrush LegendBrush(backColor);
    painter.setBrush(LegendBrush);

    int npos = 0;
    for (int nc=0; nc< m_oaCurves.size(); nc++)
    {
        Curve const *pCurve = m_oaCurves.at(nc);
        if(pCurve->isVisible() || pCurve->pointsVisible())
        {
            pCurve->name(strong);
            if(pCurve->size()>0 && strong.length())//is there anything to draw ?
            {

                LegendPen.setColor(pCurve->color());
                LegendPen.setStyle(xfl::getStyle(pCurve->stipple()));
                LegendPen.setWidth(pCurve->width());

                painter.setPen(LegendPen);

                painter.drawLine(Place.x(),                   Place.y() + ypos*npos + ypos/3,
                                 Place.x() + int(LegendSize), Place.y() + ypos*npos + ypos/3);
                if(pCurve->pointsVisible())
                {
                    int x1 = Place.x() + int(LegendSize/2);
                    int y1 = Place.y() + int(ypos*npos+ ypos/3);

                    LegendPen.setStyle(Qt::SolidLine);
                    painter.setPen(LegendPen);
                    xfl::drawSymbol(painter, pCurve->pointStyle(), m_BackColor, pCurve->color(), QPoint(x1, y1));
                }

                painter.setPen(TextPen);
                painter.drawText(Place.x() + int(LegendSize*3/2),    Place.y()  + ypos*npos + int(ypos/2),
                                 strong);

                npos++;
            }
        }
    }

    painter.restore();
}


void Graph::expFormat(double &f, int &exp) const
{
    if (f==0.0)
    {
        exp = 0;
        f = 0.0;
        return;
    }
    double f1 = fabs(f);
    if(f1<1)
        exp = int(log10(f1)-1);
    else
        exp = int(log10(f1));

    f = f/pow(10.0,exp);
}


void Graph::exportToFile(QFile &XFile, bool bCSV)
{
    int maxpoints(0);
    QString strong;
    QTextStream out(&XFile);

    for(int i=0; i<m_oaCurves.size(); i++)
    {
        Curve *pCurve = curve(i);
        if(pCurve)
        {
            maxpoints = qMax(maxpoints,pCurve->size());

            pCurve->name(strong);
            if(!bCSV) out << "     "<<m_XTitle<<"       "<< strong <<"    ";
            else      out << m_XTitle<<","<< strong << ", , ";
        }
    }
    out<<"\n"; //end of title line

    for(int j=0; j<maxpoints; j++)
    {
        for(int i=0; i<m_oaCurves.size(); i++)
        {
            Curve *pCurve = curve(i);
            if(pCurve && j<pCurve->size())
            {
                if(!bCSV) strong= QString("%1     %2  ").arg(pCurve->x(j),13,'g',7).arg(pCurve->y(j),13,'g',7);
                else      strong= QString("%1, %2, , ").arg(pCurve->x(j),13,'g',7).arg(pCurve->y(j),13,'g',7);
            }
            else
            {
                if(!bCSV) strong= "                                 ";
                else      strong= ", , , ";
            }
            out << strong;
        }
        out<<"\n"; //end of data line
    }
    out<<"\n"; //end of file
    XFile.close();
}


void Graph::highlight(QPainter &painter, Curve *pCurve, int ref)
{
    if(!pCurve) return;
    if(ref<0 || ref>pCurve->size()-1) return;

    painter.save();
    int x = int(pCurve->x(ref)/m_scalex)  +m_ptoffset.x();
    int y = int(pCurve->y(ref)/m_scaley)  +m_ptoffset.y();

    QPen HighlightPen(QColor(255,100,100));
    HighlightPen.setWidth(2);
    painter.setPen(HighlightPen);
    int w = xfl::symbolSize();
    QRect r(x-w,y-w,2*w,2*w);
    painter.drawRect(r);
    painter.restore();
}


void Graph::saveSettings(QSettings &settings)
{
    QFont lgft;
    QColor clr;

    settings.beginGroup(m_GraphName);
    {
        //read variables
        m_BorderStyle.saveSettings(settings,"BorderStyle");

        clr = titleColor();
        settings.setValue("TitleColor", clr);

        clr = labelColor();
        settings.setValue("LabelColor", clr);

        getTitleFont(lgft);
        settings.setValue("TitleFontName", lgft.family());
        settings.setValue("TitleFontSize", lgft.pointSize());
        settings.setValue("TitleFontItalic", lgft.italic());
        settings.setValue("TitleFontBold", lgft.bold());

        getLabelFont(lgft);
        settings.setValue("LabelFontName", lgft.family());
        settings.setValue("LabelFontSize", lgft.pointSize());
        settings.setValue("LabelFontItalic", lgft.italic());
        settings.setValue("LabelFontBold", lgft.bold());

        settings.setValue("XOrigin", m_xo);
        settings.setValue("XMin",    m_xmin);
        settings.setValue("XMax",    m_xmax);
        settings.setValue("XUnit",   m_xunit);

        settings.setValue("YOrigin", m_yo);
        settings.setValue("YMin",    m_ymin);
        settings.setValue("YMax",    m_ymax);
        settings.setValue("YUnit",   m_yunit);

        clr = backgroundColor();
        settings.setValue("BackgroundColor", clr);

        settings.setValue("margin", m_iMargin);

        settings.setValue("Inverted", m_bYInverted);

        settings.setValue("XVariable", m_X);
        settings.setValue("YVariable", m_Y);

        settings.setValue("bAutoX",  m_bAutoX);
        settings.setValue("bAutoY",  m_bAutoY);
        m_Grid.saveSettings(settings);
    }
    settings.endGroup();
}


void Graph::loadSettings(QSettings &settings)
{
    QFont lgft;

    QColor clr;

    settings.beginGroup(m_GraphName);
    {
        //read variables
        m_BorderStyle.loadSettings(settings,"BorderStyle");

        clr = settings.value("TitleColor", m_TitleColor).value<QColor>();
        setTitleColor(clr);
        clr = settings.value("LabelColor", m_LabelColor).value<QColor>();
        setLabelColor(clr);

        if(settings.contains("TitleFontName"))
        {
            lgft = QFont(settings.value("TitleFontName", QString()).toString());
            int size = settings.value("TitleFontSize",8).toInt();
            if(size>0) lgft.setPointSize(size);
            lgft.setItalic(settings.value("TitleFontItalic", false).toBool());
            lgft.setBold(settings.value("TitleFontBold", false).toBool());
            setTitleFont(lgft);
        }

        if(settings.contains("LabelFontName"))
        {
            lgft = QFont(settings.value("LabelFontName", QString()).toString());
            int size = settings.value("LabelFontSize",8).toInt();
            if(size>0) lgft.setPointSize(size);
            lgft.setItalic(settings.value("LabelFontItalic", false).toBool());
            lgft.setBold(settings.value("LabelFontBold", false).toBool());
            setLabelFont(lgft);
        }

        m_xo       = settings.value("XOrigin", m_xo).toDouble();
        m_xmin     = settings.value("XMin",    m_xmin).toDouble();
        m_xmax     = settings.value("XMax",    m_xmax).toDouble();
        m_xunit    = settings.value("XUnit",   m_xunit).toDouble();

        m_yo       = settings.value("YOrigin", m_yo).toDouble();
        m_ymin     = settings.value("YMin",    m_ymin).toDouble();
        m_ymax     = settings.value("YMax",    m_ymax).toDouble();
        m_yunit    = settings.value("YUnit",   m_yunit).toDouble();


        clr  = settings.value("BackgroundColor", m_BackColor).value<QColor>();
        setBkColor(clr);

        m_iMargin = settings.value("margin", m_iMargin).toInt();

        m_bYInverted = settings.value("Inverted", m_bYInverted).toBool();

        m_X  = settings.value("XVariable",m_X).toInt();
        m_Y  = settings.value("YVariable",m_Y).toInt();

        m_bAutoX = settings.value("bAutoX", m_bAutoX).toBool();
        m_bAutoY = settings.value("bAutoY", m_bAutoY).toBool();

        m_Grid.loadSettings(settings);
    }
    settings.endGroup();
}


Curve* Graph::addCurve(QString const &name)
{
    Curve *pCurve= addCurve();
    if(pCurve) pCurve->setName(name);
    return pCurve;
}


Curve* Graph::addCurve()
{
    Curve *pCurve = new Curve();
    if(pCurve)
    {
        int nIndex = m_oaCurves.size();
        pCurve->setColor(s_CurveColors[nIndex%10]);
        pCurve->setStipple(Line::SOLID);
        pCurve->m_pParentGraph = this;
        m_oaCurves.append(pCurve);
    }
    return pCurve;
}


/**< In the case where a curve has been constructed independently and needs to be added to the Graph */
Curve* Graph::addCurve(Curve *pCurve)
{
    if(pCurve)
    {
        int nIndex = m_oaCurves.size();
        pCurve->setColor(s_CurveColors[nIndex%10]);
        pCurve->setStipple(Line::SOLID);
        pCurve->m_pParentGraph = this;
        m_oaCurves.append(pCurve);
    }
    return pCurve;
}



double Graph::clientTox(double x) const
{
    return (x-m_ptoffset.x())*m_scalex;
}

double Graph::clientToy(double y) const
{
    return (y-m_ptoffset.y())*m_scaley;
}

double Graph::clientTox(int x) const
{
    return (double(x)-double(m_ptoffset.x()))*m_scalex;
}

double Graph::clientToy(int y) const
{
    return (double(y)-double(m_ptoffset.y()))*m_scaley;
}


void Graph::copySettings(Graph *pGraph, bool bScales)
{
    if(!pGraph) return;

    m_LabelFont     = pGraph->m_LabelFont;
    m_TitleFont     = pGraph->m_TitleFont;

    if(bScales)
    {
        m_xmin            = pGraph->m_xmin;
        m_xmax            = pGraph->m_xmax;
        m_xo              = pGraph->m_xo;
        m_xunit           = pGraph->m_xunit;

        m_ymin            = pGraph->m_ymin;
        m_ymax            = pGraph->m_ymax;
        m_yo              = pGraph->m_yo;
        m_yunit           = pGraph->m_yunit;

        m_scalex        = pGraph->m_scalex;
        m_scaley        = pGraph->m_scaley;
    }


    m_BorderStyle   = pGraph->m_BorderStyle;

    m_BackColor       = pGraph->m_BackColor;
    m_LabelColor    = pGraph->m_LabelColor;
    m_TitleColor    = pGraph->m_TitleColor;

    m_bYInverted    = pGraph->m_bYInverted;

    m_iMargin       = pGraph->m_iMargin;

    m_bAutoX        = pGraph->m_bAutoX;
    m_bAutoY        = pGraph->m_bAutoY;

    m_Grid          = pGraph->m_Grid;
}


void Graph::deleteCurve(int index)
{
    Curve *pCurve = curve(index);
    if(pCurve)
    {
        m_oaCurves.removeAt(index);
        delete pCurve;
    }
}


void Graph::deleteCurve(Curve *pCurve)
{
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        Curve *pOldCurve = m_oaCurves.at(i);
        if(pOldCurve==pCurve)
        {
            m_oaCurves.removeAt(i);
            delete pCurve;
            return;
        }
    }
}


void Graph::deleteCurve(QString const &CurveTitle)
{
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        Curve *pOldCurve = m_oaCurves.at(i);
        if(pOldCurve  && pOldCurve->m_Name==CurveTitle)
        {
            m_oaCurves.removeAt(i);
            delete pOldCurve;
            return;
        }
    }
}


void Graph::deleteCurves()
{
    for (int i=m_oaCurves.size()-1; i>=0;i--)
        delete curve(i);

    m_oaCurves.clear();//removes the pointers

    if (m_bAutoX && !m_AutoScaleType)
    {
        m_xmin =  0.0;
        m_xmax =  0.1;
    }

    if (m_bAutoY && !m_AutoScaleType)
    {
        m_ymin =  0.0;
        m_ymax =  0.1;
    }
}


//___________________Start Gets______________________________________________________________


Curve* Graph::curve(int nIndex)
{
    if(m_oaCurves.size()>nIndex)
        return m_oaCurves[nIndex];
    else return nullptr;
}


Curve const* Graph::curveAt(int nIndex) const
{
    if(m_oaCurves.size()>nIndex)
        return m_oaCurves[nIndex];
    else return nullptr;
}


Curve* Graph::curve(QString const &CurveTitle)
{
    QString strong;

    for(int i=0; i<m_oaCurves.size(); i++)
    {
        Curve *pCurve = m_oaCurves.at(i);
        if(pCurve)
        {
            pCurve->name(strong);
            if(strong==CurveTitle) return pCurve;
        }
    }
    return nullptr;
}


bool Graph::initializeGraph(int width, int height)
{
    if(width>0 && height>0)
    {
        m_w =  width  -2*m_iMargin;
        m_h =  height -2*m_iMargin;
    }
    else
    {
        m_w =  m_rCltRect.width()  -2*m_iMargin;
        m_h =  m_rCltRect.height() -2*m_iMargin;
    }

    setXScale();
    setYScale();

    return true;
}


bool Graph::isInDrawRect(QPointF const &pt)
{
    if(m_rCltRect.contains(pt.toPoint())) return true;
    else return false;
}


bool Graph::isInDrawRect(QPoint const &pt)
{
    if(m_rCltRect.contains(pt)) return true;
    else return false;
}

bool Graph::isInDrawRect(int const &x, int const &y)
{
    if(m_rCltRect.contains(x,y)) return true;
    else return false;
}


void Graph::resetCurves()
{
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        Curve *pCurve = m_oaCurves.at(i);
        pCurve->clear();
    }
}


void Graph::resetLimits()
{
    resetXLimits();
    resetYLimits();
}


void Graph::resetXLimits()
{
    if(m_bAutoX)
    {
        m_xmin =  0.0;
        m_xmax =  0.1;
        m_xo   =  0.0;
    }
}


void Graph::resetYLimits()
{
    if(m_bAutoY)
    {
        m_ymin =   0.000;
        m_ymax =   0.001;
        m_yo   =   0.000;
    }
}


void Graph::scaleAxes(double zoom)
{
    if (zoom<0.01) zoom =0.01;
    m_bAutoX = false;
    m_bAutoY = false;

    double xm = (m_xmin + m_xmax)/2.0;
    m_xmin = xm+(m_xmin-xm)*zoom;
    m_xmax = xm+(m_xmax-xm)*zoom;

    double ym = (m_ymin + m_ymax)/2.0;
    m_ymin = ym+(m_ymin-ym)*zoom;
    m_ymax = ym+(m_ymax-ym)*zoom;
}


void Graph::scaleXAxis(double zoom)
{
    if (zoom<0.01) zoom =0.01;
    m_bAutoX = false;

    double xm = (m_xmin + m_xmax)/2.0;
    m_xmin = xm+(m_xmin-xm)*zoom;
    m_xmax = xm+(m_xmax-xm)*zoom;

}

void Graph::scaleYAxis(double zoom)
{
    if (zoom<0.01) zoom =0.01;
    m_bAutoY = false;

    double ym = (m_ymin + m_ymax)/2.0;
    m_ymin = ym+(m_ymin-ym)*zoom;
    m_ymax = ym+(m_ymax-ym)*zoom;
}


void Graph::setAuto(bool bAuto)
{
    m_bAutoX = bAuto;
    m_bAutoY = bAuto;
    resetXLimits();
    resetYLimits();
}


void Graph::setAutoX(bool bAuto)
{
    m_bAutoX = bAuto;
    resetXLimits();
}


void Graph::setAutoY(bool bAuto)
{
    m_bAutoY = bAuto;
    resetYLimits();
}


void Graph::setAutoXUnit()
{
    //    xunit = 100.0*m_scalex;
    m_xunit = (m_xmax-m_xmin)/3.0;

    if (m_xunit<1.0)
    {
        exp_x = int(log10(m_xunit*1.00001)-1);
        exp_x = qMax(-4, exp_x);
    }
    else exp_x = int(log10(m_xunit*1.00001));
    int main_x = int(m_xunit/pow(10.0, exp_x)*1.000001);


    if(main_x<2)
        m_xunit = pow(10.0,exp_x);
    else if (main_x<5)
        m_xunit = 2.0*pow(10.0,exp_x);
    else
        m_xunit = 5.0*pow(10.0,exp_x);
}



void Graph::setAutoYUnit()
{
    //    yunit = 100.0 * m_scaley;
    m_yunit = (m_ymax-m_ymin)/5.0;
    if (m_yunit<1.0)
    {
        exp_y = int(log10(m_yunit*1.00001)-1);
        //        exp_y = qMax(-4, exp_y);
    }
    else  exp_y = int(log10(m_yunit*1.00001));

    int main_y = int(m_yunit/pow(10.0, exp_y));

    if(main_y<2)
        m_yunit = pow(10.0,exp_y);
    else if (main_y<5)
        m_yunit = 2.0*pow(10.0,exp_y);
    else
        m_yunit = 5.0*pow(10.0,exp_y);
}


void Graph::setGraphDefaults(bool bDark)
{
    if(bDark)
    {
        m_BackColor = QColor(0,9,13);
        m_BorderStyle.m_Color = QColor(200,200,200);

        m_Grid.setDefaults();
        setTitleColor(QColor(255,255,255));
        setLabelColor(QColor(255,255,255));
    }
    else
    {
        m_BackColor = QColor(255,255,255);
        m_BorderStyle.m_Color = QColor(55,55,55);

        setTitleColor(QColor(0,0,0));
        setLabelColor(QColor(0,0,0));

    }

    m_BorderStyle.m_Stipple = Line::SOLID;
    m_BorderStyle.m_Width = 3;

    m_bYInverted = false;
}


void Graph::setLabelColor(QColor const &crColor)
{
    m_LabelColor = crColor;
}


void Graph::setInverted(bool bInverted)
{
    m_bYInverted = bInverted;
}


void Graph::setMargin(int m)
{
    m_iMargin = m;
}



void Graph::setVariables(int const & X, int const & Y)
{
    m_X = X;
    m_Y = Y;
}


void Graph::setWindow(double x1, double x2, double y1, double y2)
{
    m_bAutoX = false;
    m_bAutoY = false;
    m_xmin = x1;
    m_xmax = x2;
    m_ymin = y1;
    m_ymax = y2;
}


void Graph::setXMajGrid(bool const &state, QColor const &clr, int const &style, int const &width)
{
    m_Grid.setXMajStyle({state, LineStyle::convertLineStyle(style), width, clr, Line::NOSYMBOL});
}


void Graph::setYMajGrid(bool const &state, QColor const &clr, int const &style, int const &width)
{
    m_Grid.setYMajStyle(0, {state, LineStyle::convertLineStyle(style), width, clr, Line::NOSYMBOL});
}


bool Graph::setXScale()
{
    if(m_bAutoX)
    {
        bool bCurve = false;

        if (m_oaCurves.size())
        {
            //init only if we have a curve
            for (int nc=0; nc < m_oaCurves.size(); nc++)
            {
                Curve const *pCurve = m_oaCurves.at(nc);
                if ((pCurve->isVisible() ||pCurve->pointsVisible()) && pCurve->size()>1)
                {
                    bCurve = true;
                    break;//there is something to draw
                }
            }
        }
        if (bCurve)
        {
            Cxmin =  9999999.0;
            Cxmax = -9999999.0;
            for (int  nc=0; nc < m_oaCurves.size(); nc++)
            {
                Curve const *pCurve = m_oaCurves.at(nc);
                if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
                {
                    Cxmin = qMin(Cxmin, pCurve->xMin());
                    Cxmax = qMax(Cxmax, pCurve->xMax());
                }
            }

            if(Cxmax<=Cxmin)
                Cxmax = (Cxmin+1.0)*2.0;

            if(m_AutoScaleType == 1)
            {
                m_xmin = std::min(m_xmin, Cxmin);
                m_xmax = std::max(m_xmax, Cxmax);
            }
            else
            {
                m_xmin = Cxmin;
                m_xmax = Cxmax;
            }
            if(Cxmin>=0.0) m_xmin = 0.0;
            if(Cxmax<=0.0) m_xmax = 0.0;

        }
        else
        {
            // until things are made clear
            for (int  nc=0; nc < m_oaCurves.size(); nc++)
            {
                Curve const *pCurve = m_oaCurves.at(nc);
                if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
                {
                    m_xmin = std::min(m_xmin, pCurve->x(0));
                    m_xmax = std::max(m_xmax, pCurve->x(0));
                }
            }
        }
        m_xo=0.0;

        if(fabs((m_xmin-m_xmax)/m_xmin)<0.001)
        {
            if(fabs(m_xmin)<0.00001) m_xmax = 1.0;
            else
            {
                m_xmax = 2.0 * m_xmin;
                if(m_xmax < m_xmin)
                {
                    double tmp = m_xmax;
                    m_xmax = m_xmin;
                    m_xmin = tmp;
                }
            }
        }

        if(m_w<=0.0) return false;

        m_scalex   = (m_xmax-m_xmin)/m_w;


        //try to set an automatic scale for X Axis

        setAutoXUnit();
    }
    else
    {
        //scales are set manually
        if(m_w<=0.0) return false;

        //        m_scalex   =  (xmax-xmin)/m_w;
        if (m_xunit<1.0)
        {
            exp_x = int(log10(m_xunit*1.00001)-1);
            exp_x = qMax(-4, exp_x);
        }
        else exp_x = int(log10(m_xunit*1.00001));

    }
    m_scalex   =  (m_xmax-m_xmin)/m_w;

    //graph center position
    int Xg = (m_rCltRect.right() + m_rCltRect.left())/2;
    // curves center position
    int Xc = int((m_xmin+m_xmax)/2.0/m_scalex);
    // center graph in drawing rectangle
    m_ptoffset.rx() = (Xg-Xc);
    return true;
}


void Graph::setXUnit(double f)
{
    m_xunit = f;
}


void Graph::setXTitle(QString const &str)
{
    m_XTitle = str;
}


void Graph::setXVariable(int const & X)
{
    m_X = X;
}


void Graph::setYMin(double f)
{
    m_ymin = f;
}


void Graph::setYMax(double f)
{
    m_ymax = f;
}

void Graph::setY0(double f)
{
    m_yo = f;
}

void Graph::setYTitle(QString const &str)
{
    m_YTitle = str;
}


void Graph::setYUnit(double f)
{
    m_yunit = f;
}


bool Graph::setYScale()
{
    if(m_bAutoY)
    {
        bool bCurve = false;
        if (m_oaCurves.size())
        {
            //init only if we have a curve
            for (int nc=0; nc < m_oaCurves.size(); nc++)
            {
                Curve const *pCurve = m_oaCurves.at(nc);
                if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
                {
                    bCurve = true;
                    break;
                }
            }
        }
        if(bCurve)
        {
            Cymin =  9999999.0;
            Cymax = -9999999.0;
            for (int nc=0; nc < m_oaCurves.size(); nc++)
            {
                Curve const *pCurve = m_oaCurves.at(nc);
                if ((pCurve->isVisible() ||pCurve->pointsVisible()) && pCurve->size()>0)
                {
                    Cymin = std::min(Cymin, pCurve->yMin());
                    Cymax = std::max(Cymax, pCurve->yMax());
                }
            }
            if(Cymax<=Cymin)
            {
                Cymax = (Cymin+1.0)*2.0;
            }

            if(m_AutoScaleType == 1)
            {
                m_ymin = std::min(m_ymin, Cymin);
                m_ymax = std::max(m_ymax, Cymax);
            }
            else
            {
                m_ymin = Cymin;
                m_ymax = Cymax;
                if(Cymin>=0.0) m_ymin = 0.0;
                if(Cymax<=0.0) m_ymax = 0.0;
            }
        }
        else
        {
            // until things are made clear
            for (int nc=0; nc<m_oaCurves.size(); nc++)
            {
                Curve const *pCurve = m_oaCurves.at(nc);
                if ((pCurve->isVisible()||pCurve->pointsVisible())  && pCurve->size()>0)
                {
                    m_ymin = std::min(m_ymin, pCurve->y(0));
                    m_ymax = std::max(m_ymax, pCurve->y(0));
                }
            }
        }
        m_yo=0.0;

        if (fabs((m_ymin-m_ymax)/m_ymin)<0.001)
        {
            if(fabs(m_ymin)<0.00001) m_ymax = 1.0;
            else
            {
                m_ymax = 2.0 * m_ymin;
                if(m_ymax < m_ymin)
                {
                    double tmp = m_ymax;
                    m_ymax = m_ymin;
                    m_ymin = tmp;
                }
            }
        }

        if(m_h<=0.0) return false;

        if (!m_bYInverted)
        {
            m_scaley   = -(m_ymax-m_ymin)/m_h;
        }
        else
        {
            m_scaley   =  (m_ymax-m_ymin)/m_h;
        }

        //try to set an automatic scale for Y Axis
        setAutoYUnit();
    }
    else
    {
        //scales are set manually
        if(m_h<=0) return false;

        if (!m_bYInverted)
        {
            m_scaley   = -(m_ymax-m_ymin)/m_h;
        }
        else
        {
            m_scaley   =  (m_ymax-m_ymin)/m_h;
        }

        if (m_yunit<1.0)
        {
            exp_y = int(log10(m_yunit*1.00001)-1);
            exp_y = qMax(-4, exp_y);
        }
        else  exp_y = int(log10(m_yunit*1.00001));

    }

    //graph center position
    int Yg = (m_rCltRect.top() + m_rCltRect.bottom())/2;
    // curves center position
    int Yc = int((m_ymin+m_ymax)/2.0/m_scaley);
    // center graph in drawing rectangle
    m_ptoffset.ry() = (Yg-Yc);

    return true;
}


void Graph::setYVariable(int const & Y)
{
    m_Y = Y;
}


int Graph::xToClient(double x) const
{
    return int(x/m_scalex + m_ptoffset.x());
}


int Graph::yToClient(double y) const
{
    return int(y/m_scaley + m_ptoffset.y());
}





