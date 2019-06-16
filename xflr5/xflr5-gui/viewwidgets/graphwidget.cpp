/****************************************************************************

	GraphWidget Class
		Copyright (C) 2008-2017 Andre Deperrois 

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
#include <misc/options/displayoptions.h>
#include "graphwidget.h"
#include "graphtilewidget.h"
#include <graph/graphdlg.h>
#include <objects/objects2d/Polar.h>
#include <objects/objects3d/WPolar.h>
#include <miarex/Miarex.h>
#include <misc/options/units.h>
#include <QPen>
#include <QPainterPath>
#include <QPainter>
#include <QPaintEvent>
#include <QStatusBar>
#include <QtDebug>



void *GraphWidget::s_pMainFrame = nullptr;

GraphWidget::GraphWidget(QWidget *pParent) : QWidget(pParent)
{
	setMouseTracking(true);

	m_pParent = pParent;

	m_TitlePosition.setX(0);
	m_TitlePosition.setY(0);
	m_bDrawLegend = false;
	m_GraphTitle = "";

	m_bResetCurves = true;
	m_bTransGraph = false;
	m_bXPressed = m_bYPressed = false;

	m_pGraph = nullptr;
	setLegendPosition(QPoint(20,20));
}

void GraphWidget::setGraph(Graph *pGraph)
{
	m_pGraph = pGraph;
	if(m_pGraph) m_pGraph->setDrawRect(rect());
}

void GraphWidget::setTitles(QString &Title, QPoint &Place)
{
	m_GraphTitle = Title;
	m_TitlePosition = Place;
}



void GraphWidget::paintEvent(QPaintEvent *  event )
{
	QPainter painter(this);
	painter.save();

	QBrush BackBrush(m_pGraph->backgroundColor());
	painter.fillRect(event->rect(), BackBrush);
	if(!m_pGraph)
	{
		painter.restore();
		return;
	}

	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBackground(BackBrush);

	m_pGraph->drawGraph(painter);

	if(m_bDrawLegend) m_pGraph->drawLegend(painter, m_LegendOrigin, Settings::textFont(), Settings::textColor(), Settings::backgroundColor());
	if(hasFocus() && MainFrame::s_bShowMousePos)
	{
		QPen textPen(Settings::textColor());
		QFontMetrics fm(Settings::textFont());
		painter.setBackgroundMode(Qt::TransparentMode);

		int fmheight  = fm.height();

        painter.setFont(Settings::textFont());
		painter.setPen(textPen);
		painter.drawText(width()-14*fm.averageCharWidth(),fmheight, QString("x = %1").arg(m_pGraph->clientTox(m_LastPoint.x()),9,'f',3));
		painter.drawText(width()-14*fm.averageCharWidth(),2*fmheight, QString("y = %1").arg(m_pGraph->clientToy(m_LastPoint.y()),9,'f',3));
	}
	painter.restore();
}



void GraphWidget::resizeEvent ( QResizeEvent * event )
{
	QRect r = rect();
	if(m_pGraph) m_pGraph->setDrawRect(r);

	if(m_pGraph)
	{
		m_pGraph->initializeGraph();
		emit graphResized(m_pGraph);
	}
	event->accept();
}



void GraphWidget::showLegend(bool bShow)
{
	m_bDrawLegend = bShow;
}



void GraphWidget::setLegendPosition(QPoint pos)
{
	m_LegendOrigin = pos;
}



void GraphWidget::contextMenuEvent (QContextMenuEvent *event)
{
	event->ignore();
}


void GraphWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_R:
		{
			onResetGraphScales();
			event->accept();
			break;
		}
		case Qt::Key_V:
		{
			GraphDlg::setActivePage(0);
			onGraphSettings();
			event->accept();
			break;
		}
		case Qt::Key_G:
		{
			onGraphSettings();
			event->accept();
			break;
		}

		default:event->ignore();
	}
}




void GraphWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::CrossCursor);
	onGraphSettings();
}


void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
	bool bCtrl;
	QPoint point;
	double xu, yu, x1, y1, xmin, xmax, ymin, ymax;

	if(!m_pGraph) return;

	setFocus();

	point = event->pos();

	bCtrl = false;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

	if(!rect().contains(event->pos()))
	{
		m_bTransGraph = false;
		return;
	}

	if ((event->buttons() & Qt::LeftButton) && m_bTransGraph)
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

	else if ((event->buttons() & Qt::MidButton) && !bCtrl)
	//scaling
	{
		//zoom graph
		m_pGraph->setAuto(false);
		if(point.y()-m_LastPoint.y()<0) m_pGraph->scaleAxes(1.02);
		else                            m_pGraph->scaleAxes(1.0/1.02);

		update();
	}
	// we zoom the graph or the foil
	else if ((event->buttons() & Qt::MidButton) || event->modifiers().testFlag(Qt::AltModifier))
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
//		MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
//		pMainFrame->statusBar()->showMessage(QString("X =%1, Y = %2").arg(m_pGraph->clientTox(event->x())).arg(m_pGraph->clientToy(event->y())));
		update();
	}
	else
	{
//		update();
	}

	m_LastPoint = point;
}


void GraphWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		QPoint point = event->pos();

		m_LastPoint.rx() = point.x();
		m_LastPoint.ry() = point.y();

		m_bTransGraph = true;
		setCursor(Qt::ClosedHandCursor);

		m_LastPoint = point;
	}
}


void GraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
	setCursor(Qt::CrossCursor);
	m_bTransGraph = false;

	event->accept();
}


void GraphWidget::wheelEvent (QWheelEvent *event)
{
	double zoomFactor=1.0;

	QPoint pt(event->x(), event->y()); //client coordinates

	if(event->delta()>0)
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
		else                          zoomFactor = 1.06;
	}
	else
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
		else                          zoomFactor = 1./1.06;
	}

	if(rect().contains(pt))
	{
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
}

/**
 * The user has requested the reset of the active graph's scales to their default value
 */
void GraphWidget::onResetGraphScales()
{
	m_pGraph->setAuto(true);
	update();
}



/**
 * The user has requested an edition of the settings of the active graph
 */
void GraphWidget::onGraphSettings()
{
	GraphDlg grDlg(this);
	grDlg.setGraph(m_pGraph);

//	QAction *action = qobject_cast<QAction *>(sender());
//	grDlg.setActivePage(0);

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
					m_pGraph->setAutoYMinUnit(true);
				}
				break;
			}
			case GRAPH::POPPGRAPH:
			{
				if(grDlg.bVariableChanged())
				{
					m_pGraph->setAutoY(true);
					m_pGraph->setAutoYMinUnit(true);
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
					m_pGraph->setAutoYMinUnit(true);
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

	m_bResetCurves = true;
	update();
}






