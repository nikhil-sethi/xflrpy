/****************************************************************************

    Quaternion Class
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

#include "quaternion.h"


void Quaternion::set(Quaternion const &qt)
{
    a  = qt.a;
    qx = qt.qx;
    qy = qt.qy;
    qz = qt.qz;

    setTxx();
}


void Quaternion::fromEulerAngles(double roll, double pitch, double yaw)
{
    roll  *= PI/180.0;
    pitch *= PI/180.0;
    yaw   *= PI/180.0;

    double c1 = cos(yaw/2.0);
    double s1 = sin(yaw/2.0);
    double c2 = cos(pitch/2.0);
    double s2 = sin(pitch/2.0);
    double c3 = cos(roll/2.0);
    double s3 = sin(roll/2.0);
    double c1c2 = c1*c2;
    double s1s2 = s1*s2;
    double w =c1c2*c3 - s1s2*s3;
    double x =c1c2*s3 + s1s2*c3;
    double y =s1*c2*c3 + c1*s2*s3;
    double z =c1*s2*c3 - s1*c2*s3;
    set(w,x,y,z);
}


void Quaternion::toEulerAngles(double &roll, double &pitch, double &yaw) const
{
    // roll (x-axis rotation)
    double sinr = +2.0 * (a * qx + qy * qz);
    double cosr = +1.0 - 2.0 * (qx * qx + qy * qy);
    roll = atan2(sinr, cosr);

    // pitch (y-axis rotation)
    double sinp = +2.0 * (a * qy - qz * qx);
    if (fabs(sinp) >= 1)
        pitch = copysign(PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    double siny = +2.0 * (a * qz + qx * qy);
    double cosy = +1.0 - 2.0 * (qy * qy + qz * qz);
    yaw = atan2(siny, cosy);

    roll  *= 180.0/PI;
    pitch *= 180.0/PI;
    yaw   *= 180.0/PI;
}



/**
 * In computer graphics, Slerp is shorthand for spherical linear interpolation, introduced by Ken Shoemake
 * in the context of quaternion interpolation for the purpose of animating 3D rotation.
 * It refers to constant-speed motion along a unit-radius great circle arc,
 * given the ends and an interpolation parameter between 0 and 1.
 */
void Quaternion::slerp(Quaternion const &qt0, Quaternion const &qt1, double t, Quaternion &qslerp)
{
    Quaternion q0, q1;
    q0.set(qt0.normalized());
    q1.set(qt1.normalized());
    double dot = q0.axis().dot(q1.axis());

    if (dot < 0.0f)
    {
        q1 *= -1;
        dot = -dot;
    }

    const double DOT_THRESHOLD = 0.9995;
    if (dot > DOT_THRESHOLD)
    {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.

        qslerp = q0 + (q1 - q0)*t;
        qslerp.normalize();
        return;
    }

    double theta_0 = acos(dot);        // theta_0 = angle between input vectors
    double theta = theta_0*t;          // theta = angle between v0 and result
    double sin_theta = sin(theta);     // compute this value only once
    double sin_theta_0 = sin(theta_0); // compute this value only once

    double s0 = cos(theta) - dot * sin_theta / sin_theta_0;
    double s1 = sin_theta / sin_theta_0;

    q0 *= s0;
    q1 *= s1;
    qslerp = q0 + q1;
    qslerp.normalize();
}
