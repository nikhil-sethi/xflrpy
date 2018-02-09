/****************************************************************************

    Plane Class
	Copyright (C) 2006-2016 Andre Deperrois adeperrois@xflr5.com

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



#ifndef PLANE_H
#define PLANE_H





/**
 *@class Plane
 *@brief
 * The class which defines the Plane object used in 3D calculations.
 *  - defines the plane object
 *  - provides the methods for the calculation of the plane's geometric properties
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds
 * Angular data is stored in degrees
*/

#include "Wing.h"
#include "Body.h"


class Plane
{
	friend class PlaneDlg;
	friend class QMiarex;

public:
	Plane();
	~Plane();

//	double VolumeMass() {return m_VolumeMass;}
	double totalMass();
	double tailVolume();

	void duplicate(Plane *pPlane);
	void computePlane(void);
	void createSurfaces();
	void renameWings();

	void clearPointMasses();
	void computeVolumeInertia(double &Mass, Vector3d &CoG, double &Ixx, double &Iyy, double &Izz, double &Ixz);
	void computeBodyAxisInertia();

	void setAutoBodyName();
	void setPlaneName(QString planeName);

	void setWings(bool bWing2, bool bStab, bool bFin);
	void setBody(Body *pBody);

	bool serializePlaneWPA(QDataStream &ar, bool bIsStoring);
	bool serializePlaneXFL(QDataStream &ar, bool bIsStoring);


	int VLMPanelTotal();

	/**
	* Returns the translation to be applied to the Body object.
	* @return the translation to be applied to the Body object.
	*/
	Vector3d &bodyPos(){ return m_BodyPos; }
	
	/**
	* Returns the leading edge, root position of a specified Wing.
	* @param iw the index of the Wing for which the LE position will be returned
	* @return the LE position of the Wing
	*/
	Vector3d &WingLE(int iw){return m_WingLE[iw];}
	
	/**
	* Returns the tilt angle of a specified Wing.
	* @param iw the index of the Wing for which the tilt angle will be returned
	* @return the LE position of the Wing
	*/
	double &WingTiltAngle(int iw){ return m_WingTiltAngle[iw];}
	
	/** Returns true if the plane has a secondary main wing, false otherwise.*/ 
	bool BiPlane(){return m_bBiplane;}
	
	/** Returns the Plane's name. */
	const QString& planeName() const {return m_PlaneName;}
	
	/** Returns a reference to the QString holding the Plane's name. */
	QString& rPlaneName() {return m_PlaneName;}

	/** Returns the Plane's description. */
	const QString& planeDescription() const {return m_PlaneDescription;}

	/** Returns a reference to the QString holding the Plane's description. */
	QString& rPlaneDescription() {return m_PlaneDescription;}


	Wing *wing(int iw);
	Wing *wing(XFLR5::enumWingType wingType);

	/** Returns a pointer to the Plane's main wing. Never NULL, a Plane always has a main Wing. */
	Wing *wing()  {return m_Wing;}

	/** Returns a pointer to the Plane's secondary wing, or NULL if none. */
	Wing *wing2() {if(m_bBiplane) return m_Wing+1; else return NULL;}

	/** Returns a pointer to the Plane's elevator, or NULL if none. */
	Wing *stab()  {if(m_bStab) return m_Wing+2; else return NULL;}

	/** Returns a pointer to the Plane's fin, or NULL if none. */
	Wing *fin()   {if(m_bFin) return m_Wing+3; else return NULL;}

	/** Returns a pointer to the Plane's Body, or NULL if none. */
	Body *body()  {if(m_bBody)    return &m_Body; else return NULL;}
	
	/** Returns the Plane's CoG position */
	Vector3d &CoG()  {return m_CoG;}

	double &CoGIxx() {return m_CoGIxx;}
	double &CoGIyy() {return m_CoGIyy;}
	double &CoGIzz() {return m_CoGIzz;}
	double &CoGIxz() {return m_CoGIxz;}


	double mac()           {return m_Wing[0].m_MAChord;}
	double span()          {return m_Wing[0].m_PlanformSpan;}
	double rootChord()     {return m_Wing[0].rootChord();}
	double tipChord()      {return m_Wing[0].tipChord();}

	double projectedArea() {return m_Wing[0].m_ProjectedArea;}
	double planformArea()  {return m_Wing[0].m_PlanformArea;}

	double projectedSpan() {return m_Wing[0].m_ProjectedSpan;}
	double planformSpan()  {return m_Wing[0].m_PlanformSpan;}

	bool isWing() {return !m_bBiplane&& !m_bStab && !m_bFin && !m_bBody;}

	double aspectRatio()  {return m_Wing[0].m_AR;}
	double taperRatio()   {return m_Wing[0].m_TR;}


	bool &hasBody()       {return m_bBody;}
	bool &hasElevator()   {return m_bStab;}
	bool &hasSecondWing() {return m_bBiplane;}
	bool &hasFin()        {return m_bFin;}

	QString bodyName()    {return m_BodyName;}

public:
	Wing m_Wing[MAXWINGS];                      /**< the array of Wing objects used to define this Plane */

private:

	Body m_Body;                                /**< the Body object */

	bool m_bBody;                               /**< true if a Body has been selected for this plane */
	bool m_bBiplane;                            /**< true if this Plane is a bi-plane */
	bool m_bFin;                                /**< true if this Plane has a fin*/
	bool m_bStab;                               /**< true if this Plane has an elevator */

//	double m_VolumeMass;                        /**< the mass of the Plane's structure, excluding point masses */
	double m_TotalMass;                         /**< the Plane's total mass, i.e. the sum of the volume mass and of the point masses */

	QString m_PlaneName;                        /**< the Plane's name; this name is used to identify the object and as a reference for child WPolar and PlaneOpp objects. */
	QString m_PlaneDescription;                 /**< a free description */
	double m_TailVolume;                        /**< the tail volume, i.e lever_arm_elev x Area_Elev / MAC_wing / Area_wing */
	Vector3d m_CoG;                              /**< the position of the CoG */

	Vector3d m_WingLE[MAXWINGS];                 /**< the array of the leading edge postion of each Wing */
	double m_WingTiltAngle[MAXWINGS];           /**< the rotation in degrees of each Wing about the y-axis */
	Vector3d m_BodyPos;                          /**< the translation vector to apply to the Body */


public:
	QString m_BodyName;                         /**< identifies this plane's body */


	QList<PointMass*> m_PointMass;              /**< the array of PointMass objects */
	double m_CoGIxx;                            /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIyy;                            /**< the Iyy component of the inertia tensor, calculated at the CoG */
	double m_CoGIzz;                            /**< the Izz component of the inertia tensor, calculated at the CoG */
	double m_CoGIxz;                            /**< the Ixz component of the inertia tensor, calculated at the CoG */

	bool m_bDoubleFin;                          /**< true if the plane has a double fin, i.e. left and right */
	bool m_bSymFin;                             /**< true if the plane has a symetric fin, i.e. top and bottom */


};

#endif
