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

/**
*The public constructor
*/
inverseviewwt::inverseviewwt(QWidget *parent)
    : QWidget(parent)
{
    m_pMainFrame = nullptr;
    m_pXInverse = nullptr;

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
void inverseviewwt::keyPressEvent(QKeyEvent *event)
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->keyPressEvent(event);
    }
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::keyReleaseEvent(QKeyEvent *event)
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->keyReleaseEvent(event);
    }
}



/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::mousePressEvent(QMouseEvent *event)
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->mousePressEvent(event);
    }
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->mouseReleaseEvent(event);
    }
}


/**
*Overrides the mouseMoveEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->mouseMoveEvent(event);
    }
}



/**
*Overrides the mouseDoubleClickEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->doubleClickEvent(event->pos());
    }
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::resizeEvent(QResizeEvent *event)
{
    if(m_pXInverse)
    {
        m_pXInverse->setXInverseScale(rect());
    }
    event->accept();
}


/**
*Overrides the wheelEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::wheelEvent(QWheelEvent *pEvent)
{
    double ZoomFactor=1.0;

    if(pEvent->angleDelta().y()>0)
    {
        if(!DisplayOptions::bReverseZoom()) ZoomFactor = 1./1.06;
        else                          ZoomFactor = 1.06;
    }
    else
    {
        if(!DisplayOptions::bReverseZoom()) ZoomFactor = 1.06;
        else                          ZoomFactor = 1./1.06;
    }

    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        QPoint pt;
#if QT_VERSION >= 0x050F00
        pt = pEvent->position().toPoint();
#else
        pt = pEvent->pos();
#endif
        m_pXInverse->zoomEvent(pt, ZoomFactor);
    }
}


/**
*Overrides the paintEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void inverseviewwt::paintEvent(QPaintEvent *event)
{
    if(m_pMainFrame->m_iApp == xfl::INVERSEDESIGN && m_pXInverse)
    {
        QPainter painter(this);
        m_pXInverse->paintView(painter);
    }
    else
    {
        QPainter painter(this);
        painter.fillRect(rect(), DisplayOptions::backgroundColor());
    }
    event->accept();
}


/**
* Overrides the contextMenuEvent function of the base class.
* Dispatches the handling to the active child application.
*/
void inverseviewwt::contextMenuEvent (QContextMenuEvent * event)
{
    QPoint ScreenPt = event->globalPos();

    switch(m_pMainFrame->m_iApp)
    {
        case xfl::INVERSEDESIGN:
        {
            m_pMainFrame->m_pInverseContextMenu->exec(ScreenPt);
            break;
        }
        default:
            break;
    }
}
