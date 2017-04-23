/****************************************************************************

	AeroData Class
	Submitted by BuboMaximus 2014/03/17
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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


#include "AeroDataDlg.h"
#include <misc/Units.h>

#include <QtDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <math.h>
// International Standard Atmosphere

#define STANDARDTEMPERATURE  288.15  // [°K]
#define STANDARDGRAVITY      9.80665   // [m/s²]
#define STANDARDPRESSURE     101325  // [Pa]
#define STANDARDLAPSERATE    0.0065  // [K/m]

bool AeroDataDlg::s_bCelsius = true;
double AeroDataDlg::s_Altitude = 0.0;
double AeroDataDlg::s_Temperature = STANDARDTEMPERATURE;

AeroDataDlg::AeroDataDlg(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(tr("Air data"));

	UniversalGasConstant = 8.3144621;     // [J/(mol.K)]
	DryAirMolarMass      = 0.02896442;    // [kg/mol]
	AdiabaticIndex       = 1.4;
	SutherlandsConstant  = 120;           // [K]
	ReferenceViscosity   = 17.894e-6;     // [Pa.s] !!!!

	setupLayout();

	if(s_bCelsius)	m_pctrlTemperature->setValue(s_Temperature-STANDARDTEMPERATURE+15);
	else
	{
		double temp = s_Temperature-STANDARDTEMPERATURE+15;
		m_pctrlTemperature->setValue(temp*9.0/5.0 + 32.0);
	}


	m_pctrlAltitude->setValue(s_Altitude);

	updateResults();
}



double AeroDataDlg::AirTemperature(double Altitude)   //[K]
{
// Troposphere only <= 11000 m
	return STANDARDTEMPERATURE -  Altitude * STANDARDLAPSERATE;
}


double AeroDataDlg::AirPressure(double Altitude)    //[Pa]
{
// Troposphere only <= 11000 m
	return STANDARDPRESSURE * pow((AirTemperature(Altitude) / STANDARDTEMPERATURE),
								  (STANDARDGRAVITY * DryAirMolarMass / UniversalGasConstant / STANDARDLAPSERATE));
}


double AeroDataDlg::AirDensity(double Altitude, double temp)   //[kg/m3]
{
// Troposphere only <= 11000 m
// TemperatureCorrection is 0 for standard atmosphere

	return  AirPressure(Altitude) * DryAirMolarMass / UniversalGasConstant / (AirTemperature(Altitude) + TemperatureCorrection(temp));
}


double AeroDataDlg::DynamicViscosity(double Altitude, double temp)     //[kg/m3]
{
// Troposphere only <= 11000 m
// TemperatureCorrection is 0 for standard atmosphere

	double T = AirTemperature(Altitude) + TemperatureCorrection(temp);
	return    ReferenceViscosity * (STANDARDTEMPERATURE + SutherlandsConstant)
								 / (T                   + SutherlandsConstant) * pow((T / STANDARDTEMPERATURE), 1.5);
}


double AeroDataDlg::TemperatureCorrection(double temp)
{
	return temp-STANDARDTEMPERATURE;
}


double AeroDataDlg::KinematicViscosity(double Altitude , double temp)   //[kg/m3]
{
// Troposphere only <= 11000 m
// TemperatureCorrection is 0 for standard atmosphere
	return DynamicViscosity(Altitude, temp) / AirDensity(Altitude, temp);
}

double AeroDataDlg::KinematicViscosity()
{
	return KinematicViscosity(s_Altitude, s_Temperature);
}


double AeroDataDlg::SpeedOfSound(double temp)       //[m/s]
{
	return sqrt(AdiabaticIndex * UniversalGasConstant * temp / DryAirMolarMass);
}


double AeroDataDlg::AirDensity()
{
	return AirDensity(s_Altitude, s_Temperature);
}




void AeroDataDlg::setupLayout()
{
	QLabel *pValid = new QLabel(tr("Applicable in the troposphere\n i.e. Altitude < 11000m"));
	QFont TitleFont("Arial");
	TitleFont.setBold(true);
	pValid->setFont(TitleFont);
	pValid->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
	QGridLayout *pDataLayout = new QGridLayout;
	{
		QLabel *pTempLabel           = new QLabel(tr("Temperature"));
		QLabel *pAltitudeLabel       = new QLabel(tr("Altitude"));
		QLabel *pAirPressureLabel    = new QLabel(tr("Air Pressure"));
		QLabel *pAirDensLabel        = new QLabel(tr("Air Density"));
		QLabel *pDynamicViscLabel    = new QLabel(tr("Dynamic Viscosity"));
		QLabel *pKinematicViscLabel  = new QLabel(tr("Kinematic Viscosity"));
		QLabel *pSpeedOfSoundLabel   = new QLabel(tr("Speed of Sound"));
		pTempLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		pAltitudeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		pAirPressureLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		pAirDensLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		pDynamicViscLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		pSpeedOfSoundLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		m_pctrlTemperature = new DoubleEdit(s_Temperature, 1, this);
		m_pctrlAltitude    = new DoubleEdit(s_Altitude, 1, this);
		connect(m_pctrlTemperature, SIGNAL(editingFinished()), this, SLOT(updateResults()));
		connect(m_pctrlAltitude,    SIGNAL(editingFinished()), this, SLOT(updateResults()));

		m_pctrlAirPressure        = new QLabel("1000.0");
		m_pctrlAirDensity         = new QLabel("1.225");
		m_pctrlDynamicViscosity   = new QLabel("1.5e-5");
		m_pctrlKinematicViscosity = new QLabel("1.5e-5");
		m_pctrlSpeedOfSound       = new QLabel("300");
		m_pctrlAirPressure->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
		m_pctrlAirDensity->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
		m_pctrlDynamicViscosity->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
		m_pctrlKinematicViscosity->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
		m_pctrlSpeedOfSound->setAlignment(Qt::AlignRight| Qt::AlignVCenter);

		m_pctrlTempUnit = new QComboBox;
		m_pctrlTempUnit->addItem(QString::fromUtf8("  °C"));
		m_pctrlTempUnit->addItem(QString::fromUtf8("  °F"));
		if(s_bCelsius) m_pctrlTempUnit->setCurrentIndex(0);
		else           m_pctrlTempUnit->setCurrentIndex(1);
		connect(m_pctrlTempUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTempUnit()));

		QLabel * pAltitudeUnitLabel = new QLabel("m");
		QLabel * pPressureUnitLabel = new QLabel("Pa");
		QLabel * pDensUnitLabel     = new QLabel("kg/m3");
		QLabel * pDynViscUnitLabel  = new QLabel(QString::fromUtf8("m/s²"));
		QLabel * pKinViscUnitLabel  = new QLabel(QString::fromUtf8("m²/s"));
		QLabel * pSpeedUnitLabel    = new QLabel("m/s");
		pAltitudeUnitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pPressureUnitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pDensUnitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pDynViscUnitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pKinViscUnitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pSpeedUnitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		pDataLayout->addWidget(pTempLabel,          1, 1);
		pDataLayout->addWidget(pAltitudeLabel,      2, 1);
		pDataLayout->addWidget(pAirPressureLabel,   4, 1);
		pDataLayout->addWidget(pAirDensLabel,       5, 1);
		pDataLayout->addWidget(pDynamicViscLabel,   6, 1);
		pDataLayout->addWidget(pKinematicViscLabel, 7, 1);
		pDataLayout->addWidget(pSpeedOfSoundLabel,  8, 1);

		pDataLayout->addWidget(m_pctrlTemperature,        1, 2);
		pDataLayout->addWidget(m_pctrlAltitude,           2, 2);
		pDataLayout->addWidget(m_pctrlAirPressure,        4, 2);
		pDataLayout->addWidget(m_pctrlAirDensity,         5, 2);
		pDataLayout->addWidget(m_pctrlDynamicViscosity,   6, 2);
		pDataLayout->addWidget(m_pctrlKinematicViscosity, 7, 2);
		pDataLayout->addWidget(m_pctrlSpeedOfSound,       8, 2);

		pDataLayout->addWidget(m_pctrlTempUnit,     1, 3);
		pDataLayout->addWidget(pAltitudeUnitLabel, 2, 3);
		pDataLayout->addWidget(pPressureUnitLabel, 4, 3);
		pDataLayout->addWidget(pDensUnitLabel,     5, 3);
		pDataLayout->addWidget(pDynViscUnitLabel,  6, 3);
		pDataLayout->addWidget(pKinViscUnitLabel,  7, 3);
		pDataLayout->addWidget(pSpeedUnitLabel,    8, 3);

		pDataLayout->setColumnStretch(3,1);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		m_pOKButton = new QPushButton(tr("OK"));
		m_pCancelButton = new QPushButton(tr("Cancel"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pOKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pCancelButton);
		pCommandButtons->addStretch(1);
		connect(m_pOKButton,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(m_pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}


	QVBoxLayout * pMainLayout = new QVBoxLayout();
	{
		pMainLayout->addWidget(pValid);
		pMainLayout->addSpacing(17);
		pMainLayout->addStretch();
		pMainLayout->addLayout(pDataLayout);
		pMainLayout->addStretch();
		pMainLayout->addSpacing(17);
		pMainLayout->addLayout(pCommandButtons);
		pMainLayout->addStretch();
	}

	setLayout(pMainLayout);
}


void AeroDataDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!m_pOKButton->hasFocus() && !m_pCancelButton->hasFocus())
			{
//				updateResults();
				m_pOKButton->setFocus();
				return;
			}
			else
			{
				accept();
				return;
			}
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
		}
		default:
			event->ignore();
	}
}


void AeroDataDlg::updateResults()
{

	if(m_pctrlTempUnit->currentIndex()==0)
	{
		s_Temperature = m_pctrlTemperature->value()+STANDARDTEMPERATURE-15;
	}
	else
	{
		// convert first to Celsius : Deduct 32, then multiply by 5, then divide by 9
		s_Temperature = (m_pctrlTemperature->value()-32.0)*5.0/9.0;
		s_Temperature += STANDARDTEMPERATURE-15;
	}

	s_Altitude = m_pctrlAltitude->value();
	m_pctrlAirPressure->setText(QString("%1").arg(AirPressure(s_Altitude)));

	double density = AirDensity(s_Altitude, s_Temperature);
	double dynViscosity = DynamicViscosity(s_Altitude, s_Temperature);

	m_pctrlAirDensity->setText(QString("%1").arg(density));
	m_pctrlDynamicViscosity->setText(QString("%1").arg(dynViscosity, 7, 'e', 2));
	m_pctrlKinematicViscosity->setText(QString("%1").arg(dynViscosity/density, 7, 'e', 2));

	m_pctrlSpeedOfSound->setText(QString("%1").arg(SpeedOfSound(s_Temperature), 7, 'f', 1));
}



void AeroDataDlg::OnTempUnit()
{
	s_bCelsius = m_pctrlTempUnit->currentIndex()==0;

	if(s_bCelsius)
	{
		m_pctrlTemperature->setValue(s_Temperature-STANDARDTEMPERATURE+15);
	}
	else
	{
		m_pctrlTemperature->setValue( (s_Temperature-STANDARDTEMPERATURE+15)*9.0/5.0+32);
	}
}






