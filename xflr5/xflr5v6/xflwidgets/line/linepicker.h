/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QStringList>
#include <QPushButton>
#include <QMenu>

#include <xflwidgets/line/linebtn.h>


#define NCOLORROWS 24
#define NCOLORCOLS 3

class TextClrBtn;

class LinePicker : public QWidget
{
    Q_OBJECT

    public:
        LinePicker(QWidget *pParent=nullptr);

        void initDialog(LineStyle const &ls);

        void setColor(QColor const &clr);

        static void setColorList(QStringList const&colorlist, QStringList const &colornames);

        static QColor randomColor(bool bLightColor);

        static void setBackgroundColor(QColor clr) {s_BackgroundColor=clr;}
        static void setTextColor(QColor clr) {s_TextColor=clr;}
        static void setDontUseNativeMacDlg(bool bDontUse) {s_bDontUseNativeDlg=bDontUse;}

    signals:
        void colorChanged(QColor);

    public slots:
        void onClickedLB(LineStyle ls);

        void onOtherColor();

    private:
        void setupLayout();

    private:
        LineBtn m_lb[NCOLORROWS*NCOLORCOLS];

        TextClrBtn *m_pColorButton;

        LineStyle m_theStyle;

        static QStringList s_LineColorList;
        static QStringList s_LineColorNames;
        static QColor s_BackgroundColor;
        static QColor s_TextColor;
        static bool s_bDontUseNativeDlg;
};


