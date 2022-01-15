/****************************************************************************

    AeroData Class
    Copyright (C) 2015 André Deperrois

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


#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <cmath>

#include "aerodatadlg.h"
#include <xflcore/units.h>
#include <xflwidgets/customwts/doubleedit.h>

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

    if(s_bCelsius)    m_pdeTemperature->setValue(s_Temperature-STANDARDTEMPERATURE+15);
    else
    {
        double temp = s_Temperature-STANDARDTEMPERATURE+15;
        m_pdeTemperature->setValue(temp*9.0/5.0 + 32.0);
    }


    m_pdeAltitude->setValue(s_Altitude);

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

        m_pdeTemperature = new DoubleEdit(s_Temperature, 1, this);
        m_pdeAltitude    = new DoubleEdit(s_Altitude, 1, this);
        connect(m_pdeTemperature, SIGNAL(editingFinished()), this, SLOT(updateResults()));
        connect(m_pdeAltitude,    SIGNAL(editingFinished()), this, SLOT(updateResults()));

        m_plabAirPressure        = new QLabel("1000.0");
        m_plabAirDensity         = new QLabel("1.225");
        m_plabDynamicViscosity   = new QLabel("1.5e-5");
        m_plabKinematicViscosity = new QLabel("1.5e-5");
        m_plabSpeedOfSound       = new QLabel("300");
        m_plabAirPressure->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
        m_plabAirDensity->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
        m_plabDynamicViscosity->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
        m_plabKinematicViscosity->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
        m_plabSpeedOfSound->setAlignment(Qt::AlignRight| Qt::AlignVCenter);

        m_pcbTempUnit = new QComboBox;
        m_pcbTempUnit->addItem(QString::fromUtf8("  °C"));
        m_pcbTempUnit->addItem(QString::fromUtf8("  °F"));
        if(s_bCelsius) m_pcbTempUnit->setCurrentIndex(0);
        else           m_pcbTempUnit->setCurrentIndex(1);
        connect(m_pcbTempUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(onTempUnit()));

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

        pDataLayout->addWidget(m_pdeTemperature,        1, 2);
        pDataLayout->addWidget(m_pdeAltitude,           2, 2);
        pDataLayout->addWidget(m_plabAirPressure,        4, 2);
        pDataLayout->addWidget(m_plabAirDensity,         5, 2);
        pDataLayout->addWidget(m_plabDynamicViscosity,   6, 2);
        pDataLayout->addWidget(m_plabKinematicViscosity, 7, 2);
        pDataLayout->addWidget(m_plabSpeedOfSound,       8, 2);

        pDataLayout->addWidget(m_pcbTempUnit,     1, 3);
        pDataLayout->addWidget(pAltitudeUnitLabel, 2, 3);
        pDataLayout->addWidget(pPressureUnitLabel, 4, 3);
        pDataLayout->addWidget(pDensUnitLabel,     5, 3);
        pDataLayout->addWidget(pDynViscUnitLabel,  6, 3);
        pDataLayout->addWidget(pKinViscUnitLabel,  7, 3);
        pDataLayout->addWidget(pSpeedUnitLabel,    8, 3);

        pDataLayout->setColumnStretch(3,1);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout();
    {
        pMainLayout->addWidget(pValid);
        pMainLayout->addSpacing(17);
        pMainLayout->addStretch();
        pMainLayout->addLayout(pDataLayout);
        pMainLayout->addStretch();
        pMainLayout->addSpacing(17);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch();
    }

    setLayout(pMainLayout);
}


void AeroDataDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void AeroDataDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                //                updateResults();
                m_pButtonBox->setFocus();
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


void AeroDataDlg::updateResults()
{

    if(m_pcbTempUnit->currentIndex()==0)
    {
        s_Temperature = m_pdeTemperature->value()+STANDARDTEMPERATURE-15;
    }
    else
    {
        // convert first to Celsius : Deduct 32, then multiply by 5, then divide by 9
        s_Temperature = (m_pdeTemperature->value()-32.0)*5.0/9.0;
        s_Temperature += STANDARDTEMPERATURE-15;
    }

    s_Altitude = m_pdeAltitude->value();
    m_plabAirPressure->setText(QString("%1").arg(AirPressure(s_Altitude)));

    double density = AirDensity(s_Altitude, s_Temperature);
    double dynViscosity = DynamicViscosity(s_Altitude, s_Temperature);

    m_plabAirDensity->setText(QString("%1").arg(density));
    m_plabDynamicViscosity->setText(QString("%1").arg(dynViscosity, 7, 'e', 2));
    m_plabKinematicViscosity->setText(QString("%1").arg(dynViscosity/density, 7, 'e', 2));

    m_plabSpeedOfSound->setText(QString("%1").arg(SpeedOfSound(s_Temperature), 7, 'f', 1));
}



void AeroDataDlg::onTempUnit()
{
    s_bCelsius = m_pcbTempUnit->currentIndex()==0;

    if(s_bCelsius)
    {
        m_pdeTemperature->setValue(s_Temperature-STANDARDTEMPERATURE+15);
    }
    else
    {
        m_pdeTemperature->setValue( (s_Temperature-STANDARDTEMPERATURE+15)*9.0/5.0+32);
    }
}






