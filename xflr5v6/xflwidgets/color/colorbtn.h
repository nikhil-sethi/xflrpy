/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QAbstractButton>
#include <QPaintEvent>



class ColorPicker;

class ColorBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        ColorBtn(QWidget *pParent = nullptr);

        void paintEvent (QPaintEvent * pEvent ) override;
        QSize sizeHint() const  override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        bool event(QEvent* pEvent) override;

        QColor const &color() const {return m_Color;}
        void setColor(QColor const & color);

        bool isCurrent() const {return m_bIsCurrent;}
        void setCurrent(bool bCurrent) {m_bIsCurrent=bCurrent;}

        bool hasBackGround() const {return m_bHasBackGround;}
        void setBackground(bool bBack) {m_bHasBackGround=bBack;}


    signals:
        void clickedCB(QColor);

    public slots:
        void onSetColor(QColor clr);

    private:
        bool m_bIsCurrent;
        bool m_bMouseHover;
        bool m_bHasBackGround;
        QColor m_Color;
};




