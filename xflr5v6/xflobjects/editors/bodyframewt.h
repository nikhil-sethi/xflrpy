/****************************************************************************

    BodyFrameWt Class
    Copyright (C)  André Deperrois

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


#include <xflwidgets/view/section2dwt.h>

class Body;

class BodyFrameWt : public Section2dWt
{
    Q_OBJECT

    friend class GL3dBodyDlg;

    public:
        BodyFrameWt(QWidget *pParent=nullptr, Body *pBody=nullptr);

        void setBody(Body *pBody);
        void drawFrameLines();
        void drawFramePoints();
        void drawScaleLegend(QPainter &painter);

        void setScale() override;
        void paintEvent(QPaintEvent *event) override;
        void resizeEvent (QResizeEvent *event) override;

        int highlightPoint(Vector3d real) override;
        int selectPoint(Vector3d real) override;
        void dragSelectedPoint(double x, double y) override;
        void createActions() override;

    signals:
        void pointSelChanged();

    private slots:
        void onInsertPt() override;
        void onRemovePt() override;
        void onScaleFrame();
        void onShowCurFrameOnly();

    private:
        Body *m_pBody;
        QAction *m_pShowCurFrameOnly;
        static bool s_bCurFrameOnly;

};

