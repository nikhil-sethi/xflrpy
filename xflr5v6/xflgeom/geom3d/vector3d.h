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

/**
  @file This file implements the Vector3d class
  */

#include <cmath>

#include <QString>

#pragma once


/** @brief
 * A simple class which implements the usual properties, methods and operators associated to a 3D Vector.
 *
 * Caution : although the definition of operators allows for compact programming,
 * the operations take much longer at run time probably due to the temporary memory allocations.
 * If speed is required, implement directly the vectorial operations component by component in the source code.
 */
class Vector3d
{
    public:
        double x;
        double y;
        double z;

        //constructors
        Vector3d() : x{0},y{0},z{0}
        {
        }

        Vector3d(double xi, double yi, double zi) : x{xi}, y{yi}, z{zi}
        {
        }

        Vector3d(double const*coords) : x{coords[0]}, y{coords[1]}, z{coords[2]}
        {
        }

        virtual ~Vector3d() = default;

        double dir(int i) const
        {
            if(i==0) return x;
            if(i==1) return y;
            if(i==2) return z;
            return 0.0;
        }

        double & coord(int i)
        {
            if      (i==0) return x;
            else if (i==1) return y;
            else if (i==2) return z;
            return x;
        }

        double const& coord(int i) const
        {
            if      (i==0) return x;
            else if (i==1) return y;
            else if (i==2) return z;
            return x;
        }

        // avoid due to compiler conflicts with index operator of std::vector and QVector
/*        double &operator[](int i)
        {
            if(i==0) return x;
            if(i==1) return y;
            if(i==2) return z;
            return x;
        }*/

        float xf() const {return float(x);}
        float yf() const {return float(y);}
        float zf() const {return float(z);}

        bool operator ==(const Vector3d& V)
        {
            //used only to compare point positions
            //		return (V.x-x)*(V.x-x) + (V.y-y)*(V.y-y) + (V.z-z)*(V.z-z)<0.0001*0.0001;

            if(fabs(V.x-x)<0.0001)
            {
                if(fabs(V.y-y)<0.0001)
                {
                    if(fabs(V.z-z)<0.0001) return true;
                }
            }
            return false;
        }

        void crossP(Vector3d const &T, Vector3d &crossproduct) const
        {
            crossproduct.x =  y*T.z - z*T.y;
            crossproduct.y = -x*T.z + z*T.x;
            crossproduct.z =  x*T.y - y*T.x;
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

        void operator*=(double d)
        {
            x *= d;
            y *= d;
            z *= d;
        }

        Vector3d operator +(Vector3d const &V) const {return Vector3d(x+V.x, y+V.y, z+V.z);}
        Vector3d operator -(Vector3d const &V) const {return Vector3d(x-V.x, y-V.y, z-V.z);}
        Vector3d operator *(double const &d)   const {return Vector3d(x*d, y*d, z*d);}
        Vector3d operator /(double const &d)   const {return Vector3d(x/d, y/d, z/d);}

        Vector3d operator *(Vector3d const &T) const
        {
            Vector3d C;
            C.x =  y*T.z - z*T.y;
            C.y = -x*T.z + z*T.x;
            C.z =  x*T.y - y*T.x;
            return C;
        }

        void copy(Vector3d const &V)
        {
            x = V.x;
            y = V.y;
            z = V.z;
        }

        void reset() {x=y=z=0.0;}

        void set(double x0, double y0, double z0)
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
            double abs = norm();
            if(abs< 1.e-15) return;
            x/=abs;
            y/=abs;
            z/=abs;
        }

        double norm() const{return sqrt(x*x+y*y+z*z);}

        float normf() const{return sqrtf(x*x+y*y+z*z);}

        double dot(Vector3d const &V) const{return x*V.x + y*V.y + z*V.z;}

        float dotf(Vector3d const &V) const{return float(x*V.x + y*V.y + z*V.z);}

        bool isSame(Vector3d const &V, double precision=1.0e-6) const
        {
            if(fabs(V.x-x)<precision)
            {
                if(fabs(V.y-y)<precision)
                {
                    if(fabs(V.z-z)<precision) return true;
                }
            }
            return false;
        }

        void translate(Vector3d const &T)
        {
            x += T.x;
            y += T.y;
            z += T.z;
        }

        void translate(double tx, double ty, double tz)
        {
            x += tx;
            y += ty;
            z += tz;
        }

        Vector3d normalized() const
        {
            double l = norm();
            if(fabs(l)<0.000000001) return Vector3d(0.0,0.0,0.0);
            else return Vector3d(x/l, y/l, z/l);
        }

        Vector3d translated(double tx, double ty, double tz) const
        {
            return Vector3d(x+tx, y+ty, z+tz);
        }

        void reverse()
        {
            x=-x;
            y=-y;
            z=-z;
        }

        Vector3d reversed() const{return Vector3d(-x, -y, -z);}

        int size() const{return 3;}//dimension

        double distanceTo(Vector3d const &pt) const {return sqrt((pt.x-x)*(pt.x-x) + (pt.y-y)*(pt.y-y) + (pt.z-z)*(pt.z-z));}
        double distanceTo(double X, double Y, double Z) const {return sqrt((X-x)*(X-x) + (Y-y)*(Y-y) + (Z-z)*(Z-z));}

        //other methods
        virtual void rotate(Vector3d const &R, double Angle);
        virtual void rotate(Vector3d const &O, Vector3d const &R, double Angle);
        virtual void rotateX(Vector3d const &O, double rx);
        virtual void rotateY(Vector3d const &O, double ry);
        virtual void rotateZ(Vector3d const &O, double rz);
        virtual void rotateX(double angleDeg);
        virtual void rotateY(double angleDeg);
        virtual void rotateZ(double angleDeg);


        void Euler(double phi, double theta, double psi);
        void Euler(double phi, double theta);

        double vectorAngle(Vector3d const &V1, const Vector3d &positivedir) const;

        void listCoords(int i0) const;
        void listCoords(int i0, int i1) const;
        void listCoords(QString const &msg) const;
        QString listCoords() const;
};
