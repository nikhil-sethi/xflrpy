/****************************************************************************

    Graph Classes
	Copyright (C) 2003-2016 Andre Deperrois adeperrois@xflr5.com

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



#include "Graph.h"
#include <math.h>
#include <QtDebug>

//static variables must be initialized in C++
QColor Graph::m_CurveColors[10];


Graph::Graph()
{
	//Type is used to determine automatic scales
	m_Type = 1;

	m_X = 0;
	m_Y = 1;

	xmin         = .0;
	xmax         = .10;
	ymin         = .0;
	ymax         = .10;
	xunit        = 0.1;
	yunit        = 0.1;
	xo           = 0.0;
	yo           = 0.0;
	m_XMinorUnit = 0.01;
	m_YMinorUnit = 0.01;

	m_scalex     = 0.1;
	m_scaley     = 0.1;
	Cxmin        = 0.0;
	Cxmax        = 1.0;
	Cymin        = 0.0;
	Cymax        = 1.0;

	exp_x = 0;
	exp_y = 0;

	m_bYInverted  = false;
	m_bAutoX        = true;
	m_bAutoY        = true;
	m_bXAutoMinGrid = true;
	m_bYAutoMinGrid = true;
	m_bBorder       = true;

	m_ptoffset.rx() = 0;
	m_ptoffset.ry() = 0;

	m_CurveColors[0] = QColor(255,   0,   0);
	m_CurveColors[1] = QColor(  0,   0, 255);
	m_CurveColors[2] = QColor(  0, 255,   0);
	m_CurveColors[3] = QColor(255, 255,   0);
	m_CurveColors[4] = QColor(  0, 255, 255);
	m_CurveColors[5] = QColor(255,   0, 255);
	m_CurveColors[6] = QColor(255, 125,  70);
	m_CurveColors[7] = QColor( 70, 125, 255);
	m_CurveColors[8] = QColor(125, 255,  70);
	m_CurveColors[9] = QColor(255, 70,  200);

	m_AxisStyle   = 0;
	m_AxisWidth   = 1;
	m_BorderStyle = 0;
	m_BorderWidth = 2;
	m_XMajStyle   = 0;
	m_XMajWidth   = 1;
	m_XMinStyle   = 1;
	m_XMinWidth   = 1;
	m_YMajStyle   = 0;
	m_YMajWidth   = 1;
	m_YMinStyle   = 1;
	m_YMinWidth   = 1;

	m_iMargin = 41;

	m_h       = 0;
	m_w       = 0;

	setGraphDefaults();
}

Graph::~Graph()
{
}



Curve* Graph::addCurve()
{
	Curve *pCurve = new Curve();
	if(pCurve)
	{
		int nIndex = m_oaCurves.size();
		pCurve->setColor(m_CurveColors[nIndex%10]);
		pCurve->setStyle(0);
		pCurve->m_pParentGraph = this;
		m_oaCurves.append(pCurve);
	}
	return pCurve;
}

/**< In the case where a curve has been constructed independantly and needs to be added to the Graph */
Curve* Graph::addCurve(Curve *pCurve)
{
	if(pCurve)
	{
		int nIndex = m_oaCurves.size();
		pCurve->setColor(m_CurveColors[nIndex%10]);
		pCurve->setStyle(0);
		pCurve->m_pParentGraph = this;
		m_oaCurves.append(pCurve);
	}
	return pCurve;
}



double Graph::clientTox(double x)
{
	return (x-m_ptoffset.x())*m_scalex;
}

double Graph::clientToy(double y)
{
	return (y-m_ptoffset.y())*m_scaley;
}

double Graph::clientTox(int x)
{
	return ((double)x-(double)m_ptoffset.x())*m_scalex;
}

double Graph::clientToy(int y)
{
	return ((double)y-(double)m_ptoffset.y())*m_scaley;
}



void Graph::copySettings(Graph *pGraph, bool bScales)
{
	if(bScales)
	{
		xmin            = pGraph->xmin;
		xmax            = pGraph->xmax;
		xo              = pGraph->xo;
		xunit           = pGraph->xunit;

		ymin            = pGraph->ymin;
		ymax            = pGraph->ymax;
		yo              = pGraph->yo;
		yunit           = pGraph->yunit;

		m_scalex        = pGraph->m_scalex;
		m_scaley        = pGraph->m_scaley;
	}

	m_AxisColor     = pGraph->m_AxisColor;
	m_BkColor       = pGraph->m_BkColor;
	m_bBorder       = pGraph->m_bBorder;
	m_BorderColor   = pGraph->m_BorderColor;
	m_BorderStyle   = pGraph->m_BorderStyle;
	m_BorderWidth   = pGraph->m_BorderWidth;
	m_LabelColor    = pGraph->m_LabelColor;
	m_TitleColor    = pGraph->m_TitleColor;
	m_AxisStyle     = pGraph->m_AxisStyle;
	m_AxisWidth     = pGraph->m_AxisWidth;
	m_XMajClr       = pGraph->m_XMajClr;
	m_XMajStyle     = pGraph->m_XMajStyle;
	m_XMajWidth     = pGraph->m_XMajWidth;
	m_XMinClr       = pGraph->m_XMinClr;
	m_XMinStyle     = pGraph->m_XMinStyle;
	m_XMinWidth     = pGraph->m_XMinWidth;
	m_YMajClr       = pGraph->m_YMajClr;
	m_YMajStyle     = pGraph->m_YMajStyle;
	m_YMajWidth     = pGraph->m_YMajWidth;
	m_YMinClr       = pGraph->m_YMinClr;
	m_YMinStyle     = pGraph->m_YMinStyle;
	m_YMinWidth     = pGraph->m_YMinWidth;

	m_bAutoX        = pGraph->m_bAutoX;
	m_bAutoY        = pGraph->m_bAutoY;
	m_bXAutoMinGrid = pGraph->m_bXAutoMinGrid;
	m_bYAutoMinGrid = pGraph->m_bYAutoMinGrid;
	m_bYInverted    = pGraph->m_bYInverted;
	m_bXMajGrid     = pGraph->m_bXMajGrid;
	m_bXMinGrid     = pGraph->m_bXMinGrid;
	m_bYMajGrid     = pGraph->m_bYMajGrid;
	m_bYMinGrid     = pGraph->m_bYMinGrid;
	m_bBorder       = pGraph->m_bBorder;
	m_iMargin       = pGraph->m_iMargin;
}


void Graph::deleteCurve(int index)
{
    Curve * pCurve = curve(index);
    m_oaCurves.removeAt(index);
	delete pCurve;
}


void Graph::deleteCurve(Curve *pCurve)
{
	Curve *pOldCurve = NULL;
	for(int i=0; i<m_oaCurves.size(); i++)
	{
		pOldCurve = m_oaCurves.at(i);
		if(pOldCurve==pCurve)
		{
			m_oaCurves.removeAt(i);
			delete pCurve;
			return;
		}
	}
}


void Graph::deleteCurve(QString CurveTitle)
{
	Curve *pOldCurve = NULL;
	for(int i=0; i<m_oaCurves.size(); i++)
	{
		pOldCurve = m_oaCurves.at(i);
		if(pOldCurve->m_CurveName==CurveTitle)
		{
			m_oaCurves.removeAt(i);
			delete pOldCurve;
			return;
		}
	}
}


void Graph::deleteCurves()
{
	for (int i=m_oaCurves.size()-1; i>=0;i--)
		delete curve(i);

    m_oaCurves.clear();//removes the pointers

    if (m_bAutoX && !m_Type)
    {
		xmin =  0.0;
		xmax =  0.1;
	}

	if (m_bAutoY && !m_Type)
	{
		ymin =  0.0;
		ymax =  0.1;
	}
}


//___________________Start Gets______________________________________________________________



QColor Graph::labelColor()
{
	return m_LabelColor;
}

QColor Graph::axisColor()
{
	return m_AxisColor;
}



int Graph::axisStyle()
{
	return m_AxisStyle;
}

int Graph::axisWidth()
{
	return m_AxisWidth;
}


bool Graph::bAutoX()
{
	return m_bAutoX;
}
bool Graph::bAutoY()
{
	return m_bAutoY;
}




bool Graph::bAutoXMin()
{
	return m_bXAutoMinGrid;
}
bool Graph::bAutoYMin()
{
	return m_bYAutoMinGrid;
}



bool Graph::hasBorder()
{
	return m_bBorder;
}


QRect * Graph::clientRect()
{
	return &m_rCltRect;
}


Curve* Graph::curve(int nIndex)
{
    if(m_oaCurves.size()>nIndex)
		return m_oaCurves[nIndex];
	else return NULL;
}


Curve* Graph::curve(QString CurveTitle)
{
	QString strong;
	Curve * pCurve;
	for(int i=0; i<m_oaCurves.size(); i++)
	{
		pCurve = m_oaCurves.at(i);
		if(pCurve)
		{
			pCurve->curveName(strong);
			if(strong==CurveTitle) return pCurve;
		}
	}
	return NULL;
}


bool Graph::bInverted()
{
	return m_bYInverted;
}

int Graph::margin()
{
	return m_iMargin;
}


void Graph::graphName(QString &GraphName)
{
	GraphName = m_GraphName;
}



QColor Graph::titleColor()
{
	return m_TitleColor;
}
double Graph::xOrigin()
{
	return xo;
}


bool Graph::bXMajGrid()
{
	return m_bXMajGrid;
}


void Graph::bXMajGrid(bool &bstate, QColor &clr, int &style, int &width)
{
	bstate = m_bXMajGrid;
	clr   = m_XMajClr;
	style = m_XMajStyle;
	width = m_XMajWidth;
}


bool Graph::bXMinGrid()
{
	return m_bXMinGrid;
}

void Graph::bXMinGrid(bool &state, bool &bAuto, QColor &clr, int &style, int &width, double &unit)
{
	state = m_bXMinGrid;
	bAuto = m_bXAutoMinGrid;
	clr   = m_XMinClr;
	style = m_XMinStyle;
	width = m_XMinWidth;
	unit  = m_XMinorUnit;
}

double Graph::xMin()
{
	return xmin;
}

double Graph::xMax()
{
	return xmax;
}


double Graph::xScale()
{
	return m_scalex;
}


double Graph::xUnit()
{
	return xunit;
}


int Graph::xVariable()
{
	return m_X;
}



bool Graph::yMajGrid()
{
	return m_bYMajGrid;
}

void Graph::yMajGrid(bool &state, QColor &clr, int &style, int &width)
{
	state = m_bYMajGrid;
	clr   = m_YMajClr;
	style = m_YMajStyle;
	width = m_YMajWidth;
}


bool Graph::bYMinGrid()
{
	return m_bYMinGrid;
}


void Graph::bYMinGrid(bool &state, bool &bAuto, QColor &clr, int &style, int &width, double &unit)
{
	state = m_bYMinGrid;
	bAuto = m_bYAutoMinGrid;
	clr   = m_YMinClr;
	style = m_YMinStyle;
	width = m_YMinWidth;
	unit  = m_YMinorUnit;
}


double Graph::yOrigin()
{
	return yo;
}
double Graph::yMin()
{
	return ymin;
}
double Graph::yMax()
{
	return ymax;
}
double Graph::yUnit()
{
	return yunit;
}


double Graph::yScale()
{
	return m_scaley;
}


int Graph::yVariable()
{
	return m_Y;
}


bool Graph::initializeGraph(int width, int height)
{
	if(width>0 && height>0)
	{
		m_w =  width  -2*m_iMargin;
		m_h =  height -2*m_iMargin;
	}
	else
	{
		m_w =  m_rCltRect.width()  -2*m_iMargin;
		m_h =  m_rCltRect.height() -2*m_iMargin;
	}

	setXScale();
	setYScale();

	if(m_bXAutoMinGrid) m_XMinorUnit = xunit/5.0;
	if(m_bYAutoMinGrid) m_YMinorUnit = yunit/5.0;

	return true;
}


bool Graph::isInDrawRect(QPoint const &pt)
{
	if(m_rCltRect.contains(pt)) return true;
	else return false;
}

bool Graph::isInDrawRect(int const &x, int const &y)
{
	if(m_rCltRect.contains(x,y)) return true;
	else return false;
}


void Graph::resetCurves()
{
	Curve *pCurve;
	for(int i=0; i<m_oaCurves.size(); i++)
	{
		pCurve = m_oaCurves.at(i);
		pCurve->clear();
	}
}


void Graph::resetLimits()
{
	resetXLimits();
	resetYLimits();
}


void Graph::resetXLimits()
{
	if(m_bAutoX)
	{
		xmin =  0.0;
		xmax =  0.1;
		xo   =  0.0;
	}
}


void Graph::resetYLimits()
{
	if(m_bAutoY)
	{
		ymin =   0.000;
		ymax =   0.001;
		yo   =   0.000;
	}
}


void Graph::scaleAxes(double zoom)
{
	if (zoom<0.01) zoom =0.01;
	m_bAutoX = false;
	m_bAutoY = false;

	double xm = (xmin + xmax)/2.0;
	xmin = xm+(xmin-xm)*zoom;
	xmax = xm+(xmax-xm)*zoom;

	double ym = (ymin + ymax)/2.0;
	ymin = ym+(ymin-ym)*zoom;
	ymax = ym+(ymax-ym)*zoom;
}


void Graph::scaleXAxis(double zoom)
{
	if (zoom<0.01) zoom =0.01;
	m_bAutoX = false;

	double xm = (xmin + xmax)/2.0;
	xmin = xm+(xmin-xm)*zoom;
	xmax = xm+(xmax-xm)*zoom;

}

void Graph::scaleYAxis(double zoom)
{
	if (zoom<0.01) zoom =0.01;
	m_bAutoY = false;

	double ym = (ymin + ymax)/2.0;
	ymin = ym+(ymin-ym)*zoom;
	ymax = ym+(ymax-ym)*zoom;
}

//___________________Start Sets______________________________________________________________


void Graph::setAuto(bool bAuto)
{
	m_bAutoX = bAuto;
	m_bAutoY = bAuto;
	resetXLimits();
	resetYLimits();
}

void Graph::setAutoX(bool bAuto)
{
	m_bAutoX = bAuto;
	resetXLimits();
}
void Graph::setAutoY(bool bAuto)
{
	m_bAutoY = bAuto;
	resetYLimits();
}
void Graph::setAutoXMinUnit(bool bAuto)
{
	m_bXAutoMinGrid = bAuto;
	if(bAuto) m_XMinorUnit = xunit/5.0;
}


void Graph::setAutoXUnit()
{
//	xunit = 100.0*m_scalex;
	xunit = (xmax-xmin)/3.0;

	if (xunit<1.0)
	{
		exp_x = (int)log10(xunit*1.00001)-1;
		exp_x = qMax(-4, exp_x);
	}
	else exp_x = (int)log10(xunit*1.00001);
	int main_x = (int)(xunit/pow(10.0, exp_x)*1.000001);


	if(main_x<2)
		xunit = pow(10.0,exp_x);
	else if (main_x<5)
		xunit = 2.0*pow(10.0,exp_x);
	else
		xunit = 5.0*pow(10.0,exp_x);

}


void Graph::setAutoYMinUnit(bool bAuto)
{
	m_bYAutoMinGrid = bAuto;
	if(bAuto) m_YMinorUnit = yunit/5.0;
}


void Graph::setAutoYUnit()
{
//	yunit = 100.0 * m_scaley;
	yunit = (ymax-ymin)/5.0;
	if (yunit<1.0)
	{
		exp_y = (int)log10(yunit*1.00001)-1;
//		exp_y = qMax(-4, exp_y);
	}
	else  exp_y = (int)log10(yunit*1.00001);

	int main_y = (int)(yunit/pow(10.0, exp_y));

	if(main_y<2)
		yunit = pow(10.0,exp_y);
	else if (main_y<5)
		yunit = 2.0*pow(10.0,exp_y);
	else
		yunit = 5.0*pow(10.0,exp_y);	
}

void Graph::setAxisData(int s, int w, QColor clr)
{
	m_AxisStyle = s;
	m_AxisWidth = w;
	m_AxisColor = clr;
}

void Graph::setAxisColor(QColor crColor)
{
	m_AxisColor = crColor;
}

void Graph::setAxisStyle(int nStyle)
{
	m_AxisStyle = nStyle;
}

void Graph::setAxisWidth(int Width)
{
	m_AxisWidth = Width;
}


void Graph::setBkColor(QColor cr)
{
	m_BkColor = cr;
}

void Graph::setBorderColor(QColor crBorder)
{
	m_BorderColor = crBorder;
}

void Graph::setBorder(bool bBorder)
{
	m_bBorder = bBorder;
}

void Graph::setBorderWidth(int w)
{
	m_BorderWidth = w;
}

void Graph::setBorderStyle(int s)
{
	m_BorderStyle = s;
}

void Graph::setDrawRect(QRect Rect)
{
	m_rCltRect = Rect;
}

void Graph::setGraphName(QString GraphName)
{
	m_GraphName = GraphName;
}


void Graph::setGraphDefaults(bool bDark)
{
	if(bDark)
	{
		m_BkColor = QColor(0,9,13);
		m_BorderColor = QColor(200,200,200);

		setAxisColor(QColor(200,200,200));
		setTitleColor(QColor(255,255,255));
		setLabelColor(QColor(255,255,255));

		m_XMajClr   = QColor(90,90,90);
		m_YMajClr   = QColor(90,90,90);

		m_XMinClr   = QColor(50,50,50);
		m_YMinClr   = QColor(50,50,50);
	}
	else
	{
		m_BkColor = QColor(255,255,255);
		m_BorderColor = QColor(55,55,55);

		setAxisColor(QColor(55,55,55));
		setTitleColor(QColor(0,0,0));
		setLabelColor(QColor(0,0,0));

		m_XMajClr   = QColor(165,165,165);
		m_YMajClr   = QColor(165,165,165);

		m_XMinClr   = QColor(205,205,205);
		m_YMinClr   = QColor(205,205,205);
	}

	m_BorderStyle = 0;
	m_BorderWidth = 3;

	m_AxisStyle = 0;
	m_AxisWidth = 1;

	m_bYInverted = false;

	m_bXMajGrid = true;
	m_bYMajGrid = true;
	m_bXMinGrid = false;
	m_bYMinGrid = false;

	m_XMajStyle = 1;
	m_YMajStyle = 1;
	m_XMajWidth = 1;
	m_YMajWidth = 1;

	m_XMinStyle = 1;
	m_YMinStyle = 1;
	m_XMinWidth = 1;
	m_YMinWidth = 1;

	m_XMinorUnit = 0.1;
	m_YMinorUnit = 0.1;
}




void Graph::setLabelColor(QColor crColor)
{
	m_LabelColor = crColor;
}


void Graph::setInverted(bool bInverted)
{
	m_bYInverted = bInverted;
}


void Graph::setMargin(int m)
{
	m_iMargin = m;
}



void Graph::setTitleColor(QColor crColor)
{
	m_TitleColor = crColor;
}




void Graph::setType(int type)
{
	m_Type = type;
}



void Graph::setVariables(int const & X, int const & Y)
{
	m_X = X;
	m_Y = Y;
}


void Graph::setWindow(double x1, double x2, double y1, double y2)
{
	m_bAutoX = false;
	m_bAutoY = false;
	xmin = x1;
	xmax = x2;
	ymin = y1;
	ymax = y2;
}

void Graph::setX0(double f){
	xo = f;
}



void Graph::setXMajGrid(bool const &state, QColor const &clr, int const &style, int const &width)
{
	m_bXMajGrid = state;
	m_XMajClr   = clr;
	m_XMajStyle = style;
	m_XMajWidth = width;
}


void Graph::setXMajGrid(bool const &bGrid)
{
	m_bXMajGrid = bGrid;
}

void Graph::setXMinGrid(bool const &bGrid)
{
	m_bXMinGrid = bGrid;
}



void Graph::setXMax(double f)
{
	xmax = f;
}


void Graph::setXMin(double f)
{
	xmin = f;
}

void Graph::setXMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit)
{
	m_bXMinGrid = state;
	m_bXAutoMinGrid = bAuto;
	m_XMinClr   = clr;
	m_XMinStyle = style;
	m_XMinWidth = width;
	if(unit>0.0) m_XMinorUnit  = unit;
}



void Graph::setXMinorUnit(double f)
{
	m_XMinorUnit = f;
}


bool Graph::setXScale()
{
	Curve *pCurve;
	int nc;

	if(m_bAutoX)
	{
		bool bCurve = false;

		if (m_oaCurves.size())
		{
			//init only if we have a curve
			for (nc=0; nc < m_oaCurves.size(); nc++)
			{
				pCurve = m_oaCurves[nc];
				if ((pCurve->isVisible() ||pCurve->pointsVisible()) && pCurve->size()>1)
				{
					bCurve = true;
					break;//there is something to draw
				}
			}
		}
		if (bCurve)
		{
			Cxmin =  9999999.0;
			Cxmax = -9999999.0;
			for (nc=0; nc < m_oaCurves.size(); nc++)
			{
				pCurve = m_oaCurves[nc];
				if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
				{
					Cxmin = qMin(Cxmin, pCurve->xMin());
					Cxmax = qMax(Cxmax, pCurve->xMax());
				}
			}

			if(Cxmax<=Cxmin)
				Cxmax = (Cxmin+1.0)*2.0;

			if(m_Type == 1)
			{
				xmin = qMin(xmin, Cxmin);
				xmax = qMax(xmax, Cxmax);
			}
			else
			{
				xmin = Cxmin;
				xmax = Cxmax;
			}
			if(Cxmin>=0.0) xmin = 0.0;
			if(Cxmax<=0.0) xmax = 0.0;

		}
		else
		{
			// until things are made clear
			for (nc=0; nc < m_oaCurves.size(); nc++)
			{
				pCurve = m_oaCurves[nc];
				if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
				{
					xmin = qMin(xmin, pCurve->x[0]);
					xmax = qMax(xmax, pCurve->x[0]);
				}
			}
		}
		xo=0.0;

		if(qAbs((xmin-xmax)/xmin)<0.001)
		{
			if(qAbs(xmin)<0.00001) xmax = 1.0;
			else
			{
				xmax = 2.0 * xmin;
				if(xmax < xmin)
				{
					double tmp = xmax;
					xmax = xmin;
					xmin = tmp;
				}
			}
		}

		if(m_w<=0.0) return false;

		m_scalex   = (xmax-xmin)/m_w;


		//try to set an automatic scale for X Axis

		setAutoXUnit();
	}
	else
	{
		//scales are set manually
		if(m_w<=0.0) return false;

//		m_scalex   =  (xmax-xmin)/m_w;
		if (xunit<1.0)
		{
			exp_x = (int)log10(xunit*1.00001)-1;
			exp_x = qMax(-4, exp_x);
		}
		else exp_x = (int)log10(xunit*1.00001);

	}
	m_scalex   =  (xmax-xmin)/m_w;

	//graph center position
	int Xg = (m_rCltRect.right() + m_rCltRect.left())/2;
	// curves center position
	int Xc = (int)((xmin+xmax)/2.0/m_scalex);
	// center graph in drawing rectangle
	m_ptoffset.rx() = (Xg-Xc);
	return true;
}



void Graph::setXUnit(double f){
	xunit = f;
}

void Graph::setXTitle(QString str)
{
	m_XTitle = str;
}



void Graph::setXVariable(int const & X)
{
	m_X = X;
}




void Graph::setYMin(double f){
	ymin = f;
}

void Graph::setYMinorUnit(double f){
	m_YMinorUnit = f;
}


void Graph::setYMax(double f){
	ymax = f;
}

void Graph::setY0(double f){
	yo = f;
}

void Graph::setYTitle(QString str)
{
	m_YTitle = str;
}

void Graph::setYUnit(double f){
	yunit = f;
}



bool Graph::setYScale()
{
	int nc;
	Curve *pCurve;

	if(m_bAutoY)
	{
		bool bCurve = false;
		if (m_oaCurves.size())
		{
			//init only if we have a curve
			for (nc=0; nc < m_oaCurves.size(); nc++)
			{
				pCurve = m_oaCurves[nc];
				if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
					{
						bCurve = true;
						break;
					}
			}
		}
		if(bCurve)
		{
			Cymin =  9999999.0;
			Cymax = -9999999.0;
			for (nc=0; nc < m_oaCurves.size(); nc++)
			{
				pCurve = m_oaCurves[nc];
				if ((pCurve->isVisible() ||pCurve->pointsVisible()) && pCurve->size()>0)
				{
					Cymin = qMin(Cymin, pCurve->yMin());
					Cymax = qMax(Cymax, pCurve->yMax());
				}
			}
			if(Cymax<=Cymin)
			{
				Cymax = (Cymin+1.0)*2.0;
			}

			if(m_Type == 1)
			{
				ymin = qMin(ymin, Cymin);
				ymax = qMax(ymax, Cymax);
			}
			else
			{
				ymin = Cymin;
				ymax = Cymax;
			}
			if(Cymin>=0.0) ymin = 0.0;
			if(Cymax<=0.0) ymax = 0.0;
		}
		else
		{
			// until things are made clear
			for (int nc=0; nc < m_oaCurves.size(); nc++)
			{
				pCurve = m_oaCurves[nc];
				if ((pCurve->isVisible() ||pCurve->pointsVisible())  && pCurve->size()>0)
				{
					ymin = qMin(ymin, pCurve->y[0]);
					ymax = qMax(ymax, pCurve->y[0]);
				}
			}
		}
		yo=0.0;

		if (qAbs((ymin-ymax)/ymin)<0.001)
		{
			if(qAbs(ymin)<0.00001) ymax = 1.0;
			else
			{
				ymax = 2.0 * ymin;
				if(ymax < ymin)
				{
					double tmp = ymax;
					ymax = ymin;
					ymin = tmp;
				}
			}
		}

		if(m_h<=0.0) return false;

		if (!m_bYInverted)
		{
			m_scaley   = -(ymax-ymin)/m_h;
		}
		else
		{
			m_scaley   =  (ymax-ymin)/m_h;
		}

		//try to set an automatic scale for Y Axis
		setAutoYUnit();
	}
	else
	{
		//scales are set manually
		if(m_h<=0) return false;

		if (!m_bYInverted)
		{
			m_scaley   = -(ymax-ymin)/m_h;
		}
		else
		{
			m_scaley   =  (ymax-ymin)/m_h;
		}

		if (yunit<1.0)
		{
			exp_y = (int)log10(yunit*1.00001)-1;
			exp_y = qMax(-4, exp_y);
		}
		else  exp_y = (int)log10(yunit*1.00001);

	}

	//graph center position
	int Yg = (m_rCltRect.top() + m_rCltRect.bottom())/2;
	// curves center position
	int Yc = (int)((ymin+ymax)/2.0/m_scaley);
	// center graph in drawing rectangle
	m_ptoffset.ry() = (Yg-Yc);

	return true;
}

void Graph::setYMajGrid(bool const &state, QColor const &clr, int const &style, int const &width)
{
	m_bYMajGrid = state;
	m_YMajClr   = clr;
	m_YMajStyle = style;
	m_YMajWidth = width;
}


void Graph::setYMajGrid(bool const &bGrid)
{
	m_bYMajGrid = bGrid;
}

void Graph::setYMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit)
{
	m_bYMinGrid = state;
	m_bYAutoMinGrid = bAuto;
	m_YMinClr   = clr;
	m_YMinStyle = style;
	m_YMinWidth = width;
	if(unit>0.0) m_YMinorUnit  = unit;
}


void Graph::setYMinGrid(bool const &bGrid)
{
	m_bYMinGrid = bGrid;
}



void Graph::setYVariable(int const & Y)
{
	m_Y = Y;
}


int Graph::xToClient(double x)
{
	return (int)(x/m_scalex + m_ptoffset.x());
}



int Graph::yToClient(double y)
{
	return (int)(y/m_scaley + m_ptoffset.y());
}



Curve*  Graph::getClosestPoint(const double &x, const double &y, double &xSel, double &ySel, int &nSel)
{
	static int i, n1;
	static double dist, dmax, x1, y1;
	dmax = 1.e40;
	Curve *pOldCurve, *pCurveSel;
	pCurveSel = NULL;
	
	for(i=0; i<m_oaCurves.size(); i++)
	{
		pOldCurve = m_oaCurves.at(i);
		pOldCurve->closestPoint(x, y, x1, y1, dist, n1);
		if(dist<dmax)
		{
			dmax = dist;
			xSel = x1;
			ySel = y1;
			pCurveSel = pOldCurve;
			nSel = i;
		}
	}
	return pCurveSel;
}


Curve* Graph::getCurvePoint(const int &xClt, const int &yClt,int &nSel)
{
	static int i, n, xc, yc;
	static double dist, x1, y1, x,y;
	Curve *pOldCurve;

	x= clientTox(xClt);
	y= clientToy(yClt);
	for(i=0; i<m_oaCurves.size(); i++)
	{
		pOldCurve = m_oaCurves.at(i);
		pOldCurve->closestPoint(x, y, x1, y1, dist, n);

		xc = xToClient(x1);
		yc = yToClient(y1);

		if((xClt-xc)*(xClt-xc) + (yClt-yc)*(yClt-yc) <16)//sqrt(16) pixels distance
		{
			nSel = n;
			return pOldCurve;
		}
	}
	nSel = -1;
	return  NULL;
}


bool Graph::selectPoint(QString const &CurveName, int sel)
{
	QString str;
	Curve *pCurve = NULL;
	
	if(sel<0) 
	{
//		pCurve->SetSelected(-1);
		return false;				
	}		
	
	for(int i=0; i<m_oaCurves.size(); i++)
	{
		pCurve = m_oaCurves.at(i);
		pCurve->curveName(str);
		if(str == CurveName)
		{
			if(sel>pCurve->count())
			{
				pCurve->setSelected(-1);
				return false;				
			}
			else
			{
				pCurve->setSelected(sel);
				return true;
			}
		}
	}
//	pCurve->SetSelected(-1);
	return false;
}



void Graph::deselectPoint()
{
	Curve *pCurve;
	for(int i=0; i<m_oaCurves.size(); i++)
	{
		pCurve = m_oaCurves.at(i);
		pCurve->setSelected(-1);
	}
}







