/****************************************************************************

    GraphDlg  Classes
    Copyright (C) 2008-2019 André Deperrois

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

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>

#include <xflgraph/graph.h>

class DoubleEdit;
class IntEdit;
class ColorBtn;
class LineBtn;
class TextClrBtn;

class GraphDlg : public QDialog
{
    Q_OBJECT
    friend class Graph;


    public:
        GraphDlg(QWidget *pParent);
        void setControls();
        void setGraph(Graph *pGraph);
        Graph* graph() {return m_pGraph;}
        bool bVariableChanged(){return m_bVariableChanged;}

        int &XSel(){return m_XSel;}
        int &YSel(){return m_YSel;}

        static void setActivePage(int iPage);



    private slots:
        void onButton(QAbstractButton *pButton);
        void onOK();
        void onTitleFont();
        void onLabelFont();
        void onTitleColor();
        void onLabelColor();
        void onRestoreParams();
        void onAutoX();
        void onAutoY();
        void onAxisStyle();
        void onMargin();
        void onXMajGridStyle();
        void onXMinGridStyle();
        void onYMajGridStyle();
        void onYMinGridStyle();
        void onXMajGridShow(int state);
        void onYMajGridShow(int state);
        void onXMinGridShow(int state);
        void onYMinGridShow(int state);
        void onGraphBorder(int state);
        void onGraphBackColor();
        void onBorderStyle();
        void onYInverted();
        void onActivePage(int index);
        void onVariableChanged();


    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void reject() override;

        void setupLayout();
        void setButtonColors();
        void setApplied(bool bApplied);
        void connectSignals();
        void applyChanges();

        void fillVariableList();

        QWidget *m_pParent;

        QTabWidget *m_pTabWidget;
        QWidget *m_pScalePage, *m_pFontPage, *m_pGridPage;
        QWidget *m_pVariablePage;

        QListWidget *m_plwXSel, *m_plwYSel;

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbTitleButton, *m_ppbLabelButton;
        TextClrBtn*m_ptcbTitleClr, *m_ptcbLabelClr;
        QLabel *m_plabTitleLabel, *m_plabLabel;

        QCheckBox *m_pchXAuto, *m_pchYAuto, *m_pchYInverted;
        DoubleEdit *m_pdeXMin, *m_pdeXMax, *m_pdeXOrigin, *m_pdeXUnit;
        DoubleEdit *m_pdeYMin, *m_pdeYMax, *m_pdeYOrigin, *m_pdeYUnit;

        QCheckBox *m_pchXMajGridShow, *m_pchYMajGridShow, *m_pchXMinGridShow, *m_pchYMinGridShow;
        LineBtn *m_plbAxisStyle, *m_plbXMajGridStyle, *m_plbYMajGridStyle, *m_plbXMinGridStyle, *m_plbYMinGridStyle;

        QCheckBox *m_pchGraphBorder;
        ColorBtn *m_pcbGraphBack;
        LineBtn *m_plbBorderStyle;
        IntEdit *m_pieMargin;

        QFont *m_pTitleFont, *m_pLabelFont;

        bool m_bApplied;



    private:
        Graph *m_pGraph;
        Graph m_SaveGraph;
        int m_XSel, m_YSel;

        bool m_bVariableChanged;
        static int s_iActivePage;

};

