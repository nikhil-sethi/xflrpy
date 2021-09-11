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

    friend class Miarex;

    public:
        AeroDataDlg(QWidget *parent = nullptr);

    public:
        double AirDensity();   //[kg/m3]
        double KinematicViscosity();     //[kg/m3]


    private:
        void setupLayout();

        double TemperatureCorrection(double temp);
        double AirTemperature(double Altitude);   //[K]
        double AirPressure(double Altitude);    //[Pa]
        double AirDensity(double Altitude, double temp);   //[kg/m3]
        double DynamicViscosity(double Altitude, double temp);     //[kg/m3]
        double KinematicViscosity(double Altitude , double Temp);   //[kg/m3]
        double SpeedOfSound(double temp);       //[m/s]

    private:
        void keyPressEvent(QKeyEvent *event);

    private slots:
        void onTempUnit();
        void updateResults();
        void onButton(QAbstractButton *pButton);

    private:
        static double s_Altitude;    // meters
        static double s_Temperature; // degree Kelvin
        static bool s_bCelsius;

        double UniversalGasConstant;        // [J/(mol.K)]
        double DryAirMolarMass;             // [kg/mol]
        double AdiabaticIndex;
        double SutherlandsConstant;         // [K]
        double ReferenceViscosity;          // [Pa.s] !!!!

    private:
        QComboBox *m_pcbTempUnit;

        DoubleEdit *m_pdeTemperature, *m_pdeAltitude;

        QLabel *m_plabAirDensity, *m_plabAirPressure;
        QLabel *m_plabKinematicViscosity;
        QLabel *m_plabDynamicViscosity;
        QLabel *m_plabSpeedOfSound;

        QDialogButtonBox *m_pButtonBox;
};











