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

#include "exponentialslider.h"

#include <cmath>

//caution, use only for "symetrical" sliders ranging from -range to +range

ExponentialSlider::ExponentialSlider(QWidget * pParent)
{
    setParent(pParent);
    setOrientation(Qt::Horizontal);
    m_bCentered = false;
    m_expo = 1.0;
}


ExponentialSlider::ExponentialSlider(Qt::Orientation orientation, QWidget * pParent)
{
    setParent(pParent);
    setOrientation(orientation);
    m_bCentered = false;
    m_expo = 1.0;
}


ExponentialSlider::ExponentialSlider(bool bCentered, double expo, Qt::Orientation orientation, QWidget * pParent)
{
    setParent(pParent);
    setOrientation(orientation);
    m_bCentered = bCentered;
    m_expo = expo;
}


double ExponentialSlider::expValue() const
{
    if(m_bCentered)
    {
        double mid      = double(minimum() + maximum())/2.0;
        double span     = double(maximum() - minimum());
        double halfspan = span/2.0;
        double position = double(value());
        double frac     = (position-mid)/halfspan;

        if(frac>=0.0) return mid + pow( frac, m_expo) * halfspan;
        else          return mid - pow(-frac, m_expo) * halfspan;
    }
    else
    {
        double span     = double(maximum() - minimum());
        double position = double(value());
        double frac     = position/span;
        return pow( frac, m_expo) * span;
    }
}


void ExponentialSlider::setExpValue(double expValue)
{
    if(m_bCentered)
    {
        int mid    = int((minimum() + maximum())/2.0);
        int span   = maximum() - minimum();
        double halfspan = double(span)/2.0;

        double yRel = expValue / halfspan;
        yRel = qMin(yRel,  1.0);
        yRel = qMax(yRel, -1.0);

        double xRel = pow(qAbs(yRel), 1.0/m_expo);

        if(yRel<0.0) xRel = -xRel;

        setValue(mid + int(xRel*halfspan));
    }
    else
    {
        int span   = maximum() - minimum();

        double yRel = expValue / span;
        yRel = qMin(yRel, 1.0);
        yRel = qMax(yRel, 0.0);

        double xRel = pow(yRel, 1.0/m_expo);

        setValue(int(xRel*span));
    }
}


