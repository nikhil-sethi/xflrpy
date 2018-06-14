/****************************************************************************

	OpPointWidget Class
	Copyright (C) 2016-2016 Andre Deperrois 

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

#ifndef OPPOINTWIDGET_H
#define OPPOINTWIDGET_H

#include <QObject>
#include <QWidget>
#include <graph/graph.h>
#include <objects/objects3d/vector3d.h>
#include <objects/objects2d/OpPoint.h>

class OpPointWidget : public QWidget
{
	Q_OBJECT
	friend class XDirect;
	friend class XDirectTileWidget;
	friend class XDirectStyleDlg;
	friend class MainFrame;
public:
	OpPointWidget(QWidget *parent = 0);

public slots:
	void onXDirectStyle();
	void onShowNeutralLine();
//	void onShowPanels();
	void onResetFoilScale();

	void onShowPressure(bool bPressure);
	void onShowBL(bool bBL);

	void onGraphSettings();

	void setNeutralLineColor(QColor clr){m_crNeutralColor = clr;}
	QColor neutralLineColor(){return m_crNeutralColor;}
	void setGraph(Graph* pGraph){m_pCpGraph = pGraph;}
	void loadSettings(QSettings *pSettings);
	void saveSettings(QSettings *pSettings);

signals:
	void graphChanged(Graph *);

protected:
	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);


private:
	void resetGraphScale();
	void setFoilScale();
	void paintOpPoint(QPainter &painter);
	void paintGraph(QPainter &painter);

	void paintPressure(QPainter &painter, double scalex, double scaley);
	void paintBL(QPainter &painter, OpPoint* pOpPoint, double scalex, double scaley);

	void showPressure(bool bPressure){m_bPressure = bPressure;}
	void showBL(bool bBL){m_bBL = bBL;}

	Vector3d mousetoReal(QPoint point);

	static void *s_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */

	double m_fScale, m_fYScale;
	QPointF m_FoilOffset;

	QColor m_crBLColor;         /**< the color used to draw the boundary layer */
	QColor m_crPressureColor;   /**< the color used to draw the pressure arrows */
	QColor m_crNeutralColor;    /**< the color used to draw the neutral line */
	int m_iBLStyle;             /**< the index of the style used to draw the boundary layer */
	int m_iBLWidth;             /**< the width of the line used to draw the boundary layer */
	int m_iPressureStyle;       /**< the index of the style used to draw the pressure arrows*/
	int m_iPressureWidth;       /**< the width of the line used to draw the pressure arrows */
	int m_iNeutralStyle;        /**< the index of the style used to draw the neutral line */
	int m_iNeutralWidth;        /**< the width of the line used to draw the neutral line */

	bool m_bTransFoil;
	bool m_bTransGraph;
	bool m_bAnimate;
	bool m_bBL;                /**< true if the Boundary layer shoud be displayed */
	bool m_bPressure;          /**< true if the pressure distirbution should be displayed */
	bool m_bNeutralLine;
//	bool m_bShowPanels;
	bool m_bXPressed;                  /**< true if the X key is pressed */
	bool m_bYPressed;                  /**< true if the Y key is pressed */

	Graph *m_pCpGraph;

	QPoint m_LastPoint;
//	QRect m_rGraphRect;
};

#endif // OPPOINTWIDGET_H
