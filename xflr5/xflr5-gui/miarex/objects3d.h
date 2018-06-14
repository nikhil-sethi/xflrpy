/****************************************************************************

	Objects3D    Copyright (C) 2014-2016 Andre Deperrois 

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
  * @file This file implements the variables and methods used to manage 3D objects
  */


#ifndef OBJECTS3D_H
#define OBJECTS3D_H

#include <QList>
#include <objects/objects3d/Body.h>
#include <objects/objects3d/Plane.h>
#include <objects/objects3d/PlaneOpp.h>

class Plane;
class Body;
class WPolar;
class PlaneOpp;

class Objects3d
{
public:
    Objects3d();

	static void      addBody(Body *pBody);
	static Plane *   addPlane(Plane *pPlane);
	static void      addWPolar(WPolar *pWPolar);
	static void      deleteObjects();
	static void      deletePlane(Plane *pPlane);
	static void      deletePlaneResults(Plane *pPlane, bool bDeletePolars=false);
	static void      deleteWPolar(WPolar *pWPolar);
	static Plane *   duplicatePlane(Plane *pPlane);
	static Body*     getBody(QString BodyName);
	static Plane*    getPlane(QString PlaneName);
	static PlaneOpp* getPlaneOpp(Plane *pPlane, WPolar *pWPolar, double x);
	static Wing*     getWing(QString PlaneName);
	static WPolar*   getWPolar(Plane *pPlane, QString WPolarName);
	static void      insertPOpp(PlaneOpp *pPOpp);
	static WPolar *  insertNewWPolar(WPolar *pModWPolar, Plane *pCurPlane);
	static bool      planeExists(QString planeName);
	static void      renamePlane(QString PlaneName);
	static Plane *   setModPlane(Plane *pModPlane);
	static void      setStaticPointers();

public:
	// object variable lists

	static QList <Plane*>    s_oaPlane;   /**< The array of void pointers to the Plane objects. */
	static QList <Body*>     s_oaBody;    /**< The array of void pointers to the Body objects. */
	static QList <WPolar*>   s_oaWPolar;  /**< The array of void pointers to the WPolar objects. */
	static QList <PlaneOpp*> s_oaPOpp;    /**< The array of void pointers to the PlaneOpp objects. */
};

#endif // OBJECTS3D_H
