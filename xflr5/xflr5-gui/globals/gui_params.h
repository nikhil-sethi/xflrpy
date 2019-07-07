/****************************************************************************

    GUI_PARAMS

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


/**
*@file
*
* This files defines the values of the main constant parameters used throughout the program.
*
* A modification of this file triggers the compilation of the whole project.
* 
*/

#ifndef GUI_PARAMS_H
#define GUI_PARAMS_H


#define VERSIONNAME     "xflr5 v6.47"

#define MAJOR_VERSION    6
#define MINOR_VERSION    47


//General
#define MAXRECENTFILES         8  /**< Defines the maximum number of file names in the recent file list */
#define SETTINGSFORMAT     53773  /**< A random number which defines the format of the settings file */



#define QUESTION (BB || !BB) /**< Shakespeare */
#define NOWIND NOFUN         /**< techwinder */

#endif // GUI_PARAMS_H
 
