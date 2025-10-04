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

#include <QPainter>
#include <QFontMetrics>
#include <QTextStream>

#include <xflgraph/graph.h>
#include <xflgraph/curve.h>
#include <xflcore/xflcore.h>

#define MININTERVAL  0.000000001


bool Graph::s_bHighlightPoint = false;


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

    xmin         = .0;
    xmax         = .10;
    ymin         = .0;
    ymax         = .10;
    xunit        = 0.1;
    yunit        = 0.1;
    xo           = 0.0;
    yo           = 0.0;

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

    m_iMargin = 41;

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
    else                           color = m_BkColor;
    QPen BorderPen(color);
    BorderPen.setStyle(xfl::getStyle(m_BorderStyle.m_Stipple));
    BorderPen.setWidth(m_BorderStyle.m_Width);

    painter.setPen(BorderPen);
    painter.fillRect(m_rCltRect, m_BkColor);
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
    double scaley;
    QPoint To, Min, Max;
    QRect rViewRect;

    int ptside = 5;
    Curve* pCurve = curve(nIndex);

    scaley = m_scaley;

    QBrush FillBrush(m_BkColor);
    painter.setBrush(FillBrush);

    QPen CurvePen(pCurve->color());
    CurvePen.setStyle(xfl::getStyle(pCurve->stipple()));
    CurvePen.setWidth(std::max(pCurve->width(), 1));
    painter.setPen(CurvePen);

    Min.setX(int(xmin/m_scalex) +m_ptoffset.x());
    Min.setY(int(ymin/scaley)   +m_ptoffset.y());
    Max.setX(int(xmax/m_scalex) +m_ptoffset.x());
    Max.setY(int(ymax/scaley)   +m_ptoffset.y());
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
            xfl::drawSymbol(painter, pCurve->pointStyle(), m_BkColor, pCurve->color(), polycurve.at(i).x(), polycurve.at(i).y());
        }
    }

    if(s_bHighlightPoint)
    {
        int point = pCurve->selected();
        if(point>=0)
        {
            //highlight
            QColor HighColor(200, 100, 77);
            CurvePen.setWidth(pCurve->width()+1);
            CurvePen.setColor(HighColor);
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
    double xp=0, yp=0;
    QPen AxesPen;
    double scaley = m_scaley;
    painter.save();

    AxesPen.setColor(m_Grid.xAxisColor());
    AxesPen.setStyle(xfl::getStyle(m_Grid.xAxisStipple()));
    AxesPen.setWidth(m_Grid.xAxisWidth());
    painter.setPen(AxesPen);

    //vertical axis
    if(xo>=xmin && xo<=xmax) xp = xo;
    else if(xo>xmax)         xp = xmax;
    else                     xp = xmin;

    painter.drawLine(int(xp/m_scalex) + m_ptoffset.x(), int(ymin/scaley) + m_ptoffset.y(),
                     int(xp/m_scalex) + m_ptoffset.x(), int(ymax/scaley) + m_ptoffset.y());

    //horizontal axis
    if(yo>=ymin && yo<=ymax) yp = yo;
    else if(yo>ymax)         yp = ymax;
    else                     yp = ymin;


    painter.drawLine(int(xmin/m_scalex) +m_ptoffset.x(), int(yp/scaley) + m_ptoffset.y(),
                     int(xmax/m_scalex) +m_ptoffset.x(), int( yp/scaley) + m_ptoffset.y());

    painter.restore();
}


void Graph::drawTitles(QPainter &painter)
{
    //draws the x & y axis name

    double xp=0, yp=0;

    double scaley = m_scaley;
    painter.save();
    int XPosXTitle = 5;
    int YPosXTitle = -10;
    int XPosYTitle = -5;
    int YPosYTitle =  5;

    if(xo>=xmin && xo<=xmax) xp = xo;
    else if(xo>xmax)         xp = xmax;
    else                     xp = xmin;

    if(yo>=ymin && yo<=ymax) yp = yo;
    else if(yo>ymax)         yp = ymax;
    else                     yp = ymin;

    painter.setFont(m_TitleFont);

    QPen TitlePen(m_TitleColor);
    painter.setPen(TitlePen);

    painter.drawText(  int(xmax/m_scalex) + m_ptoffset.x() + XPosXTitle,
                       int(yp  /scaley)   + m_ptoffset.y() + YPosXTitle, m_XTitle);

    painter.drawText(  m_ptoffset.x() + int(xp/m_scalex)   + XPosYTitle,
                       m_rCltRect.top() + m_iMargin - YPosYTitle, m_YTitle);
    painter.restore();
}


void Graph::drawXTicks(QPainter &painter)
{
    int exp=0;

    if(fabs(xunit)<0.00000001) return;
    if(fabs(xmax-xmin)/xunit>30.0) return;

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
    double xt = xo-(xo-xmin);//one tick at the origin
    int nx = int((xo-xmin)/xunit);
    xt = xo - nx*xunit;

    double yp = 0;
    if(yo>=ymin && yo<=ymax) yp = yo;
    else if(yo>ymax)         yp = ymax;
    else                     yp = ymin;

    int iTick = 0;

    while(fabs(xunit)>MININTERVAL && xt<=xmax*1.0001 && iTick<100)
    {
        //Draw ticks
        if(xt>=xmin)
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
        xt += xunit ;
        iTick++;
    }
    painter.restore();
}


void Graph::drawYTicks(QPainter &painter)
{
    if(fabs(xunit)<0.00000001) return;
    if(fabs(ymax-ymin)/yunit>30.0) return;
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
    if(xo>=xmin && xo<=xmax) xp = xo;
    else if(xo>xmax)         xp = xmax;
    else                     xp = xmin;

    double yt = yo-int((yo-ymin)*1.0001/yunit)*yunit;//one tick at the origin

    int iTick=0;

    while(fabs(yunit)>MININTERVAL && yt<=ymax*1.0001 && iTick<100)
    {
        //Draw ticks
        if(yt>=ymin)
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
        yt += yunit ;
        iTick++;
    }
    painter.restore();
}


void Graph::drawXMajGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(xunit)<0.00000001)     return;
    if(fabs(xmax-xmin)/xunit>30.0) return;

    painter.save();
    int YMin, YMax;

    QPen GridPen(m_Grid.xMajColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    GridPen.setWidth(m_Grid.xMajWidth());
    painter.setPen(GridPen);

    YMin = int(ymin/scaley) + m_ptoffset.y();
    YMax = int(ymax/scaley) + m_ptoffset.y();


    double xt = xo-int((xo-xmin)*1.0001/xunit)*xunit;//one tick at the origin
    while(xt<=xmax*1.001)
    {
        if(xt>=xmin)
        {
            painter.drawLine(int(xt/m_scalex) + m_ptoffset.x(), YMin, int(xt/m_scalex) + m_ptoffset.x(), YMax);
        }
        xt += xunit ;
    }
    painter.restore();
}


void Graph::drawYMajGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(yunit)<0.00000001) return;
    if(fabs(ymax-ymin)/yunit>30.0) return;

    painter.save();

    QPen GridPen(m_Grid.yMajColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMajStipple(0)));
    GridPen.setWidth(m_Grid.yMajWidth(0));
    painter.setPen(GridPen);


    double yt = yo-int((yo-ymin)*1.0001/yunit)*yunit;//one tick at the origin

    int XMin = qMax(int(xmin/m_scalex + m_ptoffset.x()), m_rCltRect.left());
    int XMax = qMin(int(xmax/m_scalex + m_ptoffset.x()), m_rCltRect.right());

    while(yt<=ymax*1.0001)
    {
        if(yt>=ymin)
        {
            painter.drawLine(XMin, int(yt/scaley)   + m_ptoffset.y(), XMax, int(yt/scaley)   + m_ptoffset.y());
        }
        yt += yunit ;
    }
    painter.restore();
}

void Graph::drawXMinGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(xunit)<0.00000001) return;
//    if(fabs(m_XMinorUnit)<0.00000001) return;
    if(fabs(xmax-xmin)/xunit>30.0) return;
//    if(fabs(xmax-xmin)/m_XMinorUnit>100.0) return;
    int YMin(0), YMax(0);

    painter.save();
    QPen GridPen(m_Grid.xMinColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMinStipple()));
    GridPen.setWidth(m_Grid.xMinWidth());
    painter.setPen(GridPen);


    YMin = int(ymin/scaley)+ m_ptoffset.y();
    YMax = int(ymax/scaley)+ m_ptoffset.y();

    double m_XMinorUnit = xunit/5.0;
    double xDelta = m_XMinorUnit;
    double xt = xo-int((xo-xmin)*1.0001/xDelta)*xDelta;//one tick at the origin


    while(xt<=xmax*1.001)
    {
        if(xt>=xmin)
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
    if(fabs(yunit)<0.00000001) return;
//    if(fabs(m_YMinorUnit)<0.00000001) return;
    if(fabs(ymax-ymin)/yunit>30.0) return;
//    if(fabs(ymax-ymin)/m_YMinorUnit>100.0) return;

    double m_YMinorUnit = xunit/5.0;

    painter.save();
    QPen GridPen(m_Grid.yMinColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMinStipple(0)));
    GridPen.setWidth(m_Grid.yMinWidth(0));
    painter.setPen(GridPen);

    double yDelta = m_YMinorUnit;
    double yt = yo-int((yo-ymin)*1.0001/yDelta)*yDelta;//one tick at the origin
    int XMin = qMax(int(xmin/m_scalex + m_ptoffset.x()), m_rCltRect.left());
    int XMax = qMin(int(xmax/m_scalex + m_ptoffset.x()), m_rCltRect.right());

    while(yt<=ymax*1.0001)
    {
        if(yt>=ymin)
        {
            painter.drawLine(XMin, int(yt/scaley)   + m_ptoffset.y(), XMax, int(yt/scaley)   + m_ptoffset.y());
        }
        yt += yDelta ;
    }
    painter.restore();
}


void Graph::drawLegend(QPainter &painter, QPoint &Place, QFont const&LegendFont, QColor const &LegendColor, QColor const &backColor)
{
    painter.save();
    int LegendSize, ypos;
    QString strong;

    LegendSize = 30;
    //    ypos = 12;
    QFontMetrics fm(LegendFont);
    ypos = fm.height();

    painter.setFont(LegendFont);

    painter.setBackgroundMode(Qt::TransparentMode);

    Curve* pCurve;

    QPen TextPen(LegendColor);
    QPen LegendPen(Qt::gray);
    QBrush LegendBrush(backColor);
    painter.setBrush(LegendBrush);

    int npos = 0;
    for (int nc=0; nc< m_oaCurves.size(); nc++)
    {
        pCurve = m_oaCurves[nc];
        if(pCurve->isVisible())
        {
            pCurve->curveName(strong);
            if(pCurve->size()>0 && strong.length())//is there anything to draw ?
            {

                LegendPen.setColor(pCurve->color());
                LegendPen.setStyle(xfl::getStyle(pCurve->stipple()));
                LegendPen.setWidth(pCurve->width());

                painter.setPen(LegendPen);

                painter.drawLine(Place.x(),                     Place.y() + ypos*npos + ypos/3,
                                 Place.x() + int(LegendSize), Place.y() + ypos*npos + ypos/3);
                if(pCurve->pointStyle())
                {
                    int x1 = Place.x() + int(LegendSize/2);
                    int y1 = Place.y() + int(ypos*npos+ ypos/3);

                    xfl::drawSymbol(painter, pCurve->pointStyle(), m_BkColor, pCurve->color(), QPoint(x1, y1));
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
    int i,j, maxpoints;
    Curve *pCurve;
    QString strong;
    QTextStream out(&XFile);

    maxpoints = 0;
    for(i=0; i<m_oaCurves.size(); i++)
    {
        pCurve = curve(i);
        if(pCurve)
        {
            maxpoints = qMax(maxpoints,pCurve->size());

            pCurve->curveName(strong);
            if(!bCSV) out << "     "<<m_XTitle<<"       "<< strong <<"    ";
            else      out << m_XTitle<<","<< strong << ", , ";
        }
    }
    out<<"\n"; //end of title line

    for(j=0; j<maxpoints; j++)
    {
        for(i=0; i<m_oaCurves.size(); i++)
        {
            pCurve = curve(i);
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
    QRect r(x-3,y-3,6,6);
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

        settings.setValue("XOrigin", xo);
        settings.setValue("XMin",    xmin);
        settings.setValue("XMax",    xmax);
        settings.setValue("XUnit",   xunit);

        settings.setValue("YOrigin", yo);
        settings.setValue("YMin",    ymin);
        settings.setValue("YMax",    ymax);
        settings.setValue("YUnit",   yunit);

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


        clr = settings.value("TitleColor", QColor(255,255,255)).value<QColor>();
        setTitleColor(clr);
        clr = settings.value("LabelColor", QColor(255,255,255)).value<QColor>();
        setLabelColor(clr);

        lgft = QFont(settings.value("TitleFontName", QString()).toString());
        int size = settings.value("TitleFontSize",8).toInt();
        if(size>0) lgft.setPointSize(size);
        lgft.setItalic(settings.value("TitleFontItalic", false).toBool());
        lgft.setBold(settings.value("TitleFontBold", false).toBool());
        setTitleFont(lgft);

        lgft = QFont(settings.value("LabelFontName", QString()).toString());
        size = settings.value("LabelFontSize",8).toInt();
        if(size>0) lgft.setPointSize(size);
        lgft.setItalic(settings.value("LabelFontItalic", false).toBool());
        lgft.setBold(settings.value("LabelFontBold", false).toBool());
        setLabelFont(lgft);

        xo       = settings.value("XOrigin", 0.0).toDouble();
        xmin     = settings.value("XMin",    0.0).toDouble();
        xmax     = settings.value("XMax",    1.0).toDouble();
        xunit    = settings.value("XUnit",   0.2).toDouble();

        yo       = settings.value("YOrigin", 0.0).toDouble();
        ymin     = settings.value("YMin",    0.0).toDouble();
        ymax     = settings.value("YMax",    1.0).toDouble();
        yunit    = settings.value("YUnit",   0.2).toDouble();


        clr  = settings.value("BackgroundColor", QColor(15,19,20)).value<QColor>();
        setBkColor(clr);

        m_iMargin = settings.value("margin", 61).toInt();

        m_bYInverted = settings.value("Inverted", false).toBool();

        m_X  = settings.value("XVariable",0).toInt();
        m_Y  = settings.value("YVariable",0).toInt();

        m_bAutoX = settings.value("bAutoX", true).toBool();
        m_bAutoY = settings.value("bAutoY", true).toBool();

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
        pCurve->setStipple(0);
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
        pCurve->setStipple(0);
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
        xmin            = pGraph->xmin;
        xmax            = pGraph->xmax;
        xo              = pGraph->xo;
        xunit           = pGraph->xunit;

        ymin            = pGraph->ymin;
        ymax            = pGraph->ymax;
        yo              = pGraph->yo;
        yunit           = pGraph->yunit;

        m_scalex        = pGraph->m_scalex;
        m_scaley        = pGraph->m_scaley;
    }


    m_BorderStyle   = pGraph->m_BorderStyle;

    m_BkColor       = pGraph->m_BkColor;
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
    Curve * pCurve = curve(index);
    m_oaCurves.removeAt(index);
    delete pCurve;
}


void Graph::deleteCurve(Curve *pCurve)
{
    Curve *pOldCurve = nullptr;
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        pOldCurve = m_oaCurves.at(i);
        if(pOldCurve==pCurve)
        {
            m_oaCurves.removeAt(i);
            delete pCurve;
            return;
        }
    }
}


void Graph::deleteCurve(QString CurveTitle)
{
    Curve *pOldCurve = nullptr;
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        pOldCurve = m_oaCurves.at(i);
        if(pOldCurve->m_CurveName==CurveTitle)
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
        xmin =  0.0;
        xmax =  0.1;
    }

    if (m_bAutoY && !m_AutoScaleType)
    {
        ymin =  0.0;
        ymax =  0.1;
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
    Curve * pCurve;
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        pCurve = m_oaCurves.at(i);
        if(pCurve)
        {
            pCurve->curveName(strong);
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
    Curve *pCurve;
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        pCurve = m_oaCurves.at(i);
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
        xmin =  0.0;
        xmax =  0.1;
        xo   =  0.0;
    }
}


void Graph::resetYLimits()
{
    if(m_bAutoY)
    {
        ymin =   0.000;
        ymax =   0.001;
        yo   =   0.000;
    }
}


void Graph::scaleAxes(double zoom)
{
    if (zoom<0.01) zoom =0.01;
    m_bAutoX = false;
    m_bAutoY = false;

    double xm = (xmin + xmax)/2.0;
    xmin = xm+(xmin-xm)*zoom;
    xmax = xm+(xmax-xm)*zoom;

    double ym = (ymin + ymax)/2.0;
    ymin = ym+(ymin-ym)*zoom;
    ymax = ym+(ymax-ym)*zoom;
}


void Graph::scaleXAxis(double zoom)
{
    if (zoom<0.01) zoom =0.01;
    m_bAutoX = false;

    double xm = (xmin + xmax)/2.0;
    xmin = xm+(xmin-xm)*zoom;
    xmax = xm+(xmax-xm)*zoom;

}

void Graph::scaleYAxis(double zoom)
{
    if (zoom<0.01) zoom =0.01;
    m_bAutoY = false;

    double ym = (ymin + ymax)/2.0;
    ymin = ym+(ymin-ym)*zoom;
    ymax = ym+(ymax-ym)*zoom;
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
    xunit = (xmax-xmin)/3.0;

    if (xunit<1.0)
    {
        exp_x = int(log10(xunit*1.00001)-1);
        exp_x = qMax(-4, exp_x);
    }
    else exp_x = int(log10(xunit*1.00001));
    int main_x = int(xunit/pow(10.0, exp_x)*1.000001);


    if(main_x<2)
        xunit = pow(10.0,exp_x);
    else if (main_x<5)
        xunit = 2.0*pow(10.0,exp_x);
    else
        xunit = 5.0*pow(10.0,exp_x);
}



void Graph::setAutoYUnit()
{
    //    yunit = 100.0 * m_scaley;
    yunit = (ymax-ymin)/5.0;
    if (yunit<1.0)
    {
        exp_y = int(log10(yunit*1.00001)-1);
        //        exp_y = qMax(-4, exp_y);
    }
    else  exp_y = int(log10(yunit*1.00001));

    int main_y = int(yunit/pow(10.0, exp_y));

    if(main_y<2)
        yunit = pow(10.0,exp_y);
    else if (main_y<5)
        yunit = 2.0*pow(10.0,exp_y);
    else
        yunit = 5.0*pow(10.0,exp_y);
}


void Graph::setGraphDefaults(bool bDark)
{
    if(bDark)
    {
        m_BkColor = QColor(0,9,13);
        m_BorderStyle.m_Color = QColor(200,200,200);

        m_Grid.setDefaults();
        setTitleColor(QColor(255,255,255));
        setLabelColor(QColor(255,255,255));
    }
    else
    {
        m_BkColor = QColor(255,255,255);
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
    xmin = x1;
    xmax = x2;
    ymin = y1;
    ymax = y2;
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
    Curve *pCurve(nullptr);
    int nc(0);

    if(m_bAutoX)
    {
        bool bCurve = false;

        if (m_oaCurves.size())
        {
            //init only if we have a curve
            for (nc=0; nc < m_oaCurves.size(); nc++)
            {
                pCurve = m_oaCurves[nc];
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
            for (nc=0; nc < m_oaCurves.size(); nc++)
            {
                pCurve = m_oaCurves[nc];
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
                xmin = std::min(xmin, Cxmin);
                xmax = std::max(xmax, Cxmax);
            }
            else
            {
                xmin = Cxmin;
                xmax = Cxmax;
            }
            if(Cxmin>=0.0) xmin = 0.0;
            if(Cxmax<=0.0) xmax = 0.0;

        }
        else
        {
            // until things are made clear
            for (nc=0; nc < m_oaCurves.size(); nc++)
            {
                pCurve = m_oaCurves[nc];
                if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
                {
                    xmin = std::min(xmin, pCurve->x(0));
                    xmax = std::max(xmax, pCurve->x(0));
                }
            }
        }
        xo=0.0;

        if(fabs((xmin-xmax)/xmin)<0.001)
        {
            if(fabs(xmin)<0.00001) xmax = 1.0;
            else
            {
                xmax = 2.0 * xmin;
                if(xmax < xmin)
                {
                    double tmp = xmax;
                    xmax = xmin;
                    xmin = tmp;
                }
            }
        }

        if(m_w<=0.0) return false;

        m_scalex   = (xmax-xmin)/m_w;


        //try to set an automatic scale for X Axis

        setAutoXUnit();
    }
    else
    {
        //scales are set manually
        if(m_w<=0.0) return false;

        //        m_scalex   =  (xmax-xmin)/m_w;
        if (xunit<1.0)
        {
            exp_x = int(log10(xunit*1.00001)-1);
            exp_x = qMax(-4, exp_x);
        }
        else exp_x = int(log10(xunit*1.00001));

    }
    m_scalex   =  (xmax-xmin)/m_w;

    //graph center position
    int Xg = (m_rCltRect.right() + m_rCltRect.left())/2;
    // curves center position
    int Xc = int((xmin+xmax)/2.0/m_scalex);
    // center graph in drawing rectangle
    m_ptoffset.rx() = (Xg-Xc);
    return true;
}


void Graph::setXUnit(double f){
    xunit = f;
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
    ymin = f;
}


void Graph::setYMax(double f)
{
    ymax = f;
}

void Graph::setY0(double f)
{
    yo = f;
}

void Graph::setYTitle(QString const &str)
{
    m_YTitle = str;
}


void Graph::setYUnit(double f)
{
    yunit = f;
}



bool Graph::setYScale()
{
    int nc;
    Curve *pCurve;

    if(m_bAutoY)
    {
        bool bCurve = false;
        if (m_oaCurves.size())
        {
            //init only if we have a curve
            for (nc=0; nc < m_oaCurves.size(); nc++)
            {
                pCurve = m_oaCurves[nc];
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
            for (nc=0; nc < m_oaCurves.size(); nc++)
            {
                pCurve = m_oaCurves[nc];
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
                ymin = std::min(ymin, Cymin);
                ymax = std::max(ymax, Cymax);
            }
            else
            {
                ymin = Cymin;
                ymax = Cymax;
                if(Cymin>=0.0) ymin = 0.0;
                if(Cymax<=0.0) ymax = 0.0;
            }
        }
        else
        {
            // until things are made clear
            for (int nc=0; nc<m_oaCurves.size(); nc++)
            {
                pCurve = m_oaCurves[nc];
                if ((pCurve->isVisible()||pCurve->pointsVisible())  && pCurve->size()>0)
                {
                    ymin = std::min(ymin, pCurve->y(0));
                    ymax = std::max(ymax, pCurve->y(0));
                }
            }
        }
        yo=0.0;

        if (fabs((ymin-ymax)/ymin)<0.001)
        {
            if(fabs(ymin)<0.00001) ymax = 1.0;
            else
            {
                ymax = 2.0 * ymin;
                if(ymax < ymin)
                {
                    double tmp = ymax;
                    ymax = ymin;
                    ymin = tmp;
                }
            }
        }

        if(m_h<=0.0) return false;

        if (!m_bYInverted)
        {
            m_scaley   = -(ymax-ymin)/m_h;
        }
        else
        {
            m_scaley   =  (ymax-ymin)/m_h;
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
            m_scaley   = -(ymax-ymin)/m_h;
        }
        else
        {
            m_scaley   =  (ymax-ymin)/m_h;
        }

        if (yunit<1.0)
        {
            exp_y = int(log10(yunit*1.00001)-1);
            exp_y = qMax(-4, exp_y);
        }
        else  exp_y = int(log10(yunit*1.00001));

    }

    //graph center position
    int Yg = (m_rCltRect.top() + m_rCltRect.bottom())/2;
    // curves center position
    int Yc = int((ymin+ymax)/2.0/m_scaley);
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


Curve*  Graph::getClosestPoint(const double &x, const double &y, double &xSel, double &ySel, int &nSel)
{
    int i, n1;
    double dist, dmax, x1, y1;
    dmax = 1.e40;
    Curve *pOldCurve, *pCurveSel;
    pCurveSel = nullptr;

    for(i=0; i<m_oaCurves.size(); i++)
    {
        pOldCurve = m_oaCurves.at(i);
        pOldCurve->closestPoint(x, y, x1, y1, dist, n1);
        if(dist<dmax)
        {
            dmax = dist;
            xSel = x1;
            ySel = y1;
            pCurveSel = pOldCurve;
            nSel = i;
        }
    }
    return pCurveSel;
}


Curve* Graph::getCurvePoint(const int &xClt, const int &yClt,int &nSel)
{
    int i, n, xc, yc;
    double dist, x1, y1, x,y;
    Curve *pOldCurve;

    x= clientTox(xClt);
    y= clientToy(yClt);
    for(i=0; i<m_oaCurves.size(); i++)
    {
        pOldCurve = m_oaCurves.at(i);
        pOldCurve->closestPoint(x, y, x1, y1, dist, n);

        xc = xToClient(x1);
        yc = yToClient(y1);

        if((xClt-xc)*(xClt-xc) + (yClt-yc)*(yClt-yc) <16)//sqrt(16) pixels distance
        {
            nSel = n;
            return pOldCurve;
        }
    }
    nSel = -1;
    return  nullptr;
}


bool Graph::selectPoint(QString const &CurveName, int sel)
{
    QString str;
    Curve *pCurve = nullptr;

    if(sel<0)
    {
        //        pCurve->SetSelected(-1);
        return false;
    }

    for(int i=0; i<m_oaCurves.size(); i++)
    {
        pCurve = m_oaCurves.at(i);
        pCurve->curveName(str);
        if(str == CurveName)
        {
            if(sel>pCurve->count())
            {
                pCurve->setSelected(-1);
                return false;
            }
            else
            {
                pCurve->setSelected(sel);
                return true;
            }
        }
    }
    //    pCurve->SetSelected(-1);
    return false;
}


void Graph::deselectPoint()
{
    Curve *pCurve;
    for(int i=0; i<m_oaCurves.size(); i++)
    {
        pCurve = m_oaCurves.at(i);
        pCurve->setSelected(-1);
    }
}







