/****************************************************************************

	QGraph Classes
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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

#ifndef QGraph_H
#define QGraph_H


#include "Graph.h"
#include "Curve.h"
#include <QRect>
#include <QSettings>
#include <QPoint>
#include <QFont>

#define MAXTIMEGRAPHS  4  /**< The max number of graphs available for display in the stability time view. */
#define MAXWINGGRAPHS  5  /**< The max number of graphs available for display in QXDirect. */
#define MAXPOLARGRAPHS 5  /**< The max number of graphs available for display in QXDirect. */
#define MAXGRAPHS      6  /**< The max number of graphs available for display at one time. */

namespace QGRAPH
{
	typedef enum{OPPGRAPH, POLARGRAPH, POPPGRAPH, WPOLARGRAPH, STABTIMEGRAPH, CPGRAPH, INVERSEGRAPH, OTHERGRAPH} enumGraphType;
}

class GRAPHLIBSHARED_EXPORT QGraph : public Graph
{
public: 
	QGraph();
	virtual ~QGraph();

	void drawGraph(QRect const & rect, QPainter &painter);
	void drawGraph(QPainter &painter);
	void drawAxes(QPainter &painter);
	void drawCurve(int nIndex, QPainter &painter);
	void drawLegend(QPainter &painter, QPoint &Place, QFont &LegendFont, QColor &LegendColor, QColor &backColor);
	void drawTitles(QPainter &painter);
	void drawXMinGrid(QPainter &painter);
	void drawYMinGrid(QPainter &painter);
	void drawXMajGrid(QPainter &painter);
	void drawYMajGrid(QPainter &painter);
	void drawXTicks(QPainter &painter);
	void drawYTicks(QPainter &painter);
	void expFormat(double &f, int &exp);
	void exportToFile(QFile &XFile, bool bCSV);
	void highlight(QPainter &painter, Curve *pCurve, int ref);

	void loadSettings(QSettings *pSettings);
	void saveSettings(QSettings *pSettings);
	QPoint getOffset();

	void copySettings(QGraph *pGraph, bool bScales=true);
	void getLabelFont(QFont &labelFont);
	void getTitleFont(QFont &titleFont);
	void setLabelFont(QFont &font);
	void setTitleFont(QFont &font);


	static void setOppHighlighting(bool bHighLight){s_bHighlightPoint = bHighLight;}
	static bool isHighLighting(){return s_bHighlightPoint;}

	QGRAPH::enumGraphType &graphType(){return m_graphType;}

public:
	static void *s_pMainFrame;
	void *m_pParent;

private:
	QFont m_TitleFont;
	QFont m_LabelFont;
	QGRAPH::enumGraphType m_graphType;

	static bool s_bHighlightPoint;       /**< true if the active OpPoint should be highlighted on the polar curve. */

};

#endif
