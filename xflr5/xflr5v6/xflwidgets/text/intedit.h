/****************************************************************************

    IntEdit Class
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
#include <QIntValidator>
#include <QLineEdit>
#include <QKeyEvent>

#include <xflwidgets/text/numedit.h>

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


