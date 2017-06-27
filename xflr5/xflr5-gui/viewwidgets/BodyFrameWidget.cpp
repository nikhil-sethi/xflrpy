/****************************************************************************

	BodyFrameWidget Class
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

#include "Settings.h"
#include <miarex/design/BodyScaleDlg.h>
#include "BodyFrameWidget.h"
#include <misc/Units.h>
#include <QPainter>
#include <QtDebug>

bool BodyFrameWidget::s_bCurFrameOnly = false;

BodyFrameWidget::BodyFrameWidget(QWidget *pParent, Body *pBody)
	:Section2dWidget(pParent)
{
	m_pBody = pBody;

	m_pShowCurFrameOnly = NULL;
	createActions();
	createContextMenu();
}


void BodyFrameWidget::setScale()
{
	if(!m_pBody)
	{
		//scale is set by user zooming
		m_fRefScale = (double)rect().width();
		m_fScale = m_fRefScale;
	}
	else
	{
		m_fRefScale = ((double)rect().width())/(m_pBody->length()/15.0);
		m_fScale = m_fRefScale;

	}

	m_ptOffset.rx() = rect().width()/2;
	m_ptOffset.ry() = rect().height()/2;

	m_ViewportTrans = QPoint(0,0);
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void BodyFrameWidget::resizeEvent (QResizeEvent *event)
{
	Q_UNUSED(event);
	setScale();
}



void BodyFrameWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.save();
	painter.fillRect(rect(), Settings::s_BackgroundColor);

	drawScaleLegend(painter);
	drawBackImage(painter);

	paintGrids(painter);
	drawFrameLines();
	drawFramePoints();

	painter.restore();
}




void BodyFrameWidget::drawFrameLines()
{
	if(!m_pBody) return;

	int k;
	Vector3d Point;
	double hinc, u, v;
	int nh;

	QPainter painter(this);
	painter.save();
	nh = 23;
//	xinc = 0.1;
	hinc = 1.0/(double)(nh-1);

	QPen framePen(m_pBody->bodyColor());
	framePen.setWidth(2);
	painter.setPen(framePen);

	QPolygonF rightPolyline, leftPolyline;

	if(m_pBody->m_LineType ==XFLR5::BODYSPLINETYPE)
	{
		if(m_pBody->activeFrame())
		{
			u = m_pBody->getu(m_pBody->activeFrame()->m_Position.x);

			v = 0.0;
			for (k=0; k<nh; k++)
			{
				m_pBody->getPoint(u,v,true, Point);
				rightPolyline.append(QPointF(Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
				leftPolyline.append(QPointF(-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
				v += hinc;
			}
		}
	}
	else
	{
		Frame *pFrame = m_pBody->activeFrame();
		if(pFrame)
		{
			for (k=0; k<m_pBody->sideLineCount();k++)
			{
				rightPolyline.append(QPointF( pFrame->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), pFrame->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
				leftPolyline.append( QPointF(-pFrame->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), pFrame->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
			}
		}
	}

	painter.drawPolyline(rightPolyline);
	painter.drawPolyline(leftPolyline);

	framePen.setStyle(Qt::DashLine);
	framePen.setWidth(1);
	painter.setPen(framePen);

	if(!s_bCurFrameOnly)
	{
		for(int j=0; j<m_pBody->frameCount(); j++)
		{
			if(m_pBody->frame(j)!=m_pBody->activeFrame())
			{
				rightPolyline.clear();
				leftPolyline.clear();

				if(m_pBody->m_LineType ==XFLR5::BODYSPLINETYPE)
				{
					u = m_pBody->getu(m_pBody->frame(j)->m_Position.x);

					v = 0.0;
					for (k=0; k<nh; k++)
					{
						m_pBody->getPoint(u,v,true, Point);
						rightPolyline.append(QPointF(Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
						leftPolyline.append(QPointF(-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
						v += hinc;
					}

				}
				else
				{
					for (k=0; k<m_pBody->sideLineCount();k++)
					{
						rightPolyline.append(QPointF( m_pBody->frame(j)->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), m_pBody->frame(j)->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
						leftPolyline.append( QPointF(-m_pBody->frame(j)->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), m_pBody->frame(j)->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
					}
				}

				painter.drawPolyline(rightPolyline);
				painter.drawPolyline(leftPolyline);
			}
		}
	}
	painter.restore();
}




void BodyFrameWidget::drawFramePoints()
{
	if(!m_pBody->activeFrame()) return;

	Frame *m_pFrame = m_pBody->activeFrame();
	QPainter painter(this);
	painter.save();

	QPen pointPen(m_pBody->bodyColor());


	for (int k=0; k<m_pFrame->pointCount();k++)
	{
		if(Frame::s_iSelect==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(Qt::red);
		}
		else if(Frame::s_iHighlight==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(m_pBody->bodyColor().lighter());
		}
		else
		{
			pointPen.setWidth(2);
			pointPen.setColor(m_pBody->bodyColor());
		}

		painter.setPen(pointPen);
		QRectF rectF( m_pFrame->m_CtrlPoint[k].y *  m_fScale -3 +m_ptOffset.x(),
					  m_pFrame->m_CtrlPoint[k].z * -m_fScale -3 +m_ptOffset.y(),
					  7,7);
		painter.drawEllipse(rectF);
	}
	painter.restore();
}



void BodyFrameWidget::setBody(Body *pBody)
{
	m_pBody = pBody;
	setScale();
}



void BodyFrameWidget::onInsertPt()
{
	Vector3d real = mousetoReal(m_PointDown);
	real.z = real.y;
	real.y = real.x;
	real.x = m_pBody->activeFrame()->position().x;
	if(m_pBody->activeFrame())
	{
		m_pBody->insertPoint(real);
		emit objectModified();
	}
}


void BodyFrameWidget::onRemovePt()
{
	if(m_pBody->activeFrame())
	{
		Vector3d real = mousetoReal(m_PointDown);
		real.z = real.y;
		real.y = real.x;
		real.x = m_pBody->activeFrame()->position().x;

		int n =   m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale);
		if (n>=0)
		{
			for (int i=0; i<m_pBody->frameCount();i++)
			{
				m_pBody->frame(i)->removePoint(n);
			}
			m_pBody->setNURBSKnots();
			emit objectModified();
		}
	}
}


void BodyFrameWidget::onScaleFrame()
{
	if(!m_pBody) return;

	BodyScaleDlg dlg(this);

	dlg.m_FrameID = m_pBody->m_iActiveFrame;
	dlg.initDialog(true);

	if(dlg.exec()==QDialog::Accepted)
	{
		m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
		emit objectModified();

	}
}


int BodyFrameWidget::highlightPoint(Vector3d real)
{
	if(!m_pBody->activeFrame()) Frame::s_iHighlight = -1;
	else
	{
		real.z = real.y;
		real.y = real.x;
		real.x = m_pBody->activeFrame()->position().x;
		Frame::s_iHighlight = m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale);
	}
	return Frame::s_iHighlight;
}



int BodyFrameWidget::selectPoint(Vector3d real)
{
	if(!m_pBody->activeFrame()) Frame::s_iSelect = -1;
	else
	{
		real.z = real.y;
		real.y = real.x;
		real.x = m_pBody->activeFrame()->position().x;
		Frame::s_iSelect = m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale);
	}
	emit pointSelChanged();
	return Frame::s_iSelect;
}



void BodyFrameWidget::dragSelectedPoint(double x, double y)
{
	if (!m_pBody->activeFrame() || (Frame::s_iSelect<0) ||  (Frame::s_iSelect > m_pBody->activeFrame()->pointCount())) return;

	m_pBody->activeFrame()->selectedPoint().set(m_pBody->activeFrame()->position().x, qMax(x,0.0), y);
}


void BodyFrameWidget::drawScaleLegend(QPainter &painter)
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



void BodyFrameWidget::createActions()
{
	m_ActionList.clear();

	QAction *pScaleBody = new QAction(tr("Scale Frame"), this);
	connect(pScaleBody,  SIGNAL(triggered()), this, SLOT(onScaleFrame()));
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

	m_pShowCurFrameOnly = new QAction(tr("Show Current Frame Only"), this);
	m_pShowCurFrameOnly->setCheckable(true);
	connect(m_pShowCurFrameOnly, SIGNAL(triggered()), this, SLOT(onShowCurFrameOnly()));
	m_ActionList.append(m_pShowCurFrameOnly);

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



void BodyFrameWidget::onShowCurFrameOnly()
{
	s_bCurFrameOnly = !s_bCurFrameOnly;
	m_pShowCurFrameOnly->setChecked(s_bCurFrameOnly);
	update();
}








