/****************************************************************************

    LinePicker Class
    Copyright (C) André Deperrois

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

#include <QCheckBox>

#include <xflcore/linestyle.h>


class LineBtn;
class LineCbBox;
class LineDelegate;

class LinePickerWt : public QWidget
{
    Q_OBJECT
    public:
        LinePickerWt(QWidget *pParent=nullptr);

        void enableBoxes(bool bEnable);
        void fillBoxes();

        void setPointStyle(int pointStyle);
        void setLineStipple(int lineStyle);
        void setLineWidth(int width);
        void setLineColor(QColor color);

        LineStyle const &ls2()      const {return m_LineStyle;}
        LineStyle const &theStyle() const {return m_LineStyle;}

        void setTheStyle(LineStyle const &ls) {m_LineStyle = ls;}

        void setTheStyle(Line::enumLineStipple stipple, int w, const QColor &clr, Line::enumPointStyle pointstyle)
        {
            m_LineStyle.m_Stipple = stipple;
            m_LineStyle.m_Width = w;
            m_LineStyle.m_Color = clr;
            m_LineStyle.m_Symbol = pointstyle;
        }


    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onPointStyle(int val);
        void onLineStyle(int val);
        void onLineWidth(int val);
        void onLineColor();

    signals:
        void styleChanged(LineStyle) const;

    private:
        LineBtn *m_plbLineColor;
        LineCbBox *m_plcbPointStyle, *m_plcbLineWidth, *m_plcbLineStyle;
        QCheckBox *m_pchFlowDownStyle;

        bool m_bAcceptPointStyle;

        LineStyle m_LineStyle;
        LineDelegate *m_pPointStyleDelegate, *m_pLineStyleDelegate, *m_pLineWidthDelegate;
};


