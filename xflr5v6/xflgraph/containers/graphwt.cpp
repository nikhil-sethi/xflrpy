/****************************************************************************

    GraphWt Class
        Copyright (C) André Deperrois

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


#include <globals/mainframe.h>

#include "graphwt.h"
#include "graphtilewt.h"
#include <xflgraph/controls/graphdlg.h>
#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects3d/wpolar.h>
#include <miarex/miarex.h>
#include <xflcore/units.h>
#include <QPen>
#include <QPainterPath>
#include <QPainter>
#include <QPaintEvent>


FontStruct GraphWt::s_TextFontStruct;
QColor GraphWt::s_BackgroundColor = Qt::black;
QColor GraphWt::s_TextColor = Qt::white;

GraphWt::GraphWt(QWidget *pParent) : QWidget(pParent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    m_DefaultSize = QSize(700,500);
    m_MinSize = QSize(30, 30);

    m_bOverlayRectangle = false;

    m_TitlePosition.setX(0);
    m_TitlePosition.setY(0);
    m_bDrawLegend = false;
    m_GraphTitle = "";

    m_bTransGraph = false;
    m_bXPressed = m_bYPressed = false;

    m_pGraph = nullptr;
    setLegendPosition(QPoint(20,20));
}


void GraphWt::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    if(m_pGraph) m_pGraph->setDrawRect(rect());
}


void GraphWt::setTitles(QString &Title, QPoint &Place)
{
    m_GraphTitle = Title;
    m_TitlePosition = Place;
}


void GraphWt::paintEvent(QPaintEvent *pEvent)
{
    QPainter painter(this);
    painter.save();

    QBrush BackBrush(m_pGraph->backgroundColor());
    painter.fillRect(pEvent->rect(), BackBrush);
    if(!m_pGraph)
    {
        painter.restore();
        return;
    }

    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(BackBrush);

    m_pGraph->drawGraph(painter);

    if(m_bOverlayRectangle)
    {
        painter.save();
        QBrush overlaybrush(QColor(0,175,225,105));
        painter.setBrush(overlaybrush);
        QPointF topleft(m_pGraph->toClient(m_TopLeft.x(), m_TopLeft.y()));
        QPointF botright(m_pGraph->toClient(m_BotRight.x(), m_BotRight.y()));
        QRectF rect(topleft, botright);
        painter.drawRect(rect);
        painter.restore();
    }

    if(m_bDrawLegend) m_pGraph->drawLegend(painter, m_LegendOrigin, DisplayOptions::textFont(), DisplayOptions::textColor(), DisplayOptions::backgroundColor());

    if(hasFocus() && DisplayOptions::bMousePos())
    {
        QPen textPen(DisplayOptions::textColor());
        QFontMetrics fm(DisplayOptions::textFont());
        painter.setBackgroundMode(Qt::TransparentMode);

        int fmheight  = fm.height();

        painter.setFont(DisplayOptions::textFont());
        painter.setPen(textPen);
        painter.drawText(width()-14*fm.averageCharWidth(),  fmheight, QString("x = %1").arg(m_pGraph->clientTox(m_LastPoint.x()),9,'f',5));
        painter.drawText(width()-14*fm.averageCharWidth(),2*fmheight, QString("y = %1").arg(m_pGraph->clientToy(m_LastPoint.y()),9,'f',5));
    }
    painter.restore();
}



void GraphWt::resizeEvent (QResizeEvent * pEvent )
{
    QRect r = rect();
    if(m_pGraph) m_pGraph->setDrawRect(r);

    if(m_pGraph)
    {
        m_pGraph->initializeGraph();
        emit graphResized(m_pGraph);
    }
    pEvent->accept();
}


void GraphWt::contextMenuEvent(QContextMenuEvent *pEvent)
{
    pEvent->ignore();
}


void GraphWt::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_R:
        {
            onResetGraphScales();
            pEvent->accept();
            break;
        }
        case Qt::Key_V:
        {
            GraphDlg::setActivePage(0);
            onGraphSettings();
            pEvent->accept();
            break;
        }
        case Qt::Key_G:
        {
            onGraphSettings();
            pEvent->accept();
            break;
        }

        default:pEvent->ignore();
    }
}




void GraphWt::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    Q_UNUSED(pEvent);
    setCursor(Qt::CrossCursor);
    onGraphSettings();
}


void GraphWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    bool bCtrl;
    QPoint point;
    double xu, yu, x1, y1, xmin, xmax, ymin, ymax;

    if(!m_pGraph) return;

    setFocus();

    point = pEvent->pos();

    bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    if(!rect().contains(pEvent->pos()))
    {
        m_bTransGraph = false;
        return;
    }

    if ((pEvent->buttons() & Qt::LeftButton) && m_bTransGraph)
    {
        // we translate the curves inside the graph
        m_pGraph->setAuto(false);
        x1 =  m_pGraph->clientTox(m_LastPoint.x()) ;
        y1 =  m_pGraph->clientToy(m_LastPoint.y()) ;

        xu = m_pGraph->clientTox(point.x());
        yu = m_pGraph->clientToy(point.y());

        xmin = m_pGraph->xMin() - xu+x1;
        xmax = m_pGraph->xMax() - xu+x1;
        ymin = m_pGraph->yMin() - yu+y1;
        ymax = m_pGraph->yMax() - yu+y1;

        m_pGraph->setWindow(xmin, xmax, ymin, ymax);
        update();
    }

    else if ((pEvent->buttons() & Qt::MidButton) && !bCtrl)
    //scaling
    {
        //zoom graph
        m_pGraph->setAuto(false);
        if(point.y()-m_LastPoint.y()<0) m_pGraph->scaleAxes(1.02);
        else                            m_pGraph->scaleAxes(1.0/1.02);

        update();
    }
    // we zoom the graph or the foil
    else if ((pEvent->buttons() & Qt::MidButton) || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        if(m_pGraph)
        {
            //zoom graph
            m_pGraph->setAuto(false);
            if(point.y()-m_LastPoint.y()<0) m_pGraph->scaleAxes(1.02);
            else                            m_pGraph->scaleAxes(1.0/1.02);
            update();
        }
    }
    else if(m_pGraph->isInDrawRect(point))
    {
        update();
    }
    else
    {
//        update();
    }

    m_LastPoint = point;
}


void GraphWt::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->buttons() & Qt::LeftButton)
    {
        QPoint point = pEvent->pos();

        m_LastPoint.rx() = point.x();
        m_LastPoint.ry() = point.y();

        m_bTransGraph = true;
        setCursor(Qt::ClosedHandCursor);

        m_LastPoint = point;
    }
}


void GraphWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    setCursor(Qt::CrossCursor);
    m_bTransGraph = false;

    pEvent->accept();
}


void GraphWt::wheelEvent (QWheelEvent *pEvent)
{
    double zoomFactor=1.0;

    if(pEvent->angleDelta().y()>0)
    {
        if(!DisplayOptions::bReverseZoom()) zoomFactor = 1./1.06;
        else                          zoomFactor = 1.06;
    }
    else
    {
        if(!DisplayOptions::bReverseZoom()) zoomFactor = 1.06;
        else                          zoomFactor = 1./1.06;
    }

    if (m_bXPressed)
    {
        //zoom x scale
        m_pGraph->setAutoX(false);
        m_pGraph->scaleXAxis(1./zoomFactor);
    }
    else if(m_bYPressed)
    {
        //zoom y scale
        m_pGraph->setAutoY(false);
        m_pGraph->scaleYAxis(1./zoomFactor);
    }
    else
    {
        //zoom both
        m_pGraph->setAuto(false);
        m_pGraph->scaleAxes(1./zoomFactor);
    }

    m_pGraph->setAutoXUnit();
    m_pGraph->setAutoYUnit();
    update();
}


/**
 * The user has requested the reset of the active graph's scales to their default value
 */
void GraphWt::onResetGraphScales()
{
    m_pGraph->setAuto(true);
    update();
}


/**
 * The user has requested an edition of the settings of the active graph
 */
void GraphWt::onGraphSettings()
{
    GraphDlg grDlg(this);
    grDlg.setGraph(m_pGraph);

//    QAction *action = qobject_cast<QAction *>(sender());
//    grDlg.setActivePage(0);

    if(grDlg.exec() == QDialog::Accepted)
    {
        switch(m_pGraph->graphType())
        {
            case  GRAPH::INVERSEGRAPH:
            {
                break;
            }
            case GRAPH::OPPGRAPH:
            {
                if(m_pGraph->yVariable() == 0 || m_pGraph->yVariable()>=2)
                {
                    m_pGraph->setYTitle(tr("Cp"));
                    m_pGraph->setInverted(true);
                }
                else
                {
                    m_pGraph->setYTitle(tr("Q"));
                    m_pGraph->setInverted(false);
                }
                m_pGraph->resetYLimits();
                break;
            }
            case GRAPH::POLARGRAPH:
            {
                QString Title;
                Title = Polar::variableName(m_pGraph->xVariable());
                m_pGraph->setXTitle(Title);

                Title = Polar::variableName(m_pGraph->yVariable());
                m_pGraph->setYTitle(Title);

                if(grDlg.bVariableChanged())
                {
                    m_pGraph->setAuto(true);
                }
                break;
            }
            case GRAPH::POPPGRAPH:
            {
                if(grDlg.bVariableChanged())
                {
                    m_pGraph->setAutoY(true);
                }
                break;

            }
            case GRAPH::WPOLARGRAPH:
            {
                QString Title;

                Title  = Miarex::WPolarVariableName(m_pGraph->xVariable());
                m_pGraph->setXTitle(Title);

                Title  = Miarex::WPolarVariableName(m_pGraph->yVariable());
                m_pGraph->setYTitle(Title);

                if(grDlg.bVariableChanged())
                {
                    m_pGraph->setAuto(true);
                }
                break;
            }
            case GRAPH::CPGRAPH:
            {
                break;
            }
            case GRAPH::STABTIMEGRAPH:
            {
                break;
            }
            case GRAPH::OTHERGRAPH:
                return;
        }
        emit graphChanged(m_pGraph);
    }

    update();
}




void GraphWt::setOverlayedRect(bool bShow, double tlx, double tly, double brx, double bry)
{
    m_bOverlayRectangle = bShow;
    m_TopLeft  = QPointF(tlx, tly);
    m_BotRight = QPointF(brx, bry);
}





