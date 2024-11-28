/****************************************************************************

    NumEdit Class
    Copyright (C) 2021 Andre Deperrois

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


#include <QLineEdit>
#include <QKeyEvent>



class NumEdit : public QLineEdit
{
    Q_OBJECT
    public:
        NumEdit(QWidget *pWidget);

    protected:
        void focusOutEvent (QFocusEvent * pEvent) override;
        QSize sizeHint() const override;

        virtual void readValue() = 0;
        virtual void formatValue() = 0;

    signals:
        void valueChanged() const;

    public slots:
        void paste();

};

