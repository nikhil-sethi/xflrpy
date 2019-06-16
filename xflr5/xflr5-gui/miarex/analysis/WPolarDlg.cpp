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

#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QMessageBox>

#include <math.h>

#include "AeroDataDlg.h"
#include "CtrlTableDelegate.h"
#include "WPolarDlg.h"
#include <globals/globals.h>
#include <miarex/Miarex.h>
#include <misc/options/units.h>
#include <misc/options/displayoptions.h>
#include <misc/text/DoubleEdit.h>
#include <objects/objects3d/Plane.h>
#include <objects/objects3d/WPolar.h>


WPolar WPolarDlg::s_WPolar;

WPolarDlg::WPolarDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Analysis Definition"));

    m_pPlane     = nullptr;
    m_anglePrecision = nullptr;
    m_bAutoName = true;
    m_WingLoad   = 0.0;
    m_UnitType  = 1;

    setupLayout();
    connectSignals();
}


WPolarDlg::~WPolarDlg()
{
    delete m_pCtrlDelegate;
    if(m_anglePrecision) delete[] m_anglePrecision;
}


void WPolarDlg::connectSignals()
{
    connect(m_pctrlAutoName, SIGNAL(toggled(bool)), this, SLOT(onAutoName()));
    connect(m_pctrlLLTMethod, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_pctrlVLM1Method, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_pctrlVLM2Method, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_pctrlPanelMethod, SIGNAL(toggled(bool)), this, SLOT(onMethod()));

    connect(m_pctrlUnit1, SIGNAL(toggled(bool)), this, SLOT(onUnit()));
    connect(m_pctrlUnit2, SIGNAL(toggled(bool)), this, SLOT(onUnit()));

    connect(m_pctrlType1, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));
    connect(m_pctrlType2, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));
    connect(m_pctrlType4, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));
    connect(m_pctrlType5, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));

    connect(m_pctrlTiltGeom, SIGNAL(clicked()), this, SLOT(onTiltedGeom()));
    connect(m_pctrlViscous, SIGNAL(clicked()), this, SLOT(onViscous()));
    connect(m_pctrlIgnoreBodyPanels, SIGNAL(clicked()), this, SLOT(onIgnoreBodyPanels()));

    connect(m_pctrlGroundEffect, SIGNAL(clicked()), this, SLOT(onGroundEffect()));
    connect(m_pctrlPlaneInertia, SIGNAL(clicked()), this, SLOT(onPlaneInertia()));

    connect(m_pctrlXCmRef,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlZCmRef,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlDensity,    SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlViscosity,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlAlpha,      SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlBeta,       SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlWeight,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlQInf,       SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlHeight,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlRefArea,    SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlRefSpan,    SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlRefChord,   SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlWPolarName, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pctrlWPolarName, SIGNAL(textEdited ( const QString &  )), this, SLOT(onPolarName()));

    connect(m_pctrlArea1, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_pctrlArea2, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_pctrlArea3, SIGNAL(clicked()),this, SLOT(onArea()));
}


void WPolarDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)           onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void WPolarDlg::enableControls()
{
    m_pctrlWPolarName->setEnabled(!m_pctrlAutoName->isChecked());

    switch (s_WPolar.polarType())
    {
        case XFLR5::FIXEDSPEEDPOLAR:
        {
            m_pctrlQInf->setEnabled(true);
            m_pctrlAlpha->setEnabled(false);
            break;
        }
        case XFLR5::FIXEDLIFTPOLAR:
        {
            m_pctrlQInf->setEnabled(false);
            m_pctrlAlpha->setEnabled(false);
            break;
        }
        case XFLR5::FIXEDAOAPOLAR:
        {
            m_pctrlQInf->setEnabled(false);
            m_pctrlAlpha->setEnabled(true);
            break;
        }
        case XFLR5::BETAPOLAR:
        {
            m_pctrlQInf->setEnabled(true);
            m_pctrlAlpha->setEnabled(true);
            break;
        }
        default:
        {
            m_pctrlQInf->setEnabled(true);
            break;
        }
    }

    m_pctrlViscous->setEnabled(s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD);
    m_pctrlTiltGeom->setEnabled(s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD);
    m_pctrlIgnoreBodyPanels->setEnabled(m_pPlane && m_pPlane->body());
    m_pctrlBeta->setEnabled(s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD && s_WPolar.polarType()!=XFLR5::BETAPOLAR);
    m_pctrlGroundEffect->setEnabled(s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD);
    m_pctrlHeight->setEnabled(m_pctrlGroundEffect->isChecked() && s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD);

    m_pctrlWeight->setEnabled(!s_WPolar.m_bAutoInertia);
    m_pctrlXCmRef->setEnabled(!s_WPolar.m_bAutoInertia);
    m_pctrlZCmRef->setEnabled(!s_WPolar.m_bAutoInertia);

    m_pctrlVLM1Method->setEnabled(!s_WPolar.isBetaPolar() && fabs(s_WPolar.m_BetaSpec)<PRECISION);

    m_pctrlRefArea->setEnabled(m_pctrlArea3->isChecked());
    m_pctrlRefChord->setEnabled(m_pctrlArea3->isChecked());
    m_pctrlRefSpan->setEnabled(m_pctrlArea3->isChecked());
}



void WPolarDlg::initDialog(Plane *pPlane, WPolar *pWPolar)
{
    m_pPlane = pPlane;
    if(!m_pPlane) return;

    blockSignals(true);

    if(pWPolar)
    {
        m_pctrlAutoName->setChecked(false);
        m_bAutoName = false;
        m_pctrlWPolarName->setText(pWPolar->polarName());
        s_WPolar.duplicateSpec(pWPolar);
    }
    else
    {
        m_pctrlAutoName->setChecked(true);
    }


    if(m_pPlane->isWing())
    {
    }
    else
    {
        s_WPolar.analysisMethod()=XFLR5::VLMMETHOD;
        s_WPolar.bThinSurfaces() = true;
        m_pctrlPanelMethod->setVisible(false);
    }

    //initialize the name box
    s_WPolar.planeName() = m_pPlane->planeName();

    // initialize units
    if(m_UnitType==1) m_pctrlUnit1->setChecked(true);
    else              m_pctrlUnit2->setChecked(true);


    //initialize polar type
    if(s_WPolar.polarType()==XFLR5::FIXEDSPEEDPOLAR)     m_pctrlType1->setChecked(true);
    else if(s_WPolar.polarType()==XFLR5::FIXEDLIFTPOLAR) m_pctrlType2->setChecked(true);
    else if(s_WPolar.polarType()==XFLR5::FIXEDAOAPOLAR)  m_pctrlType4->setChecked(true);
    else if(s_WPolar.polarType()==XFLR5::BETAPOLAR)      m_pctrlType5->setChecked(true);


    //initialize inertia
    if(s_WPolar.m_bAutoInertia)
    {
        m_pctrlWeight->setValue(m_pPlane->totalMass() * Units::kgtoUnit());
        m_pctrlXCmRef->setValue(m_pPlane->CoG().x * Units::mtoUnit());
        m_pctrlZCmRef->setValue(m_pPlane->CoG().z * Units::mtoUnit());
        s_WPolar.mass()    = m_pPlane->totalMass();
        s_WPolar.CoG().x   = m_pPlane->CoG().x;
        s_WPolar.CoG().z   = m_pPlane->CoG().z;
    }
    else
    {
        m_pctrlWeight->setValue(s_WPolar.mass()  * Units::kgtoUnit());
        m_pctrlXCmRef->setValue(s_WPolar.CoG().y * Units::mtoUnit());
        m_pctrlZCmRef->setValue(s_WPolar.CoG().z * Units::mtoUnit());
    }


    //initialize ground data
    m_pctrlHeight->setValue(s_WPolar.m_Height*Units::mtoUnit());
    if(s_WPolar.bGround())
    {
        m_pctrlHeight->setEnabled(true);
        m_pctrlGroundEffect->setChecked(true);
    }
    else
    {
        m_pctrlHeight->setEnabled(false);
        m_pctrlGroundEffect->setChecked(false);
    }


    m_pctrlXCmRef->setValue(s_WPolar.CoG().x*Units::mtoUnit());
    m_pctrlZCmRef->setValue(s_WPolar.CoG().z*Units::mtoUnit());

    m_pctrlQInf->setValue(s_WPolar.m_QInfSpec*Units::mstoUnit());
    m_pctrlWeight->setValue(s_WPolar.mass()*Units::kgtoUnit());
    m_pctrlBeta->setValue(s_WPolar.m_BetaSpec);
    m_pctrlAlpha->setValue(s_WPolar.m_AlphaSpec);


    m_pctrlViscous->setChecked(s_WPolar.bViscous());
    m_pctrlTiltGeom->setChecked(s_WPolar.bTilted());

    // force ignore body panels by default
    s_WPolar.bIgnoreBodyPanels()=true;
    m_pctrlIgnoreBodyPanels->setChecked(m_pPlane->body() || s_WPolar.bIgnoreBodyPanels());
    //	if(!m_pPlane) s_WPolar.bIgnoreBodyPanels()=false;


    if(s_WPolar.analysisMethod()==XFLR5::LLTMETHOD)
    {
        m_pctrlLLTMethod->setChecked(true);
        m_pctrlViscous->setChecked(true);
        m_pctrlViscous->setEnabled(false);
    }
    else if(s_WPolar.analysisMethod()==XFLR5::VLMMETHOD)
    {
        m_pctrlVLM1Method->setChecked( s_WPolar.bVLM1());
        m_pctrlVLM2Method->setChecked(!s_WPolar.bVLM1());
        m_pctrlViscous->setEnabled(true);
    }
    else if(s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD)
    {
        if(s_WPolar.bThinSurfaces())
        {
            m_pctrlVLM1Method->setChecked(s_WPolar.bVLM1());
            m_pctrlVLM2Method->setChecked(!s_WPolar.bVLM1());
        }
        else
        {
            m_pctrlPanelMethod->setChecked(true);
        }
        m_pctrlViscous->setEnabled(true);
    }


    m_pctrlArea1->setChecked(s_WPolar.referenceDim()==XFLR5::PLANFORMREFDIM);
    m_pctrlArea2->setChecked(s_WPolar.referenceDim()==XFLR5::PROJECTEDREFDIM);
    m_pctrlArea3->setChecked(s_WPolar.referenceDim()==XFLR5::MANUALREFDIM);

    if(m_pctrlArea1->isChecked())
    {
        s_WPolar.referenceArea() = m_pPlane->planformArea();
        s_WPolar.referenceSpanLength() = m_pPlane->planformSpan();
        m_pctrlRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_pctrlArea2->isChecked())
    {
        s_WPolar.referenceArea() = m_pPlane->projectedArea();
        s_WPolar.referenceSpanLength() = m_pPlane->projectedSpan();
        m_pctrlRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
    }
    else if(m_pctrlArea3->isChecked())
    {
        m_pctrlRefArea->setValue(s_WPolar.referenceArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
    }

    s_WPolar.referenceChordLength() = m_pPlane->mac();
    m_pctrlRefChord->setValue(s_WPolar.referenceChordLength()*Units::mtoUnit());

    s_WPolar.bWakeRollUp() = false;

    m_pctrlPlaneInertia->setChecked(s_WPolar.m_bAutoInertia);


    setDensity();
    setWingLoad();
    setReynolds();
    setWPolarName();
    fillExtraDragList();
    enableControls();

    m_pctrlQInf->setSelection(0,-1);
    m_pctrlQInf->setFocus();

    blockSignals(false);

}



void WPolarDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                readExtraDragData();
                readValues();
                setWPolarName();
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                onOK();
                return;
            }
            break;
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


void WPolarDlg::onArea()
{
    if(m_pctrlArea1->isChecked())
    {
        s_WPolar.referenceDim() = XFLR5::PLANFORMREFDIM;
        m_pctrlRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pctrlRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
        m_pctrlRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_pctrlArea2->isChecked())
    {
        s_WPolar.referenceDim() = XFLR5::PROJECTEDREFDIM;
        m_pctrlRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pctrlRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
        m_pctrlRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
    }
    else if(m_pctrlArea3->isChecked())
    {
        s_WPolar.referenceDim() = XFLR5::MANUALREFDIM;
        //		m_pctrlRefArea->SetValue(s_WPolar.referenceArea()Length*Units::m2toUnit());
        //		m_pctrlRefSpan->SetValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
        //		m_pctrlRefChord->SetValue(s_WPolar.referenceChordLength()*Units::mtoUnit());
    }

    setWPolarName();
    enableControls();
}



void WPolarDlg::onEditingFinished()
{
    readExtraDragData();
    readValues();
    setReynolds();
    setWPolarName();
    enableControls();
}


void WPolarDlg::onAutoName()
{
    m_bAutoName = m_pctrlAutoName->isChecked();
    if(m_bAutoName) setWPolarName();
    enableControls();
}


void WPolarDlg::onTiltedGeom()
{
    s_WPolar.bTilted() = m_pctrlTiltGeom->isChecked();
    setWPolarName();
    enableControls();
}


void WPolarDlg::onPlaneInertia()
{
    if(m_pctrlPlaneInertia->isChecked())
    {
        if(m_pPlane)
        {
            m_pctrlWeight->setValue(m_pPlane->totalMass() * Units::kgtoUnit());
            m_pctrlXCmRef->setValue(m_pPlane->CoG().x * Units::mtoUnit());
            m_pctrlZCmRef->setValue(m_pPlane->CoG().z * Units::mtoUnit());
            s_WPolar.mass()    = m_pPlane->totalMass();
            s_WPolar.CoG().x   = m_pPlane->CoG().x;
            s_WPolar.CoG().z   = m_pPlane->CoG().z;
        }
    }
    else
    {
        s_WPolar.mass()    = m_pctrlWeight->value() / Units::kgtoUnit();
        s_WPolar.CoG().x   = m_pctrlXCmRef->value() / Units::mtoUnit();
        s_WPolar.CoG().z   = m_pctrlZCmRef->value() / Units::mtoUnit();
    }
    s_WPolar.m_bAutoInertia = m_pctrlPlaneInertia->isChecked();
    setWPolarName();
    enableControls();
}


void WPolarDlg::onViscous()
{
    s_WPolar.bViscous() = m_pctrlViscous->isChecked();
    setWPolarName();
    enableControls();
}


void WPolarDlg::onIgnoreBodyPanels()
{
    s_WPolar.bIgnoreBodyPanels() = m_pctrlIgnoreBodyPanels->isChecked();
    setWPolarName();
    enableControls();
}


void WPolarDlg::onGroundEffect()
{
    s_WPolar.bGround() = m_pctrlGroundEffect->isChecked();
    m_pctrlHeight->setEnabled(s_WPolar.bGround());
    setWPolarName();
}


void WPolarDlg::onMethod()
{
    if (m_pctrlLLTMethod->isChecked())
    {
        s_WPolar.bViscous()      = true;
        s_WPolar.bThinSurfaces() = true;
        s_WPolar.bWakeRollUp()   = false;
        s_WPolar.bTilted()   = false;
        s_WPolar.analysisMethod()  = XFLR5::LLTMETHOD;
        m_pctrlTiltGeom->setChecked(false);
    }
    else if (m_pctrlVLM1Method->isChecked() || m_pctrlVLM2Method->isChecked())
    {
        s_WPolar.bVLM1() = m_pctrlVLM1Method->isChecked();
        s_WPolar.bThinSurfaces() = true;
        s_WPolar.analysisMethod() = XFLR5::PANEL4METHOD;
    }
    else if (m_pctrlPanelMethod->isChecked())
    {
        s_WPolar.bThinSurfaces() = false;
        s_WPolar.analysisMethod() = XFLR5::PANEL4METHOD;
    }

    enableControls();
    setWPolarName();
}


void WPolarDlg::readExtraDragData()
{
    for(int i=0; i<MAXEXTRADRAG; i++)
    {
        s_WPolar.m_ExtraDragArea[i]= m_pExtraDragControlModel->index(i, 1, QModelIndex()).data().toDouble()/Units::m2toUnit();
        s_WPolar.m_ExtraDragCoef[i]= m_pExtraDragControlModel->index(i, 2, QModelIndex()).data().toDouble();
    }
}


void WPolarDlg::onOK()
{
    if(!m_pctrlWPolarName->text().length())
    {
        QMessageBox::warning(this, tr("Warning"),tr("Must enter a name for the polar"));
        m_pctrlWPolarName->setFocus();
        return;
    }
    s_WPolar.polarName() = m_pctrlWPolarName->text();

    if(qAbs(s_WPolar.mass())<PRECISION && s_WPolar.polarType()==XFLR5::FIXEDLIFTPOLAR)
    {
        QMessageBox::warning(this, tr("Warning"),tr("Mass must be non-zero for type 2 polars"));
        m_pctrlWeight->setFocus();
        return;
    }
    if(!m_pPlane->isWing() && s_WPolar.analysisMethod()==XFLR5::PANEL4METHOD) s_WPolar.bThinSurfaces() = true;

    readExtraDragData();

    accept();
}


void WPolarDlg::onUnit()
{
    if(m_pctrlUnit1->isChecked())
    {
        m_UnitType   = 1;
        m_pctrlViscosity->setValue(s_WPolar.m_Viscosity);
        m_pctrlDensityUnit->setText("kg/m3");
        m_pctrlViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");
    }
    else
    {
        m_UnitType   = 2;
        m_pctrlViscosity->setValue(s_WPolar.m_Viscosity* 10.7182881);
        m_pctrlDensityUnit->setText("slugs/ft3");
        m_pctrlViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");
    }
    setDensity();
}


void WPolarDlg::onPolarName()
{
    m_bAutoName = false;
    m_pctrlAutoName->setChecked(false);
}



void WPolarDlg::onPolarType()
{
    if (m_pctrlType1->isChecked())
    {
        s_WPolar.polarType() = XFLR5::FIXEDSPEEDPOLAR;
    }
    else if(m_pctrlType2->isChecked())
    {
        s_WPolar.polarType() = XFLR5::FIXEDLIFTPOLAR;
    }
    else if(m_pctrlType4->isChecked())
    {
        s_WPolar.polarType() = XFLR5::FIXEDAOAPOLAR;
    }
    else if(m_pctrlType5->isChecked())
    {
        s_WPolar.polarType() = XFLR5::BETAPOLAR;
        s_WPolar.bVLM1() = false;
        if(m_pctrlVLM1Method->isChecked())
        {
            m_pctrlVLM1Method->blockSignals(true);
            m_pctrlVLM2Method->blockSignals(true);
            m_pctrlVLM2Method->setChecked(true);
            m_pctrlVLM1Method->blockSignals(false);
            m_pctrlVLM2Method->blockSignals(false);
        }
    }
    enableControls();
    setReynolds();
    setWPolarName();
}


void WPolarDlg::readValues()
{
    s_WPolar.m_AlphaSpec     = m_pctrlAlpha->value();
    s_WPolar.m_BetaSpec      = m_pctrlBeta->value();
    if(fabs(s_WPolar.m_BetaSpec)>PRECISION)
    {
        s_WPolar.bVLM1() = false;
        if(m_pctrlVLM1Method->isChecked())
        {
            m_pctrlVLM1Method->blockSignals(true);
            m_pctrlVLM2Method->blockSignals(true);
            m_pctrlVLM2Method->setChecked(true);
            m_pctrlVLM1Method->blockSignals(false);
            m_pctrlVLM2Method->blockSignals(false);
        }
    }

    s_WPolar.mass()          = m_pctrlWeight->value() / Units::kgtoUnit();
    s_WPolar.CoG().x         = m_pctrlXCmRef->value() / Units::mtoUnit();
    s_WPolar.CoG().z         = m_pctrlZCmRef->value() / Units::mtoUnit();
    s_WPolar.m_QInfSpec      = m_pctrlQInf->value() / Units::mstoUnit();
    s_WPolar.m_Height        = m_pctrlHeight->value() / Units::mtoUnit();

    if(m_pctrlUnit1->isChecked())
    {
        s_WPolar.viscosity() = m_pctrlViscosity->value();
        s_WPolar.density()   = m_pctrlDensity->value();
    }
    else
    {
        s_WPolar.density()   = m_pctrlDensity->value() / 0.00194122;
        s_WPolar.viscosity() = m_pctrlViscosity->value() / 10.7182881;
    }

    //    qDebug("%13.8g    %13.8g",m_pctrlViscosity->value(),s_WPolar.viscosity());

    if(m_pctrlArea1->isChecked())
    {
        s_WPolar.referenceDim() = XFLR5::PLANFORMREFDIM;
        s_WPolar.referenceArea()       = m_pPlane->planformArea();
        s_WPolar.referenceSpanLength() = m_pPlane->planformSpan();
    }
    else if(m_pctrlArea2->isChecked())
    {
        s_WPolar.referenceDim() = XFLR5::PROJECTEDREFDIM;
        s_WPolar.referenceArea()       = m_pPlane->projectedArea();
        s_WPolar.referenceSpanLength() = m_pPlane->projectedSpan();
    }
    else if(m_pctrlArea3->isChecked())
    {
        s_WPolar.referenceDim() = XFLR5::MANUALREFDIM;
        s_WPolar.referenceArea()       = m_pctrlRefArea->value() /Units::m2toUnit();
        s_WPolar.referenceSpanLength() = m_pctrlRefSpan->value() /Units::mtoUnit();
    }

    s_WPolar.referenceChordLength() = m_pctrlRefChord->value() /Units::mtoUnit();

    setDensity();

    setWingLoad();
}



void WPolarDlg::setDensity()
{
    int exp, precision;
    if(m_pctrlUnit1->isChecked())
    {
        exp = (int)log(s_WPolar.density());
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pctrlDensity->setPrecision(precision);
        m_pctrlDensity->setValue(s_WPolar.density());
    }
    else
    {
        exp = (int)log(s_WPolar.density()* 0.00194122);
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pctrlDensity->setPrecision(precision);
        m_pctrlDensity->setValue(s_WPolar.density()* 0.00194122);
    }
}


void WPolarDlg::setupLayout()
{
    QString strSpeedUnit, strLengthUnit, strWeightUnit;

    Units::getSpeedUnitLabel(strSpeedUnit);
    Units::getLengthUnitLabel(strLengthUnit);
    Units::getWeightUnitLabel(strWeightUnit);


    QFont fnt;
    QFont symbolFont("Symbol");

    QFontMetrics fm(fnt);


    QTabWidget *pTabWidget = new QTabWidget(this);
    pTabWidget->setMinimumWidth(fm.averageCharWidth() * 83);
    QWidget *pPolarTypePage = new QWidget(this);
    QWidget *pMethodPage = new QWidget(this);
    QWidget *pInertiaPage = new QWidget(this);
    QWidget *pCoefficientPage = new QWidget(this);
    QWidget *pOptionPage = new QWidget(this);
    QWidget *pAngleControlPage = new QWidget(this);


    QVBoxLayout *pPolarTypePageLayout = new QVBoxLayout;
    {
        QHBoxLayout *pAnalysisSettingsLayout = new QHBoxLayout;
        {
            QVBoxLayout *pTypeLayout = new QVBoxLayout;
            {
                m_pctrlType1 = new QRadioButton(tr("Type 1 (Fixed Speed)"));
                m_pctrlType2 = new QRadioButton(tr("Type 2 (Fixed Lift)"));
                m_pctrlType4 = new QRadioButton(tr("Type 4 (Fixed aoa)"));
                m_pctrlType5 = new QRadioButton(tr("Type 5 (Beta range)"));
                pTypeLayout->addWidget(m_pctrlType1);
                pTypeLayout->addWidget(m_pctrlType2);
                pTypeLayout->addWidget(m_pctrlType4);
                pTypeLayout->addWidget(m_pctrlType5);
                pTypeLayout->addStretch();
            }

            QGridLayout *pTypeDataLayout = new QGridLayout;
            {
                QLabel *lab1 = new QLabel(tr("Free Stream Speed ="));
                QLabel *lab5 = new QLabel("a =");
                QLabel *lab6 = new QLabel("b =");
                lab5->setFont(symbolFont);
                lab6->setFont(symbolFont);
                lab1->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                lab5->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                lab6->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_pctrlQInf    = new DoubleEdit(10.05);
                m_pctrlQInf->setMin(0.0);
                m_pctrlAlpha   = new DoubleEdit(1.00,2);
                m_pctrlBeta    = new DoubleEdit(0.00,2);
                QLabel *labSpeedUnit   = new QLabel(strSpeedUnit);

                QLabel *lab7 = new QLabel(QString::fromUtf8("°"));
                QLabel *lab8 = new QLabel(QString::fromUtf8("°"));
                pTypeDataLayout->addWidget(lab1,1,1);
                pTypeDataLayout->addWidget(lab5,2,1);
                pTypeDataLayout->addWidget(lab6,3,1);
                pTypeDataLayout->addWidget(m_pctrlQInf,1,2);
                pTypeDataLayout->addWidget(m_pctrlAlpha,2,2);
                pTypeDataLayout->addWidget(m_pctrlBeta,3,2);
                pTypeDataLayout->addWidget(labSpeedUnit ,1,3);
                pTypeDataLayout->addWidget(lab7 ,2,3);
                pTypeDataLayout->addWidget(lab8 ,3,3);
                pTypeDataLayout->setRowStretch(4,1);
                pTypeDataLayout->setColumnStretch(3,1);
            }

            pAnalysisSettingsLayout->addLayout(pTypeLayout);
            pAnalysisSettingsLayout->addStretch();
            pAnalysisSettingsLayout->addLayout(pTypeDataLayout);
        }

        QVBoxLayout *pFlightLayout = new QVBoxLayout;
        {
            m_pctrlWingLoad  = new QLabel(tr("Wing Loading = 0.033 kg/dm2"));
            m_pctrlSRe       = new QLabel(tr("SRe"));
            m_pctrlRRe       = new QLabel(tr("RRe"));
            m_pctrlQInfCl    = new QLabel(tr("QInfCl"));

            m_pctrlWingLoad->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_pctrlSRe->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_pctrlRRe->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_pctrlQInfCl->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            pFlightLayout->addWidget(m_pctrlWingLoad);
            pFlightLayout->addWidget(m_pctrlSRe);
            pFlightLayout->addWidget(m_pctrlRRe);
            pFlightLayout->addWidget(m_pctrlQInfCl);
            pFlightLayout->addStretch();
        }

        pPolarTypePageLayout->addLayout(pAnalysisSettingsLayout);
        pPolarTypePageLayout->addStretch();
        pPolarTypePageLayout->addLayout(pFlightLayout);
        pPolarTypePage->setLayout(pPolarTypePageLayout);
    }

    QVBoxLayout *pMethodPageLayout = new QVBoxLayout;
    {
        QGroupBox *pAnalysisMethods = new QGroupBox(tr("Analysis Methods"));
        {
            QVBoxLayout *pMethodLayout = new QVBoxLayout;
            {
                m_pctrlLLTMethod   = new QRadioButton(tr("LLT (Wing only)"));
                m_pctrlVLM1Method = new QRadioButton(tr("Horseshoe vortex")+ " (VLM1) "+tr("(No sideslip)"));
                m_pctrlVLM2Method = new QRadioButton(tr("Ring vortex")+" (VLM2)");
                m_pctrlPanelMethod = new QRadioButton(tr("3D Panels"));

                pMethodLayout->addWidget(m_pctrlLLTMethod);
                pMethodLayout->addWidget(m_pctrlVLM1Method);
                pMethodLayout->addWidget(m_pctrlVLM2Method);
                pMethodLayout->addWidget(m_pctrlPanelMethod);
                pMethodLayout->addStretch();
            }
            pAnalysisMethods->setLayout(pMethodLayout);

        }

        QGroupBox *pOptionsGroupBox = new QGroupBox(tr("Options"));
        {
            QVBoxLayout *pOptionsLayout = new QVBoxLayout;
            {
                m_pctrlViscous = new QCheckBox(tr("Viscous"));
                m_pctrlTiltGeom = new QCheckBox(tr("Tilt. Geom."));
                pOptionsLayout->addWidget(m_pctrlViscous);
                pOptionsLayout->addWidget(m_pctrlTiltGeom);
                //			OptionsLayout->addWidget(m_pctrlIgnoreBody);
                m_pctrlIgnoreBodyPanels = new QCheckBox(tr("Ignore Body Panels - RECOMMENDED"));
                pOptionsLayout->addWidget(m_pctrlIgnoreBodyPanels);
                pOptionsLayout->addStretch();
            }
            pOptionsGroupBox->setLayout(pOptionsLayout);
        }

        pMethodPageLayout->addWidget(pAnalysisMethods);
        pMethodPageLayout->addWidget(pOptionsGroupBox);
        pMethodPage->setLayout(pMethodPageLayout);
    }

    QVBoxLayout *pInertiaPageLayout = new QVBoxLayout;
    {
        QGroupBox *pInertiaBox = new QGroupBox(tr("Inertia properties"));
        {
            QVBoxLayout *pInertiaLayout = new QVBoxLayout;
            {
                m_pctrlPlaneInertia = new QCheckBox(tr("Use plane inertia"));
                m_pctrlPlaneInertia->setToolTip("Activate this checbox for the polar to use dynamically the plane's inertia properties for each analysis");

                QGridLayout *pInertiaDataLayout = new QGridLayout;
                {
                    QLabel *lab2 = new QLabel(tr("Plane Mass ="));
                    QLabel *lab3 = new QLabel(tr("X_CoG ="));
                    QLabel *lab4 = new QLabel(tr("Z_CoG ="));
                    lab2->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    lab3->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    lab4->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    pInertiaDataLayout->addWidget(lab2,1,1);
                    pInertiaDataLayout->addWidget(lab3,2,1);
                    pInertiaDataLayout->addWidget(lab4,3,1);
                    m_pctrlWeight  = new DoubleEdit(0,3);
                    m_pctrlWeight->setMin(0.0);
                    m_pctrlXCmRef  = new DoubleEdit(100.00,3);
                    m_pctrlZCmRef  = new DoubleEdit(100.00,3);
                    pInertiaDataLayout->addWidget(m_pctrlWeight,1,2);
                    pInertiaDataLayout->addWidget(m_pctrlXCmRef,2,2);
                    pInertiaDataLayout->addWidget(m_pctrlZCmRef,3,2);
                    QLabel *labWeightUnit  = new QLabel(strWeightUnit);
                    QLabel *labLengthUnit1 = new QLabel(strLengthUnit);
                    QLabel *labLengthUnit3 = new QLabel(strLengthUnit);

                    pInertiaDataLayout->addWidget(labWeightUnit ,1,3);
                    pInertiaDataLayout->addWidget(labLengthUnit1 ,2,3);
                    pInertiaDataLayout->addWidget(labLengthUnit3 ,3,3);
                    pInertiaDataLayout->setColumnStretch(1,1);
                }
                pInertiaLayout->addWidget(m_pctrlPlaneInertia);
                pInertiaLayout->addLayout(pInertiaDataLayout);
                pInertiaLayout->addStretch();
            }
            pInertiaBox->setLayout(pInertiaLayout);
        }

        pInertiaPageLayout->addWidget(pInertiaBox);
        pInertiaPage->setLayout(pInertiaPageLayout);
    }

    QVBoxLayout *pCoefficientPageLayout = new QVBoxLayout;
    {
        QGroupBox *pAreaBox = new QGroupBox(tr("Ref. dimensions for aero coefficients"));
        {
            QVBoxLayout *pAreaOptions = new QVBoxLayout;
            {
                m_pctrlArea1 = new QRadioButton(tr("Wing Planform"));
                m_pctrlArea2 = new QRadioButton(tr("Wing Planform projected on xy plane"));
                m_pctrlArea3 = new QRadioButton(tr("User defined"));

                QGridLayout *pRefAreaLayout = new QGridLayout;
                {
                    QLabel *labRefArea  = new QLabel(tr("Ref. area="));
                    QLabel *labRefSpan  = new QLabel(tr("Ref. span length="));
                    QLabel *labRefChord = new QLabel(tr("Ref. chord length="));
                    m_pctrlRefArea  = new DoubleEdit(0.0, 3);
                    m_pctrlRefChord = new DoubleEdit(0.0, 3);
                    m_pctrlRefSpan  = new DoubleEdit(0.0, 3);
                    QLabel *labAreaUnit = new QLabel(Units::areaUnitLabel());
                    QLabel *labLengthUnit4 = new QLabel(Units::lengthUnitLabel());
                    QLabel *labLengthUnit5 = new QLabel(Units::lengthUnitLabel());

                    labRefArea->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    labRefSpan->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    labRefChord->setAlignment(Qt::AlignRight | Qt::AlignCenter);

                    labAreaUnit->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
                    labLengthUnit4->setAlignment(Qt::AlignLeft | Qt::AlignCenter);

                    pRefAreaLayout->addWidget(labRefArea,1,1);
                    pRefAreaLayout->addWidget(m_pctrlRefArea,1,2);
                    pRefAreaLayout->addWidget(labAreaUnit,1,3);
                    pRefAreaLayout->addWidget(labRefSpan,2,1);
                    pRefAreaLayout->addWidget(m_pctrlRefSpan,2,2);
                    pRefAreaLayout->addWidget(labLengthUnit4,2,3);
                    pRefAreaLayout->addWidget(labRefChord,3,1);
                    pRefAreaLayout->addWidget(m_pctrlRefChord,3,2);
                    pRefAreaLayout->addWidget(labLengthUnit5,3,3);

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

    QHBoxLayout *pOptionPageLayout =new QHBoxLayout;
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
                    m_pctrlDensity = new DoubleEdit(1.225,6);
                    m_pctrlDensityUnit = new QLabel("kg/m3");
                    m_pctrlNu = new QLabel("n =");
                    m_pctrlRho->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pctrlNu->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pctrlViscosity = new DoubleEdit(1.500e-5,3);
                    m_pctrlViscosityUnit = new QLabel("m2/s");
                    m_pctrlRho->setFont(symbolFont);
                    m_pctrlNu->setFont(symbolFont);
                    m_pctrlDensity->setPrecision(6);
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

        QGroupBox *pGroundGroupBox =  new QGroupBox(tr("Ground Effect"));
        {
            QVBoxLayout * pGroundLayout = new QVBoxLayout;
            {
                m_pctrlGroundEffect = new QCheckBox(tr("Ground Effect"));
                QHBoxLayout *pGroundHeightLayout = new QHBoxLayout;
                {
                    QLabel *lab10 = new QLabel(tr("Height ="));
                    m_pctrlHeight = new DoubleEdit(0.00,2);
                    m_pctrlHeight->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    QLabel *labLengthUnit2 = new QLabel(strLengthUnit);
                    pGroundHeightLayout->addStretch();
                    pGroundHeightLayout->addWidget(lab10);
                    pGroundHeightLayout->addWidget(m_pctrlHeight);
                    pGroundHeightLayout->addWidget(labLengthUnit2);
                }
                pGroundLayout->addWidget(m_pctrlGroundEffect);
                pGroundLayout->addLayout(pGroundHeightLayout);
                pGroundLayout->addStretch(1);
            }
            pGroundGroupBox->setLayout(pGroundLayout);
        }

        pOptionPageLayout->addWidget(pAeroDataGroupBox);

        pOptionPageLayout->addWidget(pGroundGroupBox);
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

        m_pExtraDragControlModel = new QStandardItemModel(this);
        m_pExtraDragControlModel->setColumnCount(3);
        m_pExtraDragControlModel->setHeaderData(0, Qt::Horizontal, tr("Extra drag"));
        m_pExtraDragControlModel->setHeaderData(1, Qt::Horizontal, tr("Extra area")+" ("+Units::areaUnitLabel()+")");
        m_pExtraDragControlModel->setHeaderData(2, Qt::Horizontal, tr("Extra drag coef."));


        m_pExtraDragControlTable->setModel(m_pExtraDragControlModel);

        m_pCtrlDelegate = new CtrlTableDelegate(this);
        m_pExtraDragControlTable->setItemDelegate(m_pCtrlDelegate);
        m_pCtrlDelegate->m_pCtrlModel = m_pExtraDragControlModel;

        connect(m_pCtrlDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onEditingFinished()));


        m_anglePrecision = new int[3];
        m_anglePrecision[0]  = 0;
        m_anglePrecision[1]  = 3;
        m_anglePrecision[2]  = 5;

        m_pCtrlDelegate->m_Precision = m_anglePrecision;

        QLabel* pExtraLabel = new QLabel(QString::fromUtf8("D = 1/2 rho V² ( S (CD_induced+CD_Visc) + S_Extra1.CD_Extra1 + ... + S_ExtraN.Cd_ExtraN)"));

        pExtraDragPageLayout->addWidget(m_pExtraDragControlTable);
        pExtraDragPageLayout->addWidget(pExtraLabel);
        pAngleControlPage->setLayout(pExtraDragPageLayout);
    }

    pTabWidget->addTab(pPolarTypePage, tr("Polar Type"));
    pTabWidget->addTab(pMethodPage, tr("Analysis"));
    pTabWidget->addTab(pInertiaPage, tr("Inertia"));
    pTabWidget->addTab(pCoefficientPage, tr("Ref. dimensions"));
    pTabWidget->addTab(pOptionPage, tr("Aero data"));
    pTabWidget->addTab(pAngleControlPage, tr("Extra drag"));

    pTabWidget->setCurrentIndex(0);
    connect(pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
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
        pMainLayout->addWidget(pTabWidget,13);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox,1);
    }
    setLayout(pMainLayout);
}



void WPolarDlg::fillExtraDragList()
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
        m_pExtraDragControlModel->setData(ind, s_WPolar.m_ExtraDragArea[i]*Units::m2toUnit());
        ind = m_pExtraDragControlModel->index(i, 2, QModelIndex());
        m_pExtraDragControlModel->setData(ind, s_WPolar.m_ExtraDragCoef[i]);
    }

    m_pExtraDragControlTable->resizeColumnsToContents();
}



void WPolarDlg::onTabChanged(int index)
{
    if(index==5)
    {
        resizeColumns();
    }
}


void WPolarDlg::resizeColumns()
{
    double wc = (double)m_pExtraDragControlTable->width()*.97;
    int wCols  = (int)(wc/3);
    m_pExtraDragControlTable->setColumnWidth(0, wCols);
    m_pExtraDragControlTable->setColumnWidth(1, wCols);
    m_pExtraDragControlTable->setColumnWidth(2, wCols);

}


void WPolarDlg::setWPolarName()
{
    if(!m_bAutoName) return;

    setAutoWPolarName(&s_WPolar, m_pPlane);
    m_pctrlWPolarName->setText(s_WPolar.polarName());
}



void WPolarDlg::setWingLoad()
{
    QString str,str1, str2;

    if(s_WPolar.referenceArea()>0)
    {
        m_WingLoad = s_WPolar.mass()/s_WPolar.referenceArea();//kg/dm2

        str = QString("%1 ").arg(m_WingLoad * Units::kgtoUnit() / Units::m2toUnit(),7,'f',3);

        Units::getWeightUnitLabel(str1);
        Units::getAreaUnitLabel(str2);
    }
    m_pctrlWingLoad->setText(tr("Wing Loading = ")+str+str1+"/"+str2);
}



void WPolarDlg::setReynolds()
{
    QString strange, str, strUnit;
    Units::getSpeedUnitLabel(strUnit);

    if(s_WPolar.polarType() == XFLR5::FIXEDSPEEDPOLAR)
    {
        double RRe = m_pPlane->rootChord() * s_WPolar.m_QInfSpec/s_WPolar.m_Viscosity;
        ReynoldsFormat(str, RRe);
        strange = tr("Root Re =");
        m_pctrlRRe->setText(strange+str);

        double SRe = m_pPlane->tipChord() * s_WPolar.m_QInfSpec/s_WPolar.m_Viscosity;
        ReynoldsFormat(str, SRe);
        strange = tr("Tip Re =");
        m_pctrlSRe->setText(strange+str);

        m_pctrlQInfCl->setText(" ");
    }
    else if(s_WPolar.polarType() == XFLR5::FIXEDLIFTPOLAR)
    {
        double QCl =  sqrt(2.* 9.81 /s_WPolar.density()* s_WPolar.mass() /s_WPolar.referenceArea()) * Units::mstoUnit();
        str = QString("%1 ").arg(QCl,5,'f',2);
        str += strUnit;
        strange = tr("Vinf.sqrt(Cl) =");
        m_pctrlQInfCl->setText(strange+str);

        double RRe = m_pPlane->rootChord() * QCl/s_WPolar.m_Viscosity;
        ReynoldsFormat(str, RRe);
        strange = tr("Root Re.sqrt(Cl) =");
        m_pctrlRRe->setText(strange+str);

        double SRe = m_pPlane->tipChord() * QCl/s_WPolar.m_Viscosity;
        ReynoldsFormat(str, SRe);
        strange = tr("Tip Re.sqrt(Cl) =");
        m_pctrlSRe->setText(strange+str);
    }
    else if(s_WPolar.polarType() ==XFLR5::FIXEDAOAPOLAR)
    {
        m_pctrlQInfCl->setText(" ");
        m_pctrlRRe->setText(" ");
        m_pctrlSRe->setText(" ");
    }
}




void WPolarDlg::onAeroData()
{
    AeroDataDlg dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        s_WPolar.density() = dlg.AirDensity();
        s_WPolar.m_Viscosity = dlg.KinematicViscosity();

        if(m_pctrlUnit1->isChecked())
        {
            m_pctrlViscosity->setValue(s_WPolar.m_Viscosity);
        }
        else
        {
            m_pctrlViscosity->setValue(s_WPolar.m_Viscosity* 10.7182881);
        }
        setDensity();
    }
}







