/****************************************************************************

    LineBtn Class
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



#ifndef LINEBTN_H
#define LINEBTN_H


#include <QAbstractButton>
#include <linestyle.h>

class LineBtn : public QAbstractButton
{
    Q_OBJECT

public:
    LineBtn(QWidget *parent = nullptr);

    void setStyle(int const &lineStyle, int const &width, QColor const & color, const int &pointStyle);
    void setColor(QColor const & color);
    void setStyle(int const &lineStyle);
    void setWidth(int const &width);
    void setPointStyle(int const & pointStyle);

    QColor color()   const {return m_LineStyle.m_Color;}
    int lineStyle()  const {return m_LineStyle.m_Style;}
    int lineWidth()  const {return m_LineStyle.m_Width;}
    int pointStyle() const {return m_LineStyle.m_PointStyle;}

signals:
    void clickedLB();

public:
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;

private:
    LineStyle m_LineStyle;
};

#endif
