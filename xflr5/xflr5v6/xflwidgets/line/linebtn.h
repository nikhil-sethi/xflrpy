/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/



#pragma once


#include <QAbstractButton>

#include <xflcore/linestyle.h>


class LineBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        LineBtn(QWidget *parent = nullptr);
        LineBtn(LineStyle ls, QWidget *parent = nullptr);

        void setTheStyle(const LineStyle &ls);
        void setTheStyle(Line::enumLineStipple style, int width, const QColor &color, Line::enumPointStyle pointstyle);

        void setColor(QColor const &color);
        void setStipple(Line::enumLineStipple stipple);
        void setWidth(int width);
        void setPointStyle(Line::enumPointStyle pointstyle);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}

        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}

        LineStyle const &theLineStyle() const {return m_LineStyle;}

        QColor btnColor() const {return m_LineStyle.m_Color;}
        void setBtnColor(QColor clr) {m_LineStyle.m_Color=clr;}

        Line::enumLineStipple btnStyle()      const {return m_LineStyle.m_Stipple;}
        void setBtnStyle(Line::enumLineStipple iStyle) {m_LineStyle.m_Stipple = iStyle;}

        int btnWidth()      const {return m_LineStyle.m_Width;}
        void setBtnWidth(int iWidth) {m_LineStyle.m_Width=iWidth;}

        Line::enumPointStyle btnPointStyle() const {return m_LineStyle.m_Symbol;}
        void setBtnPointStyle(Line::enumPointStyle iSymbol) {m_LineStyle.m_Symbol=iSymbol;}

        static void setBackgroundColor(QColor const &clr) {s_BackgroundColor=clr;}

    signals:
        void clickedLB(LineStyle);

    public:
        void mousePressEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

        bool event(QEvent* pEvent) override;


    private:
        bool m_bIsCurrent;
        bool m_bMouseHover;
        bool m_bHasBackGround;
        LineStyle m_LineStyle;

        static QColor s_BackgroundColor;
};


