/****************************************************************************

    Plane class
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

/** @file
*
* This file implements the class for the Plane object.
*/



#pragma once


/**
*@class Plane
*@brief
* The class which defines the Plane object used in 3D calculations.
*  - defines the plane object
*  - provides the methods for the calculation of the plane's geometric properties
* The data is stored in International Standard Units, i.e. meters, kg, and seconds
* Angular data is stored in degrees
*/

#include <xflobjects/xflobject.h>
#include <xflobjects/objects3d/wing.h>
#include <xflobjects/objects3d/body.h>

class PlaneOpp;

class Plane : public XflObject
{
    friend class PlaneDlg;
    friend class Miarex;

    public:
        Plane();
        ~Plane();

        //    double VolumeMass() {return m_VolumeMass;}
        double totalMass() const;
        double tailVolume() const;

        void duplicate(const Plane *pPlane);
        void computePlane(void);
        void createSurfaces();
        void renameWings();

        void clearPointMasses();
        void computeVolumeInertia(double &Mass, Vector3d &CoG, double &Ixx, double &Iyy, double &Izz, double &Ixz) const;
        void computeBodyAxisInertia();

        void setAutoBodyName();

        void setWings(bool bWing2, bool bStab, bool bFin);
        void setBody(Body *pBody);

        bool serializePlaneWPA(QDataStream &ar, bool bIsStoring);
        bool serializePlaneXFL(QDataStream &ar, bool bIsStoring);


        bool hasWPolar(const WPolar *pWPolar) const;
        bool hasPOpp(const PlaneOpp *pPOpp) const;

        int VLMPanelTotal();

        void setName(QString const &planeName) override;

        /**
    * Returns the translation to be applied to the Body object.
    * @return the translation to be applied to the Body object.
    */
        Vector3d const &bodyPos() const {return m_BodyPos;}
        void setBodyPos(Vector3d pos) {m_BodyPos=pos;}

        /**
    * Returns the leading edge, root position of a specified Wing.
    * @param iw the index of the Wing for which the LE position will be returned
    * @return the LE position of the Wing
    */
        Vector3d wingLE(int iw) const {return m_WingLE[iw];}
        void setWingLE(int iw, Vector3d pos) {if(iw>=0&&iw<MAXWINGS) m_WingLE[iw]=pos;}
        /**
        * Returns the tilt angle of a specified Wing.
        * @param iw the index of the Wing for which the tilt angle will be returned
        * @return the LE position of the Wing
        */
        double wingTiltAngle(int iw) const{return m_WingTiltAngle[iw];}
        void setWingTiltAngle(int iw, double ry) {if(iw>=0&&iw<MAXWINGS) m_WingTiltAngle[iw]=ry;}

        /** Returns true if the plane has a secondary main wing, false otherwise.*/
        bool biPlane() const {return m_bBiplane;}

        /** Returns the Plane's description. */
        const QString& description() const {return m_PlaneDescription;}
        void setDescription(QString desc) {m_PlaneDescription=desc;}

        Wing *mainWing() {return &m_Wing[0];}

        Wing *wing(int iw);
        Wing const *wingAt(int iw) const;
        Wing *wing(xfl::enumWingType wingType);

        /** Returns a pointer to the Plane's main wing. Never NULL, a Plane always has a main Wing. */
        Wing *wing()  {return m_Wing;}
        Wing const*wing() const {return m_Wing;}

        /** Returns a pointer to the Plane's secondary wing, or NULL if none. */
        Wing *wing2() {if(m_bBiplane) return m_Wing+1; else return nullptr;}
        Wing const*wing2() const {if(m_bBiplane) return m_Wing+1; else return nullptr;}

        /** Returns a pointer to the Plane's elevator, or NULL if none. */
        Wing *stab()  {if(m_bStab) return m_Wing+2; else return nullptr;}
        Wing const*stab() const {if(m_bStab) return m_Wing+2; else return nullptr;}

        /** Returns a pointer to the Plane's fin, or NULL if none. */
        Wing *fin()   {if(m_bFin) return m_Wing+3; else return nullptr;}
        Wing const *fin() const {if(m_bFin) return m_Wing+3; else return nullptr;}

        /** Returns a pointer to the Plane's Body, or NULL if none. */
        Body *body()  {if(m_bBody)    return &m_Body; else return nullptr;}
        Body const *body() const  {if(m_bBody)    return &m_Body; else return nullptr;}

        /** Returns the Plane's CoG position */
        Vector3d const &CoG() const  {return m_CoG;}
        void setCoG(Vector3d cg) {m_CoG=cg;}

        double CoGIxx() const {return m_CoGIxx;}
        double CoGIyy() const {return m_CoGIyy;}
        double CoGIzz() const {return m_CoGIzz;}
        double CoGIxz() const {return m_CoGIxz;}

        void setCoGIxx(double ixx) {m_CoGIxx=ixx;}
        void setCoGIyy(double iyy) {m_CoGIxx=iyy;}
        void setCoGIzz(double izz) {m_CoGIxx=izz;}
        void setCoGIxz(double ixz) {m_CoGIxx=ixz;}


        double mac()           const {return m_Wing[0].m_MAChord;}
        double span()          const {return m_Wing[0].m_PlanformSpan;}
        double rootChord()     const {return m_Wing[0].rootChord();}
        double tipChord()      const {return m_Wing[0].tipChord();}

        double projectedArea() const {return m_Wing[0].m_ProjectedArea;}
        double planformArea()  const {return m_Wing[0].m_PlanformArea;}

        double projectedSpan() const {return m_Wing[0].m_ProjectedSpan;}
        double planformSpan()  const {return m_Wing[0].m_PlanformSpan;}

        bool isWing() const {return !m_bBiplane&& !m_bStab && !m_bFin && !m_bBody;}

        double aspectRatio()  const {return m_Wing[0].m_AR;}
        double taperRatio()   const {return m_Wing[0].m_TR;}

        int spanStationCount();

        bool hasBody()       const {return m_bBody;}
        bool hasElevator()   const {return m_bStab;}
        bool hasSecondWing() const {return m_bBiplane;}
        bool hasFin()        const {return m_bFin;}

        void setBody(bool bBody)          {m_bBody=bBody;}
        void setElevator(bool bStab)      {m_bStab=bStab;}
        void setSecondWing(bool bBiPlane) {m_bBiplane=bBiPlane;}
        void setFin(bool bFin)            {m_bFin = bFin;}

        void setDoubleFin(bool b) {m_Wing[3].m_bDoubleFin=b;}
        void setSymFin(   bool b) {m_Wing[3].m_bSymFin=b;}
        bool bDoubleFin() const {return m_Wing[3].m_bDoubleFin;}
        bool bSymFin() const {return m_Wing[3].m_bSymFin;}

        QString const &bodyName()    const {return m_BodyName;}

        QString planeData(bool ) const;

    public:
        Wing m_Wing[MAXWINGS];                      /**< the array of Wing objects used to define this Plane */

    private:

        Body m_Body;                                /**< the Body object */

        bool m_bBody;                               /**< true if a Body has been selected for this plane */
        bool m_bBiplane;                            /**< true if this Plane is a bi-plane */
        bool m_bFin;                                /**< true if this Plane has a fin*/
        bool m_bStab;                               /**< true if this Plane has an elevator */

        QString m_PlaneDescription;                 /**< a free description */
        double m_TailVolume;                        /**< the tail volume, i.e lever_arm_elev x Area_Elev / MAC_wing / Area_wing */
        Vector3d m_CoG;                             /**< the position of the CoG */

        Vector3d m_WingLE[MAXWINGS];                /**< the array of the leading edge postion of each Wing */
        double m_WingTiltAngle[MAXWINGS];           /**< the rotation in degrees of each Wing about the y-axis */
        Vector3d m_BodyPos;                         /**< the translation vector to apply to the Body */


    public:
        QString m_BodyName;                         /**< identifies this plane's body */


        QVector<PointMass> m_PointMass;            /**< the array of PointMass objects @todo pointers not necessary, objects are constructed on the heap anyway*/
        double m_CoGIxx;                            /**< the Ixx component of the inertia tensor, calculated at the CoG */
        double m_CoGIyy;                            /**< the Iyy component of the inertia tensor, calculated at the CoG */
        double m_CoGIzz;                            /**< the Izz component of the inertia tensor, calculated at the CoG */
        double m_CoGIxz;                            /**< the Ixz component of the inertia tensor, calculated at the CoG */
};

