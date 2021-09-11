/****************************************************************************

    LegendWt Class
        Copyright (C) 2015 André Deperrois 

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

#include <QPainter>

#include "legendwt.h"

#include <xflgraph/containers/graphtilewt.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <globals/mainframe.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects3d/planeopp.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects2d/oppoint.h>

#include <miarex/miarex.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xinverse/xinverse.h>
#include <xdirect/xdirect.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflgraph/curve.h>


MainFrame* LegendWt::s_pMainFrame = nullptr;
Miarex* LegendWt::s_pMiarex = nullptr;
XDirect* LegendWt::s_pXDirect = nullptr;



LegendWt::LegendWt(QWidget *pParent) : QWidget(pParent)
{
    setMouseTracking(true);
    m_pGraphTileWt = dynamic_cast<GraphTileWidget*>(pParent);
    m_pGraph = nullptr;
    m_MiarexView = xfl::OTHERVIEW;
    m_LegendPosition = QPointF(11.0,11.0);
    m_bTrans = false;
}


LegendWt::~LegendWt()
{
}

void LegendWt::setMiarexView(xfl::enumMiarexViews eMiarexView)
{
    m_MiarexView = eMiarexView;
}



void LegendWt::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), DisplayOptions::backgroundColor());


    int bottom = rect().height();

    if(m_pGraphTileWt)
    {
        if(m_pGraphTileWt->xflr5App()==xfl::MIAREX)
        {
            switch(m_MiarexView)
            {
                case xfl::WOPPVIEW:
                    drawPOppGraphLegend(painter, m_LegendPosition, bottom);
                    break;
                case xfl::WPOLARVIEW:
                    drawWPolarLegend(painter, m_LegendPosition, bottom);
                    break;
                case xfl::STABPOLARVIEW:
                    drawWPolarLegend(painter, m_LegendPosition, bottom);
                    break;
                case xfl::STABTIMEVIEW:
                    if(m_pGraph) drawStabTimeLegend(painter, m_pGraph, m_LegendPosition, bottom);
                    break;
                case xfl::WCPVIEW:
                    if(m_pGraph) drawCpLegend(painter, m_pGraph, m_LegendPosition, bottom);
                    break;
                default: break;
            }
        }
        else //XFLR5::XFOILANALYSIS
        {
            drawPolarLegend(painter, m_LegendPosition, bottom);
        }
    }
}



QSize LegendWt::sizeHint() const
{
    return QSize(150,150);
}




/**
* Draws the legend of the polar graphs
*@param painter the instance of the QPainter object associated to the active view
*@param the top left postition where the legend is to be drawn
*@param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWt::drawWPolarLegend(QPainter &painter, QPointF place, int bottom)
{
    painter.save();

    int ny=0, x1=0, y1=0;

    double LegendSize = 30;
    double LegendWidth = 280;

    QFontMetrics fm(DisplayOptions::textFont());
    double ypos = fm.height();

    painter.setFont(DisplayOptions::textFont());


    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);
    TextPen.setWidth(1);

    QStringList strPlaneList; // we need to make an inventory of planes which have a visible polar of the desired type
    WPolar * pWPolar=nullptr;
    Plane *pPlane=nullptr;

    for (int j=0; j<Objects3d::planeCount(); j++)
    {
        pPlane = Objects3d::planeAt(j);
        for (int i=0; i<Objects3d::polarCount(); i++)
        {
            pWPolar = Objects3d::polarAt(i);
            if (pWPolar->planeName()==pPlane->name() && pWPolar->isVisible() && !isFiltered(pWPolar))
            {
                if(m_MiarexView==xfl::WPOLARVIEW || (m_MiarexView==xfl::STABPOLARVIEW && pWPolar->isStabilityPolar()))
                {
                    strPlaneList.append(pPlane->name());
                    break;
                }
            }
        }// finished inventory
    }

#if QT_VERSION >= 0x050000
    strPlaneList.sort(Qt::CaseInsensitive);
#else
    strPlaneList.sort();
#endif

//    painter.setBackgroundMode(Qt::TransparentMode);
    QBrush LegendBrush(DisplayOptions::backgroundColor());
    painter.setBrush(LegendBrush);

    QPen LegendPen;
    LegendPen.setWidth(1);

    ny =0;
    for (int k=0; k<strPlaneList.size(); k++)
    {
        int nPlanePlrs = 0;
        for (int l=0; l < Objects3d::polarCount(); l++)
        {
            pWPolar = Objects3d::polarAt(l);

            if (pWPolar->dataSize() && pWPolar->isVisible() && !isFiltered(pWPolar) && pWPolar->planeName()==strPlaneList.at(k))
            {
                if(m_MiarexView==xfl::WPOLARVIEW || (m_MiarexView==xfl::STABPOLARVIEW && pWPolar->isStabilityPolar()))
                    nPlanePlrs++;
            }
        }

        if (nPlanePlrs)
        {
            double YPos = place.y() + (ny+nPlanePlrs+2) * ypos;// bottom line of this Plane's legend
            if(abs(bottom) > abs(YPos))
            {
                ny++;
                painter.drawText(int(place.x()), int(place.y() + ypos*ny- ypos/2), strPlaneList.at(k));
            }
            else
            {
                // move rigth if outside screen
                place.rx() += LegendWidth;
                ny=1;
                painter.setPen(TextPen);
                painter.drawText(int(place.x()), int(place.y() + ypos*ny-ypos/2), strPlaneList.at(k));
            }

            for (int nc=0; nc<Objects3d::polarCount(); nc++)
            {
                pWPolar = Objects3d::polarAt(nc);
                if(strPlaneList.at(k) == pWPolar->planeName())
                {
                    if(!pWPolar->dataSize())
                    {
                    }
                    else if(!pWPolar->isVisible() || isFiltered(pWPolar))
                    {
                    }
                    else if(m_MiarexView!=xfl::WPOLARVIEW && (m_MiarexView!=xfl::STABPOLARVIEW || !pWPolar->isStabilityPolar()))
                    {
                    }
                    else
                    {
                        LegendPen.setColor(pWPolar->color());
                        LegendPen.setStyle(xfl::getStyle(pWPolar->lineStipple()));
                        LegendPen.setWidth(pWPolar->lineWidth());
                        painter.setPen(LegendPen);

                        painter.drawLine(int(place.x() + 0.5*LegendSize), int(place.y() + 1.*ypos*ny),
                                         int(place.x() + 1.5*LegendSize), int(place.y() + 1.*ypos*ny));

                        if(pWPolar->pointStyle()!=Line::NOSYMBOL)
                        {
                            x1 = place.x() + 1.0*LegendSize;
                            y1 = place.y() + 1.*ypos*ny;

                            xfl::drawSymbol(painter, pWPolar->pointStyle(), DisplayOptions::backgroundColor(), pWPolar->color(), x1, y1);
                        }

                        painter.setPen(TextPen);
                        painter.drawText(int(place.x() + 2.0*LegendSize),
                                         int(place.y() + 1.*ypos*ny+ypos/3), pWPolar->polarName());
                        ny++ ;
                    }
                }
            }
            if(nPlanePlrs) ny++;
        }
    }
    painter.restore();
}



/**
* Draws the curve legend for the graphs in the operating point view
* @param painter the instance of the QPainter object associated to the active view
* @param the top left postition where the legend is to be drawn
* @param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWt::drawPOppGraphLegend(QPainter &painter, QPointF place, double bottom)
{

    double LegendSize(30.0), LegendWidth(300.0);
    double ypos(0);
    int nc(0), ny(0);
    double x1(0), y1(0);

    QStringList str; // we need to make an inventory of wings
    bool bFound(false);
    PlaneOpp *pPOpp(nullptr);

    for (int i=0; i<Objects3d::planeOppCount(); i++)
    {
        bFound = false;
        pPOpp = Objects3d::planeOppAt(i);
        for (int j=0; j<str.size(); j++)
        {
            if (pPOpp->planeName() == str.at(j))    bFound = true;
        }
        if (!bFound)
        {
            str.append(pPOpp->planeName());
        }
    }

    painter.save();
    painter.setFont(DisplayOptions::textFont());

    QFontMetrics fm(DisplayOptions::textFont());
    ypos = fm.height();

    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);
    TextPen.setWidth(1);

//    painter.setBackgroundMode(Qt::TransparentMode);
    QBrush LegendBrush(DisplayOptions::backgroundColor());
    painter.setBrush(LegendBrush);

    QPen LegendPen;
    LegendPen.setWidth(1);

    if(s_pMiarex->curPOppOnly())
    {
        if(!s_pMiarex->curPOpp() || !s_pMiarex->curPOpp()->isVisible())
        {
            painter.restore();
            return;
        }
        ny++ ;

        painter.setPen(TextPen);
        painter.drawText(int(place.x() + 1.0*LegendSize), int(place.y() + ypos*ny-ypos/2.0), s_pMiarex->curPOpp()->planeName());

        LegendPen.setColor(s_pMiarex->curPOpp()->color());
        LegendPen.setStyle(xfl::getStyle(s_pMiarex->curPOpp()->lineStipple()));
        LegendPen.setWidth(s_pMiarex->curPOpp()->width());
        painter.setPen(LegendPen);

        painter.drawLine(int(place.x() + 1.5*LegendSize), int(place.y() + 1.*ypos*ny),
                         int(place.x() + 2.5*LegendSize), int(place.y() + 1.*ypos*ny));

        if(s_pMiarex->curPOpp()->pointStyle()!=Line::NOSYMBOL)
        {
            x1 = place.x() + 2.0*LegendSize;
            y1 = place.y() + 1.*ypos*ny;

            xfl::drawSymbol(painter, s_pMiarex->curPOpp()->pointStyle(), DisplayOptions::backgroundColor(), s_pMiarex->curPOpp()->color(), x1, y1);
        }

        painter.setPen(TextPen);
        painter.drawText(int(place.x() + 3*LegendSize),
                         int(place.y() + 1.*ypos*ny+ypos/3),
                         s_pMiarex->POppTitle(s_pMiarex->m_pCurPOpp));
    }
    else
    {
        bool bStarted = false;
        for (int k = 0; k<str.size(); k++)
        {
            int PlanePts = 0;
            for (int l=0; l<Objects3d::planeOppCount(); l++)
            {
                pPOpp = Objects3d::planeOppAt(l);
                if (pPOpp->isVisible() && pPOpp->planeName() == str.at(k)) PlanePts++;
            }
            if (PlanePts)
            {
                double YPos = place.y() + (ny+PlanePts+2) * ypos;// bottom line of this foil's legend

                painter.setPen(TextPen);
                if (!bStarted || (fabs(bottom) > fabs(YPos)))
                {
                    ny++;
                    painter.drawText(int(place.x() + 1.0*LegendSize),
                                     int(place.y() + ypos*ny-ypos/2.0), str.at(k));
                }
                else
                {
                    // move right if outside screen
                    place.rx() += LegendWidth;
                    ny=1;
                    painter.drawText(int(place.x() + 1.0*LegendSize),
                                     int(place.y() + ypos*ny-ypos/2), str.at(k));
                }

                bStarted = true;
                for (nc=0; nc < Objects3d::planeOppCount(); nc++)
                {
                    pPOpp = Objects3d::planeOppAt(nc);
                    if(str.at(k) == pPOpp->planeName() && pPOpp->isVisible())
                    {
                        if(qAbs(bottom)<fabs(place.y() + 1.*ypos*ny+ypos))
                        {
                            //move right
                            place.rx() += LegendWidth;
                            ny=2;
                        }

                        LegendPen.setColor(pPOpp->color());
                        LegendPen.setStyle(xfl::getStyle(pPOpp->lineStipple()));
                        LegendPen.setWidth(pPOpp->width());
                        painter.setPen(LegendPen);

                        painter.drawLine(int(place.x() + 1.5*LegendSize), int(place.y() + 1.*ypos*ny),
                                         int(place.x() + 2.5*LegendSize), int(place.y() + 1.*ypos*ny));

                        if(pPOpp->pointStyle()!=Line::NOSYMBOL)
                        {
                            x1 = place.x() + 2.0*LegendSize;
                            y1 = place.y() + 1.*ypos*ny;
//                            painter.drawRect(x1-2, place.y() + 1.*ypos*ny-2, 4, 4);
                            xfl::drawSymbol(painter, pPOpp->pointStyle(), DisplayOptions::backgroundColor(), pPOpp->color(), x1, y1);
                        }

                        painter.setPen(TextPen);
                        painter.drawText(int(place.x() + 3.0*LegendSize),
                                         int(place.y() + 1.0*ypos*ny + ypos/3),
                                         s_pMiarex->POppTitle(pPOpp));
                        ny++ ;
                    }
                }
                if (PlanePts) ny++;
            }
        }
    }
    painter.restore();
}


/**
* Draws the legend of the Cp graph
*@param painter the instance of the QPainter object associated to the active view
*@param the top left postition where the legend is to be drawn
*@param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWt::drawCpLegend(QPainter &painter, Graph const *pGraph, QPointF place, int bottom)
{
    painter.save();

    int i, ny;
    Curve const *pCurve=nullptr;
    QString strong;

    double LegendSize = 30;
    double LegendWidth = 350;
    double dny = 14;
    bottom -= 15;//margin

    QPen CurvePen;
    QPen TextPen(DisplayOptions::textColor());

    ny=0;

    for (i=0; i<pGraph->curveCount(); i++)
    {
        pCurve = pGraph->curveAt(i);
        if(pCurve->size())
        {
            ny++;

            if(qAbs(bottom)<fabs(place.y() + dny*(ny+1)))
            {
                //move right
                place.rx() += LegendWidth;
                ny=0;
            }

            CurvePen.setColor(pCurve->color());
            CurvePen.setStyle(xfl::getStyle(pCurve->stipple()));
            CurvePen.setWidth(pCurve->width());
            painter.setPen(CurvePen);

            painter.drawLine(int(place.x() + 0.5*LegendSize), int(place.y() + 1.*dny*ny),
                             int(place.x() + 1.5*LegendSize), int(place.y() + 1.*dny*ny));

            if(pCurve->pointsVisible())
            {
                int x1 = int(place.x() + 2.0*LegendSize);
                int y1 = int(place.y() + 1.*dny*ny);

                xfl::drawSymbol(painter, pCurve->pointStyle(), DisplayOptions::backgroundColor(), pCurve->color(), x1, y1);
            }

            pCurve->curveName(strong);
            painter.setPen(TextPen);
            painter.drawText(int(place.x() + 2.0*LegendSize), int(place.y() + 1.*dny*ny), strong);
        }
    }
    painter.restore();
}



/**
* Draws the legend for the time response graph- 4 curves
*@param painter the instance of the QPainter object associated to the active view
*@param the top left postition where the legend is to be drawn
*@param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWt::drawStabTimeLegend(QPainter &painter, Graph const *pGraph, QPointF place, int bottom)
{
    painter.save();

    Curve const *pCurve=nullptr;
    QString strong;

    double LegendSize = 30;
    double LegendWidth = 350;
    double dny = 14;
    bottom -= 15;//margin

    QPen CurvePen;
    QPen TextPen(DisplayOptions::textColor());

    int ny=0;

    for (int i=0; i<pGraph->curveCount(); i++)
    {
        pCurve = pGraph->curveAt(i);
        if(pCurve->size() && pCurve->isVisible())
        {
            ny++;

            if(qAbs(bottom)<fabs(place.y() + dny*(ny+1)))
            {
                //move right
                place.rx() += LegendWidth;
                ny=0;
            }

            CurvePen.setColor(pCurve->color());
            CurvePen.setStyle(xfl::getStyle(pCurve->stipple()));
            CurvePen.setWidth(pCurve->width());
            painter.setPen(CurvePen);

            painter.drawLine(int(place.x() + 1.5*LegendSize), int(place.y() + 1.*dny*ny),
                             int(place.x() + 2.5*LegendSize), int(place.y() + 1.*dny*ny));

            if(pCurve->pointsVisible())
            {
                double x1 = place.x() + 2.0*LegendSize;
                double y1 = place.y() + 1.*dny*ny;
                xfl::drawSymbol(painter, pCurve->pointStyle(), DisplayOptions::backgroundColor(), pCurve->color(), x1, y1);
            }

            pCurve->curveName(strong);
            painter.setPen(TextPen);
            painter.drawText(int(place.x() + 3*LegendSize),
                             int(place.y() + 1.*dny*ny+dny/3), strong);
        }
    }
    painter.restore();
}




/**
 * Paints the legend of the polar graphs
 * @param place the top-left point where the legend will be placed
 * @param bottom the number of pixels to the bottom of the client area
 * @param painter a reference to the QPainter object with which to draw
 */
void LegendWt::drawPolarLegend(QPainter &painter, QPointF place, int bottom)
{
    double LegendSize = 30;
    double LegendWidth = 240;
    painter.save();
    painter.setFont(DisplayOptions::textFont());

    QFont fnt(DisplayOptions::textFont()); //two step to shut valgrind up
    QFontMetrics fm(fnt);
    double legendHeight = fm.height()+1;

    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);
    TextPen.setWidth(1);

    QStringList str; // we need to make an inventory of foils
    Polar * pPolar=nullptr;
    Foil *pFoil=nullptr;
    for (int j=0; j<Objects2d::foilCount(); j++)
    {
        pFoil = Objects2d::foilAt(j);
        for (int i=0; i<Objects2d::polarCount(); i++)
        {
            pPolar = Objects2d::polarAt(i);
            if (pPolar->foilName() == pFoil->name() && pPolar->m_Alpha.size() && pPolar->isVisible())
            {
                str.append(pFoil->name());
                break;
            }
        }// finished inventory
    }

    int nFoils= str.size();

    //    painter.setBackgroundMode(Qt::TransparentMode);
    QBrush LegendBrush(DisplayOptions::backgroundColor());
    painter.setBrush(LegendBrush);

    QPen LegendPen;
    LegendPen.setWidth(1);

    int ny =0;
    for (int k=0; k<nFoils; k++)
    {
        int FoilPlrs = 0;
        for (int l=0; l<Objects2d::polarCount(); l++)
        {
            pPolar = Objects2d::polarAt(l);
            if (pPolar->m_Alpha.size() &&
                    pPolar->polarName().length() &&
                    pPolar->isVisible() &&
                    pPolar->foilName() == str.at(k)) FoilPlrs++;
        }
        if (FoilPlrs)
        {
            int YBotPos = int(place.y() + (ny+FoilPlrs+2) * legendHeight);// bottom line of this foil's legend
            if(abs(bottom) > abs(YBotPos))
            {
                ny++;
            }
            else if (k>0)
            {
                // move rigth if less than client bottom area
                place.rx() += LegendWidth;
                ny=1;
            }
            else
            {
                ny=1;
            }
            painter.setPen(TextPen);
            painter.drawText(int(place.x() + 0.5*LegendSize), int(place.y() + legendHeight*ny-legendHeight/2),
                             str.at(k));
        }
        for (int nc=0; nc<Objects2d::polarCount(); nc++)
        {
            pPolar = Objects2d::polarAt(nc);
            if(str.at(k) == pPolar->foilName())
            {
                if (pPolar->m_Alpha.size() && pPolar->polarName().length() && pPolar->isVisible())
                {
                    //is there anything to draw ?
                    LegendPen.setColor(pPolar->color());
                    LegendPen.setStyle(xfl::getStyle(pPolar->polarStyle()));
                    LegendPen.setWidth(pPolar->lineWidth());
                    painter.setPen(LegendPen);

                    painter.drawLine(int(place.x() + 1.0*LegendSize), int(place.y() + 1.*legendHeight*ny),
                                     int(place.x() + 2.0*LegendSize), int(place.y() + 1.*legendHeight*ny));
                    if(pPolar->pointStyle())
                    {
                        double x1 = place.x() + 1.5*LegendSize;
                        double y1 = place.y() + 1.*legendHeight*ny;

                        xfl::drawSymbol(painter, pPolar->pointStyle(), DisplayOptions::backgroundColor(), pPolar->color(), x1, y1);
                    }

                    painter.setPen(TextPen);
                    painter.drawText(int(place.x() + 2.5*LegendSize), int(place.y() + 1.*legendHeight*ny+legendHeight/3),
                                     pPolar->polarName());
                    ny++ ;
                }
            }
        }
        if (FoilPlrs) ny++;
    }
    //    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.restore();
}


void LegendWt::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_R:
        {
            m_LegendPosition = QPointF(11.0,11.0);
            update();
            event->accept();
            break;
        }
        default:event->ignore();
    }
}



void LegendWt::mouseMoveEvent(QMouseEvent *event)
{
    setFocus();
    if(m_bTrans)
    {
        m_LegendPosition.rx() += event->pos().x()-m_PointDown.x();
        m_LegendPosition.ry() += event->pos().y()-m_PointDown.y();
        m_PointDown = event->pos();
        update();
    }
}


void LegendWt::mousePressEvent(QMouseEvent *event)
{
    m_PointDown.setX(event->pos().x());
    m_PointDown.setY(event->pos().y());
    m_bTrans = true;
    setCursor(Qt::ClosedHandCursor);
}



void LegendWt::mouseReleaseEvent(QMouseEvent *)
{
    m_bTrans = false;
    setCursor(Qt::CrossCursor);
}


bool LegendWt::isFiltered(WPolar *pWPolar)
{
    if(!pWPolar) return true;
    if(pWPolar->isFixedSpeedPolar() && !s_pMiarex->m_bType1) return true;
    if(pWPolar->isFixedLiftPolar()  && !s_pMiarex->m_bType2) return true;
    if(pWPolar->isFixedaoaPolar()   && !s_pMiarex->m_bType4) return true;
    if(pWPolar->isStabilityPolar()  && !s_pMiarex->m_bType7) return true;
    return false;
}









