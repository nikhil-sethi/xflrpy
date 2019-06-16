/****************************************************************************

    Graph Classes
    Copyright (C) 2003-2019 Andre Deperrois

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


#include <math.h>

#include <graph/graph.h>
#include <graph/curve.h>
#include <graph/graph_globals.h>

#include <math.h>
#include <QPainter>
#include <QFontMetrics>
#include <QTextStream>

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
    m_XMinorUnit = 0.01;
    m_YMinorUnit = 0.01;

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
    m_bXAutoMinGrid = true;
    m_bYAutoMinGrid = true;
    m_bBorder       = true;

    m_ptoffset.rx() = 0;
    m_ptoffset.ry() = 0;

    m_AxisStyle   = 0;
    m_AxisWidth   = 1;
    m_BorderStyle = 0;
    m_BorderWidth = 2;
    m_XMajStyle   = 0;
    m_XMajWidth   = 1;
    m_XMinStyle   = 1;
    m_XMinWidth   = 1;
    m_YMajStyle   = 0;
    m_YMajWidth   = 1;
    m_YMinStyle   = 1;
    m_YMinWidth   = 1;

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

    //	Draw Border
    if(m_bBorder) color = m_BorderColor;
    else          color = m_BkColor;
    QPen BorderPen(color);
    BorderPen.setStyle(getStyle(m_BorderStyle));
    BorderPen.setWidth(m_BorderWidth);

    painter.setPen(BorderPen);
    painter.fillRect(m_rCltRect, m_BkColor);
    painter.drawRect(m_rCltRect);
    initializeGraph();

    painter.setClipRect(m_rCltRect);

    painter.setBackgroundMode(Qt::TransparentMode);

    if(m_bXMinGrid) drawXMinGrid(painter);
    if(m_bYMinGrid) drawYMinGrid(painter);
    if(m_bXMajGrid) drawXMajGrid(painter);
    if(m_bYMajGrid) drawYMajGrid(painter);

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
    int i, ptside;
    QPoint From, To, Min, Max;
    QRect rViewRect;

    ptside = 5;
    Curve* pCurve = curve(nIndex);

    scaley = m_scaley;

    QBrush FillBrush(m_BkColor);
    painter.setBrush(FillBrush);

    QPen CurvePen(pCurve->color());
    CurvePen.setStyle(getStyle(pCurve->style()));
    CurvePen.setWidth((int)pCurve->width());
    painter.setPen(CurvePen);

    Min.setX(int(xmin/m_scalex) +m_ptoffset.x());
    Min.setY(int(ymin/scaley) +m_ptoffset.y());
    Max.setX(int(xmax/m_scalex) +m_ptoffset.x());
    Max.setY(int(ymax/scaley) +m_ptoffset.y());
    rViewRect.setTopLeft(Min);
    rViewRect.setBottomRight(Max);

    if(pCurve->size()>=1)
    {
        From.setX(int(pCurve->x[0]/m_scalex+m_ptoffset.x()));
        From.setY(int(pCurve->y[0]/scaley  +m_ptoffset.y()));

        if(pCurve->isVisible())
        {
            for (i=1; i<pCurve->size();i++)
            {
                To.setX(int(pCurve->x[i]/m_scalex+m_ptoffset.x()));
                To.setY(int(pCurve->y[i]/scaley  +m_ptoffset.y()));
                painter.drawLine(From, To);

                From = To;
            }
        }

        if(pCurve->pointsVisible())
        {
            for (i=0; i<pCurve->size();i++)
            {
                QPoint pt(int(pCurve->x[i]/m_scalex+m_ptoffset.x()), int(pCurve->y[i]/  scaley+m_ptoffset.y()));
                drawPoint(painter, pCurve->pointStyle(), pt, m_BkColor);
            }
        }

    }

    if(s_bHighlightPoint)
    {
        int point = pCurve->selected();
        if(point>=0)
        {
            //highlight
            QColor HighColor(200, 100, 77);
            CurvePen.setWidth((int)pCurve->width()+1);
            CurvePen.setColor(HighColor);
            painter.setPen(CurvePen);
            To.setX(int(pCurve->x[point]/m_scalex+m_ptoffset.x()));
            To.setY(int(pCurve->y[point]/scaley  +m_ptoffset.y()));
            painter.drawRect(To.x()-ptside,To.y()-ptside, 2*ptside,2*ptside);
        }
    }
    painter.restore();
}


void Graph::drawAxes(QPainter &painter)
{
    double xp, yp, scaley;
    QPen AxesPen;
    scaley = m_scaley;
    painter.save();

    AxesPen.setColor(m_AxisColor);
    AxesPen.setStyle(getStyle(m_AxisStyle));
    AxesPen.setWidth(m_AxisWidth);
    painter.setPen(AxesPen);

    //vertical axis
    if(xo>=xmin && xo<=xmax) xp = xo;
    else if(xo>xmax)         xp = xmax;
    else                     xp = xmin;

    painter.drawLine((int)(xp/m_scalex) + m_ptoffset.x(), (int)(ymin/scaley) + m_ptoffset.y(),
                     (int)(xp/m_scalex) + m_ptoffset.x(), (int)(ymax/scaley) + m_ptoffset.y());

    //horizontal axis
    if(yo>=ymin && yo<=ymax) yp = yo;
    else if(yo>ymax)         yp = ymax;
    else                     yp = ymin;


    painter.drawLine((int)(xmin/m_scalex) +m_ptoffset.x(), (int)(yp/scaley) + m_ptoffset.y(),
                     (int)(xmax/m_scalex) +m_ptoffset.x(), (int)( yp/scaley) + m_ptoffset.y());

    painter.restore();
}


void Graph::drawTitles(QPainter &painter)
{
    //draws the x & y axis name
    double scaley;
    int XPosXTitle, YPosXTitle, XPosYTitle, YPosYTitle;
    double xp, yp;

    scaley = m_scaley;
    painter.save();
    XPosXTitle = 5;
    YPosXTitle = -10;
    XPosYTitle = -5;
    YPosYTitle =  5;

    if(xo>=xmin && xo<=xmax) xp = xo;
    else if(xo>xmax)         xp = xmax;
    else					 xp = xmin;

    if(yo>=ymin && yo<=ymax) yp = yo;
    else if(yo>ymax)         yp = ymax;
    else                     yp = ymin;

    painter.setFont(m_TitleFont);

    QPen TitlePen(m_TitleColor);
    painter.setPen(TitlePen);

    painter.drawText(  (int)(xmax/m_scalex) + m_ptoffset.x() + XPosXTitle,
                       (int)(yp  /scaley)   + m_ptoffset.y() + YPosXTitle, m_XTitle);

    painter.drawText(  m_ptoffset.x() + (int)(xp/m_scalex)   + XPosYTitle,
                       m_rCltRect.top() + m_iMargin - YPosYTitle, m_YTitle);
    painter.restore();
}


void Graph::drawXTicks(QPainter &painter)
{
    double main, scaley, xt, yp;
    int exp, TickSize, height, yExpOff, nx;

    exp=0;

    if(fabs(xunit)<0.00000001) return;
    if(fabs(xmax-xmin)/xunit>30.0) return;

    scaley = m_scaley;
    painter.save();
    QString strLabel, strLabelExp;

    QFontMetrics fm(m_LabelFont);

    painter.setFont(m_LabelFont);


    TickSize = 5;
    height  = fm.height()/2;
    yExpOff = height/2;


    QPen LabelPen(m_AxisColor);

    LabelPen.setStyle(getStyle(m_AxisStyle));
    LabelPen.setWidth(m_AxisWidth);
    painter.setPen(LabelPen);
    xt = xo-(xo-xmin);//one tick at the origin
    nx = (int)((xo-xmin)/xunit);
    xt = xo - nx*xunit;

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
                painter.drawText((int)(xt/m_scalex) - fm.width(strLabel)/2 + m_ptoffset.x(),
                                 (int)(yp/scaley)   + TickSize*2 +height   + m_ptoffset.y(),
                                 strLabel);
            }
            else if(exp_x>=4 || exp_x<=-4)
            {
                main = xt;
                expFormat(main, exp);

                strLabel = QString("%1 10").arg(main,5,'f',1);
                painter.drawText(int(xt/m_scalex) - fm.width(strLabel)/2  +m_ptoffset.x(),
                                 int(yp/scaley)   + TickSize*2 +height    +m_ptoffset.y(),
                                 strLabel);
                strLabelExp = QString("%1").arg(exp);

                painter.drawText(int(xt/m_scalex) + fm.width(strLabel)/2       +m_ptoffset.x(),
                                 int(yp/scaley)   + TickSize*2 +height-yExpOff +m_ptoffset.y(),
                                 strLabelExp);
            }
            else
            {
                if(exp_x>0)         strLabel = QString("%1").arg(xt,0,'f',0);
                else if (exp_x>=-1) strLabel = QString("%1").arg(xt,6,'f',1);
                else if (exp_x>=-2) strLabel = QString("%1").arg(xt,6,'f',2);
                else if (exp_x>=-3) strLabel = QString("%1").arg(xt,6,'f',3);
                painter.drawText((int)(xt/m_scalex) - fm.width(strLabel)/2 + m_ptoffset.x(),
                                 (int)(yp/scaley)   + TickSize*2 +height   + m_ptoffset.y(),
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
    double scaley, xp, main, yt;
    int TickSize, fmheight, fmheight4, exp;
    if(fabs(xunit)<0.00000001) return;
    if(fabs(ymax-ymin)/yunit>30.0) return;
    scaley = m_scaley;
    painter.save();
    QString strLabel, strLabelExp;
    exp = 0;
    QFontMetrics fm(m_LabelFont);
    painter.setFont(m_LabelFont);

    fmheight  = fm.height();
    fmheight4 = (int)((double)fmheight/4.0);

    TickSize = 5;

    QPen LabelPen(m_AxisColor);
    LabelPen.setStyle(getStyle(m_AxisStyle));
    LabelPen.setWidth(m_AxisWidth);


    if(xo>=xmin && xo<=xmax) xp = xo;
    else if(xo>xmax)         xp = xmax;
    else                     xp = xmin;

    yt = yo-int((yo-ymin)*1.0001/yunit)*yunit;//one tick at the origin

    int iTick=0;

    while(fabs(yunit)>MININTERVAL && yt<=ymax*1.0001 && iTick<100)
    {
        //Draw ticks
        if(yt>=ymin)
        {
            painter.setPen(LabelPen);
            painter.drawLine((int)(xp/m_scalex)          + m_ptoffset.x(), (int)(yt/scaley) + m_ptoffset.y(),
                             (int)(xp/m_scalex)-TickSize + m_ptoffset.x(), (int)(yt/scaley) + m_ptoffset.y());

            painter.setPen(m_LabelColor);


            if(fabs(yt)<MININTERVAL)
            {
                strLabel = "0";
                painter.drawText((int)(xp/m_scalex) - fm.width(strLabel)-TickSize*2 +m_ptoffset.x(),
                                 (int)(yt/scaley)   + fmheight4 +m_ptoffset.y(),
                                 strLabel);
            }
            else if(abs(exp_y)>=4)
            {
                main = yt;
                expFormat(main, exp);

                strLabel    = QString("%1 10").arg(main,5,'f',1);
                strLabelExp = QString("%1").arg(exp);

                painter.drawText((int)(xp/m_scalex)   - fm.width(strLabel)-TickSize*3 + m_ptoffset.x(),
                                 (int)(yt/scaley) + fmheight4                     + m_ptoffset.y(),
                                 strLabel);

                if(exp_y>=4)
                {
                    painter.drawText(int(xp/m_scalex)   - TickSize*3 + m_ptoffset.x(),
                                     int(yt/scaley)              + m_ptoffset.y(),
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

                painter.drawText((int)(xp/m_scalex)   - fm.width(strLabel)-TickSize*2 +m_ptoffset.x(),
                                 (int)(yt/scaley) + fmheight4 +m_ptoffset.y(),
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

    QPen GridPen(m_XMajClr);

    GridPen.setStyle(getStyle(m_XMajStyle));
    GridPen.setWidth(m_XMajWidth);
    painter.setPen(GridPen);

    YMin = (int)(ymin/scaley) + m_ptoffset.y();
    YMax = (int)(ymax/scaley) + m_ptoffset.y();


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
    int width=1;
    if(m_YMajWidth<=1) width = 1;

    QPen GridPen(m_YMajClr);

    GridPen.setStyle(getStyle(m_YMajStyle));
    GridPen.setWidth(width);
    painter.setPen(GridPen);

    double yt = yo-int((yo-ymin)*1.0001/yunit)*yunit;//one tick at the origin

    int XMin = qMax((int)(xmin/m_scalex + m_ptoffset.x()), m_rCltRect.left());
    int XMax = qMin((int)(xmax/m_scalex + m_ptoffset.x()), m_rCltRect.right());

    while(yt<=ymax*1.0001)
    {
        if(yt>=ymin)
        {
            painter.drawLine(XMin, (int)(yt/scaley)   + m_ptoffset.y(), XMax, (int)(yt/scaley)   + m_ptoffset.y());
        }
        yt += yunit ;
    }
    painter.restore();
}

void Graph::drawXMinGrid(QPainter &painter)
{
    double scaley = m_scaley;
    if(fabs(xunit)<0.00000001) return;
    if(fabs(m_XMinorUnit)<0.00000001) return;
    if(fabs(xmax-xmin)/xunit>30.0) return;
    if(fabs(xmax-xmin)/m_XMinorUnit>100.0) return;
    int YMin, YMax;

    painter.save();
    QPen GridPen(m_XMinClr);
    GridPen.setStyle(getStyle(m_XMinStyle));
    GridPen.setWidth(m_XMinWidth);
    painter.setPen(GridPen);


    YMin = (int)(ymin/scaley)+ m_ptoffset.y();
    YMax = (int)(ymax/scaley)+ m_ptoffset.y();

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
    if(fabs(m_YMinorUnit)<0.00000001) return;
    if(fabs(ymax-ymin)/yunit>30.0) return;
    if(fabs(ymax-ymin)/m_YMinorUnit>100.0) return;

    painter.save();
    QPen GridPen(m_YMinClr);
    GridPen.setStyle(getStyle(m_YMinStyle));
    GridPen.setWidth(m_YMinWidth);
    painter.setPen(GridPen);

    double yDelta = m_YMinorUnit;
    double yt = yo-int((yo-ymin)*1.0001/yDelta)*yDelta;//one tick at the origin
    int XMin = qMax((int)(xmin/m_scalex + m_ptoffset.x()), m_rCltRect.left());
    int XMax = qMin((int)(xmax/m_scalex + m_ptoffset.x()), m_rCltRect.right());

    while(yt<=ymax*1.0001)
    {
        if(yt>=ymin)
        {
            painter.drawLine(XMin, (int)(yt/scaley)   + m_ptoffset.y(), XMax, (int)(yt/scaley)   + m_ptoffset.y());
        }
        yt += yDelta ;
    }
    painter.restore();
}


void Graph::drawLegend(QPainter &painter, QPoint &Place, QFont &LegendFont, QColor &LegendColor, QColor &backColor)
{
    painter.save();
    int LegendSize, ypos;
    QString strong;

    LegendSize = 30;
    //	ypos = 12;
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
                LegendPen.setStyle(getStyle(pCurve->style()));
                LegendPen.setWidth(pCurve->width());

                painter.setPen(LegendPen);

                painter.drawLine(Place.x(),                     Place.y() + ypos*npos + ypos/3,
                                 Place.x() + (int)(LegendSize), Place.y() + ypos*npos + ypos/3);
                if(pCurve->pointStyle())
                {
                    int x1 = Place.x() + 0.5*LegendSize;
                    int y1 = Place.y() + 1.*ypos*npos+ ypos/3;

                    drawPoint(painter, pCurve->pointStyle(), QPoint(x1, y1), m_BkColor);
                }

                painter.setPen(TextPen);
                painter.drawText(Place.x() + (int)(1.5*LegendSize),    Place.y()  + ypos*npos+(int)(ypos/2),
                                 strong);

                npos++;
            }
        }
    }

    painter.restore();

}

void Graph::expFormat(double &f, int &exp)
{

    if (f==0.0)
    {
        exp = 0;
        f = 0.0;
        return;
    }
    double f1 = fabs(f);
    if(f1<1)
        exp = (int)log10(f1)-1;
    else
        exp = (int)log10(f1);

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
                if(!bCSV) strong= QString("%1     %2  ").arg(pCurve->x[j],13,'g',7).arg(pCurve->y[j],13,'g',7);
                else      strong= QString("%1, %2, , ").arg(pCurve->x[j],13,'g',7).arg(pCurve->y[j],13,'g',7);
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



QPoint Graph::getOffset()
{
    return m_ptoffset;
}


void Graph::highlight(QPainter &painter, Curve *pCurve, int ref)
{
    if(!pCurve) return;
    if(ref<0 || ref>pCurve->size()-1) return;

    painter.save();
    int x = int(pCurve->x[ref]/m_scalex)  +m_ptoffset.x();
    int y = int(pCurve->y[ref]/m_scaley)  +m_ptoffset.y();

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
    int k,s,w;
    bool ba, bs;
    double f;

    settings.beginGroup(m_GraphName);
    {
        //read variables
        clr = axisColor();
        settings.setValue("AxisColor", clr);
        k = axisStyle();
        settings.setValue("AxisStyle", k);
        k = axisWidth();
        settings.setValue("AxisWidth", k);

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

        bXMajGrid(bs,clr,s,w);
        settings.setValue("XMajGridColor", clr);
        settings.setValue("XMajGridShow",bs);
        settings.setValue("XMajGridStyle",s);
        settings.setValue("XMajGridWidth",w);

        yMajGrid(bs,clr,s,w);
        settings.setValue("YMajGridColor", clr);
        settings.setValue("YMajGridShow",bs);
        settings.setValue("YMajGridStyle",s);
        settings.setValue("YMajGridWidth",w);

        bXMinGrid(bs,ba,clr,s,w,f);
        settings.setValue("XMinGridColor", clr);
        settings.setValue("XMinGridAuto",ba);
        settings.setValue("XMinGridShow",bs);
        settings.setValue("XMinGridStyle",s);
        settings.setValue("XMinGridWidth",w);
        settings.setValue("XMinGridUnit",f);

        bYMinGrid(bs,ba,clr,s,w,f);
        settings.setValue("YMinGridColor", clr);
        settings.setValue("YMinGridAuto",ba);
        settings.setValue("YMinGridShow",bs);
        settings.setValue("YMinGridStyle",s);
        settings.setValue("YMinGridWidth",w);
        settings.setValue("YMinGridUnit",f);

        clr = borderColor();
        s   = borderStyle();
        w   = borderWidth();
        settings.setValue("BorderColor", clr);
        settings.setValue("BorderStyle", s);
        settings.setValue("BorderWidth", w);
        settings.setValue("BorderShow", m_bBorder);

        clr = backgroundColor();
        settings.setValue("BackgroundColor", clr);

        settings.setValue("margin", m_iMargin);

        settings.setValue("Inverted", m_bYInverted);

        settings.setValue("XVariable", m_X);
        settings.setValue("YVariable", m_Y);
    }
    settings.endGroup();
}


void Graph::loadSettings(QSettings &settings)
{
    QFont lgft;
    bool bs, ba;
    int s,w;

    double f;
    QColor clr;

    settings.beginGroup(m_GraphName);
    {
        //read variables
        clr = settings.value("AxisColor", QColor(255,255,255)).value<QColor>();
        s = settings.value("AxisStyle",0).toInt();
        w = settings.value("AxisWidth",1).toInt();
        setAxisData(s,w,clr);

        clr = settings.value("TitleColor", QColor(255,255,255)).value<QColor>();
        setTitleColor(clr);
        clr = settings.value("LabelColor", QColor(255,255,255)).value<QColor>();
        setLabelColor(clr);

        lgft = QFont(settings.value("TitleFontName","Comic Sans MS").toString());
        int size = settings.value("TitleFontSize",8).toInt();
        if(size>0) lgft.setPointSize(size);
        lgft.setItalic(settings.value("TitleFontItalic", false).toBool());
        lgft.setBold(settings.value("TitleFontBold", false).toBool());
        setTitleFont(lgft);

        lgft = QFont(settings.value("LabelFontName","Comic Sans MS").toString());
        size = settings.value("LabelFontSize",8).toInt();
        if(size>0) lgft.setPointSize(size);
        lgft.setItalic(settings.value("LabelFontItalic", false).toBool());
        lgft.setBold(settings.value("LabelFontBold", false).toBool());
        setLabelFont(lgft);

        clr  = settings.value("XMajGridColor", QColor(90,90,90)).value<QColor>();
        bs = settings.value("XMajGridShow",true).toBool();
        s  = settings.value("XMajGridStyle",1).toInt();
        w  = settings.value("XMajGridWidth",1).toInt();
        setXMajGrid(bs,clr,s,w);

        clr  = settings.value("YMajGridColor", QColor(90,90,90)).value<QColor>();
        bs = settings.value("YMajGridShow",true).toBool();
        s  = settings.value("YMajGridStyle",1).toInt();
        w  = settings.value("YMajGridWidth",1).toInt();
        setYMajGrid(bs,clr,s,w);

        clr  = settings.value("XMinGridColor", QColor(90,90,90)).value<QColor>();
        ba = settings.value("XMinGridAuto",true).toBool();
        bs = settings.value("XMinGridShow",false).toBool();
        s  = settings.value("XMinGridStyle",2).toInt();
        w  = settings.value("XMinGridWidth",1).toInt();
        f  = settings.value("XMinGridUnit", 0.01).toDouble();
        setXMinGrid(bs,ba,clr,s,w,f);

        clr  = settings.value("YMinGridColor", QColor(90,90,90)).value<QColor>();
        ba = settings.value("YMinGridAuto",true).toBool();
        bs = settings.value("YMinGridShow",false).toBool();
        s  = settings.value("YMinGridStyle",2).toInt();
        w  = settings.value("YMinGridWidth",1).toInt();
        f  = settings.value("YMinGridUnit",0.01).toDouble();
        setYMinGrid(bs,ba,clr,s,w,f);

        clr  = settings.value("BorderColor", QColor(200,200,200)).value<QColor>();
        s  = settings.value("BorderStyle",0).toInt();
        w  = settings.value("BorderWidth",2).toInt();
        m_bBorder = settings.value("BorderShow", true).toBool();
        setBorderColor(clr);
        setBorderStyle(s);
        setBorderWidth(w);

        clr  = settings.value("BackgroundColor", QColor(15,19,20)).value<QColor>();
        setBkColor(clr);

        m_iMargin = settings.value("margin", 61).toInt();

        m_bYInverted = settings.value("Inverted", false).toBool();

        m_X  = settings.value("XVariable",0).toInt();
        m_Y  = settings.value("YVariable",0).toInt();
    }
    settings.endGroup();
}


void Graph::setLabelFont(QFont &font)
{
    m_LabelFont = font;
}


void Graph::setTitleFont(QFont &font)
{
    m_TitleFont = font;
}




void Graph::getTitleFont(QFont &titleFont)
{
    titleFont = m_TitleFont;
}
void Graph::getLabelFont(QFont &labelFont)
{
    labelFont = m_LabelFont;
}










Curve* Graph::addCurve()
{
    Curve *pCurve = new Curve();
    if(pCurve)
    {
        int nIndex = m_oaCurves.size();
        pCurve->setColor(s_CurveColors[nIndex%10]);
        pCurve->setStyle(0);
        pCurve->m_pParentGraph = this;
        m_oaCurves.append(pCurve);
    }
    return pCurve;
}

/**< In the case where a curve has been constructed independantly and needs to be added to the Graph */
Curve* Graph::addCurve(Curve *pCurve)
{
    if(pCurve)
    {
        int nIndex = m_oaCurves.size();
        pCurve->setColor(s_CurveColors[nIndex%10]);
        pCurve->setStyle(0);
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
    return ((double)x-(double)m_ptoffset.x())*m_scalex;
}

double Graph::clientToy(int y) const
{
    return ((double)y-(double)m_ptoffset.y())*m_scaley;
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

    m_AxisColor     = pGraph->m_AxisColor;
    m_BkColor       = pGraph->m_BkColor;
    m_bBorder       = pGraph->m_bBorder;
    m_BorderColor   = pGraph->m_BorderColor;
    m_BorderStyle   = pGraph->m_BorderStyle;
    m_BorderWidth   = pGraph->m_BorderWidth;
    m_LabelColor    = pGraph->m_LabelColor;
    m_TitleColor    = pGraph->m_TitleColor;
    m_AxisStyle     = pGraph->m_AxisStyle;
    m_AxisWidth     = pGraph->m_AxisWidth;
    m_XMajClr       = pGraph->m_XMajClr;
    m_XMajStyle     = pGraph->m_XMajStyle;
    m_XMajWidth     = pGraph->m_XMajWidth;
    m_XMinClr       = pGraph->m_XMinClr;
    m_XMinStyle     = pGraph->m_XMinStyle;
    m_XMinWidth     = pGraph->m_XMinWidth;
    m_YMajClr       = pGraph->m_YMajClr;
    m_YMajStyle     = pGraph->m_YMajStyle;
    m_YMajWidth     = pGraph->m_YMajWidth;
    m_YMinClr       = pGraph->m_YMinClr;
    m_YMinStyle     = pGraph->m_YMinStyle;
    m_YMinWidth     = pGraph->m_YMinWidth;

    m_bAutoX        = pGraph->m_bAutoX;
    m_bAutoY        = pGraph->m_bAutoY;
    m_bXAutoMinGrid = pGraph->m_bXAutoMinGrid;
    m_bYAutoMinGrid = pGraph->m_bYAutoMinGrid;
    m_bYInverted    = pGraph->m_bYInverted;
    m_bXMajGrid     = pGraph->m_bXMajGrid;
    m_bXMinGrid     = pGraph->m_bXMinGrid;
    m_bYMajGrid     = pGraph->m_bYMajGrid;
    m_bYMinGrid     = pGraph->m_bYMinGrid;
    m_bBorder       = pGraph->m_bBorder;
    m_iMargin       = pGraph->m_iMargin;
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



int Graph::axisStyle() const
{
    return m_AxisStyle;
}

int Graph::axisWidth() const
{
    return m_AxisWidth;
}

bool Graph::bAutoX() const
{
    return m_bAutoX;
}

bool Graph::bAutoY() const
{
    return m_bAutoY;
}


bool Graph::bAutoXMin() const
{
    return m_bXAutoMinGrid;
}

bool Graph::bAutoYMin() const
{
    return m_bYAutoMinGrid;
}


bool Graph::hasBorder() const
{
    return m_bBorder;
}


QRect * Graph::clientRect()
{
    return &m_rCltRect;
}


Curve* Graph::curve(int nIndex)
{
    if(m_oaCurves.size()>nIndex)
        return m_oaCurves[nIndex];
    else return nullptr;
}


Curve* Graph::curve(QString CurveTitle)
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


bool Graph::bInverted() const
{
    return m_bYInverted;
}



void Graph::graphName(QString &GraphName)
{
    GraphName = m_GraphName;
}


double Graph::xOrigin() const
{
    return xo;
}


bool Graph::bXMajGrid() const
{
    return m_bXMajGrid;
}


void Graph::bXMajGrid(bool &bstate, QColor &clr, int &style, int &width)
{
    bstate = m_bXMajGrid;
    clr   = m_XMajClr;
    style = m_XMajStyle;
    width = m_XMajWidth;
}


bool Graph::bXMinGrid() const
{
    return m_bXMinGrid;
}

void Graph::bXMinGrid(bool &state, bool &bAuto, QColor &clr, int &style, int &width, double &unit)
{
    state = m_bXMinGrid;
    bAuto = m_bXAutoMinGrid;
    clr   = m_XMinClr;
    style = m_XMinStyle;
    width = m_XMinWidth;
    unit  = m_XMinorUnit;
}

double Graph::xMin() const
{
    return xmin;
}

double Graph::xMax() const
{
    return xmax;
}


double Graph::xScale() const
{
    return m_scalex;
}


double Graph::xUnit() const
{
    return xunit;
}


int Graph::xVariable() const
{
    return m_X;
}



bool Graph::yMajGrid() const
{
    return m_bYMajGrid;
}

void Graph::yMajGrid(bool &state, QColor &clr, int &style, int &width)
{
    state = m_bYMajGrid;
    clr   = m_YMajClr;
    style = m_YMajStyle;
    width = m_YMajWidth;
}


bool Graph::bYMinGrid() const
{
    return m_bYMinGrid;
}


void Graph::bYMinGrid(bool &state, bool &bAuto, QColor &clr, int &style, int &width, double &unit)
{
    state = m_bYMinGrid;
    bAuto = m_bYAutoMinGrid;
    clr   = m_YMinClr;
    style = m_YMinStyle;
    width = m_YMinWidth;
    unit  = m_YMinorUnit;
}


double Graph::yOrigin() const
{
    return yo;
}

double Graph::yMin() const
{
    return ymin;
}

double Graph::yMax() const
{
    return ymax;
}

double Graph::yUnit() const
{
    return yunit;
}


double Graph::yScale() const
{
    return m_scaley;
}


int Graph::yVariable() const
{
    return m_Y;
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

    if(m_bXAutoMinGrid) m_XMinorUnit = xunit/5.0;
    if(m_bYAutoMinGrid) m_YMinorUnit = yunit/5.0;

    return true;
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

//___________________Start Sets______________________________________________________________


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
void Graph::setAutoXMinUnit(bool bAuto)
{
    m_bXAutoMinGrid = bAuto;
    if(bAuto) m_XMinorUnit = xunit/5.0;
}


void Graph::setAutoXUnit()
{
    //	xunit = 100.0*m_scalex;
    xunit = (xmax-xmin)/3.0;

    if (xunit<1.0)
    {
        exp_x = (int)log10(xunit*1.00001)-1;
        exp_x = qMax(-4, exp_x);
    }
    else exp_x = (int)log10(xunit*1.00001);
    int main_x = (int)(xunit/pow(10.0, exp_x)*1.000001);


    if(main_x<2)
        xunit = pow(10.0,exp_x);
    else if (main_x<5)
        xunit = 2.0*pow(10.0,exp_x);
    else
        xunit = 5.0*pow(10.0,exp_x);

}


void Graph::setAutoYMinUnit(bool bAuto)
{
    m_bYAutoMinGrid = bAuto;
    if(bAuto) m_YMinorUnit = yunit/5.0;
}


void Graph::setAutoYUnit()
{
    //	yunit = 100.0 * m_scaley;
    yunit = (ymax-ymin)/5.0;
    if (yunit<1.0)
    {
        exp_y = (int)log10(yunit*1.00001)-1;
        //		exp_y = qMax(-4, exp_y);
    }
    else  exp_y = (int)log10(yunit*1.00001);

    int main_y = (int)(yunit/pow(10.0, exp_y));

    if(main_y<2)
        yunit = pow(10.0,exp_y);
    else if (main_y<5)
        yunit = 2.0*pow(10.0,exp_y);
    else
        yunit = 5.0*pow(10.0,exp_y);
}

void Graph::setAxisData(int s, int w, QColor clr)
{
    m_AxisStyle = s;
    m_AxisWidth = w;
    m_AxisColor = clr;
}

void Graph::setAxisColor(QColor crColor)
{
    m_AxisColor = crColor;
}

void Graph::setAxisStyle(int nStyle)
{
    m_AxisStyle = nStyle;
}

void Graph::setAxisWidth(int Width)
{
    m_AxisWidth = Width;
}


void Graph::setBkColor(QColor cr)
{
    m_BkColor = cr;
}

void Graph::setBorderColor(QColor crBorder)
{
    m_BorderColor = crBorder;
}

void Graph::setBorder(bool bBorder)
{
    m_bBorder = bBorder;
}

void Graph::setBorderWidth(int w)
{
    m_BorderWidth = w;
}

void Graph::setBorderStyle(int s)
{
    m_BorderStyle = s;
}

void Graph::setDrawRect(QRect Rect)
{
    m_rCltRect = Rect;
}

void Graph::setGraphName(QString GraphName)
{
    m_GraphName = GraphName;
}


void Graph::setGraphDefaults(bool bDark)
{
    if(bDark)
    {
        m_BkColor = QColor(0,9,13);
        m_BorderColor = QColor(200,200,200);

        setAxisColor(QColor(200,200,200));
        setTitleColor(QColor(255,255,255));
        setLabelColor(QColor(255,255,255));

        m_XMajClr   = QColor(90,90,90);
        m_YMajClr   = QColor(90,90,90);

        m_XMinClr   = QColor(50,50,50);
        m_YMinClr   = QColor(50,50,50);
    }
    else
    {
        m_BkColor = QColor(255,255,255);
        m_BorderColor = QColor(55,55,55);

        setAxisColor(QColor(55,55,55));
        setTitleColor(QColor(0,0,0));
        setLabelColor(QColor(0,0,0));

        m_XMajClr   = QColor(165,165,165);
        m_YMajClr   = QColor(165,165,165);

        m_XMinClr   = QColor(205,205,205);
        m_YMinClr   = QColor(205,205,205);
    }

    m_BorderStyle = 0;
    m_BorderWidth = 3;

    m_AxisStyle = 0;
    m_AxisWidth = 1;

    m_bYInverted = false;

    m_bXMajGrid = true;
    m_bYMajGrid = true;
    m_bXMinGrid = false;
    m_bYMinGrid = false;

    m_XMajStyle = 1;
    m_YMajStyle = 1;
    m_XMajWidth = 1;
    m_YMajWidth = 1;

    m_XMinStyle = 1;
    m_YMinStyle = 1;
    m_XMinWidth = 1;
    m_YMinWidth = 1;

    m_XMinorUnit = 0.1;
    m_YMinorUnit = 0.1;
}




void Graph::setLabelColor(QColor crColor)
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

void Graph::setX0(double f){
    xo = f;
}



void Graph::setXMajGrid(bool const &state, QColor const &clr, int const &style, int const &width)
{
    m_bXMajGrid = state;
    m_XMajClr   = clr;
    m_XMajStyle = style;
    m_XMajWidth = width;
}


void Graph::setXMajGrid(bool const &bGrid)
{
    m_bXMajGrid = bGrid;
}

void Graph::setXMinGrid(bool const &bGrid)
{
    m_bXMinGrid = bGrid;
}



void Graph::setXMax(double f)
{
    xmax = f;
}


void Graph::setXMin(double f)
{
    xmin = f;
}

void Graph::setXMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit)
{
    m_bXMinGrid = state;
    m_bXAutoMinGrid = bAuto;
    m_XMinClr   = clr;
    m_XMinStyle = style;
    m_XMinWidth = width;
    if(unit>0.0) m_XMinorUnit  = unit;
}



void Graph::setXMinorUnit(double f)
{
    m_XMinorUnit = f;
}


bool Graph::setXScale()
{
    Curve *pCurve;
    int nc;

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
                    xmin = std::min(xmin, pCurve->x[0]);
                    xmax = std::max(xmax, pCurve->x[0]);
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

        //		m_scalex   =  (xmax-xmin)/m_w;
        if (xunit<1.0)
        {
            exp_x = (int)log10(xunit*1.00001)-1;
            exp_x = qMax(-4, exp_x);
        }
        else exp_x = (int)log10(xunit*1.00001);

    }
    m_scalex   =  (xmax-xmin)/m_w;

    //graph center position
    int Xg = (m_rCltRect.right() + m_rCltRect.left())/2;
    // curves center position
    int Xc = (int)((xmin+xmax)/2.0/m_scalex);
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

void Graph::setYMinorUnit(double f)
{
    m_YMinorUnit = f;
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
                    ymin = std::min(ymin, pCurve->y[0]);
                    ymax = std::max(ymax, pCurve->y[0]);
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
            exp_y = (int)log10(yunit*1.00001)-1;
            exp_y = qMax(-4, exp_y);
        }
        else  exp_y = (int)log10(yunit*1.00001);

    }

    //graph center position
    int Yg = (m_rCltRect.top() + m_rCltRect.bottom())/2;
    // curves center position
    int Yc = (int)((ymin+ymax)/2.0/m_scaley);
    // center graph in drawing rectangle
    m_ptoffset.ry() = (Yg-Yc);

    return true;
}

void Graph::setYMajGrid(bool const &state, QColor const &clr, int const &style, int const &width)
{
    m_bYMajGrid = state;
    m_YMajClr   = clr;
    m_YMajStyle = style;
    m_YMajWidth = width;
}


void Graph::setYMajGrid(bool const &bGrid)
{
    m_bYMajGrid = bGrid;
}

void Graph::setYMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit)
{
    m_bYMinGrid = state;
    m_bYAutoMinGrid = bAuto;
    m_YMinClr   = clr;
    m_YMinStyle = style;
    m_YMinWidth = width;
    if(unit>0.0) m_YMinorUnit  = unit;
}


void Graph::setYMinGrid(bool const &bGrid)
{
    m_bYMinGrid = bGrid;
}



void Graph::setYVariable(int const & Y)
{
    m_Y = Y;
}


int Graph::xToClient(double x) const
{
    return (int)(x/m_scalex + m_ptoffset.x());
}



int Graph::yToClient(double y) const
{
    return (int)(y/m_scaley + m_ptoffset.y());
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
        //		pCurve->SetSelected(-1);
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
    //	pCurve->SetSelected(-1);
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







