/****************************************************************************

    LegendWidget Class
        Copyright (C) 2015 André Deperrois 

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

#pragma once

#include <QWidget>
#include <xflcore/core_enums.h>

class MainFrame;
class Miarex;
class XDirect;
class GraphTileWidget;
class Graph;
class WPolar;

class LegendWt : public QWidget
{
    Q_OBJECT
    public:
        LegendWt(QWidget *pParent = nullptr);
        ~LegendWt();

        void keyPressEvent(QKeyEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        QSize sizeHint() const override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

        void setGraph(Graph*pGraph){m_pGraph = pGraph;}

    public:
        GraphTileWidget *m_pGraphTileWt;             /**< a void pointer to the instance of the GraphTileWidget object. */

        static MainFrame *s_pMainFrame;   /**< a void pointer to the instance of the MainFrame object. */
        static Miarex *s_pMiarex;      /**< a void pointer to the instance of the QMiarex object. */
        static XDirect *s_pXDirect;     /**< a void pointer to the instance of the QXDirect object. */

        void setMiarexView(xfl::enumMiarexViews eMiarexView);

    private:
        void drawWPolarLegend(QPainter &painter, QPointF place, int bottom);
        void drawPOppGraphLegend(QPainter &painter, QPointF place, double bottom);
        void drawStabTimeLegend(QPainter &painter, const Graph *pGraph, QPointF place, int bottom);
        void drawCpLegend(QPainter &painter, const Graph *pGraph, QPointF place, int bottom);
        void drawPolarLegend(QPainter &painter, QPointF place, int bottom);
        bool isFiltered(WPolar *pWPolar);

    private:
        Graph const *m_pGraph;

        xfl::enumMiarexViews m_MiarexView;
        QPointF m_LegendPosition;
        QPointF m_PointDown;
        bool m_bTrans;
};

