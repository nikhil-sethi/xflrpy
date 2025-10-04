/****************************************************************************

    FoilDesignWt Class
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

#include <QVector>
#include <xflwidgets/view/section2dwt.h>

class Foil;
class SplineFoil;

class FoilDesignWt : public Section2dWt
{
    public:
        FoilDesignWt(QWidget *pParent=nullptr);

        void setObjects(Foil *pBufferFoil, SplineFoil *pSF);

        void setScale() override;
        void paintEvent(QPaintEvent *event) override;
        void resizeEvent (QResizeEvent *event) override;

        int highlightPoint(Vector3d real) override;
        int selectPoint(Vector3d real) override;
        void dragSelectedPoint(double x, double y) override;

    public slots:
        void onInsertPt() override;
        void onRemovePt() override;

    private:
        void paintSplines(QPainter &painter);
        void paintFoils(QPainter &painter);
        void paintLegend(QPainter &painter);
        void paintLECircle(QPainter &painter);


    private:
        SplineFoil *m_pSF;          /**< a pointer to the SplineFoil object */
        Foil *m_pBufferFoil;

    public:
        bool m_bLECircle;           /**< true if the leading edge circle should be displayed >*/
        double m_LERad;             /**< the radius of the leading edge circle to draw >*/

};

