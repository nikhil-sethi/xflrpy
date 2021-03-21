/****************************************************************************

    GraphDlg  Classes
    Copyright (C) 2008-2019 Andre Deperrois

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

#ifndef GRAPHDLG_H
#define GRAPHDLG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>

#include <graph/graph.h>

class DoubleEdit;
class IntEdit;
class ColorButton;
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
    void onAutoMinGrid();
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
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);
    void reject();

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

    QListWidget *m_pctrlXSel, *m_pctrlYSel;

    QDialogButtonBox *m_pButtonBox;
    QPushButton *m_pctrlTitleButton, *m_pctrlLabelButton;
    TextClrBtn*m_pctrlTitleClr, *m_pctrlLabelClr;
    QLabel *m_pctrlTitleLabel, *m_pctrlLabelLabel;

    QCheckBox *m_pctrlXAuto, *m_pctrlYAuto, *m_pctrlYInverted;
    DoubleEdit *m_pctrlXMin, *m_pctrlXMax, *m_pctrlXOrigin,*m_pctrlXUnit;
    DoubleEdit *m_pctrlYMin, *m_pctrlYMax, *m_pctrlYOrigin,*m_pctrlYUnit;

    QCheckBox *m_pctrlXMajGridShow, *m_pctrlYMajGridShow, *m_pctrlXMinGridShow, *m_pctrlYMinGridShow;
    QCheckBox *m_pctrlAutoXMinUnit, *m_pctrlAutoYMinUnit;
    LineBtn *m_pctrlAxisStyle, *m_pctrlXMajGridStyle, *m_pctrlYMajGridStyle, *m_pctrlXMinGridStyle, *m_pctrlYMinGridStyle;
    DoubleEdit *m_pctrlXMinorUnit, *m_pctrlYMinorUnit;

    QCheckBox *m_pctrlGraphBorder;
    ColorButton *m_pctrlGraphBack;
    LineBtn *m_pctrlBorderStyle;
    IntEdit *m_pctrlMargin;

    QFont *m_pTitleFont, *m_pLabelFont;

    bool m_bApplied;

public:

private:
    Graph *m_pGraph;
    Graph m_SaveGraph;
    int m_XSel, m_YSel;

    bool m_bVariableChanged;
    static int s_iActivePage;

};

#endif // GRAPHDLG_H
