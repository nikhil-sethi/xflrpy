/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/

#include <globals/globals.h>
#include "Units.h"
#include <QGridLayout>
#include <QVBoxLayout>

double Units::s_mtoUnit  = 1.0;
double Units::s_mstoUnit = 1.0;
double Units::s_m2toUnit = 1.0;
double Units::s_kgtoUnit = 1.0;
double Units::s_NtoUnit  = 1.0;
double Units::s_NmtoUnit = 1.0;
double Units::s_PatoUnit = 1.0;
double Units::s_kgm2toUnit = 1.0;

int Units::s_LengthUnitIndex = 3;
int Units::s_SpeedUnitIndex  = 0;
int Units::s_AreaUnitIndex   = 3;
int Units::s_WeightUnitIndex = 1;
int Units::s_ForceUnitIndex  = 0;
int Units::s_MomentUnitIndex = 0;
int Units::s_PressureUnitIndex = 0;
int Units::s_InertiaUnitIndex = 0;


Units::Units(QWidget *parent): QWidget(parent)
{
	m_bLengthOnly = false;
	m_Question = tr("Select the units for this project :");
	setWindowTitle(tr("Units Dialog"));
	setupLayout();
}


void Units::setupLayout()
{
	QFont fixedWidthFont("Courier");

	QGridLayout *pUnitsLayout = new QGridLayout;
	{
		QLabel *lab1 = new QLabel(tr("Length")+":");
		QLabel *lab2 = new QLabel(tr("Area")+":");
		QLabel *lab3 = new QLabel(tr("Speed")+":");
		QLabel *lab4 = new QLabel(tr("Mass")+":");
		QLabel *lab5 = new QLabel(tr("Force")+":");
		QLabel *lab6 = new QLabel(tr("Moment")+":");
		QLabel *lab7 = new QLabel(tr("Pressure")+":");
		QLabel *lab8 = new QLabel(tr("Inertia")+":");
		lab1->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab2->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab3->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab4->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab5->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab6->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab7->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab8->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		lab1->setFont(fixedWidthFont);
		lab2->setFont(fixedWidthFont);
		lab3->setFont(fixedWidthFont);
		lab4->setFont(fixedWidthFont);
		lab5->setFont(fixedWidthFont);
		lab6->setFont(fixedWidthFont);
		lab7->setFont(fixedWidthFont);
		lab8->setFont(fixedWidthFont);
		pUnitsLayout->addWidget(lab1, 1,1);
		pUnitsLayout->addWidget(lab2, 2,1);
		pUnitsLayout->addWidget(lab3, 3,1);
		pUnitsLayout->addWidget(lab4, 4,1);
		pUnitsLayout->addWidget(lab5, 5,1);
		pUnitsLayout->addWidget(lab6, 6,1);
		pUnitsLayout->addWidget(lab7, 7,1);
		pUnitsLayout->addWidget(lab8, 8,1);

		m_pctrlQuestion = new QLabel(tr("Define the project units"));

		m_pctrlLengthFactor   = new QLabel(" ");
		m_pctrlSurfaceFactor  = new QLabel(" ");
		m_pctrlWeightFactor   = new QLabel(" ");
		m_pctrlSpeedFactor    = new QLabel(" ");
		m_pctrlForceFactor    = new QLabel(" ");
		m_pctrlMomentFactor   = new QLabel(" ");
		m_pctrlPressureFactor = new QLabel(" ");
		m_pctrlInertiaFactor  = new QLabel(" ");

		m_pctrlLengthFactor->setFont(fixedWidthFont);
		m_pctrlSurfaceFactor->setFont(fixedWidthFont);
		m_pctrlWeightFactor->setFont(fixedWidthFont);
		m_pctrlSpeedFactor->setFont(fixedWidthFont);
		m_pctrlForceFactor->setFont(fixedWidthFont);
		m_pctrlMomentFactor->setFont(fixedWidthFont);
		m_pctrlPressureFactor->setFont(fixedWidthFont);
		m_pctrlInertiaFactor->setFont(fixedWidthFont);
		m_pctrlLengthFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSurfaceFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlWeightFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSpeedFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlForceFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlMomentFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlPressureFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlInertiaFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);

		pUnitsLayout->addWidget(m_pctrlLengthFactor,   1,2);
		pUnitsLayout->addWidget(m_pctrlSurfaceFactor,  2,2);
		pUnitsLayout->addWidget(m_pctrlSpeedFactor,    3,2);
		pUnitsLayout->addWidget(m_pctrlWeightFactor,   4,2);
		pUnitsLayout->addWidget(m_pctrlForceFactor,    5,2);
		pUnitsLayout->addWidget(m_pctrlMomentFactor,   6,2);
		pUnitsLayout->addWidget(m_pctrlPressureFactor, 7,2);
		pUnitsLayout->addWidget(m_pctrlInertiaFactor,  8,2);

		m_pctrlLength    = new QComboBox;
		m_pctrlSurface   = new QComboBox;
		m_pctrlSpeed     = new QComboBox;
		m_pctrlWeight    = new QComboBox;
		m_pctrlForce     = new QComboBox;
		m_pctrlMoment    = new QComboBox;
		m_pctrlPressure  = new QComboBox;
		m_pctrlInertia   = new QComboBox;
		pUnitsLayout->addWidget(m_pctrlLength,   1,3);
		pUnitsLayout->addWidget(m_pctrlSurface,  2,3);
		pUnitsLayout->addWidget(m_pctrlSpeed,    3,3);
		pUnitsLayout->addWidget(m_pctrlWeight,   4,3);
		pUnitsLayout->addWidget(m_pctrlForce,    5,3);
		pUnitsLayout->addWidget(m_pctrlMoment,   6,3);
		pUnitsLayout->addWidget(m_pctrlPressure, 7,3);
		pUnitsLayout->addWidget(m_pctrlInertia, 8,3);


		m_pctrlLengthInvFactor  = new QLabel(" ");
		m_pctrlSurfaceInvFactor = new QLabel(" ");
		m_pctrlWeightInvFactor  = new QLabel(" ");
		m_pctrlSpeedInvFactor   = new QLabel(" ");
		m_pctrlForceInvFactor   = new QLabel(" ");
		m_pctrlMomentInvFactor  = new QLabel(" ");
		m_pctrlPressureInvFactor  = new QLabel(" ");
		m_pctrlInertiaInvFactor  = new QLabel(" ");
		m_pctrlLengthInvFactor->setFont(fixedWidthFont);
		m_pctrlSurfaceInvFactor->setFont(fixedWidthFont);
		m_pctrlWeightInvFactor->setFont(fixedWidthFont);
		m_pctrlSpeedInvFactor->setFont(fixedWidthFont);
		m_pctrlForceInvFactor->setFont(fixedWidthFont);
		m_pctrlMomentInvFactor->setFont(fixedWidthFont);
		m_pctrlPressureInvFactor->setFont(fixedWidthFont);
		m_pctrlInertiaInvFactor->setFont(fixedWidthFont);

		m_pctrlLengthInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSurfaceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlWeightInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSpeedInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlForceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlMomentInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlPressureInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlInertiaInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		pUnitsLayout->addWidget(m_pctrlLengthInvFactor, 1,4);
		pUnitsLayout->addWidget(m_pctrlSurfaceInvFactor, 2,4);
		pUnitsLayout->addWidget(m_pctrlSpeedInvFactor, 3,4);
		pUnitsLayout->addWidget(m_pctrlWeightInvFactor, 4,4);
		pUnitsLayout->addWidget(m_pctrlForceInvFactor, 5,4);
		pUnitsLayout->addWidget(m_pctrlMomentInvFactor, 6,4);
		pUnitsLayout->addWidget(m_pctrlPressureInvFactor, 7,4);
		pUnitsLayout->addWidget(m_pctrlInertiaInvFactor, 8,4);
		pUnitsLayout->setColumnStretch(4,2);
//		UnitsLayout->setColumnMinimumWidth(4,220);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(m_pctrlQuestion);
		pMainLayout->addSpacing(23);
		pMainLayout->addLayout(pUnitsLayout);
		pMainLayout->addStretch(1);

	}

	setLayout(pMainLayout);

	connect(m_pctrlLength,   SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlSurface,  SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlSpeed,    SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlWeight,   SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlForce,    SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlMoment,   SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlPressure, SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
	connect(m_pctrlInertia, SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
}


void Units::initWidget()
{
	QStringList list;
	list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
	m_pctrlLength->clear();
	m_pctrlLength->addItems(list);		//5

	m_pctrlSurface->clear();
	m_pctrlSurface->addItem(QString::fromUtf8("mm²"));		//0
	m_pctrlSurface->addItem(QString::fromUtf8("cm²"));		//1
	m_pctrlSurface->addItem(QString::fromUtf8("dm²"));		//2
	m_pctrlSurface->addItem(QString::fromUtf8("m²"));		//3
	m_pctrlSurface->addItem(QString::fromUtf8("in²"));		//4
	m_pctrlSurface->addItem(QString::fromUtf8("ft²"));		//5

	m_pctrlSpeed->clear();
	m_pctrlSpeed->addItem("m/s");       //0
	m_pctrlSpeed->addItem("km/h");      //1
	m_pctrlSpeed->addItem("ft/s");      //2
	m_pctrlSpeed->addItem("kt (int.)"); //3
	m_pctrlSpeed->addItem("mph");       //4

	m_pctrlWeight->clear();
	m_pctrlWeight->addItem("g");        //0
	m_pctrlWeight->addItem("kg");       //1
	m_pctrlWeight->addItem("oz");       //2
	m_pctrlWeight->addItem("lb");       //3

	m_pctrlForce->clear();
	m_pctrlForce->addItem("N");	        //0
	m_pctrlForce->addItem("lbf");		//1

	m_pctrlMoment->clear();
	m_pctrlMoment->addItem("N.m");	    //0
	m_pctrlMoment->addItem("lbf.in");	//1
	m_pctrlMoment->addItem("lbf.ft");	//2

	m_pctrlPressure->clear();
	m_pctrlPressure->addItem("Pa");     //0
	m_pctrlPressure->addItem("hPa");    //1
	m_pctrlPressure->addItem("kPa");    //2
	m_pctrlPressure->addItem("MPa");    //3
	m_pctrlPressure->addItem("bar");    //4
	m_pctrlPressure->addItem("psi");    //5
	m_pctrlPressure->addItem("ksi");    //6

	m_pctrlInertia->clear();
	m_pctrlInertia->addItem(QString::fromUtf8("kg.m²"));    //0
	m_pctrlInertia->addItem(QString::fromUtf8("lbm.ft²"));	//1

	m_pctrlLength->setCurrentIndex(s_LengthUnitIndex);
	m_pctrlWeight->setCurrentIndex(s_WeightUnitIndex);
	m_pctrlSurface->setCurrentIndex(s_AreaUnitIndex);
	m_pctrlSpeed->setCurrentIndex(s_SpeedUnitIndex);
	m_pctrlForce->setCurrentIndex(s_ForceUnitIndex);
	m_pctrlMoment->setCurrentIndex(s_MomentUnitIndex);
	m_pctrlPressure->setCurrentIndex(s_PressureUnitIndex);
	m_pctrlInertia->setCurrentIndex(s_InertiaUnitIndex);

	m_pctrlLength->setFocus();
	onSelChanged(" ");

	if(m_bLengthOnly)
	{
		m_pctrlSpeed->setEnabled(false);
		m_pctrlSurface->setEnabled(false);
		m_pctrlWeight->setEnabled(false);
		m_pctrlForce->setEnabled(false);
		m_pctrlMoment->setEnabled(false);
		m_pctrlPressure->setEnabled(false);
		m_pctrlInertia->setEnabled(false);
	}
	m_pctrlQuestion->setText(m_Question);
}


void Units::onSelChanged(const QString &)
{
	QString strUnitLabel, strange, strUnit;
	int len1 = 11;
	int len2 = 17;

	s_LengthUnitIndex   = m_pctrlLength->currentIndex();
	s_AreaUnitIndex     = m_pctrlSurface->currentIndex();
	s_WeightUnitIndex   = m_pctrlWeight->currentIndex();
	s_SpeedUnitIndex    = m_pctrlSpeed->currentIndex();
	s_ForceUnitIndex    = m_pctrlForce->currentIndex();
	s_MomentUnitIndex   = m_pctrlMoment->currentIndex();
	s_PressureUnitIndex = m_pctrlPressure->currentIndex();
	s_InertiaUnitIndex  = m_pctrlInertia->currentIndex();
	setUnitConversionFactors();


	getLengthUnitLabel(strUnitLabel);
	strange= QString("1 m = %1").arg(s_mtoUnit,11,'f',5);
	m_pctrlLengthFactor->setText(strange);
	strUnit = QString("%1 m").arg(1./s_mtoUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlLengthInvFactor->setText(strange);

	getAreaUnitLabel(strUnitLabel);
	strange= QString(QString::fromUtf8("1 m² = %1")).arg(s_m2toUnit,11,'f',5);
	m_pctrlSurfaceFactor->setText(strange);
	strUnit = QString::fromUtf8("%1 m²").arg(1./s_m2toUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlSurfaceInvFactor->setText(strange);

	getSpeedUnitLabel(strUnitLabel);
	strange= QString("1 m/s = %1").arg(s_mstoUnit,11,'f',5);
	m_pctrlSpeedFactor->setText(strange);
	strUnit = QString("%1 m/s").arg(1./s_mstoUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlSpeedInvFactor->setText(strange);

	getWeightUnitLabel(strUnitLabel);
	strange= QString("1 kg = %1").arg(s_kgtoUnit,11,'f',5);
	m_pctrlWeightFactor->setText(strange);
	strUnit = QString("%1 kg").arg(1./s_kgtoUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlWeightInvFactor->setText(strange);

	getForceUnitLabel(strUnitLabel);
	strange= QString("1 N = %1").arg(s_NtoUnit,11,'f',5);
	m_pctrlForceFactor->setText(strange);
	strUnit = QString("%1 N").arg(1./s_NtoUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlForceInvFactor->setText(strange);

	getMomentUnitLabel(strUnitLabel);
	strange= QString("1 N.m = %1").arg(s_NmtoUnit,11,'f',5);
	m_pctrlMomentFactor->setText(strange);
	strUnit = QString("%1 N.m").arg(1./s_NmtoUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlMomentInvFactor->setText(strange);

	strUnitLabel = pressureUnitLabel();
	strange.sprintf("1 Pa = %11.5g",s_PatoUnit);
	m_pctrlPressureFactor->setText(strange);
	strUnit.sprintf("%11.5g Pa",1./s_PatoUnit);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlPressureInvFactor->setText(strange);

	strUnitLabel = inertiaUnitLabel();
	strange= QString::fromUtf8("1 kg.m² = %1").arg(s_kgm2toUnit, 11,'f',5);
	m_pctrlInertiaFactor->setText(strange);
	strUnit = QString::fromUtf8("%1 kg.m²").arg(1./s_kgm2toUnit,11,'f',5);
	strUnitLabel = "1 "+strUnitLabel;
	strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
	m_pctrlInertiaInvFactor->setText(strange);
}




/**
 * Returns the name of the user-selected area unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the area unit
 *@param unit the index of the area unit
 */
void Units::getAreaUnitLabel(QString &str)
{
	switch(s_AreaUnitIndex)
	{
		case 0:
		{
			str="mm"+QString::fromUtf8("²");
			break;
		}
		case 1:
		{
			str="cm"+QString::fromUtf8("²");
			break;
		}
		case 2:
		{
			str="dm"+QString::fromUtf8("²");
			break;
		}
		case 3:
		{
			str="m"+QString::fromUtf8("²");
			break;
		}
		case 4:
		{
			str="in"+QString::fromUtf8("²");
			break;
		}
		case 5:
		{
			str="ft"+QString::fromUtf8("²");
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}



/**
 * Returns the name of the user-selected length unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the length unit
 *@param unit the index of the length unit
 */
void Units::getLengthUnitLabel(QString &str)
{
	switch(s_LengthUnitIndex)
	{
		case 0:
		{
			str="mm";
			break;
		}
		case 1:
		{
			str="cm";
			break;
		}
		case 2:
		{
			str="dm";
			break;
		}
		case 3:
		{
			str="m";
			break;
		}
		case 4:
		{
			str="in";
			break;
		}
		case 5:
		{
			str="ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the user-selected force unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the force unit
 *@param unit the index of the force unit
 */
void Units::getForceUnitLabel(QString &str)
{
	switch(s_ForceUnitIndex)
	{
		case 0:{
			str="N";
			break;
		}
		case 1:{
			str="lbf";
			break;
		}

		default:{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the user-selected moment unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the moment unit
 *@param unit the index of the moment unit
 */
void Units::getMomentUnitLabel(QString &str)
{
	switch(s_MomentUnitIndex)
	{
		case 0:
		{
			str="N.m";
			break;
		}
		case 1:
		{
			str="lbf.in";
			break;
		}
		case 2:
		{
			str="lbf.ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}



/**
 * Returns the name of the user-selected speed unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the speed unit
 *@param unit the index of the speed unit
 */
void Units::getSpeedUnitLabel(QString &str)
{
	switch(s_SpeedUnitIndex){
		case 0:{
			str="m/s";
			break;
		}
		case 1:{
			str="km/h";
			break;
		}
		case 2:{
			str="ft/s";
			break;
		}
		case 3:{
			str="kt";
			break;
		}
		case 4:{
			str="mph";
			break;
		}
		default:{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the user-selected mass unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the mass unit
 *@param unit the index of the mass unit
 */
void Units::getWeightUnitLabel(QString &str)
{
	switch(s_WeightUnitIndex)
	{
		case 0:{
			str="g";
			break;
		}
		case 1:{
			str="kg";
			break;
		}
		case 2:{
			str="oz";
			break;
		}
		case 3:{
			str="lb";
			break;
		}
		default:{
			str=" ";
			break;
		}
	}
}



/**
 * Returns the name of the custom pressure unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the pressure unit
 *@param unit the index of the pressure unit
 */
void Units::getPressureUnitLabel(QString &str)
{
	switch(s_PressureUnitIndex)
	{
		case 0:
		{
			str="Pa";
			break;
		}
		case 1:
		{
			str="hPa";
			break;
		}
		case 2:
		{
			str="kPa";
			break;
		}
		case 3:
		{
			str="MPa";
			break;
		}
		case 4:
		{
			str="bar";
			break;
		}
		case 5:
		{
			str="psi";
			break;
		}
		case 6:
		{
			str="ksi";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the custom pressure unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the pressure unit
 *@param unit the index of the pressure unit
 */
void Units::getInertiaUnitLabel(QString &str)
{
	switch(s_InertiaUnitIndex)
	{
		case 0:
		{
			str=QString::fromUtf8("kg.m²");
			break;
		}
		case 1:
		{
			str=QString::fromUtf8("lbm.ft²");
			break;
		}
	}
}




QString Units::lengthUnitLabel()
{
	QString str;
	getLengthUnitLabel(str);
	return str;
}

QString Units::speedUnitLabel()
{
	QString str;
	getSpeedUnitLabel(str);
	return str;
}

QString Units::weightUnitLabel()
{
	QString str;
	getWeightUnitLabel(str);
	return str;
}


QString Units::areaUnitLabel()
{
	QString str;
	getAreaUnitLabel(str);
	return str;
}


QString Units::momentUnitLabel()
{
	QString str;
	getMomentUnitLabel(str);
	return str;
}


QString Units::forceUnitLabel()
{
	QString str;
	getForceUnitLabel(str);
	return str;
}


QString Units::pressureUnitLabel()
{
	QString str;
	getPressureUnitLabel(str);
	return str;
}



QString Units::inertiaUnitLabel()
{
	QString str;
	getInertiaUnitLabel(str);
	return str;
}


/**
* Initializes the conversion factors for all user-defined units
*/
void Units::setUnitConversionFactors()
{
	switch(s_LengthUnitIndex)
	{
		case 0:{//mm
			s_mtoUnit  = 1000.0;
			break;
		}
		case 1:{//cm
			s_mtoUnit  = 100.0;
			break;
		}
		case 2:{//dm
			s_mtoUnit  = 10.0;
			break;
		}
		case 3:{//m
			s_mtoUnit  = 1.0;
			break;
		}
		case 4:{//in
			s_mtoUnit  = 1000.0/25.4;
			break;
		}
		case 5:{//ft
			s_mtoUnit  = 1000.0/25.4/12.0;
			break;
		}
		default:{//m
			s_mtoUnit  = 1.0;
			break;
		}
	}
	switch(s_AreaUnitIndex)
	{
		case 0:{//mm²
			s_m2toUnit = 1000000.0;
			break;
		}
		case 1:{//cm²
			s_m2toUnit = 10000.0;
			break;
		}
		case 2:{//dm²
			s_m2toUnit = 100.0;
			break;
		}
		case 3:{//m²
			s_m2toUnit = 1.0;
			break;
		}
		case 4:{//in²
			s_m2toUnit = 1./0.254/0.254*100.0;
			break;
		}
		case 5:{//ft²
			s_m2toUnit = 1./0.254/0.254/144.0*100.0;
			break;
		}
		default:{
			s_m2toUnit = 1.0;
			break;
		}
	}

	switch(s_WeightUnitIndex){
		case 0:{///g
			s_kgtoUnit = 1000.0;
			break;
		}
		case 1:{//kg
			s_kgtoUnit = 1.0;

			break;
		}
		case 2:{//oz
			s_kgtoUnit = 1./ 2.83495e-2;
			break;
		}
		case 3:{//lb
			s_kgtoUnit = 1.0/0.45359237;
			break;
		}
		default:{
			s_kgtoUnit = 1.0;
			break;
		}
	}

	switch(s_SpeedUnitIndex){
		case 0:{// m/s
			s_mstoUnit = 1.0;
			break;
		}
		case 1:{// km/h
			s_mstoUnit = 3600.0/1000.0;
			break;
		}
		case 2:{// ft/s
			s_mstoUnit = 100.0/2.54/12.0;
			break;
		}
		case 3:{// kt (int.)
			s_mstoUnit = 1.0/0.514444;
			break;
		}
		case 4:{// mph
			s_mstoUnit = 3600.0/1609.344;
			break;
		}
		default:{
			s_mstoUnit = 1.0;
			break;
		}
	}

	switch(s_ForceUnitIndex){
		case 0:{//N
			s_NtoUnit = 1.0;
			break;
		}
		case 1:{//lbf
			s_NtoUnit = 1.0/4.44822;
			break;
		}
		default:{
			s_NtoUnit = 1.0;
			break;
		}
	}

	switch(s_MomentUnitIndex)
	{
		case 0:{//N.m
			s_NmtoUnit = 1.0;
			break;
		}
		case 1:{//lbf.in
			s_NmtoUnit = 1.0/4.44822/0.0254;
			break;
		}
		case 2:{//lbf.0t
			s_NmtoUnit = 1.0/4.44822/12.0/0.0254;
			break;
		}
		default:{
			s_NmtoUnit = 1.0;
			break;
		}
	}

	switch(s_PressureUnitIndex)
	{
		case 0:{//Pa
			s_PatoUnit = 1.0;
			break;
		}
		case 1:{//hPa
			s_PatoUnit = 1.0/100.0;
			break;
		}
		case 2:{//kPa
			s_PatoUnit = 1.0/1000.0;
			break;
		}
		case 3:{//MPa
			s_PatoUnit = 1.0/1000000;
			break;
		}
		case 4:{//bar
			s_PatoUnit = 1.0/100000;
			break;
		}
		case 5:{//psi
			s_PatoUnit = 0.000145038;
			break;
		}
		case 6:{//ksi
			s_PatoUnit = 0.000000145038;
			break;
		}
		default:{
			s_PatoUnit = 1.0;
			break;
		}
	}

	switch(s_InertiaUnitIndex)
	{
		case 0:{//Pa
			s_kgm2toUnit = 1.0;
			break;
		}
		case 1:{//hPa
			s_kgm2toUnit = 1.0/0.45359237 * 1000.0/25.4/12.0 * 1000.0/25.4/12.0;
			break;
		}
	}
}



