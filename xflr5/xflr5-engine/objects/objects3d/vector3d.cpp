/****************************************************************************

    Vector Class
	Copyright (C) 2008 Andre Deperrois 

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



#include <math.h>

#include "vector3d.h"
#include <analysis3d/analysis3d_params.h>



/** Rotates this vector around the axis defined by the vector R and by the angle Angle
* @param R the axis of rotation
* @param Angle the angle of rotation in degrees
*/
void Vector3d::rotate(Vector3d const &R, double Angle)
{
    double norm, ux, uy,uz, ca, sa, x0, y0, z0;

	ca = cos(Angle *PI/180.0);
	sa = sin(Angle *PI/180.0);

	x0 = x;
	y0 = y;
	z0 = z;

	norm = sqrt(R.x*R.x+ R.y*R.y + R.z*R.z);
	ux = R.x/norm;
	uy = R.y/norm;
	uz = R.z/norm;

	x =     (ca+ux*ux*(1-ca))  *x0  +  (ux*uy*(1-ca)-uz*sa) *y0 +  (ux*uz*(1-ca)+uy*sa) *z0;
	y =   (uy*ux*(1-ca)+uz*sa) *x0  +    (ca+uy*uy*(1-ca))  *y0 +  (uy*uz*(1-ca)-ux*sa) *z0;
	z =   (uz*ux*(1-ca)-uy*sa) *x0  +  (uz*uy*(1-ca)+ux*sa) *y0 +    (ca+uz*uz*(1-ca))  *z0;
}


/** Rotates the point defined by this vector around origin O, the rotation axis defined by vector R, and by the angle Angle
* @param O the center of rotation
* @param R the axis of rotation
* @param Angle the angle of rotation in degrees
*/
void Vector3d::rotate(Vector3d &O, Vector3d const &R, double Angle)
{
    Vector3d OP;
	OP.x = x-O.x;
	OP.y = y-O.y;
	OP.z = z-O.z;

	OP.rotate(R, Angle);

	x = O.x + OP.x;
	y = O.y + OP.y;
	z = O.z + OP.z;
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the x direction, by an angle XTilt
 * @param O The origin of the rotation
 * @param XTilt the angle of rotation in degrees
*/
void Vector3d::rotateX(Vector3d const &O, double XTilt)
{
	Vector3d OP;
	OP.x = x-O.x;
	OP.y = y-O.y;
	OP.z = z-O.z;
		
	XTilt *=PI/180.0;
	y = O.y + OP.y * cos(XTilt) - OP.z * sin(XTilt);
	z = O.z + OP.y * sin(XTilt) + OP.z * cos(XTilt);
}

/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the y direction, by an angle YTilt
 * @param O The origin of the rotation
 * @param YTilt the angle of rotation in degrees
*/
void Vector3d::rotateY(Vector3d const &O, double YTilt)
{
	//Rotate the vector around the Y-axis, by an angle YTilt
	Vector3d OP;
	OP.x = x-O.x;
	OP.y = y-O.y;
	OP.z = z-O.z;
		
	YTilt *=PI/180.0;

	x = O.x + OP.x * cos(YTilt) + OP.z * sin(YTilt);
	z = O.z - OP.x * sin(YTilt) + OP.z * cos(YTilt);
}


/**
 * The vector is interpreted as a point.
 * Rotates the point around the axis defined by point O and the z direction, by an angle ZTilt
 * @param O The origin of the rotation
 * @param ZTilt the angle of rotation in degrees
*/
void Vector3d::rotateZ(Vector3d const &O, double ZTilt)
{
	//Rotate the vector around the Z-axis, by an angle ZTilt
	Vector3d OP;
	OP.x = x-O.x;
	OP.y = y-O.y;
	OP.z = z-O.z;
	
	ZTilt *=PI/180.0;

	x = O.x + OP.x * cos(ZTilt) - OP.y * sin(ZTilt);
	y = O.y + OP.x * sin(ZTilt) + OP.y * cos(ZTilt);
}


void  Vector3d::rotateX(double delta)
{
	delta *=PI/180.0;

	double yo = y;
	double zo = z;
	y =  yo * cos(delta) - zo * sin(delta);
	z =  yo * sin(delta) + zo * cos(delta);
}



void  Vector3d::rotateY(double YTilt)
{
	YTilt *=PI/180.0;

	double xo = x;
	double zo = z;
	x =  xo * cos(YTilt) + zo * sin(YTilt);
	z = -xo * sin(YTilt) + zo * cos(YTilt);
}


void Vector3d::displayCoords(QString msg) const
{
	QString strange;
	strange.sprintf("%s  %13.5g, %13.5g, %13.5g", msg.toStdString().c_str(), x,y,z);
	qDebug("%s", strange.toStdString().c_str());
}

