/****************************************************************************

    XDirectTileWidget Class
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


#include <QGridLayout>

#include "graphtilewt.h"


class OpPointWt;

class XDirectTileWidget : public GraphTileWidget
{
    Q_OBJECT

    public:
        XDirectTileWidget(QWidget *pParent = nullptr);
        ~XDirectTileWidget();

    public:
        void connectSignals() override;
        void adjustLayout() override;
        void setGraphList(QVector<Graph *> pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation =Qt::Horizontal) override;

        OpPointWt *opPointWidget() {return m_pOpPointWidget;}

    public slots:
        void onResetCurGraphScales();

    private:
        void setupMainLayout() override;

        //    QStackedWidget *m_pLegendStack;
        OpPointWt *m_pOpPointWidget;

        QGridLayout *m_pMainGridLayout;
};

