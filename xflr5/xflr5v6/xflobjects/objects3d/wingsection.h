/****************************************************************************

    WingSection Class
    Copyright (C) 2013 André Deperrois 

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
 *@file This class defines the wing section object used to define wing geometries.
 */


#ifndef WINGSECTION_H
#define WINGSECTION_H



#include <QString>


#include <xflcore/core_enums.h>

/**
*@class WingSection
*@brief  The class which defines the wing section object used to construct wing geometries.
*/
class WingSection
{
public:
    WingSection()
    {
        m_NXPanels   = 0;
        m_NYPanels   = 0;
        m_XPanelDist = xfl::COSINE;
        m_YPanelDist = xfl::COSINE;

        m_Chord     = 0.0;
        m_Length    = 0.0;
        m_YPosition = 0.0;
        m_YProj     = 0.0;
        m_Offset    = 0.0;
        m_Dihedral  = 0.0;
        m_ZPos      = 0.0;
        m_Twist     = 0.0;

        m_RightFoilName.clear();
        m_LeftFoilName.clear();
    }


    QString rightFoilName() const {return m_RightFoilName;}
    QString leftFoilName()  const {return m_LeftFoilName;}

    int m_NXPanels;         /**< VLM Panels along chord, for each Wing Panel */
    int m_NYPanels;         /**< VLM Panels along span, for each Wing Panel */
    xfl::enumPanelDistribution m_XPanelDist;       /**< VLM Panel distribution type, for each Wing Panel */
    xfl::enumPanelDistribution m_YPanelDist;       /**< VLM Panel distribution type, for each Wing Panel */

    double m_Chord;         /**< Chord length at each panel side */
    double m_Length;        /**< the length of each panel */
    double m_YPosition;     /**< b-position of each panel end on developed surface */
    double m_YProj;         /**< b-position of each panel end projected on tbe xy plane */
    double m_Offset;        /**< b-position of each panel end */
    double m_Dihedral;      /**< b-position of each panel end */
    double m_ZPos;          /**< vertical offset - calculation result only */
    double m_Twist;         /**< Twist value of each foil (measured to the wing root) */

    QString m_LeftFoilName;  /**< The name of the foil on the leftt side of the section */
    QString m_RightFoilName; /**< The name of the foil on the right side of the section */
};

#endif // WINGSECTION_H
