/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QIntValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <xflwidgets/customwts/numedit.h>


class IntEdit : public NumEdit
{
    public:
        IntEdit(QWidget *pParent=nullptr);
        IntEdit(int val, QWidget *pParent=nullptr);

        ~IntEdit() {delete m_pIV;}

        void keyPressEvent(QKeyEvent *pEvent) override;

        int value() const {return m_Value;}
        void setValue(int val);

        void initialize(int value);
        void setValueNoFormat(int val);

        void formatValue() override;
        void readValue() override;
        void setMin(int min) {m_pIV->setBottom(min);}
        void setMax(int max) {m_pIV->setTop(max);}


    public:
        QIntValidator *m_pIV;
        int m_Value;

};








