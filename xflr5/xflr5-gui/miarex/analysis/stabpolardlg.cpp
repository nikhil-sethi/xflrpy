/****************************************************************************

    StabPolarDlg Class
    Copyright (C) 2010-2019 Andre Deperrois

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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <math.h>
#include <QDebug>

#include "./ctrltabledelegate.h"
#include "aerodatadlg.h"
#include "stabpolardlg.h"
#include <globals/globals.h>
#include <miarex/miarex.h>
#include <misc/options/units.h>
#include <misc/options/settings.h>
#include <misc/text/doubleedit.h>
#include <objects/objects3d/plane.h>
#include <objects/objects3d/wpolar.h>

WPolar StabPolarDlg::s_StabWPolar;

StabPolarDlg::StabPolarDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Stability Polar Definition"));

    s_StabWPolar.setPoints(2);
    s_StabWPolar.setCurveWidth(2);
    m_bAutoName = true;
    m_UnitType   = 1;

    m_pWingList[0] = nullptr;
    m_pWingList[1] = nullptr;
    m_pWingList[2] = nullptr;
    m_pWingList[3] = nullptr;
    m_pInertiaControlTable = nullptr;
    m_pInertiaControlModel = nullptr;

    m_pAngleControlTable = nullptr;
    m_pAngleControlModel = nullptr;


    s_StabWPolar.setPolarType(XFLR5::STABILITYPOLAR);
    s_StabWPolar.bVLM1() = false;

    setupLayout();
    connectSignals();
}



StabPolarDlg::~StabPolarDlg()
{
    delete [] m_anglePrecision;
    delete [] m_massPrecision;
    delete m_pMassCtrlDelegate;
    delete m_pAngleCtrlDelegate;
    delete m_pDragCtrlDelegate;
}



void StabPolarDlg::connectSignals()
{
    connect(m_pctrlUnit1,   SIGNAL(clicked()), this, SLOT(onUnit()));
    connect(m_pctrlUnit2,   SIGNAL(clicked()), this, SLOT(onUnit()));
    connect(m_pctrlViscous, SIGNAL(clicked()), this, SLOT(onViscous()));
    connect(m_pctrlIgnoreBodyPanels, SIGNAL(clicked()), this, SLOT(onIgnoreBodyPanels()));
    connect(m_pctrlArea1, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_pctrlArea2, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_pctrlArea3, SIGNAL(clicked()),this, SLOT(onArea()));

    connect(m_pctrlWingMethod2, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_pctrlWingMethod3, SIGNAL(toggled(bool)), this, SLOT(onMethod()));

    connect(m_pctrlDensity,   SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlViscosity, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlBeta, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlPhi,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    connect(m_pctrlRefArea,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlRefSpan,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlRefChord, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    connect(m_pctrlWPolarName, SIGNAL(editingFinished()), this, SLOT(onWPolarName()));
    connect(m_pctrlAutoName, SIGNAL(toggled(bool)), this, SLOT(onAutoName()));

    connect(m_pctrlAutoPlaneInertia, SIGNAL(clicked(bool)), this, SLOT(onAutoInertia(bool)));
    connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

    connect(m_pMassCtrlDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onInertiaCellChanged(QWidget *)));
    connect(m_pAngleCtrlDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onAngleCellChanged(QWidget *)));
    connect(m_pDragCtrlDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onDragCellChanged(QWidget *)));
}


void StabPolarDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)            onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}



void StabPolarDlg::fillInertiaPage()
{
    QString strLen, strMass, strInertia;
    strLen     = Units::lengthUnitLabel();
    strMass    = Units::weightUnitLabel();
    strInertia = Units::inertiaUnitLabel();

    if(s_StabWPolar.m_bAutoInertia)
    {
        s_StabWPolar.setMass(m_pPlane->totalMass());
        s_StabWPolar.setCoG(m_pPlane->CoG());
        s_StabWPolar.setCoGIxx(m_pPlane->m_CoGIxx);
        s_StabWPolar.setCoGIyy(m_pPlane->m_CoGIyy);
        s_StabWPolar.setCoGIzz(m_pPlane->m_CoGIzz);
        s_StabWPolar.setCoGIxz(m_pPlane->m_CoGIxz);
    }

    m_pInertiaControlModel->setRowCount(7);

    QModelIndex ind;

    ind = m_pInertiaControlModel->index(0, 0, QModelIndex()); // mass
    ind = m_pInertiaControlModel->index(1, 0, QModelIndex()); // x_CoG
    ind = m_pInertiaControlModel->index(2, 0, QModelIndex()); // z_CoG
    ind = m_pInertiaControlModel->index(3, 0, QModelIndex()); // Ixx
    ind = m_pInertiaControlModel->index(4, 0, QModelIndex()); // Iyy
    ind = m_pInertiaControlModel->index(5, 0, QModelIndex()); // Izz
    ind = m_pInertiaControlModel->index(6, 0, QModelIndex()); // Ixz

    ind = m_pInertiaControlModel->index(0, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("Mass"));
    ind = m_pInertiaControlModel->index(0, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.mass()*Units::kgtoUnit());
    ind = m_pInertiaControlModel->index(0, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[0]*Units::kgtoUnit());
    ind = m_pInertiaControlModel->index(0, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strMass);

    ind = m_pInertiaControlModel->index(1, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("CoG_x"));
    ind = m_pInertiaControlModel->index(1, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.CoG().x*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(1, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[1]*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(1, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strLen);

    ind = m_pInertiaControlModel->index(2, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("CoG_z"));
    ind = m_pInertiaControlModel->index(2, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.CoG().z*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(2, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[2]*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(2, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strLen);

    ind = m_pInertiaControlModel->index(3, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("Ixx"));
    ind = m_pInertiaControlModel->index(3, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_CoGIxx*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(3, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[3]*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(3, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strInertia);

    ind = m_pInertiaControlModel->index(4, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("Iyy"));
    ind = m_pInertiaControlModel->index(4, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_CoGIyy*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(4, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[4]*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(4, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strInertia);

    ind = m_pInertiaControlModel->index(5, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("Izz"));
    ind = m_pInertiaControlModel->index(5, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_CoGIzz*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(5, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[5]*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(5, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strInertia);

    ind = m_pInertiaControlModel->index(6, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, tr("Ixz"));
    ind = m_pInertiaControlModel->index(6, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_CoGIxz*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(6, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_StabWPolar.m_inertiaGain[6]*Units::kgm2toUnit());
    ind = m_pInertiaControlModel->index(6, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strInertia);
}


void StabPolarDlg::resizeColumns()
{
    double w = double(m_pInertiaControlTable->width())*.93;
    int wCols  = int(w/4);
    m_pInertiaControlTable->setColumnWidth(0, wCols);
    m_pInertiaControlTable->setColumnWidth(1, wCols);
    m_pInertiaControlTable->setColumnWidth(2, wCols);
    m_pInertiaControlTable->setColumnWidth(3, wCols);

    double wc = double(m_pAngleControlTable->width())*.97;
    wCols  = int(wc/2);
    m_pAngleControlTable->setColumnWidth(0, wCols);
    m_pAngleControlTable->setColumnWidth(1, wCols);

    double wxd = double(m_pExtraDragControlTable->width())*.97;
    wCols  = int(wxd/3);
    m_pExtraDragControlTable->setColumnWidth(0, wCols);
    m_pExtraDragControlTable->setColumnWidth(1, wCols);
    m_pExtraDragControlTable->setColumnWidth(2, wCols);
}



void StabPolarDlg::fillControlList()
{
    m_pAngleControlModel->setRowCount(s_StabWPolar.m_nControls);//temporary
    QString str, strong;
    QModelIndex ind;
    int i;
    Units::getLengthUnitLabel(str);

    s_StabWPolar.m_nControls = 0;
    if(!m_pPlane->isWing())
    {
        ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 0, QModelIndex());
        m_pAngleControlModel->setData(ind, tr("Wing Tilt (")+QString::fromUtf8("°")+")");

        ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 1, QModelIndex());
        m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[0]);

        ++s_StabWPolar.m_nControls;

        if(m_pWingList[2])
        {
            ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 0, QModelIndex());
            m_pAngleControlModel->setData(ind, tr("Elevator Tilt ")+QString::fromUtf8("(°)"));


            ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 1, QModelIndex());
            m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[1]);

            ++s_StabWPolar.m_nControls;
        }
    }

    for(i=0; i<m_pWingList[0]->m_nFlaps; i++)
    {
        ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 0, QModelIndex());
        strong = QString(tr("Wing Flap %1 ")+QString::fromUtf8("(°)")).arg(i+1);
        m_pAngleControlModel->setData(ind, strong);

        ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 1, QModelIndex());
        m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[i+s_StabWPolar.m_nControls]);
    }
    s_StabWPolar.m_nControls += m_pWingList[0]->m_nFlaps;


    if(m_pWingList[2])
    {
        for(i=0; i<m_pWingList[2]->m_nFlaps; i++)
        {
            ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 0, QModelIndex());
            strong = QString(tr("Elevator Flap %1 ")+QString::fromUtf8("(°)")).arg(i+1);
            m_pAngleControlModel->setData(ind, strong);

            ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 1, QModelIndex());
            m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[i+s_StabWPolar.m_nControls]);
        }
        s_StabWPolar.m_nControls += m_pWingList[2]->m_nFlaps;
    }
    if(m_pWingList[3])
    {
        for(i=0; i<m_pWingList[3]->m_nFlaps; i++)
        {
            ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 0, QModelIndex());
            strong = QString(tr("Fin Flap %1 ")+QString::fromUtf8("(°)")).arg(i+1);
            m_pAngleControlModel->setData(ind, strong);

            ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 1, QModelIndex());
            m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[i+s_StabWPolar.m_nControls]);
        }
        s_StabWPolar.m_nControls += m_pWingList[3]->m_nFlaps;
    }

    m_pAngleControlTable->resizeColumnsToContents();
}


void StabPolarDlg::setViscous()
{
    bool bViscous=true;
    int nCtrl = 0;

    if(!m_pPlane->isWing()) nCtrl++;          // add one for the wing tilt
    if(m_pPlane && m_pWingList[2]) nCtrl++;   // add one for the elevator tilt

    for(int i=nCtrl; i<s_StabWPolar.m_nControls; i++)
    {
        if(qAbs(s_StabWPolar.m_ControlGain[i])>PRECISION)
        {
            bViscous = false;
            break;
        }
    }

    if(!bViscous)
    {
        m_pctrlViscous->setChecked(false);
        m_pctrlViscous->setEnabled(false);
        s_StabWPolar.bViscous() = false;
    }
    else
    {
        if(s_StabWPolar.bViscous()) m_pctrlViscous->setChecked(true);
        m_pctrlViscous->setEnabled(true);
    }
}


void StabPolarDlg::initDialog(Plane *pPlane, WPolar *pWPolar)
{
    if(!pPlane) return;

    m_pPlane = pPlane;
    m_pWingList[0] = pPlane->wing();
    m_pWingList[1] = pPlane->wing2();
    m_pWingList[2] = pPlane->stab();
    m_pWingList[3] = pPlane->fin();

    if(m_UnitType==1) m_pctrlUnit1->setChecked(true);
    else              m_pctrlUnit2->setChecked(true);
    onUnit();

    if(pWPolar && pWPolar->isStabilityPolar())
    {
        m_bAutoName = false;
        m_pctrlWPolarName->setText(pWPolar->polarName());
        s_StabWPolar.duplicateSpec(pWPolar);
    }

    m_pctrlArea1->setChecked(s_StabWPolar.referenceDim()==XFLR5::PLANFORMREFDIM);
    m_pctrlArea2->setChecked(s_StabWPolar.referenceDim()==XFLR5::PROJECTEDREFDIM);
    m_pctrlArea3->setChecked(s_StabWPolar.referenceDim()==XFLR5::MANUALREFDIM);

    if(m_pctrlArea1->isChecked())
    {
        m_pctrlRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_pctrlArea2->isChecked())
    {
        m_pctrlRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
    }
    else if(m_pctrlArea3->isChecked())
    {
        m_pctrlRefArea->setValue(s_StabWPolar.referenceArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(s_StabWPolar.referenceSpanLength()*Units::mtoUnit());
    }
    m_pctrlRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());

    m_pctrlAutoName->setChecked(m_bAutoName);

    s_StabWPolar.setPlaneName(m_pPlane->planeName());
    m_pctrlWPolarName->setText(s_StabWPolar.polarName());

    if(m_pPlane->isWing()) m_pctrlAnalysisControls->setCurrentIndex(0);
    else
    {
        s_StabWPolar.setAnalysisMethod(XFLR5::PANEL4METHOD);
        m_pctrlAnalysisControls->setCurrentIndex(1);
    }

    s_StabWPolar.m_nControls = 0;
    s_StabWPolar.m_nControls += m_pWingList[0]->m_nFlaps;

    if(!m_pPlane->isWing())
    {
        s_StabWPolar.m_nControls++; // Wing Tilt
        if(m_pWingList[2])
        {
            s_StabWPolar.m_nControls++;//stab tilt
            s_StabWPolar.m_nControls += m_pWingList[2]->m_nFlaps;
        }
        if(m_pWingList[3]) s_StabWPolar.m_nControls+=m_pWingList[3]->m_nFlaps;

    }

    m_pctrlBeta->setValue(s_StabWPolar.m_BetaSpec);
    m_pctrlPhi->setValue(s_StabWPolar.m_BankAngle);

    if(s_StabWPolar.analysisMethod()==XFLR5::LLTMETHOD)
    {
        s_StabWPolar.setAnalysisMethod(XFLR5::PANEL4METHOD);
        s_StabWPolar.bThinSurfaces() = true;
    }

    if(!m_pPlane->isWing()) m_pctrlPanelMethod->setChecked(true);
    else
    {
        m_pctrlPanelMethod->setChecked(false);

        m_pctrlWingMethod2->setChecked(s_StabWPolar.analysisMethod()==XFLR5::VLMMETHOD);
        m_pctrlWingMethod3->setChecked(s_StabWPolar.analysisMethod()==XFLR5::PANEL4METHOD);
    }

    m_pctrlViscous->setChecked(s_StabWPolar.bViscous());
    m_pctrlIgnoreBodyPanels->setEnabled(m_pPlane && m_pPlane->body());
    m_pctrlIgnoreBodyPanels->setChecked(m_pPlane && m_pPlane->body() && s_StabWPolar.bIgnoreBodyPanels());

    if(!m_pPlane || !m_pPlane->body()) s_StabWPolar.bIgnoreBodyPanels()=false;

    fillControlList();

    m_pctrlAutoPlaneInertia->setChecked(s_StabWPolar.bAutoInertia());
    fillInertiaPage();

    fillExtraDragList();

    setWPolarName();

    m_pAngleControlTable->setFocus();

    enableControls();
}


void StabPolarDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                readCtrlData();
                readInertiaData();
                readExtraDragData();
                setWPolarName();
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                onOK();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            event->ignore();
    }
}



void StabPolarDlg::onArea()
{
    if(m_pctrlArea1->isChecked())
    {
        s_StabWPolar.setReferenceDim(XFLR5::PLANFORMREFDIM);
        m_pctrlRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pctrlRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
        m_pctrlRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_pctrlArea2->isChecked())
    {
        s_StabWPolar.setReferenceDim(XFLR5::PROJECTEDREFDIM);
        m_pctrlRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
        m_pctrlRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
    }
    else if(m_pctrlArea3->isChecked())
    {
        s_StabWPolar.setReferenceDim(XFLR5::MANUALREFDIM);
        //        m_pctrlRefArea->SetValue(s_WPolar.referenceArea()Length*Units::m2toUnit());
        //        m_pctrlRefSpan->SetValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
        //        m_pctrlRefChord->SetValue(s_WPolar.m_referenceChordLength*Units::mtoUnit());
    }

    setWPolarName();

    enableControls();
}


void StabPolarDlg::onAutoName()
{
    m_bAutoName = m_pctrlAutoName->isChecked();
    if(m_bAutoName) setWPolarName();
    enableControls();
}


void StabPolarDlg::onAngleCellChanged(QWidget *)
{
    readCtrlData();
    setWPolarName();
}


void StabPolarDlg::onInertiaCellChanged(QWidget *)
{
    readInertiaData();
    setWPolarName();
}



void StabPolarDlg::onDragCellChanged(QWidget *)
{
    readExtraDragData();
    setWPolarName();
}

void StabPolarDlg::onEditingFinished()
{
    readData();
    setWPolarName();
}



void StabPolarDlg::onUnit()
{
    if(m_pctrlUnit1->isChecked())
    {
        m_UnitType   = 1;
        m_pctrlViscosity->setValue(s_StabWPolar.viscosity());
        m_pctrlDensityUnit->setText("kg/m3");
        m_pctrlViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");
    }
    else
    {
        m_UnitType   = 2;
        m_pctrlViscosity->setValue(s_StabWPolar.viscosity()* 10.7182881);
        m_pctrlDensityUnit->setText("slugs/ft3");
        m_pctrlViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");
    }
    setDensity();
}


void StabPolarDlg::onOK()
{
    readCtrlData();
    readInertiaData();
    readExtraDragData();
    readData();

    if(qAbs(s_StabWPolar.mass())<PRECISION)
    {
        QMessageBox::warning(this, tr("Warning"),tr("Mass must be non-zero for type 7 polars"));
        m_pInertiaControlTable->setFocus();
        return;
    }



    if(!m_pctrlWPolarName->text().length())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Must enter a name for the polar"));
        m_pctrlWPolarName->setFocus();
        return;
    }
    s_StabWPolar.setPolarName(m_pctrlWPolarName->text());

    accept();
}



void StabPolarDlg::onViscous()
{
    s_StabWPolar.bViscous() = m_pctrlViscous->isChecked();
    setWPolarName();
}



void StabPolarDlg::onIgnoreBodyPanels()
{
    s_StabWPolar.bIgnoreBodyPanels() = m_pctrlIgnoreBodyPanels->isChecked();
    setWPolarName();
}


void StabPolarDlg::onWPolarName()
{
    m_pctrlAutoName->setChecked(false);
    m_bAutoName = false;
}


void StabPolarDlg::readCtrlData()
{
    //    s_StabPolar.m_ControlGain.clear();
    for(int icg=0; icg<s_StabWPolar.m_nControls; icg++)
    {
        s_StabWPolar.m_ControlGain[icg] = m_pAngleControlModel->index(icg, 1, QModelIndex()).data().toDouble(); //is the gain, AVL-like
    }

    setViscous();
}



void StabPolarDlg::onAutoInertia(bool isChecked)
{
    s_StabWPolar.m_bAutoInertia = isChecked;
    if(s_StabWPolar.m_bAutoInertia)
    {
        fillInertiaPage();
    }
    else
    {
        /*        m_pctrlMass->setValue(s_StabPolar.m_Mass*Units::kgtoUnit());
        m_pctrlCoGx->setValue(s_StabPolar.m_CoG.x*Units::mtoUnit());
        m_pctrlCoGz->setValue(s_StabPolar.m_CoG.z*Units::mtoUnit());
        m_pctrlIxx->setValue(s_StabPolar.m_CoGIxx*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit());
        m_pctrlIyy->setValue(s_StabPolar.m_CoGIyy*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit());
        m_pctrlIzz->setValue(s_StabPolar.m_CoGIzz*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit());
        m_pctrlIxz->setValue(s_StabPolar.m_CoGIxz*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit());*/
    }

    enableControls();

    setWPolarName();
}


void StabPolarDlg::readInertiaData()
{
    if(s_StabWPolar.bAutoInertia())
    {
        s_StabWPolar.retrieveInertia(m_pPlane);
        for(int ii=0; ii<7; ii++) s_StabWPolar.m_inertiaGain[ii] = 0.0;
    }
    else
    {
        s_StabWPolar.setMass(m_pInertiaControlModel->index(0, 1, QModelIndex()).data().toDouble() / Units::kgtoUnit());
        s_StabWPolar.setCoGx(m_pInertiaControlModel->index(1, 1, QModelIndex()).data().toDouble() / Units::mtoUnit());
        s_StabWPolar.setCoGz(m_pInertiaControlModel->index(2, 1, QModelIndex()).data().toDouble() / Units::mtoUnit());
        s_StabWPolar.setCoGIxx(m_pInertiaControlModel->index(3, 1, QModelIndex()).data().toDouble() / Units::kgm2toUnit());
        s_StabWPolar.setCoGIyy(m_pInertiaControlModel->index(4, 1, QModelIndex()).data().toDouble() / Units::kgm2toUnit());
        s_StabWPolar.setCoGIzz(m_pInertiaControlModel->index(5, 1, QModelIndex()).data().toDouble() / Units::kgm2toUnit());
        s_StabWPolar.setCoGIxz(m_pInertiaControlModel->index(6, 1, QModelIndex()).data().toDouble() / Units::kgm2toUnit());

        s_StabWPolar.m_inertiaGain[0] = m_pInertiaControlModel->index(0, 2, QModelIndex()).data().toDouble() / Units::kgtoUnit();
        s_StabWPolar.m_inertiaGain[1] = m_pInertiaControlModel->index(1, 2, QModelIndex()).data().toDouble() / Units::mtoUnit();
        s_StabWPolar.m_inertiaGain[2] = m_pInertiaControlModel->index(2, 2, QModelIndex()).data().toDouble() / Units::mtoUnit();
        s_StabWPolar.m_inertiaGain[3] = m_pInertiaControlModel->index(3, 2, QModelIndex()).data().toDouble() / Units::kgm2toUnit();
        s_StabWPolar.m_inertiaGain[4] = m_pInertiaControlModel->index(4, 2, QModelIndex()).data().toDouble() / Units::kgm2toUnit();
        s_StabWPolar.m_inertiaGain[5] = m_pInertiaControlModel->index(5, 2, QModelIndex()).data().toDouble() / Units::kgm2toUnit();
        s_StabWPolar.m_inertiaGain[6] = m_pInertiaControlModel->index(6, 2, QModelIndex()).data().toDouble() / Units::kgm2toUnit();
    }
}


void StabPolarDlg::readData()
{
    if(m_pctrlUnit1->isChecked())
    {
        s_StabWPolar.setDensity(m_pctrlDensity->value());
        s_StabWPolar.setViscosity(m_pctrlViscosity->value());
    }
    else
    {
        s_StabWPolar.setDensity(m_pctrlDensity->value()   / 0.00194122);
        s_StabWPolar.setViscosity(m_pctrlViscosity->value() / 10.7182881);
    }

    s_StabWPolar.m_BetaSpec  = m_pctrlBeta->value();
    s_StabWPolar.m_BankAngle = m_pctrlPhi->value();
    setDensity();

    s_StabWPolar.bViscous() = m_pctrlViscous->isChecked();
    s_StabWPolar.bIgnoreBodyPanels() = m_pctrlIgnoreBodyPanels->isChecked();


    if(m_pctrlArea1->isChecked())
    {
        s_StabWPolar.setReferenceDim(XFLR5::PLANFORMREFDIM);
        s_StabWPolar.setReferenceArea(m_pPlane->planformArea());
        s_StabWPolar.setReferenceSpanLength(m_pPlane->planformSpan());
    }
    else if(m_pctrlArea2->isChecked())
    {
        s_StabWPolar.setReferenceDim(XFLR5::PROJECTEDREFDIM);
        s_StabWPolar.setReferenceArea(m_pPlane->projectedArea());
        s_StabWPolar.setReferenceSpanLength(m_pPlane->projectedSpan());
    }
    else if(m_pctrlArea3->isChecked())
    {
        s_StabWPolar.setReferenceDim(XFLR5::MANUALREFDIM);
        s_StabWPolar.setReferenceArea(m_pctrlRefArea->value() /Units::m2toUnit());
        s_StabWPolar.setReferenceSpanLength(m_pctrlRefSpan->value() /Units::mtoUnit());
    }
    s_StabWPolar.setReferenceChordLength(m_pctrlRefChord->value() /Units::mtoUnit());
}


void StabPolarDlg::setDensity()
{
    int exp, precision;
    if(m_pctrlUnit1->isChecked())
    {
        exp = int(log(s_StabWPolar.density()));
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pctrlDensity->setPrecision(precision);
        m_pctrlDensity->setValue(s_StabWPolar.density());
    }
    else
    {
        exp = int(log(s_StabWPolar.density()* 0.00194122));
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pctrlDensity->setPrecision(precision);
        m_pctrlDensity->setValue(s_StabWPolar.density()* 0.00194122);
    }
}


void StabPolarDlg::onTabChanged(int index)
{
    if(index==2 || index==3 || index==5)
    {
        resizeColumns();
    }
}


void StabPolarDlg::setupLayout()
{
    QString strLen, strMass, strInertia, strArea;
    Units::getAreaUnitLabel(strArea);
    Units::getLengthUnitLabel(strLen);
    Units::getWeightUnitLabel(strMass);
    strInertia = strMass+"."+strLen+QString::fromUtf8("²");

    QFont symbolFont("Symbol");
    QFont italicFnt;
    italicFnt.setItalic(true);

    QFontMetrics fm(italicFnt);


    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->setMinimumWidth(fm.averageCharWidth() * 103);

    QWidget  *pMethodPage       = new QWidget(this);
    QWidget  *pCoefficientPage  = new QWidget(this);
    QWidget  *pOptionPage       = new QWidget(this);
    QWidget  *pMassControlPage  = new QWidget(this);
    QWidget  *pAngleControlPage = new QWidget(this);
    QWidget  *pExtraDragPage = new QWidget(this);


    QVBoxLayout *pMethodPageLayout = new QVBoxLayout;
    {
        QVBoxLayout *pMethodLayout = new QVBoxLayout;
        {
            m_pctrlAnalysisControls = new QStackedWidget;
            {
                QGroupBox *pWingMethodBox = new QGroupBox(tr("Analysis methods"));
                {
                    QVBoxLayout *pWingMethodLayout = new QVBoxLayout;
                    {
                        m_pctrlWingMethod2 = new QRadioButton(tr("Ring vortex")+" (VLM2)");
                        m_pctrlWingMethod3 = new QRadioButton(tr("3D Panels"));
                        pWingMethodLayout->addWidget(m_pctrlWingMethod2);
                        pWingMethodLayout->addWidget(m_pctrlWingMethod3);
                        pWingMethodLayout->addStretch();
                    }
                    pWingMethodBox->setLayout(pWingMethodLayout);
                }

                QGroupBox *pPlaneMethodBox = new QGroupBox(tr("Plane analysis methods"));
                {
                    QVBoxLayout *pPlaneMethodLayout = new QVBoxLayout;
                    {
                        m_pctrlPanelMethod = new QRadioButton(tr("Mix 3D Panels/VLM2"));
                        m_pctrlIgnoreBodyPanels = new QCheckBox(tr("Ignore Body Panels"));
                        pPlaneMethodLayout->addStretch();
                        pPlaneMethodLayout->addWidget(m_pctrlPanelMethod);
                        pPlaneMethodLayout->addWidget(m_pctrlIgnoreBodyPanels);
                        pPlaneMethodLayout->addStretch();

                    }
                    pPlaneMethodBox->setLayout(pPlaneMethodLayout);
                }

                m_pctrlAnalysisControls->addWidget(pWingMethodBox);

                m_pctrlAnalysisControls->addWidget(pPlaneMethodBox);

            }
            QVBoxLayout *pViscousLayout = new QVBoxLayout;
            {
                m_pctrlViscous = new QCheckBox(tr("Viscous Analysis"));
                QLabel *lab11 = new QLabel(tr("Note : the analysis may be of the viscous type only if all the flap controls are inactive"));
                pViscousLayout->addWidget(m_pctrlViscous);
                pViscousLayout->addWidget(lab11);
            }
            pMethodLayout->addWidget(m_pctrlAnalysisControls);
            pMethodLayout->addLayout(pViscousLayout);
        }

        QGroupBox *pAttitudeGroupBox = new QGroupBox(tr("Flight attitude"));
        {
            QGridLayout *pAttitudeLayout = new QGridLayout;
            {
                QLabel *lab2 = new QLabel(tr("b ="));
                QLabel *lab3 = new QLabel(tr("f ="));
                lab2->setFont(QFont("Symbol"));
                lab3->setFont(QFont("Symbol"));
                lab2->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                lab3->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                QLabel *lab4 = new QLabel(QString::fromUtf8("°"));
                QLabel *lab5 = new QLabel(QString::fromUtf8("°"));
                m_pctrlBeta  = new DoubleEdit(0.818,2);
                m_pctrlPhi   = new DoubleEdit(0.414,2);

                pAttitudeLayout->addWidget(lab2,1,1);
                pAttitudeLayout->addWidget(m_pctrlBeta,1,2);
                pAttitudeLayout->addWidget(lab4 ,1,3);
                pAttitudeLayout->addWidget(lab3,2,1);
                pAttitudeLayout->addWidget(m_pctrlPhi,2,2);
                pAttitudeLayout->addWidget(lab5,2,3);
                pAttitudeLayout->setRowStretch(3,1);
                pAttitudeLayout->setColumnStretch(3,1);
            }

            pAttitudeGroupBox->setLayout(pAttitudeLayout);
        }

        pMethodPageLayout->addLayout(pMethodLayout);
        pMethodPageLayout->addStretch();
        pMethodPageLayout->addWidget(pAttitudeGroupBox);
        pMethodPage->setLayout(pMethodPageLayout);
    }

    QVBoxLayout *pCoefficientPageLayout = new QVBoxLayout;
    {
        QGroupBox *pAreaBox = new QGroupBox(tr("Ref. dimensions for aero coefficients"));
        {
            QVBoxLayout *pAreaOptions = new QVBoxLayout;
            {
                m_pctrlArea1 = new QRadioButton(tr("Wing Planform"));
                m_pctrlArea2 = new QRadioButton(tr("Wing Planform projected on xy plane"));
                m_pctrlArea3 = new QRadioButton(tr("Manual input"));

                QGridLayout *pRefAreaLayout = new QGridLayout;
                {
                    QLabel *labRefArea  = new QLabel(tr("Ref. area="));
                    QLabel *labRefSpan  = new QLabel(tr("Ref. span length="));
                    QLabel *labRefChord = new QLabel(tr("Ref. chord length="));
                    m_pctrlRefArea  = new DoubleEdit(0.0, 3);
                    m_pctrlRefChord = new DoubleEdit(0.0, 3);
                    m_pctrlRefSpan  = new DoubleEdit(0.0, 3);
                    QString strUnit;
                    Units::getAreaUnitLabel(strUnit);
                    QLabel *m_pctrlAreaUnit = new QLabel(strUnit);
                    Units::getLengthUnitLabel(strUnit);
                    QLabel *m_pctrlLengthUnit4 = new QLabel(strUnit);
                    QLabel *m_pctrlLengthUnit5 = new QLabel(strUnit);

                    labRefArea->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    labRefSpan->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    labRefChord->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pctrlAreaUnit->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
                    m_pctrlLengthUnit4->setAlignment(Qt::AlignLeft | Qt::AlignCenter);

                    pRefAreaLayout->addWidget(labRefArea,1,1);
                    pRefAreaLayout->addWidget(m_pctrlRefArea,1,2);
                    pRefAreaLayout->addWidget(m_pctrlAreaUnit,1,3);
                    pRefAreaLayout->addWidget(labRefSpan,2,1);
                    pRefAreaLayout->addWidget(m_pctrlRefSpan,2,2);
                    pRefAreaLayout->addWidget(m_pctrlLengthUnit4,2,3);
                    pRefAreaLayout->addWidget(labRefChord,3,1);
                    pRefAreaLayout->addWidget(m_pctrlRefChord,3,2);
                    pRefAreaLayout->addWidget(m_pctrlLengthUnit5,3,3);
                    pRefAreaLayout->setColumnStretch(1,1);
                }

                pAreaOptions->addWidget(m_pctrlArea1);
                pAreaOptions->addWidget(m_pctrlArea2);
                pAreaOptions->addWidget(m_pctrlArea3);
                pAreaOptions->addLayout(pRefAreaLayout);
                pAreaOptions->addStretch();
            }
            pAreaBox->setLayout(pAreaOptions);
        }

        pCoefficientPageLayout->addWidget(pAreaBox);

        pCoefficientPage->setLayout(pCoefficientPageLayout);
    }

    QVBoxLayout *pMassControlPageLayout  = new QVBoxLayout;
    {
        m_pctrlAutoPlaneInertia = new QCheckBox(tr("Use plane inertia"));

        m_pInertiaControlTable = new QTableView(this);
        m_pInertiaControlTable->setFont(Settings::s_TableFont);

        m_pInertiaControlTable->setWindowTitle(tr("Controls"));
        m_pInertiaControlTable->setMinimumWidth(400);
        m_pInertiaControlTable->setMinimumHeight(150);
        m_pInertiaControlTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pInertiaControlTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pInertiaControlTable->horizontalHeader()->setStretchLastSection(true);

        m_pInertiaControlModel = new CtrlTableModel(this);
        m_pInertiaControlModel->setRowCount(7);
        m_pInertiaControlModel->setColumnCount(4);
        m_pInertiaControlModel->setHeaderData(0, Qt::Horizontal, tr("Inertia parameter"));
        m_pInertiaControlModel->setHeaderData(1, Qt::Horizontal, tr("Mean value"));
        m_pInertiaControlModel->setHeaderData(2, Qt::Horizontal, tr("Gain")+QString::fromUtf8("(unit/ctrl)"));
        m_pInertiaControlModel->setHeaderData(3, Qt::Horizontal, tr("Unit"));

        m_pInertiaControlTable->setModel(m_pInertiaControlModel);

        m_pMassCtrlDelegate = new CtrlTableDelegate(this);
        m_pInertiaControlTable->setItemDelegate(m_pMassCtrlDelegate);
        m_pMassCtrlDelegate->m_pCtrlModel = m_pInertiaControlModel;

        m_massPrecision = new int[3];
        m_massPrecision[0]  = 2;
        m_massPrecision[1]  = 3;
        m_massPrecision[2]  = 3;

        m_pMassCtrlDelegate->m_Precision = m_massPrecision;

        pMassControlPageLayout->addWidget(m_pctrlAutoPlaneInertia);
        pMassControlPageLayout->addWidget(m_pInertiaControlTable);

        pMassControlPage->setLayout(pMassControlPageLayout);
    }

    QVBoxLayout *pAngleControlPageLayout  = new QVBoxLayout;
    {
        m_pAngleControlTable = new QTableView(this);
        m_pAngleControlTable->setFont(Settings::s_TableFont);

        m_pAngleControlTable->setWindowTitle(tr("Controls"));
        m_pAngleControlTable->setMinimumWidth(400);
        m_pAngleControlTable->setMinimumHeight(150);
        m_pAngleControlTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pAngleControlTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pAngleControlTable->horizontalHeader()->setStretchLastSection(true);

        m_pAngleControlModel = new CtrlTableModel(this);
        m_pAngleControlModel->setRowCount(10);//temporary
        m_pAngleControlModel->setColumnCount(2);
        m_pAngleControlModel->setHeaderData(0, Qt::Horizontal, tr("Control Name"));
        m_pAngleControlModel->setHeaderData(1, Qt::Horizontal, tr("Gain")+QString::fromUtf8("(°/ctrl)"));


        m_pAngleControlTable->setModel(m_pAngleControlModel);

        m_pAngleCtrlDelegate = new CtrlTableDelegate(this);
        m_pAngleControlTable->setItemDelegate(m_pAngleCtrlDelegate);
        m_pAngleCtrlDelegate->m_pCtrlModel = m_pAngleControlModel;

        m_anglePrecision = new int[2];
        m_anglePrecision[0]  = 1;
        m_anglePrecision[1]  = 2;

        m_pAngleCtrlDelegate->m_Precision = m_anglePrecision;


        QLabel* SignLabel = new QLabel(tr("Note: + sign means trailing edge down"));

        pAngleControlPageLayout->addWidget(m_pAngleControlTable);
        pAngleControlPageLayout->addWidget(SignLabel);
        pAngleControlPage->setLayout(pAngleControlPageLayout);
    }

    QVBoxLayout *pOptionPageLayout = new QVBoxLayout;
    {
        QGroupBox *pAeroDataGroupBox = new QGroupBox(tr("Air Data"));
        {
            QVBoxLayout *pAeroDataLayout = new QVBoxLayout;
            {
                QHBoxLayout *pAeroUnitLayout = new QHBoxLayout;
                {
                    QLabel *lab9 = new QLabel(tr("Unit"));
                    lab9->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pctrlUnit1 = new QRadioButton(tr("International"));
                    m_pctrlUnit2 = new QRadioButton(tr("Imperial"));
                    pAeroUnitLayout->addWidget(lab9);
                    pAeroUnitLayout->addWidget(m_pctrlUnit1);
                    pAeroUnitLayout->addWidget(m_pctrlUnit2);
                    pAeroUnitLayout->addStretch();
                }
                QGridLayout *pAeroDataValuesLayout = new QGridLayout;
                {
                    m_pctrlRho = new QLabel("r =");
                    m_pctrlDensity = new DoubleEdit(1.225,3);
                    m_pctrlDensityUnit = new QLabel("kg/m3");
                    m_pctrlNu = new QLabel("n =");
                    m_pctrlRho->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pctrlNu->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pctrlViscosity = new DoubleEdit(1.500e-5,3);
                    m_pctrlViscosityUnit = new QLabel("m2/s");
                    m_pctrlRho->setFont(symbolFont);
                    m_pctrlNu->setFont(symbolFont);
                    m_pctrlDensity->setPrecision(5);
                    m_pctrlViscosity->setPrecision(3);
                    m_pctrlDensity->setMin(0.0);
                    m_pctrlViscosity->setMin(0.0);
                    pAeroDataValuesLayout->addWidget(m_pctrlRho,1,1);
                    pAeroDataValuesLayout->addWidget(m_pctrlDensity,1,2);
                    pAeroDataValuesLayout->addWidget(m_pctrlDensityUnit,1,3);
                    pAeroDataValuesLayout->addWidget(m_pctrlNu,2,1);
                    pAeroDataValuesLayout->addWidget(m_pctrlViscosity,2,2);
                    pAeroDataValuesLayout->addWidget(m_pctrlViscosityUnit,2,3);
                    pAeroDataValuesLayout->setRowStretch(3,1);
                    pAeroDataValuesLayout->setColumnStretch(1,3);
                    pAeroDataValuesLayout->setColumnStretch(4,3);
                }
                pAeroDataLayout->addLayout(pAeroUnitLayout);
                pAeroDataLayout->addLayout(pAeroDataValuesLayout);
                QPushButton *pFromData = new QPushButton(tr("From Altitude and Temperature"));
                connect(pFromData, SIGNAL(clicked()), this, SLOT(onAeroData()));
                pAeroDataLayout->addWidget(pFromData);
            }
            pAeroDataGroupBox->setLayout(pAeroDataLayout);
        }
        pOptionPageLayout->addWidget(pAeroDataGroupBox);
        pOptionPage->setLayout(pOptionPageLayout);
    }

    QVBoxLayout *pExtraDragPageLayout  = new QVBoxLayout;
    {
        m_pExtraDragControlTable = new QTableView(this);
        m_pExtraDragControlTable->setFont(Settings::s_TableFont);

        m_pExtraDragControlTable->setWindowTitle(tr("Extra drag"));
        m_pExtraDragControlTable->setMinimumWidth(400);
        m_pExtraDragControlTable->setMinimumHeight(150);
        m_pExtraDragControlTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pExtraDragControlTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pExtraDragControlTable->horizontalHeader()->setStretchLastSection(true);

        m_pExtraDragControlModel = new CtrlTableModel(this);
        m_pExtraDragControlModel->setColumnCount(3);
        m_pExtraDragControlModel->setHeaderData(0, Qt::Horizontal, tr("Extra drag"));
        m_pExtraDragControlModel->setHeaderData(1, Qt::Horizontal, tr("Extra area")+" ("+Units::areaUnitLabel()+")");
        m_pExtraDragControlModel->setHeaderData(2, Qt::Horizontal, tr("Extra drag coef."));


        m_pExtraDragControlTable->setModel(m_pExtraDragControlModel);

        m_pDragCtrlDelegate = new CtrlTableDelegate(this);
        m_pExtraDragControlTable->setItemDelegate(m_pDragCtrlDelegate);
        m_pDragCtrlDelegate->m_pCtrlModel = m_pExtraDragControlModel;

        m_anglePrecision = new int[3];
        m_anglePrecision[0]  = 0;
        m_anglePrecision[1]  = 3;
        m_anglePrecision[2]  = 5;

        m_pDragCtrlDelegate->m_Precision = m_anglePrecision;

        QLabel* pExtraLabel = new QLabel(QString::fromUtf8("D = 1/2 rho V² ( S (CD_induced+CD_Visc) + S_Extra1.CD_Extra1 + ... + S_ExtraN.Cd_ExtraN)"));

        pExtraDragPageLayout->addWidget(m_pExtraDragControlTable);
        pExtraDragPageLayout->addWidget(pExtraLabel);
        pExtraDragPage->setLayout(pExtraDragPageLayout);
    }


    m_pTabWidget->addTab(pMethodPage,       tr("Analysis"));
    m_pTabWidget->addTab(pCoefficientPage,  tr("Ref. dimensions"));
    m_pTabWidget->addTab(pMassControlPage,  tr("Mass and inertia"));
    m_pTabWidget->addTab(pAngleControlPage, tr("Control parameters"));
    m_pTabWidget->addTab(pOptionPage,       tr("Aero data"));
    m_pTabWidget->addTab(pExtraDragPage,    tr("Extra drag"));

    m_pTabWidget->setCurrentIndex(0);

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        QHBoxLayout *pPolarNameLayout = new QHBoxLayout;
        {
            m_pctrlAutoName = new QCheckBox(tr("Auto Analysis Name"));
            m_pctrlWPolarName = new QLineEdit(tr("Polar Name"));
            pPolarNameLayout->addWidget(m_pctrlAutoName);
            pPolarNameLayout->addWidget(m_pctrlWPolarName);
        }

        pMainLayout->addLayout(pPolarNameLayout,1);
        pMainLayout->addSpacing(13);
        pMainLayout->addWidget(m_pTabWidget,13);
        pMainLayout->addStretch(1);
        pMainLayout->addSpacing(23);
        pMainLayout->addWidget(m_pButtonBox,1);
    }

    setLayout(pMainLayout);
}



void StabPolarDlg::readExtraDragData()
{
    for(int i=0; i<MAXEXTRADRAG; i++)
    {
        s_StabWPolar.m_ExtraDragArea[i]= m_pExtraDragControlModel->index(i, 1, QModelIndex()).data().toDouble()/Units::m2toUnit();
        s_StabWPolar.m_ExtraDragCoef[i]= m_pExtraDragControlModel->index(i, 2, QModelIndex()).data().toDouble();
    }
}



void StabPolarDlg::fillExtraDragList()
{
    m_pExtraDragControlModel->setRowCount(MAXEXTRADRAG);
    QString str;
    QModelIndex ind;
    Units::getLengthUnitLabel(str);

    for(int i=0; i<MAXEXTRADRAG; i++)
    {
        ind = m_pExtraDragControlModel->index(i, 0, QModelIndex());
        m_pExtraDragControlModel->setData(ind, QString("Extra %1").arg(i));
        ind = m_pExtraDragControlModel->index(i, 1, QModelIndex());
        m_pExtraDragControlModel->setData(ind, s_StabWPolar.m_ExtraDragArea[i]*Units::m2toUnit());
        ind = m_pExtraDragControlModel->index(i, 2, QModelIndex());
        m_pExtraDragControlModel->setData(ind, s_StabWPolar.m_ExtraDragCoef[i]);
    }

    m_pExtraDragControlTable->resizeColumnsToContents();
}


void StabPolarDlg::onAeroData()
{
    AeroDataDlg dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        s_StabWPolar.setDensity(dlg.AirDensity());
        s_StabWPolar.setViscosity(dlg.KinematicViscosity());

        if(m_pctrlUnit1->isChecked())
        {
            m_pctrlViscosity->setValue(s_StabWPolar.viscosity());
        }
        else
        {
            m_pctrlViscosity->setValue(s_StabWPolar.viscosity()* 10.7182881);
        }
        setDensity();
    }
}


void StabPolarDlg::enableControls()
{
    m_pctrlWPolarName->setEnabled(!m_pctrlAutoName->isChecked());

    m_pctrlRefArea->setEnabled(m_pctrlArea3->isChecked());
    m_pctrlRefChord->setEnabled(m_pctrlArea3->isChecked());
    m_pctrlRefSpan->setEnabled(m_pctrlArea3->isChecked());

    m_pInertiaControlTable->setEnabled(!s_StabWPolar.bAutoInertia());
}



void StabPolarDlg::setWPolarName()
{
    if(!m_bAutoName || !m_pPlane) return;

    setAutoWPolarName(&s_StabWPolar, m_pPlane);
    m_pctrlWPolarName->setText(s_StabWPolar.polarName());
}



void StabPolarDlg::onMethod()
{
    if (m_pctrlWingMethod2->isChecked())
    {
        s_StabWPolar.bThinSurfaces()  = true;
        s_StabWPolar.setAnalysisMethod(XFLR5::VLMMETHOD);
    }
    else if (m_pctrlWingMethod3->isChecked())
    {
        s_StabWPolar.bThinSurfaces()  = false;
        s_StabWPolar.setAnalysisMethod(XFLR5::PANEL4METHOD);
    }

    setWPolarName();
}



