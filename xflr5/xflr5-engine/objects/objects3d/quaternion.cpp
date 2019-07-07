/****************************************************************************

    Quaternion Class
    Copyright (C) 2008-2008 Andre Deperrois 

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
#include <math.h>
#include <QtGlobal>



void Quaternion::operator *=(Quaternion Q)
{
    double t1,t2,t3,t4;

    t1 = a*Q.a  - qx*Q.qx - qy*Q.qy - qz*Q.qz;
    t2 = a*Q.qx + qx*Q.a  + qy*Q.qz - qz*Q.qy ;
    t3 = a*Q.qy + qy*Q.a  + qz*Q.qx - qx*Q.qz ;
    t4 = a*Q.qz + qz*Q.a  + qx*Q.qy - qy*Q.qx ;

    a  = t1;
    qx = t2;
    qy = t3;
    qz = t4;
    Settxx();
}


Quaternion Quaternion::operator *(Quaternion Q)
{
    Quaternion prod;

    prod.a = a*Q.a  - qx*Q.qx - qy*Q.qy - qz*Q.qz;

    prod.qx = a*Q.qx + qx*Q.a  + qy*Q.qz - qz*Q.qy ;
    prod.qy = a*Q.qy + qy*Q.a  + qz*Q.qx - qx*Q.qz ;
    prod.qz = a*Q.qz + qz*Q.a  + qx*Q.qy - qy*Q.qx ;
    prod.Settxx();

    return prod;
}


void Quaternion::operator =(Quaternion Q)
{
    a  = Q.a;
    qx = Q.qx;
    qy = Q.qy;
    qz = Q.qz;
    Settxx();
}


void Quaternion::operator ~()
{
    qx = -qx;
    qy = -qy;
    qz = -qz;
    Settxx();
}


void Quaternion::Normalize()
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
    Settxx();
}










