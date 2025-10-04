/****************************************************************************

    ExponentialSlider class
    Copyright (C) 2016 Andre Deperrois xflr5@yahoo.com

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

#ifndef EXPONENTIALSLIDER_H
#define EXPONENTIALSLIDER_H
#include <QSlider>

class ExponentialSlider : public QSlider
{
public:
    ExponentialSlider(QWidget * pParent = nullptr);
    ExponentialSlider(Qt::Orientation orientation, QWidget * pParent = nullptr);
    ExponentialSlider(bool bCentered, double expo, Qt::Orientation orientation, QWidget * pParent = nullptr);

    double expValue() const;
    void setExpValue(double expValue);

    void setExponential(double expo){m_expo = expo;}
    void setCentered(bool bCentered){m_bCentered = bCentered;}

private:
    double m_expo;
    bool m_bCentered;
};

#endif // EXPONENTIALSLIDER_H
