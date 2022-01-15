/****************************************************************************

    Polar Class
    Copyright (C) 2003-2019 André Deperrois

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

/** 
 * @file
 * This file implements the Polar class for the 2D analysis of Foil objects
 *
 */


#pragma once


#include <QVector>

#include <xflcore/core_enums.h>
#include <xflobjects/xflobject.h>
#include <xflobjects/objects2d/oppoint.h>

class Foil;

// first name space


/**
*@brief
 * This class defines the polar object for the 2D analysis of foils
 *
    The class stores both the analysis parameters and the analysis results.
    Each instance of this class is uniquely associated to an instance of a Foil object.
*/
class Polar : public XflObject
{
    public:
        Polar();
        void addOpPointData(OpPoint const*pOpPoint);


        void addPoint(double Alpha, double Cd, double Cdp, double Cl, double Cm,
                      double Xtr1, double Xtr2, double HMom, double Cpmn, double Reynolds, double XCp);
        void exportPolar(QTextStream &out, QString versionName, bool bCSV, bool bDataOnly=false) const;
        void resetPolar();


        void copySpecification(Polar const*pPolar);
        void copyPolar(Polar const*pPolar);

        bool hasOpp(OpPoint const *pOpp) const;

        void replaceOppDataAt(int pos, const OpPoint *pOpp);
        void insertOppDataAt(int pos, OpPoint const*pOpp);
        void removePoint(int i);

        double getCm0() const;
        double getZeroLiftAngle() const;
        void getAlphaLimits(double &amin, double &amax) const;
        void getClLimits(double &Clmin, double &Clmax) const;
        void getLinearizedCl(double &Alpha0, double &slope) const;

        QString const &foilName() const {return m_FoilName;}
        QString const &polarName() const {return m_Name;}

        void setPolarType(xfl::enumPolarType type);

        void setFoilName(QString const &newFoilName) {m_FoilName = newFoilName;}
        void setPolarName(QString const &newPolarName) {m_Name = newPolarName;}

        void setAutoPolarName();

        void getProperties(QString &polarProps) const;
        QString properties() const;

        QVector<double> const &getPlrVariable(int iVar) const;




        double aoa()      const {return m_ASpec;}
        double Reynolds() const {return m_Reynolds;}
        double Mach()     const {return m_Mach;}
        double NCrit()    const {return m_NCrit;}
        double XtrTop()   const {return m_XTop;}
        double XtrBot()   const {return m_XBot;}

        void setAoa(double alpha)   {m_ASpec = alpha;}
        void setReynolds(double Re) {m_Reynolds=Re;}
        void setMach(double m)      {m_Mach=m;}
        void setNCrit(double n)     {m_NCrit=n;}
        void setXtrTop(double xtr)  {m_XTop=xtr;}
        void setXtrBot(double xtr)  {m_XBot=xtr;}

        int ReType()  const {return m_ReType;}
        int MaType()  const {return m_MaType;}
        void setReType(int type) {m_ReType=type;}
        void setMaType(int type) {m_MaType=type;}

        xfl::enumPolarType polarType() const {return m_PolarType;}

        bool isFixedSpeedPolar()  const {return m_PolarType==xfl::FIXEDSPEEDPOLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isFixedLiftPolar()   const {return m_PolarType==xfl::FIXEDLIFTPOLAR;}    /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isFixedaoaPolar()    const {return m_PolarType==xfl::FIXEDAOAPOLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isRubberChordPolar() const {return m_PolarType==xfl::RUBBERCHORDPOLAR;}


        static QString autoPolarName(xfl::enumPolarType polarType, double Re, double Mach, double NCrit, double ASpec=0.0, double XTop=1.0, double XBot=1.0);
        static QString variableName(int iVar);


    public:

        QVector<double> m_Alpha;             /**< the array of aoa values, in degrees */
        QVector<double> m_Cl;                /**< the array of lift coefficients */
        QVector<double> m_XCp;               /**< the array of centre of pressure positions */
        QVector<double> m_Cd;                /**< the array of drag coefficients */
        QVector<double> m_Cdp;               /**< the array of Cdp ? */
        QVector<double> m_Cm;                /**< the array of pitching moment coefficients */
        QVector<double> m_XTr1;              /**< the array of transition points on the top surface */
        QVector<double> m_XTr2;              /**< the array of transition points on the bottom surface */
        QVector<double> m_HMom;              /**< the array of flap hinge moments */
        QVector<double> m_Cpmn;              /**< the array of Cpmn ? */
        QVector<double> m_ClCd;              /**< the array of glide ratios */
        QVector<double> m_Cl32Cd;            /**< the array of power factors*/
        QVector<double> m_RtCl;              /**< the array of aoa values */
        QVector<double> m_Re;                /**< the array of Re coefficients */


    public:

        QString m_FoilName;                 /**< the name of the parent Foil to which this Polar object is attached */

        //Analysis specification
        xfl::enumPolarType m_PolarType;          /**< the Polar type */
        int m_ReType;                       /**< the type of Reynolds number input, cf. XFoil documentation */
        int m_MaType;                       /**< the type of Mach number input, cf. XFoil documentation */
        double m_ASpec;                     /**< the specified aoa in the case of Type 4 polars */
        double m_Mach;                      /**< the Mach number */
        double m_NCrit;                     /**< the transition criterion */
        double m_XTop;                      /**< the point of forced transition on the upper surface */
        double m_XBot;                      /**< the point of forced transition on the lower surface */
        double m_Reynolds;                  /**< the Reynolds number for a type 4 analysis */

};





