/****************************************************************************

    Vector3d Class
    Copyright (C) 2008 André Deperrois 

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



#include "vector3d.h"
#include <xflanalysis/analysis3d_params.h>
#include <xflcore/constants.h>

void Vector3d::listCoords(int i0) const
{
    QString strong;
    strong = QString::asprintf(" %3d: %19g  %19g  %19g", i0, x,y,z);
    qDebug("%s", strong.toStdString().c_str());
}


void Vector3d::listCoords(int i0, int i1) const
{
    QString strong;
    strong = QString::asprintf(" %3d  %3d: %19g  %19g  %19g", i0, i1, x,y,z);
    qDebug("%s", strong.toStdString().c_str());
}


void Vector3d::listCoords(QString const &msg) const
{
    QString strong, str;
    str = QString::asprintf("%19g  %19g  %19g", x,y,z);
    strong = msg+ " " +str;
    qDebug("%s", strong.toStdString().c_str());
}


QString Vector3d::listCoords() const
{
    return QString::asprintf(" %11g  %11g  %11g", x,y,z);
}


/**
 * Convention
 *   - first rotation is by an angle phi around the z-axis
 *   - second rotation is by an angle theta in [0,PI] about the former x-axis
 *   - third rotation is by an angle psi about the former z-axis
 * All angles are given in degrees
 */
void Vector3d::Euler(double phi, double theta, double psi)
{
    double cpsi = cos(psi  *PI/180.0);        double spsi = sin(psi*PI/180.0);
    double cthe = cos(theta*PI/180.0);        double sthe = sin(theta*PI/180.0);
    double cphi = cos(phi  *PI/180.0);        double sphi = sin(phi*PI/180.0);

    double a11 = cpsi*cphi - cthe*sphi*spsi;
    double a12 = cpsi*sphi + cthe*cphi*spsi;
    double a13 = spsi*sthe;
    double a21 = -spsi*cphi - cthe*sphi*cpsi;
    double a22 = -spsi*sphi + cthe*cphi*cpsi;
    double a23 = cpsi*sthe;
    double a31 = sthe*sphi;
    double a32 = -sthe*cphi;
    double a33 = cthe;

    double x0=x, y0=y, z0=z;
    x = a11*x0 + a12*y0 + a13*z0;
    y = a21*x0 + a22*y0 + a23*z0;
    z = a31*x0 + a32*y0 + a33*z0;
}

/**
 * Convention
 *   - first rotation is by an angle phi around the z-axis
 *   - second rotation is by an angle theta in [0,PI] about the former x-axis
 * All angles are given in degrees
 */
void Vector3d::Euler(double phi, double theta)
{
    double cthe = cos(theta*PI/180.0);        double sthe = sin(theta*PI/180.0);
    double cphi = cos(phi  *PI/180.0);        double sphi = sin(phi*PI/180.0);

    double a11 = cphi;
    double a12 = sphi;
    double a13 = 0;
    double a21 = -cthe*sphi;
    double a22 =  cthe*cphi;
    double a23 = sthe;
    double a31 = sthe*sphi;
    double a32 = -sthe*cphi;
    double a33 = cthe;

    double x0=x, y0=y, z0=z;
    x = a11*x0 + a12*y0 + a13*z0;
    y = a21*x0 + a22*y0 + a23*z0;
    z = a31*x0 + a32*y0 + a33*z0;
}


/**
 * Computes and returns the signed angle in degrees between two 3d vectors
 * @todo UNUSED - there is no such signed angle
 */
double Vector3d::vectorAngle(Vector3d const &V1, Vector3d const &positivedir) const
{
    Vector3d u = this->normalized();
    Vector3d v = V1.normalized();
    double costh = v.dot(u);
    double sinth = (u*v).dot(positivedir);

    if(costh>=0)
    {
        if(sinth>=0.0) return acos(costh)*180.0/PI;
        else           return 360.0-acos(costh)*180.0/PI;
    }
    else
    {
        if(sinth>=0.0) return acos(costh)*180.0/PI;
        else           return 360.0-acos(costh)*180.0/PI;
    }
}



/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the x direction, by an angle XTilt
 * @param O The origin of the rotation
 * @param XTilt the angle of rotation in degrees
*/
void Vector3d::rotateX(Vector3d const &O, double rx)
{
    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    rx *=PI/180.0;
    y = O.y + OP.y * cos(rx) - OP.z * sin(rx);
    z = O.z + OP.y * sin(rx) + OP.z * cos(rx);
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the y direction, by an angle YTilt
 * @param O The origin of the rotation
 * @param YTilt the angle of rotation in degrees
*/
void Vector3d::rotateY(Vector3d const &O, double ry)
{
    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    ry *=PI/180.0;

    x = O.x + OP.x * cos(ry) + OP.z * sin(ry);
    z = O.z - OP.x * sin(ry) + OP.z * cos(ry);
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the z direction, by an angle ZTilt
 * @param O The origin of the rotation
 * @param ZTilt the angle of rotation in degrees
*/
void Vector3d::rotateZ(Vector3d const &O, double rz)
{
    //Rotate the vector around the Z-axis, by an angle ZTilt
    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    rz *=PI/180.0;

    x = O.x + OP.x * cos(rz) - OP.y * sin(rz);
    y = O.y + OP.x * sin(rz) + OP.y * cos(rz);
}


void Vector3d::rotateX(double angleDeg)
{
    angleDeg *=PI/180.0;

    double yo = y;
    double zo = z;
    y =  yo * cos(angleDeg) - zo * sin(angleDeg);
    z =  yo * sin(angleDeg) + zo * cos(angleDeg);
}


void Vector3d::rotateY(double angleDeg)
{
    angleDeg *=PI/180.0;

    double xo = x;
    double zo = z;
    x =  xo * cos(angleDeg) + zo * sin(angleDeg);
    z = -xo * sin(angleDeg) + zo * cos(angleDeg);
}


void Vector3d::rotateZ(double angleDeg)
{
    angleDeg *=PI/180.0;

    double xo = x;
    double yo = y;
    x =  xo * cos(angleDeg) - yo * sin(angleDeg);
    y =  xo * sin(angleDeg) + yo * cos(angleDeg);
}


/** Rotates this vector around the axis defined by the vector R and by the angle Angle
* @param R the axis of rotation, assumed to have unit length
* @param Angle the angle of rotation in degrees
*/
void Vector3d::rotate(Vector3d const &R, double Angle)
{
    double ca = cos(Angle *PI/180.0);
    double sa = sin(Angle *PI/180.0);

    double x0 = x;
    double y0 = y;
    double z0 = z;

    double ux = R.x;
    double uy = R.y;
    double uz = R.z;

    x =     (ca+ux*ux*(1-ca))  *x0  +  (ux*uy*(1-ca)-uz*sa) *y0 +  (ux*uz*(1-ca)+uy*sa) *z0;
    y =   (uy*ux*(1-ca)+uz*sa) *x0  +    (ca+uy*uy*(1-ca))  *y0 +  (uy*uz*(1-ca)-ux*sa) *z0;
    z =   (uz*ux*(1-ca)-uy*sa) *x0  +  (uz*uy*(1-ca)+ux*sa) *y0 +    (ca+uz*uz*(1-ca))  *z0;
}


/** Rotates the point defined by this vector around origin O, the rotation axis defined by vector R, and by the angle Angle
* @param O the center of rotation
* @param R the axis of rotation
* @param Angle the angle of rotation in degrees
*/
void Vector3d::rotate(Vector3d const &O, Vector3d const &R, double Angle)
{
    if(fabs(R.norm())<0.00001) return;

    Vector3d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;
    OP.z = z-O.z;

    OP.rotate(R, Angle);

    x = O.x + OP.x;
    y = O.y + OP.y;
    z = O.z + OP.z;
}
