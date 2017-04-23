/****************************************************************************

	BodyLineWidget Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#include "globals.h"
#include "BodyLineWidget.h"
#include <miarex/design/BodyScaleDlg.h>
#include <misc/Settings.h>
#include <misc/Units.h>
#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>


BodyLineWidget::BodyLineWidget(QWidget *pParent, Body *pBody)
		:Section2dWidget(pParent)
{
	m_pBody = pBody;
	createActions();
	createContextMenu();

}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void BodyLineWidget::resizeEvent (QResizeEvent *event)
{
	setScale();
	event->accept();
}




void BodyLineWidget::setScale()
{
	if(!m_pBody)
	{
		//scale is set by user zooming
		m_fRefScale = (double)rect().width()-150.0;
		m_fScale = m_fRefScale;
	}
	else
	{
		m_fRefScale = ((double)rect().width()-150.0)/m_pBody->length();
		m_fScale = m_fRefScale;

	}

	m_ptOffset.rx() = rect().width()/2;
	m_ptOffset.ry() = rect().height()/2;

	m_ViewportTrans = QPoint(0,0);
}


void BodyLineWidget::drawBodyLines()
{
	if(!m_pBody) return;
	QPainter painter(this);
	painter.save();
	int i,k;
	double zpos;


	QPen linePen(m_pBody->m_BodyColor);
	linePen.setWidth(1);
	painter.setPen(linePen);
	linePen.setStyle(Qt::DashLine);

	QPolygonF midLine,topLine, botLine;
	//Middle Line
	for (k=0; k<m_pBody->frameCount();k++)
	{
		zpos = (m_pBody->frame(k)->m_CtrlPoint.first().z +m_pBody->frame(k)->m_CtrlPoint.last().z )/2.0;
		midLine.append(QPointF(m_pBody->frame(k)->m_Position.x*m_fScale + m_ptOffset.x(), zpos*-m_fScale + m_ptOffset.y()));
	}

	if(m_pBody->m_LineType==XFLR5::BODYPANELTYPE)
	{
		//Top Line
		for (k=0; k<m_pBody->frameCount();k++)
		{
			topLine.append(QPointF(m_pBody->frame(k)->m_Position.x*m_fScale + m_ptOffset.x(), m_pBody->frame(k)->m_CtrlPoint[0].z*-m_fScale+ m_ptOffset.y()));
		}

		//Bottom Line
		for (k=0; k<m_pBody->frameCount();k++)
		{
			botLine.append(QPointF(m_pBody->frame(k)->m_Position.x*m_fScale + m_ptOffset.x(),
								   m_pBody->frame(k)->m_CtrlPoint[ m_pBody->frame(k)->pointCount()-1].z*-m_fScale + m_ptOffset.y()));
		}
	}
	else
	{
		Vector3d Point;
		double xinc, u, v;

		int nh = 50;
		xinc = 1./(double)(nh-1);
		u=0.0; v = 0.0;

		//top line
		u=0.0;
		v = 0.0;
		for (i=0; i<=nh; i++)
		{
			m_pBody->getPoint(u,v,true, Point);
			topLine.append(QPointF(Point.x*m_fScale + m_ptOffset.x(), Point.z*-m_fScale + m_ptOffset.y()));
			u += xinc;
		}

		//bottom line
		u=0.0;
		v = 1.0;
		for (i=0; i<=nh; i++)
		{
			m_pBody->getPoint(u,v,true, Point);
			botLine.append(QPointF(Point.x*m_fScale + m_ptOffset.x(), Point.z*-m_fScale + m_ptOffset.y()));
			u += xinc;
		}
	}

	QRect r(rect());
	painter.drawPolyline(midLine);
	painter.drawPolyline(topLine);
	painter.drawPolyline(botLine);

	painter.restore();
}


void BodyLineWidget::drawBodyPoints()
{
	if(!m_pBody) return;

	QPainter painter(this);
	painter.save();
	QPen pointPen;

	for (int k=0; k<m_pBody->frameCount();k++)
	{
		if(m_pBody->m_iActiveFrame==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(Qt::red);
		}
		else if(m_pBody->m_iHighlightFrame==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(m_pBody->m_BodyColor.lighter());
		}
		else
		{
			pointPen.setColor(m_pBody->m_BodyColor);
			pointPen.setWidth(2);
		}

		painter.setPen(pointPen);
		QRectF rectF( m_pBody->frame(k)->m_Position.x * m_fScale - 3 + m_ptOffset.x(),
					 (m_pBody->frame(k)->m_CtrlPoint.first().z + m_pBody->frame(k)->m_CtrlPoint.last().z ) /2.0* -m_fScale -3+ m_ptOffset.y(),
					  7,7);
		painter.drawEllipse(rectF);

	}
	painter.restore();
}


void BodyLineWidget::setBody(Body *pBody)
{
	m_pBody = pBody;
	setScale();
}


void BodyLineWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.save();
	painter.fillRect(rect(), Settings::s_BackgroundColor);

	drawScaleLegend(painter);
	drawBackImage(painter);

	paintGrids(painter);
	drawBodyLines();
	drawBodyPoints();

	painter.restore();
}



void BodyLineWidget::onScaleBody()
{
	if(!m_pBody) return;

	BodyScaleDlg dlg(this);

	dlg.m_FrameID = m_pBody->m_iActiveFrame;
	dlg.initDialog(false);

	if(dlg.exec()==QDialog::Accepted)
	{
		m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
		emit objectModified();
	}
}


void BodyLineWidget::onInsertPt()
{
	Vector3d Real = mousetoReal(m_PointDown);
	m_pBody->insertFrame(Real);
	emit objectModified();
}


void BodyLineWidget::onRemovePt()
{
	Vector3d Real = mousetoReal(m_PointDown);

	int n =  m_pBody->isFramePos(Vector3d(Real.x, 0.0, Real.y), m_fScale/m_fRefScale);
	if (n>=0)
	{
		n = m_pBody->removeFrame(n);
		emit objectModified();
	}
}


int BodyLineWidget::highlightPoint(Vector3d real)
{
	m_pBody->m_iHighlightFrame = m_pBody->isFramePos(Vector3d(real.x, 0.0, real.y), m_fScale/m_fRefScale);
	return m_pBody->m_iHighlightFrame;
}



int BodyLineWidget::selectPoint(Vector3d real)
{
	m_pBody->m_iActiveFrame = m_pBody->isFramePos(Vector3d(real.x, 0.0, real.y), m_fScale/m_fRefScale);
	m_pBody->setActiveFrame(m_pBody->m_iActiveFrame);
	emit frameSelChanged();
	return m_pBody->m_iActiveFrame;
}


void BodyLineWidget::dragSelectedPoint(double x, double y)
{
	if(!m_pBody->activeFrame())return;
	m_pBody->activeFrame()->setPosition(Vector3d(x,0,y));
}


void BodyLineWidget::drawScaleLegend(QPainter &painter)
{
	painter.save();
	QPen TextPen(Settings::s_TextColor);
	painter.setPen(TextPen);
	painter.drawText(5,10, QString("X-Scale = %1").arg(m_fScale/m_fRefScale,4,'f',1));
	painter.drawText(5,22, QString("Y-Scale = %1").arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
	painter.drawText(5,34, QString("x  = %1").arg(m_MousePos.x * Units::mtoUnit(),7,'f',2) + Units::lengthUnitLabel());
	painter.drawText(5,46, QString("y  = %1").arg(m_MousePos.y * Units::mtoUnit(),7,'f',2) + Units::lengthUnitLabel());
	painter.restore();
}



void BodyLineWidget::createActions()
{
	m_ActionList.clear();


	QAction *pScaleBody = new QAction(tr("Scale Body"), this);
	connect(pScaleBody,  SIGNAL(triggered()), this, SLOT(onScaleBody()));
	m_ActionList.append(pScaleBody);

	QAction *pSeparator0 = new QAction(this);
	pSeparator0->setSeparator(true);
	m_ActionList.append(pSeparator0);

	QAction *pInsertPt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
	connect(pInsertPt, SIGNAL(triggered()), this, SLOT(onInsertPt()));
	m_ActionList.append(pInsertPt);

	QAction *pRemovePt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
	connect(pRemovePt, SIGNAL(triggered()), this, SLOT(onRemovePt()));
	m_ActionList.append(pRemovePt);

	QAction *pSeparator1 = new QAction(this);
	pSeparator1->setSeparator(true);
	m_ActionList.append(pSeparator1);

	QAction *pResetScaleAction = new QAction(tr("Reset Scales"), this);
	connect(pResetScaleAction, SIGNAL(triggered()), this, SLOT(onResetScales()));
	m_ActionList.append(pResetScaleAction);

	QAction *pGridSettingsAction = new QAction(tr("Grid Settings"), this);
	connect(pGridSettingsAction, SIGNAL(triggered()), this, SLOT(onGridSettings()));
	m_ActionList.append(pGridSettingsAction);

	QAction *pSeparator2 = new QAction(this);
	pSeparator2->setSeparator(true);
	m_ActionList.append(pSeparator2);

	QAction *pLoadBackImage = new QAction(tr("Load background image")   +"\tCtrl+Shift+I", this);
	connect(pLoadBackImage, SIGNAL(triggered()), this, SLOT(onLoadBackImage()));
	m_ActionList.append(pLoadBackImage);

	QAction *pClearBackImage = new QAction(tr("Clear background image") +"\tCtrl+Shift+I", this);
	connect(pClearBackImage, SIGNAL(triggered()), this, SLOT(onClearBackImage()));
	m_ActionList.append(pClearBackImage);
}






