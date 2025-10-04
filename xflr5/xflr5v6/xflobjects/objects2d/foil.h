/****************************************************************************

    Reference Foil Class
    Copyright (C) 2003-2016 André Deperrois 

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
 * This class defines the foil object used in 2d and 3d calculations
 *
 */



#pragma once

#include <QTextStream>
#include <QColor>

#include <xflgeom/geom2d/vector2d.h>
#include <xflobjects/xflobject.h>
#include <xfoil_params.h>

#define MIDPOINTCOUNT 1000

/**
*@class Foil
*@brief
*    The class which defines the Foil object used in 2D and 3D calculations.

The class stores two geometries:
    - the base foil, which unless advised otherwise is unchanged from the moment it has been first loaded or created
    - the current foil, on which the geometrical modifications are applied
       such as flap deflection, camber and thickness scaling
@todo One of the very early classes in this project. Would need a general revision.
Also it mixes the construction methods and the GUI; would be better to move the GUI to a derived child class for polymorphism.
*/
class Foil : public XflObject
{
    public:
        Foil();

        int isPoint(Vector2d const &Real) const;

        void getLowerY(double x, double &y, double &normx, double &normy) const;
        void getUpperY(double x, double &y, double &normx, double &normy) const;

        double deRotate();
        double baseUpperY(double x) const;
        double baseLowerY(double x) const;
        Vector2d midYRel(double sRel) const;
        Vector2d lowerYRel(double xRel, double &normx, double &normy) const;
        Vector2d upperYRel(double xRel, double &normx, double &normy) const;
        double camber(double x) const;
        double camberSlope(double x) const;
        double length() const;
        double area() const;
        double topSlope(double x) const;
        double bottomSlope(double x) const;
        double normalizeGeometry();
        void compMidLine(bool bParams);

        bool exportFoil(QTextStream &out) const;
        bool initFoil();

        void copyFoil(Foil const *pSrcFoil, bool bMetaData=true);

        void setFlap();
        void setTEFlap();
        void setLEFlap();
        void setNaca009();
        void setLEFlapData(bool bFlap, double xhinge, double yhinge, double angle);
        void setTEFlapData(bool bFlap, double xhinge, double yhinge, double angle);

        bool intersect(Vector2d const &A, Vector2d const &B, Vector2d const &C, Vector2d const &D, Vector2d *M) const;

        bool bCenterLine() const {return m_bCenterLine;}
        void showCenterLine(bool bShow) {m_bCenterLine=bShow;}

        void setEditStyle();

        QString const &description() const {return m_FoilDescription;}
        void setDescription(QString const &description) {m_FoilDescription=description;}

        double camber()     const {return m_fCamber;}
        double xCamber()    const {return m_fXCamber;}
        double thickness()  const {return m_fThickness;}
        double xThickness() const {return m_fXThickness;}
        double TEGap() const {return m_TEGap;}

        int iHighLight() const {return m_iHighLight;}
        void setHighLight(int iH) {m_iHighLight = iH;}

        void displayCoords(bool bBaseCoords=false) const;

        QString properties() const;

    public:
        // Base geometry;
       int m_nb;                              /**< the number of points of the base foil */
       double m_xb[IBX];                      /**< the array of x-coordinates of the base foil points */
       double m_yb[IBX];                      /**< the array of y-coordinates of the base foil points*/
       int m_n;                               /**<  the number of points of the current foil */
       double m_x[IBX];                       /**< the array of x-coordinates of the current foil points */
       double m_y[IBX];                       /**< the array of y-coordinates of the current foil points*/

       double m_nx[IBX];                      /**< the array of x-coordinates of the current foil normal Vector2ds*/
       double m_ny[IBX];                      /**< the array of x-coordinates of the current foil normal Vector2ds*/
       Vector2d m_rpMid[MIDPOINTCOUNT];              /**< the mid camber line points */


    public:
        QString m_FoilDescription;             /**< a free description */

        bool m_bCenterLine;                  /**< true if the foil mid camber line is to be displayed */

        int m_iBaseInt;                      /**< the number of points on the lower surface of the base foil */
        int m_iBaseExt;                      /**< the number of points on the upper surface of the base foil */

        int m_iInt;                          /**< the number of points on the lower surface of the current foil */
        int m_iExt;                          /**< the number of points on the upper surface of the current foil */

        int m_iHighLight;                    /**< the index of the point to highlight in the display */


        double m_fCamber;                    /**< the Foil's max camber */
        double m_fThickness;                 /**< the Foil's max thickness */
        double m_fXCamber;                   /**< the x-position of the Foil's max camber point */
        double m_fXThickness;                /**< the x-position of the Foil's max thickness point */

        double m_TEGap;                        /**< the trailing edge gap */
        Vector2d m_TE;                        /**< the trailing edge point */
        Vector2d m_LE;                        /**< the leading edge point */

        Vector2d m_rpBaseMid[MIDPOINTCOUNT];          /**< the mid camber line points of the base geometry */
        Vector2d m_BaseExtrados[IQX];           /**< the upper surface points of the base geometry */
        Vector2d m_BaseIntrados[IQX];          /**< the lower surface points of the base geometry */

        Vector2d m_rpExtrados[IQX];           /**< the upper surface points */
        Vector2d m_rpIntrados[IQX];           /**< the lower surface points */

    public:

        bool m_bTEFlap;          /**< true if the foil has a trailing edge flap */
        double m_TEFlapAngle;    /**< the trailing edge flap angle, in degrees*/
        double m_TEXHinge;       /**< the x-position of the trailing edge flap, in chord % */
        double m_TEYHinge;       /**< the y-position of the trailng edge flap, in chord %*/

        bool m_bLEFlap;          /**< true if the foil has a leading edge flap */
        double m_LEFlapAngle;    /**< the leading edge flap angle, in degrees */
        double m_LEXHinge;       /**< the x-position of the leading edge flap, in chord % */
        double m_LEYHinge;       /**< the y-position of the leading edge flap, in chord %*/

};

