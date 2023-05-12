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

#pragma once



/**
 *@file
 *
 * This file defines the AeroDataDlg class, which is used to calculate the air density and kinematic viscosity
 * as a function of temperature and altitude, using the ISA atmospheric model
 *
 */



#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>

class DoubleEdit;

/**
 *@class AeroDataDlg
 *@brief The class is used to calculate the air density and kinematic viscosity
 * as a function of temperature and altitude, using the ISA atmospheric model
 */

class AeroDataDlg : public QDialog
{
    Q_OBJECT

    public:
        AeroDataDlg(QWidget *parent = nullptr);

    public:
        double AirDensity() const;   //[kg/m3]
        double KinematicViscosity() const;     //[kg/m3]


    private:
        void setupLayout();

        double TemperatureCorrection(double temp) const;
        double AirTemperature(double Altitude)  const;   //[K]
        double AirPressure(double Altitude) const;    //[Pa]
        double AirDensity(double Altitude, double temp) const;   //[kg/m3]
        double DynamicViscosity(double Altitude, double temp) const;     //[kg/m3]
        double KinematicViscosity(double Altitude , double Temp) const;   //[kg/m3]
        double SpeedOfSound(double temp) const;       //[m/s]

    private:
        void keyPressEvent(QKeyEvent *pEvent);

    private slots:
        void onTempUnit();
        void updateResults() const;
        void onButton(QAbstractButton *pButton);


    public:
        static double s_Altitude;    // meters
        static double s_Temperature; // degree Kelvin
        static bool s_bCelsius;



    private:
        double UniversalGasConstant;        // [J/(mol.K)]
        double DryAirMolarMass;             // [kg/mol]
        double AdiabaticIndex;
        double SutherlandsConstant;         // [K]
        double ReferenceViscosity;          // [Pa.s] !!!!

        QComboBox *m_pcbTempUnit;

        DoubleEdit *m_pdeTemperature, *m_pdeAltitude;

        QLabel *m_plabAirDensity, *m_plabAirPressure;
        QLabel *m_plabKinematicViscosity;
        QLabel *m_plabDynamicViscosity;
        QLabel *m_plabSpeedOfSound;

        QDialogButtonBox *m_pButtonBox;
};











