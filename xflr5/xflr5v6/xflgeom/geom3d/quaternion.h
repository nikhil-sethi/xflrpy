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




/** @file
 *
 * This class defines the Quaternion used for the calculation of rotations in 3D display
 *
 */


#pragma once


/**
*@class Quaternion
*@brief
 * The class which defines the quaternion object used for the calculation of rotations in 3D display.

 Home made class. Since it was written, Qt has developed and provided a QQuaternion class.
*/

#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/constants.h>

class Quaternion
{
    public:
        inline Quaternion(void);
        inline Quaternion(double const &t, double x, double y, double z);
        inline Quaternion(double Angle, Vector3d const &R);

        /** from Euler Angles */
        Quaternion(double pitch, double roll, double yaw);

        void set(Quaternion const &qt);

        double norm() const {return sqrt(a*a + qx*qx + qy*qy + qz*qz);}
        inline void normalize();
        inline Quaternion normalized() const;

        inline void operator*=(double d);
        inline void operator*=(Quaternion const &Q);
        inline void operator ~();

        inline Quaternion operator +(Quaternion const &qt);
        inline Quaternion operator -(Quaternion const &qt);
        inline Quaternion operator *(double d);
        inline Quaternion operator *(Quaternion const &Q) const;

        void listQuaternion() const
        {
            qDebug("quat: a=%17g  qx=%17g  qx=%17g  qx=%17g", a, qx, qy, qz);
        }


        inline void conjugate(Vector3d const &Vin, Vector3d &Vout) const;
        inline void conjugate(Vector3d &V) const;
        inline void conjugate(double &x, double &y, double &z) const;

        inline Vector3d rotateVector(Vector3d const &Vin) const;
        inline void setTxx();

        inline void set(double const &real, double const &x, double const &y, double const &z);
        inline void set(double const &Angle, Vector3d const &R);

        inline void toMatrix(double *q, bool bTranspose=false) const;

        Vector3d axis() const {return Vector3d(qx, qy, qz);}
        double angle() const {return m_theta*180/PI;}

        void toEulerAngles(double &roll, double &pitch, double &yaw) const;
        void fromEulerAngles(double roll, double pitch, double yaw);

        inline void from2UnitVectors(Vector3d const &A, Vector3d const &B);

        static void slerp(const Quaternion &qt0, const Quaternion &qt1, double t, Quaternion &qslerp);

    private:
        double t2, t3, t4, t5, t6, t7, t8, t9, t10;


    public:
        double a, qx, qy,qz;
        double m_theta;

};


inline Quaternion::Quaternion(void) : t2{0}, t3{0}, t4{0}, t5{0}, t6{0}, t7{0}, t8{0}, t9{0}, t10{0},
                                      a{1.0}, qx{0}, qy{0}, qz{0}, m_theta{0}
{
    setTxx();
}


inline Quaternion::Quaternion(double const &t, double x, double y, double z) :
                  t2{0}, t3{0}, t4{0}, t5{0}, t6{0}, t7{0}, t8{0}, t9{0}, t10{0},
                  a{t}, qx{x}, qy{y}, qz{z}, m_theta{0}
{
    setTxx();
}


inline Quaternion::Quaternion(double Angle, Vector3d const &R):
    t2{0}, t3{0}, t4{0}, t5{0}, t6{0}, t7{0}, t8{0}, t9{0}, t10{0}
{
    Vector3d N;
    N = R;
    N.normalize();
    m_theta = Angle*PI/180.0;

    a = cos(m_theta/2.0);
    double sina = sin(m_theta/2.0);

    qx = N.x*sina;
    qy = N.y*sina;
    qz = N.z*sina;

    setTxx();
}


/** from Euler Angles */
inline Quaternion::Quaternion(double pitch, double roll, double yaw)
{
    pitch *= PI/180.0;
    roll  *= PI/180.0;
    yaw   *= PI/180.0;

        // Abbreviations for the various angular functions
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);

    a = cy*cr*cp + sy*sr*sp;
    qx = cy*sr*cp - sy*cr*sp;
    qy = cy*cr*sp + sy*sr*cp;
    qz = sy*cr*cp - cy*sr*sp;

    setTxx();
}


//inline functions
inline void Quaternion::conjugate(Vector3d const &Vin, Vector3d &Vout) const
{
    Vout.x = 2.0*( (t8 + t10)*Vin.x + (t6 -  t4)*Vin.y + (t3 + t7)*Vin.z ) + Vin.x;
    Vout.y = 2.0*( (t4 +  t6)*Vin.x + (t5 + t10)*Vin.y + (t9 - t2)*Vin.z ) + Vin.y;
    Vout.z = 2.0*( (t7 -  t3)*Vin.x + (t2 +  t9)*Vin.y + (t5 + t8)*Vin.z ) + Vin.z;
}


inline void Quaternion::conjugate(Vector3d &V) const
{
    double Rx = V.x;
    double Ry = V.y;
    double Rz = V.z;

    V.x = 2.0*( (t8 + t10)*Rx + (t6 -  t4)*Ry + (t3 + t7)*Rz ) + Rx;
    V.y = 2.0*( (t4 +  t6)*Rx + (t5 + t10)*Ry + (t9 - t2)*Rz ) + Ry;
    V.z = 2.0*( (t7 -  t3)*Rx + (t2 +  t9)*Ry + (t5 + t8)*Rz ) + Rz;
}


inline void Quaternion::conjugate(double &x, double &y, double &z) const
{
    double Rx = x;
    double Ry = y;
    double Rz = z;

    x = 2.0*( (t8 + t10)*Rx + (t6 -  t4)*Ry + (t3 + t7)*Rz ) + Rx;
    y = 2.0*( (t4 +  t6)*Rx + (t5 + t10)*Ry + (t9 - t2)*Rz ) + Ry;
    z = 2.0*( (t7 -  t3)*Rx + (t2 +  t9)*Ry + (t5 + t8)*Rz ) + Rz;
}


inline void Quaternion::setTxx()
{
    t2 =   a*qx;
    t3 =   a*qy;
    t4 =   a*qz;
    t5 =  -qx*qx;
    t6 =   qx*qy;
    t7 =   qx*qz;
    t8 =  -qy*qy;
    t9 =   qy*qz;
    t10 = -qz*qz;

    m_theta = 2.0*atan2(sqrt(qx*qx+qy*qy+qz*qz), a);
}


inline void Quaternion::set(double const &real, double const &x, double const &y, double const &z)
{
    a = real;
    qx = x;
    qy = y;
    qz = z;

    setTxx();
}


inline void Quaternion::set(double const &Angle, Vector3d const &R)
{
    Vector3d N;
    N = R;
    N.normalize();
    m_theta = Angle*PI/180.0;

    a = cos(m_theta/2.0);
    double sina = sin(m_theta/2.0);

    qx = N.x*sina;
    qy = N.y*sina;
    qz = N.z*sina;
    setTxx();
}


inline void Quaternion::toMatrix(double* q, bool bTranspose) const
{
    double x2 = qx*qx;
    double y2 = qy*qy;
    double z2 = qz*qz;
    double xy = qx*qy;
    double xz = qx*qz;
    double yz = qy*qz;
    double wx = a*qx;
    double wy = a*qy;
    double wz = a*qz;

    if(!bTranspose)
    {
        q[0]  = 1 - 2*y2 - 2*z2;
        q[1]  = 2*xy + 2*wz;
        q[2]  = 2*xz - 2*wy;
        q[3]  = 0.0;
        q[4]  = 2*xy - 2*wz;
        q[5]  = 1 - 2*x2 - 2*z2;
        q[6]  = 2*yz + 2*wx;
        q[7]  = 0.0;
        q[8]  = 2*xz + 2*wy;
        q[9]  = 2*yz - 2*wx;
        q[10] = 1 - 2*x2 - 2*y2;
        q[11] = 0.0;
        q[12] = 0.0;
        q[13] = 0.0;
        q[14] = 0.0;
        q[15] = 1.0;
    }
    else
    {
        q[0]  = 1 - 2*y2 - 2*z2;
        q[4]  = 2*xy + 2*wz;
        q[8]  = 2*xz - 2*wy;
        q[12] = 0.0;
        q[1]  = 2*xy - 2*wz;
        q[5]  = 1 - 2*x2 - 2*z2;
        q[9]  = 2*yz + 2*wx;
        q[13] = 0.0;
        q[2]  = 2*xz + 2*wy;
        q[6]  = 2*yz - 2*wx;
        q[10] = 1 - 2*x2 - 2*y2;
        q[14] = 0.0;
        q[3]  = 0.0;
        q[7]  = 0.0;
        q[11] = 0.0;
        q[15] = 1.0;
    }
}


inline void Quaternion::operator *=(Quaternion const &Q)
{
    double t1 = a*Q.a  - qx*Q.qx - qy*Q.qy - qz*Q.qz;
    double t2 = a*Q.qx + qx*Q.a  + qy*Q.qz - qz*Q.qy ;
    double t3 = a*Q.qy + qy*Q.a  + qz*Q.qx - qx*Q.qz ;
    double t4 = a*Q.qz + qz*Q.a  + qx*Q.qy - qy*Q.qx ;

    a  = t1;
    qx = t2;
    qy = t3;
    qz = t4;

    setTxx();
}


inline void Quaternion::operator *=(double d)
{
    a  *= d;
    qx *= d;
    qy *= d;
    qz *= d;

    setTxx();
}


inline Vector3d Quaternion::rotateVector(Vector3d const &Vin) const
{
    double Rx = Vin.x;
    double Ry = Vin.y;
    double Rz = Vin.z;

    Vector3d Vout;
    Vout.x = 2.0*( (t8 + t10)*Rx + (t6 -  t4)*Ry + (t3 + t7)*Rz ) + Rx;
    Vout.y = 2.0*( (t4 +  t6)*Rx + (t5 + t10)*Ry + (t9 - t2)*Rz ) + Ry;
    Vout.z = 2.0*( (t7 -  t3)*Rx + (t2 +  t9)*Ry + (t5 + t8)*Rz ) + Rz;

    return Vout;
}


inline void Quaternion::normalize()
{
    double norm = sqrt(a*a + qx*qx + qy*qy + qz*qz);
    if (norm < 1.0e-10)
    {
        a = 1.0;
        qx = 0.0;
        qy = 0.0;
        qz = 0.0;
    }
    else
    {
        a *= 1/norm;

        qx *= 1/norm;
        qy *= 1/norm;
        qz *= 1/norm;
    }
    setTxx();
}


inline Quaternion Quaternion::normalized() const
{
    Quaternion qt;
    double norm = sqrt(a*a + qx*qx + qy*qy + qz*qz);
    if (norm < 1.0e-10)
    {
        qt.a = 1.0;
        qt.qx = 0.0;
        qt.qy = 0.0;
        qt.qz = 0.0;
    }
    else
    {
        qt.a = a/norm;

        qt.qx = qx/norm;
        qt.qy = qy/norm;
        qt.qz = qz/norm;
    }
    qt.setTxx();
    return qt;
}


inline void Quaternion::operator ~()
{
    qx = -qx;
    qy = -qy;
    qz = -qz;

    setTxx();
}


inline Quaternion Quaternion::operator +(Quaternion const &qt)
{
    Quaternion sum;
    sum.a  = a  + qt.a;
    sum.qx = qx + qt.qx;
    sum.qy = qy + qt.qy;
    sum.qz = qz + qt.qz;
    sum.setTxx();
    return sum;
}


inline Quaternion Quaternion::operator -(Quaternion const &qt)
{
    Quaternion diff;
    diff.a  = a  - qt.a;
    diff.qx = qx - qt.qx;
    diff.qy = qy - qt.qy;
    diff.qz = qz - qt.qz;
    diff.setTxx();
    return diff;
}


inline Quaternion Quaternion::operator *(double d)
{
    Quaternion scaled;
    scaled.a  *= d;
    scaled.qx *= d;
    scaled.qy *= d;
    scaled.qz *= d;

    scaled.setTxx();

    return scaled;
}


inline Quaternion Quaternion::operator *(Quaternion const &qt) const
{
    Quaternion prod;

    prod.a = a*qt.a  - qx*qt.qx - qy*qt.qy - qz*qt.qz;

    prod.qx = a*qt.qx + qx*qt.a  + qy*qt.qz - qz*qt.qy ;
    prod.qy = a*qt.qy + qy*qt.a  + qz*qt.qx - qx*qt.qz ;
    prod.qz = a*qt.qz + qz*qt.a  + qx*qt.qy - qy*qt.qx ;
    prod.setTxx();

    return prod;
}


/** Builds this quaternion so that it rotates Vector A onto Vector B.
 * A and B are assumed to be unit vectors
*/
inline void Quaternion::from2UnitVectors(Vector3d const &A, Vector3d const &B)
{
    float cosa   = float(A.dot(B));
    float sina2  = sqrtf((1.0f - cosa)*0.5f);
    float cosa2  = sqrtf((1.0f + cosa)*0.5f);

    Vector3d Omega(A*B);
    Omega.normalize();
    Omega *= double(sina2);
    set(double(cosa2), Omega.x, Omega.y, Omega.z);
}

