/****************************************************************************

    Objects3D    Copyright (C) 2014-2019 André Deperrois

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


#pragma once

#include <QVector>

class Body;
class Plane;
class PlaneOpp;
class WPolar;
class Wing;
struct LineStyle;

namespace  Objects3d
{
    extern QVector <Plane*>    s_oaPlane;   /**< The array of void pointers to the Plane objects. */
    extern QVector <Body*>     s_oaBody;    /**< The array of void pointers to the Body objects. @todo deprecated, remove*/
    extern QVector <WPolar*>   s_oaWPolar;  /**< The array of void pointers to the WPolar objects. */
    extern QVector <PlaneOpp*> s_oaPOpp;    /**< The array of void pointers to the PlaneOpp objects. */

    void      addBody(Body *pBody);
    Plane *   addPlane(Plane *pPlane);
    inline void appendPlane(Plane*pPlane) {s_oaPlane.append(pPlane);}
    void      addWPolar(WPolar *pWPolar);
    inline void appendWPolar(WPolar*pWPolar) {s_oaWPolar.append(pWPolar);}
    inline void removePolarAt(int i) {if(i<0 || i>=s_oaWPolar.size()) return; s_oaWPolar.removeAt(i);}
    inline void insertPolar(int i, WPolar*pWPolar) {s_oaWPolar.insert(i, pWPolar);}
    void      deleteObjects();
    void      deletePlane(Plane *pPlane);
    void      deletePlaneResults(Plane *pPlane, bool bDeletePolars=false);
    void      deleteWPolar(WPolar *pWPolar);
    Plane *   duplicatePlane(Plane *pPlane);
    Body*     getBody(QString const &BodyName);
    Plane*    getPlane(QString const&PlaneName);
    PlaneOpp* getPlaneOpp(const Plane *pPlane, const WPolar *pWPolar, double x);
    Wing*     getWing(QString const &PlaneName);
    WPolar*   getWPolar(const Plane *pPlane, QString const &WPolarName);
    void      insertPOpp(PlaneOpp *pPOpp);
    inline void appendPlaneOpp(PlaneOpp*pPOpp) {s_oaPOpp.append(pPOpp);}
    inline void removePOppAt(int i)  {if(i<0 || i>=s_oaPOpp.size()) return; s_oaPOpp.removeAt(i);}
    WPolar *  insertNewWPolar(WPolar *pModWPolar, Plane *pCurPlane);
    bool      planeExists(QString const &planeName);
    void      renamePlane(QString const &PlaneName);
    Plane *   setModPlane(Plane *pModPlane);
    void      setWPolarChildrenStyle(WPolar *pWPolar);

    void setPlaneStyle(Plane *pPlane, LineStyle const &ls, bool bStipple, bool bWidth, bool bColor, bool bPoints);
    void setWPolarStyle(WPolar *pWPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints);
    void setWPolarPOppStyle(WPolar const* pWPolar, bool bStipple, bool bWidth, bool bColor, bool bPoints);

    void setPlaneVisible(const Plane *pPlane, bool bVisible, bool bStabilityPolarsOnly);
    void setWPolarVisible(WPolar *pWPolar, bool bVisible);
    void setWPolarColor(const Plane *pPlane, WPolar *pWPolar);

    Plane *plane(QString const &PlaneName);
    WPolar* wPolar(const Plane *pPlane, QString const &WPolarName);

    inline Plane*    planeAt(int idx)    {if(idx<0 || idx>=s_oaPlane.size())  return nullptr; else return s_oaPlane.at(idx);}
    inline WPolar*   polarAt(int idx)    {if(idx<0 || idx>=s_oaWPolar.size()) return nullptr; else return s_oaWPolar.at(idx);}
    inline PlaneOpp* planeOppAt(int idx) {if(idx<0 || idx>=s_oaPOpp.size())   return nullptr; else return s_oaPOpp.at(idx);}

    inline int planeCount()    {return s_oaPlane.size();}
    inline int polarCount()    {return s_oaWPolar.size();}
    inline int planeOppCount() {return s_oaPOpp.size();}

};


