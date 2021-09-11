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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

#include <QDebug>

#include "aerodatadlg.h"
#include "stabpolardlg.h"
#include <miarex/miarex.h>

#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/customwts/ctrltabledelegate.h>
#include <xflwidgets/customwts/doubleedit.h>

WPolar StabPolarDlg::s_StabWPolar;
QByteArray StabPolarDlg::s_Geometry;

StabPolarDlg::StabPolarDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Stability Polar Definition"));

    s_StabWPolar.setPointStyle(Line::BIGCIRCLE);
    s_StabWPolar.setWidth(2);
    m_bAutoName = true;
    m_UnitType   = 1;

    m_pWingList[0] = nullptr;
    m_pWingList[1] = nullptr;
    m_pWingList[2] = nullptr;
    m_pWingList[3] = nullptr;
    m_ptvInertiaControl = nullptr;
    m_pInertiaControlModel = nullptr;

    m_ptvAngleControl = nullptr;
    m_pAngleControlModel = nullptr;


    s_StabWPolar.setPolarType(xfl::STABILITYPOLAR);
    s_StabWPolar.setVLM1(false);

    setupLayout();
    connectSignals();
}



StabPolarDlg::~StabPolarDlg()
{
    delete m_pMassCtrlDelegate;
    delete m_pAngleCtrlDelegate;
    delete m_pDragCtrlDelegate;
}



void StabPolarDlg::connectSignals()
{
    connect(m_prbUnit1,   SIGNAL(clicked()), this, SLOT(onUnit()));
    connect(m_prbUnit2,   SIGNAL(clicked()), this, SLOT(onUnit()));
    connect(m_pchViscous, SIGNAL(clicked()), this, SLOT(onViscous()));
    connect(m_pchIgnoreBodyPanels, SIGNAL(clicked()), this, SLOT(onIgnoreBodyPanels()));
    connect(m_prbArea1, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_prbArea2, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_prbArea3, SIGNAL(clicked()),this, SLOT(onArea()));

    connect(m_prbWingMethod2, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_prbWingMethod3, SIGNAL(toggled(bool)), this, SLOT(onMethod()));

    connect(m_pdeDensity,   SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeViscosity, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeBeta, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdePhi,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    connect(m_pdeRefArea,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeRefSpan,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeRefChord, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    connect(m_pleWPolarName, SIGNAL(editingFinished()), this, SLOT(onWPolarName()));
    connect(m_pchAutoName, SIGNAL(toggled(bool)), this, SLOT(onAutoName()));

    connect(m_pchAutoPlaneInertia, SIGNAL(clicked(bool)), this, SLOT(onAutoInertia(bool)));
    connect(m_ptwMain, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

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
    strMass    = Units::massUnitLabel();
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
    double w = double(m_ptvInertiaControl->width())*.93;
    int wCols  = int(w/4);
    m_ptvInertiaControl->setColumnWidth(0, wCols);
    m_ptvInertiaControl->setColumnWidth(1, wCols);
    m_ptvInertiaControl->setColumnWidth(2, wCols);
    m_ptvInertiaControl->setColumnWidth(3, wCols);

    double wc = double(m_ptvAngleControl->width())*.97;
    wCols  = int(wc/2);
    m_ptvAngleControl->setColumnWidth(0, wCols);
    m_ptvAngleControl->setColumnWidth(1, wCols);

    double wxd = double(m_ptvExtraDragControl->width())*.97;
    wCols  = int(wxd/3);
    m_ptvExtraDragControl->setColumnWidth(0, wCols);
    m_ptvExtraDragControl->setColumnWidth(1, wCols);
    m_ptvExtraDragControl->setColumnWidth(2, wCols);
}


void StabPolarDlg::fillControlList()
{
//    m_pAngleControlModel->setRowCount(17);//temporary
    QString strong;
    QModelIndex ind;


    s_StabWPolar.m_nControls = 0;
    if(!m_pPlane->isWing())
    {
        ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 0, QModelIndex());
        m_pAngleControlModel->setData(ind, tr("Wing Tilt (")+QChar(0260)+")");

        ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 1, QModelIndex());
        m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[0]);

        s_StabWPolar.m_nControls++;

        if(m_pWingList[2])
        {
            ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 0, QModelIndex());
            m_pAngleControlModel->setData(ind, tr("Elevator Tilt ")+QString::fromUtf8("(°)"));


            ind = m_pAngleControlModel->index(s_StabWPolar.m_nControls, 1, QModelIndex());
            m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[1]);

            s_StabWPolar.m_nControls++;
        }
    }

    for(int i=0; i<m_pWingList[0]->m_nFlaps; i++)
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
        for(int i=0; i<m_pWingList[2]->m_nFlaps; i++)
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
        for(int i=0; i<m_pWingList[3]->m_nFlaps; i++)
        {
            ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 0, QModelIndex());
            strong = QString(tr("Fin Flap %1 ")+QString::fromUtf8("(°)")).arg(i+1);
            m_pAngleControlModel->setData(ind, strong);

            ind = m_pAngleControlModel->index(i+s_StabWPolar.m_nControls, 1, QModelIndex());
            m_pAngleControlModel->setData(ind, s_StabWPolar.m_ControlGain[i+s_StabWPolar.m_nControls]);
        }
        s_StabWPolar.m_nControls += m_pWingList[3]->m_nFlaps;
    }

    m_ptvAngleControl->resizeColumnsToContents();
    m_pAngleControlModel->setRowCount(s_StabWPolar.m_nControls);
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
        m_pchViscous->setChecked(false);
        m_pchViscous->setEnabled(false);
        s_StabWPolar.setViscous(false);
    }
    else
    {
        if(s_StabWPolar.bViscous()) m_pchViscous->setChecked(true);
        m_pchViscous->setEnabled(true);
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

    if(m_UnitType==1) m_prbUnit1->setChecked(true);
    else              m_prbUnit2->setChecked(true);
    onUnit();

    if(pWPolar && pWPolar->isStabilityPolar())
    {
        m_bAutoName = false;
        m_pleWPolarName->setText(pWPolar->polarName());
        s_StabWPolar.duplicateSpec(pWPolar);
    }

    m_prbArea1->setChecked(s_StabWPolar.referenceDim()==xfl::PLANFORMREFDIM);
    m_prbArea2->setChecked(s_StabWPolar.referenceDim()==xfl::PROJECTEDREFDIM);
    m_prbArea3->setChecked(s_StabWPolar.referenceDim()==xfl::MANUALREFDIM);

    if(m_prbArea1->isChecked())
    {
        m_pdeRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_prbArea2->isChecked())
    {
        m_pdeRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
    }
    else if(m_prbArea3->isChecked())
    {
        m_pdeRefArea->setValue(s_StabWPolar.referenceArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(s_StabWPolar.referenceSpanLength()*Units::mtoUnit());
    }
    m_pdeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());

    m_pchAutoName->setChecked(m_bAutoName);

    s_StabWPolar.setPlaneName(m_pPlane->name());
    m_pleWPolarName->setText(s_StabWPolar.polarName());

    if(m_pPlane->isWing()) m_pswAnalysisControls->setCurrentIndex(0);
    else
    {
        s_StabWPolar.setAnalysisMethod(xfl::PANEL4METHOD);
        m_pswAnalysisControls->setCurrentIndex(1);
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

    m_pdeBeta->setValue(s_StabWPolar.m_BetaSpec);
    m_pdePhi->setValue(s_StabWPolar.m_BankAngle);

    if(s_StabWPolar.analysisMethod()==xfl::LLTMETHOD)
    {
        s_StabWPolar.setAnalysisMethod(xfl::PANEL4METHOD);
        s_StabWPolar.setThinSurfaces(true);
    }

    if(!m_pPlane->isWing()) m_prbPanelMethod->setChecked(true);
    else
    {
        m_prbPanelMethod->setChecked(false);

        m_prbWingMethod2->setChecked(s_StabWPolar.analysisMethod()==xfl::VLMMETHOD);
        m_prbWingMethod3->setChecked(s_StabWPolar.analysisMethod()==xfl::PANEL4METHOD);
    }

    m_pchViscous->setChecked(s_StabWPolar.bViscous());
    m_pchIgnoreBodyPanels->setEnabled(m_pPlane && m_pPlane->body());
    m_pchIgnoreBodyPanels->setChecked(m_pPlane && m_pPlane->body() && s_StabWPolar.bIgnoreBodyPanels());

    if(!m_pPlane || !m_pPlane->body()) s_StabWPolar.setIgnoreBodyPanels(false);

    fillControlList();

    m_pchAutoPlaneInertia->setChecked(s_StabWPolar.bAutoInertia());
    fillInertiaPage();

    fillExtraDragList();

    setWPolarName();

    m_ptvAngleControl->setFocus();

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
    if(m_prbArea1->isChecked())
    {
        s_StabWPolar.setReferenceDim(xfl::PLANFORMREFDIM);
        m_pdeRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pdeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
        m_pdeRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_prbArea2->isChecked())
    {
        s_StabWPolar.setReferenceDim(xfl::PROJECTEDREFDIM);
        m_pdeRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
        m_pdeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
    }
    else if(m_prbArea3->isChecked())
    {
        s_StabWPolar.setReferenceDim(xfl::MANUALREFDIM);
    }

    setWPolarName();
    enableControls();
}


void StabPolarDlg::onAutoName()
{
    m_bAutoName = m_pchAutoName->isChecked();
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
    if(m_prbUnit1->isChecked())
    {
        m_UnitType   = 1;
        m_pdeViscosity->setValue(s_StabWPolar.viscosity());
        m_plabDensityUnit->setText("kg/m3");
        m_plabViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");
    }
    else
    {
        m_UnitType   = 2;
        m_pdeViscosity->setValue(s_StabWPolar.viscosity()* 10.7182881);
        m_plabDensityUnit->setText("slugs/ft3");
        m_plabViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");
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
        m_ptvInertiaControl->setFocus();
        return;
    }



    if(!m_pleWPolarName->text().length())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Must enter a name for the polar"));
        m_pleWPolarName->setFocus();
        return;
    }
    s_StabWPolar.setPolarName(m_pleWPolarName->text());

    accept();
}


void StabPolarDlg::onViscous()
{
    s_StabWPolar.setViscous(m_pchViscous->isChecked());
    setWPolarName();
}


void StabPolarDlg::onIgnoreBodyPanels()
{
    s_StabWPolar.setIgnoreBodyPanels(m_pchIgnoreBodyPanels->isChecked());
    setWPolarName();
}


void StabPolarDlg::onWPolarName()
{
    m_pchAutoName->setChecked(false);
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
    if(m_prbUnit1->isChecked())
    {
        s_StabWPolar.setDensity(m_pdeDensity->value());
        s_StabWPolar.setViscosity(m_pdeViscosity->value());
    }
    else
    {
        s_StabWPolar.setDensity(m_pdeDensity->value()   / 0.00194122);
        s_StabWPolar.setViscosity(m_pdeViscosity->value() / 10.7182881);
    }

    s_StabWPolar.m_BetaSpec  = m_pdeBeta->value();
    s_StabWPolar.m_BankAngle = m_pdePhi->value();
    setDensity();

    s_StabWPolar.setViscous(m_pchViscous->isChecked());
    s_StabWPolar.setIgnoreBodyPanels(m_pchIgnoreBodyPanels->isChecked());


    if(m_prbArea1->isChecked())
    {
        s_StabWPolar.setReferenceDim(xfl::PLANFORMREFDIM);
        s_StabWPolar.setReferenceArea(m_pPlane->planformArea());
        s_StabWPolar.setReferenceSpanLength(m_pPlane->planformSpan());
    }
    else if(m_prbArea2->isChecked())
    {
        s_StabWPolar.setReferenceDim(xfl::PROJECTEDREFDIM);
        s_StabWPolar.setReferenceArea(m_pPlane->projectedArea());
        s_StabWPolar.setReferenceSpanLength(m_pPlane->projectedSpan());
    }
    else if(m_prbArea3->isChecked())
    {
        s_StabWPolar.setReferenceDim(xfl::MANUALREFDIM);
        s_StabWPolar.setReferenceArea(m_pdeRefArea->value() /Units::m2toUnit());
        s_StabWPolar.setReferenceSpanLength(m_pdeRefSpan->value() /Units::mtoUnit());
    }
    s_StabWPolar.setReferenceChordLength(m_pdeRefChord->value() /Units::mtoUnit());
}


void StabPolarDlg::setDensity()
{
    int exp, precision;
    if(m_prbUnit1->isChecked())
    {
        exp = int(log(s_StabWPolar.density()));
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pdeDensity->setDigits(precision);
        m_pdeDensity->setValue(s_StabWPolar.density());
    }
    else
    {
        exp = int(log(s_StabWPolar.density()* 0.00194122));
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pdeDensity->setDigits(precision);
        m_pdeDensity->setValue(s_StabWPolar.density()* 0.00194122);
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
    Units::getMassUnitLabel(strMass);
    strInertia = strMass+"."+strLen+QString::fromUtf8("²");

    QFont symbolFont("Symbol");
    QFont italicFnt;
    italicFnt.setItalic(true);

    QFontMetrics fm(italicFnt);


    m_ptwMain = new QTabWidget(this);
    m_ptwMain->setMinimumWidth(fm.averageCharWidth() * 103);

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
            m_pswAnalysisControls = new QStackedWidget;
            {
                QGroupBox *pWingMethodBox = new QGroupBox(tr("Analysis methods"));
                {
                    QVBoxLayout *pWingMethodLayout = new QVBoxLayout;
                    {
                        m_prbWingMethod2 = new QRadioButton(tr("Ring vortex")+" (VLM2)");
                        m_prbWingMethod3 = new QRadioButton(tr("3D Panels"));
                        pWingMethodLayout->addWidget(m_prbWingMethod2);
                        pWingMethodLayout->addWidget(m_prbWingMethod3);
                        pWingMethodLayout->addStretch();
                    }
                    pWingMethodBox->setLayout(pWingMethodLayout);
                }

                QGroupBox *pPlaneMethodBox = new QGroupBox(tr("Plane analysis methods"));
                {
                    QVBoxLayout *pPlaneMethodLayout = new QVBoxLayout;
                    {
                        m_prbPanelMethod = new QRadioButton(tr("Mix 3D Panels/VLM2"));
                        m_pchIgnoreBodyPanels = new QCheckBox(tr("Ignore Body Panels"));
                        pPlaneMethodLayout->addStretch();
                        pPlaneMethodLayout->addWidget(m_prbPanelMethod);
                        pPlaneMethodLayout->addWidget(m_pchIgnoreBodyPanels);
                        pPlaneMethodLayout->addStretch();

                    }
                    pPlaneMethodBox->setLayout(pPlaneMethodLayout);
                }

                m_pswAnalysisControls->addWidget(pWingMethodBox);

                m_pswAnalysisControls->addWidget(pPlaneMethodBox);

            }
            QVBoxLayout *pViscousLayout = new QVBoxLayout;
            {
                m_pchViscous = new QCheckBox(tr("Viscous Analysis"));
                QLabel *lab11 = new QLabel(tr("Note : the analysis may be of the viscous type only if all the flap controls are inactive"));
                pViscousLayout->addWidget(m_pchViscous);
                pViscousLayout->addWidget(lab11);
            }
            pMethodLayout->addWidget(m_pswAnalysisControls);
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
                QLabel *lab4 = new QLabel(QChar(0260));
                QLabel *lab5 = new QLabel(QChar(0260));
                m_pdeBeta  = new DoubleEdit(0.818,2);
                m_pdePhi   = new DoubleEdit(0.414,2);

                pAttitudeLayout->addWidget(lab2,1,1);
                pAttitudeLayout->addWidget(m_pdeBeta,1,2);
                pAttitudeLayout->addWidget(lab4 ,1,3);
                pAttitudeLayout->addWidget(lab3,2,1);
                pAttitudeLayout->addWidget(m_pdePhi,2,2);
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
                m_prbArea1 = new QRadioButton(tr("Wing Planform"));
                m_prbArea2 = new QRadioButton(tr("Wing Planform projected on xy plane"));
                m_prbArea3 = new QRadioButton(tr("Manual input"));

                QGridLayout *pRefAreaLayout = new QGridLayout;
                {
                    QLabel *plabRefArea  = new QLabel(tr("Ref. area="));
                    QLabel *plabRefSpan  = new QLabel(tr("Ref. span length="));
                    QLabel *plabRefChord = new QLabel(tr("Ref. chord length="));
                    m_pdeRefArea  = new DoubleEdit(0.0, 3);
                    m_pdeRefChord = new DoubleEdit(0.0, 3);
                    m_pdeRefSpan  = new DoubleEdit(0.0, 3);
                    QString strUnit;
                    Units::getAreaUnitLabel(strUnit);
                    QLabel *plabAreaUnit = new QLabel(strUnit);
                    Units::getLengthUnitLabel(strUnit);
                    QLabel *plabLengthUnit4 = new QLabel(strUnit);
                    QLabel *plabLengthUnit5 = new QLabel(strUnit);

                    plabRefArea->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    plabRefSpan->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    plabRefChord->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    plabAreaUnit->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
                    plabLengthUnit4->setAlignment(Qt::AlignLeft | Qt::AlignCenter);

                    pRefAreaLayout->addWidget(plabRefArea,     1,1);
                    pRefAreaLayout->addWidget(m_pdeRefArea,    1,2);
                    pRefAreaLayout->addWidget(plabAreaUnit,    1,3);
                    pRefAreaLayout->addWidget(plabRefSpan,     2,1);
                    pRefAreaLayout->addWidget(m_pdeRefSpan,    2,2);
                    pRefAreaLayout->addWidget(plabLengthUnit4, 2,3);
                    pRefAreaLayout->addWidget(plabRefChord,    3,1);
                    pRefAreaLayout->addWidget(m_pdeRefChord,   3,2);
                    pRefAreaLayout->addWidget(plabLengthUnit5, 3,3);
                    pRefAreaLayout->setColumnStretch(1,1);
                }

                pAreaOptions->addWidget(m_prbArea1);
                pAreaOptions->addWidget(m_prbArea2);
                pAreaOptions->addWidget(m_prbArea3);
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
        m_pchAutoPlaneInertia = new QCheckBox(tr("Use plane inertia"));

        m_ptvInertiaControl = new QTableView(this);
        m_ptvInertiaControl->setFont(DisplayOptions::tableFont());

        m_ptvInertiaControl->setWindowTitle(tr("Controls"));
        m_ptvInertiaControl->setMinimumWidth(400);
        m_ptvInertiaControl->setMinimumHeight(150);
        m_ptvInertiaControl->setSelectionMode(QAbstractItemView::SingleSelection);
        m_ptvInertiaControl->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ptvInertiaControl->horizontalHeader()->setStretchLastSection(true);

        m_pInertiaControlModel = new CtrlTableModel(this);
        m_pInertiaControlModel->setRowCount(7);
        m_pInertiaControlModel->setColumnCount(4);
        m_pInertiaControlModel->setHeaderData(0, Qt::Horizontal, tr("Inertia parameter"));
        m_pInertiaControlModel->setHeaderData(1, Qt::Horizontal, tr("Mean value"));
        m_pInertiaControlModel->setHeaderData(2, Qt::Horizontal, tr("Gain")+QString::fromUtf8("(unit/ctrl)"));
        m_pInertiaControlModel->setHeaderData(3, Qt::Horizontal, tr("Unit"));

        m_ptvInertiaControl->setModel(m_pInertiaControlModel);

        m_pMassCtrlDelegate = new CtrlTableDelegate(this);
        m_ptvInertiaControl->setItemDelegate(m_pMassCtrlDelegate);

        m_pMassCtrlDelegate->setPrecision({2,3,3});

        pMassControlPageLayout->addWidget(m_pchAutoPlaneInertia);
        pMassControlPageLayout->addWidget(m_ptvInertiaControl);

        pMassControlPage->setLayout(pMassControlPageLayout);
    }

    QVBoxLayout *pAngleControlPageLayout  = new QVBoxLayout;
    {
        m_ptvAngleControl = new QTableView(this);
        m_ptvAngleControl->setFont(DisplayOptions::tableFont());

        m_ptvAngleControl->setWindowTitle(tr("Controls"));
        m_ptvAngleControl->setMinimumWidth(400);
        m_ptvAngleControl->setMinimumHeight(150);
        m_ptvAngleControl->setSelectionMode(QAbstractItemView::SingleSelection);
        m_ptvAngleControl->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ptvAngleControl->horizontalHeader()->setStretchLastSection(true);

        m_pAngleControlModel = new CtrlTableModel(this);
        m_pAngleControlModel->setRowCount(10);//temporary
        m_pAngleControlModel->setColumnCount(2);
        m_pAngleControlModel->setHeaderData(0, Qt::Horizontal, tr("Control Name"));
        m_pAngleControlModel->setHeaderData(1, Qt::Horizontal, tr("Gain")+QString::fromUtf8("(°/ctrl)"));


        m_ptvAngleControl->setModel(m_pAngleControlModel);

        m_pAngleCtrlDelegate = new CtrlTableDelegate(this);
        m_ptvAngleControl->setItemDelegate(m_pAngleCtrlDelegate);

        m_pAngleCtrlDelegate->setPrecision({1,2});


        QLabel* SignLabel = new QLabel(tr("Note: + sign means trailing edge down"));

        pAngleControlPageLayout->addWidget(m_ptvAngleControl);
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
                    m_prbUnit1 = new QRadioButton(tr("International"));
                    m_prbUnit2 = new QRadioButton(tr("Imperial"));
                    pAeroUnitLayout->addWidget(lab9);
                    pAeroUnitLayout->addWidget(m_prbUnit1);
                    pAeroUnitLayout->addWidget(m_prbUnit2);
                    pAeroUnitLayout->addStretch();
                }
                QGridLayout *pAeroDataValuesLayout = new QGridLayout;
                {
                    m_plabRho = new QLabel("r =");
                    m_pdeDensity = new DoubleEdit(1.225,3);
                    m_plabDensityUnit = new QLabel("kg/m3");
                    m_plabNu = new QLabel("n =");
                    m_plabRho->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_plabNu->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pdeViscosity = new DoubleEdit(1.500e-5,3);
                    m_plabViscosityUnit = new QLabel("m2/s");
                    m_plabRho->setFont(symbolFont);
                    m_plabNu->setFont(symbolFont);
                    m_pdeDensity->setDigits(5);
                    m_pdeViscosity->setDigits(3);
                    m_pdeDensity->setMin(0.0);
                    m_pdeViscosity->setMin(0.0);
                    pAeroDataValuesLayout->addWidget(m_plabRho,1,1);
                    pAeroDataValuesLayout->addWidget(m_pdeDensity,1,2);
                    pAeroDataValuesLayout->addWidget(m_plabDensityUnit,1,3);
                    pAeroDataValuesLayout->addWidget(m_plabNu,2,1);
                    pAeroDataValuesLayout->addWidget(m_pdeViscosity,2,2);
                    pAeroDataValuesLayout->addWidget(m_plabViscosityUnit,2,3);
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
        m_ptvExtraDragControl = new QTableView(this);
        m_ptvExtraDragControl->setFont(DisplayOptions::tableFont());

        m_ptvExtraDragControl->setWindowTitle(tr("Extra drag"));
        m_ptvExtraDragControl->setMinimumWidth(400);
        m_ptvExtraDragControl->setMinimumHeight(150);
        m_ptvExtraDragControl->setSelectionMode(QAbstractItemView::SingleSelection);
        m_ptvExtraDragControl->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ptvExtraDragControl->horizontalHeader()->setStretchLastSection(true);

        m_pExtraDragControlModel = new CtrlTableModel(this);
        m_pExtraDragControlModel->setColumnCount(3);
        m_pExtraDragControlModel->setHeaderData(0, Qt::Horizontal, tr("Extra drag"));
        m_pExtraDragControlModel->setHeaderData(1, Qt::Horizontal, tr("Extra area")+" ("+Units::areaUnitLabel()+")");
        m_pExtraDragControlModel->setHeaderData(2, Qt::Horizontal, tr("Extra drag coef."));


        m_ptvExtraDragControl->setModel(m_pExtraDragControlModel);

        m_pDragCtrlDelegate = new CtrlTableDelegate(this);
        m_ptvExtraDragControl->setItemDelegate(m_pDragCtrlDelegate);

        m_pDragCtrlDelegate->setPrecision({0,3,5});

        QLabel* pExtraLabel = new QLabel(QString::fromUtf8("D = 1/2 rho V² ( S (CD_induced+CD_Visc) + S_Extra1.CD_Extra1 + ... + S_ExtraN.Cd_ExtraN)"));

        pExtraDragPageLayout->addWidget(m_ptvExtraDragControl);
        pExtraDragPageLayout->addWidget(pExtraLabel);
        pExtraDragPage->setLayout(pExtraDragPageLayout);
    }

    m_ptwMain->addTab(pMethodPage,       tr("Analysis"));
    m_ptwMain->addTab(pCoefficientPage,  tr("Ref. dimensions"));
    m_ptwMain->addTab(pMassControlPage,  tr("Mass and inertia"));
    m_ptwMain->addTab(pAngleControlPage, tr("Control parameters"));
    m_ptwMain->addTab(pOptionPage,       tr("Aero data"));
    m_ptwMain->addTab(pExtraDragPage,    tr("Extra drag"));

    m_ptwMain->setCurrentIndex(0);

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        QHBoxLayout *pPolarNameLayout = new QHBoxLayout;
        {
            m_pchAutoName = new QCheckBox(tr("Auto Analysis Name"));
            m_pleWPolarName = new QLineEdit(tr("Polar Name"));
            pPolarNameLayout->addWidget(m_pchAutoName);
            pPolarNameLayout->addWidget(m_pleWPolarName);
        }

        pMainLayout->addLayout(pPolarNameLayout,1);
        pMainLayout->addSpacing(13);
        pMainLayout->addWidget(m_ptwMain,13);
        pMainLayout->addStretch(1);
        pMainLayout->addSpacing(23);
        pMainLayout->addWidget(m_pButtonBox,1);
    }

    setLayout(pMainLayout);
}


void StabPolarDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_Geometry);
    pEvent->accept();
}


void StabPolarDlg::hideEvent(QHideEvent *pEvent)
{
    s_Geometry = saveGeometry();
    pEvent->accept();
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

    m_ptvExtraDragControl->resizeColumnsToContents();
}


void StabPolarDlg::onAeroData()
{
    AeroDataDlg dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        s_StabWPolar.setDensity(dlg.AirDensity());
        s_StabWPolar.setViscosity(dlg.KinematicViscosity());

        if(m_prbUnit1->isChecked())
        {
            m_pdeViscosity->setValue(s_StabWPolar.viscosity());
        }
        else
        {
            m_pdeViscosity->setValue(s_StabWPolar.viscosity()* 10.7182881);
        }
        setDensity();
    }
}


void StabPolarDlg::enableControls()
{
    m_pleWPolarName->setEnabled(!m_pchAutoName->isChecked());

    m_pdeRefArea->setEnabled(m_prbArea3->isChecked());
    m_pdeRefChord->setEnabled(m_prbArea3->isChecked());
    m_pdeRefSpan->setEnabled(m_prbArea3->isChecked());

    m_ptvInertiaControl->setEnabled(!s_StabWPolar.bAutoInertia());
}


void StabPolarDlg::setWPolarName()
{
    if(!m_bAutoName || !m_pPlane) return;

    xfl::setAutoWPolarName(&s_StabWPolar, m_pPlane);
    m_pleWPolarName->setText(s_StabWPolar.polarName());
}


void StabPolarDlg::onMethod()
{
    if (m_prbWingMethod2->isChecked())
    {
        s_StabWPolar.setThinSurfaces(true);
        s_StabWPolar.setAnalysisMethod(xfl::VLMMETHOD);
    }
    else if (m_prbWingMethod3->isChecked())
    {
        s_StabWPolar.setThinSurfaces(false);
        s_StabWPolar.setAnalysisMethod(xfl::PANEL4METHOD);
    }

    setWPolarName();
}



