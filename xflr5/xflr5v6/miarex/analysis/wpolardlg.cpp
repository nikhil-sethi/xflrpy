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

#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QMessageBox>



#include "aerodatadlg.h"
#include <xflwidgets/customwts/ctrltabledelegate.h>
#include "wpolardlg.h"
#include <miarex/miarex.h>

#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/customwts/doubleedit.h>

QByteArray WPolarDlg::s_Geometry;

WPolar WPolarDlg::s_WPolar;

WPolarDlg::WPolarDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Analysis Definition"));

    m_pPlane     = nullptr;
    m_bAutoName = true;
    m_WingLoad   = 0.0;
    m_UnitType  = 1;

    setupLayout();
    connectSignals();
}


WPolarDlg::~WPolarDlg()
{
    delete m_pControlDelegate;
}


void WPolarDlg::connectSignals()
{
    connect(m_pchAutoName, SIGNAL(toggled(bool)), this, SLOT(onAutoName()));
    connect(m_prbLLTMethod, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_prbVLM1Method, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_prbVLM2Method, SIGNAL(toggled(bool)), this, SLOT(onMethod()));
    connect(m_prbPanelMethod, SIGNAL(toggled(bool)), this, SLOT(onMethod()));

    connect(m_prbUnit1, SIGNAL(toggled(bool)), this, SLOT(onUnit()));
    connect(m_prbUnit2, SIGNAL(toggled(bool)), this, SLOT(onUnit()));

    connect(m_prbType1, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));
    connect(m_prbType2, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));
    connect(m_prbType4, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));
    connect(m_prbType5, SIGNAL(toggled(bool)), this, SLOT(onPolarType()));

    connect(m_pchTiltGeom, SIGNAL(clicked()), this, SLOT(onTiltedGeom()));
    connect(m_pchViscous, SIGNAL(clicked()), this, SLOT(onViscous()));
    connect(m_pchIgnoreBodyPanels, SIGNAL(clicked()), this, SLOT(onIgnoreBodyPanels()));

    connect(m_pchGroundEffect, SIGNAL(clicked()), this, SLOT(onGroundEffect()));
    connect(m_pchPlaneInertia, SIGNAL(clicked()), this, SLOT(onPlaneInertia()));

    connect(m_pdeXCmRef,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeZCmRef,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeDensity,    SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeViscosity,  SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeAlpha,      SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeBeta,       SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeWeight,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeQInf,       SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeHeight,     SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeRefArea,    SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeRefSpan,    SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pdeRefChord,   SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pleWPolarName, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_pleWPolarName, SIGNAL(textEdited ( const QString &  )), this, SLOT(onPolarName()));

    connect(m_prbArea1, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_prbArea2, SIGNAL(clicked()),this, SLOT(onArea()));
    connect(m_prbArea3, SIGNAL(clicked()),this, SLOT(onArea()));
}


void WPolarDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void WPolarDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void WPolarDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)           onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void WPolarDlg::enableControls()
{
    m_pleWPolarName->setEnabled(!m_pchAutoName->isChecked());

    switch (s_WPolar.polarType())
    {
        case xfl::FIXEDSPEEDPOLAR:
        {
            m_pdeQInf->setEnabled(true);
            m_pdeAlpha->setEnabled(false);
            break;
        }
        case xfl::FIXEDLIFTPOLAR:
        {
            m_pdeQInf->setEnabled(false);
            m_pdeAlpha->setEnabled(false);
            break;
        }
        case xfl::FIXEDAOAPOLAR:
        {
            m_pdeQInf->setEnabled(false);
            m_pdeAlpha->setEnabled(true);
            break;
        }
        case xfl::BETAPOLAR:
        {
            m_pdeQInf->setEnabled(true);
            m_pdeAlpha->setEnabled(true);
            break;
        }
        default:
        {
            m_pdeQInf->setEnabled(true);
            break;
        }
    }

    m_pchViscous->setEnabled(s_WPolar.analysisMethod()==xfl::PANEL4METHOD);
    m_pchTiltGeom->setEnabled(s_WPolar.analysisMethod()==xfl::PANEL4METHOD);
    m_pchIgnoreBodyPanels->setEnabled(m_pPlane && m_pPlane->body());
    m_pdeBeta->setEnabled(s_WPolar.analysisMethod()==xfl::PANEL4METHOD && s_WPolar.polarType()!=xfl::BETAPOLAR);
    m_pchGroundEffect->setEnabled(s_WPolar.analysisMethod()==xfl::PANEL4METHOD);
    m_pdeHeight->setEnabled(m_pchGroundEffect->isChecked() && s_WPolar.analysisMethod()==xfl::PANEL4METHOD);

    m_pdeWeight->setEnabled(!s_WPolar.m_bAutoInertia);
    m_pdeXCmRef->setEnabled(!s_WPolar.m_bAutoInertia);
    m_pdeZCmRef->setEnabled(!s_WPolar.m_bAutoInertia);

    m_prbVLM1Method->setEnabled(!s_WPolar.isBetaPolar() && fabs(s_WPolar.m_BetaSpec)<PRECISION);

    m_pdeRefArea->setEnabled(m_prbArea3->isChecked());
    m_pdeRefChord->setEnabled(m_prbArea3->isChecked());
    m_pdeRefSpan->setEnabled(m_prbArea3->isChecked());
}



void WPolarDlg::initDialog(Plane *pPlane, WPolar *pWPolar)
{
    m_pPlane = pPlane;
    if(!m_pPlane) return;

    blockSignals(true);

    if(pWPolar)
    {
        m_pchAutoName->setChecked(false);
        m_bAutoName = false;
        m_pleWPolarName->setText(pWPolar->polarName());
        s_WPolar.duplicateSpec(pWPolar);
    }
    else
    {
        m_pchAutoName->setChecked(true);
    }


    if(m_pPlane->isWing())
    {
    }
    else
    {
        s_WPolar.setAnalysisMethod(xfl::VLMMETHOD);
        s_WPolar.setThinSurfaces(true);
        m_prbPanelMethod->setVisible(false);
    }

    //initialize the name box
    s_WPolar.setPlaneName(m_pPlane->name());

    // initialize units
    if(m_UnitType==1) m_prbUnit1->setChecked(true);
    else              m_prbUnit2->setChecked(true);


    //initialize polar type
    if     (s_WPolar.polarType()==xfl::FIXEDSPEEDPOLAR) m_prbType1->setChecked(true);
    else if(s_WPolar.polarType()==xfl::FIXEDLIFTPOLAR)  m_prbType2->setChecked(true);
    else if(s_WPolar.polarType()==xfl::FIXEDAOAPOLAR)   m_prbType4->setChecked(true);
    else if(s_WPolar.polarType()==xfl::BETAPOLAR)       m_prbType5->setChecked(true);


    //initialize inertia
    if(s_WPolar.m_bAutoInertia)
    {
        m_pdeWeight->setValue(m_pPlane->totalMass() * Units::kgtoUnit());
        m_pdeXCmRef->setValue(m_pPlane->CoG().x * Units::mtoUnit());
        m_pdeZCmRef->setValue(m_pPlane->CoG().z * Units::mtoUnit());
        s_WPolar.setMass(m_pPlane->totalMass());
        s_WPolar.setCoGx(m_pPlane->CoG().x);
        s_WPolar.setCoGz(m_pPlane->CoG().z);
    }
    else
    {
        m_pdeWeight->setValue(s_WPolar.mass()  * Units::kgtoUnit());
        m_pdeXCmRef->setValue(s_WPolar.CoG().y * Units::mtoUnit());
        m_pdeZCmRef->setValue(s_WPolar.CoG().z * Units::mtoUnit());
    }


    //initialize ground data
    m_pdeHeight->setValue(s_WPolar.m_Height*Units::mtoUnit());
    if(s_WPolar.bGround())
    {
        m_pdeHeight->setEnabled(true);
        m_pchGroundEffect->setChecked(true);
    }
    else
    {
        m_pdeHeight->setEnabled(false);
        m_pchGroundEffect->setChecked(false);
    }


    m_pdeXCmRef->setValue(s_WPolar.CoG().x*Units::mtoUnit());
    m_pdeZCmRef->setValue(s_WPolar.CoG().z*Units::mtoUnit());

    m_pdeQInf->setValue(s_WPolar.m_QInfSpec*Units::mstoUnit());
    m_pdeWeight->setValue(s_WPolar.mass()*Units::kgtoUnit());
    m_pdeBeta->setValue(s_WPolar.m_BetaSpec);
    m_pdeAlpha->setValue(s_WPolar.m_AlphaSpec);


    m_pchViscous->setChecked(s_WPolar.bViscous());
    m_pchTiltGeom->setChecked(s_WPolar.bTilted());

    // force ignore body panels by default
    s_WPolar.setIgnoreBodyPanels(true);
    m_pchIgnoreBodyPanels->setChecked(m_pPlane->body() || s_WPolar.bIgnoreBodyPanels());
    //    if(!m_pPlane) s_WPolar.bIgnoreBodyPanels()=false;


    if(s_WPolar.analysisMethod()==xfl::LLTMETHOD)
    {
        m_prbLLTMethod->setChecked(true);
        m_pchViscous->setChecked(true);
        m_pchViscous->setEnabled(false);
    }
    else if(s_WPolar.analysisMethod()==xfl::VLMMETHOD)
    {
        m_prbVLM1Method->setChecked( s_WPolar.bVLM1());
        m_prbVLM2Method->setChecked(!s_WPolar.bVLM1());
        m_pchViscous->setEnabled(true);
    }
    else if(s_WPolar.analysisMethod()==xfl::PANEL4METHOD)
    {
        if(s_WPolar.bThinSurfaces())
        {
            m_prbVLM1Method->setChecked(s_WPolar.bVLM1());
            m_prbVLM2Method->setChecked(!s_WPolar.bVLM1());
        }
        else
        {
            m_prbPanelMethod->setChecked(true);
        }
        m_pchViscous->setEnabled(true);
    }


    m_prbArea1->setChecked(s_WPolar.referenceDim()==xfl::PLANFORMREFDIM);
    m_prbArea2->setChecked(s_WPolar.referenceDim()==xfl::PROJECTEDREFDIM);
    m_prbArea3->setChecked(s_WPolar.referenceDim()==xfl::MANUALREFDIM);

    if(m_prbArea1->isChecked())
    {
        s_WPolar.setReferenceArea(m_pPlane->planformArea());
        s_WPolar.setReferenceSpanLength(m_pPlane->planformSpan());
        m_pdeRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_prbArea2->isChecked())
    {
        s_WPolar.setReferenceArea(m_pPlane->projectedArea());
        s_WPolar.setReferenceSpanLength(m_pPlane->projectedSpan());
        m_pdeRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
    }
    else if(m_prbArea3->isChecked())
    {
        m_pdeRefArea->setValue(s_WPolar.referenceArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
    }

    s_WPolar.setReferenceChordLength(m_pPlane->mac());
    m_pdeRefChord->setValue(s_WPolar.referenceChordLength()*Units::mtoUnit());

    s_WPolar.setWakeRollUp(false);

    m_pchPlaneInertia->setChecked(s_WPolar.m_bAutoInertia);


    setDensity();
    setWingLoad();
    setReynolds();
    setWPolarName();
    fillExtraDragList();
    enableControls();

    m_pdeQInf->setSelection(0,-1);
    m_pdeQInf->setFocus();

    blockSignals(false);

}



void WPolarDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
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
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void WPolarDlg::onArea()
{
    if(m_prbArea1->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PLANFORMREFDIM);
        m_pdeRefArea->setValue(m_pPlane->planformArea()*Units::m2toUnit());
        m_pdeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
        m_pdeRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
    }
    else if(m_prbArea2->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PROJECTEDREFDIM);
        m_pdeRefArea->setValue(m_pPlane->projectedArea()*Units::m2toUnit());
        m_pdeRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
        m_pdeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
    }
    else if(m_prbArea3->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::MANUALREFDIM);
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
    m_bAutoName = m_pchAutoName->isChecked();
    if(m_bAutoName) setWPolarName();
    enableControls();
}


void WPolarDlg::onTiltedGeom()
{
    s_WPolar.setTilted(m_pchTiltGeom->isChecked());
    setWPolarName();
    enableControls();
}


void WPolarDlg::onPlaneInertia()
{
    if(m_pchPlaneInertia->isChecked())
    {
        if(m_pPlane)
        {
            m_pdeWeight->setValue(m_pPlane->totalMass() * Units::kgtoUnit());
            m_pdeXCmRef->setValue(m_pPlane->CoG().x * Units::mtoUnit());
            m_pdeZCmRef->setValue(m_pPlane->CoG().z * Units::mtoUnit());
            s_WPolar.setMass(m_pPlane->totalMass());
            s_WPolar.setCoG(m_pPlane->CoG());
        }
    }
    else
    {
        s_WPolar.setMass(m_pdeWeight->value() / Units::kgtoUnit());
        s_WPolar.setCoGx(m_pdeXCmRef->value() / Units::mtoUnit());
        s_WPolar.setCoGz(m_pdeZCmRef->value() / Units::mtoUnit());
    }
    s_WPolar.m_bAutoInertia = m_pchPlaneInertia->isChecked();
    setWPolarName();
    enableControls();
}


void WPolarDlg::onViscous()
{
    s_WPolar.setViscous(m_pchViscous->isChecked());
    setWPolarName();
    enableControls();
}


void WPolarDlg::onIgnoreBodyPanels()
{
    s_WPolar.setIgnoreBodyPanels(m_pchIgnoreBodyPanels->isChecked());
    setWPolarName();
    enableControls();
}


void WPolarDlg::onGroundEffect()
{
    s_WPolar.setGroundEffect(m_pchGroundEffect->isChecked());
    m_pdeHeight->setEnabled(s_WPolar.bGround());
    setWPolarName();
}


void WPolarDlg::onMethod()
{
    if (m_prbLLTMethod->isChecked())
    {
        s_WPolar.setViscous(true);
        s_WPolar.setThinSurfaces(true);
        s_WPolar.setWakeRollUp(false);
        s_WPolar.setTilted(false);
        s_WPolar.setAnalysisMethod(xfl::LLTMETHOD);
        m_pchTiltGeom->setChecked(false);
    }
    else if (m_prbVLM1Method->isChecked() || m_prbVLM2Method->isChecked())
    {
        s_WPolar.setVLM1(m_prbVLM1Method->isChecked());
        s_WPolar.setThinSurfaces(true);
        s_WPolar.setAnalysisMethod(xfl::PANEL4METHOD);
    }
    else if (m_prbPanelMethod->isChecked())
    {
        s_WPolar.setThinSurfaces(false);
        s_WPolar.setAnalysisMethod(xfl::PANEL4METHOD);
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
    if(!m_pleWPolarName->text().length())
    {
        QMessageBox::warning(this, tr("Warning"),tr("Must enter a name for the polar"));
        m_pleWPolarName->setFocus();
        return;
    }
    s_WPolar.setPolarName(m_pleWPolarName->text());

    if(qAbs(s_WPolar.mass())<PRECISION && s_WPolar.polarType()==xfl::FIXEDLIFTPOLAR)
    {
        QMessageBox::warning(this, tr("Warning"),tr("Mass must be non-zero for type 2 polars"));
        m_pdeWeight->setFocus();
        return;
    }
    if(!m_pPlane->isWing() && s_WPolar.analysisMethod()==xfl::PANEL4METHOD) s_WPolar.setThinSurfaces(true);

    readExtraDragData();

    accept();
}


void WPolarDlg::onUnit()
{
    if(m_prbUnit1->isChecked())
    {
        m_UnitType   = 1;
        m_pdeViscosity->setValue(s_WPolar.m_Viscosity);
        m_plabDensityUnit->setText("kg/m3");
        m_plabViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");
    }
    else
    {
        m_UnitType   = 2;
        m_pdeViscosity->setValue(s_WPolar.m_Viscosity* 10.7182881);
        m_plabDensityUnit->setText("slugs/ft3");
        m_plabViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");
    }
    setDensity();
}


void WPolarDlg::onPolarName()
{
    m_bAutoName = false;
    m_pchAutoName->setChecked(false);
}



void WPolarDlg::onPolarType()
{
    if (m_prbType1->isChecked())
    {
        s_WPolar.setPolarType(xfl::FIXEDSPEEDPOLAR);
    }
    else if(m_prbType2->isChecked())
    {
        s_WPolar.setPolarType(xfl::FIXEDLIFTPOLAR);
    }
    else if(m_prbType4->isChecked())
    {
        s_WPolar.setPolarType(xfl::FIXEDAOAPOLAR);
    }
    else if(m_prbType5->isChecked())
    {
        s_WPolar.setPolarType(xfl::BETAPOLAR);
        s_WPolar.setVLM1(false);
        if(m_prbVLM1Method->isChecked())
        {
            m_prbVLM1Method->blockSignals(true);
            m_prbVLM2Method->blockSignals(true);
            m_prbVLM2Method->setChecked(true);
            m_prbVLM1Method->blockSignals(false);
            m_prbVLM2Method->blockSignals(false);
        }
    }
    enableControls();
    setReynolds();
    setWPolarName();
}


void WPolarDlg::readValues()
{
    s_WPolar.m_AlphaSpec     = m_pdeAlpha->value();
    s_WPolar.m_BetaSpec      = m_pdeBeta->value();
    if(fabs(s_WPolar.m_BetaSpec)>PRECISION)
    {
        s_WPolar.setVLM1(false);
        if(m_prbVLM1Method->isChecked())
        {
            m_prbVLM1Method->blockSignals(true);
            m_prbVLM2Method->blockSignals(true);
            m_prbVLM2Method->setChecked(true);
            m_prbVLM1Method->blockSignals(false);
            m_prbVLM2Method->blockSignals(false);
        }
    }

    s_WPolar.setMass(m_pdeWeight->value() / Units::kgtoUnit());
    s_WPolar.setCoGx(m_pdeXCmRef->value() / Units::mtoUnit());
    s_WPolar.setCoGz(m_pdeZCmRef->value() / Units::mtoUnit());
    s_WPolar.m_QInfSpec      = m_pdeQInf->value() / Units::mstoUnit();
    s_WPolar.m_Height        = m_pdeHeight->value() / Units::mtoUnit();

    if(m_prbUnit1->isChecked())
    {
        s_WPolar.setViscosity(m_pdeViscosity->value());
        s_WPolar.setDensity(m_pdeDensity->value());
    }
    else
    {
        s_WPolar.setDensity(m_pdeDensity->value() / 0.00194122);
        s_WPolar.setViscosity(m_pdeViscosity->value() / 10.7182881);
    }

    if(m_prbArea1->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PLANFORMREFDIM);
        s_WPolar.setReferenceArea(m_pPlane->planformArea());
        s_WPolar.setReferenceSpanLength(m_pPlane->planformSpan());
    }
    else if(m_prbArea2->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PROJECTEDREFDIM);
        s_WPolar.setReferenceArea(m_pPlane->projectedArea());
        s_WPolar.setReferenceSpanLength(m_pPlane->projectedSpan());
    }
    else if(m_prbArea3->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::MANUALREFDIM);
        s_WPolar.setReferenceArea(m_pdeRefArea->value() /Units::m2toUnit());
        s_WPolar.setReferenceSpanLength(m_pdeRefSpan->value() /Units::mtoUnit());
    }

    s_WPolar.setReferenceChordLength(m_pdeRefChord->value() /Units::mtoUnit());

    setDensity();

    setWingLoad();
}



void WPolarDlg::setDensity()
{
    int exp, precision;
    if(m_prbUnit1->isChecked())
    {
        exp = int(log(s_WPolar.density()));
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pdeDensity->setDigits(precision);
        m_pdeDensity->setValue(s_WPolar.density());
    }
    else
    {
        exp = int(log(s_WPolar.density()* 0.00194122));
        if(exp>1) precision = 1;
        else if(exp<-4) precision = 4;
        else precision = 3-exp;
        m_pdeDensity->setDigits(precision);
        m_pdeDensity->setValue(s_WPolar.density()* 0.00194122);
    }
}


void WPolarDlg::setupLayout()
{
    QString strSpeedUnit, strLengthUnit, strWeightUnit;

    Units::getSpeedUnitLabel(strSpeedUnit);
    Units::getLengthUnitLabel(strLengthUnit);
    Units::getMassUnitLabel(strWeightUnit);


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
                m_prbType1 = new QRadioButton(tr("Type 1 (Fixed Speed)"));
                m_prbType2 = new QRadioButton(tr("Type 2 (Fixed Lift)"));
                m_prbType4 = new QRadioButton(tr("Type 4 (Fixed aoa)"));
                m_prbType5 = new QRadioButton(tr("Type 5 (Beta range)"));
                pTypeLayout->addWidget(m_prbType1);
                pTypeLayout->addWidget(m_prbType2);
                pTypeLayout->addWidget(m_prbType4);
                pTypeLayout->addWidget(m_prbType5);
                pTypeLayout->addStretch();
            }

            QGridLayout *pTypeDataLayout = new QGridLayout;
            {
                QLabel *lab1 = new QLabel(tr("Free Stream Speed ="));
                QLabel *lab5 = new QLabel(QString(QChar(0x03B1)) + "=");
                QLabel *lab6 = new QLabel(QString(QChar(0x03B2)) + "=");
                lab5->setFont(symbolFont);
                lab6->setFont(symbolFont);
                lab1->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                lab5->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                lab6->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                m_pdeQInf    = new DoubleEdit(10.05);
                m_pdeQInf->setMin(0.0);
                m_pdeAlpha   = new DoubleEdit(1.00,2);
                m_pdeBeta    = new DoubleEdit(0.00,2);
                QLabel *labSpeedUnit   = new QLabel(strSpeedUnit);

                QLabel *lab7 = new QLabel(QChar(0260));
                QLabel *lab8 = new QLabel(QChar(0260));
                pTypeDataLayout->addWidget(lab1,1,1);
                pTypeDataLayout->addWidget(lab5,2,1);
                pTypeDataLayout->addWidget(lab6,3,1);
                pTypeDataLayout->addWidget(m_pdeQInf,1,2);
                pTypeDataLayout->addWidget(m_pdeAlpha,2,2);
                pTypeDataLayout->addWidget(m_pdeBeta,3,2);
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
            m_plabWingLoad  = new QLabel(tr("Wing Loading = 0.033 kg/dm2"));
            m_plabSRe       = new QLabel(tr("SRe"));
            m_plabRRe       = new QLabel(tr("RRe"));
            m_plabQInfCl    = new QLabel(tr("QInfCl"));

            m_plabWingLoad->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_plabSRe->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_plabRRe->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_plabQInfCl->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            pFlightLayout->addWidget(m_plabWingLoad);
            pFlightLayout->addWidget(m_plabSRe);
            pFlightLayout->addWidget(m_plabRRe);
            pFlightLayout->addWidget(m_plabQInfCl);
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
                m_prbLLTMethod   = new QRadioButton(tr("LLT (Wing only)"));
                m_prbVLM1Method = new QRadioButton(tr("Horseshoe vortex")+ " (VLM1) "+tr("(No sideslip)"));
                m_prbVLM2Method = new QRadioButton(tr("Ring vortex")+" (VLM2)");
                m_prbPanelMethod = new QRadioButton(tr("3D Panels"));

                pMethodLayout->addWidget(m_prbLLTMethod);
                pMethodLayout->addWidget(m_prbVLM1Method);
                pMethodLayout->addWidget(m_prbVLM2Method);
                pMethodLayout->addWidget(m_prbPanelMethod);
                pMethodLayout->addStretch();
            }
            pAnalysisMethods->setLayout(pMethodLayout);

        }

        QGroupBox *pOptionsGroupBox = new QGroupBox(tr("Options"));
        {
            QVBoxLayout *pOptionsLayout = new QVBoxLayout;
            {
                m_pchViscous = new QCheckBox(tr("Viscous"));
                m_pchTiltGeom = new QCheckBox(tr("Tilted geometry - NOT RECOMMENDED"));
                pOptionsLayout->addWidget(m_pchViscous);
                pOptionsLayout->addWidget(m_pchTiltGeom);
                m_pchIgnoreBodyPanels = new QCheckBox(tr("Ignore Body Panels - RECOMMENDED"));
                pOptionsLayout->addWidget(m_pchIgnoreBodyPanels);
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
                m_pchPlaneInertia = new QCheckBox(tr("Use plane inertia"));
                m_pchPlaneInertia->setToolTip("Activate this checbox for the polar to use dynamically the plane's inertia properties for each analysis");

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
                    m_pdeWeight  = new DoubleEdit(0,3);
                    m_pdeWeight->setMin(0.0);
                    m_pdeXCmRef  = new DoubleEdit(100.00,3);
                    m_pdeZCmRef  = new DoubleEdit(100.00,3);
                    pInertiaDataLayout->addWidget(m_pdeWeight,1,2);
                    pInertiaDataLayout->addWidget(m_pdeXCmRef,2,2);
                    pInertiaDataLayout->addWidget(m_pdeZCmRef,3,2);
                    QLabel *labWeightUnit  = new QLabel(strWeightUnit);
                    QLabel *labLengthUnit1 = new QLabel(strLengthUnit);
                    QLabel *labLengthUnit3 = new QLabel(strLengthUnit);

                    pInertiaDataLayout->addWidget(labWeightUnit ,1,3);
                    pInertiaDataLayout->addWidget(labLengthUnit1 ,2,3);
                    pInertiaDataLayout->addWidget(labLengthUnit3 ,3,3);
                    pInertiaDataLayout->setColumnStretch(1,1);
                }
                pInertiaLayout->addWidget(m_pchPlaneInertia);
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
                m_prbArea1 = new QRadioButton(tr("Wing Planform"));
                m_prbArea2 = new QRadioButton(tr("Wing Planform projected on xy plane"));
                m_prbArea3 = new QRadioButton(tr("User defined"));

                QGridLayout *pRefAreaLayout = new QGridLayout;
                {
                    QLabel *labRefArea  = new QLabel(tr("Ref. area="));
                    QLabel *labRefSpan  = new QLabel(tr("Ref. span length="));
                    QLabel *labRefChord = new QLabel(tr("Ref. chord length="));
                    m_pdeRefArea  = new DoubleEdit(0.0, 3);
                    m_pdeRefChord = new DoubleEdit(0.0, 3);
                    m_pdeRefSpan  = new DoubleEdit(0.0, 3);
                    QLabel *labAreaUnit = new QLabel(Units::areaUnitLabel());
                    QLabel *labLengthUnit4 = new QLabel(Units::lengthUnitLabel());
                    QLabel *labLengthUnit5 = new QLabel(Units::lengthUnitLabel());

                    labRefArea->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    labRefSpan->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    labRefChord->setAlignment(Qt::AlignRight | Qt::AlignCenter);

                    labAreaUnit->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
                    labLengthUnit4->setAlignment(Qt::AlignLeft | Qt::AlignCenter);

                    pRefAreaLayout->addWidget(labRefArea,1,1);
                    pRefAreaLayout->addWidget(m_pdeRefArea,1,2);
                    pRefAreaLayout->addWidget(labAreaUnit,1,3);
                    pRefAreaLayout->addWidget(labRefSpan,2,1);
                    pRefAreaLayout->addWidget(m_pdeRefSpan,2,2);
                    pRefAreaLayout->addWidget(labLengthUnit4,2,3);
                    pRefAreaLayout->addWidget(labRefChord,3,1);
                    pRefAreaLayout->addWidget(m_pdeRefChord,3,2);
                    pRefAreaLayout->addWidget(labLengthUnit5,3,3);

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
                    m_pdeDensity = new DoubleEdit(1.225,6);
                    m_plabDensityUnit = new QLabel("kg/m3");
                    m_plabNu = new QLabel("n =");
                    m_plabRho->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_plabNu->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    m_pdeViscosity = new DoubleEdit(1.500e-5,3);
                    m_plabViscosityUnit = new QLabel("m2/s");
                    m_plabRho->setFont(symbolFont);
                    m_plabNu->setFont(symbolFont);
                    m_pdeDensity->setDigits(6);
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

        QGroupBox *pGroundGroupBox =  new QGroupBox(tr("Ground Effect"));
        {
            QVBoxLayout * pGroundLayout = new QVBoxLayout;
            {
                m_pchGroundEffect = new QCheckBox(tr("Ground Effect"));
                QHBoxLayout *pGroundHeightLayout = new QHBoxLayout;
                {
                    QLabel *lab10 = new QLabel(tr("Height ="));
                    m_pdeHeight = new DoubleEdit(0.00,2);
                    m_pdeHeight->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                    QLabel *labLengthUnit2 = new QLabel(strLengthUnit);
                    pGroundHeightLayout->addStretch();
                    pGroundHeightLayout->addWidget(lab10);
                    pGroundHeightLayout->addWidget(m_pdeHeight);
                    pGroundHeightLayout->addWidget(labLengthUnit2);
                }
                pGroundLayout->addWidget(m_pchGroundEffect);
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
        m_ptvExtraDragControl = new QTableView(this);
        m_ptvExtraDragControl->setFont(DisplayOptions::tableFont());

        m_ptvExtraDragControl->setWindowTitle(tr("Extra drag"));
        m_ptvExtraDragControl->setMinimumWidth(400);
        m_ptvExtraDragControl->setMinimumHeight(150);
        m_ptvExtraDragControl->setSelectionMode(QAbstractItemView::SingleSelection);
        m_ptvExtraDragControl->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ptvExtraDragControl->horizontalHeader()->setStretchLastSection(true);

        m_pExtraDragControlModel = new QStandardItemModel(this);
        m_pExtraDragControlModel->setColumnCount(3);
        m_pExtraDragControlModel->setHeaderData(0, Qt::Horizontal, tr("Extra drag"));
        m_pExtraDragControlModel->setHeaderData(1, Qt::Horizontal, tr("Extra area")+" ("+Units::areaUnitLabel()+")");
        m_pExtraDragControlModel->setHeaderData(2, Qt::Horizontal, tr("Extra drag coef."));


        m_ptvExtraDragControl->setModel(m_pExtraDragControlModel);

        m_pControlDelegate = new CtrlTableDelegate(this);
        m_ptvExtraDragControl->setItemDelegate(m_pControlDelegate);

        connect(m_pControlDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onEditingFinished()));

        m_pControlDelegate->setPrecision({0,3,5});

        QLabel* pExtraLabel = new QLabel(QString::fromUtf8("D = 1/2 rho V² ( S (CD_induced+CD_Visc) + S_Extra1.CD_Extra1 + ... + S_ExtraN.Cd_ExtraN)"));

        pExtraDragPageLayout->addWidget(m_ptvExtraDragControl);
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
            m_pchAutoName = new QCheckBox(tr("Auto Analysis Name"));
            m_pleWPolarName = new QLineEdit(tr("Polar Name"));
            pPolarNameLayout->addWidget(m_pchAutoName);
            pPolarNameLayout->addWidget(m_pleWPolarName);
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

    m_ptvExtraDragControl->resizeColumnsToContents();
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
    double wc = double(m_ptvExtraDragControl->width())*.97;
    int wCols  = int(wc/3);
    m_ptvExtraDragControl->setColumnWidth(0, wCols);
    m_ptvExtraDragControl->setColumnWidth(1, wCols);
    m_ptvExtraDragControl->setColumnWidth(2, wCols);

}


void WPolarDlg::setWPolarName()
{
    if(!m_bAutoName) return;

    xfl::setAutoWPolarName(&s_WPolar, m_pPlane);
    m_pleWPolarName->setText(s_WPolar.polarName());
}



void WPolarDlg::setWingLoad()
{
    QString str,str1, str2;

    if(s_WPolar.referenceArea()>0)
    {
        m_WingLoad = s_WPolar.mass()/s_WPolar.referenceArea();//kg/dm2

        str = QString("%1 ").arg(m_WingLoad * Units::kgtoUnit() / Units::m2toUnit(),7,'f',3);

        Units::getMassUnitLabel(str1);
        Units::getAreaUnitLabel(str2);
    }
    m_plabWingLoad->setText(tr("Wing Loading = ")+str+str1+"/"+str2);
}



void WPolarDlg::setReynolds()
{
    QString strange, str, strUnit;
    Units::getSpeedUnitLabel(strUnit);

    if(s_WPolar.polarType() == xfl::FIXEDSPEEDPOLAR)
    {
        double RRe = m_pPlane->rootChord() * s_WPolar.m_QInfSpec/s_WPolar.m_Viscosity;
        xfl::ReynoldsFormat(str, RRe);
        strange = tr("Root Re =");
        m_plabRRe->setText(strange+str);

        double SRe = m_pPlane->tipChord() * s_WPolar.m_QInfSpec/s_WPolar.m_Viscosity;
        xfl::ReynoldsFormat(str, SRe);
        strange = tr("Tip Re =");
        m_plabSRe->setText(strange+str);

        m_plabQInfCl->setText(" ");
    }
    else if(s_WPolar.polarType() == xfl::FIXEDLIFTPOLAR)
    {
        double QCl =  sqrt(2.* 9.81 /s_WPolar.density()* s_WPolar.mass() /s_WPolar.referenceArea());
        str = QString("%1 ").arg(QCl,5,'f',2);
        str += strUnit;
        strange = tr("Vinf.sqrt(Cl) =");
        m_plabQInfCl->setText(strange+str);

        double RRe = m_pPlane->rootChord() * QCl/s_WPolar.m_Viscosity;
        xfl::ReynoldsFormat(str, RRe);
        strange = tr("Root Re.sqrt(Cl) =");
        m_plabRRe->setText(strange+str);

        double SRe = m_pPlane->tipChord() * QCl/s_WPolar.m_Viscosity;
        xfl::ReynoldsFormat(str, SRe);
        strange = tr("Tip Re.sqrt(Cl) =");
        m_plabSRe->setText(strange+str);
    }
    else if(s_WPolar.polarType() ==xfl::FIXEDAOAPOLAR)
    {
        m_plabQInfCl->setText(" ");
        m_plabRRe->setText(" ");
        m_plabSRe->setText(" ");
    }
}


void WPolarDlg::onAeroData()
{
    AeroDataDlg dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        s_WPolar.setDensity(dlg.AirDensity());
        s_WPolar.setViscosity(dlg.KinematicViscosity());

        if(m_prbUnit1->isChecked())
        {
            m_pdeViscosity->setValue(s_WPolar.m_Viscosity);
        }
        else
        {
            m_pdeViscosity->setValue(s_WPolar.m_Viscosity* 10.7182881);
        }
        setDensity();
    }
}







