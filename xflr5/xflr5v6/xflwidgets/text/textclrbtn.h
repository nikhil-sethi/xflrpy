/****************************************************************************

    TextBtn Class
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


#ifndef TEXTBTN_H
#define TEXTBTN_H

#include <QAbstractButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QFont>
#include <QColor>

class TextClrBtn : public QAbstractButton
{
    Q_OBJECT

public:
    TextClrBtn(QWidget *parent = nullptr);

    void setFont(QFont const &font);
    void setTextColor(QColor const & textColor);
    void setBackgroundColor(QColor const & textColor);
    QColor textColor() const {return m_TextColor;}


signals:
    void clickedTB();

public:
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;

private:
    QColor m_TextColor;
    QColor m_BackgroundColor;
    QFont m_TextFont;
};

#endif // TEXTBTN_H
