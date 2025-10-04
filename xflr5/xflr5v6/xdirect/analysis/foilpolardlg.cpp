/****************************************************************************

    FoilPolarDlg Class
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

#include <QGroupBox>
#include <QVBoxLayout>
#include "foilpolardlg.h"
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xdirect/xdirect.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflobjects/objects2d/polar.h>

QByteArray FoilPolarDlg::s_WindowGeometry;

int FoilPolarDlg::s_UnitType = 1;
double FoilPolarDlg::s_Viscosity = 1.5e-5;
double FoilPolarDlg::s_Density   = 1.225;
double FoilPolarDlg::s_Chord = 1.0;
double FoilPolarDlg::s_Span = 1.0;
double FoilPolarDlg::s_Mass = 1.0;

Polar FoilPolarDlg::s_RefPolar;

FoilPolarDlg::FoilPolarDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Foil Polar Definition"));

    m_bAutoName = true;

    setupLayout();
    connectSignals();
}


void FoilPolarDlg::setupLayout()
{
    QFont SymbolFont("Symbol");

    QGroupBox *pNameGroupBox = new QGroupBox(tr("Analysis Name"));
    {
        QVBoxLayout *pAnalysisLayout = new QVBoxLayout;
        {
            QHBoxLayout *pAutoNameLayout = new QHBoxLayout;
            {
                m_prbAuto1 = new QRadioButton(tr("Automatic"));
                m_prbAuto2 = new QRadioButton(tr("User Defined"));
                m_pleAnalysisName = new QLineEdit(tr("Analysis Name"));
                pAutoNameLayout->addStretch(1);
                pAutoNameLayout->addWidget(m_prbAuto1);
                pAutoNameLayout->addStretch(1);
                pAutoNameLayout->addWidget(m_prbAuto2);
                pAutoNameLayout->addStretch(1);
            }
            pAnalysisLayout->addLayout(pAutoNameLayout);
            pAnalysisLayout->addWidget(m_pleAnalysisName);
        }
        pNameGroupBox->setLayout(pAnalysisLayout);
    }

    QGroupBox *pTypeGroup = new QGroupBox(tr("Analysis Type"));
    {
        QHBoxLayout *pAnalysisTypeLayout = new QHBoxLayout;
        {
          m_rbtype1 = new QRadioButton(tr("Type 1"));
            m_rbtype2 = new QRadioButton(tr("Type 2"));
            m_rbtype3 = new QRadioButton(tr("Type 3"));
            m_rbtype4 = new QRadioButton(tr("Type 4"));
            pAnalysisTypeLayout->addWidget(m_rbtype1);
            pAnalysisTypeLayout->addWidget(m_rbtype2);
            pAnalysisTypeLayout->addWidget(m_rbtype3);
            pAnalysisTypeLayout->addWidget(m_rbtype4);
            pTypeGroup->setLayout(pAnalysisTypeLayout);
        }
    }

    QGroupBox *pAeroGroupBox = new QGroupBox(tr("Reynolds and Mach Numbers"));
    {
       QVBoxLayout *pReMachLayout = new QVBoxLayout;
        {
            QHBoxLayout *pType2DataLayout = new QHBoxLayout;
            {
                //type 2 input data
                QGroupBox *pPlaneDataGroupBox = new QGroupBox(tr("Plane Data"));
                {
                    QGridLayout *PlaneDataLayout = new QGridLayout;
                    {
                        m_pdeChord = new DoubleEdit(0,3);
                        m_pdeMass = new DoubleEdit(0,3);
                        m_pdeSpan = new DoubleEdit(0,3);
                        QLabel *ChordLab = new QLabel(tr("Chord"));
                        QLabel *MassLab = new QLabel(tr("Mass"));
                        QLabel *SpanLab = new QLabel(tr("Span"));
                        m_plabLengthUnit1 = new QLabel("m");
                        m_plabLengthUnit2 = new QLabel("m");
                        m_plabMassUnit = new QLabel("kg");
                        PlaneDataLayout->addWidget(ChordLab,1,1);
                        PlaneDataLayout->addWidget(m_pdeChord,1,2);
                        PlaneDataLayout->addWidget(m_plabLengthUnit1,1,3);
                        PlaneDataLayout->addWidget(SpanLab,2,1);
                        PlaneDataLayout->addWidget(m_pdeSpan,2,2);
                        PlaneDataLayout->addWidget(m_plabLengthUnit2,2,3);
                        PlaneDataLayout->addWidget(MassLab,3,1);
                        PlaneDataLayout->addWidget(m_pdeMass,3,2);
                        PlaneDataLayout->addWidget(m_plabMassUnit,3,3);
                    }
                    pPlaneDataGroupBox->setLayout(PlaneDataLayout);
                }
                QGroupBox *pAeroDataGroupBox = new QGroupBox(tr("Fluid properties"));
                {
                    QGridLayout *pAeroDataLayout = new QGridLayout;
                    {
                        QLabel *lab9 = new QLabel(tr("Unit"));
                        m_prbFluidUnit1 = new QRadioButton(tr("International"));
                        m_prbFluidUnit2 = new QRadioButton(tr("Imperial"));
                        m_plabRho = new QLabel("r =");
                        m_pdeDensity = new DoubleEdit(1.225,3);
                        m_plabDensityUnit = new QLabel("kg/m3");
                        m_plabNu = new QLabel("n =");
                        m_plabRho->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                        m_plabNu->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                        m_pdeViscosity = new DoubleEdit(1.500e-5,3);
                        m_plabViscosityUnit = new QLabel(QString::fromUtf8("m²/s"));
                        m_plabRho->setFont(SymbolFont);
                        m_plabNu->setFont(SymbolFont);
                        m_pdeDensity->setDigits(5);
                        m_pdeViscosity->setDigits(3);
                        m_pdeDensity->setMin(0.0);
                        m_pdeViscosity->setMin(0.0);
                        pAeroDataLayout->addWidget(lab9,1,1);
                        pAeroDataLayout->addWidget(m_prbFluidUnit1,   1,2);
                        pAeroDataLayout->addWidget(m_prbFluidUnit2,   1,3);
                        pAeroDataLayout->addWidget(m_plabRho,          2,1);
                        pAeroDataLayout->addWidget(m_pdeDensity,      2,2);
                        pAeroDataLayout->addWidget(m_plabDensityUnit,  2,3);
                        pAeroDataLayout->addWidget(m_plabNu,           3,1);
                        pAeroDataLayout->addWidget(m_pdeViscosity,    3,2);
                        pAeroDataLayout->addWidget(m_plabViscosityUnit,3,3);
                    }
                    pAeroDataGroupBox->setLayout(pAeroDataLayout);
                }

                pType2DataLayout->addWidget(pPlaneDataGroupBox);
                pType2DataLayout->addWidget(pAeroDataGroupBox);
            }

            QHBoxLayout *pReMachValuesLayout= new QHBoxLayout;
            {
                m_plabRe  = new QLabel(tr("  Re ="));
                m_plabRe->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_plabReUnit  = new QLabel(tr(" "));
                m_plabReUnit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_plabMach = new QLabel(tr("Mach ="));
                m_plabMach->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pdeReynolds = new DoubleEdit();
                m_pdeReynolds->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pdeReynolds->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                QFontMetrics fm(m_pdeReynolds->font());
                m_pdeReynolds->setMinimumWidth(fm.averageCharWidth()*15);
                m_pdeMach = new DoubleEdit(0.0, 3);
                m_pdeMach->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pReMachValuesLayout->addStretch(1);
                pReMachValuesLayout->addWidget(m_plabRe);
                pReMachValuesLayout->addWidget(m_pdeReynolds);
                pReMachValuesLayout->addWidget(m_plabReUnit);
                pReMachValuesLayout->addStretch(1);
                pReMachValuesLayout->addWidget(m_plabMach);
                pReMachValuesLayout->addWidget(m_pdeMach);
                pReMachValuesLayout->addStretch(1);
            }

            pReMachLayout->addLayout(pType2DataLayout);
            pReMachLayout->addLayout(pReMachValuesLayout);
        }
        pAeroGroupBox->setLayout(pReMachLayout);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QGroupBox *pTransGroup = new QGroupBox(tr("Transition settings"));
    {
        QGridLayout *pTransitionsLayout = new QGridLayout;
        {
            QLabel *pFreeTransLabel   = new QLabel(tr("Free transitions (e^n) method"));
            QLabel *pForceTransLabel  = new QLabel(tr("Forced transition:"));
            QLabel*pNCritLabel      = new QLabel(tr("NCrit="));
            QLabel *pTopTripLabel     = new QLabel(tr("TripLocation (top)"));
            QLabel *pBotTripLabel     = new QLabel(tr("TripLocation (bot)"));
            m_pdeNCrit    = new DoubleEdit();
            m_pdeTopTrans = new DoubleEdit();
            m_pdeBotTrans = new DoubleEdit();

            m_pdeNCrit->setAlignment(   Qt::AlignRight);
            m_pdeTopTrans->setAlignment(Qt::AlignRight);
            m_pdeBotTrans->setAlignment(Qt::AlignRight);
            pTransitionsLayout->addWidget(pFreeTransLabel,   1,1, 1,1, Qt::AlignLeft| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pForceTransLabel,  2,1, 1,1, Qt::AlignLeft| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pNCritLabel,       1,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pTopTripLabel,     2,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pBotTripLabel,     3,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(m_pdeNCrit,     1,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(m_pdeTopTrans,  2,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(m_pdeBotTrans,  3,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransGroup->setLayout(pTransitionsLayout);
        }
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch();
        pMainLayout->addWidget(pNameGroupBox);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pTypeGroup);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pAeroGroupBox);
        pMainLayout->addWidget(pTransGroup);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch();
    }

    setLayout(pMainLayout);

    m_pdeTopTrans->setDigits(2);
    m_pdeTopTrans->setMin(0.0);
    m_pdeTopTrans->setMax(1.0);

    m_pdeBotTrans->setDigits(2);
    m_pdeBotTrans->setMin(0.0);
    m_pdeBotTrans->setMax(1.0);


    m_pdeNCrit->setDigits(3);
    m_pdeNCrit->setMin(0.0);
    m_pdeNCrit->setMax(1000000.0);


    m_pdeMach->setMin(0.0);
    m_pdeMach->setMax(1000.0);
}


void FoilPolarDlg::connectSignals()
{
    connect(m_prbAuto1, SIGNAL(clicked()), this, SLOT(onAutoName()));
    connect(m_prbAuto2, SIGNAL(clicked()), this, SLOT(onAutoName()));

    connect(m_rbtype1, SIGNAL(clicked()), this, SLOT(onPolarType()));
    connect(m_rbtype2, SIGNAL(clicked()), this, SLOT(onPolarType()));
    connect(m_rbtype3, SIGNAL(clicked()), this, SLOT(onPolarType()));
    connect(m_rbtype4, SIGNAL(clicked()), this, SLOT(onPolarType()));

    connect(m_pdeReynolds, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pdeMach,     SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pdeNCrit,    SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pdeTopTrans, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pdeBotTrans, SIGNAL(editingFinished()), this, SLOT(editingFinished()));

    connect(m_pleAnalysisName, SIGNAL(textEdited (const QString &)), this, SLOT(onNameChanged()));

    connect(m_prbFluidUnit1, SIGNAL(clicked(bool)), this, SLOT(onFluiUnit()));
    connect(m_prbFluidUnit2, SIGNAL(clicked(bool)), this, SLOT(onFluiUnit()));

    connect(m_pdeChord,     SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pdeSpan,      SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pdeMass,      SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pdeViscosity, SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pdeDensity,   SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
}


void FoilPolarDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)
    {
        onOK();
    }
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)     reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)    reject();
}


void FoilPolarDlg::editingFinished()
{
    setPlrName();
}


void FoilPolarDlg::initDialog()
{
    if(XDirect::curFoil()) s_RefPolar.m_FoilName = XDirect::curFoil()->name();
    else                   s_RefPolar.m_FoilName = "";

    QString str = tr("Analysis parameters for ");
    setWindowTitle(str+ s_RefPolar.m_FoilName);

    m_pdeReynolds->setValue(s_RefPolar.m_Reynolds);
    m_pdeMach->setValue(s_RefPolar.m_Mach);
    m_pdeNCrit->setValue(s_RefPolar.m_NCrit);
    m_pdeTopTrans->setValue(s_RefPolar.m_XTop);
    m_pdeBotTrans->setValue(s_RefPolar.m_XBot);

    switch(s_RefPolar.polarType())
    {
        case xfl::FIXEDSPEEDPOLAR:
        {
            m_rbtype1->setChecked(true);
            break;
        }
        case xfl::FIXEDLIFTPOLAR:
        {
            m_rbtype2->setChecked(true);
            break;
        }
        case xfl::RUBBERCHORDPOLAR:
        {
            m_rbtype3->setChecked(true);
            break;
        }
        case xfl::FIXEDAOAPOLAR:
        {
            m_rbtype4->setChecked(true);
            break;
        }
        default:
        {
            m_rbtype1->setChecked(true);
            break;
        }
    }

    Units::getLengthUnitLabel(str);
    m_plabLengthUnit1->setText(str);
    m_plabLengthUnit2->setText(str);

    Units::getMassUnitLabel(str);
    m_plabMassUnit->setText(str);

    m_prbFluidUnit1->setChecked(s_UnitType==1);
    m_prbFluidUnit2->setChecked(s_UnitType!=1);
    m_pdeViscosity->setValue(s_Viscosity);
    onFluiUnit();

    m_pdeMass->setValue(s_Mass*Units::kgtoUnit());
    m_pdeSpan->setValue(s_Span*Units::mtoUnit());
    m_pdeChord->setValue(s_Chord*Units::mtoUnit());

    onPolarType();

    m_bAutoName = true;
    m_prbAuto1->setChecked(true);

}


void FoilPolarDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                readParams();
                setPlrName();
                m_pButtonBox->setFocus();
                return;
            }
/*            else if(m_pButtonBox->hasFocus())
            {
                onOK();
                return;
            }*/
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void FoilPolarDlg::onAutoName()
{
    if(m_prbAuto2->isChecked())
    {
        m_bAutoName = false;
        m_pleAnalysisName->setFocus();
        m_pleAnalysisName->selectAll();
    }
    else
    {
        m_bAutoName = true;
        setPlrName();
    }
}


void FoilPolarDlg::onNameChanged()
{
    m_bAutoName = false;
    m_prbAuto1->setChecked(false);
    m_prbAuto2->setChecked(true);
}


void FoilPolarDlg::onOK()
{
    s_RefPolar.setName(m_pleAnalysisName->text());
    accept();
}


void FoilPolarDlg::onPolarType()
{
    if(m_rbtype1->isChecked())
    {
        m_plabRe->setText(tr("Reynolds ="));
        m_plabReUnit->setText(" ");
        m_plabMach->setText(tr("Mach ="));
        m_pdeReynolds->setValue(s_RefPolar.m_Reynolds);
        s_RefPolar.m_PolarType = xfl::FIXEDSPEEDPOLAR;
    }
    else if(m_rbtype2->isChecked())
    {
        m_plabRe->setText(tr("Re.sqrt(Cl) ="));
        m_plabReUnit->setText(" ");
        m_plabMach->setText(tr("Ma.sqrt(Cl) ="));
        s_RefPolar.m_PolarType = xfl::FIXEDLIFTPOLAR;
        onCalcReynolds();
    }
    else if(m_rbtype3->isChecked())
    {
        m_plabRe->setText(tr("Re.Cl ="));
        m_plabReUnit->setText(" ");
        m_plabMach->setText(tr("Mach ="));
        m_pdeReynolds->setValue(s_RefPolar.m_Reynolds);
        s_RefPolar.m_PolarType = xfl::RUBBERCHORDPOLAR;
    }
    else if(m_rbtype4->isChecked())
    {
        m_plabRe->setText(tr("Alpha ="));
        m_plabReUnit->setText(QChar(0260));
        m_plabMach->setText(tr("Mach ="));
        m_pdeReynolds->setValue(s_RefPolar.m_ASpec);
        s_RefPolar.m_PolarType = xfl::FIXEDAOAPOLAR;
    }

    m_pdeChord->setEnabled(     s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);
    m_pdeSpan->setEnabled(      s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);
    m_pdeMass->setEnabled(      s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);
    m_pdeViscosity->setEnabled( s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);
    m_pdeDensity->setEnabled(   s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);
    m_prbFluidUnit1->setEnabled(s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);
    m_prbFluidUnit2->setEnabled(s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR);

    setPlrName();
}


void FoilPolarDlg::setPlrName()
{
    readParams();

    if(m_bAutoName)
    {
        m_PlrName= Polar::autoPolarName(s_RefPolar.m_PolarType, s_RefPolar.m_Reynolds, s_RefPolar.m_Mach, s_RefPolar.m_NCrit, s_RefPolar.m_ASpec, s_RefPolar.m_XTop, s_RefPolar.m_XBot);
        m_pleAnalysisName->setText(m_PlrName);
    }
}


void FoilPolarDlg::onFluiUnit()
{
    if(m_prbFluidUnit1->isChecked())
    {
        s_UnitType   = 1;

        m_pdeViscosity->setValue(s_Viscosity);
        m_plabViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");

        m_pdeDensity->setValue(s_Density);
        m_plabDensityUnit->setText("kg/m3");
    }
    else
    {
        s_UnitType   = 2;

        m_pdeViscosity->setValue(s_Viscosity* 10.7182881);
        m_plabViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");

        m_pdeDensity->setValue(s_Density*0.00194122);
        m_plabDensityUnit->setText("slugs/ft3");
    }
}


void FoilPolarDlg::readParams()
{
    bool bOK;
    QString str;
    str = m_pdeReynolds->text();
    str.replace(" ","");
    if(s_RefPolar.m_PolarType==xfl::FIXEDAOAPOLAR)
        s_RefPolar.m_ASpec    = locale().toDouble(str, &bOK);
    else
        s_RefPolar.m_Reynolds = locale().toDouble(str, &bOK);

    s_RefPolar.m_Mach     = m_pdeMach->value();

    s_RefPolar.m_NCrit = m_pdeNCrit->value();
    s_RefPolar.m_XTop  = m_pdeTopTrans->value();
    s_RefPolar.m_XBot  = m_pdeBotTrans->value();

    s_Mass = m_pdeMass->value()/Units::kgtoUnit();
    s_Chord = m_pdeChord->value()/Units::mtoUnit();
    s_Span = m_pdeSpan->value()/Units::mtoUnit();
    s_Viscosity = m_pdeViscosity->value();

    if(m_prbFluidUnit1->isChecked()) s_UnitType=1; else s_UnitType=2;

    s_Viscosity = m_pdeViscosity->value();
    s_Density = m_pdeDensity->value();
    if(s_UnitType==2)
    {
        s_Viscosity *= 1.0/10.7182881;
        s_Density   *= 1.0/0.00194122;
    }
}


void FoilPolarDlg::onCalcReynolds()
{
    readParams();

    if(s_RefPolar.m_PolarType==xfl::FIXEDLIFTPOLAR)
    {
        double lift   = s_Mass*9.81;
        double area   = s_Chord*s_Span;
        double VCl05  = sqrt(2.0*lift/s_Density/area);
        double ReCl05 = s_Chord/s_Viscosity*VCl05;
        m_pdeReynolds->setValue(ReCl05);
    }
}


void FoilPolarDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_WindowGeometry);
    pEvent->accept();
}



void FoilPolarDlg::hideEvent(QHideEvent *pEvent)
{
    s_WindowGeometry = saveGeometry();

    pEvent->accept();
}


void FoilPolarDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FoilPolarDlg");
    {
        s_WindowGeometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        s_UnitType      = settings.value("UnitType", 1).toInt();
        s_Chord         = settings.value("Chord", 1.0).toDouble();
        s_Span          = settings.value("Span", 1.0).toDouble();
        s_Mass          = settings.value("Mass", 1.0).toDouble();
        s_Density       = settings.value("Density", 1.225).toDouble();
        s_Viscosity     = settings.value("Viscosity", 1.5e-5).toDouble();

        s_RefPolar.setNCrit( settings.value("NCrit").toDouble());
        s_RefPolar.setXtrTop(settings.value("XTopTr").toDouble());
        s_RefPolar.setXtrBot(settings.value("XBotTr").toDouble());
        s_RefPolar.setMach(  settings.value("Mach").toDouble());
        s_RefPolar.setAoa(   settings.value("ASpec").toDouble());

        int b = settings.value("Type").toInt();
        if     (b==1) s_RefPolar.setPolarType(xfl::FIXEDSPEEDPOLAR);
        else if(b==2) s_RefPolar.setPolarType(xfl::FIXEDLIFTPOLAR);
        else if(b==3) s_RefPolar.setPolarType(xfl::RUBBERCHORDPOLAR);
        else if(b==4) s_RefPolar.setPolarType(xfl::FIXEDAOAPOLAR);

    }
    settings.endGroup();
}


void FoilPolarDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FoilPolarDlg");
    {
        settings.setValue("WindowGeom", s_WindowGeometry);

        settings.setValue("UnitType", s_UnitType);
        settings.setValue("Chord", s_Chord);
        settings.setValue("Span", s_Span);
        settings.setValue("Mass", s_Mass);
        settings.setValue("Density", s_Density);
        settings.setValue("Viscosity", s_Viscosity);

        settings.setValue("NCrit",  s_RefPolar.NCrit());
        settings.setValue("XTopTr", s_RefPolar.XtrTop());
        settings.setValue("XBotTr", s_RefPolar.XtrBot());
        settings.setValue("Mach",   s_RefPolar.Mach());
        settings.setValue("ASpec",  s_RefPolar.aoa());

        if(     s_RefPolar.polarType()==xfl::FIXEDSPEEDPOLAR)  settings.setValue("Type", 1);
        else if(s_RefPolar.polarType()==xfl::FIXEDLIFTPOLAR)   settings.setValue("Type", 2);
        else if(s_RefPolar.polarType()==xfl::RUBBERCHORDPOLAR) settings.setValue("Type", 3);
        else if(s_RefPolar.polarType()==xfl::FIXEDAOAPOLAR)    settings.setValue("Type", 4);
    }
    settings.endGroup();
}

