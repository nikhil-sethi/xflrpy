/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/



#pragma once

#include <QComboBox>
#include <xflcore/linestyle.h>

class LineDelegate;

class LineCbBox : public QComboBox
{
    public:
        LineCbBox(int nItem, QWidget *pParent=nullptr);

        QSize sizeHint() const override;
        void paintEvent (QPaintEvent *pEvent) override;

        void setLine(Line::enumLineStipple style, int width, QColor color, Line::enumPointStyle pointStyle);
        void setLine(const LineStyle &ls);

        bool points() const {return m_LineStyle.m_Symbol!=Line::NOSYMBOL;}

    private:
        LineStyle m_LineStyle;

};


