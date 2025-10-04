/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QDoubleValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <xflwidgets/customwts/numedit.h>


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






