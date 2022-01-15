/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#pragma once

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
        void setTextColor(QColor const & TextColor);
        void setBackgroundColor(QColor const & TextColor);
        QColor textColor() const { return m_TextColor;}

        void setContour(bool bContour) {m_bContour=bContour;}
        void setRoundedRect(bool bRound) {m_bRoundedRect=bRound;}

    signals:
        void clickedTB();

    public:
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

    private:
        QColor m_TextColor;
        QColor m_BackgroundColor;
        QFont m_TextFont;

        bool m_bContour;
        bool m_bRoundedRect;
};

