/****************************************************************************

    FoilPolarDlg Class
    Copyright (C) 2008-2016 Andre Deperrois 

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
#include <globals/globals.h>
#include <misc/options/units.h>
#include <xdirect/xdirect.h>
#include <misc/text/doubleedit.h>
#include <objects/objects2d/polar.h>

QByteArray FoilPolarDlg::s_WindowGeometry;

int FoilPolarDlg::s_UnitType = 1;
double FoilPolarDlg::s_Viscosity = 1.5e-5;
double FoilPolarDlg::s_Density   = 1.225;
double FoilPolarDlg::s_Chord = 1.0;
double FoilPolarDlg::s_Span = 1.0;
double FoilPolarDlg::s_Mass = 1.0;


FoilPolarDlg::FoilPolarDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Foil Polar Definition"));

    m_PolarType = XFLR5::FIXEDSPEEDPOLAR;
    m_NCrit     = 9.0;
    m_XTop      = 1.0;
    m_XBot      = 1.0;
    m_Mach      = 0.0;
    m_Reynolds  = 100000.0;
    m_ASpec     = 0.0;

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
                m_pctrlAuto1 = new QRadioButton(tr("Automatic"));
                m_pctrlAuto2 = new QRadioButton(tr("User Defined"));
                m_pctrlAnalysisName = new QLineEdit(tr("Analysis Name"));
                pAutoNameLayout->addStretch(1);
                pAutoNameLayout->addWidget(m_pctrlAuto1);
                pAutoNameLayout->addStretch(1);
                pAutoNameLayout->addWidget(m_pctrlAuto2);
                pAutoNameLayout->addStretch(1);
            }
            pAnalysisLayout->addLayout(pAutoNameLayout);
            pAnalysisLayout->addWidget(m_pctrlAnalysisName);
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
                        m_pctrlChord = new DoubleEdit(0,3);
                        m_pctrlMass = new DoubleEdit(0,3);
                        m_pctrlSpan = new DoubleEdit(0,3);
                        QLabel *ChordLab = new QLabel(tr("Chord"));
                        QLabel *MassLab = new QLabel(tr("Mass"));
                        QLabel *SpanLab = new QLabel(tr("Span"));
                        m_pctrlLengthUnit1 = new QLabel("m");
                        m_pctrlLengthUnit2 = new QLabel("m");
                        m_pctrlMassUnit = new QLabel("kg");
                        PlaneDataLayout->addWidget(ChordLab,1,1);
                        PlaneDataLayout->addWidget(m_pctrlChord,1,2);
                        PlaneDataLayout->addWidget(m_pctrlLengthUnit1,1,3);
                        PlaneDataLayout->addWidget(SpanLab,2,1);
                        PlaneDataLayout->addWidget(m_pctrlSpan,2,2);
                        PlaneDataLayout->addWidget(m_pctrlLengthUnit2,2,3);
                        PlaneDataLayout->addWidget(MassLab,3,1);
                        PlaneDataLayout->addWidget(m_pctrlMass,3,2);
                        PlaneDataLayout->addWidget(m_pctrlMassUnit,3,3);
                    }
                    pPlaneDataGroupBox->setLayout(PlaneDataLayout);
                }
                QGroupBox *pAeroDataGroupBox = new QGroupBox(tr("Fluid properties"));
                {
                    QGridLayout *pAeroDataLayout = new QGridLayout;
                    {
                        QLabel *lab9 = new QLabel(tr("Unit"));
                        m_pctrlFluidUnit1 = new QRadioButton(tr("International"));
                        m_pctrlFluidUnit2 = new QRadioButton(tr("Imperial"));
                        m_pctrlRho = new QLabel("r =");
                        m_pctrlDensity = new DoubleEdit(1.225,3);
                        m_pctrlDensityUnit = new QLabel("kg/m3");
                        m_pctrlNu = new QLabel("n =");
                        m_pctrlRho->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                        m_pctrlNu->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                        m_pctrlViscosity = new DoubleEdit(1.500e-5,3);
                        m_pctrlViscosityUnit = new QLabel(QString::fromUtf8("m²/s"));
                        m_pctrlRho->setFont(SymbolFont);
                        m_pctrlNu->setFont(SymbolFont);
                        m_pctrlDensity->setPrecision(5);
                        m_pctrlViscosity->setPrecision(3);
                        m_pctrlDensity->setMin(0.0);
                        m_pctrlViscosity->setMin(0.0);
                        pAeroDataLayout->addWidget(lab9,1,1);
                        pAeroDataLayout->addWidget(m_pctrlFluidUnit1,   1,2);
                        pAeroDataLayout->addWidget(m_pctrlFluidUnit2,   1,3);
                        pAeroDataLayout->addWidget(m_pctrlRho,          2,1);
                        pAeroDataLayout->addWidget(m_pctrlDensity,      2,2);
                        pAeroDataLayout->addWidget(m_pctrlDensityUnit,  2,3);
                        pAeroDataLayout->addWidget(m_pctrlNu,           3,1);
                        pAeroDataLayout->addWidget(m_pctrlViscosity,    3,2);
                        pAeroDataLayout->addWidget(m_pctrlViscosityUnit,3,3);
                    }
                    pAeroDataGroupBox->setLayout(pAeroDataLayout);
                }

                pType2DataLayout->addWidget(pPlaneDataGroupBox);
                pType2DataLayout->addWidget(pAeroDataGroupBox);
            }

            QHBoxLayout *pReMachValuesLayout= new QHBoxLayout;
            {
                m_pctrlReLabel  = new QLabel(tr("  Re ="));
                m_pctrlReLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pctrlReUnit  = new QLabel(tr(" "));
                m_pctrlReUnit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pctrlMachLabel = new QLabel(tr("Mach ="));
                m_pctrlMachLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pctrlReynolds = new DoubleEdit();
                m_pctrlReynolds->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pctrlReynolds->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                QFontMetrics fm(m_pctrlReynolds->font());
                m_pctrlReynolds->setMinimumWidth(fm.averageCharWidth()*15);
                m_pctrlMach = new DoubleEdit(0.0, 3);
                m_pctrlMach->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pReMachValuesLayout->addStretch(1);
                pReMachValuesLayout->addWidget(m_pctrlReLabel);
                pReMachValuesLayout->addWidget(m_pctrlReynolds);
                pReMachValuesLayout->addWidget(m_pctrlReUnit);
                pReMachValuesLayout->addStretch(1);
                pReMachValuesLayout->addWidget(m_pctrlMachLabel);
                pReMachValuesLayout->addWidget(m_pctrlMach);
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
            m_pctrlNCrit    = new DoubleEdit();
            m_pctrlTopTrans = new DoubleEdit();
            m_pctrlBotTrans = new DoubleEdit();

            m_pctrlNCrit->setAlignment(   Qt::AlignRight);
            m_pctrlTopTrans->setAlignment(Qt::AlignRight);
            m_pctrlBotTrans->setAlignment(Qt::AlignRight);
            pTransitionsLayout->addWidget(pFreeTransLabel,   1,1, 1,1, Qt::AlignLeft| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pForceTransLabel,  2,1, 1,1, Qt::AlignLeft| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pNCritLabel,       1,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pTopTripLabel,     2,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(pBotTripLabel,     3,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(m_pctrlNCrit,     1,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(m_pctrlTopTrans,  2,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            pTransitionsLayout->addWidget(m_pctrlBotTrans,  3,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
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

    m_pctrlTopTrans->setPrecision(2);
    m_pctrlTopTrans->setMin(0.0);
    m_pctrlTopTrans->setMax(1.0);

    m_pctrlBotTrans->setPrecision(2);
    m_pctrlBotTrans->setMin(0.0);
    m_pctrlBotTrans->setMax(1.0);


    m_pctrlNCrit->setPrecision(3);
    m_pctrlNCrit->setMin(0.0);
    m_pctrlNCrit->setMax(1000000.0);


    m_pctrlMach->setMin(0.0);
    m_pctrlMach->setMax(1000.0);
}


void FoilPolarDlg::connectSignals()
{
    connect(m_pctrlAuto1, SIGNAL(clicked()), this, SLOT(onAutoName()));
    connect(m_pctrlAuto2, SIGNAL(clicked()), this, SLOT(onAutoName()));

    connect(m_rbtype1, SIGNAL(clicked()), this, SLOT(onPolarType()));
    connect(m_rbtype2, SIGNAL(clicked()), this, SLOT(onPolarType()));
    connect(m_rbtype3, SIGNAL(clicked()), this, SLOT(onPolarType()));
    connect(m_rbtype4, SIGNAL(clicked()), this, SLOT(onPolarType()));

    connect(m_pctrlReynolds, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pctrlMach,     SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pctrlNCrit,    SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pctrlTopTrans, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_pctrlBotTrans, SIGNAL(editingFinished()), this, SLOT(editingFinished()));

    connect(m_pctrlAnalysisName, SIGNAL(textEdited (const QString &)), this, SLOT(onNameChanged()));

    connect(m_pctrlFluidUnit1, SIGNAL(clicked(bool)), this, SLOT(onFluiUnit()));
    connect(m_pctrlFluidUnit2, SIGNAL(clicked(bool)), this, SLOT(onFluiUnit()));

    connect(m_pctrlChord,     SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pctrlSpan,      SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pctrlMass,      SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pctrlViscosity, SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
    connect(m_pctrlDensity,   SIGNAL(editingFinished()), this, SLOT(onCalcReynolds()));
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
//    OKButton->setFocus();
}


void FoilPolarDlg::initDialog()
{
    if(XDirect::curFoil()) XDirect::s_RefPolar.m_FoilName = XDirect::curFoil()->foilName();
    else                   XDirect::s_RefPolar.m_FoilName = "";

    QString str = tr("Analysis parameters for ");
    setWindowTitle(str+ XDirect::s_RefPolar.m_FoilName);

    m_pctrlReynolds->setValue(XDirect::s_RefPolar.m_Reynolds);
    m_pctrlMach->setValue(XDirect::s_RefPolar.m_Mach);
    m_pctrlNCrit->setValue(XDirect::s_RefPolar.m_ACrit);
    m_pctrlTopTrans->setValue(XDirect::s_RefPolar.m_XTop);
    m_pctrlBotTrans->setValue(XDirect::s_RefPolar.m_XBot);

    switch(XDirect::s_RefPolar.polarType())
    {
        case XFLR5::FIXEDSPEEDPOLAR:
        {
            m_rbtype1->setChecked(true);
            break;
        }
        case XFLR5::FIXEDLIFTPOLAR:
        {
            m_rbtype2->setChecked(true);
            break;
        }
        case XFLR5::RUBBERCHORDPOLAR:
        {
            m_rbtype3->setChecked(true);
            break;
        }
        case XFLR5::FIXEDAOAPOLAR:
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
    m_pctrlLengthUnit1->setText(str);
    m_pctrlLengthUnit2->setText(str);

    Units::getWeightUnitLabel(str);
    m_pctrlMassUnit->setText(str);

    m_pctrlFluidUnit1->setChecked(s_UnitType==1);
    m_pctrlFluidUnit2->setChecked(s_UnitType!=1);
    m_pctrlViscosity->setValue(s_Viscosity);
    onFluiUnit();

    m_pctrlMass->setValue(s_Mass*Units::kgtoUnit());
    m_pctrlSpan->setValue(s_Span*Units::mtoUnit());
    m_pctrlChord->setValue(s_Chord*Units::mtoUnit());

    onPolarType();

    m_bAutoName = true;
    m_pctrlAuto1->setChecked(true);

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
    if(m_pctrlAuto2->isChecked())
    {
        m_bAutoName = false;
        m_pctrlAnalysisName->setFocus();
        m_pctrlAnalysisName->selectAll();
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
    m_pctrlAuto1->setChecked(false);
    m_pctrlAuto2->setChecked(true);
}


void FoilPolarDlg::onOK()
{
    XDirect::s_RefPolar.m_PlrName = m_pctrlAnalysisName->text();

    XDirect::s_RefPolar.setPolarType(m_PolarType);
    XDirect::s_RefPolar.setNCrit(m_NCrit);
    XDirect::s_RefPolar.setXtrBot(m_XBot);
    XDirect::s_RefPolar.setXtrTop(m_XTop);
    XDirect::s_RefPolar.setMach(m_Mach);
    XDirect::s_RefPolar.setReynolds(m_Reynolds);
    XDirect::s_RefPolar.setAoa(m_ASpec);

    accept();
}


void FoilPolarDlg::onPolarType()
{
    if(m_rbtype1->isChecked())
    {
        m_pctrlReLabel->setText(tr("Reynolds ="));
        m_pctrlReUnit->setText(" ");
        m_pctrlMachLabel->setText(tr("Mach ="));
        m_pctrlReynolds->setValue(XDirect::s_RefPolar.m_Reynolds);
//        m_pctrlReynolds->setPrecision(0);
        m_PolarType = XFLR5::FIXEDSPEEDPOLAR;
    }
    else if(m_rbtype2->isChecked())
    {
        m_pctrlReLabel->setText(tr("Re.sqrt(Cl) ="));
        m_pctrlReUnit->setText(" ");
        m_pctrlMachLabel->setText(tr("Ma.sqrt(Cl) ="));
        m_PolarType = XFLR5::FIXEDLIFTPOLAR;
//        m_pctrlReynolds->setPrecision(0);
        onCalcReynolds();
    }
    else if(m_rbtype3->isChecked())
    {
        m_pctrlReLabel->setText(tr("Re.Cl ="));
        m_pctrlReUnit->setText(" ");
        m_pctrlMachLabel->setText(tr("Mach ="));
        m_pctrlReynolds->setValue(XDirect::s_RefPolar.m_Reynolds);
//        m_pctrlReynolds->setPrecision(0);
        m_PolarType = XFLR5::RUBBERCHORDPOLAR;
    }
    else if(m_rbtype4->isChecked())
    {
        m_pctrlReLabel->setText(tr("Alpha ="));
        m_pctrlReUnit->setText(QString::fromUtf8("°"));
        m_pctrlMachLabel->setText(tr("Mach ="));
        m_pctrlReynolds->setValue(m_ASpec);
//        m_pctrlReynolds->setPrecision(3);
        m_PolarType = XFLR5::FIXEDAOAPOLAR;
    }

    m_pctrlChord->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);
    m_pctrlSpan->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);
    m_pctrlMass->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);
    m_pctrlViscosity->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);
    m_pctrlDensity->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);
    m_pctrlFluidUnit1->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);
    m_pctrlFluidUnit2->setEnabled(m_PolarType==XFLR5::FIXEDLIFTPOLAR);

    setPlrName();
}


void FoilPolarDlg::setPlrName()
{
    readParams();

    if(m_bAutoName)
    {
        m_PlrName= Polar::autoPolarName(m_PolarType, m_Reynolds, m_Mach, m_NCrit, m_ASpec, m_XTop, m_XBot);
        m_pctrlAnalysisName->setText(m_PlrName);
    }
}


void FoilPolarDlg::onFluiUnit()
{
    if(m_pctrlFluidUnit1->isChecked())
    {
        s_UnitType   = 1;

        m_pctrlViscosity->setValue(s_Viscosity);
        m_pctrlViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");

        m_pctrlDensity->setValue(s_Density);
        m_pctrlDensityUnit->setText("kg/m3");
    }
    else
    {
        s_UnitType   = 2;

        m_pctrlViscosity->setValue(s_Viscosity* 10.7182881);
        m_pctrlViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");

        m_pctrlDensity->setValue(s_Density*0.00194122);
        m_pctrlDensityUnit->setText("slugs/ft3");
    }
}


void FoilPolarDlg::readParams()
{
    bool bOK;
    QString str;
    str = m_pctrlReynolds->text();
    str.replace(" ","");
    if(m_PolarType==XFLR5::FIXEDAOAPOLAR) m_ASpec    = locale().toDouble(str, &bOK);
    else                                  m_Reynolds = locale().toDouble(str, &bOK);

    m_Mach     = m_pctrlMach->value();
//  m_pctrlMach->clear();
//    m_pctrlMach->insert(str.setNum(m_Mach,'f',3));

    m_NCrit  = m_pctrlNCrit->value();
    m_XTop = m_pctrlTopTrans->value();
    m_XBot = m_pctrlBotTrans->value();

    s_Mass = m_pctrlMass->value()/Units::kgtoUnit();
    s_Chord = m_pctrlChord->value()/Units::mtoUnit();
    s_Span = m_pctrlSpan->value()/Units::mtoUnit();
    s_Viscosity = m_pctrlViscosity->value();

    if(m_pctrlFluidUnit1->isChecked()) s_UnitType=1; else s_UnitType=2;

    s_Viscosity = m_pctrlViscosity->value();
    s_Density = m_pctrlDensity->value();
    if(s_UnitType==2)
    {
        s_Viscosity *= 1.0/10.7182881;
        s_Density   *= 1.0/0.00194122;
    }
}





void FoilPolarDlg::onCalcReynolds()
{
    readParams();

    if(m_PolarType==XFLR5::FIXEDLIFTPOLAR)
    {
        double lift   = s_Mass*9.81;
        double area   = s_Chord*s_Span;
        double VCl05  = sqrt(2.0*lift/s_Density/area);
        double ReCl05 = s_Chord/s_Viscosity*VCl05;
        m_pctrlReynolds->setValue(ReCl05);
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
    }
    settings.endGroup();
}

