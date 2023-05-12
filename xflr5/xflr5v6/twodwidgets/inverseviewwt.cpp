/****************************************************************************

    inverseviewwt Class
    Copyright (C) 2009-2016 André Deperrois

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


#include <QMenu>
#include <QMouseEvent>

#include <globals/mainframe.h>
#include <xflgraph/graph.h>
#include <xinverse/xinverse.h>
#include <twodwidgets/inverseviewwt.h>
#include <xflcore/displayoptions.h>

MainFrame* InverseViewWt::s_pMainFrame(nullptr);
XInverse* InverseViewWt::s_pXInverse(nullptr);

/**
*The public constructor
*/
InverseViewWt::InverseViewWt(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    QSizePolicy sizepol;
    sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
    sizepol.setVerticalPolicy(QSizePolicy::Expanding);
    setSizePolicy(sizepol);
}


/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::keyPressEvent(QKeyEvent *pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pXInverse->keyPressEvent(pEvent);
    }
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::keyReleaseEvent(QKeyEvent *pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pXInverse->keyReleaseEvent(pEvent);
    }
}


/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::mousePressEvent(QMouseEvent *pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pXInverse->mousePressEvent(pEvent);
    }
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pXInverse->mouseReleaseEvent(pEvent);
    }
}


/**
*Overrides the mouseMoveEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pXInverse->mouseMoveEvent(pEvent);
    }
}


/**
*Overrides the mouseDoubleClickEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::mouseDoubleClickEvent (QMouseEvent * pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pXInverse->doubleClickEvent(pEvent->pos());
    }
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::resizeEvent(QResizeEvent *pEvent)
{
    if(s_pXInverse)
    {
        s_pXInverse->setXInverseScale(rect());
    }
    pEvent->accept();
}


/**
*Overrides the wheelEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::wheelEvent(QWheelEvent *pEvent)
{
    double ZoomFactor=1.0;

    if(pEvent->angleDelta().y()>0)
    {
        if(!DisplayOptions::bReverseZoom()) ZoomFactor = 1./1.06;
        else                                ZoomFactor = 1.06;
    }
    else
    {
        if(!DisplayOptions::bReverseZoom()) ZoomFactor = 1.06;
        else                                ZoomFactor = 1./1.06;
    }

    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        QPoint pt;
#if QT_VERSION >= 0x050F00
        pt = pEvent->position().toPoint();
#else
        pt = pEvent->pos();
#endif
        s_pXInverse->zoomEvent(pt, ZoomFactor);
    }
}


/**
*Overrides the paintEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWt::paintEvent(QPaintEvent *pEvent)
{
    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        QPainter painter(this);
        s_pXInverse->paintView(painter);
    }
    else
    {
        QPainter painter(this);
        painter.fillRect(rect(), DisplayOptions::backgroundColor());
    }
    pEvent->accept();
}


/**
* Overrides the contextMenuEvent function of the base class.
* Dispatches the handling to the active child application.
*/
void InverseViewWt::contextMenuEvent (QContextMenuEvent * pEvent)
{
    QPoint ScreenPt = pEvent->globalPos();

    if(s_pMainFrame->isInverseDesign() && s_pXInverse)
    {
        s_pMainFrame->m_pInverseContextMenu->exec(ScreenPt);
    }
}
