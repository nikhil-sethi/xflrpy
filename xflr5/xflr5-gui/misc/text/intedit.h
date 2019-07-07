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


#ifndef INTEDIT_H
#define INTEDIT_H

#include <QIntValidator>
#include <QLineEdit>
#include <QKeyEvent>


class IntEdit : public QLineEdit
{
public:
    IntEdit(QWidget *pParent=NULL);
    IntEdit(int val, QWidget *pParent=NULL);

    ~IntEdit() {delete m_pDV;}

    void focusOutEvent ( QFocusEvent * event );
    void keyPressEvent(QKeyEvent *event);
    QSize sizeHint() const;


    int value(){return m_Value;}
    void setValue(int val);


    void setValueNoFormat(int val);

    void formatValue();
    int readValue();
    void setMin(int min) {m_pDV->setBottom(min);}
    void setMax(int max) {m_pDV->setTop(max);}



public:
    QIntValidator *m_pDV;
    int m_Value;//we need to store a full precision value, irrespective of the display
};

#endif // IntEdit_H
