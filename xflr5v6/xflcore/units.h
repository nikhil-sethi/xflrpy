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


#pragma once

#include <QComboBox>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QSettings>

class Units : public QWidget
{
    Q_OBJECT

    public:
        Units(QWidget *parent);
        void initWidget();

        static void getLengthUnitLabel(  QString &str);
        static void getSpeedUnitLabel(   QString &str);
        static void getMassUnitLabel(    QString &str);
        static void getAreaUnitLabel(    QString &str);
        static void getMomentUnitLabel(  QString &str);
        static void getForceUnitLabel(   QString &str);
        static void getPressureUnitLabel(QString &str);
        static void getInertiaUnitLabel( QString &str);

        static QString lengthUnitLabel();
        static QString speedUnitLabel();
        static QString massUnitLabel();
        static QString areaUnitLabel();
        static QString momentUnitLabel();
        static QString forceUnitLabel();
        static QString pressureUnitLabel();
        static QString inertiaUnitLabel();

        static double mtoUnit()    {return s_mtoUnit;}
        static double mstoUnit()   {return s_mstoUnit;}
        static double m2toUnit()   {return s_m2toUnit;}
        static double kgtoUnit()   {return s_kgtoUnit;}
        static double NtoUnit()    {return s_NtoUnit;}
        static double NmtoUnit()   {return s_NmtoUnit;}
        static double PatoUnit()   {return s_PatoUnit;}
        static double kgm2toUnit() {return s_kgm2toUnit;}

        static int lengthUnitIndex()   {return s_LengthUnitIndex;}
        static int areaUnitIndex()     {return s_AreaUnitIndex;}
        static int weightUnitIndex()   {return s_WeightUnitIndex;}
        static int speedUnitIndex()    {return s_SpeedUnitIndex;}
        static int forceUnitIndex()    {return s_ForceUnitIndex;}
        static int momentUnitIndex()   {return s_MomentUnitIndex;}
        static int pressureUnitIndex() {return s_PressureUnitIndex;}
        static int inertiaUnitIndex()  {return s_InertiaUnitIndex;}

        static void setLengthUnitIndex(int index)   {s_LengthUnitIndex    = index;}
        static void setAreaUnitIndex(int index)     {s_AreaUnitIndex      = index;}
        static void setWeightUnitIndex(int index)   {s_WeightUnitIndex    = index;}
        static void setSpeedUnitIndex(int index)    {s_SpeedUnitIndex     = index;}
        static void setForceUnitIndex(int index)    {s_ForceUnitIndex     = index;}
        static void setMomentUnitIndex(int index)   {s_MomentUnitIndex    = index;}
        static void setPressureUnitIndex(int index) {s_PressureUnitIndex  = index;}
        static void setInertiaUnitIndex(int index)  { s_InertiaUnitIndex  = index;}

        static void setUnitConversionFactors();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private slots:
        void onSelChanged(const QString &);

    private:
        QComboBox  *m_pcbMoment;
        QComboBox  *m_pcbSurface;
        QComboBox  *m_pcbWeight;
        QComboBox  *m_pcbSpeed;
        QComboBox  *m_pcbLength;
        QComboBox  *m_pcbForce;
        QComboBox  *m_pcbPressure;
        QComboBox  *m_pcbInertia;
        QLabel *m_plabForceFactor,    *m_plabForceInvFactor;
        QLabel *m_plabLengthFactor,   *m_plabLengthInvFactor;
        QLabel *m_plabSpeedFactor,    *m_plabSpeedInvFactor;
        QLabel *m_plabSurfaceFactor,  *m_plabSurfaceInvFactor;
        QLabel *m_plabWeightFactor,   *m_plabWeightInvFactor;
        QLabel *m_plabMomentFactor,   *m_plabMomentInvFactor;
        QLabel *m_plabPressureFactor, *m_plabPressureInvFactor;
        QLabel *m_plabInertiaFactor,  *m_plabInertiaInvFactor;

        QLabel *m_plabQuestion;

    private:
        void setupLayout();

        bool m_bLengthOnly;
        static int s_LengthUnitIndex;    /**< The index of the custom unit in the array of length units. @todo use an enumeration instead. */
        static int s_AreaUnitIndex;      /**< The index of the custom unit in the array of area units. */
        static int s_WeightUnitIndex;    /**< The index of the custom unit in the array of mass units. */
        static int s_MomentUnitIndex;    /**< The index of the custom unit in the array of moment units. */
        static int s_SpeedUnitIndex;     /**< The index of the custom unit in the array of speed units. */
        static int s_ForceUnitIndex;     /**< The index of the custom unit in the array of force units. */
        static int s_PressureUnitIndex;  /**< The index of the custom unit in the array of pressure units. */
        static int s_InertiaUnitIndex;   /**< The index of the custom unit in the array of inertai units. */

        static double s_mtoUnit;    /**< Conversion factor from meters to the custom length unit. */
        static double s_mstoUnit;   /**< Conversion factor from m/s to the custom speed unit. */
        static double s_m2toUnit;   /**< Conversion factor from square meters to the custom area unit. */
        static double s_kgtoUnit;   /**< Conversion factor from kg to the custom mass unit. */
        static double s_NtoUnit;    /**< Conversion factor from Newtons to the custom force unit. */
        static double s_NmtoUnit;   /**< Conversion factor from N.m to the custom unit for moments. */
        static double s_PatoUnit;   /**< Conversion factor from Pascal to the custom unit for pressures. */
        static double s_kgm2toUnit; /**< Conversion factor from kg.m² to custom unit for inertias */
    public:

        QString m_Question;
};

