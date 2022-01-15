/****************************************************************************

    InertiaDlg Class
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
#include <QLabel>
#include <QStackedWidget>
#include <QRadioButton>
#include <QPushButton>
#include <QStandardItemModel>
#include <QDialogButtonBox>


#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/pointmass.h>

class CPTableView;
class FloatEditDelegate;
class DoubleEdit;
class Plane;
class Wing;
class Body;

class InertiaDlg : public QDialog
{
    Q_OBJECT
    friend class WingDlg;
    friend class BodyDlg;
    friend class EditBodyDlg;
    friend class PlaneDlg;
    friend class Miarex;
    friend class MainFrame;

    public:
        InertiaDlg(QWidget *pParent);
        ~InertiaDlg();

        void initDialog();

    private slots:
        void onBodyInertia();
        void onWingInertia();
        void onWing2Inertia();
        void onStabInertia();
        void onFinInertia();
        void onCellChanged(QWidget *);
        void onExportToAVL();
        void onInsertMassRow();
        void onDeleteMassRow();
        void onVolumeMass();
        void onButton(QAbstractButton *pButton);

    private:
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void onOK();
        void fillMassModel();
        void clearPointMasses() {m_PointMass.clear();}
        void computeInertia();
        void computeBodyAxisInertia();
        void setupLayout();
        void readData();

    private:
        //layout widget variables
        QStackedWidget *m_pswTop;
        QPushButton *m_ppbWingInertia, *m_ppbWing2Inertia, *m_ppbStabInertia, *m_ppbFinInertia, *m_ppbBodyInertia;

        QLabel *m_plabVolumeMassLabel, *m_plabTotalMassLabel;
        CPTableView *m_ptvMass;
        QStandardItemModel *m_pMassModel;
        FloatEditDelegate *m_pFloatDelegate;
        DoubleEdit *m_pdeCoGIxx, *m_pdeCoGIyy, *m_pdeCoGIzz, *m_pdeCoGIxz;
        DoubleEdit *m_pdeXCoG,*m_pdeYCoG,*m_pdeZCoG;
        DoubleEdit *m_pdeVolumeMass;

        DoubleEdit *m_pdeTotalIxx, *m_pdeTotalIyy, *m_pdeTotalIzz, *m_pdeTotalIxz;
        DoubleEdit *m_pdeXTotalCoG,*m_pdeYTotalCoG,*m_pdeZTotalCoG;
        DoubleEdit *m_pdeTotalMass;

        QPushButton *m_ppbExportToAVL;
        QDialogButtonBox *m_pButtonBox;

        QMenu *m_pContextMenu;
        QAction *m_pInsertMassRow, *m_pDeleteMassRow;

        //member variables
        Body *m_pBody;
        Wing *m_pWing;
        Plane *m_pPlane;

        double m_VolumeMass;
        Vector3d m_VolumeCoG;

        double m_CoGIxx, m_CoGIyy, m_CoGIzz, m_CoGIxz;

        QVector<PointMass> m_PointMass;

        bool m_bChanged;


    public:
        static QByteArray s_Geometry;
};

