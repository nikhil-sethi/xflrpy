/****************************************************************************

    GUI_enums functions

    Copyright (C) 2008-2019 Andre Deperrois

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


#ifndef GUI_ENUMS_H
#define GUI_ENUMS_H

// first name space
namespace XFLR5
{
	/** @enum The different applications available to the user */
	typedef enum {NOAPP, XFOILANALYSIS, DIRECTDESIGN, INVERSEDESIGN, MIAREX, OPENGL3}	enumApp;

	/**< @enum The different image formats usable to export screen captures*/
	typedef enum {PNG, JPEG, BMP}	enumImageFormat;

	/** @enum The different number of graphs in the polar view */
	typedef enum {ONEGRAPH, TWOGRAPHS, FOURGRAPHS, ALLGRAPHS} enumGraphView;

	/** This enumeration defines the options for the active view.
	 *May be the operaring point view, the polar view, the 3D view, the Cp view, or the stability view*/
	typedef enum {WOPPVIEW, WPOLARVIEW,  WCPVIEW, W3DVIEW, STABTIMEVIEW, STABPOLARVIEW, OTHERVIEW} enumMiarexViews;

	typedef enum {BOOLVALUE, INTEGER, DOUBLEVALUE, STRING, PANELDISTRIBUTION, FOILNAME, BODYTYPE, POLARTYPE, ANALYSISMETHOD, REFDIMENSIONS, WINGTYPE, BOUNDARYCONDITION} enumDataType;

	/** @enum The different formats usable to export data to text format files*/
	typedef enum {TXT, CSV}	enumTextFileType;
}

#endif // GUI_ENUMS_H

