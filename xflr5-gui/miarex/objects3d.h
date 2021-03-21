/****************************************************************************

    Objects3D    Copyright (C) 2014-2019 Andre Deperrois

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

#include <QVector>

class Plane;
class Wing;
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
    static void      setWPolarChildrenStyle(WPolar *pWPolar);

    static Plane*    planeAt(int idx)    {if(idx<0 || idx>=s_oaPlane.size())  return nullptr; else return s_oaPlane.at(idx);}
    static WPolar*   polarAt(int idx)    {if(idx<0 || idx>=s_oaWPolar.size()) return nullptr; else return s_oaWPolar.at(idx);}
    static PlaneOpp* planeOppAt(int idx) {if(idx<0 || idx>=s_oaPOpp.size())   return nullptr; else return s_oaPOpp.at(idx);}

    static int       planeCount()   {return s_oaPlane.size();}
    static int       polarCount()    {return s_oaWPolar.size();}
    static int       planeOppCount() {return s_oaPOpp.size();}

public:
    // object variable lists

    static QVector <Plane*>    s_oaPlane;   /**< The array of void pointers to the Plane objects. */
    static QVector <Body*>     s_oaBody;    /**< The array of void pointers to the Body objects. @todo deprecated, remove*/
    static QVector <WPolar*>   s_oaWPolar;  /**< The array of void pointers to the WPolar objects. */
    static QVector <PlaneOpp*> s_oaPOpp;    /**< The array of void pointers to the PlaneOpp objects. */
};

#endif // OBJECTS3D_H
