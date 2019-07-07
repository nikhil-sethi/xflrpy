/****************************************************************************

    WingWidget Class
        Copyright (C) 2015-2019 Andre Deperrois

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

#ifndef WINGWIDGET_H
#define WINGWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QPixmap>


class Plane;
class PlaneOpp;
class Graph;
class Miarex;

class WingWidget : public QWidget
{
    Q_OBJECT
public:
    WingWidget(QWidget *pParent = nullptr);
    ~WingWidget();

    void contextMenuEvent (QContextMenuEvent *pEvent);
    void keyPressEvent(QKeyEvent *pEvent);
    void mouseDoubleClickEvent (QMouseEvent *pEvent);
    void mouseMoveEvent(QMouseEvent *pEvent);
    void mousePressEvent(QMouseEvent *pEvent);
    void mouseReleaseEvent(QMouseEvent *pEvent);
    void paintEvent(QPaintEvent *pEvent);
    void resizeEvent (QResizeEvent *pEvent);
    void wheelEvent (QWheelEvent *pEvent);

    void setWingScale();
    void setWingGraph(Graph *pGraph);

private:
    void paintXCmRef(QPainter &painter, QPointF ORef, double scale);
    void paintXCP(QPainter &painter, QPointF ORef, double scale);
    void paintXTr(QPainter &painter, QPointF ORef, double scale);
    void paintWing(QPainter &painter, QPointF ORef, double scale);


signals:

public slots:
    void onResetWingScale();

public:

    static Miarex *s_pMiarex;

private:

    bool m_bTrans;

    double m_WingScale;
    QPointF m_ptOffset;              /**< client offset position for wing display */
    QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
    Graph * m_pGraph;
    QPixmap m_PixText;
};

#endif // WINGWIDGET_H
