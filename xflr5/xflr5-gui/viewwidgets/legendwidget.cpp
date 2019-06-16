/****************************************************************************

	LegendWidget Class
		Copyright (C) 2015 Andre Deperrois 

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

#include "legendwidget.h"
#include "graphtilewidget.h"
#include <globals/globals.h>
#include <graph_globals.h>
#include <globals/mainframe.h>
#include <objects/objects3d/Plane.h>
#include <objects/objects3d/WPolar.h>
#include <objects/objects3d/PlaneOpp.h>
#include <objects/objects2d/Foil.h>
#include <objects/objects2d/Polar.h>
#include <objects/objects2d/OpPoint.h>
#include <misc/options/displayoptions.h>
#include <miarex/Miarex.h>
#include <miarex/objects3d.h>
#include <xinverse/XInverse.h>
#include <xdirect/XDirect.h>
#include <xdirect/objects2d.h>
#include <graph/curve.h>


MainFrame* LegendWidget::s_pMainFrame = nullptr;
Miarex* LegendWidget::s_pMiarex = nullptr;
XDirect* LegendWidget::s_pXDirect = nullptr;



LegendWidget::LegendWidget(QWidget *pParent) : QWidget(pParent)
{
	setMouseTracking(true);
	m_pParent = pParent;
	m_pGraph = nullptr;
	m_MiarexView = XFLR5::OTHERVIEW;
	m_LegendPosition = QPointF(11.0,11.0);
	m_bTrans = false;
}


LegendWidget::~LegendWidget()
{
}

void LegendWidget::setMiarexView(XFLR5::enumMiarexViews eMiarexView)
{
	m_MiarexView = eMiarexView;
}



void LegendWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.fillRect(rect(), Settings::backgroundColor());


	double bottom = rect().height();

	if(m_pParent)
	{
		GraphTileWidget *pParent = (GraphTileWidget*)m_pParent;
		if(pParent->xflr5App()==XFLR5::MIAREX)
		{
			switch(m_MiarexView)
			{
				case XFLR5::WOPPVIEW:
					drawPOppGraphLegend(painter, m_LegendPosition, bottom);
					break;
				case XFLR5::WPOLARVIEW:
					drawWPolarLegend(painter, m_LegendPosition, bottom);
					break;
				case XFLR5::STABPOLARVIEW:
					drawWPolarLegend(painter, m_LegendPosition, bottom);
					break;
				case XFLR5::STABTIMEVIEW:
					if(m_pGraph) drawStabTimeLegend(painter, m_pGraph, m_LegendPosition, bottom);
					break;
				case XFLR5::WCPVIEW:
					if(m_pGraph) drawCpLegend(painter, m_pGraph, m_LegendPosition, bottom);
					break;
				default: break;
			}
		}
		else //XFLR5::XFOILANALYSIS
		{
			drawPolarLegend(painter, m_LegendPosition, bottom);
		}
	}
}



QSize LegendWidget::sizeHint() const
{
	return QSize(150,150);
}




/**
* Draws the legend of the polar graphs
*@param painter the instance of the QPainter object associated to the active view
*@param the top left postition where the legend is to be drawn
*@param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWidget::drawWPolarLegend(QPainter &painter, QPointF place, int bottom)
{
	painter.save();

    int ny, x1, y1;

    double LegendSize = 30;
    double LegendWidth = 280;

	QFontMetrics fm(Settings::s_TextFont);
    double ypos = fm.height();

	painter.setFont(Settings::s_TextFont);


	QPen TextPen(Settings::s_TextColor);
	painter.setPen(TextPen);
	TextPen.setWidth(1);

	QStringList strPlaneList; // we need to make an inventory of planes which have a visible polar of the desired type
    WPolar * pWPolar=nullptr;
    Plane *pPlane=nullptr;

    for (int j=0; j<Objects3d::s_oaPlane.size(); j++)
	{
        pPlane = Objects3d::s_oaPlane.at(j);
        for (int i=0; i<Objects3d::s_oaWPolar.size(); i++)
		{
            pWPolar = Objects3d::s_oaWPolar.at(i);
            if (pWPolar->planeName()==pPlane->planeName() && pWPolar->isVisible() && !isFiltered(pWPolar))
			{
				if(m_MiarexView==XFLR5::WPOLARVIEW || (m_MiarexView==XFLR5::STABPOLARVIEW && pWPolar->isStabilityPolar()))
				{
					strPlaneList.append(pPlane->planeName());
					break;
				}
			}
		}// finished inventory
	}

#if QT_VERSION >= 0x050000
	strPlaneList.sort(Qt::CaseInsensitive);
#else
	strPlaneList.sort();
#endif

//	painter.setBackgroundMode(Qt::TransparentMode);
	QBrush LegendBrush(Settings::s_BackgroundColor);
	painter.setBrush(LegendBrush);

	QPen LegendPen;
	LegendPen.setWidth(1);

	ny =0;
    for (int k=0; k<strPlaneList.size(); k++)
	{
		int nPlanePlrs = 0;
        for (int l=0; l < Objects3d::s_oaWPolar.size(); l++)
		{
            pWPolar = Objects3d::s_oaWPolar.at(l);

            if (pWPolar->dataSize() && pWPolar->isVisible() && !isFiltered(pWPolar) && pWPolar->planeName()==strPlaneList.at(k))
			{
				if(m_MiarexView==XFLR5::WPOLARVIEW || (m_MiarexView==XFLR5::STABPOLARVIEW && pWPolar->isStabilityPolar()))
					nPlanePlrs++;
			}
		}

		if (nPlanePlrs)
		{
			double YPos = place.y() + (ny+nPlanePlrs+2) * ypos;// bottom line of this Plane's legend
			if(abs(bottom) > abs(YPos))
			{
				ny++;
                painter.drawText(place.x(), place.y() + ypos*ny- double(ypos)/2, strPlaneList.at(k));
			}
			else
			{
				// move rigth if outside screen
				place.rx() += LegendWidth;
				ny=1;
				painter.setPen(TextPen);
                painter.drawText(place.x(), place.y() + ypos*ny-double(ypos)/2, strPlaneList.at(k));
			}

            for (int nc=0; nc<Objects3d::s_oaWPolar.size(); nc++)
			{
                pWPolar = Objects3d::s_oaWPolar.at(nc);
				if(strPlaneList.at(k) == pWPolar->planeName())
				{
					if(!pWPolar->dataSize())
					{
					}
                    else if(!pWPolar->isVisible() || isFiltered(pWPolar))
					{
					}
					else if(m_MiarexView!=XFLR5::WPOLARVIEW && (m_MiarexView!=XFLR5::STABPOLARVIEW || !pWPolar->isStabilityPolar()))
					{
					}
					else
					{
						LegendPen.setColor(color(pWPolar->curveColor()));
						LegendPen.setStyle(getStyle(pWPolar->curveStyle()));
						LegendPen.setWidth(pWPolar->curveWidth());
						painter.setPen(LegendPen);

						painter.drawLine(place.x() + 0.5*LegendSize, place.y() + 1.*ypos*ny,
										 place.x() + 1.5*LegendSize, place.y() + 1.*ypos*ny);

						if(pWPolar->points())
						{
							x1 = place.x() + 1.0*LegendSize;
							y1 = place.y() + 1.*ypos*ny;

							drawPoint(painter, pWPolar->points(), Settings::s_BackgroundColor, QPoint(x1, y1));
						}

						painter.setPen(TextPen);
						painter.drawText(place.x() + 2.0*LegendSize,
										 place.y() + 1.*ypos*ny+ypos/3, pWPolar->polarName());
						ny++ ;
					}
				}
			}
			if(nPlanePlrs) ny++;
		}
	}
	painter.restore();
}



/**
* Draws the curve legend for the graphs in the operating point view
* @param painter the instance of the QPainter object associated to the active view
* @param the top left postition where the legend is to be drawn
* @param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWidget::drawPOppGraphLegend(QPainter &painter, QPointF place, double bottom)
{
	painter.save();

	double LegendSize, LegendWidth;
	double ypos;
	int i, j, k,l, x1, y1, nc, ny;

	ny=0;

//	QString str1, str2, str3, str4, str5, str6;
	LegendSize = 30.0;
	LegendWidth = 300.0;

	QStringList str; // we need to make an inventory of wings
	bool bFound;
	PlaneOpp *pPOpp = nullptr;


    for (i=0; i<Objects3d::s_oaPOpp.size(); i++)
	{
		bFound = false;
        pPOpp = (PlaneOpp*)Objects3d::s_oaPOpp.at(i);
		for (j=0; j<str.size(); j++)
		{
			if (pPOpp->planeName() == str.at(j))	bFound = true;
		}
		if (!bFound)
		{
			str.append(pPOpp->planeName());
		}
	}


	painter.setFont(Settings::s_TextFont);

	QFontMetrics fm(Settings::s_TextFont);
	ypos = fm.height();

	QPen TextPen(Settings::s_TextColor);
	painter.setPen(TextPen);
	TextPen.setWidth(1);

//	painter.setBackgroundMode(Qt::TransparentMode);
	QBrush LegendBrush(Settings::s_BackgroundColor);
	painter.setBrush(LegendBrush);

	QPen LegendPen;
	LegendPen.setWidth(1);

	if(s_pMiarex->curPOppOnly())
	{
		if(!s_pMiarex->curPOpp() || !s_pMiarex->curPOpp()->isVisible())
		{
			painter.restore();
			return;
		}
		ny++ ;

		painter.setPen(TextPen);
		painter.drawText(place.x() + 1.0*LegendSize, place.y() + ypos*ny-ypos/2.0, s_pMiarex->curPOpp()->planeName());

		LegendPen.setColor(color(s_pMiarex->curPOpp()->color()));
		LegendPen.setStyle(getStyle(s_pMiarex->curPOpp()->style()));
		LegendPen.setWidth(s_pMiarex->curPOpp()->width());
		painter.setPen(LegendPen);

		painter.drawLine(place.x() + 1.5*LegendSize, place.y() + 1.*ypos*ny,
						 place.x() + 2.5*LegendSize, place.y() + 1.*ypos*ny);

		if(s_pMiarex->curPOpp()->points())
		{
			x1 = place.x() + 2.0*LegendSize;
			y1 = place.y() + 1.*ypos*ny;
//			painter.drawRect(x1-2, place.y() + 1.*ypos*ny-2, 4, 4);

			drawPoint(painter, s_pMiarex->curPOpp()->points(), Settings::s_BackgroundColor, QPoint(x1,y1));
		}

		painter.setPen(TextPen);
		painter.drawText(place.x() + 3*LegendSize,
						 place.y() + 1.*ypos*ny+ypos/3,
						 s_pMiarex->POppTitle(s_pMiarex->m_pCurPOpp));
	}
	else
	{
		bool bStarted = false;
		for (k = 0; k<str.size(); k++)
		{
			int PlanePts = 0;
            for (l=0; l < Objects3d::s_oaPOpp.size(); l++)
			{
                pPOpp = (PlaneOpp*)Objects3d::s_oaPOpp.at(l);
				if (pPOpp->isVisible() && pPOpp->planeName() == str.at(k)) PlanePts++;
			}
			if (PlanePts)
			{
				double YPos = place.y() + (ny+PlanePts+2) * ypos;// bottom line of this foil's legend

				painter.setPen(TextPen);
				if (!bStarted || (fabs(bottom) > fabs(YPos)))
				{
					ny++;
					painter.drawText(place.x() + 1.0*LegendSize,
								  place.y() + ypos*ny-ypos/2.0, str.at(k));
				}
				else
				{
					// move right if outside screen
					place.rx() += LegendWidth;
					ny=1;
					painter.drawText(place.x() + 1.0*LegendSize,
									 place.y() + ypos*ny-ypos/2, str.at(k));
				}

				bStarted = true;
                for (nc=0; nc < Objects3d::s_oaPOpp.size(); nc++)
				{
                    pPOpp = (PlaneOpp*)Objects3d::s_oaPOpp.at(nc);
					if(str.at(k) == pPOpp->planeName() && pPOpp->isVisible())
					{
						if(qAbs(bottom)<fabs(place.y() + 1.*ypos*ny+ypos))
						{
							//move right
							place.rx() += LegendWidth;
							ny=2;
						}

						LegendPen.setColor(color(pPOpp->color()));
						LegendPen.setStyle(getStyle(pPOpp->style()));
						LegendPen.setWidth(pPOpp->width());
						painter.setPen(LegendPen);

						painter.drawLine(place.x() + 1.5*LegendSize, place.y() + 1.*ypos*ny,
										 place.x() + 2.5*LegendSize, place.y() + 1.*ypos*ny);

						if(pPOpp->points())
						{
							x1 = place.x() + 2.0*LegendSize;
							y1 = place.y() + 1.*ypos*ny;
//							painter.drawRect(x1-2, place.y() + 1.*ypos*ny-2, 4, 4);
							drawPoint(painter, pPOpp->points(), Settings::s_BackgroundColor, QPoint(x1,y1));
						}


						painter.setPen(TextPen);
						painter.drawText(place.x() + 3.0*LegendSize,
										 place.y() + 1.0*ypos*ny + ypos/3,
										 s_pMiarex->POppTitle(pPOpp));
						ny++ ;
					}
				}
				if (PlanePts) ny++;
			}
		}
	}
	painter.restore();
}





/**
* Draws the legend of the Cp graph
*@param painter the instance of the QPainter object associated to the active view
*@param the top left postition where the legend is to be drawn
*@param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWidget::drawCpLegend(QPainter &painter, Graph *pGraph, QPointF place, int bottom)
{
	painter.save();
	double LegendSize, LegendWidth, dny, x1, y1, i, ny;
	Curve *pCurve=nullptr;
	QString strong;

	LegendSize = 30;
	LegendWidth = 350;
	dny = 14;
	bottom -= 15;//margin

	QPen CurvePen;
	QPen TextPen(Settings::s_TextColor);

	ny=0;

	for (i=0; i<pGraph->curveCount(); i++)
	{
		pCurve = pGraph->curve(i);
		if(pCurve->size())
		{
			ny++;

			if(qAbs(bottom)<fabs(place.y() + dny*(ny+1)))
			{
				//move right
				place.rx() += LegendWidth;
				ny=0;
			}

			CurvePen.setColor(pCurve->color());
			CurvePen.setStyle(getStyle(pCurve->style()));
			CurvePen.setWidth(pCurve->width());
			painter.setPen(CurvePen);

			painter.drawLine(place.x() + 0.5*LegendSize, place.y() + 1.*dny*ny,
							 place.x() + 1.5*LegendSize, place.y() + 1.*dny*ny);

			if(pCurve->pointsVisible())
			{
				x1 = place.x() + 2.0*LegendSize;
				y1 = place.y() + 1.*dny*ny;
//				painter.drawRect(x1-2, place.y() + 1.*dny*ny-2,4,4);
				drawPoint(painter, pCurve->pointStyle(), Settings::s_BackgroundColor, QPoint(x1,y1));
			}

			pCurve->curveName(strong);
			painter.setPen(TextPen);
			painter.drawText(place.x() + 2.0*LegendSize,place.y() + 1.*dny*ny, strong);
		}
	}
	painter.restore();
}



/**
* Draws the legend for the time response graph- 4 curves
*@param painter the instance of the QPainter object associated to the active view
*@param the top left postition where the legend is to be drawn
*@param the y coordinate of the bottom of the drawing rectangle
*/
void LegendWidget::drawStabTimeLegend(QPainter &painter, Graph *pGraph, QPointF place, int bottom)
{
	painter.save();
	double LegendSize, LegendWidth, dny, x1, y1, i, ny;
	Curve *pCurve=nullptr;
	QString strong;

	LegendSize = 30;
	LegendWidth = 350;
	dny = 14;
	bottom -= 15;//margin

	QPen CurvePen;
	QPen TextPen(Settings::s_TextColor);

	ny=0;

	for (i=0; i<pGraph->curveCount(); i++)
	{
		pCurve = pGraph->curve(i);
		if(pCurve->size() && pCurve->isVisible())
		{
			ny++;

			if(qAbs(bottom)<fabs(place.y() + dny*(ny+1)))
			{
				//move right
				place.rx() += LegendWidth;
				ny=0;
			}

			CurvePen.setColor(pCurve->color());
			CurvePen.setStyle(getStyle(pCurve->style()));
			CurvePen.setWidth(pCurve->width());
			painter.setPen(CurvePen);

			painter.drawLine(place.x() + 1.5*LegendSize, place.y() + 1.*dny*ny,
							 place.x() + 2.5*LegendSize, place.y() + 1.*dny*ny);

			if(pCurve->pointsVisible())
			{
				x1 = place.x() + (int)(2.0*LegendSize);
				y1 = place.y() + 1.*dny*ny;
//				painter.drawRect(x1-2, place.y() + 1.*dny*ny-2,4,4);
				drawPoint(painter, pCurve->pointStyle(), Settings::s_BackgroundColor, QPoint(x1,y1));
			}

			pCurve->curveName(strong);
			painter.setPen(TextPen);
			painter.drawText(place.x() + 3*LegendSize,
							 place.y() + 1.*dny*ny+dny/3, strong);
		}
	}
	painter.restore();
}




/**
 * Paints the legend of the polar graphs
 * @param place the top-left point where the legend will be placed
 * @param bottom the number of pixels to the bottom of the client area
 * @param painter a reference to the QPainter object with which to draw
 */
void LegendWidget::drawPolarLegend(QPainter &painter, QPointF place, int bottom)
{
	double LegendSize, LegendWidth, legendHeight, x1, y1;
	int i,j,k,l,nc,ny,nFoils;

	LegendSize = 30;
	LegendWidth = 240;

	painter.setFont(Settings::textFont());

	QFont fnt(Settings::textFont()); //two step to shut valgrind up
	QFontMetrics fm(fnt);
	legendHeight = fm.height()+1;

	QPen TextPen(Settings::textColor());
	painter.setPen(TextPen);
	TextPen.setWidth(1);

	QStringList str; // we need to make an inventory of foils
	Polar * pPolar;
	Foil *pFoil;
    for (j=0; j<Objects2d::s_oaFoil.size(); j++)
	{
        pFoil = Objects2d::s_oaFoil.at(j);
        for (i=0; i<Objects2d::s_oaPolar.size(); i++)
		{
            pPolar = Objects2d::s_oaPolar.at(i);
			if (pPolar->foilName() == pFoil->foilName() && pPolar->m_Alpha.size() && pPolar->isVisible())
			{
				str.append(pFoil->foilName());
				break;
			}
		}// finished inventory
	}

	nFoils= str.size();

//	painter.setBackgroundMode(Qt::TransparentMode);
	QBrush LegendBrush(Settings::s_BackgroundColor);
	painter.setBrush(LegendBrush);

	QPen LegendPen;
	LegendPen.setWidth(1);

	ny =0;
	for (k=0; k<nFoils; k++)
	{
		int FoilPlrs = 0;
        for (l=0; l < Objects2d::s_oaPolar.count(); l++)
		{
            pPolar = Objects2d::s_oaPolar.at(l);
			if (pPolar->m_Alpha.size() &&
				pPolar->polarName().length() &&
				pPolar->isVisible() &&
				pPolar->foilName() == str.at(k)) FoilPlrs++;
		}
		if (FoilPlrs)
		{
			int YBotPos = place.y() + (ny+FoilPlrs+2) * legendHeight;// bottom line of this foil's legend
			if(abs(bottom) > abs(YBotPos))
			{
				ny++;
			}
			else if (k>0)
			{
			// move rigth if less than client bottom area
				place.rx() += LegendWidth;
				ny=1;
			}
			else
			{
				ny=1;
			}
			painter.setPen(TextPen);
			painter.drawText(place.x() + 0.5*LegendSize, place.y() + legendHeight*ny-legendHeight/2,
							 str.at(k));
		}
        for (nc=0; nc<Objects2d::s_oaPolar.count(); nc++)
		{
            pPolar = Objects2d::s_oaPolar.at(nc);
			if(str.at(k) == pPolar->foilName())
			{
				if (pPolar->m_Alpha.size() && pPolar->polarName().length() && pPolar->isVisible())
				{
					//is there anything to draw ?
					LegendPen.setColor(colour(pPolar));
					LegendPen.setStyle(getStyle(pPolar->polarStyle()));
					LegendPen.setWidth(pPolar->polarWidth());
					painter.setPen(LegendPen);

					painter.drawLine(place.x() + 1.0*LegendSize, place.y() + 1.*legendHeight*ny,
									 place.x() + 2.0*LegendSize, place.y() + 1.*legendHeight*ny);
					if(pPolar->pointStyle())
					{
						x1 = place.x() + 1.5*LegendSize;
						y1 = place.y() + 1.*legendHeight*ny;
//						painter.drawRect(x1-2, place.y() + 1.*legendHeight*ny, 4, 4);
						drawPoint(painter, pPolar->pointStyle(), Settings::s_BackgroundColor, QPoint(x1, y1));
					}

					painter.setPen(TextPen);
					painter.drawText(place.x() + 2.5*LegendSize, place.y() + 1.*legendHeight*ny+legendHeight/3,
									 pPolar->polarName());
					ny++ ;
				}
			}
		}
		if (FoilPlrs) ny++;
	}
//	painter.setBackgroundMode(Qt::OpaqueMode);

}


void LegendWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_R:
		{
			m_LegendPosition = QPointF(11.0,11.0);
			update();
			event->accept();
			break;
		}
		default:event->ignore();
	}
}



void LegendWidget::mouseMoveEvent(QMouseEvent *event)
{
	setFocus();
	if(m_bTrans)
	{
		m_LegendPosition.rx() += event->pos().x()-m_PointDown.x();
		m_LegendPosition.ry() += event->pos().y()-m_PointDown.y();
		m_PointDown = event->pos();
		update();
	}
}


void LegendWidget::mousePressEvent(QMouseEvent *event)
{
	m_PointDown.setX(event->pos().x());
	m_PointDown.setY(event->pos().y());
	m_bTrans = true;
	setCursor(Qt::ClosedHandCursor);
}



void LegendWidget::mouseReleaseEvent(QMouseEvent *)
{
	m_bTrans = false;
	setCursor(Qt::CrossCursor);
}


bool LegendWidget::isFiltered(WPolar *pWPolar)
{
    if(!pWPolar) return true;
    if(pWPolar->isFixedSpeedPolar() && !s_pMiarex->m_bType1) return true;
    if(pWPolar->isFixedLiftPolar()  && !s_pMiarex->m_bType2) return true;
    if(pWPolar->isFixedaoaPolar()   && !s_pMiarex->m_bType4) return true;
    if(pWPolar->isStabilityPolar()  && !s_pMiarex->m_bType7) return true;
    return false;
}









