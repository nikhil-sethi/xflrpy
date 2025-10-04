/****************************************************************************

    Global Functions 
    Copyright (C) 2008-2016 André Deperrois 

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
* This file contains the declaration of methods used throughout the program and not specific to one application.
*/

#ifndef ENGINE_GLOBALS_H
#define ENGINE_GLOBALS_H


#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects2d/foil.h>


using namespace std;


double GetPlrPointFromCl(Foil *pFoil, double Re, double Cl, int PlrVar, bool &bOutRe, bool &bError);
double GetPlrPointFromAlpha(Foil *pFoil, double Re, double Alpha, int PlrVar, bool &bOutRe, bool &bError);

double splineBlend(int const &index, int const &p, double const &t, double *knots);



#endif // ENGINE_GLOBALS_H
 
