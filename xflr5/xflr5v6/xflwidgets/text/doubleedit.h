/****************************************************************************

    DoubleEdit Class
    Copyright (C) 2013 Andre Deperrois 

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

#include <QDoubleValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <xflwidgets/text/numedit.h>


class DoubleEdit : public NumEdit
{
    public:
        DoubleEdit(QWidget *pParent=nullptr);
        DoubleEdit(double val, int decimals=-1, QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;


        double value() const {return m_Value;}
        float valuef() const {return float(m_Value);}
        void setValue(double val);
        void setValuef(float val);

        void initialize(double value, int decimals);

        void setValueNoFormat(double val);

        void formatValue() override;
        void readValue() override;
        void setMin(double min) {m_MinValue=min;}
        void setMax(double max) {m_MaxValue=max;}
        void setRange(double min, double max) {m_MinValue=min; m_MaxValue=max;}


        void setDigits(int decimals) {m_Digits = decimals;}
        int digits() const {return m_Digits;}

        void setNotation(QDoubleValidator::Notation notation) {m_Notation = notation;}


    public:
        double m_Value;//we need to store a full precision value, irrespective of the display
        double m_MinValue;
        double m_MaxValue;
        int m_Digits;
        QDoubleValidator::Notation m_Notation;

};






