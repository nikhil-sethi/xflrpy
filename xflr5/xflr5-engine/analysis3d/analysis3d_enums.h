/****************************************************************************

    Engine enums
    Copyright (C) 2008-2018 Andre Deperrois 

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

#ifndef ENGINE_ENUMS_H
#define ENGINE_ENUMS_H

// first name space
namespace XFLR5
{
    /** @enum The different types of analysis methods for 3D calculations of wings and planes. */
    typedef enum {LLTMETHOD, VLMMETHOD, PANEL4METHOD, TRILINMETHOD, TRICSTMETHOD} enumAnalysisMethod;

    /** @enum The different types of polar available for 2D and 3D calculations. */
    typedef enum {FIXEDSPEEDPOLAR, FIXEDLIFTPOLAR, RUBBERCHORDPOLAR, FIXEDAOAPOLAR, STABILITYPOLAR, BETAPOLAR} enumPolarType;

    /** @enum The different type of reference areaand length */
    typedef enum {PLANFORMREFDIM, PROJECTEDREFDIM, MANUALREFDIM} enumRefDimension;

    /** @enum The two types of body : flat panels or NURBS*/
    typedef enum  {BODYPANELTYPE, BODYSPLINETYPE }     enumBodyLineType;

    /** @enum The 4 types of wing */
    typedef enum {MAINWING, SECONDWING, ELEVATOR, FIN, OTHERWING} enumWingType;

    /** @enum The different types of panel distribution on the wing */
    typedef enum {COSINE, UNIFORM, SINE, INVERSESINE} enumPanelDistribution;

    /** @enum The two types of boundary conditions for 3D panel analysis. VLM BC are of the NEUMANN type*/
    typedef enum {DIRICHLET, NEUMANN} enumBC;
}

#endif // ENGINE_ENUMS_H

