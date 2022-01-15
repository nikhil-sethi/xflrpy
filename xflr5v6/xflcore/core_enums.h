/****************************************************************************

    Engine enums
    Copyright (C) 2008-2018 André Deperrois

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

#pragma once


namespace xfl
{
    /** @enum The different applications available to the user */
    typedef enum {NOAPP, XFOILANALYSIS, DIRECTDESIGN, INVERSEDESIGN, MIAREX}    enumApp;
    /** This enumeration defines the options for the active view.
     *
     *May be the operaring point view, the polar view, the 3D view, the Cp view, or the stability view*/
    typedef enum {WOPPVIEW, WPOLARVIEW,  WCPVIEW, W3DVIEW, STABTIMEVIEW, STABPOLARVIEW, OTHERVIEW} enumMiarexViews;

    /** @enum The different types of analysis methods for 3D calculations of wings and planes. */
    typedef enum {LLTMETHOD, VLMMETHOD, PANEL4METHOD, TRILINMETHOD, TRIUNIMETHOD} enumAnalysisMethod;

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

    /** @enum The status of the 3d analysis */
    typedef enum {PENDING, RUNNING, CANCELLED, FINISHED} enumAnalysisStatus;

    /** @enum The different formats usable to export data to text format files*/
    typedef enum {TXT, CSV}	enumTextFileType;

    /** @enum The different line stymes */
    typedef enum {SOLID, DASH, DOT, DASHDOT, DASHDOTDOT, NOLINE} enumLineStyle;

    /**< @enum The different symbols on a curve*/
    typedef enum {NOSYMBOL, LITTLECIRCLE, BIGCIRCLE, LITTLESQUARE, BIGSQUARE, TRIANGLE, CROSS} enumPointStyle;

    /**< @enum The different image formats usable to export screen captures*/
    typedef enum {PNG, JPEG, BMP} enumImageFormat;

    /** @enum The different number of graphs in the polar view */
    typedef enum {ONEGRAPH, TWOGRAPHS, FOURGRAPHS, ALLGRAPHS, NOGRAPH} enumGraphView;

    /** This enumeration defines the options for the active Foil analysis view. */
    typedef enum {DESIGNVIEW, OPPVIEW, POLARVIEW, BLVIEW, PROFILEVIEW} enumXDirectViews;

    /** @enum The different objects selectable in the PlaneTreeView */
    typedef enum {NOOBJECT, PLANE, WPOLAR, PLANEOPP, STABILITYMODE} enumPlaneSelectionType;

    /** @enum The different objects selectable in the FoilTreeView */
    typedef enum {NONE, FOIL, POLAR, OPPOINT} enumFoilSelectionType;

    /** @enum< 0 = initial conditions, 1=forced response, 2=modal response */
    typedef enum {INITIALCONDITIONS, FORCEDRESPONSE, MODALRESPONSE} enumStabTimeResponse;

    /** used for GUI conversions to/from strings */
    typedef enum {BOOLVALUE, INTEGER, DOUBLEVALUE, STRING, PANELDISTRIBUTION, FOILNAME, BODYTYPE, FUSEDRAG,
                  POLARTYPE, ANALYSISMETHOD, REFDIMENSIONS, WINGTYPE, BOUNDARYCONDITION} enumDataType;
    /**
     * enumeration used to identify the type of surface on which the panel lies.
     * May be on a bottom, mid, top, side, or body surface.
     */
    typedef enum {BOTSURFACE, MIDSURFACE, TOPSURFACE, SIDESURFACE, BODYSURFACE, NOSURFACE} enumSurfacePosition;

}
