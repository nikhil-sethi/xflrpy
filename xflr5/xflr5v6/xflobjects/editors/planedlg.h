/****************************************************************************

    PlaneDlg Class
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
#include <QAction>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>

class Plane;
class DoubleEdit;

class gl3dPlaneView;

/** The class to define and edit planes. SUes */

class PlaneDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class gl3dPlaneView;

    public:
        PlaneDlg(QWidget *parent);
        void initDialog();

    private slots:
        void onOK();
        void onFin();
        void onStab();
        void onBodyCheck();
        void onDefineWing();
        void onDefineStab();
        void onDefineFin();
        void onDefineBody();
        void onDefineBodyObject();
        void onChanged();
        void onDescriptionChanged();
        void onImportWing();
        void onDefaultBody();
        void onPlaneName();
        void onSymFin();
        void onDoubleFin();
        void onBiplane();
        void onDefineWing2();
        void onImportWing2();
        void onImportPlaneBody();
        void onImportXMLBody();
        void onInertia();
        void onButton(QAbstractButton *pButton);

    private:
        void connectSignals();
        void setupLayout();
        void setResults();
        void readParams();
        void setParams();
        void keyPressEvent(QKeyEvent *event) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void reject() override;

    private:

        Plane *m_pPlane;   /**< A pointer to the plane which is currently edited in this dialog window */
        bool m_bChanged;   /**< Set to true whenever the data in the window has been changed */
        bool m_bDescriptionChanged;
        bool m_bAcceptName;


    private:
        QLabel *m_plabWingSpan;
        QLabel *m_plabWingSurface;
        QLabel *m_plabStabVolume;
        QLabel *m_plabFinSurface;
        QLabel *m_plabStabLeverArm;
        QLabel *m_plabStabSurface;
        QLabel *m_plabPlaneVolume;
        QLabel *m_plabVLMTotalPanels;
        DoubleEdit *m_pdeXBody;
        DoubleEdit *m_pdeZBody;
        DoubleEdit *m_pdeXLEFin;
        DoubleEdit *m_pdeYLEFin;
        DoubleEdit *m_pdeZLEFin;
        DoubleEdit *m_pdeZLEStab;
        DoubleEdit *m_pdeXLEStab;
        DoubleEdit *m_pdeXLEWing;
        DoubleEdit *m_pdeZLEWing;
        DoubleEdit *m_pdeXLEWing2;
        DoubleEdit *m_pdeZLEWing2;
        DoubleEdit *m_pdeStabTilt;
        DoubleEdit *m_pdeFinTilt;
        DoubleEdit *m_pdeWingTilt;
        DoubleEdit *m_pdeWingTilt2;
        QLineEdit *m_plePlaneName;
        QTextEdit *m_ptePlaneDescription;
        QCheckBox *m_pchBiplane;
        QCheckBox *m_pchBody;
        QCheckBox *m_pchStabCheck;
        QCheckBox *m_pchFinCheck;
        QCheckBox *m_pchDoubleFin;
        QCheckBox *m_pchSymFin;
        QPushButton *m_ppbDefineWing;
        QPushButton *m_ppbImportWing;
        QPushButton *m_ppbDefineWing2;
        QPushButton *m_ppbImportWing2;
        QPushButton *m_ppbDefineFin;
        QPushButton *m_ppbVTail;
        QPushButton *m_ppbDefineStab;
        QPushButton *m_ppbBodyActions;
        QPushButton *m_ppbPlaneInertia;

        QCheckBox *m_pchAxes, *m_pchLight, *m_pchSurfaces, *m_pchOutline, *m_pchPanels;
        QCheckBox *m_pchFoilNames, *m_pchShowMasses;

        QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
        QToolButton *m_ptbX, *m_ptbY, *m_ptbZ, *m_ptbIso, *m_ptbFlip;
        QPushButton *m_ppbReset;
        QSlider *m_pslClipPlanePos;

        gl3dPlaneView *m_pglPlaneView;

        QDialogButtonBox *m_pButtonBox;


        QAction *m_pImportXMLBody, *m_pImportPlaneBody;


        static bool s_bOutline;
        static bool s_bSurfaces;
        static bool s_bVLMPanels;
        static bool s_bAxes;
        static bool s_bShowMasses;
        static bool s_bFoilNames;

        static QByteArray s_Geometry;

};

