/****************************************************************************

	FoilPolarDlg Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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
#include "FoilPolarDlg.h"
#include <globals.h>
#include <misc/Units.h>
#include <xdirect/XDirect.h>

int FoilPolarDlg::s_UnitType = 1;
double FoilPolarDlg::s_Viscosity = 1.5e-5;
double FoilPolarDlg::s_Density   = 1.225;
double FoilPolarDlg::s_Chord = 0.0;
double FoilPolarDlg::s_Span = 0.0;
double FoilPolarDlg::s_Mass = 0.0;


FoilPolarDlg::FoilPolarDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Foil Polar Definition"));
	m_PolarType = XFOIL::FIXEDSPEEDPOLAR;
	m_NCrit     = 9.0;
	m_XTop      = 1.0;
	m_XBot      = 1.0;
	m_Mach      = 0.0;
	m_Reynolds  = 100000.0;
	m_ASpec     = 0.0;
	m_bAutoName = true;
	setupLayout();
}


void FoilPolarDlg::setupLayout()
{
	QFont SymbolFont("Symbol");

    QGroupBox *NameGroupBox = new QGroupBox(tr("Analysis Name"));
    {
        QVBoxLayout *AnalysisLayout = new QVBoxLayout;
        {
            QHBoxLayout *autoname = new QHBoxLayout;
            {
                m_pctrlAuto1 = new QRadioButton(tr("Automatic"));
                m_pctrlAuto2 = new QRadioButton(tr("User Defined"));
                m_pctrlAnalysisName = new QLineEdit(tr("Analysis Name"));
                autoname->addStretch(1);
                autoname->addWidget(m_pctrlAuto1);
                autoname->addStretch(1);
                autoname->addWidget(m_pctrlAuto2);
                autoname->addStretch(1);
            }
            AnalysisLayout->addLayout(autoname);
            AnalysisLayout->addWidget(m_pctrlAnalysisName);
        }
        NameGroupBox->setLayout(AnalysisLayout);
    }

    QGroupBox *TypeGroup = new QGroupBox(tr("Analysis Type"));
    {
        QHBoxLayout *AnalysisType = new QHBoxLayout;
        {
		  m_rbtype1 = new QRadioButton(tr("Type 1"));
            m_rbtype2 = new QRadioButton(tr("Type 2"));
            m_rbtype3 = new QRadioButton(tr("Type 3"));
            m_rbtype4 = new QRadioButton(tr("Type 4"));
            AnalysisType->addWidget(m_rbtype1);
            AnalysisType->addWidget(m_rbtype2);
            AnalysisType->addWidget(m_rbtype3);
            AnalysisType->addWidget(m_rbtype4);
            TypeGroup->setLayout(AnalysisType);
        }
    }

	QGroupBox *pAeroGroupBox = new QGroupBox(tr("Reynolds and Mach Numbers"));
    {
	   QVBoxLayout *pReMachLayout = new QVBoxLayout;
        {
			QHBoxLayout *Type2DataLayout = new QHBoxLayout;
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
				QGroupBox *pAeroDataGroupBox = new QGroupBox(tr("Aerodynamic Data"));
				{
					QGridLayout *AeroDataLayout = new QGridLayout;
					{
						QLabel *lab9 = new QLabel(tr("Unit"));
						m_pctrlUnit1 = new QRadioButton(tr("International"));
						m_pctrlUnit2 = new QRadioButton(tr("Imperial"));
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
						AeroDataLayout->addWidget(lab9,1,1);
						AeroDataLayout->addWidget(m_pctrlUnit1,1,2);
						AeroDataLayout->addWidget(m_pctrlUnit2,1,3);
						AeroDataLayout->addWidget(m_pctrlRho,2,1);
						AeroDataLayout->addWidget(m_pctrlDensity,2,2);
						AeroDataLayout->addWidget(m_pctrlDensityUnit,2,3);
						AeroDataLayout->addWidget(m_pctrlNu,3,1);
						AeroDataLayout->addWidget(m_pctrlViscosity,3,2);
						AeroDataLayout->addWidget(m_pctrlViscosityUnit,3,3);
					}
					pAeroDataGroupBox->setLayout(AeroDataLayout);
				}

				Type2DataLayout->addWidget(pPlaneDataGroupBox);
				Type2DataLayout->addWidget(pAeroDataGroupBox);
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

			pReMachLayout->addLayout(Type2DataLayout);
			pReMachLayout->addLayout(pReMachValuesLayout);
		}
		pAeroGroupBox->setLayout(pReMachLayout);
	}

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
        OKButton = new QPushButton(tr("OK"));
        OKButton->setAutoDefault(false);
        CancelButton = new QPushButton(tr("Cancel"));
        CancelButton->setAutoDefault(false);
        CommandButtons->addStretch(1);
        CommandButtons->addWidget(OKButton);
        CommandButtons->addStretch(1);
        CommandButtons->addWidget(CancelButton);
        CommandButtons->addStretch(1);
		connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
        connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    }

    QGroupBox *TransGroup = new QGroupBox(tr("Transition settings"));
    {
        QGridLayout *Transitions = new QGridLayout;
        {
            QLabel *FreeTransLabel   = new QLabel(tr("Free transitions (e^n) method"));
            QLabel *ForceTransLabel  = new QLabel(tr("Forced transition:"));
            QLabel	*NCritLabel      = new QLabel(tr("NCrit="));
            QLabel *TopTripLabel     = new QLabel(tr("TripLocation (top)"));
            QLabel *BotTripLabel     = new QLabel(tr("TripLocation (bot)"));
            m_pctrlNCrit    = new DoubleEdit();
            m_pctrlTopTrans = new DoubleEdit();
            m_pctrlBotTrans = new DoubleEdit();

            m_pctrlNCrit->setAlignment(   Qt::AlignRight);
            m_pctrlTopTrans->setAlignment(Qt::AlignRight);
            m_pctrlBotTrans->setAlignment(Qt::AlignRight);
            Transitions->addWidget(FreeTransLabel,   1,1, 1,1, Qt::AlignLeft| Qt::AlignVCenter);
            Transitions->addWidget(ForceTransLabel,  2,1, 1,1, Qt::AlignLeft| Qt::AlignVCenter);
            Transitions->addWidget(NCritLabel,       1,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            Transitions->addWidget(TopTripLabel,     2,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            Transitions->addWidget(BotTripLabel,     3,2, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            Transitions->addWidget(m_pctrlNCrit,     1,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            Transitions->addWidget(m_pctrlTopTrans,  2,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            Transitions->addWidget(m_pctrlBotTrans,  3,3, 1,1, Qt::AlignRight| Qt::AlignVCenter);
            TransGroup->setLayout(Transitions);
        }
    }

	QVBoxLayout *mainLayout = new QVBoxLayout;
	{
		mainLayout->addStretch();
		mainLayout->addWidget(NameGroupBox);
		mainLayout->addStretch();
		mainLayout->addWidget(TypeGroup);
		mainLayout->addStretch();
		mainLayout->addWidget(pAeroGroupBox);
		mainLayout->addWidget(TransGroup);
		mainLayout->addStretch();
		mainLayout->addLayout(CommandButtons);
		mainLayout->addStretch();
	}

	setLayout(mainLayout);

	m_pctrlTopTrans->setPrecision(2);
	m_pctrlTopTrans->setMin(0.0);
	m_pctrlTopTrans->setMax(1.0);

	m_pctrlBotTrans->setPrecision(2);
	m_pctrlBotTrans->setMin(0.0);
	m_pctrlBotTrans->setMax(1.0);


	m_pctrlNCrit->setPrecision(3);
	m_pctrlNCrit->setMin(0.0);
	m_pctrlNCrit->setMax(1000000.0);


	m_pctrlReynolds->setPrecision(0);
	m_pctrlReynolds->setMin(-1.0e10);
	m_pctrlReynolds->setMax(1.e10);


	m_pctrlMach->setMin(0.0);
	m_pctrlMach->setMax(1000.0);

	connect(m_pctrlAuto1, SIGNAL(clicked()), this, SLOT(onAutoName()));
	connect(m_pctrlAuto2, SIGNAL(clicked()), this, SLOT(onAutoName()));

	connect(m_rbtype1, SIGNAL(clicked()), this, SLOT(onPolarType()));
	connect(m_rbtype2, SIGNAL(clicked()), this, SLOT(onPolarType()));
	connect(m_rbtype3, SIGNAL(clicked()), this, SLOT(onPolarType()));
	connect(m_rbtype4, SIGNAL(clicked()), this, SLOT(onPolarType()));

	connect(m_pctrlReynolds, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
	connect(m_pctrlMach, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
	connect(m_pctrlNCrit, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
	connect(m_pctrlTopTrans, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
	connect(m_pctrlBotTrans, SIGNAL(editingFinished()), this, SLOT(editingFinished()));

	connect(m_pctrlAnalysisName, SIGNAL(textEdited (const QString &)), this, SLOT(onNameChanged()));

	connect(m_pctrlUnit1, SIGNAL(toggled(bool)), this, SLOT(onUnit()));
	connect(m_pctrlUnit2, SIGNAL(toggled(bool)), this, SLOT(onUnit()));

	connect(m_pctrlChord, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(m_pctrlSpan, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(m_pctrlMass, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(m_pctrlViscosity, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(m_pctrlDensity, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}


void FoilPolarDlg::editingFinished()
{
	setPlrName();
//	OKButton->setFocus();
}


void FoilPolarDlg::initDialog()
{
	if(QXDirect::curFoil()) m_FoilName = QXDirect::curFoil()->foilName();
	else                    m_FoilName = "";

	QString str = tr("Analysis parameters for ");
	setWindowTitle(str+ m_FoilName);

	m_NCrit     = QXDirect::s_refPolar.NCrit();
	m_XBot      = QXDirect::s_refPolar.XtrBot();
	m_XTop      = QXDirect::s_refPolar.XtrTop();
	m_Mach      = QXDirect::s_refPolar.Mach();
	m_Reynolds  = QXDirect::s_refPolar.Reynolds();
	m_ASpec     = QXDirect::s_refPolar.aoa();
	m_PolarType = QXDirect::s_refPolar.polarType();


	m_pctrlReynolds->setValue(m_Reynolds);
	m_pctrlMach->setValue(m_Mach);
	m_pctrlNCrit->setValue(m_NCrit);
	m_pctrlTopTrans->setValue(m_XTop);
	m_pctrlBotTrans->setValue(m_XBot);

	switch(QXDirect::s_refPolar.polarType())
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

	m_pctrlUnit1->setChecked(s_UnitType==1);
	m_pctrlUnit2->setChecked(s_UnitType!=1);
	m_pctrlViscosity->setValue(s_Viscosity);
	m_pctrlDensity->setValue(s_Density);
	onUnit();

	m_pctrlMass->setValue(s_Mass);
	m_pctrlSpan->setValue(s_Span);
	m_pctrlChord->setValue(s_Chord);

	onPolarType();

	m_bAutoName = true;
	m_pctrlAuto1->setChecked(true);

}


void FoilPolarDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				readParams();
				setPlrName();
				OKButton->setFocus();
				return;
			}
			else if(OKButton->hasFocus())
			{
				onOK();
				return;
			}
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
		default:
			event->ignore();
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
	m_PlrName = m_pctrlAnalysisName->text();

	QXDirect::s_refPolar.polarType() = m_PolarType;
	QXDirect::s_refPolar.NCrit()     = m_NCrit;
	QXDirect::s_refPolar.XtrBot()    = m_XBot;
	QXDirect::s_refPolar.XtrTop()    = m_XTop;
	QXDirect::s_refPolar.Mach()      = m_Mach;
	QXDirect::s_refPolar.Reynolds()  = m_Reynolds;
	QXDirect::s_refPolar.aoa()       = m_ASpec;

	accept();
}


void FoilPolarDlg::onPolarType()
{
	if(m_rbtype1->isChecked())
	{
		m_pctrlReLabel->setText(tr("Reynolds ="));
		m_pctrlReUnit->setText(" ");
		m_pctrlMachLabel->setText(tr("Mach ="));
		m_pctrlReynolds->setPrecision(0);
		m_pctrlReynolds->setValue(m_Reynolds);
		m_PolarType = XFOIL::FIXEDSPEEDPOLAR;
	}
	else if(m_rbtype2->isChecked())
	{
		m_pctrlReLabel->setText(tr("Re.sqrt(Cl) ="));
		m_pctrlReUnit->setText(" ");
		m_pctrlMachLabel->setText(tr("Ma.sqrt(Cl) ="));
		m_pctrlReynolds->setPrecision(0);
		m_pctrlReynolds->setValue(m_Reynolds);		
		m_PolarType = XFOIL::FIXEDLIFTPOLAR;
	}
	else if(m_rbtype3->isChecked())
	{
		m_pctrlReLabel->setText(tr("Re.Cl ="));
		m_pctrlReUnit->setText(" ");
		m_pctrlMachLabel->setText(tr("Mach ="));
		m_pctrlReynolds->setPrecision(0);
		m_pctrlReynolds->setValue(m_Reynolds);
		m_PolarType = XFOIL::RUBBERCHORDPOLAR;
	}
	else if(m_rbtype4->isChecked())
	{
		m_pctrlReLabel->setText(tr("Alpha ="));
		m_pctrlReUnit->setText(QString::fromUtf8("°"));
		m_pctrlMachLabel->setText(tr("Mach ="));
		m_pctrlReynolds->setPrecision(2);
		m_pctrlReynolds->setValue(m_ASpec);
		m_PolarType = XFOIL::FIXEDAOAPOLAR;
	}

	m_pctrlChord->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);
	m_pctrlSpan->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);
	m_pctrlMass->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);
	m_pctrlViscosity->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);
	m_pctrlDensity->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);
	m_pctrlUnit1->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);
	m_pctrlUnit2->setEnabled(m_PolarType==XFOIL::FIXEDLIFTPOLAR);

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



void FoilPolarDlg::onUnit()
{
	if(m_pctrlUnit1->isChecked())
	{
		s_UnitType   = 1;
		m_pctrlViscosity->setValue(s_Viscosity);
		m_pctrlDensityUnit->setText("kg/m3");
		m_pctrlViscosityUnit->setText("m"+QString::fromUtf8("²")+"/s");
	}
	else
	{
		s_UnitType   = 2;
		m_pctrlViscosity->setValue(s_Viscosity* 10.7182881);
		m_pctrlDensityUnit->setText("slugs/ft3");
		m_pctrlViscosityUnit->setText("ft"+QString::fromUtf8("²")+"/s");
	}
	setDensity();
}


void FoilPolarDlg::readParams()
{
	bool bOK;
    QString str;
    str = m_pctrlReynolds->text();
    str.replace(" ","");
	if(m_PolarType==XFOIL::FIXEDAOAPOLAR) m_ASpec    = locale().toDouble(str, &bOK);
	else                                  m_Reynolds = locale().toDouble(str, &bOK);

	m_Mach     = m_pctrlMach->value();
//  m_pctrlMach->clear();
//	m_pctrlMach->insert(str.setNum(m_Mach,'f',3));

	m_NCrit  = m_pctrlNCrit->value();
	m_XTop = m_pctrlTopTrans->value();
	m_XBot = m_pctrlBotTrans->value();

    s_Mass = m_pctrlMass->value();
    s_Chord = m_pctrlChord->value();
    s_Span = m_pctrlSpan->value();
    s_Viscosity = m_pctrlViscosity->value();
    s_Density = m_pctrlDensity->value();
}



void FoilPolarDlg::setDensity()
{
	int exp, precision;
	if(m_pctrlUnit1->isChecked())
	{
		exp = (int)log(s_Density);
		if(exp>1) precision = 1;
		else if(exp<-4) precision = 4;
		else precision = 3-exp;
		m_pctrlDensity->setPrecision(precision);
		m_pctrlDensity->setValue(s_Density);
	}
	else
	{
		exp = (int)log(s_Density* 0.00194122);
		if(exp>1) precision = 1;
		else if(exp<-4) precision = 4;
		else precision = 3-exp;
		m_pctrlDensity->setPrecision(precision);
		m_pctrlDensity->setValue(s_Density* 0.00194122);
	}
}



void FoilPolarDlg::onEditingFinished()
{
	readParams();
	if(m_PolarType==XFOIL::FIXEDLIFTPOLAR)
	{
		//compute Re.sqrt(Cl)
		m_pctrlReynolds->setValue(sqrt(2.0*s_Mass*9.81*s_Density*s_Chord/s_Viscosity/s_Viscosity/s_Span));
	}
}









