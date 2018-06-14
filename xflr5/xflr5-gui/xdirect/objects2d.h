/****************************************************************************

	Objects2D    Copyright (C) 2016-2016 Andre Deperrois 

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

#include <QList>
#include <objects/objects2d/Foil.h>
#include <objects/objects2d/Polar.h>
#include <objects/objects2d/OpPoint.h>

class Objects2d
{
public:
	Objects2d();

	static void      deleteObjects();

	static void      deleteFoilResults(Foil *pFoil, bool bDeletePolars=false);


	static Foil *    duplicateFoil(Foil *pFoil);
	static Foil*     getFoil(QString FoilName);
	static Foil *    foil(QString strFoilName);
	static Foil*     deleteFoil(Foil *pFoil);
	static void      insertThisFoil(Foil *pFoil);
	static Foil *    addFoil(Foil *pFoil);
	static Foil *    deleteThisFoil(Foil *pFoil);
	static bool      FoilExists(QString FoilName);
	static void      renameFoil(QString FoilName);
	static void      renameThisFoil(Foil *pFoil, QString newFoilName);
	static Foil *    setModFoil(Foil *pModFoil);
	static void      setStaticPointers();

	static Polar *createPolar(Foil *pFoil, double Re, double Mach, double NCrit,
                              double XtrTop = 1.0, double XtrBot=1.0, XFLR5::enumPolarType polarType = XFLR5::FIXEDSPEEDPOLAR);
	static void      addPolar(Polar *pPolar);
	static Polar *   insertNewPolar(Polar *pModPolar, Foil *pCurFoil);
	static Polar*    getPolar(Foil *pFoil, QString PolarName);
	static Polar *   getPolar(QString m_FoilName, QString PolarName);
	static void      deletePolar(Polar *pPolar);

	static OpPoint*  getOpp(Foil *pFoil, Polar *pPolar, double Alpha);
	static OpPoint*  getFoilOpp(Foil *pFoil, Polar *pPolar, double x);
	static void      insertOpPoint(OpPoint *pNewPoint);
	static bool      deleteOpp(OpPoint *pOpp);
	static OpPoint*  addOpPoint(void *pFoilPtr, void *pPolarPtr, void *pXFoilPtr, bool bStoreOpp);

	static void addXFoilData(OpPoint *pOpp, void *pXFoilPtr, void *pFoilPtr);


public:
	// object variable lists

	static QList <Foil *> s_oaFoil;   /**< The array of void pointers to the Foil objects. */
	static QList <Polar *> s_oaPolar;  /**< The array of void pointers to the Polar objects. */
	static QList <OpPoint *> s_oaOpp;    /**< The array of void pointers to the OpPoint objects. */
};

#endif // OBJECTS2D_H
