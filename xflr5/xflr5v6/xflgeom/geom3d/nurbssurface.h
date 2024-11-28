/****************************************************************************

    NURBSSurface Class
    Copyright (C) André Deperrois

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



#include "frame.h"

#define MAXVLINES      100
#define MAXULINES      100


/**
 * @class NURBSSurface
 * This class implements a 3D NURBSsurface built on an array of Frame objects.
 * The NURBS surface is described by two parameters u and v with range in [0,1].
 *
 * The NURBS control points are those of the array of Frames objects.
 * When used to describe a half body, u describes the NURBS in the x direction, and v in the z direction.
 *
*/
class NURBSSurface
{
    friend class Body;
    public:
        NURBSSurface(int iAxis=0);
        ~NURBSSurface();

        void   appendFrame(Frame*pFrame);
        Frame *appendNewFrame();
        void   clearFrames();
        Frame *frameAt(int iFrame) {return m_pFrame.at(iFrame);}
        Frame *firstFrame() {return m_pFrame.first();}
        Frame *lastFrame() {return m_pFrame.last();}
        int    frameCount() const {return m_pFrame.size();}
        int    framePointCount() const;
        double getu(double pos, double v) const;
        double getv(double u, Vector3d r) const;
        void   getPoint(double u, double v, Vector3d &Pt) const;
        void   getNormal(double u, double v, Vector3d &N) const;
        Vector3d point(double u, double v) const;
        void   insertFrame(Frame *pNewFrame);
        bool   intersectNURBS(Vector3d A, Vector3d B, Vector3d &I) const;
        void   removeFrame(int iFrame);
        void   setKnots();
        int    setvDegree(int nvDegree);
        int    setuDegree(int nuDegree);
        int    uDegree() const {return m_iuDegree;}
        int    vDegree() const {return m_ivDegree;}

        double weight(const double &d, int const &i, int const &N) const;

        double splineBlend(int const &index, int const &p, double const &t, const double *knots) const;

        double basis(int i, int deg, double t, const double *knots) const;
        double basisDerivative(int i, int deg, double t, const double *knots) const;

    private:
        QVector<Frame*> m_pFrame;            /**< a pointer to the array of Frame objects */

        int m_iuDegree;                 /**< the degree of the NURBS in the u direction */
        int m_ivDegree;                 /**< the degree of the NURBS in the v direction */
        int m_nuKnots;                  /**< the number of knots in the u direction */
        int m_nvKnots;                  /**< the number of knots in the v direction */

        double m_uKnots[MAXVLINES*2];   /**< the array of knots in the u direction */
        double m_vKnots[MAXULINES*2];   /**< the array of knots in the v direction */

        int m_iRes;                     /**< the number of output points to draw the NURBS in both directions */

        double m_Bunch;                 /**< a bunch parameter used to modify the output of point density at the end or at the middle of the NURBS */
        double m_EdgeWeightu;           /**< for a full NURBS. Unused, though, not practical */
        double m_EdgeWeightv;           /**< for a full NURBS. Unused, though, not practical */

        int m_uAxis;                    /**< used to identify along which axis parameter u is set; 0=x, 1=y, 2=z */
        int m_vAxis;                    /**< used to identify along which axis parameter u is set; 0=x, 1=y, 2=z */
};


