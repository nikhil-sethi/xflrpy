/****************************************************************************

    Units Class
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


#include <xflcore/xflcore.h>
#include "units.h"
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

        m_plabQuestion = new QLabel(tr("Define the project units"));

        m_plabLengthFactor   = new QLabel(" ");
        m_plabSurfaceFactor  = new QLabel(" ");
        m_plabWeightFactor   = new QLabel(" ");
        m_plabSpeedFactor    = new QLabel(" ");
        m_plabForceFactor    = new QLabel(" ");
        m_plabMomentFactor   = new QLabel(" ");
        m_plabPressureFactor = new QLabel(" ");
        m_plabInertiaFactor  = new QLabel(" ");

        m_plabLengthFactor->setFont(fixedWidthFont);
        m_plabSurfaceFactor->setFont(fixedWidthFont);
        m_plabWeightFactor->setFont(fixedWidthFont);
        m_plabSpeedFactor->setFont(fixedWidthFont);
        m_plabForceFactor->setFont(fixedWidthFont);
        m_plabMomentFactor->setFont(fixedWidthFont);
        m_plabPressureFactor->setFont(fixedWidthFont);
        m_plabInertiaFactor->setFont(fixedWidthFont);
        m_plabLengthFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabSurfaceFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabWeightFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabSpeedFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabForceFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabMomentFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabPressureFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabInertiaFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);

        pUnitsLayout->addWidget(m_plabLengthFactor,   1,2);
        pUnitsLayout->addWidget(m_plabSurfaceFactor,  2,2);
        pUnitsLayout->addWidget(m_plabSpeedFactor,    3,2);
        pUnitsLayout->addWidget(m_plabWeightFactor,   4,2);
        pUnitsLayout->addWidget(m_plabForceFactor,    5,2);
        pUnitsLayout->addWidget(m_plabMomentFactor,   6,2);
        pUnitsLayout->addWidget(m_plabPressureFactor, 7,2);
        pUnitsLayout->addWidget(m_plabInertiaFactor,  8,2);

        m_pcbLength    = new QComboBox;
        m_pcbSurface   = new QComboBox;
        m_pcbSpeed     = new QComboBox;
        m_pcbWeight    = new QComboBox;
        m_pcbForce     = new QComboBox;
        m_pcbMoment    = new QComboBox;
        m_pcbPressure  = new QComboBox;
        m_pcbInertia   = new QComboBox;
        pUnitsLayout->addWidget(m_pcbLength,   1,3);
        pUnitsLayout->addWidget(m_pcbSurface,  2,3);
        pUnitsLayout->addWidget(m_pcbSpeed,    3,3);
        pUnitsLayout->addWidget(m_pcbWeight,   4,3);
        pUnitsLayout->addWidget(m_pcbForce,    5,3);
        pUnitsLayout->addWidget(m_pcbMoment,   6,3);
        pUnitsLayout->addWidget(m_pcbPressure, 7,3);
        pUnitsLayout->addWidget(m_pcbInertia, 8,3);


        m_plabLengthInvFactor  = new QLabel(" ");
        m_plabSurfaceInvFactor = new QLabel(" ");
        m_plabWeightInvFactor  = new QLabel(" ");
        m_plabSpeedInvFactor   = new QLabel(" ");
        m_plabForceInvFactor   = new QLabel(" ");
        m_plabMomentInvFactor  = new QLabel(" ");
        m_plabPressureInvFactor  = new QLabel(" ");
        m_plabInertiaInvFactor  = new QLabel(" ");
        m_plabLengthInvFactor->setFont(fixedWidthFont);
        m_plabSurfaceInvFactor->setFont(fixedWidthFont);
        m_plabWeightInvFactor->setFont(fixedWidthFont);
        m_plabSpeedInvFactor->setFont(fixedWidthFont);
        m_plabForceInvFactor->setFont(fixedWidthFont);
        m_plabMomentInvFactor->setFont(fixedWidthFont);
        m_plabPressureInvFactor->setFont(fixedWidthFont);
        m_plabInertiaInvFactor->setFont(fixedWidthFont);

        m_plabLengthInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabSurfaceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabWeightInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabSpeedInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabForceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabMomentInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabPressureInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_plabInertiaInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        pUnitsLayout->addWidget(m_plabLengthInvFactor, 1,4);
        pUnitsLayout->addWidget(m_plabSurfaceInvFactor, 2,4);
        pUnitsLayout->addWidget(m_plabSpeedInvFactor, 3,4);
        pUnitsLayout->addWidget(m_plabWeightInvFactor, 4,4);
        pUnitsLayout->addWidget(m_plabForceInvFactor, 5,4);
        pUnitsLayout->addWidget(m_plabMomentInvFactor, 6,4);
        pUnitsLayout->addWidget(m_plabPressureInvFactor, 7,4);
        pUnitsLayout->addWidget(m_plabInertiaInvFactor, 8,4);
        pUnitsLayout->setColumnStretch(4,2);
//        UnitsLayout->setColumnMinimumWidth(4,220);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addSpacing(23);
        pMainLayout->addLayout(pUnitsLayout);
        pMainLayout->addStretch(1);

    }

    setLayout(pMainLayout);

    connect(m_pcbLength,   SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbSurface,  SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbSpeed,    SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbWeight,   SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbForce,    SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbMoment,   SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbPressure, SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
    connect(m_pcbInertia, SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
}


void Units::initWidget()
{
    QStringList list;
    list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
    m_pcbLength->clear();
    m_pcbLength->addItems(list);        //5

    m_pcbSurface->clear();
    m_pcbSurface->addItem(QString::fromUtf8("mm²"));        //0
    m_pcbSurface->addItem(QString::fromUtf8("cm²"));        //1
    m_pcbSurface->addItem(QString::fromUtf8("dm²"));        //2
    m_pcbSurface->addItem(QString::fromUtf8("m²"));        //3
    m_pcbSurface->addItem(QString::fromUtf8("in²"));        //4
    m_pcbSurface->addItem(QString::fromUtf8("ft²"));        //5

    m_pcbSpeed->clear();
    m_pcbSpeed->addItem("m/s");       //0
    m_pcbSpeed->addItem("km/h");      //1
    m_pcbSpeed->addItem("ft/s");      //2
    m_pcbSpeed->addItem("kt (int.)"); //3
    m_pcbSpeed->addItem("mph");       //4

    m_pcbWeight->clear();
    m_pcbWeight->addItem("g");        //0
    m_pcbWeight->addItem("kg");       //1
    m_pcbWeight->addItem("oz");       //2
    m_pcbWeight->addItem("lb");       //3

    m_pcbForce->clear();
    m_pcbForce->addItem("N");            //0
    m_pcbForce->addItem("lbf");        //1

    m_pcbMoment->clear();
    m_pcbMoment->addItem("N.m");        //0
    m_pcbMoment->addItem("lbf.in");    //1
    m_pcbMoment->addItem("lbf.ft");    //2

    m_pcbPressure->clear();
    m_pcbPressure->addItem("Pa");     //0
    m_pcbPressure->addItem("hPa");    //1
    m_pcbPressure->addItem("kPa");    //2
    m_pcbPressure->addItem("MPa");    //3
    m_pcbPressure->addItem("bar");    //4
    m_pcbPressure->addItem("psi");    //5
    m_pcbPressure->addItem("ksi");    //6

    m_pcbInertia->clear();
    m_pcbInertia->addItem(QString::fromUtf8("kg.m²"));    //0
    m_pcbInertia->addItem(QString::fromUtf8("lbm.ft²"));    //1

    m_pcbLength->setCurrentIndex(s_LengthUnitIndex);
    m_pcbWeight->setCurrentIndex(s_WeightUnitIndex);
    m_pcbSurface->setCurrentIndex(s_AreaUnitIndex);
    m_pcbSpeed->setCurrentIndex(s_SpeedUnitIndex);
    m_pcbForce->setCurrentIndex(s_ForceUnitIndex);
    m_pcbMoment->setCurrentIndex(s_MomentUnitIndex);
    m_pcbPressure->setCurrentIndex(s_PressureUnitIndex);
    m_pcbInertia->setCurrentIndex(s_InertiaUnitIndex);

    m_pcbLength->setFocus();
    onSelChanged(" ");

    if(m_bLengthOnly)
    {
        m_pcbSpeed->setEnabled(false);
        m_pcbSurface->setEnabled(false);
        m_pcbWeight->setEnabled(false);
        m_pcbForce->setEnabled(false);
        m_pcbMoment->setEnabled(false);
        m_pcbPressure->setEnabled(false);
        m_pcbInertia->setEnabled(false);
    }
    m_plabQuestion->setText(m_Question);
}


void Units::onSelChanged(const QString &)
{
    QString strUnitLabel, strange, strUnit;
    int len1 = 11;
    int len2 = 17;

    s_LengthUnitIndex   = m_pcbLength->currentIndex();
    s_AreaUnitIndex     = m_pcbSurface->currentIndex();
    s_WeightUnitIndex   = m_pcbWeight->currentIndex();
    s_SpeedUnitIndex    = m_pcbSpeed->currentIndex();
    s_ForceUnitIndex    = m_pcbForce->currentIndex();
    s_MomentUnitIndex   = m_pcbMoment->currentIndex();
    s_PressureUnitIndex = m_pcbPressure->currentIndex();
    s_InertiaUnitIndex  = m_pcbInertia->currentIndex();
    setUnitConversionFactors();


    getLengthUnitLabel(strUnitLabel);
    strange= QString("1 m = %1").arg(s_mtoUnit,11,'f',5);
    m_plabLengthFactor->setText(strange);
    strUnit = QString("%1 m").arg(1./s_mtoUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabLengthInvFactor->setText(strange);

    getAreaUnitLabel(strUnitLabel);
    strange= QString(QString::fromUtf8("1 m² = %1")).arg(s_m2toUnit,11,'f',5);
    m_plabSurfaceFactor->setText(strange);
    strUnit = QString::fromUtf8("%1 m²").arg(1./s_m2toUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabSurfaceInvFactor->setText(strange);

    getSpeedUnitLabel(strUnitLabel);
    strange= QString("1 m/s = %1").arg(s_mstoUnit,11,'f',5);
    m_plabSpeedFactor->setText(strange);
    strUnit = QString("%1 m/s").arg(1./s_mstoUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabSpeedInvFactor->setText(strange);

    getMassUnitLabel(strUnitLabel);
    strange= QString("1 kg = %1").arg(s_kgtoUnit,11,'f',5);
    m_plabWeightFactor->setText(strange);
    strUnit = QString("%1 kg").arg(1./s_kgtoUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabWeightInvFactor->setText(strange);

    getForceUnitLabel(strUnitLabel);
    strange= QString("1 N = %1").arg(s_NtoUnit,11,'f',5);
    m_plabForceFactor->setText(strange);
    strUnit = QString("%1 N").arg(1./s_NtoUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabForceInvFactor->setText(strange);

    getMomentUnitLabel(strUnitLabel);
    strange= QString("1 N.m = %1").arg(s_NmtoUnit,11,'f',5);
    m_plabMomentFactor->setText(strange);
    strUnit = QString("%1 N.m").arg(1./s_NmtoUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabMomentInvFactor->setText(strange);

    strUnitLabel = pressureUnitLabel();
    strange = QString::asprintf("1 Pa = %11.5g",s_PatoUnit);
    m_plabPressureFactor->setText(strange);
    strUnit = QString::asprintf("%11.5g Pa",1./s_PatoUnit);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabPressureInvFactor->setText(strange);

    strUnitLabel = inertiaUnitLabel();
    strange= QString::fromUtf8("1 kg.m² = %1").arg(s_kgm2toUnit, 11,'f',5);
    m_plabInertiaFactor->setText(strange);
    strUnit = QString::fromUtf8("%1 kg.m²").arg(1./s_kgm2toUnit,11,'f',5);
    strUnitLabel = "1 "+strUnitLabel;
    strange= strUnitLabel.rightJustified(len1) +" = " + strUnit.leftJustified(len2);
    m_plabInertiaInvFactor->setText(strange);
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
void Units::getMassUnitLabel(QString &str)
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

QString Units::massUnitLabel()
{
    QString str;
    getMassUnitLabel(str);
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


void Units::loadSettings(QSettings &settings)
{
    settings.beginGroup("Units");
    {
        setLengthUnitIndex(  settings.value("LengthUnit").toInt());
        setAreaUnitIndex(    settings.value("AreaUnit").toInt());
        setWeightUnitIndex(  settings.value("WeightUnit").toInt());
        setSpeedUnitIndex(   settings.value("SpeedUnit").toInt());
        setForceUnitIndex(   settings.value("ForceUnit").toInt());
        setMomentUnitIndex(  settings.value("MomentUnit").toInt());
        setPressureUnitIndex(settings.value("PressureUnit").toInt());
        setInertiaUnitIndex( settings.value("InertiaUnit").toInt());

        setUnitConversionFactors();
    }
    settings.endGroup();
}


void Units::saveSettings(QSettings &settings)
{
    settings.beginGroup("Units");
    {
        settings.setValue("LengthUnit",   lengthUnitIndex());
        settings.setValue("AreaUnit",     areaUnitIndex());
        settings.setValue("WeightUnit",   weightUnitIndex());
        settings.setValue("SpeedUnit",    speedUnitIndex());
        settings.setValue("ForceUnit",    forceUnitIndex());
        settings.setValue("MomentUnit",   momentUnitIndex());
        settings.setValue("PressureUnit", pressureUnitIndex());
        settings.setValue("InertiaUnit",  inertiaUnitIndex());

    }
    settings.endGroup();
}

