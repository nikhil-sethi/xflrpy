/****************************************************************************

	GraphWidget Class
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


#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include "QGraph.h"

class GraphWidget : public QWidget
{
	Q_OBJECT

	friend class XFoilAnalysisDlg;
	friend class LLTAnalysisDlg;
	friend class BatchDlg;
	friend class XDirectTileWidget;
	friend class MiarexTileWidget;

public:
	GraphWidget(QWidget *pParent=NULL);
	QGraph *graph(){return m_pGraph;}

	void setGraph(QGraph *pGraph);
	void setTitles(QString &Title, QPoint &Place);

	bool &bResetCurves(){return m_bResetCurves;}

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);
	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent (QWheelEvent *event);


signals:
	void graphChanged(QGraph *);
	void graphResized(QGraph *);

public slots:
	void onGraphSettings();
	void onResetGraphScales();


private:
	QPoint m_TitlePosition;
	QString m_GraphTitle;
	QGraph *m_pGraph;

	QPoint m_LegendOrigin;
	bool m_bDrawLegend;
	void showLegend(bool bShow);
	void setLegendPosition(QPoint pos);

public:
	static void *s_pMainFrame;
	void *m_pParent;

private:
	QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
	bool m_bTransGraph;
	bool m_bXPressed;                  /**< true if the X key is pressed */
	bool m_bYPressed;                  /**< true if the Y key is pressed */
	bool m_bResetCurves;
};

#endif // GRAPHWIDGET_H
