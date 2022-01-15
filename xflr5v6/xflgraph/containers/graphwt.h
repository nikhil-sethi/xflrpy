/****************************************************************************

    GraphWt Class
        Copyright (C) André Deperrois

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

#include <xflcore/displayoptions.h>

class Graph;

class GraphWt : public QWidget
{
    Q_OBJECT

    friend class XFoilAnalysisDlg;
    friend class LLTAnalysisDlg;
    friend class BatchDlg;
    friend class XDirectTileWidget;
    friend class MiarexTileWidget;

    public:
        GraphWt(QWidget *pParent=nullptr);
        Graph *graph(){return m_pGraph;}

        virtual void setGraph(Graph *pGraph);
        void setTitles(QString &Title, QPoint &Place);

        void showLegend(bool bShow) {m_bDrawLegend = bShow;}
        void setLegendPosition(QPoint const &pos) {m_LegendOrigin = pos;}

        void setOverlayedRect(bool bShow, double tlx, double tly, double brx, double bry);

        void setDefaultSize(QSize sz) {m_DefaultSize = sz;}
        void setMinSize(QSize sz){m_MinSize = sz;}

        static QColor const& textFontColor() {return s_TextColor;}
        static void setTextColor(QColor textcolor) {s_TextColor=textcolor;}
        static void setBackgroundColor(QColor bkcolor) {s_BackgroundColor=bkcolor;}

        static void setTextFontStruct(FontStruct const &fntstruct) {s_TextFontStruct=fntstruct;}
        static FontStruct const &textFontStruct() {return s_TextFontStruct;}

    protected:
        void paintEvent(QPaintEvent *pEvent) override;
        void resizeEvent (QResizeEvent *pEvent) override;
        void contextMenuEvent (QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void mouseDoubleClickEvent (QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void wheelEvent (QWheelEvent *pEvent) override;

        QSize sizeHint() const override {return m_DefaultSize;}

    signals:
        void graphChanged(Graph *);
        void graphResized(Graph *);

    public slots:
        void onGraphSettings();
        void onResetGraphScales();

    protected:

        QSize m_DefaultSize;
        QSize m_MinSize;

        QPoint m_TitlePosition;
        QString m_GraphTitle;
        Graph *m_pGraph;

        QPoint m_LegendOrigin;
        bool m_bDrawLegend;

        QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
        bool m_bTransGraph;
        bool m_bXPressed;                  /**< true if the X key is pressed */
        bool m_bYPressed;                  /**< true if the Y key is pressed */

        bool m_bOverlayRectangle;
        QPointF m_TopLeft, m_BotRight; // in graph coordinates; should really be a Vector2d

        static FontStruct s_TextFontStruct;
        static QColor s_TextColor;
        static QColor s_BackgroundColor;
};

