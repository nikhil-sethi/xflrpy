/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QPushButton>
#include <QPaintEvent>


class ColorMenuBtn : public QPushButton
{
    Q_OBJECT

    public:
        ColorMenuBtn(QWidget *pParent = nullptr, bool bShowLeft=false);
        ~ColorMenuBtn();

        void paintEvent (QPaintEvent * pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;

        void showLeft() {m_bShowLeft=true;}

        QColor const &color() const {return m_Color;}
        void setColor(QColor const & color);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}

        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}


    signals:
        void clickedCB(QColor);

    private slots:
        void onColorChanged(QColor newclr);

    private:
        bool m_bIsCurrent;
        bool m_bMouseHover;
        bool m_bHasBackGround;
        QColor m_Color;
        bool m_bShowLeft;
        static QColor s_BackgroundColor;
};




