/****************************************************************************

    Section2dWidget Class
    Copyright (C) 2015-2019 Andre Deperrois

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

#include "section2dwidget.h"
#include <globals/globals.h>
#include <misc/options/displayoptions.h>
#include <design/GridSettingsDlg.h>
#include <graph_globals.h>


Section2dWidget::Section2dWidget(QWidget *parent) : QWidget(parent)
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
    m_bNeutralLine   = true;
    m_bScale         = false;
    m_bShowLegend    = true;
    m_bIsImageLoaded = false;
    m_bXDown = m_bYDown = false;

    m_bXGrid     = false;
    m_XGridUnit  = 0.05;
    m_XGridStyle = 1;
    m_XGridWidth = 1;
    m_XGridColor = QColor(150,150,150);

    m_bYGrid     = false;
    m_YGridUnit  = 0.05;
    m_YGridStyle = 1;
    m_YGridWidth = 1;
    m_YGridColor = QColor(150,150,150);

    m_bXMinGrid  = false;
    m_XMinUnit = 0.01;
    m_XMinStyle  = 2;
    m_XMinWidth  = 1;
    m_XMinColor  = QColor(70,70,70);

    m_bYMinGrid  = false;
    m_YMinUnit = 0.01;
    m_YMinStyle  = 2;
    m_YMinWidth  = 1;
    m_YMinColor  = QColor(70,70,70);

    m_NeutralStyle = 3;
    m_NeutralWidth = 1;
    m_NeutralColor = QColor(213,213,255);

    m_fScale    = 1.0;
    m_fRefScale = 1.0;

    m_fScaleY    = 1.0;
    m_ptOffset.rx() = 0;
    m_ptOffset.ry() = 0;

    m_ViewportTrans = QPoint(0,0);
}



void Section2dWidget::createActions()
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



void Section2dWidget::createContextMenu()
{
    m_pSection2dContextMenu = new QMenu(this);
    for(int iAc=0; iAc<m_ActionList.count(); iAc++)
        m_pSection2dContextMenu->addAction(m_ActionList.at(iAc));
}



void Section2dWidget::setContextMenu(QMenu *pMenu)
{
    m_pSection2dContextMenu = pMenu;
}



QSize Section2dWidget::sizeHint() const
{
    return QSize(350,250);
}


QSize Section2dWidget::minimumSizeHint() const
{
    return QSize(300,100);

}


void Section2dWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.save();
    painter.fillRect(rect(), Settings::s_BackgroundColor);

    painter.setFont(Settings::s_TextFont);

    QPen TextPen(Settings::s_TextColor);
    painter.setPen(TextPen);

    paintGrids(painter);

    painter.drawText(5,10, QString(tr("X-Scale = %1")).arg(m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,22, QString(tr("Y-Scale = %1")).arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,34, QString(tr("x  = %1")).arg(m_MousePos.x,7,'f',4));
    painter.drawText(5,46, QString(tr("y  = %1")).arg(m_MousePos.y,7,'f',4));

    painter.restore();
}




void Section2dWidget::contextMenuEvent (QContextMenuEvent *pEvent)
{
    m_pSection2dContextMenu->exec(pEvent->globalPos());
    setCursor(Qt::CrossCursor);
    pEvent->accept();
}


void Section2dWidget::keyPressEvent(QKeyEvent *pEvent)
{
    //	bool bShift = false;
    //	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;

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
void Section2dWidget::onLoadBackImage()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, tr("Open Image File"),
                                            Settings::s_LastDirName,
                                            "Image files (*.png *.jpg *.bmp)");
    m_bIsImageLoaded = m_BackImage.load(PathName);

    update();
}


/**
 * The user has requested to clear the background image.
 */
void Section2dWidget::onClearBackImage()
{
    m_bIsImageLoaded = false;
    update();
}


void Section2dWidget::keyReleaseEvent(QKeyEvent *pEvent)
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


void Section2dWidget::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    //	if(!hasFocus()) setFocus();

    QPoint center = rect().center();

    m_ptOffset.rx() += -pEvent->pos().x() + center.x();
    m_ptOffset.ry() += -pEvent->pos().y() + center.y();
    m_ViewportTrans.rx() += -pEvent->pos().x() + center.x();
    m_ViewportTrans.ry() += -pEvent->pos().y() + center.y();

    update();
    pEvent->accept();
}


void Section2dWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    //	if(!hasFocus()) setFocus();
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
                else		                    m_fScale /= 1.02;
            }
        }
        else
        {
            if(point.y()-m_PointDown.y()>0) m_fScaleY *= 1.02;
            else                            m_fScaleY /= 1.02;
        }

        m_PointDown = point;

        int a = rect().center().x();
        m_ptOffset.rx() = a + (int)((m_ptOffset.x()-a)*m_fScale/scale);
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


void Section2dWidget::mousePressEvent(QMouseEvent *pEvent)
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
                //				m_bTrans = false;
            }
            else
            {
                //dragging the view
                setCursor(m_hcMove);
                m_bTrans = true;
                //				m_bDrag = false;
            }
        }
    }
    pEvent->accept();
}


void Section2dWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QPoint point = pEvent->pos();

    if(m_bZoomPlus)
    {
        m_ZoomRect.setBottomRight(point);
        QRect ZRect = m_ZoomRect.normalized();

        if(!ZRect.isEmpty())
        {
            m_ZoomRect = ZRect;

            double ZoomFactor = qMin((double)rect().width()  / (double)m_ZoomRect.width() ,
                                     (double)rect().height() / (double)m_ZoomRect.height());

            double newScale = qMin(ZoomFactor*m_fScale, 32.0*m_fRefScale);

            ZoomFactor = qMin(ZoomFactor, newScale/m_fScale);

            m_fScale = ZoomFactor*m_fScale;
            int a = rect().center().x();
            int b = rect().center().y();

            int aZoom = (int)((m_ZoomRect.right() + m_ZoomRect.left())/2);
            int bZoom = (int)((m_ZoomRect.top()   + m_ZoomRect.bottom())/2);

            //translate view
            m_ptOffset.rx() += (a - aZoom);
            m_ptOffset.ry() += (b - bZoom);
            //scale view
            m_ptOffset.rx() = (int)(ZoomFactor * (m_ptOffset.x()-a)+a);
            m_ptOffset.ry() = (int)(ZoomFactor * (m_ptOffset.y()-b)+b);

            //			m_ZoomRect.setBottomRight(m_ZoomRect.topLeft());
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
void Section2dWidget::resizeEvent (QResizeEvent *event)
{
    setScale();
    event->accept();
}


void Section2dWidget::wheelEvent (QWheelEvent *pEvent)
{
    double zoomFactor=1.0;

    if(pEvent->delta()>0)
    {
        if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
        else                          zoomFactor = 1.06;
    }
    else
    {
        if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
        else                          zoomFactor = 1./1.06;
    }
    zoomView(zoomFactor);

    update();

    pEvent->accept();
}


void Section2dWidget::zoomView(double zoomFactor)
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
void Section2dWidget::setScale()
{
    //scale is set by user zooming
    m_fRefScale = (double)rect().width()-150.0;
    m_fScale = m_fRefScale;

    m_ptOffset.rx() = rect().width()/2;
    m_ptOffset.ry() = rect().height()/2;

    m_ViewportTrans = QPoint(0,0);
}



/**
 * Draws the grids.
 * @param painter a reference to the QPainter object with which to draw.
 */
void Section2dWidget::paintGrids(QPainter &painter)
{
    painter.save();
    if(m_bZoomPlus&& !m_ZoomRect.isEmpty())
    {
        QRect ZRect = m_ZoomRect.normalized();
        QPen ZoomPen(QColor(100,100,100));
        ZoomPen.setStyle(Qt::DashLine);
        painter.setPen(ZoomPen);
        painter.drawRect(ZRect);
    }


    if (m_bNeutralLine)
    {
        QPen NPen(m_NeutralColor);
        NPen.setStyle(getStyle(m_NeutralStyle));
        NPen.setWidth(m_NeutralWidth);
        painter.setPen(NPen);

        painter.drawLine(m_ptOffset.x(), rect().bottom(), m_ptOffset.x(), rect().top());
        painter.drawLine(rect().right(), m_ptOffset.y(), rect().left(), m_ptOffset.y());
    }

    //draw grids
    if(m_bXGrid)	drawXGrid(painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
    if(m_bYGrid)	drawYGrid(painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
    if(m_bXMinGrid) drawXMinGrid(painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
    if(m_bYMinGrid) drawYMinGrid(painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);

    if(m_bScale) drawScale(painter, m_fScale);

    painter.restore();
}


/**
 * Draws the X main grid.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale factor in the x-direction
 * @param scaley the scale factor in the y-direction
 * @param Offset the Foil leading edge offset in the client area
 * @param dRect the drawing rectangle
 */
void Section2dWidget::drawXGrid(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    Q_UNUSED(scaley);
    painter.save();
    QPen GridPen(m_XGridColor);
    GridPen.setStyle(getStyle(m_XGridStyle));
    GridPen.setWidth(m_XGridWidth);
    painter.setPen(GridPen);

    int YMin = rect().top();
    int YMax = rect().bottom();

    double xt = -m_XGridUnit;

    int iter = 0;
    while(int(xt*scalex) + Offset.x()>rect().left() && iter<100)
    {
        //Draw  grid
        painter.drawLine(int(xt*scalex) + Offset.x(), YMin, int(xt*scalex) + Offset.x(), YMax);
        xt -= m_XGridUnit;
        iter++;
    }

    xt = m_XGridUnit;
    iter = 0;
    while(int(xt*scalex) + Offset.x()<rect().right() && iter<100)
    {
        //Draw  grid
        painter.drawLine(int(xt*scalex) + Offset.x(), YMin, int(xt*scalex) + Offset.x(), YMax);
        xt += m_XGridUnit ;
        iter++;
    }

    painter.restore();
}



/**
 * Draws the Y main grid.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale factor in the x-direction
 * @param scaley the scale factor in the y-direction
 * @param Offset the Foil leading edge offset in the client area
 * @param dRect the drawing rectangle
 */
void Section2dWidget::drawYGrid(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    Q_UNUSED(scalex);

    painter.save();
    QPen GridPen(m_YGridColor);
    GridPen.setStyle(getStyle(m_YGridStyle));
    GridPen.setWidth(m_YGridWidth);
    painter.setPen(GridPen);

    int XMin = rect().left();
    int XMax = rect().right();

    double yt = -m_YGridUnit;//one tick at the origin
    int iter = 0;
    while((int)(yt*scaley) + Offset.y()>rect().top() && iter<100)
    {
        painter.drawLine(XMin, (int)(yt*scaley) + Offset.y(), XMax, (int)(yt*scaley) + Offset.y());
        yt -= m_YGridUnit;
        iter++;
    }

    iter = 0;
    yt = m_YGridUnit;
    while((int)(yt*scaley) + Offset.y()<rect().bottom() && iter<100)
    {
        painter.drawLine(XMin, (int)(yt*scaley) + Offset.y(), XMax, (int)(yt*scaley) + Offset.y());
        yt += m_YGridUnit;
        iter++;
    }

    painter.restore();
}



/**
 * Draws the X minor grid.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale factor in the x-direction
 * @param scaley the scale factor in the y-direction
 * @param Offset the Foil leading edge offset in the client area
 * @param dRect the drawing rectangle
 */
void Section2dWidget::drawXMinGrid(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    painter.save();

    QPen GridPen(m_XMinColor);
    GridPen.setWidth(m_XMinWidth);
    GridPen.setStyle(getStyle(m_XMinStyle));

    painter.setPen(GridPen);

    int YMin = rect().top();
    int YMax = rect().bottom();

    double xt = -m_XMinUnit;

    int iter = 0;
    while(int(xt*scalex) + Offset.x()>rect().left() && iter<100)
    {
        //Draw  grid
        painter.drawLine(int(xt*scalex) + Offset.x(), YMin, int(xt*scalex) + Offset.x(), YMax);
        xt -= m_XMinUnit;
        iter++;
    }

    xt = m_XMinUnit;
    iter = 0;
    while(int(xt*scalex) + Offset.x()<rect().right() && iter<100)
    {
        //Draw  grid
        painter.drawLine(int(xt*scalex) + Offset.x(), YMin, int(xt*scalex) + Offset.x(), YMax);
        xt += m_XMinUnit ;
        iter++;
    }

    painter.restore();
}



/**
 * Draws the Y minor grid.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale factor in the x-direction
 * @param scaley the scale factor in the y-direction
 * @param Offset the Foil leading edge offset in the client area
 * @param dRect the drawing rectangle
 */
void Section2dWidget::drawYMinGrid(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    painter.save();

    QPen GridPen(m_YMinColor);
    GridPen.setWidth(m_YMinWidth);
    GridPen.setStyle(getStyle(m_YMinStyle));

    painter.setPen(GridPen);

    int XMin = rect().left();
    int XMax = rect().right();

    double yt =  +(rect().bottom() - m_ptOffset.y())/m_fScale/m_fScaleY;

    int iter = -m_YMinUnit;
    yt = 0;
    while((int)(yt*scaley) + Offset.y()>rect().top() && iter<100)
    {
        //Draw  grid
        painter.drawLine(XMin, int(yt*scaley) + Offset.y(), XMax, int(yt*scaley) + Offset.y());
        yt -= m_YMinUnit;
        iter++;
    }

    yt = m_YMinUnit;
    iter = 0;
    while((int)(yt*scaley) + Offset.y()<rect().bottom() && iter<100)
    {
        //Draw  grid
        painter.drawLine(XMin, int(yt*scaley) + Offset.y(), XMax, int(yt*scaley) + Offset.y());
        yt += m_YMinUnit ;
        iter++;
    }
    painter.restore();
}





/**
 * Draws the scale on the neutral line.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex
 */
void Section2dWidget::drawScale(QPainter &painter, double scalex)
{
    int i;
    painter.save();

    painter.setFont(Settings::s_TextFont);

    QFontMetrics fm(Settings::s_TextFont);
    int dD = fm.height();
    int dW = fm.width("0.1");

    int TickSize, offy;

    TickSize = (int)(dD/2);
    offy = m_ptOffset.y();

    QPen TextPen(Settings::s_TextColor);
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

    painter.drawLine(int(xt*scalex) + m_ptOffset.x(), offy, int(xmax*scalex) + m_ptOffset.x(), offy);

    QString strLabel;

    while(xt<=xmax*1.001)
    {
        //Draw  ticks
        painter.drawLine(int(xt*scalex) + m_ptOffset.x(), offy, int(xt*scalex) + m_ptOffset.x(), offy+TickSize*2);
        strLabel = QString("%1").arg(xt,4,'f',1);
        painter.drawText(int(xt*scalex)+m_ptOffset.x()-dW/2, offy+dD*2, strLabel);
        xt += XGridUnit ;
    }

    //	while(xht<=xmax*1.001)
    xht = 0;
    for(i=0;i<1/XHalfGridUnit;i++)
    {
        if(i%2!=0) painter.drawLine(int(xht*scalex) + m_ptOffset.x(), offy, int(xht*scalex) + m_ptOffset.x(), offy+TickSize*2);
        xht += XHalfGridUnit ;
    }

    xmt=0;
    //	while(xmt<=xmax*1.001)
    for(i=0;i<1/XMinGridUnit;i++)
    {
        if(i%5!=0) painter.drawLine(int(xmt*scalex) + m_ptOffset.x(), offy,int(xmt*scalex) + m_ptOffset.x(), offy+TickSize);
        xmt += XMinGridUnit ;
    }

    painter.restore();
}


/**
 * Ends the zoom-in action.
 */
void Section2dWidget::releaseZoom()
{
   //	pMainFrame->zoomInAct->setChecked(false);
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
Vector3d Section2dWidget::mousetoReal(QPoint &point)
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
void Section2dWidget::onResetXScale()
{
    setScale();
    releaseZoom();
    update();
}


/**
 * The user has requested to reset the y-scale to its default value.
 */
void Section2dWidget::onResetYScale()
{
    m_fScaleY = 1.0;
    update();
}

/**
 * The user has requested to reset the scales to their default value.
 */
void Section2dWidget::onResetScales()
{
    m_fScaleY = 1.0;
    setScale();
    releaseZoom();
    update();
}



/**
 * The user has requested the launch the interface for the edition of the grid parameters.
 */
void Section2dWidget::onGridSettings()
{
    GridSettingsDlg dlg;

    dlg.m_bScale       = m_bScale;
    dlg.m_bNeutralLine = m_bNeutralLine;
    dlg.m_NeutralStyle = m_NeutralStyle;
    dlg.m_NeutralWidth = m_NeutralWidth;
    dlg.m_NeutralColor = m_NeutralColor;

    dlg.m_bXGrid     = m_bXGrid;
    dlg.m_bXMinGrid  = m_bXMinGrid;
    dlg.m_XStyle     = m_XGridStyle;
    dlg.m_XWidth     = m_XGridWidth;
    dlg.m_XColor     = m_XGridColor;
    dlg.m_XUnit      = m_XGridUnit;
    dlg.m_XMinStyle  = m_XMinStyle;
    dlg.m_XMinWidth  = m_XMinWidth;
    dlg.m_XMinColor  = m_XMinColor;
    dlg.m_XMinUnit   = m_XMinUnit;

    dlg.m_bYGrid     = m_bYGrid;
    dlg.m_bYMinGrid  = m_bYMinGrid;
    dlg.m_YStyle     = m_YGridStyle;
    dlg.m_YWidth     = m_YGridWidth;
    dlg.m_YColor     = m_YGridColor;
    dlg.m_YUnit      = m_YGridUnit;
    dlg.m_YMinStyle  = m_YMinStyle;
    dlg.m_YMinWidth  = m_YMinWidth;
    dlg.m_YMinColor  = m_YMinColor;
    dlg.m_YMinUnit   = m_YMinUnit;

    dlg.initDialog();

    if(dlg.exec() == QDialog::Accepted)
    {
        m_bScale       = dlg.m_bScale;
        m_bNeutralLine = dlg.m_bNeutralLine;
        m_NeutralStyle = dlg.m_NeutralStyle;
        m_NeutralWidth = dlg.m_NeutralWidth;
        m_NeutralColor = dlg.m_NeutralColor;

        m_bXGrid     = dlg.m_bXGrid;
        m_bXMinGrid  = dlg.m_bXMinGrid;
        m_XGridStyle = dlg.m_XStyle;
        m_XGridWidth = dlg.m_XWidth;
        m_XGridColor = dlg.m_XColor;
        m_XGridUnit  = dlg.m_XUnit;
        m_XMinStyle  = dlg.m_XMinStyle;
        m_XMinWidth  = dlg.m_XMinWidth;
        m_XMinColor  = dlg.m_XMinColor;
        m_XMinUnit   = dlg.m_XMinUnit;

        m_bYGrid     = dlg.m_bYGrid;
        m_bYMinGrid  = dlg.m_bYMinGrid;
        m_YGridStyle = dlg.m_YStyle;
        m_YGridWidth = dlg.m_YWidth;
        m_YGridColor = dlg.m_YColor;
        m_YGridUnit  = dlg.m_YUnit;
        m_YMinStyle  = dlg.m_YMinStyle;
        m_YMinWidth  = dlg.m_YMinWidth;
        m_YMinColor  = dlg.m_YMinColor;
        m_YMinUnit   = dlg.m_YMinUnit;
    }
    update();
}




void Section2dWidget::drawScaleLegend(QPainter &painter)
{
    painter.save();
    QPen TextPen(Settings::s_TextColor);
    painter.setPen(TextPen);
    painter.drawText(5,10, QString(tr("X-Scale = %1")).arg(m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,22, QString(tr("Y-Scale = %1")).arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,34, QString(tr("x  = %1")).arg(m_MousePos.x,7,'f',4));
    painter.drawText(5,46, QString(tr("y  = %1")).arg(m_MousePos.y,7,'f',4));
    painter.restore();
}



void Section2dWidget::drawBackImage(QPainter &painter)
{
    if(m_bIsImageLoaded && !m_BackImage.isNull())
    {
        painter.save();

        double xscale = m_fScale          /m_fRefScale;
        double yscale = m_fScale*m_fScaleY/m_fRefScale;

        //zoom from the center of the viewport
        QPoint VCenter = rect().center();

        //draw the background image in the viewport
        int w = (int)((double)m_BackImage.width()* xscale);
        int h = (int)((double)m_BackImage.height()* yscale);
        //the coordinates of the top left corner are measured from the center of the viewport
        double xtop = VCenter.x() + m_ViewportTrans.x() - (double)m_BackImage.width()  /2.*xscale;
        double ytop = VCenter.y() + m_ViewportTrans.y() - (double)m_BackImage.height() /2.*yscale;

        painter.drawPixmap(xtop, ytop, w, h, m_BackImage);
        painter.restore();
    }
}





/**
 * The user has requested to zoom in on the display by drawing a rectangle on the screen.
 */
void Section2dWidget::onZoomIn()
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
void Section2dWidget::onZoomYOnly()
{
    m_bZoomYOnly = !m_bZoomYOnly;
    //	pMainFrame->zoomYAct->setChecked(m_bZoomYOnly);

}


/**
 * The user has requested to zoom out.
 */
void Section2dWidget::onZoomLess()
{
    // can't do two things at the same time can we ?
    releaseZoom();

    double ZoomFactor = 0.8;
    double newScale = qMax(ZoomFactor*m_fScale, m_fRefScale);

    ZoomFactor = qMax(ZoomFactor, newScale/m_fScale);

    m_fScale = ZoomFactor*m_fScale;
    int a = rect().center().x();
    int b = rect().center().y();

    //scale
    m_ptOffset.rx() = (int)(ZoomFactor*(m_ptOffset.x()-a)+a);
    m_ptOffset.ry() = (int)(ZoomFactor*(m_ptOffset.y()-b)+b);

    update();
}



