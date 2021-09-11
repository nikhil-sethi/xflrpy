/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QStringList>
#include <QPushButton>
#include <QSlider>
#include <QMenu>

#include <xflwidgets/color/colorbtn.h>
#include <xflwidgets/line/linebtn.h>


class TextClrBtn;

class ColorPicker : public QWidget
{
    Q_OBJECT

    public:
        ColorPicker(QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;

        void initDialog(QColor color);
        void acceptColor();

        QColor theColor() const {return m_Color;}

        static double saturationFactor() {return s_SatFactor;}
        static void setSaturationFactor(double s) {s_SatFactor=s;}
        static void setColorList(QStringList const&colorlist);

        static QColor randomColor(bool bLightColor);

        static void setBackgroundColor(QColor const &clr) {s_BackgroundColor=clr;}
        static void setTextColor(QColor const &clr) {s_TextColor=clr;}
        static void setDontUseNativeMacDlg(bool bDontUse) {s_bDontUseNativeDlg=bDontUse;}

    signals:
        void colorChanged(QColor);

    private:
        void setActiveColor(QColor const &clr);
        void setupLayout();
        void setSaturation();

    public slots:
        void onClickedCB(QColor clr);
        void onOtherColor();
        void onAlphaReleased();
        void onSaturationMoved(int val);

    private:
        QVector<ColorBtn*> m_pCB;
        TextClrBtn *m_pColorButton;

        QSlider *m_pslSaturation;
        QSlider *m_pslAlpha;

        QColor m_Color;

        bool m_bChanged;

        static int s_nRows;
        static int s_nCols;

        static QStringList s_ColourList;
        static QStringList s_ColourNames;

        static double s_SatFactor;

        static bool s_bDontUseNativeDlg;
        static QColor s_BackgroundColor;
        static QColor s_TextColor;
};


