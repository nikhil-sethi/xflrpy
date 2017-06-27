/****************************************************************************

	InverseViewWidget Class
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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
#include <QMenu>
#include <QtDebug>
#include "mainframe.h"
#include "misc/Settings.h"
#include "QGraph.h"
#include "xinverse/XInverse.h"
#include "inverseviewwidget.h"


/**
*The public constructor
*/
InverseViewWidget::InverseViewWidget(QWidget *parent)
	: QWidget(parent)
{
	m_pMainFrame = NULL;
	m_pXInverse = NULL;

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
void InverseViewWidget::keyPressEvent(QKeyEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyPressEvent(event);
	}
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::keyReleaseEvent(QKeyEvent *event)
{

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyReleaseEvent(event);
	}
}



/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::mousePressEvent(QMouseEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mousePressEvent(event);
	}
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mouseReleaseEvent(event);
	}
}


/**
*Overrides the mouseMoveEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::mouseMoveEvent(QMouseEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mouseMoveEvent(event);
	}
}



/**
*Overrides the mouseDoubleClickEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->doubleClickEvent(event->pos());
	}
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::resizeEvent(QResizeEvent *event)
{
//	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	if(m_pXInverse)
	{
		pXInverse->setXInverseScale(rect());
	}
	event->accept();
}




/**
*Overrides the wheelEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::wheelEvent(QWheelEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	double ZoomFactor=1.0;

	QPoint pt(event->x(), event->y()); //client coordinates

	if(event->delta()>0)
	{
		if(!Settings::s_bReverseZoom) ZoomFactor = 1./1.06;
		else                          ZoomFactor = 1.06;
	}
	else
	{
		if(!Settings::s_bReverseZoom) ZoomFactor = 1.06;
		else                          ZoomFactor = 1./1.06;
	}

	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->zoomEvent(pt, ZoomFactor);
	}
}


/**
*Overrides the paintEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::paintEvent(QPaintEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		QPainter painter(this);
		pXInverse->paintView(painter);
	}
	else
	{
		QPainter painter(this);
		painter.fillRect(rect(), Settings::s_BackgroundColor);
	}
	event->accept();
}


/**
*Overrides the contextMenuEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void InverseViewWidget::contextMenuEvent (QContextMenuEvent * event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QPoint ScreenPt = event->globalPos();

	switch(pMainFrame->m_iApp)
	{
		case XFLR5::INVERSEDESIGN:
		{
			pMainFrame->m_pInverseContextMenu->exec(ScreenPt);
			break;
		}
		default:
			break;
	}
}
