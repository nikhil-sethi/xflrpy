/****************************************************************************

    Vector3d Class
    Copyright (C) 2008 Andre Deperrois sail7@xflr5.com

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
  @file This file implements the Vector3d class
  */

#include <QString>

#include <math.h>

#include <xflr5-engine_global.h>


#pragma once


/** @brief
 * A simple class which implements the usual properties, methods and operators associated to a 3D Vector.
 *
 * Caution : although the definition of operators allows for compact programming,
 * the operations take much longer at run time probably due to the temporary memory allocations.
 * If speed is required, implement directly the vectorial operations component by component in the source code.
 */
class XFLR5ENGINELIBSHARED_EXPORT Vector3d
{
public:
    double x;
    double y;
    double z;

    //inline constructors
    Vector3d()
    {
        x  = 0.0;
        y  = 0.0;
        z  = 0.0;
    }

    Vector3d(double const &xi, double const &yi, double const &zi)
    {
        x  = xi;
        y  = yi;
        z  = zi;
    }

    //inline operators
    double operator[](const int &i)
    {
        if(i==0) return x;
        if(i==1) return y;
        if(i==2) return z;
        return 0.0;
    }

    bool operator ==(Vector3d const &V)
    {
        //used only to compare point positions
        return (V.x-x)*(V.x-x) + (V.y-y)*(V.y-y) + (V.z-z)*(V.z-z)<0.000000001;
    }

    void operator =(Vector3d const &T)
    {
        x = T.x;
        y = T.y;
        z = T.z;
    }

    void operator+=(Vector3d const &T)
    {
        x += T.x;
        y += T.y;
        z += T.z;
    }

    void operator-=(Vector3d const &T)
    {
        x -= T.x;
        y -= T.y;
        z -= T.z;
    }

    void operator*=(double const &d)
    {
        x *= d;
        y *= d;
        z *= d;
    }

    Vector3d operator *(double const &d) const
    {
        Vector3d T(x*d, y*d, z*d);
        return T;
    }

    Vector3d operator *(Vector3d const &T) const
    {
        Vector3d C;
        C.x =  y*T.z - z*T.y;
        C.y = -x*T.z + z*T.x;
        C.z =  x*T.y - y*T.x;
        return C;
    }

    Vector3d operator /(double const &d) const
    {
        Vector3d T(x/d, y/d, z/d);
        return T;
    }

    Vector3d operator +(Vector3d const &V) const
    {
        Vector3d T(x+V.x, y+V.y, z+V.z);
        return T;
    }


    Vector3d operator -(Vector3d const &V) const
    {
        Vector3d T(x-V.x, y-V.y, z-V.z);
        return T;
    }


    //inline methods

    float xf() const {return float(x);}
    float yf() const {return float(y);}
    float zf() const {return float(z);}

    void copy(Vector3d const &V)
    {
        x = V.x;
        y = V.y;
        z = V.z;
    }

    void set(double const &x0, double const &y0, double const &z0)
    {
        x = x0;
        y = y0;
        z = z0;
    }

    void set(Vector3d const &V)
    {
        x = V.x;
        y = V.y;
        z = V.z;
    }

    void normalize()
    {
        double abs = VAbs();
        if(abs< 1.e-10) return;
        x/=abs;
        y/=abs;
        z/=abs;
    }

    double VAbs() const
    {
        return sqrt(x*x+y*y+z*z);
    }

    double dot(Vector3d const &V) const
    {
        return x*V.x + y*V.y + z*V.z;
    }

    bool isSame(Vector3d const &V) const
    {
        //used only to compare point positions
        return (V.x-x)*(V.x-x) + (V.y-y)*(V.y-y) + (V.z-z)*(V.z-z)<0.0000001;
    }

    void translate(Vector3d const &T)
    {
        x += T.x;
        y += T.y;
        z += T.z;
    }

    void translate(const double &tx, const double &ty, const double &tz)
    {
        x += tx;
        y += ty;
        z += tz;
    }

    Vector3d normalized() const
    {
        double l = VAbs();
        if(fabs(l)<0.000000001) return Vector3d(0.0,0.0,0.0);
        else return Vector3d(x/l, y/l, z/l);
    }

    Vector3d translated(const double &tx, const double &ty, const double &tz) const
    {
        return Vector3d(x+tx, y+ty, z+tz);
    }

    int size() const
    {
        return 3;//dimension
    }

    //other methods
    void rotate(Vector3d const &R, double Angle);
    void rotate(Vector3d &O, Vector3d const &R, double Angle);
    void rotateX(Vector3d const &O, double XTilt);
    void rotateY(Vector3d const &O, double YTilt);
    void rotateZ(Vector3d const &O, double ZTilt);
    void rotateX(double delta);
    void rotateY(double YTilt);
    //    void rotateZ(double ZRot);

    void displayCoords(QString msg) const;
};

