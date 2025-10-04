/****************************************************************************

    ColorButton Class
    Copyright (C) 2009-2016 Andre Deperrois 

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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QAbstractButton>
#include <QPaintEvent>

class ColorButton : public QAbstractButton
{
public:
    ColorButton(QWidget *pParent = nullptr);

    void paintEvent (QPaintEvent *pEvent);
    QSize sizeHint() const;

    void setColor(QColor const & color);
    QColor const &color() const {return m_Color;}

private:
    QColor m_Color;
};

#endif // COLORBUTTON_H
