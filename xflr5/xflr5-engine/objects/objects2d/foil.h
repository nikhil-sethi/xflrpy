/****************************************************************************

    Reference Foil Class
    Copyright (C) 2003-2016 Andre Deperrois 

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



#ifndef FOIL_H
#define FOIL_H

#include <QTextStream>
#include <QPoint>


#include <objects/objects3d/vector3d.h>
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
class XFLR5ENGINELIBSHARED_EXPORT Foil
{

public:
    Foil();


    int isPoint(Vector3d const &Real);

    void getLowerY(double x, double &y, double &normx, double &normy);
    void getUpperY(double x, double &y, double &normx, double &normy);

    double deRotate();
    double baseUpperY(double x) const;
    double baseLowerY(double x) const;
    Vector3d midYRel(double sRel) const;
    Vector3d lowerYRel(double xRel, double &normx, double &normy) const;
    Vector3d upperYRel(double xRel, double &normx, double &normy) const;
    double camber(double x) const;
    double camberSlope(double x) const;
    double length() const;
    double area() const;
    double topSlope(double const &x);
    double bottomSlope(double const &x);
    double normalizeGeometry();
    void compMidLine(bool bParams);

    bool exportFoil(QTextStream &out);
    bool initFoil();

    void copyFoil(Foil *pSrcFoil);

    void setFlap();
    void setTEFlap();
    void setLEFlap();
    void setNaca009();
    void setLEFlapData(bool bFlap, double xhinge, double yhinge, double angle);
    void setTEFlapData(bool bFlap, double xhinge, double yhinge, double angle);

    bool intersect(Vector3d const &A, Vector3d const &B, Vector3d const &C, Vector3d const &D, Vector3d *M);

    bool &isVisible() {return m_bIsFoilVisible;}
    bool &showCenterLine() {return m_bCenterLine;}
    int &foilLineWidth() {return m_FoilWidth;}
    int &foilLineStyle() {return m_FoilStyle;}
    int &foilPointStyle() {return m_PointStyle;}
    QString const &foilName() const {return m_FoilName;}
    void setFoilName(QString const &FoilName) {m_FoilName = FoilName;}


    void getColor(int &r, int &g, int &b, int &a);
    void setColor(int r, int g, int b, int a=255);
    int red() const {return m_red;}
    int green() const {return m_green;}
    int blue() const {return m_blue;}
    int alphaChannel() const {return m_alphaChannel;}

    QString foilDescription() const {return m_FoilDescription;}
    void setFoilDescription(QString const &description) {m_FoilDescription=description;}

    double camber() const {return m_fCamber;}
    double xCamber() const {return m_fXCamber;}
    double thickness() const {return m_fThickness;}
    double xThickness() const {return m_fXThickness;}
    double TEGap() const {return m_TEGap;}

    int iHighLight() const {return m_iHighLight;}
    void setHighLight(int iH) {m_iHighLight = iH;}

    void displayCoords(bool bBaseCoords=false);


public:
    // Base geometry;
   int nb;                              /**< the number of points of the base foil */
   double xb[IBX];                      /**< the array of x-coordinates of the base foil points */
   double yb[IBX];                      /**< the array of y-coordinates of the base foil points*/
   int n;                               /**<  the number of points of the current foil */
   double x[IBX];                       /**< the array of x-coordinates of the current foil points */
   double y[IBX];                       /**< the array of y-coordinates of the current foil points*/

   double nx[IBX];                      /**< the array of x-coordinates of the current foil normal Vector3ds*/
   double ny[IBX];                      /**< the array of x-coordinates of the current foil normal Vector3ds*/
   Vector3d m_rpMid[MIDPOINTCOUNT];              /**< the mid camber line points */


public:
    QString m_FoilDescription;             /**< a free description */

    bool m_bIsFoilVisible;               /**< true if the foil is to be displayed */
    bool m_bCenterLine;                  /**< true if the foil mid camber line is to be displayed */
//    bool m_bShowFoilPoints;              /**< true if the foil's panels are to be displayed */

    int m_iBaseInt;                      /**< the number of points on the lower surface of the base foil */
    int m_iBaseExt;                      /**< the number of points on the upper surface of the base foil */

    int m_iInt;                          /**< the number of points on the lower surface of the current foil */
    int m_iExt;                          /**< the number of points on the upper surface of the current foil */

    int m_PointStyle;                    /**< the index of the style for the foil's points*/
    int m_FoilStyle;                     /**< the index of the style with which to draw the Foil contour*/
    int m_FoilWidth;                     /**< the width with which to draw the Foil */
    int m_red, m_blue, m_green, m_alphaChannel;    /**< the color with which to draw the Foil */

    int m_iHighLight;                    /**< the index of the point to highlight in the display */


    double m_fCamber;                    /**< the Foil's max camber */
    double m_fThickness;                 /**< the Foil's max thickness */
    double m_fXCamber;                   /**< the x-position of the Foil's max camber point */
    double m_fXThickness;                /**< the x-position of the Foil's max thickness point */

    double m_TEGap;                        /**< the trailing edge gap */
    Vector3d m_TE;                        /**< the trailing edge point */
    Vector3d m_LE;                        /**< the leading edge point */

    Vector3d m_rpBaseMid[MIDPOINTCOUNT];          /**< the mid camber line points of the base geometry */
    Vector3d m_BaseExtrados[IQX];           /**< the upper surface points of the base geometry */
    Vector3d m_BaseIntrados[IQX];          /**< the lower surface points of the base geometry */

    Vector3d m_rpExtrados[IQX];           /**< the upper surface points */
    Vector3d m_rpIntrados[IQX];           /**< the lower surface points */

    QString m_FoilName;                    /**<  the foil's name... */

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

#endif
