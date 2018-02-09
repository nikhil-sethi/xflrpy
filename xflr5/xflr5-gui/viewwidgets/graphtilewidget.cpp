/****************************************************************************

	GraphTileWidget Class
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
#include <QtDebug>
#include <QMenu>
#include <QSplitter>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QContextMenuEvent>
#include "graphtilewidget.h"
#include <GraphDlg.h>
#include <mainframe.h>
#include "miarex/Miarex.h"
#include "xdirect/XDirect.h"
#include "xinverse/XInverse.h"
#include <misc/options/displayoptions.h>
#include <misc/options/Units.h>


void* GraphTileWidget::s_pMainFrame = NULL;
void* GraphTileWidget::s_pMiarex = NULL;
void* GraphTileWidget::s_pXDirect = NULL;



GraphTileWidget::GraphTileWidget(QWidget *parent) : QWidget(parent)
{
//	setMouseTracking(true);
	m_pLegendWidget = NULL;
	m_pMainSplitter = NULL;

	m_nGraphWidgets = 0;
	m_iActiveGraphWidget = -1;

	m_xflr5App = XFLR5::NOAPP;
	m_MiarexView = XFLR5::WPOLARVIEW;

	m_iPOppIndex = m_iWPolarIndex = 0;
	m_iStabPolarIndex = m_iStabTimeIndex = 0;

	m_SingleGraphOrientation = Qt::Horizontal;
}


GraphTileWidget::~GraphTileWidget()
{

}


QGraph *GraphTileWidget::graph(int iGraph)
{
	if(iGraph<0 || iGraph>=m_GraphWidget.count()) return NULL;

	return m_GraphWidget.at(iGraph)->graph();
}


GraphWidget *GraphTileWidget::graphWidget(int iGraph)
{
	if(iGraph<0 || iGraph>=m_GraphWidget.count()) return NULL;

	return m_GraphWidget.at(iGraph);
}



GraphWidget *GraphTileWidget::graphWidget(QGraph *pGraph)
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		if(m_GraphWidget.at(igw)->graph()==pGraph) return m_GraphWidget[igw];
	}
	return NULL;
}


void GraphTileWidget::setGraphList(QList<QGraph*>pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation)
{
	MainFrame*pMainFrame = (MainFrame*)s_pMainFrame;
	m_xflr5App = pMainFrame->xflr5App();
	m_nGraphWidgets = qMin(nGraphs,MAXGRAPHS);
	m_iActiveGraphWidget = iGraphWidget;

	for(int ig=0; ig<qMin(MAXGRAPHS, pGraphList.count()); ig++)
		m_GraphWidget.at(ig)->setGraph(pGraphList.at(ig));

	m_pLegendWidget->setGraph(pGraphList.at(0));
	m_SingleGraphOrientation = orientation;

	adjustLayout();

	update();
}




void GraphTileWidget::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::CrossCursor);
}


void GraphTileWidget::contextMenuEvent (QContextMenuEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	QMenu * pGraphMenu = pMainFrame->m_pWPlrCtxMenu;

	//get the main menu
	if(m_xflr5App==XFLR5::MIAREX)
	{
		switch(m_MiarexView)
		{
			case XFLR5::WPOLARVIEW:
				pGraphMenu = pMainFrame->m_pWPlrCtxMenu;
				break;
			case XFLR5::WOPPVIEW:
				pGraphMenu = pMainFrame->m_pWOppCtxMenu;
				break;
			case XFLR5::WCPVIEW:
				pGraphMenu = pMainFrame->m_pWCpCtxMenu;
				break;
			case XFLR5::STABTIMEVIEW:
				pGraphMenu = pMainFrame->m_pWTimeCtxMenu;
				break;
			case XFLR5::STABPOLARVIEW:
				pGraphMenu = pMainFrame->m_pWPlrCtxMenu;
				break;
			default:
				break;
		}
	}
	else
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;
		if(pXDirect->bPolarView())
			pGraphMenu = pMainFrame->m_pOperPolarCtxMenu;
		else
			pGraphMenu = pMainFrame->m_pOperFoilCtxMenu;
	}

	//execute the menu
	pGraphMenu->exec(event->globalPos());
}


void GraphTileWidget::keyPressEvent(QKeyEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
//	bool bShift = false;
	bool bCtrl  = false;
//	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;
	switch (event->key())
	{
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		{
			if(bCtrl)
			{
				event->ignore();
				return;
			}
			else
			{
				int iGraph = event->text().toInt()-1;

				if(m_xflr5App==XFLR5::XFOILANALYSIS)
				{
					QXDirect *pXDirect = (QXDirect*)s_pXDirect;
					pXDirect->setView(XFLR5::ONEGRAPH);
				}
				else if(m_xflr5App==XFLR5::MIAREX)
				{
					QMiarex *pMiarex = (QMiarex*)s_pMiarex;
					if (pMiarex->m_iView==XFLR5::WOPPVIEW)   m_iPOppIndex = iGraph;
					if (pMiarex->m_iView==XFLR5::WPOLARVIEW) m_iWPolarIndex = iGraph;
					if (pMiarex->m_iView==XFLR5::WCPVIEW && iGraph>0)	return;
					if (pMiarex->m_iView==XFLR5::STABPOLARVIEW)
					{
						if(iGraph>1) return;
						else m_iStabPolarIndex = iGraph;
						pMiarex->m_bLongitudinal = (iGraph==0);
					}
					if (pMiarex->m_iView==XFLR5::STABTIMEVIEW)
					{
						if(iGraph>3) return;
						else m_iStabTimeIndex = iGraph;
					}
					pMiarex->setView(XFLR5::ONEGRAPH);
				}
				m_nGraphWidgets = 1;
				if(iGraph>=0 && iGraph<m_GraphWidget.count())
				{
					m_iActiveGraphWidget = iGraph;
					m_pLegendWidget->setGraph(m_GraphWidget.at(iGraph)->graph());
					adjustLayout();
				}

				pMainFrame->checkGraphActions();
				update();
				setFocus();
				return;
			}
		}
		case Qt::Key_T:
		{
			onTwoGraphs();
			return;
		}
		case Qt::Key_F:
		{
			onFourGraphs();
			return;
		}
		case Qt::Key_A:
		{
			onAllGraphs();
			return;
		}
		default:event->ignore();
	}
}



void GraphTileWidget::onResetCurves(QGraph *pGraph)
{
	if(!pGraph) return;
	switch(m_xflr5App)
	{
		case XFLR5::XFOILANALYSIS:
		{
			QXDirect *pXDirect = (QXDirect*)s_pXDirect;
			pXDirect->updateView();
		}
		case XFLR5::INVERSEDESIGN:
		{
		}
		case XFLR5::MIAREX:
		{
			QMiarex::s_bResetCurves = true;
			QMiarex *pMiarex = (QMiarex*)s_pMiarex;
			pMiarex->updateView();
		}
		default:
			break;
	}
}



void GraphTileWidget::onSingleGraph()
{
	if(!isVisible()) return;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	QAction *pAction = qobject_cast<QAction *>(sender());
	if (!pAction) return;
	int iGraph = pAction->data().toInt();


	if(iGraph>=m_GraphWidget.count())
	{
		pMainFrame->checkGraphActions();
		return;
	}


	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;
		m_nGraphWidgets = 1;
		m_iActiveGraphWidget = iGraph;
		m_pLegendWidget->setGraph(m_GraphWidget.at(iGraph)->graph());
		pXDirect->setView(XFLR5::ONEGRAPH);
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;
		if (pMiarex->m_iView==XFLR5::WCPVIEW && iGraph>0) return;
		if (pMiarex->m_iView==XFLR5::STABPOLARVIEW)
		{
			if(iGraph>1) return;
			m_iStabPolarIndex = iGraph;
			pMiarex->m_bLongitudinal = (iGraph==0);
		}
		if (pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			if(iGraph>3) return;
			m_iStabTimeIndex = iGraph;
		}
		if(m_MiarexView==XFLR5::WOPPVIEW)        m_iPOppIndex = iGraph;
		if(m_MiarexView==XFLR5::WPOLARVIEW)      m_iWPolarIndex = iGraph;
		m_nGraphWidgets = 1;
		m_iActiveGraphWidget = iGraph;
		m_pLegendWidget->setGraph(m_GraphWidget.at(iGraph)->graph());
		pMiarex->setView(XFLR5::ONEGRAPH);
	}

	adjustLayout();
	pMainFrame->checkGraphActions();
	update();
	setFocus();
}


void GraphTileWidget::onTwoGraphs()
{
	if(!isVisible()) return;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;
		pXDirect->setView(XFLR5::TWOGRAPHS);
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;
		if(pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			onSingleGraph();
			return;
		}
		pMiarex->setView(XFLR5::TWOGRAPHS);
	}

	m_nGraphWidgets = 2;
	m_iActiveGraphWidget = 0;
	m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());

	adjustLayout();
	pMainFrame->checkGraphActions();
	update();
	setFocus();
}


void GraphTileWidget::onFourGraphs()
{
	if(!isVisible()) return;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;
		m_nGraphWidgets = 4;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
		pXDirect->setView(XFLR5::FOURGRAPHS);
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;
		if (pMiarex->m_iView==XFLR5::STABPOLARVIEW)
		{
			onTwoGraphs(); //there are only two graphs to display
			return;
		}
		else if(pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			onSingleGraph();
			return;
		}
		m_nGraphWidgets = 4;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
		pMiarex->setView(XFLR5::FOURGRAPHS);
	}

	adjustLayout();
	pMainFrame->checkGraphActions();
	update();
	setFocus();
}


void GraphTileWidget::onAllGraphs()
{
	if(!isVisible()) return;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;
		pXDirect->setView(XFLR5::ALLGRAPHS);
		m_nGraphWidgets = 6;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;

		if (pMiarex->m_iView==XFLR5::STABPOLARVIEW)
		{
			onTwoGraphs(); //there are only two graphs to display
			return;
		}
		else if (pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			onFourGraphs(); //there are only two graphs to display
			return;
		}
		else if(pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			onSingleGraph();
			return;
		}

		pMiarex->setView(XFLR5::ALLGRAPHS);

		m_nGraphWidgets = 6;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
	}

	adjustLayout();
	pMainFrame->checkGraphActions();
	update();
	setFocus();
}



void GraphTileWidget::onCurGraphSettings()
{
	if(!isVisible()) return;
	if(activeGraphWidget())
	{
		activeGraphWidget()->onGraphSettings();
	}
	setFocus();

}


void GraphTileWidget::onResetCurGraphScales()
{
	if(!isVisible()) return;
	if(activeGraphWidget())
	{
		activeGraphWidget()->onResetGraphScales();
	}
	setFocus();
}


void GraphTileWidget::onExportCurGraph()
{
	MainFrame*pMainFrame = (MainFrame*)s_pMainFrame;
	if(!isVisible()) return;
	QGraph *pGraph = activeGraph();
	if(!pGraph) return;
	pMainFrame->exportGraph(pGraph);
	setFocus();
}



/**
 * The user has requested an edition of the settings for all WOpp graphs
 * Launches the dialog box and updates the graphs
 */
void GraphTileWidget::onAllGraphSettings()
{
	if(!isVisible()) return;

	GraphDlg grDlg(this);
	grDlg.setActivePage(1);

	if(xflr5App()==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;

		if(pMiarex->m_iView==XFLR5::WPOLARVIEW)    grDlg.setGraph(pMiarex->m_WPlrGraph[0]);
		else if(pMiarex->m_iView==XFLR5::WOPPVIEW) grDlg.setGraph(pMiarex->m_WingGraph[0]);

		if(grDlg.exec() == QDialog::Accepted)
		{
			if(pMiarex->m_iView==XFLR5::WPOLARVIEW)
				for(int ig=1; ig<pMiarex->m_WingGraph.size(); ig++) pMiarex->m_WingGraph[ig]->copySettings(pMiarex->m_WingGraph[0]);
			else if(pMiarex->m_iView==XFLR5::WOPPVIEW)
				for(int ig=1; ig<pMiarex->m_WPlrGraph.size(); ig++) pMiarex->m_WPlrGraph[ig]->copySettings(pMiarex->m_WPlrGraph[0]);
		}
	}
	else if (m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;

		if(!pXDirect->bPolarView()) grDlg.setGraph(pXDirect->CpGraph());
		else                        grDlg.setGraph(pXDirect->PlrGraph(0));

		if(grDlg.exec() == QDialog::Accepted)
		{
			if(pXDirect->bPolarView())
				for(int ig=1; ig<MAXPOLARGRAPHS; ig++) pXDirect->PlrGraph(ig)->copySettings(pXDirect->PlrGraph(0));
		}
	}
	update();
	setFocus();
}





/**
 * Updates the display after the user has requested a reset of the scale of all WOpp graphs
 */
void GraphTileWidget::onAllGraphScales()
{
	if(!isVisible()) return;

	if(xflr5App()==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;
		if(pMiarex->m_iView == XFLR5::WOPPVIEW)
		{
			for(int ig=0; ig<pMiarex->m_WingGraph.size(); ig++)
			{
				pMiarex->m_WingGraph[ig]->setAuto(true);
				pMiarex->m_WingGraph[ig]->resetXLimits();
				pMiarex->m_WingGraph[ig]->resetYLimits();
				pMiarex->m_WingGraph[ig]->setAutoX(false);
				if(pMiarex->m_pCurPlane)
				{
					pMiarex->m_WingGraph[ig]->setXMin( -pMiarex->m_pCurPlane->planformSpan()*Units::mtoUnit());
					pMiarex->m_WingGraph[ig]->setXMax(  pMiarex->m_pCurPlane->planformSpan()*Units::mtoUnit());
				}
			}
		}
		else if(pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			for(int ig=0; ig<pMiarex->m_TimeGraph.size(); ig++)
			{
				pMiarex->m_TimeGraph[ig]->setAuto(true);
				pMiarex->m_TimeGraph[ig]->resetXLimits();
				pMiarex->m_TimeGraph[ig]->resetYLimits();
			}
		}
		else if(pMiarex->m_iView==XFLR5::WPOLARVIEW)
		{
			for(int ig=0; ig<pMiarex->m_WPlrGraph.size(); ig++)
			{
				pMiarex->m_WPlrGraph[ig]->setAuto(true);
				pMiarex->m_WPlrGraph[ig]->resetXLimits();
				pMiarex->m_WPlrGraph[ig]->resetYLimits();
			}
		}
		else if(pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			pMiarex->m_CpGraph.setAuto(true);
			pMiarex->m_CpGraph.resetXLimits();
			pMiarex->m_CpGraph.resetYLimits();
			pMiarex->m_CpGraph.setInverted(true);
		}
	}
	else if (xflr5App()==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)s_pXDirect;
		if(!pXDirect->bPolarView())
		{
			pXDirect->CpGraph()->setAuto(true);
			pXDirect->CpGraph()->resetXLimits();
			pXDirect->CpGraph()->resetYLimits();
		}
		else
		{
			for(int ig=0; ig<pXDirect->PlrGraphSize(); ig++)
			{
				pXDirect->PlrGraph(ig)->setAuto(true);
				pXDirect->PlrGraph(ig)->resetXLimits();
				pXDirect->PlrGraph(ig)->resetYLimits();
			}
		}
	}
	update();
	setFocus();
}




QGraph *GraphTileWidget::activeGraph()
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		if(m_GraphWidget.at(igw)->isVisible()&& m_GraphWidget.at(igw)->hasFocus()) return m_GraphWidget.at(igw)->graph();
	}
	return NULL;
}





GraphWidget *GraphTileWidget::activeGraphWidget()
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		if(m_GraphWidget.at(igw)->isVisible()&& m_GraphWidget.at(igw)->hasFocus()) return m_GraphWidget.at(igw);
	}
	return NULL;
}


















