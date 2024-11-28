/****************************************************************************

    ArcBall
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

// default angles for pseudo-iso view
#define YAW   -180.0
#define ROLL  -110.0
#define PITCH   0.0

#include <xflgeom/geom3d/quaternion.h>


class ArcBall
{
    public:
        ArcBall();

        void setQuat(const Quaternion &qt) { m_Quat = qt;}
        void setQuat(double r, double qx, double qy, double qz);
        void start(double ax, double ay);
        void move(double ax, double ay);
        void getSpherePoint(double xv, double yv, Vector3d &Pt);
        void rotateCrossPoint(float &angle, float &xf, float &yf, float &zf);
        void applyRotation(Quaternion const &qtrot, bool bInverted);

        void getRotationMatrix(double*m, bool bTranspose) const {m_Quat.toMatrix(m, bTranspose);}

        static void setSphereRadius(double radius){s_sphereRadius = radius;}
        static double sphereRadius() {return s_sphereRadius;}


    public:
        Vector3d m_Start;                /**< The vector point on the sphere at the time when the mouse was pressed */
        Vector3d m_Current;              /**< The current active point on the sphere */

        Quaternion m_QuatStart;          /**< the quaternion representing the rotation of the view matrix at the time when the mouse was pressed */
        Quaternion m_Quat;               /**< the quaternion representing the rotation of the view matrix */

        static double s_sphereRadius;
};



