/****************************************************************************

    Spline Foil Class
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
 *@file This class defines the splined foil object used in foil design.
 */


#pragma once


#include "spline5.h"
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/xflobject.h>


/**
*@class SplineFoil
*@brief  The class which defines the splined foil object.

The foil is contructed based on one spline for the upper surface and one spline for the lower surface.
*/
class SplineFoil : public XflObject
{
    friend class AFoil;
    friend class SplineCtrlsDlg;
    friend class FoilTableDelegate;


    public:
        SplineFoil();
        SplineFoil(SplineFoil *pSF);

        bool isSymetric()        const {return m_bSymetric;}
        bool showOutPoints()     const {return m_bOutPoints;}
        bool showCenterLine()    const {return m_bCenterLine;}

        QString const &splineFoilName() const {return m_Name;}
        void setSplineFoilName(QString const &name) {m_Name=name;}

        Spline *extrados() {return &m_Extrados;}
        Spline *intrados() {return &m_Intrados;}

        bool isModified() const  {return m_bModified;}
        void setModified(bool bModified){m_bModified = bModified;}

        void compMidLine();

        void initSplineFoil();

        bool serialize(QDataStream &ar, bool bIsStoring);
        bool serializeXFL(QDataStream &ar, bool bIsStoring);

        void copy(SplineFoil* pSF);
        void drawCtrlPoints(QPainter &painter, double scalex, double scaley, QPointF Offset);
        void drawMidLine(QPainter &painter, double scalex, double scaley, QPointF Offset);
        void drawFoil(QPainter &painter, double scalex, double scaley, QPointF Offset);
        void drawOutPoints(QPainter &painter, double scalex, double scaley, QPointF Offset);
        void exportToBuffer(Foil *pFoil);
        void exportToFile(QTextStream &out);
        void updateSplineFoil();

        double camber() const {return m_fCamber;}
        double xCamber() const {return m_fxCambMax;}
        double thickness() const {return m_fThickness;}
        double xThickness() const {return m_fxThickMax;}

        bool bClosedTE() const {return m_bForceCloseTE;}
        bool bClosedLE() const {return m_bForceCloseLE;}
        void setClosedTE(bool bClosed) {m_bForceCloseTE=bClosed;}
        void setClosedLE(bool bClosed) {m_bForceCloseLE=bClosed;}


    private:
        bool m_bModified;                /**< false if the SplineFoil has been serialized in its current dtate, false otherwise */
        bool m_bOutPoints;               /**< true if the ouput line points should be displayed */
        bool m_bCenterLine;              /**< true if the SplineFoil's mean camber line is to be displayed */
        bool m_bSymetric;                /**< true if the SplineFoil is symetric. In which case the lower surface is set as symetric of the upper surface. */
        bool m_bForceCloseLE;            /**< true if the leading end points of the top and bottom spline should be positioned at the same place */
        bool m_bForceCloseTE;            /**< true if the traling end points of the top and bottom spline should be positioned at the same place */
        int m_OutPoints;                 /**< the number of output points with which to draw the SplineFoil. */


        double m_fCamber;                /**< the SplineFoil's max camber */
        double m_fThickness;             /**< the SplineFoil's max thickness */
        double m_fxCambMax;              /**< the x-position of the SplineFoil's max camber point */
        double m_fxThickMax;             /**< the x-position of the SplineFoil's max thickness point */
        Spline5 m_Extrados;               /**< the spline which defines the upper surface */
        Spline5 m_Intrados;               /**< the spline which defines the lower surface */
        Vector3d m_rpMid[MIDPOINTCOUNT];  /**< the points on the SplineFoil's mid camber line @todo replace with a QVarLengthArray */

};

