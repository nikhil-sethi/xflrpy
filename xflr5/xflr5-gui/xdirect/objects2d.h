/****************************************************************************

    Objects2D    Copyright (C) 2016-2019 Andre Deperrois

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
#ifndef OBJECTS2D_H
#define OBJECTS2D_H

/**
  * @file This file implements the variables and methods used to manage 3D objects
  */

#include <QVector>

#include <analysis3d/analysis3d_enums.h>


class Foil;
class Polar;
class OpPoint;

class Objects2d
{
public:
    Objects2d();

    static void      deleteObjects();

    static void      deleteFoilResults(Foil *pFoil, bool bDeletePolars=false);


    static Foil*     duplicateFoil(Foil *pFoil);
    static Foil*     foil(QString strFoilName);
    static Foil*     foilAt(int iFoil);
    static Foil*     deleteFoil(Foil *pFoil);
    static Foil*     deleteThisFoil(Foil *pFoil);
    static void      deleteAllFoils();
    static void      insertThisFoil(Foil *pFoil);
    static Foil *    addFoil(Foil *pFoil);
    static void      appendFoil(Foil *pFoil) {s_oaFoil.append(pFoil);}
    static bool      FoilExists(QString FoilName);
    static void      renameFoil(QString FoilName);
    static void      renameThisFoil(Foil *pFoil, QString newFoilName);
    static Foil *    setModFoil(Foil *pModFoil);
    static void      setStaticPointers();

    static void      addPolar(Polar *pPolar);
    static void      appendPolar(Polar *pPolar) {s_oaPolar.append(pPolar);}
    static Polar*    insertNewPolar(Polar *pModPolar, Foil *pCurFoil);
    static Polar*    getPolar(Foil *pFoil, QString PolarName);
    static Polar*    getPolar(QString m_FoilName, QString PolarName);
    static Polar*    polarAt(int index);
    static void      deletePolar(Polar *pPolar);
    static void      deletePolarAt(int index);

    static OpPoint*  oppAt(int index);
    static OpPoint*  getOpp(Foil *pFoil, Polar *pPolar, double Alpha);
    static OpPoint*  getFoilOpp(Foil *pFoil, Polar *pPolar, double x);
    static void      insertOpPoint(OpPoint *pNewPoint);
    static void      appendOpp(OpPoint*pOpp) {s_oaOpp.append(pOpp);}
    static bool      deleteOpp(OpPoint *pOpp);
    static void      deleteOppAt(int index);
    static OpPoint*  addOpPoint(Foil *pFoil, Polar *pPolar, OpPoint *pOpPoint, bool bStoreOpp);

    static int foilCount() {return s_oaFoil.size();}
    static int polarCount() {return s_oaPolar.size();}
    static int oppCount() {return s_oaOpp.size();}

    static void setFoilChildrenStyle(Foil *pFoil);
    static void setPolarChildrenStyle(Polar *pPolar);

    static QVector<Foil*> * pOAFoil() {return &s_oaFoil;}
    static QVector<Polar*> * pOAPolar() {return &s_oaPolar;}
    static QVector<OpPoint*> * pOAOpp() {return &s_oaOpp;}

private:
    // object arrays
    static QVector<Foil *> s_oaFoil;   /**< The array of pointers to the Foil objects. */
    static QVector<Polar *> s_oaPolar;  /**< The array of pointers to the Polar objects. */
    static QVector<OpPoint *> s_oaOpp;    /**< The array of pointers to the OpPoint objects. */
};

#endif // OBJECTS2D_H
