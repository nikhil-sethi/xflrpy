/****************************************************************************

    OpPoint Class
    Copyright (C) 2003 André Deperrois 

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
 *@file
 *
 * This class implements the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 *
 */


#pragma once

#include <QString>
#include <QTextStream>
#include <QDataStream>


#include <xfoil_params.h>
#include <xflobjects/objects2d/blxfoil.h>
#include <xflobjects/xflobject.h>

class Foil;
class Polar;

/**
*@class OpPoint
*@brief
 * The class which defines the operating point associated to Foil objects.

An OpPoint object stores the results of an XFoil calculation.
Each instance of an OpPoint is uniquely attached to a Polar object, which is itself attached uniquely to a Foil object.
The identification of the parent Polar and Foil are made using the QString names of the objects.
*/
class OpPoint : public XflObject
{
    friend class Polar;
    friend class XDirect;
    friend class OpPointWt;

    public:
        OpPoint();

        void setHingeMoments(const Foil *pFoil);

        void exportOpp(QTextStream &out, QString Version, bool bCSV, Foil *pFoil, bool bDataOnly=false) const;

        bool serializeOppWPA(QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
        bool serializeOppXFL(QDataStream &ar, bool bIsStoring);


        QString const &foilName()  const {return m_FoilName;}
        QString const &polarName() const {return m_PlrName;}
        QString opPointName() const;

        void setFoilName(QString const &newFoilName) {m_FoilName = newFoilName;}
        void setPolarName(QString const&newPlrName) {m_PlrName = newPlrName;}

        bool bViscResults() const {return m_bViscResults;}
        bool bBL()          const {return m_bBL;}


        double aoa()      const {return m_Alpha;}
        double Reynolds() const {return m_Reynolds;}
        double Mach()     const {return m_Mach; }

        QString properties(Foil *pFoil, bool bData=false) const;
        void getOppProperties(QString &OpPointProperties, Foil *pFoil, bool bData=false) const;


        static bool bStoreOpp() {return s_bStoreOpp;}
        static void setStoreOpp(bool b) {s_bStoreOpp=b;}

    public:
        bool m_bViscResults;        /**< true if viscous results are stored in this OpPoint */
        bool m_bBL;                 /**< true if a boundary layer is stored in this OpPoint */
        bool m_bTEFlap;             /**< true if the parent foil has a flap on the trailing edge */
        bool m_bLEFlap;             /**< true if the parent foil has a flap on the leading edge */

        int m_n;                    /**< the number of foil surface points */

        double m_Reynolds;          /**< the Re number of the OpPoint */
        double m_Mach;              /**< the Mach number of the OpPoint */
        double m_Alpha;             /**< the aoa*/
        double Cl;                  /**< the lift coefficient */
        double Cm;                  /**< the pitching moment coefficient */
        double Cd;                  /**< the drag coefficient - viscous only, since we are dealing with 2D analysis */
        double Cdp;                 /**< @todo check significance in XFoil doc */
        double Xtr1;                /**< the laminar to turbulent transition point on the upper surface */
        double Xtr2;                /**< the laminar to turbulent transition point on the lower surface */
        double ACrit;               /**< the NCrit parameter which defines turbulent transition */
        double m_XCP;               /**< the x-position of the centre of pressure */

        double Cpv[IQX];            /**< the distribution of Cp on the surfaces for a viscous analysis */
        double Cpi[IQX];            /**< the distribution of Cp on the surfaces for an inviscid analysis */
        double Qv[IQX];             /**< the distribution of stream velocity on the surfaces for a viscous analysis */
        double Qi[IQX];             /**< the distribution of stream velocity on the surfaces for an inviscid analysis */

        double m_TEHMom;            /**< the moment on the foil's trailing edge flap */
        double m_LEHMom;            /**< the moment on the foil's leading edge flap */
        double XForce;              /**< the y-component of the pressure forces */
        double YForce;              /**< the y-component of the pressure forces */
        double Cpmn;                /**< @todo check significance in XFoil doc */

        BLXFoil blx;                /**< BL data from an XFoil analysis */


        QString m_FoilName;        /**< the name of the parent Foil */
        QString m_PlrName;         /**< the name of the parent Polar */


        static bool s_bStoreOpp;          /**< true if the operating points should be stored; */

};

