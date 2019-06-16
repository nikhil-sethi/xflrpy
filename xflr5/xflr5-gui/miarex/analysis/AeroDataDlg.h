/****************************************************************************

    AeroData Class
	Copyright (C) 2015 Andre Deperrois 

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

#ifndef AERODATADLG_H
#define AERODATADLG_H



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
	AeroDataDlg(QWidget *parent = NULL);

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
	void OnTempUnit();
	void updateResults();


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
	QComboBox *m_pctrlTempUnit;
	QPushButton *m_pOKButton, *m_pCancelButton;
	DoubleEdit *m_pctrlTemperature, *m_pctrlAltitude;

	QLabel *m_pctrlAirDensity, *m_pctrlAirPressure;
	QLabel *m_pctrlKinematicViscosity;
	QLabel *m_pctrlDynamicViscosity;
	QLabel *m_pctrlSpeedOfSound;
};

#endif // AERODATADLG_H









