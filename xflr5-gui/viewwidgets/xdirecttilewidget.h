/****************************************************************************

    XDirectTileWidget Class
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

#ifndef XDIRECTTILEWIDGET_H
#define XDIRECTTILEWIDGET_H

#include <QStackedWidget>
#include <QGridLayout>

#include "graphtilewidget.h"


class OpPointWidget;

class XDirectTileWidget : public GraphTileWidget
{
    Q_OBJECT

public:
    XDirectTileWidget(QWidget *pParent = nullptr);
    ~XDirectTileWidget();

public:
    void connectSignals();
    void adjustLayout();
    void setGraphList(QVector<Graph *> pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation =Qt::Horizontal);

    OpPointWidget *opPointWidget() {return m_pOpPointWidget;}

public slots:
    void onResetCurGraphScales();

private:
    void setupMainLayout();

    //    QStackedWidget *m_pLegendStack;
    OpPointWidget *m_pOpPointWidget;

    QGridLayout *m_pMainGridLayout;
};

#endif // XDIRECTTILEWIDGET_H
