/****************************************************************************

    WPolarDlg Class
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
/**
 *@file This file contains the definition of the class WPolarDlg which is used to define the data for a WPolar object.
 */

#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QRadioButton>
#include <QTableView>
#include <QDialogButtonBox>
#include <QStandardItemModel>

#include <xflobjects/objects3d/wpolar.h>

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
        void initDialog(Plane *pPlane, WPolar *pWPolar=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;


    private:

        void readExtraDragData();
        void readValues();
        void resizeColumns();
        void setDensity();
        void setReynolds();
        void setupLayout();
        void setWingLoad();
        void setWPolarName();
        void enableControls();
        void fillExtraDragList();


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

        DoubleEdit *m_pdeXCmRef, *m_pdeZCmRef;
        DoubleEdit *m_pdeDensity;
        DoubleEdit *m_pdeViscosity;
        DoubleEdit *m_pdeAlpha;
        DoubleEdit *m_pdeBeta;
        DoubleEdit *m_pdeWeight;
        DoubleEdit *m_pdeQInf;
        DoubleEdit *m_pdeHeight;
        QLineEdit *m_pleWPolarName;

        QCheckBox *m_pchPlaneInertia;
        QCheckBox *m_pchGroundEffect;
        QCheckBox *m_pchViscous;
        QCheckBox *m_pchIgnoreBodyPanels;
        QCheckBox *m_pchTiltGeom;
        QCheckBox *m_pchAutoName;

        QRadioButton *m_prbType1, *m_prbType2, *m_prbType4, *m_prbType5;
        QRadioButton *m_prbLLTMethod, *m_prbVLM1Method, *m_prbVLM2Method, *m_prbPanelMethod;
        QRadioButton *m_prbUnit1, *m_prbUnit2;
        QRadioButton *m_prbArea1, *m_prbArea2, *m_prbArea3;

        DoubleEdit *m_pdeRefChord, *m_pdeRefArea, *m_pdeRefSpan;

        QLabel *m_plabSRe;
        QLabel *m_plabRRe;
        QLabel *m_plabQInfCl;
        QLabel *m_plabWingLoad;

        QLabel *m_plabRho, *m_plabNu;
        QLabel *m_plabDensityUnit, *m_plabViscosityUnit;

        QDialogButtonBox *m_pButtonBox;

        QTableView *m_ptvExtraDragControl;
        QStandardItemModel *m_pExtraDragControlModel;
        CtrlTableDelegate *m_pControlDelegate;


        static QByteArray s_Geometry;
};

