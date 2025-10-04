/****************************************************************************

    FoilWt Class
    Copyright (C) 2021 André Deperrois

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

class FoilWt : public Section2dWt
{
public:
    FoilWt(QWidget *pParent=nullptr);

    void setBufferFoil(Foil *pBufferFoil);

    void setScale() override;
    void clearFoils() {m_oaFoil.clear();}
    void addFoil(const Foil *pFoil) {if(pFoil) m_oaFoil.append(pFoil);}
    QVector<Foil const*> const &foils() const {return m_oaFoil;}

private:
    void paintSplines(QPainter &painter);
    void paintFoils(QPainter &painter);
    void paintLegend(QPainter &painter);
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent (QResizeEvent *event) override;

    int highlightPoint(Vector3d ) override {return -1;};
    int selectPoint(Vector3d ) override {return -1;};
    void dragSelectedPoint(double , double ) override {};


private:
    Foil *m_pBufferFoil;
    QVector<Foil const*> m_oaFoil;

};

