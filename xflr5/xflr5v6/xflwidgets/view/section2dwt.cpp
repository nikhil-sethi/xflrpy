/****************************************************************************

    Section2dWidget Class
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


#include <QPainter>
#include <QFileDialog>
#include <QKeyEvent>
#include <QDebug>
#include <QAction>

#include "section2dwt.h"
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/customdlg/gridsettingsdlg.h>

#define MININTERVAL  0.000000001

QColor Section2dWt::s_BackColor=QColor(5,7,17);
QColor Section2dWt::s_TextColor=Qt::lightGray;

Section2dWt::Section2dWt(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);

    m_hcCross = QCursor(Qt::CrossCursor);
    m_hcMove  = QCursor(Qt::ClosedHandCursor);

    m_MousePos.x = 0.0;
    m_MousePos.y = 0.0;

    m_objectScale = 1.0;

    m_bZoomPlus      = false;
    m_bZoomYOnly     = false;
    m_bTrans         = false;
    m_bDrag          = false;

    m_bShowLegend    = true;
    m_bIsImageLoaded = false;
    m_bXDown = m_bYDown = false;


    m_UnitFactor = 1.0;
    m_exp_x = m_exp_y = 0.0;
/*    m_NeutralStyle = {true, Line::DASHDOT, 1, QColor(150,150,150), Line::NOSYMBOL};
    m_XStyle       = {true, Line::DASH, 1, QColor(95,95,95), Line::NOSYMBOL};
    m_YStyle       = {true, Line::DASH, 1, QColor(95,95,95), Line::NOSYMBOL};
    m_XMinStyle    = {false, Line::DOT, 1, QColor(75,75,75), Line::NOSYMBOL};
    m_YMinStyle    = {false, Line::DOT, 1, QColor(75,75,75), Line::NOSYMBOL};*/

    m_fScale    = 1.0;
    m_fRefScale = 1.0;

    m_fScaleY    = 1.0;
    m_ptOffset.rx() = 0;
    m_ptOffset.ry() = 0;

    m_ViewportTrans = QPoint(0,0);
}



void Section2dWt::createActions()
{
    m_ActionList.clear();

    QAction *pInsertPt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
    connect(pInsertPt, SIGNAL(triggered()), this, SLOT(onInsertPt()));
    m_ActionList.append(pInsertPt);

    QAction *pRemovePt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
    connect(pRemovePt, SIGNAL(triggered()), this, SLOT(onRemovePt()));
    m_ActionList.append(pRemovePt);

    QAction *pSeparator0 = new QAction(this);
    pSeparator0->setSeparator(true);
    m_ActionList.append(pSeparator0);

    QAction *pResetScaleAction = new QAction(tr("Reset Scales"), this);
    connect(pResetScaleAction, SIGNAL(triggered()), this, SLOT(onResetScales()));
    m_ActionList.append(pResetScaleAction);

    QAction *pGridSettingsAction = new QAction(tr("Grid Settings"), this);
    connect(pGridSettingsAction, SIGNAL(triggered()), this, SLOT(onGridSettings()));
    m_ActionList.append(pGridSettingsAction);

    QAction *pSeparator1 = new QAction(this);
    pSeparator1->setSeparator(true);
    m_ActionList.append(pSeparator1);

    QAction *pLoadBackImage = new QAction(tr("Load background image")   +"\tCtrl+Shift+I", this);
    connect(pLoadBackImage, SIGNAL(triggered()), this, SLOT(onLoadBackImage()));
    m_ActionList.append(pLoadBackImage);

    QAction *pClearBackImage = new QAction(tr("Clear background image") +"\tCtrl+Shift+I", this);
    connect(pClearBackImage, SIGNAL(triggered()), this, SLOT(onClearBackImage()));
    m_ActionList.append(pClearBackImage);
}



void Section2dWt::createContextMenu()
{
    m_pSection2dContextMenu = new QMenu(this);
    for(int iAc=0; iAc<m_ActionList.count(); iAc++)
        m_pSection2dContextMenu->addAction(m_ActionList.at(iAc));
}



void Section2dWt::setContextMenu(QMenu *pMenu)
{
    m_pSection2dContextMenu = pMenu;
}


QSize Section2dWt::sizeHint() const
{
    return QSize(350,250);
}


QSize Section2dWt::minimumSizeHint() const
{
    return QSize(300,100);

}


void Section2dWt::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.save();
    painter.fillRect(rect(), DisplayOptions::backgroundColor());

    painter.setFont(DisplayOptions::textFont());
    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);

    paintGrids(painter);

    painter.drawText(5,10, QString(tr("X-Scale = %1")).arg(m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,22, QString(tr("Y-Scale = %1")).arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,34, QString(tr("x = %1")).arg(m_MousePos.x,9,'f',5));
    painter.drawText(5,46, QString(tr("y = %1")).arg(m_MousePos.y,9,'f',5));

    painter.restore();
}




void Section2dWt::contextMenuEvent (QContextMenuEvent *pEvent)
{
    m_pSection2dContextMenu->exec(pEvent->globalPos());
    setCursor(Qt::CrossCursor);
    pEvent->accept();
}


void Section2dWt::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_bZoomPlus)
            {
                releaseZoom();
                pEvent->accept();
            }
            else if(m_bZoomYOnly)
            {
                m_bZoomYOnly = false;
                pEvent->accept();
            }
            m_bXDown = m_bYDown = false;
            break;
        }

        case Qt::Key_R:
            onResetScales();
            break;
        case Qt::Key_X:
            m_bXDown = true;
            break;
        case Qt::Key_Y:
            m_bYDown = true;
            break;
        case Qt::Key_I:
            if (pEvent->modifiers().testFlag(Qt::ControlModifier) & pEvent->modifiers().testFlag(Qt::ShiftModifier))
            {
                if(!m_bIsImageLoaded)
                {
                    onLoadBackImage();
                }
                else
                {
                    onClearBackImage();
                }
            }
            break;
        default:
            QWidget::keyPressEvent(pEvent);
    }
    pEvent->ignore();
}


/**
 * The user has requested to load a background image in the view.
 */
void Section2dWt::onLoadBackImage()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, tr("Open Image File"),
                                            QString(),
                                            "Image files (*.png *.jpg *.bmp)");
    m_bIsImageLoaded = m_BackImage.load(PathName);

    update();
}


/**
 * The user has requested to clear the background image.
 */
void Section2dWt::onClearBackImage()
{
    m_bIsImageLoaded = false;
    update();
}


void Section2dWt::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_X:
            m_bXDown = false;
            break;
        case Qt::Key_Y:
            m_bYDown = false;
            break;
    }
    pEvent->ignore();
}


void Section2dWt::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    //    if(!hasFocus()) setFocus();

    QPoint center = rect().center();

    m_ptOffset.rx() += -pEvent->pos().x() + center.x();
    m_ptOffset.ry() += -pEvent->pos().y() + center.y();
    m_ViewportTrans.rx() += -pEvent->pos().x() + center.x();
    m_ViewportTrans.ry() += -pEvent->pos().y() + center.y();

    update();
    pEvent->accept();
}


void Section2dWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    //    if(!hasFocus()) setFocus();
    QPoint point = pEvent->pos();
    m_MousePos = mousetoReal(point);

    if(m_bZoomPlus && (pEvent->buttons() & Qt::LeftButton))
    {
        // we're zooming in using the rectangle method
        m_ZoomRect.setBottomRight(point);
        update();
        return;
    }
    else if((pEvent->buttons() & Qt::LeftButton) && m_bTrans)
    {
        //translate
        m_ptOffset.rx() += point.x() - m_PointDown.x();
        m_ptOffset.ry() += point.y() - m_PointDown.y();
        m_ViewportTrans.rx() += point.x() - m_PointDown.x();
        m_ViewportTrans.ry() += point.y() - m_PointDown.y();

        m_PointDown.rx() = point.x();
        m_PointDown.ry() = point.y();

        update();
        return;
    }
    else if (pEvent->buttons() & Qt::LeftButton && !m_bZoomPlus)
    {
        // user is dragging the point
        dragSelectedPoint(m_MousePos.x, m_MousePos.y);
//        emit objectModified();
    }
    else if ((pEvent->buttons() & Qt::MidButton))
    {
        // user is zooming with mouse button down rather than with wheel
        double scale = m_fScale;

        if(!m_bZoomYOnly)
        {
            if (m_bXDown)
            {
                if(point.y()-m_PointDown.y()>0)
                {
                    m_fScale  *= 1.02;
                    m_fScaleY /= 1.02;
                }
                else
                {
                    m_fScale  /= 1.02;
                    m_fScaleY *= 1.02;
                }
            }
            else if (m_bYDown)
            {
                if(point.y()-m_PointDown.y()>0) m_fScaleY *= 1.02;
                else                            m_fScaleY /= 1.02;
            }
            else
            {
                if(point.y()-m_PointDown.y()>0) m_fScale *= 1.02;
                else                            m_fScale /= 1.02;
            }
        }
        else
        {
            if(point.y()-m_PointDown.y()>0) m_fScaleY *= 1.02;
            else                            m_fScaleY /= 1.02;
        }

        m_PointDown = point;

        int a = rect().center().x();
        m_ptOffset.rx() = a + int((m_ptOffset.x()-a)*m_fScale/scale);
    }
    else if(pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        double zoomFactor=1.0;

        if(point.y()-m_PointDown.y()<0) zoomFactor = 1./1.02;
        else                            zoomFactor = 1.02;

        zoomView(zoomFactor);
        m_PointDown = point;
    }
    else if(!m_bZoomPlus)
    {
        //not zooming, check if mouse passes over control point and highlight
        highlightPoint(m_MousePos);
    }
    update();
    pEvent->accept();
}


void Section2dWt::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint point = pEvent->pos();

    // get a reference for mouse movements
    m_PointDown.rx() = point.x();
    m_PointDown.ry() = point.y();

    if(m_bZoomPlus)
    {
        m_ZoomRect.setTopLeft(point);
        m_ZoomRect.setBottomRight(point);
    }
    else if(!m_bZoomPlus && (pEvent->buttons() & Qt::LeftButton))
    {
        if (pEvent->modifiers() & Qt::ShiftModifier)
        {
            onInsertPt();
        }
        else if (pEvent->modifiers() & Qt::ControlModifier)
        {
            onRemovePt();
        }
        else
        {
            //Selects the point
            int iSelect = selectPoint(mousetoReal(point));
            if(iSelect>=0)
            {
                //dragging a point
                setCursor(m_hcMove);
                m_bDrag = true;
                //                m_bTrans = false;
            }
            else
            {
                //dragging the view
                setCursor(m_hcMove);
                m_bTrans = true;
                //                m_bDrag = false;
            }
        }
    }
    pEvent->accept();
}


void Section2dWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QPoint point = pEvent->pos();

    if(m_bZoomPlus)
    {
        m_ZoomRect.setBottomRight(point);
        QRect ZRect = m_ZoomRect.normalized();

        if(!ZRect.isEmpty())
        {
            m_ZoomRect = ZRect;

            double ZoomFactor = qMin(double(rect().width())  / double(m_ZoomRect.width()) ,
                                     double(rect().height()) / double(m_ZoomRect.height()));

            double newScale = qMin(ZoomFactor*m_fScale, 32.0*m_fRefScale);

            ZoomFactor = qMin(ZoomFactor, newScale/m_fScale);

            m_fScale = ZoomFactor*m_fScale;
            int a = rect().center().x();
            int b = rect().center().y();

            int aZoom = int((m_ZoomRect.right() + m_ZoomRect.left())/2);
            int bZoom = int((m_ZoomRect.top()   + m_ZoomRect.bottom())/2);

            //translate view
            m_ptOffset.rx() += (a - aZoom);
            m_ptOffset.ry() += (b - bZoom);
            //scale view
            m_ptOffset.rx() = int(ZoomFactor * (m_ptOffset.x()-a)+a);
            m_ptOffset.ry() = int(ZoomFactor * (m_ptOffset.y()-b)+b);

            //            m_ZoomRect.setBottomRight(m_ZoomRect.topLeft());
            m_ZoomRect.setRight(m_ZoomRect.left()-1);
        }
        else
        {
            m_ZoomRect.setBottomRight(m_ZoomRect.topLeft());
            releaseZoom();
        }
    }
    else if(m_bZoomPlus && !rect().contains(point))
    {
        releaseZoom();
    }
    else if(m_bTrans)
    {
        // nothing to do
    }
    else if(m_bDrag)
    {
        // finished dragging a point
    }
    // notify the world
    emit objectModified();

    setCursor(m_hcCross);
    m_bTrans = false;
    m_bDrag = false;
    update();
    pEvent->accept();
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void Section2dWt::resizeEvent (QResizeEvent *pEvent)
{
    setScale();
    pEvent->accept();
}


void Section2dWt::showEvent(QShowEvent *pEvent)
{
    setAutoUnits();
    QWidget::showEvent(pEvent);
}


void Section2dWt::wheelEvent (QWheelEvent *pEvent)
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
    zoomView(zoomFactor);
    setAutoUnits();

    update();

    pEvent->accept();
}


void Section2dWt::zoomView(double zoomFactor)
{
    m_ZoomRect.setBottomRight(m_ZoomRect.topLeft());
    releaseZoom();

    double  scale = m_fScale;

    if(!m_bZoomYOnly)
    {
        if (m_bXDown)
        {
            m_fScale  *= zoomFactor;
            m_fScaleY *= 1./zoomFactor;
        }
        else if (m_bYDown) m_fScaleY *= zoomFactor;
        else  m_fScale *= zoomFactor;
    }
    else m_fScaleY *= zoomFactor;


    int a = rect().center().x();
    int b = rect().center().y();
    m_ptOffset.rx() = a + (m_ptOffset.x()-a)*m_fScale/scale;
    m_ptOffset.ry() = b + (m_ptOffset.y()-b)*m_fScale/scale;
    m_ViewportTrans.rx() = (m_ViewportTrans.x())*m_fScale /scale;
    m_ViewportTrans.ry() = (m_ViewportTrans.y())*m_fScale /scale;
}


/**
 * Sets the default scale for the Foil display.
 */
void Section2dWt::setScale()
{
    //scale is set by user zooming
    m_fRefScale = double(rect().width())-150.0;
    m_fScale = m_fRefScale;

    m_ptOffset.rx() = rect().width()/2;
    m_ptOffset.ry() = rect().height()/2;

    m_ViewportTrans = QPoint(0,0);
}



/**
 * Draws the grids.
 * @param painter a reference to the QPainter object with which to draw.
 */
void Section2dWt::paintGrids(QPainter &painter)
{
    painter.save();

    if(m_Grid.bXAxis())   drawXScale(painter);
    if(m_Grid.bYAxis())   drawYScale(painter);

    //draw grids
    if(m_Grid.bXMajGrid())  drawXGrid(painter, m_ptOffset);
    if(m_Grid.bYMajGrid(0)) drawYGrid(painter, m_ptOffset);
    if(m_Grid.bXMinGrid())  drawXMinGrid(painter);
    if(m_Grid.bYMinGrid(0)) drawYMinGrid(painter);

    painter.restore();
}

/**
 * Draws the scale on the neutral line.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex
 */
void Section2dWt::drawXScale(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;

    QString format = xfl::g_bLocalize ? "%L1" : "%1";

    double scalex = m_fScale;
    painter.save();
    painter.setFont(DisplayOptions::textFont());
    int dD = DisplayOptions::textFontStruct().height();
    QString strLabel = QString(format).arg(0.25,0,'f', m_Grid.nDecimals());

    int dW = int((DisplayOptions::textFontStruct().width(strLabel)*4)/16);
    int TickSize = int(dD*3/4);
    int offy = int(m_ptOffset.y());
    offy = std::max(offy,0); //pixels
    offy = std::min(offy, rect().height()-(dD*7/4)-2); //pixels


    QPen AxisPen(m_Grid.xAxisColor());
    AxisPen.setStyle(xfl::getStyle(m_Grid.xAxisStipple()));
    AxisPen.setWidth(m_Grid.xAxisWidth());
    painter.setPen(AxisPen);

    QPen LabelPen(s_TextColor);

    int XMin  = rect().left();
    int XMax = rect().right();

    double xunit = m_Grid.xMajUnit()/m_UnitFactor;
    double xt = -xunit;

    painter.drawLine(XMin, offy, XMax, offy);

    // ticks and labels for x<0
    int iter = 0;
    int xu = int(xt*scalex + m_ptOffset.x());
    while(xu>XMin && iter<500)
    {
        xu = int(xt*scalex + m_ptOffset.x());
        if(xu<XMax)
        {
            painter.setPen(AxisPen);
            painter.drawLine(xu, offy, xu, offy+TickSize);
            painter.setPen(LabelPen);
            drawLabel(painter, xu-dW, offy+(dD*7)/4, xt*m_UnitFactor, m_exp_x, Qt::AlignHCenter);
        }
        xt -= xunit;
        iter++;
    }

    // ticks and labels for x>0
    if(m_ptOffset.x()<XMin)
    {
        // skip tickmarks left of the view rectangle
        int nt = int((double(XMin)-m_ptOffset.x())/scalex/xunit);
        xt = nt * xunit;
    }
    else xt = xunit; // start at origin +1 unit
    xu = int(xt*scalex + m_ptOffset.x());
    iter = 0;
    while(xu<XMax  && iter<500)
    {
        xu = int(xt*scalex + m_ptOffset.x());
        if(xu>XMin)
        {
            painter.setPen(AxisPen);
            painter.drawLine(xu, offy, xu, offy+TickSize);
            painter.setPen(LabelPen);
            drawLabel(painter, xu-dW, offy+(dD*7)/4, xt*m_UnitFactor, m_exp_x, Qt::AlignHCenter);
        }
        xt += xunit;
        iter++;
    }

    painter.restore();
}


/**
 * Draws the scale on the neutral line.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex
 */
void Section2dWt::drawYScale(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    double scaley = m_fScale * m_fScaleY;
    painter.save();
    painter.setFont(DisplayOptions::textFont());
    int dD = DisplayOptions::textFontStruct().height();

    int height3  = int(dD/3);

    int TickSize = int(dD*3/4);
    int offx = int(m_ptOffset.x());
    offx = std::max(offx, 0);
    offx = std::min(offx, rect().width()-(TickSize*12)/8-DisplayOptions::textFontStruct().width("-1.0 10-4"));

    QPen AxisPen(m_Grid.yAxisColor());
    AxisPen.setStyle(xfl::getStyle(m_Grid.yAxisStipple()));
    AxisPen.setWidth(m_Grid.yAxisWidth());
    QPen LabelPen(s_TextColor);

    int YMin  = rect().top();
    int YMax = rect().bottom();


    double yunit = m_Grid.yMajUnit(0)/m_UnitFactor;

    double yt = -yunit;

    painter.setPen(AxisPen);
    painter.drawLine(offx, YMin, offx, YMax);

    int iter = 0;

    while(int(yt*scaley) + m_ptOffset.y()>YMin && iter<500)
    {
        int yu = int(yt*scaley + m_ptOffset.y());
        if(yu<YMax)
        {
            painter.setPen(AxisPen);
            painter.drawLine(offx, yu, offx+TickSize, yu);
            yu += height3;
            painter.setPen(LabelPen);
            drawLabel(painter, offx+(TickSize*12)/8, yu, -yt*m_UnitFactor, m_exp_y, Qt::AlignLeft);
        }
        yt -= yunit;
        iter++;
    }

    yt = yunit;
    iter = 0;
    while(int(yt*scaley) + m_ptOffset.y()<YMax  && iter<500)
    {
        int yu = int(yt*scaley + m_ptOffset.y());
        if(yu>YMin)
        {
            painter.setPen(AxisPen);
            painter.drawLine(offx, yu, offx+TickSize, yu);
            yu += height3;

            painter.setPen(LabelPen);
            drawLabel(painter, offx+(TickSize*12)/8, yu, -yt*m_UnitFactor, m_exp_y, Qt::AlignLeft);
        }
        yt += yunit;
        iter++;
    }

    painter.restore();
}


void Section2dWt::drawXGrid(QPainter &painter, const QPointF &Offset)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    painter.save();
    QPen GridPen(m_Grid.xMajColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    GridPen.setWidth(m_Grid.xMajWidth());
    painter.setPen(GridPen);

    int YMin = rect().top();
    int YMax = rect().bottom();

    double xunit = m_Grid.xMajUnit()/m_UnitFactor;
    double xt = 0;
    if(m_Grid.bXAxis()) xt -= xunit;

    int iter = 0;
    while(int(xt*m_fScale) + Offset.x()>rect().left() && iter<500)
    {
        if(int(xt*m_fScale) + Offset.x()<rect().right())
        {
            painter.drawLine(int(xt*m_fScale + Offset.x()), YMin, int(xt*m_fScale + Offset.x()), YMax);
            iter++;
        }
        xt -= xunit;
    }

    xt = xunit;
    iter = 0;
    while(int(xt*m_fScale) + Offset.x()<rect().right() && iter<500)
    {
        if(int(xt*m_fScale) + Offset.x()>rect().left())
        {
            painter.drawLine(int(xt*m_fScale + Offset.x()), YMin, int(xt*m_fScale + Offset.x()), YMax);
            iter++;
        }
        xt += xunit;
    }

    painter.restore();
}


void Section2dWt::drawYGrid(QPainter &painter, const QPointF &Offset)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;

    double scaley = m_fScale * m_fScaleY;

    painter.save();
    QPen GridPen(m_Grid.yMajColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMajStipple(0)));
    GridPen.setWidth(m_Grid.yMajWidth(0));
    painter.setPen(GridPen);

    int XMin = rect().left();
    int XMax = rect().right();

    double yunit = m_Grid.yMajUnit(0)/m_UnitFactor;
    double yt = 0;
    if(m_Grid.bXAxis()) yt -= yunit;

    int iter = 0;
    while(int(yt*scaley) + Offset.y()>rect().top() && iter<500)
    {
        if(int(yt*scaley) + Offset.y()<rect().bottom())
        {
            painter.drawLine(XMin, int(yt*scaley + Offset.y()), XMax, int(yt*scaley + Offset.y()));
            iter++;
        }
        yt -= yunit;
    }

    iter = 0;
    yt = yunit;
    while(int(yt*scaley) + Offset.y()<rect().bottom() && iter<500)
    {
        if(int(yt*scaley) + Offset.y()>rect().top())
        {
            painter.drawLine(XMin, int(yt*scaley + Offset.y()), XMax, int(yt*scaley + Offset.y()));
            iter++;
        }
        yt += yunit;
    }

    painter.restore();
}


void Section2dWt::drawXMinGrid(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    painter.save();
    double scalex = m_fScale;
    QPen GridPen(m_Grid.xMinColor());
    GridPen.setWidth(m_Grid.xMinWidth());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMinStipple()));

    painter.setPen(GridPen);

    int YMin = rect().top();
    int YMax = rect().bottom();

    double xunit = m_Grid.xMinUnit()/m_UnitFactor;
    double xt = -xunit;

    int iter = 0;
    while(int(xt*scalex) + m_ptOffset.x()>rect().left() && iter<500)
    {
        if(int(xt*scalex) + m_ptOffset.x()<rect().right())
        {
            painter.drawLine(int(xt*scalex + m_ptOffset.x()), YMin, int(xt*scalex + m_ptOffset.x()), YMax);
            iter++;
        }
        xt -= xunit;
    }

    xt = xunit;
    iter = 0;
    while(int(xt*scalex) + m_ptOffset.x()<rect().right() && iter<500)
    {
        if(rect().left()< int(xt*scalex) + m_ptOffset.x())
        {
            painter.drawLine(int(xt*scalex + m_ptOffset.x()), YMin, int(xt*scalex + m_ptOffset.x()), YMax);
            iter++;
        }
        xt += xunit;
    }

    painter.restore();
}


void Section2dWt::drawYMinGrid(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    painter.save();
    double scaley = m_fScale * m_fScaleY;
    QPen GridPen(m_Grid.yMinColor(0));
    GridPen.setWidth(m_Grid.yMinWidth(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMinStipple(0)));

    painter.setPen(GridPen);

    int XMin = rect().left();
    int XMax = rect().right();

    double yunit = m_Grid.yMinUnit(0)/m_UnitFactor;
    double yt = -yunit;

    int iter = 0;

    while(int(yt*scaley) + m_ptOffset.y()>rect().top() && iter<500)
    {
        if(int(yt*scaley) + m_ptOffset.y()<rect().bottom())
        {
            painter.drawLine(XMin, int(yt*scaley + m_ptOffset.y()), XMax, int(yt*scaley + m_ptOffset.y()));
            iter++;
        }
        yt -= yunit;
    }

    yt = yunit;
    iter = 0;
    while(int(yt*scaley) + m_ptOffset.y()<rect().bottom() && iter<500)
    {
        if(int(yt*scaley) + m_ptOffset.y()>rect().top())
        {
            painter.drawLine(XMin, int(yt*scaley + m_ptOffset.y()), XMax, int(yt*scaley + m_ptOffset.y()));
            iter++;
        }
        yt += yunit;
    }

    painter.restore();
}




void Section2dWt::drawLabel(QPainter &painter, int xu, int yu, double value, int expo, Qt::Alignment align)
{
    int exp=0;
    double main = value;
    xfl::expFormat(main, exp);

    if(fabs(value)<MININTERVAL)
    {
        QString strLabel = "0";
        painter.drawText(xu, yu, strLabel);
        return;
    }

    QString strLabel;
    QString strLabelExp;

    strLabel = QString(xfl::g_bLocalize ? "%L1" : "%1").arg(value, expo);
    if(align & Qt::AlignHCenter)
    {
        int px = DisplayOptions::textFontStruct().width(strLabel);
        painter.drawText(xu-px/2, yu, strLabel);
    }
    else if(align & Qt::AlignLeft)
    {
        painter.drawText(xu, yu, strLabel);
    }
}


/**
 * Draws the scale on the neutral line.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex
 */
void Section2dWt::drawScale(QPainter &painter, double scalex)
{
    painter.save();

    painter.setFont(DisplayOptions::textFont());

    QFontMetrics fm(DisplayOptions::textFont());
    int dD = fm.height();
    int dW = fm.horizontalAdvance("0.1");

    int TickSize, offy;

    TickSize = dD/2;
    offy = int(m_ptOffset.y());

    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);

    double xo = 0.0;
    double xmin = 0.0;
    double xmax = 1.0;

    double XGridUnit = 0.1;
    double XHalfGridUnit = 0.05;
    double XMinGridUnit = 0.01;

    double xt  = xo-int((xo-xmin)*1.0001/XGridUnit)*XGridUnit;//one tick at the origin
    double xht = xo-int((xo-xmin)*1.0001/XHalfGridUnit)*XHalfGridUnit;//one tick at the origin
    double xmt = xo-int((xo-xmin)*1.0001/XMinGridUnit)*XMinGridUnit;//one tick at the origin

    painter.drawLine(int(xt*scalex + m_ptOffset.x()), offy, int(xmax*scalex + m_ptOffset.x()), offy);

    QString strLabel;

    while(xt<=xmax*1.001)
    {
        //Draw  ticks
        painter.drawLine(int(xt*scalex + m_ptOffset.x()), offy, int(xt*scalex + m_ptOffset.x()), offy+TickSize*2);
        strLabel = QString("%1").arg(xt,4,'f',1);
        painter.drawText(int(xt*scalex+m_ptOffset.x())-dW/2, offy+dD*2, strLabel);
        xt += XGridUnit ;
    }

    //    while(xht<=xmax*1.001)
    xht = 0;
    for(int i=0;i<1/XHalfGridUnit;i++)
    {
        if(i%2!=0) painter.drawLine(int(xht*scalex + m_ptOffset.x()), offy, int(xht*scalex + m_ptOffset.x()), offy+TickSize*2);
        xht += XHalfGridUnit ;
    }

    xmt=0;
    //    while(xmt<=xmax*1.001)
    for(int i=0;i<1/XMinGridUnit;i++)
    {
        if(i%5!=0) painter.drawLine(int(xmt*scalex + m_ptOffset.x()), offy,int(xmt*scalex + m_ptOffset.x()), offy+TickSize);
        xmt += XMinGridUnit ;
    }

    painter.restore();
}


/**
 * Ends the zoom-in action.
 */
void Section2dWt::releaseZoom()
{
   //    pMainFrame->zoomInAct->setChecked(false);
    m_bZoomPlus = false;

    m_ZoomRect.setRight(m_ZoomRect.left()-1);
    m_ZoomRect.setTop(m_ZoomRect.bottom()+1);
    setCursor(m_hcCross);
}


/**
 * Converts screen coordinate to viewport coordinates
 * @param point the screen coordinates
 * @return the viewport coordinates
 */
Vector3d Section2dWt::mousetoReal(QPoint &point)
{
    Vector3d Real;

    Real.x =  (point.x() - m_ptOffset.x())/m_fScale;
    Real.y = -(point.y() - m_ptOffset.y())/m_fScale/m_fScaleY;
    Real.z = 0.0;

    return Real;
}



/**
 * The user has requested to reset the x-scale to its default value.
 */
void Section2dWt::onResetXScale()
{
    setScale();
    releaseZoom();
    update();
}


/**
 * The user has requested to reset the y-scale to its default value.
 */
void Section2dWt::onResetYScale()
{
    m_fScaleY = 1.0;
    update();
}


/**
 * The user has requested to reset the scales to their default value.
 */
void Section2dWt::onResetScales()
{
    m_fScaleY = 1.0;
    setScale();
    releaseZoom();
    update();
}


/**
 * The user has requested the launch the interface for the edition of the grid parameters.
 */
void Section2dWt::onGridSettings()
{
    GridSettingsDlg dlg;

    dlg.initDialog(m_Grid, false);

    if(dlg.exec() == QDialog::Accepted)
    {
        m_Grid = dlg.grid();
    }
    update();
}


void Section2dWt::drawScaleLegend(QPainter &painter)
{
    painter.save();

    painter.setFont(DisplayOptions::textFont());
    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);

    painter.drawText(5,10, QString(tr("X-Scale = %1")).arg(m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,22, QString(tr("Y-Scale = %1")).arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,34, QString(tr("x = %1")).arg(m_MousePos.x,9,'f',5));
    painter.drawText(5,46, QString(tr("y = %1")).arg(m_MousePos.y,9,'f',5));

    painter.restore();
}


void Section2dWt::drawBackImage(QPainter &painter)
{
    if(m_bIsImageLoaded && !m_BackImage.isNull())
    {
        painter.save();

        double xscale = m_fScale          /m_fRefScale;
        double yscale = m_fScale*m_fScaleY/m_fRefScale;

        //zoom from the center of the viewport
        QPoint VCenter = rect().center();

        //draw the background image in the viewport
        int w = int(double(m_BackImage.width())* xscale);
        int h = int(double(m_BackImage.height())* yscale);
        //the coordinates of the top left corner are measured from the center of the viewport
        double xtop = VCenter.x() + m_ViewportTrans.x() - double(m_BackImage.width())  /2.*xscale;
        double ytop = VCenter.y() + m_ViewportTrans.y() - double(m_BackImage.height()) /2.*yscale;

        painter.drawPixmap(int(xtop), int(ytop), w, h, m_BackImage);
        painter.restore();
    }
}


/**
 * The user has requested to zoom in on the display by drawing a rectangle on the screen.
 */
void Section2dWt::onZoomIn()
{
    if(!m_bZoomPlus)
    {
        if(m_fScale/m_fRefScale <32.0)
        {
            m_bZoomPlus = true;
        }
        else
        {
            releaseZoom();
        }
    }
    else
    {
        releaseZoom();
    }
}


/**
 * The user has requested to scale the y-axis only.
 */
void Section2dWt::onZoomYOnly()
{
    m_bZoomYOnly = !m_bZoomYOnly;
}


/**
 * The user has requested to zoom out.
 */
void Section2dWt::onZoomLess()
{
    releaseZoom();

    double ZoomFactor = 0.8;
    double newScale = qMax(ZoomFactor*m_fScale, m_fRefScale);

    ZoomFactor = qMax(ZoomFactor, newScale/m_fScale);

    m_fScale = ZoomFactor*m_fScale;
    int a = rect().center().x();
    int b = rect().center().y();

    //scale
    m_ptOffset.rx() = int(ZoomFactor*(m_ptOffset.x()-a)+a);
    m_ptOffset.ry() = int(ZoomFactor*(m_ptOffset.y()-b)+b);

    update();
}


void Section2dWt::setAutoUnits()
{
    double unit=0;

    setAutoUnit(m_fScale, unit, m_exp_x);
    m_Grid.setXMajUnit(unit);

    setAutoUnit(m_fScale*m_fScaleY, unit, m_exp_y);
    m_Grid.setYMajUnit(0,unit);

    m_Grid.setXMinUnit(m_Grid.xMajUnit()/5.0);
    m_Grid.setYMinUnit(0, m_Grid.yMajUnit(0)/5.0);
}


void Section2dWt::setAutoUnit(double scale, double &unit, int &exponent)
{
    int nTicks = width()/DisplayOptions::textFontStruct().averageCharWidth()/10;

/*    if     (nTicks>=10) nTicks = 10; // number of ticks in the view
    else*/
         if(nTicks>=5)  nTicks = 5;
    else if(nTicks>=2)  nTicks = 2;
    else                nTicks = 1;

    int XMin = rect().left();
    int XMax = rect().right();
    double xmin = (XMin-m_ptOffset.x())/scale;
    double xmax = (XMax-m_ptOffset.x())/scale;

    unit = (xmax-xmin)/double(nTicks) * m_UnitFactor;

    if (unit <1.0)
    {
        exponent = int(log10(unit *1.00001)-1);
//        exponent = std::max(-4, exponent);
    }
    else exponent = int(log10(unit *1.00001));
    int main = int(unit /pow(10.0, exponent)*1.000001);


    if(main<2)
        unit  = pow(10.0, exponent);
    else if (main<5)
        unit  = 2.0*pow(10.0, exponent);
    else
        unit  = 5.0*pow(10.0, exponent);
}


