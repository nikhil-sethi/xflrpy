/****************************************************************************

    WPolarDlg Class
    Copyright (C) 2009-2019 Andre Deperrois

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
/**
 *@file This file contains the definition of the class WPolarDlg which is used to define the data for a WPolar object.
 */

#ifndef WPOLARDLG_H
#define WPOLARDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QRadioButton>
#include <QTableView>
#include <QDialogButtonBox>
#include <QStandardItemModel>

#include <objects/objects3d/wpolar.h>

class Plane;
class DoubleEdit;
class CtrlTableDelegate;

/**
*@class WPolarDlg
*@brief This class provides the interface dialog box which is used to define or to edit the paramaters of a type 1, 2 or 4 polar.

* The class uses a static instance of the WPolar class as the default data. 
* This is so that the next call to the class uses the existing data, and only modifications are required.
* The creation and storage of the new polar object is managed from the calling class QMiarex.
* The rest of the methods and variables is self explanatory and not documented further.
*/
class WPolarDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class MainFrame;

public:
    WPolarDlg(QWidget *pParent=nullptr);
    ~WPolarDlg();

private:

    void connectSignals();
    void enableControls();
    void fillExtraDragList();
    void initDialog(Plane *pPlane, WPolar *pWPolar=nullptr);
    void keyPressEvent(QKeyEvent *event);
    void readExtraDragData();
    void readValues();
    void resizeColumns();
    void setDensity();
    void setReynolds();
    void setupLayout();
    void setWingLoad();
    void setWPolarName();


private slots:
    void onOK();
    void onArea();
    void onAeroData();
    void onUnit();
    void onMethod();
    void onPolarName();
    void onAutoName();
    void onTiltedGeom();
    void onViscous();
    void onIgnoreBodyPanels();
    void onGroundEffect();
    void onPolarType();
    void onEditingFinished();
    void onPlaneInertia();
    void onTabChanged(int index);
    void onButton(QAbstractButton *pButton);

private:
    static WPolar s_WPolar;

    Plane *m_pPlane;

    bool m_bAutoName;
    int m_UnitType;//1= International, 2= English
    double m_WingLoad;

    DoubleEdit *m_pctrlXCmRef, *m_pctrlZCmRef;
    DoubleEdit *m_pctrlDensity;
    DoubleEdit *m_pctrlViscosity;
    DoubleEdit *m_pctrlAlpha;
    DoubleEdit *m_pctrlBeta;
    DoubleEdit *m_pctrlWeight;
    DoubleEdit *m_pctrlQInf;
    DoubleEdit *m_pctrlHeight;
    QLineEdit *m_pctrlWPolarName;

    QCheckBox *m_pctrlPlaneInertia;
    QCheckBox *m_pctrlGroundEffect;
    QCheckBox *m_pctrlViscous;
    QCheckBox *m_pctrlIgnoreBodyPanels;
    QCheckBox *m_pctrlTiltGeom;
    QCheckBox *m_pctrlAutoName;

    QRadioButton *m_pctrlType1,*m_pctrlType2,*m_pctrlType4,*m_pctrlType5;
    QRadioButton *m_pctrlLLTMethod, *m_pctrlVLM1Method, *m_pctrlVLM2Method, *m_pctrlPanelMethod;
    QRadioButton *m_pctrlUnit1, *m_pctrlUnit2;
    QRadioButton *m_pctrlArea1, *m_pctrlArea2, *m_pctrlArea3;

    DoubleEdit *m_pctrlRefChord, *m_pctrlRefArea, *m_pctrlRefSpan;

    QLabel *m_pctrlSRe;
    QLabel *m_pctrlRRe;
    QLabel *m_pctrlQInfCl;
    QLabel *m_pctrlWingLoad;

    QLabel *m_pctrlRho, *m_pctrlNu;
    QLabel *m_pctrlDensityUnit, *m_pctrlViscosityUnit;

    QDialogButtonBox *m_pButtonBox;


    int  *m_anglePrecision, *m_massPrecision;

    QTableView *m_pExtraDragControlTable;
    QStandardItemModel *m_pExtraDragControlModel;
    CtrlTableDelegate *m_pCtrlDelegate;

};

#endif // WPOLARDLG_H
