/****************************************************************************

	OpPointWidget Class
	Copyright (C) 2016-2016 Andre Deperrois adeperrois@xflr5.com

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

#include "oppointwidget.h"


#include <math.h>
#include <QMenu>
#include <QtDebug>
#include <QStatusBar>
#include <mainframe.h>
#include <globals.h>
#include "misc/Settings.h"
#include "QGraph.h"
#include <xdirect/XDirect.h>
#include <xdirect/XDirectStyleDlg.h>
#include <graph/GraphDlg.h>

void *OpPointWidget::s_pMainFrame = NULL;

/**
*The public constructor
*/
OpPointWidget::OpPointWidget(QWidget *parent) : QWidget(parent)
{
	setMouseTracking(true);
	setCursor(Qt::CrossCursor);

	QSizePolicy sizepol;
	sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
	sizepol.setVerticalPolicy(QSizePolicy::Expanding);
	setSizePolicy(sizepol);

	m_bTransFoil   = false;
	m_bTransGraph  = false;
	m_bAnimate     = false;
	m_bBL          = false;
	m_bPressure    = false;
	m_bNeutralLine = true;
//	m_bShowPanels  = false;
	m_bXPressed = m_bYPressed = false;

	m_iNeutralStyle = 3;
	m_iNeutralWidth = 1;
	m_crNeutralColor = QColor(200,200,255);

	m_crBLColor = QColor(255,50,50);
	m_iBLStyle = 1;
	m_iBLWidth = 1;
	m_crPressureColor= QColor(100,150,100);
	m_iPressureStyle = 0;
	m_iPressureWidth = 1;

	m_fScale = m_fYScale = 1.0;
	m_pCpGraph = NULL;
}


/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_V:
		{
			if(m_pCpGraph->isInDrawRect(m_LastPoint))
			{
				GraphDlg::setActivePage(0);
				onGraphSettings();
			}
			event->accept();
			break;
		}
		case Qt::Key_G:
		{
			if(m_pCpGraph->isInDrawRect(m_LastPoint))
			{
				onGraphSettings();
			}
			event->accept();
			break;
		}
		case Qt::Key_R:
			if(m_pCpGraph->isInDrawRect(m_LastPoint))
				m_pCpGraph->setAuto(true);
			else setFoilScale();
			update();
			break;

		case Qt::Key_X:
			m_bXPressed = true;
			break;

		case Qt::Key_Y:
			m_bYPressed = true;
			break;

		default:event->ignore();

	}
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::keyReleaseEvent(QKeyEvent *event)
{
	Q_UNUSED(event);
	m_bXPressed = m_bYPressed = false;
}



/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::mousePressEvent(QMouseEvent *event)
{
	QPoint pt(event->x(), event->y()); //client coordinates

	if(event->buttons() & Qt::LeftButton)
	{
		if (m_pCpGraph->isInDrawRect(event->pos()))
		{
			m_bTransGraph = true;
		}
		else
		{
			m_bTransFoil = true;
		}
		m_LastPoint.setX(pt.x());
		m_LastPoint.setY(pt.y());
		setCursor(Qt::ClosedHandCursor);
//		if(!m_bAnimate) update();
	}

	event->accept();
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_bTransGraph = false;
	m_bTransFoil  = false;
	setCursor(Qt::CrossCursor);
	event->accept();
}


/**
*Overrides the mouseMoveEvent method of the base class.
*/
void OpPointWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pt;
	double scale;
	double a;

	pt.setX(event->x());
	pt.setY(event->y()); //client coordinates
	setFocus();

	if (event->buttons() & Qt::LeftButton)
	{
		if(m_bTransGraph)
		{
			QPoint point;
			double xu, yu, x1, y1, xmin, xmax, ymin, ymax;

			point = event->pos();

			// we translate the curves inside the graph
			m_pCpGraph->setAuto(false);
			x1 =  m_pCpGraph->clientTox(m_LastPoint.x()) ;
			y1 =  m_pCpGraph->clientToy(m_LastPoint.y()) ;

			xu = m_pCpGraph->clientTox(point.x());
			yu = m_pCpGraph->clientToy(point.y());

			xmin = m_pCpGraph->xMin() - xu+x1;
			xmax = m_pCpGraph->xMax() - xu+x1;
			ymin = m_pCpGraph->yMin() - yu+y1;
			ymax = m_pCpGraph->yMax() - yu+y1;

			m_pCpGraph->setWindow(xmin, xmax, ymin, ymax);
		}
		else if(m_bTransFoil)
		{
			// we translate the airfoil
			m_FoilOffset.rx() += pt.x()-m_LastPoint.x();
			m_FoilOffset.ry() += pt.y()-m_LastPoint.y();
		}
	}
	else if (QXDirect::curFoil() && ((event->buttons() & Qt::MidButton) || event->modifiers().testFlag(Qt::AltModifier)))
	{
		// we zoom the graph or the foil
		if(QXDirect::curFoil())
		{
			//zoom the foil
			scale = m_fScale;

			if(pt.y()-m_LastPoint.y()<0) m_fScale /= 1.02;
			else                         m_fScale *= 1.02;

			a = rect().center().x();

			m_FoilOffset.rx() = a + (m_FoilOffset.x()-a)/scale*m_fScale;
		}
	}

	m_LastPoint = pt;
	if(!m_bAnimate) update();

	event->accept();

}



void OpPointWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::CrossCursor);
	if (m_pCpGraph->isInDrawRect(event->pos()))
	{
		onGraphSettings();
		update();
	}
}




/**
 * The user has requested an edition of the settings of the active graph
 */
void OpPointWidget::onGraphSettings()
{
	GraphDlg grDlg(this);
	grDlg.setGraph(m_pCpGraph);

//	QAction *action = qobject_cast<QAction *>(sender());
//	grDlg.setActivePage(0);

	if(grDlg.exec() == QDialog::Accepted)
	{
		emit graphChanged(m_pCpGraph);
	}
	update();
}




/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	if(m_pCpGraph)
	{
		int h = rect().height();
		int h4 = (int)(h/3.0);
		QRect rGraphRect = QRect(0, 0, + rect().width(), rect().height()-h4);
		m_pCpGraph->setMargin(50);
		m_pCpGraph->setDrawRect(rGraphRect);
	}
	setFoilScale();
}

/**
 * Sets the Foil scale in the OpPoint view.
 */
void OpPointWidget::resetGraphScale()
{
	if(m_pCpGraph)
	{
		m_pCpGraph->setAuto(true);
		update();
	}
}

/**
 * Sets the Foil scale in the OpPoint view.
 */
void OpPointWidget::setFoilScale()
{
	int h, iMargin = 53;	
	if(m_pCpGraph)
	{
		iMargin = m_pCpGraph->margin();
		h =  m_pCpGraph->clientRect()->height();
		m_FoilOffset.rx() = rect().left() + iMargin;
		m_FoilOffset.ry() = (rect().height()+h)/2;
		m_fScale = rect().width()-2.0*iMargin;
		if(m_pCpGraph && m_pCpGraph->yVariable()<2)
		{
			double p0  = m_pCpGraph->xToClient(0.0);
			double p1  = m_pCpGraph->xToClient(1.0);
			m_fScale =  (p1-p0);
		}
	}
	else
	{
		m_FoilOffset.rx() = rect().left() + iMargin;
		m_FoilOffset.ry() = rect().center().y();

		m_fScale = rect().width()-2.0*iMargin;
	}
	m_fYScale = 1.0;
}




/**
*Overrides the wheelEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::wheelEvent(QWheelEvent *event)
{
	if(m_pCpGraph && m_pCpGraph->isInDrawRect(event->pos()))
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

		if (m_bXPressed)
		{
			//zoom x scale
			m_pCpGraph->setAutoX(false);
			m_pCpGraph->scaleXAxis(1./zoomFactor);
		}
		else if(m_bYPressed)
		{
			//zoom y scale
			m_pCpGraph->setAutoY(false);
			m_pCpGraph->scaleYAxis(1./zoomFactor);
		}
		else
		{
			//zoom both
			m_pCpGraph->setAuto(false);
			m_pCpGraph->scaleAxes(1./zoomFactor);
		}

		m_pCpGraph->setAutoXUnit();
		m_pCpGraph->setAutoYUnit();
		update();
	}
	else if(QXDirect::curFoil())
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

		double scale = m_fScale;


		if(m_bYPressed)
		{
			m_fYScale *= zoomFactor;
		}
		else
		{
			m_fScale *= zoomFactor;
		}

		int a = (int)((rect().right()+rect().left())/2);

		m_FoilOffset.rx() = a + (int)((m_FoilOffset.x()-a)/scale*m_fScale);

//		if(!m_bAnimate)
			update();
	}
}


/**
*Overrides the paintEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::paintEvent(QPaintEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	if(pMainFrame->m_iApp == XFLR5::XFOILANALYSIS)
	{
		QPainter painter(this);
		painter.save();

		painter.fillRect(rect(), Settings::s_BackgroundColor);
		paintGraph(painter);
		paintOpPoint(painter);

		painter.restore();
	}
	else
	{
		QPainter painter(this);
		painter.fillRect(rect(), Settings::s_BackgroundColor);
	}
	event->accept();
}



/**
 * Draws the graph
 * @param painter a reference to the QPainter object with which to draw
 */
void OpPointWidget::paintGraph(QPainter &painter)
{
	if(!m_pCpGraph) return;

	painter.save();

	QFontMetrics fm(Settings::textFont());
	int fmheight = fm.height();
	int fmWidth  = fm.averageCharWidth();
//  draw  the graph
	if(m_pCpGraph->clientRect()->width()>200 && m_pCpGraph->clientRect()->height()>150)
	{
		m_pCpGraph->drawGraph(painter);
		QPoint Place(m_pCpGraph->clientRect()->right()-73*fmWidth, m_pCpGraph->clientRect()->top()+fmheight);
		m_pCpGraph->drawLegend(painter, Place, Settings::s_TextFont, Settings::s_TextColor);
	}


	if(m_pCpGraph->isInDrawRect(m_LastPoint) && MainFrame::s_bShowMousePos)
	{
		QPen textPen(Settings::textColor());

		painter.setPen(textPen);

		painter.drawText(m_pCpGraph->clientRect()->width()-12*fm.averageCharWidth(), m_pCpGraph->clientRect()->top()+fmheight,   QString("x = %1").arg(m_pCpGraph->clientTox(m_LastPoint.x()),7,'f',3));
		painter.drawText(m_pCpGraph->clientRect()->width()-12*fm.averageCharWidth(), m_pCpGraph->clientRect()->top()+2*fmheight, QString("y = %1").arg(m_pCpGraph->clientToy(m_LastPoint.y()),7,'f',3));
	}
	painter.restore();
}



/**
 * Draws the Cp Graph and the foil
 * @param painter a reference to the QPainter object with which to draw
 */
void OpPointWidget::paintOpPoint(QPainter &painter)
{
	if (!QXDirect::curFoil() || !QXDirect::curFoil()->foilName().length())
		return;

	double Alpha;
	QString Result, str, str1;

	if (rect().width()<150 || rect().height()<150) return;

	painter.save();

	if(m_bNeutralLine)
	{
		QPen NeutralPen(m_crNeutralColor);
		NeutralPen.setStyle(getStyle(m_iNeutralStyle));
		NeutralPen.setWidth(m_iNeutralWidth);
		painter.setPen(NeutralPen);
		painter.drawLine(rect().left(),  m_FoilOffset.y(),
						 rect().right(), m_FoilOffset.y());
	}
	if(!m_pCpGraph->isInDrawRect(m_LastPoint) && MainFrame::s_bShowMousePos)
	{
		QPen textPen(Settings::textColor());
		QFontMetrics fm(Settings::textFont());
		int fmheight  = fm.height();
		painter.setPen(textPen);

		Vector3d real = mousetoReal(m_LastPoint);
		painter.drawText(m_pCpGraph->clientRect()->width()-12*fm.averageCharWidth(),
						 m_pCpGraph->clientRect()->height() + fmheight, QString("x = %1")
						 .arg(real.x,7,'f',3));
		painter.drawText(m_pCpGraph->clientRect()->width()-12*fm.averageCharWidth(),
						 m_pCpGraph->clientRect()->height() + 2*fmheight, QString("y = %1")
						 .arg(real.y,7,'f',3));
	}


	Alpha = 0.0;
	if(QXDirect::curOpp()) Alpha = QXDirect::curOpp()->aoa();


	drawFoil(painter, QXDirect::curFoil(), -Alpha, m_fScale, m_fScale*m_fYScale, m_FoilOffset);
	if(QXDirect::curFoil()->foilPointStyle()>0) drawPoints(painter, QXDirect::curFoil(), -Alpha, m_fScale,m_fScale*m_fYScale, m_FoilOffset);

/*	if(m_bShowPanels)
	{
		int memPts = QXDirect::curFoil()->foilPointStyle();
		QXDirect::curFoil()->foilPointStyle() = qMax(memPts, 1);
		drawPoints(painter, QXDirect::curFoil(), -Alpha, m_fScale, m_fScale*m_fYScale, m_FoilOffset);
		QXDirect::curFoil()->foilPointStyle() = memPts;
	}*/

	if(m_bPressure && QXDirect::curOpp()) paintPressure(painter, m_fScale, m_fScale*m_fYScale);
	if(m_bBL && QXDirect::curOpp())       paintBL(painter, QXDirect::curOpp(), m_fScale, m_fScale*m_fYScale);


	// Write Titles and results
	QString strong;

	painter.setFont(Settings::s_TextFont);
	int D = 0;
	int ZPos = rect().bottom();
	int XPos = rect().right()-10;
	QPen WritePen(Settings::s_TextColor);
	painter.setPen(WritePen);

	QFontMetrics fm(Settings::s_TextFont);
	int dD = fm.height();

	//write the foil's properties

	int Back = 5;

	if(QXDirect::curFoil()->m_bTEFlap) Back +=3;

	int LeftPos = rect().left()+10;
	ZPos = rect().bottom() - 10 - Back*dD;

	D = 0;
	str1 = QXDirect::curFoil()->foilName();
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str = "%";
	str1 = QString(tr("Thickness         = %1")).arg(QXDirect::curFoil()->thickness()*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Max. Thick.pos.   = %1")).arg(QXDirect::curFoil()->xThickness()*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Max. Camber       = %1")).arg( QXDirect::curFoil()->camber()*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Max. Camber pos.  = %1")).arg(QXDirect::curFoil()->xCamber()*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Number of Panels  =  %1")).arg( QXDirect::curFoil()->n);
	painter.drawText(LeftPos,ZPos+D, str1);
	D += dD;

	if(QXDirect::curFoil()->m_bTEFlap)
	{
		str1 = QString(tr("Flap Angle = %1")+QString::fromUtf8("°")).arg( QXDirect::curFoil()->m_TEFlapAngle, 7, 'f', 2);
		painter.drawText(LeftPos,ZPos+D, str1);
		D += dD;

		str1 = QString(tr("XHinge     = %1")).arg( QXDirect::curFoil()->m_TEXHinge, 6, 'f', 1);
		strong="%";
		painter.drawText(LeftPos,ZPos+D, str1+strong);
		D += dD;

		str1 = QString(tr("YHinge     = %1")).arg( QXDirect::curFoil()->m_TEYHinge, 6, 'f', 1);
		strong="%";
		painter.drawText(LeftPos,ZPos+D, str1+strong);
		D += dD;
	}


	D = 0;
	Back = 6;
	if(QXDirect::curPolar() && QXDirect::curOpp())
	{
		Back = 12;
		if(QXDirect::curOpp()->m_bTEFlap) Back++;
		if(QXDirect::curOpp()->m_bLEFlap) Back++;
		if(QXDirect::curOpp()->m_bViscResults && qAbs(QXDirect::curOpp()->Cd)>0.0) Back++;
		if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDLIFTPOLAR) Back++;
		if(QXDirect::curPolar()->polarType()!=XFOIL::FIXEDSPEEDPOLAR && QXDirect::curPolar()->polarType()!=XFOIL::FIXEDAOAPOLAR) Back++;
	}

	int dwidth = fm.width(tr("TE Hinge Moment/span = 0123456789"));

	ZPos = rect().bottom()-Back*dD - 10;
	XPos = rect().right()-dwidth-20;
	D=0;


	if(QXDirect::curPolar())
	{
		if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDSPEEDPOLAR)       str1 = tr("Fixed speed polar");
		else if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDLIFTPOLAR)   str1 = tr("Fixed lift polar");
		else if(QXDirect::curPolar()->polarType()==XFOIL::RUBBERCHORDPOLAR) str1 = tr("Rubber chord polar");
		else if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDAOAPOLAR)    str1 = tr("Fixed a.o.a. polar");

		painter.drawText(XPos,ZPos, dwidth, dD, Qt::AlignRight | Qt::AlignTop, str1);
		D += dD;
		if(QXDirect::curPolar()->polarType() ==XFOIL::FIXEDSPEEDPOLAR)
		{
			ReynoldsFormat(strong, QXDirect::curPolar()->Reynolds());
			strong ="Reynolds = " + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
			strong = QString("Mach = %1").arg( QXDirect::curPolar()->Mach(),9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}
		else if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDLIFTPOLAR)
		{
			ReynoldsFormat(strong, QXDirect::curPolar()->Reynolds());
			strong = tr("Re.sqrt(Cl) = ") + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;

			strong = QString(tr("M.sqrt(Cl) = %1")).arg(QXDirect::curPolar()->Mach(),9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}
		else if(QXDirect::curPolar()->polarType()==XFOIL::RUBBERCHORDPOLAR)
		{
			ReynoldsFormat(strong, QXDirect::curPolar()->Reynolds());
			strong = tr("Re.sqrt(Cl) = ") + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;

			strong = QString("Mach = %1").arg(QXDirect::curPolar()->Mach(),9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}
		else if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDAOAPOLAR)
		{
			strong = QString("Alpha = %1 "+QString::fromUtf8("°")).arg(QXDirect::curPolar()->aoa(),10,'f',2);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
			strong = QString("Mach = %1").arg(QXDirect::curPolar()->Mach(),9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}

		strong = QString("NCrit = %1").arg(QXDirect::curPolar()->NCrit(),9,'f',3);
		painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
		D += dD;

		strong = QString(tr("Forced Upper Trans. = %1")).arg(QXDirect::curPolar()->XtrTop(),9,'f',3);
		painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
		D += dD;
		strong = QString(tr("Forced Lower Trans. = %1")).arg(QXDirect::curPolar()->XtrBot(), 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
		D += dD;

		if(QXDirect::curOpp())
		{
			if(QXDirect::curPolar()->polarType()!=XFOIL::FIXEDSPEEDPOLAR)
			{
				ReynoldsFormat(Result, QXDirect::curOpp()->Reynolds());
				Result = "Re = "+ Result;
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
			if(QXDirect::curPolar()->polarType()==XFOIL::FIXEDLIFTPOLAR)
			{
				Result = QString("Ma = %1").arg(QXDirect::curOpp()->m_Mach, 9, 'f', 4);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
			if(QXDirect::curPolar()->polarType()!=XFOIL::FIXEDAOAPOLAR)
			{
				Result = QString(tr("Alpha = %1")+QString::fromUtf8("°")).arg(QXDirect::curOpp()->m_Alpha, 8, 'f', 2);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
			Result = QString(tr("Cl = %1")).arg(QXDirect::curOpp()->Cl, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			Result = QString(tr("Cm = %1")).arg(QXDirect::curOpp()->Cm, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			Result = QString(tr("Cd = %1")).arg(QXDirect::curOpp()->Cd, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			if(QXDirect::curOpp()->m_bViscResults && qAbs(QXDirect::curOpp()->Cd)>0.0)
			{
				Result = QString(tr("L/D = %1")).arg(QXDirect::curOpp()->Cl/QXDirect::curOpp()->Cd, 9, 'f', 3);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}

			Result = QString(tr("Upper Trans. = %1")).arg(QXDirect::curOpp()->Xtr1, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			Result = QString(tr("Lower Trans. = %1")).arg(QXDirect::curOpp()->Xtr2, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			if(QXDirect::curOpp()->m_bTEFlap)
			{
				Result = QString(tr("TE Hinge Moment/span = %1")).arg(QXDirect::curOpp()->m_TEHMom, 9, 'e', 2);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}

			if(QXDirect::curOpp()->m_bLEFlap)
			{
				Result = QString(tr("LE Hinge Moment/span = %1")).arg(QXDirect::curOpp()->m_LEHMom, 9, 'e', 2);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
		}
	}


	painter.restore();
}




/**
 * The method which draws the pressure arrows in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void OpPointWidget::paintPressure(QPainter &painter, double scalex, double scaley)
{
	if(!QXDirect::curFoil()) return;
	if(!QXDirect::curOpp()) return;
	if(!QXDirect::curOpp()->bViscResults()) return;

	int i;
	double alpha = -QXDirect::curOpp()->m_Alpha*PI/180.0;
	double cosa = cos(alpha);
	double sina = sin(alpha);
	double x, y ,xs, ys, xe, ye, dx, dy, x1, x2, y1, y2, r2;
	double cp;
	QPointF offset = m_FoilOffset;

	painter.save();

	QPen CpvPen(m_crPressureColor);
	CpvPen.setStyle(getStyle(m_iPressureStyle));
	CpvPen.setWidth(m_iPressureWidth);
	painter.setPen(CpvPen);


	for(i=0; i<QXDirect::curFoil()->n; i++)
	{
		if(QXDirect::curOpp()->m_bViscResults) cp = QXDirect::curOpp()->Cpv[i];
		else                                  cp = QXDirect::curOpp()->Cpi[i];
		x = QXDirect::curFoil()->x[i];
		y = QXDirect::curFoil()->y[i];

		xs = (x-0.5)*cosa - y*sina + 0.5;
		ys = (x-0.5)*sina + y*cosa;

		if(cp>0)
		{
			x += QXDirect::curFoil()->nx[i] * cp * 0.05;
			y += QXDirect::curFoil()->ny[i] * cp * 0.05;

			xe = (x-0.5)*cosa - y*sina + 0.5;
			ye = (x-0.5)*sina + y*cosa;
			painter.drawLine(xs*scalex + offset.x(), -ys*scaley + offset.y(),
							 xe*scalex + offset.x(), -ye*scaley + offset.y());

			dx = xe - xs;
			dy = ye - ys;
			r2 = sqrt(dx*dx + dy*dy);
			if(r2!=0.0) //you can never be sure...
			{
				dx = dx/r2;
				dy = dy/r2;
			}

			x1 = xs + 0.0085*dx + 0.005*dy;
			y1 = ys + 0.0085*dy - 0.005*dx;
			x2 = xs + 0.0085*dx - 0.005*dy;
			y2 = ys + 0.0085*dy + 0.005*dx;

			painter.drawLine( xs*scalex + offset.x(), -ys*scaley + offset.y(),
							  x1*scalex + offset.x(), -y1*scaley + offset.y());

			painter.drawLine( xs*scalex + offset.x(), -ys*scaley + offset.y(),
							  x2*scalex + offset.x(), -y2*scaley + offset.y());
		}
		else
		{

			x += -QXDirect::curFoil()->nx[i] * cp *0.05;
			y += -QXDirect::curFoil()->ny[i] * cp *0.05;

			xe = (x-0.5)*cosa - y*sina+ 0.5;
			ye = (x-0.5)*sina + y*cosa;
			painter.drawLine( xs*scalex + offset.x(), -ys*scaley + offset.y(),
							  xe*scalex + offset.x(), -ye*scaley + offset.y());

			dx = xe - xs;
			dy = ye - ys;
			r2 = sqrt(dx*dx + dy*dy);
			if(r2!=0.0) //you can never be sure...
			{
				dx = -dx/r2;
				dy = -dy/r2;
			}

			x1 = xe + 0.0085*dx + 0.005*dy;
			y1 = ye + 0.0085*dy - 0.005*dx;
			x2 = xe + 0.0085*dx - 0.005*dy;
			y2 = ye + 0.0085*dy + 0.005*dx;

			painter.drawLine( xe*scalex + offset.x(), -ye*scaley + offset.y(),
							  x1*scalex + offset.x(), -y1*scaley + offset.y());

			painter.drawLine( xe*scalex + offset.x(), -ye*scaley + offset.y(),
							  x2*scalex + offset.x(), -y2*scaley + offset.y());
		}
	}
	//last draw lift at XCP position
	QPen LiftPen(m_crPressureColor);
	LiftPen.setStyle(getStyle(m_iPressureStyle));
	LiftPen.setWidth(m_iPressureWidth+1);
	painter.setPen(LiftPen);

	xs =  (QXDirect::curOpp()->m_XCP-0.5)*cosa  + 0.5;
	ys = -(QXDirect::curOpp()->m_XCP-0.5)*sina ;

	xe = xs;
	ye = ys - QXDirect::curOpp()->Cl/10.0;

	painter.drawLine( xs*scalex + offset.x(), ys*scaley + offset.y(),
					  xs*scalex + offset.x(), ye*scaley + offset.y());

	dx = xe - xs;
	dy = ye - ys;
	r2 = sqrt(dx*dx + dy*dy);
	dx = -dx/r2;
	dy = -dy/r2;

	x1 = xe + 0.0085*dx + 0.005*dy;
	y1 = ye + 0.0085*dy - 0.005*dx;
	x2 = xe + 0.0085*dx - 0.005*dy;
	y2 = ye + 0.0085*dy + 0.005*dx;

	painter.drawLine( xe*scalex + offset.x(), ye*scaley + offset.y(),
					  x1*scalex + offset.x(), y1*scaley + offset.y());

	painter.drawLine( xe*scalex + offset.x(), ye*scaley + offset.y(),
					  x2*scalex + offset.x(), y2*scaley + offset.y());

	painter.restore();
}





/**
 * The method which draws the boundary layer in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void OpPointWidget::paintBL(QPainter &painter, OpPoint* pOpPoint, double scalex, double scaley)
{
	if(!QXDirect::curFoil() || !pOpPoint) return;

	QPointF offset, From, To;
	double x,y;
	int i;
	double alpha = -pOpPoint->aoa()*PI/180.0;
	double cosa = cos(alpha);
	double sina = sin(alpha);

	if(!pOpPoint->m_bViscResults || !pOpPoint->m_bBL) return;

	painter.save();

	offset = m_FoilOffset;

	QPen WakePen(m_crBLColor);
	WakePen.setStyle(getStyle(m_iBLStyle));
	WakePen.setWidth(m_iBLWidth);

	painter.setPen(WakePen);

	x = (pOpPoint->xd1[1]-0.5)*cosa - pOpPoint->yd1[1]*sina + 0.5;
	y = (pOpPoint->xd1[1]-0.5)*sina + pOpPoint->yd1[1]*cosa;
	From.rx() =  x*scalex + offset.x();
	From.ry() = -y*scaley + offset.y();
	for (i=2; i<=pOpPoint->nd1; i++)
	{
		x = (pOpPoint->xd1[i]-0.5)*cosa - pOpPoint->yd1[i]*sina + 0.5;
		y = (pOpPoint->xd1[i]-0.5)*sina + pOpPoint->yd1[i]*cosa;
		To.rx() =  x*scalex + offset.x();
		To.ry() = -y*scaley + offset.y();
		painter.drawLine(From, To);
		From = To;
	}

	x = (pOpPoint->xd2[0]-0.5)*cosa - pOpPoint->yd2[0]*sina + 0.5;
	y = (pOpPoint->xd2[0]-0.5)*sina + pOpPoint->yd2[0]*cosa;
	From.rx() =  x*scalex + offset.x();
	From.ry() = -y*scaley + offset.y();
	for (i=1; i<pOpPoint->nd2; i++)
	{
		x = (pOpPoint->xd2[i]-0.5)*cosa - pOpPoint->yd2[i]*sina + 0.5;
		y = (pOpPoint->xd2[i]-0.5)*sina + pOpPoint->yd2[i]*cosa;
		To.rx() =  x*scalex + offset.x();
		To.ry() = -y*scaley + offset.y();
		painter.drawLine(From, To);
		From = To;
	}

	x = (pOpPoint->xd3[0]-0.5)*cosa - pOpPoint->yd3[0]*sina + 0.5;
	y = (pOpPoint->xd3[0]-0.5)*sina + pOpPoint->yd3[0]*cosa;
	From.rx() =  x*scalex + offset.x();
	From.ry() = -y*scaley + offset.y();
	for (i=1; i<pOpPoint->nd3; i++)
	{
		x = (pOpPoint->xd3[i]-0.5)*cosa - pOpPoint->yd3[i]*sina + 0.5;
		y = (pOpPoint->xd3[i]-0.5)*sina + pOpPoint->yd3[i]*cosa;
		To.rx() =  x*scalex + offset.x();
		To.ry() = -y*scaley + offset.y();
		painter.drawLine(From, To);
		From = To;
	}
	painter.restore();
}




/**
*Overrides the contextMenuEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::contextMenuEvent (QContextMenuEvent * event)
{
	event->ignore();
}



/**
 * The user has requested the launch of the interface used to define the display style of the Foil
 */
void OpPointWidget::onXDirectStyle()
{
	XDirectStyleDlg xdsDlg(this);
	xdsDlg.m_iBLStyle         = m_iBLStyle;
	xdsDlg.m_iBLWidth         = m_iBLWidth;
	xdsDlg.m_crBLColor        = m_crBLColor;
	xdsDlg.m_iPressureStyle   = m_iPressureStyle;
	xdsDlg.m_iPressureWidth   = m_iPressureWidth;
	xdsDlg.m_crPressureColor  = m_crPressureColor;
	xdsDlg.m_iNeutralStyle    = m_iNeutralStyle;
	xdsDlg.m_iNeutralWidth    = m_iNeutralWidth;
	xdsDlg.m_crNeutralColor   = m_crNeutralColor;

	if(xdsDlg.exec() == QDialog::Accepted)
	{
		m_iBLStyle         = xdsDlg.m_iBLStyle;
		m_iBLWidth         = xdsDlg.m_iBLWidth;
		m_crBLColor        = xdsDlg.m_crBLColor;
		m_iPressureStyle   = xdsDlg.m_iPressureStyle;
		m_iPressureWidth   = xdsDlg.m_iPressureWidth;
		m_crPressureColor  = xdsDlg.m_crPressureColor;
		m_iNeutralStyle    = xdsDlg.m_iNeutralStyle;
		m_iNeutralWidth    = xdsDlg.m_iNeutralWidth;
		m_crNeutralColor   = xdsDlg.m_crNeutralColor;
	}
	update();
}


/**
 * Converts screen coordinates to viewport coordinates
 * @param point the screen coordinates
 * @return the viewport coordinates
 */
Vector3d OpPointWidget::mousetoReal(QPoint point)
{
	Vector3d Real;

	Real.x =  (point.x() - m_FoilOffset.x())/m_fScale;
	Real.y = -(point.y() - m_FoilOffset.y())/m_fScale;
	Real.z = 0.0;

	return Real;
}



/**
 * The user has requested to reset the scale of the foil to its automatic default value
 */
void OpPointWidget::onResetFoilScale()
{
	setFoilScale();
//	if(!m_bAnimate)
	update();
}


/**
 * The user has toggled the display of the neutral line y=0.
 */
void OpPointWidget::onShowNeutralLine()
{
	m_bNeutralLine = !m_bNeutralLine;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
	update();
}


/**
 * The user has toggled the display of the Foil's panels.
 */
/*void OpPointWidget::onShowPanels()
{
	m_bShowPanels = !m_bShowPanels;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->m_pShowPanels->setChecked(m_bShowPanels);
	update();
}*/


/**
 * The user has toggled the display of the pressure arrows
 */
void OpPointWidget::onShowPressure(bool bPressure)
{
	showPressure(bPressure);
	update();
}



/**
 * The user has toggled the display of the boundary layer
 */
void OpPointWidget::onShowBL(bool bBL)
{
	showBL(bBL);
	update();
}


void OpPointWidget::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("OpPointSettings");
	{
		pSettings->setValue("BLColor", m_crBLColor);
		pSettings->setValue("BLWidth", m_iBLWidth);
		pSettings->setValue("BLStyle", m_iBLStyle);

		pSettings->setValue("PressureWidth", m_iPressureWidth);
		pSettings->setValue("PressureStyle", m_iPressureStyle);
		pSettings->setValue("PressureColor", m_crPressureColor);

		pSettings->setValue("NeutralColor", m_crNeutralColor);
		pSettings->setValue("NeutralWidth", m_iNeutralWidth);
		pSettings->setValue("NeutralStyle", m_iNeutralStyle);
	}
	pSettings->endGroup();
}



void OpPointWidget::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("OpPointSettings");
	{
		m_iBLStyle  = pSettings->value("BLStyle", 1).toInt();
		m_iBLWidth  = pSettings->value("BLWidth", 1).toInt();
		m_crBLColor = pSettings->value("BLColor",QColor(235,50,50)).value<QColor>();

		m_iPressureStyle  = pSettings->value("PressureStyle", 0).toInt();
		m_iPressureWidth  = pSettings->value("PressureWidth", 1).toInt();
		m_crPressureColor = pSettings->value("PressureColor",QColor(100,150,100)).value<QColor>();

		m_iNeutralStyle  = pSettings->value("NeutralStyle", 3).toInt();
		m_iNeutralWidth  = pSettings->value("NeutralWidth", 1).toInt();
		m_crNeutralColor = pSettings->value("NeutralColor",QColor(190,190,190)).value<QColor>();
	}
	pSettings->endGroup();
}

