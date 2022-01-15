/****************************************************************************

    OpPointWidget Class
    Copyright (C) 2016-2016 André Deperrois 

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
#include <QObject>
#include <QWidget>

#include <xflgraph/graph.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects2d/oppoint.h>
#include <xflcore/linestyle.h>

class XDirect;

class OpPointWt : public QWidget
{
    Q_OBJECT
    friend class XDirect;
    friend class XDirectTileWidget;
    friend class XDirectStyleDlg;
    friend class MainFrame;
    public:
        OpPointWt(QWidget *parent = nullptr);

    public slots:
        void onXDirectStyle();
        void onShowNeutralLine();
        void onResetFoilScale();

        void onShowPressure(bool bPressure);
        void onShowBL(bool bBL);

        void onGraphSettings();

        void setNeutralLineColor(QColor clr){m_NeutralStyle.m_Color = clr;}
        QColor neutralLineColor() const {return m_NeutralStyle.m_Color;}
        void setGraph(Graph* pGraph){m_pCpGraph = pGraph;}
        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame=pMainFrame;}
        static void setXDirect(XDirect *pXDirect) {s_pXDirect=pXDirect;}

    signals:
        void graphChanged(Graph *);

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void keyReleaseEvent(QKeyEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void mouseDoubleClickEvent(QMouseEvent *pEvent) override;


    private:
        void resetGraphScale();
        void setFoilScale();
        void paintOpPoint(QPainter &painter);
        void paintGraph(QPainter &painter);

        void paintPressure(QPainter &painter, double scalex, double scaley);
        void paintBL(QPainter &painter, OpPoint* pOpPoint, double scalex, double scaley);

        void showPressure(bool bPressure){m_bPressure = bPressure;}
        void showBL(bool bBL){m_bBL = bBL;}

        Vector3d mousetoReal(QPoint const &point) const;

    private:
        static MainFrame *s_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
        static XDirect *s_pXDirect;

        double m_fScale, m_fYScale;
        QPointF m_FoilOffset;

        LineStyle m_BLStyle;             /**< the index of the style used to draw the boundary layer */
        LineStyle m_PressureStyle;       /**< the index of the style used to draw the pressure arrows*/
        LineStyle m_NeutralStyle;        /**< the index of the style used to draw the neutral line */

        bool m_bTransFoil;
        bool m_bTransGraph;
        bool m_bAnimate;
        bool m_bBL;                /**< true if the Boundary layer shoud be displayed */
        bool m_bPressure;          /**< true if the pressure distirbution should be displayed */
        bool m_bNeutralLine;

        bool m_bXPressed;                  /**< true if the X key is pressed */
        bool m_bYPressed;                  /**< true if the Y key is pressed */

        Graph *m_pCpGraph;

        QPoint m_LastPoint;
};

