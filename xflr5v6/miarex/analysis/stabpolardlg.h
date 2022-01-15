/****************************************************************************

    StabPolarDlg Class
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


#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>

#include <xflanalysis/analysis3d_params.h>
class Plane;
class Wing;
class WPolar;
class DoubleEdit;
class CtrlTableDelegate;


class CtrlTableModel: public QStandardItemModel
{
public:
    CtrlTableModel(QObject * parent=nullptr) : QStandardItemModel(parent)  { }

    Qt::ItemFlags flags(const QModelIndex & index) const
    {
        if (index.column() == 0)
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        else
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
};


class StabPolarDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class MainFrame;

    public:
        StabPolarDlg(QWidget *pParent=nullptr);
        ~StabPolarDlg();

        void initDialog(Plane *pPlane, WPolar *pWPolar=nullptr);

    private:
        void setupLayout();
        void connectSignals();
        void resizeColumns();
        void keyPressEvent(QKeyEvent *event) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private slots:
        void onOK();
        void onAutoInertia(bool isChecked);
        void onAutoName();
        void onWPolarName();
        void onArea();
        void onEditingFinished();
        void onViscous();
        void onIgnoreBodyPanels();
        void onUnit();
        void onAngleCellChanged(QWidget *);
        void onInertiaCellChanged(QWidget *);
        void onDragCellChanged(QWidget *);
        void onMethod();
        void onAeroData();
        void onTabChanged(int index);
        void onButton(QAbstractButton *pButton);

    private:
        void enableControls();
        void fillControlList();
        void fillExtraDragList();
        void fillInertiaPage();
        void readCtrlData();
        void readExtraDragData();
        void readInertiaData();
        void readData();
        void setDensity();
        void setWPolarName();
        void setViscous();


    private:

        QDialogButtonBox *m_pButtonBox;

        QTableView *m_ptvInertiaControl;
        CtrlTableModel *m_pInertiaControlModel;
        QTabWidget *m_ptwMain;

        QTableView *m_ptvAngleControl;
        CtrlTableModel *m_pAngleControlModel;

        QTableView *m_ptvExtraDragControl;
        CtrlTableModel *m_pExtraDragControlModel;

        CtrlTableDelegate *m_pMassCtrlDelegate, *m_pAngleCtrlDelegate, *m_pDragCtrlDelegate;

        DoubleEdit *m_pdeDensity;
        DoubleEdit *m_pdeViscosity;
        DoubleEdit *m_pdeBeta;
        DoubleEdit *m_pdePhi;
        QLineEdit *m_pleWPolarName;

        QCheckBox *m_pchViscous;
        QCheckBox *m_pchAutoName;
        QCheckBox *m_pchIgnoreBodyPanels;
        QCheckBox *m_pchAutoPlaneInertia;

        QRadioButton *m_prbUnit1, *m_prbUnit2;
        QRadioButton *m_prbArea1, *m_prbArea2, *m_prbArea3;

        DoubleEdit *m_pdeRefChord, *m_pdeRefArea, *m_pdeRefSpan;

        QStackedWidget *m_pswAnalysisControls;
        QRadioButton *m_prbWingMethod2, *m_prbWingMethod3;
        QRadioButton *m_prbPanelMethod;

        QLabel *m_plabRho, *m_plabNu;
        QLabel *m_plabDensityUnit, *m_plabViscosityUnit;

        static WPolar s_StabWPolar;

        Plane *m_pPlane;
        Wing *m_pWingList[MAXWINGS];         // pointers to the four wings of the currently selected plane

        bool m_bAutoName;
        int m_UnitType;//1= International, 2= Imperial

        static QByteArray s_Geometry;
};




