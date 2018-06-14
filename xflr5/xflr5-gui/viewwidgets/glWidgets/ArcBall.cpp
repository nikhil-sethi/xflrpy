/****************************************************************************

	ArcBall Class
	Copyright (C)  Bradley Smith, March 24, 2006
	Hideously modified in 2008 by Andre Deperrois  for miserable selfish purposes

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


#include "ArcBall.h"
#include <QtOpenGL>
#include "math.h"


ArcBall::ArcBall(void)
{
	angle = 0.0;
	Quat.a  = 0.0;

	Quat.qx = Quat.qy = Quat.qz = 0.0;

	ax = ay = az = 0.0;

	memset(ab_quat,       0, 16*sizeof(float));
	memset(ab_last,       0, 16*sizeof(float));
	memset(ab_next,       0, 16*sizeof(float));
	memset(ab_crosspoint, 0, 16*sizeof(float));

	Quaternion qti;
	double yaw = -PI;
	double pitch = 0.0;
	double roll = -2.0*PI/3.0;
	quat(roll, pitch, yaw, qti);

	Quaternion qtyaw(-30.0, Vector3d(0.0,0.0,1.0));
	setQuat(qti*qtyaw);

	// the distance from the origin to the eye
	ab_zoom  = 1.0;
	ab_zoom2 = 1.0;
	// the radius of the arcball
	ab_sphere  = 3.0;
	ab_sphere2 = 9.0;
	// the distance from the origin of the plane that intersects
	// the edge of the visible sphere (tangent to a ray from the eye)
	ab_edge = 1.0;
	// whether we are using a sphere or plane
	ab_planar = false;
	ab_planedist = 0.5;

	ab_start.set(0.0,0.0,1.0);
	ab_curr.set(0.0,0.0,1.0);
	ab_eye.set(0.0,0.0,1.0);
	ab_eyedir.set(0.0,0.0,1.0);
	ab_up.set(0.0,1.0,0.0);
	ab_out.set(1.0,0.0,0.0);

	a = ac = b = c2 = cosa = cosa2 = delta = q = sina2 = t =0.0;
	sc.set(0.0,0.0,1.0);
	ec.set(0.0,0.0,1.0);
}



/** find the intersection with the plane through the visible edge*/
void ArcBall::edgeCoords(Vector3d m, Vector3d &V)
{
	// find the intersection of the edge plane and the ray
	t = (ab_edge - ab_zoom) / (ab_eyedir.dot(m));
	aa = ab_eye + (m*t);
	// find the direction of the eye-axis from that point
	// along the edge plane
	c = (ab_eyedir * ab_edge) - aa;

	// find the intersection of the sphere with the ray going from
	// the plane outside the sphere toward the eye-axis.
	ac = aa.dot(c);
	c2 = c.dot(c);
	q = ( 0.0 - ac - sqrt( ac*ac - c2*(aa.dot(aa)-ab_sphere2 ))) / c2;
	V = aa+(c*q);
	V.normalize();
}


/** update current arcball rotation*/
void ArcBall::move(double ax, double ay)
{
	if(ab_planar)
	{
		planarCoords(ax,ay, ab_curr);
		if(ab_curr == ab_start) return;

		// d is motion since the last position
		d = ab_curr - ab_start;

		angle = d.VAbs();
		cosa2 = cos(angle/2.0);
		sina2 = sin(angle/2.0);
		// p is perpendicular to d
		p = (ab_out*d.x)-(ab_up*d.y);
		p.normalize();
		p *= sina2;
		Quat.set(cosa2, p.x, p.y, p.z);

		quatToMatrix(ab_next, Quat);
		quatNext(ab_quat,ab_last,ab_next);
		// planar style only ever relates to the last point
		quatCopy(ab_last,ab_quat);
		ab_start = ab_curr;
	} 
	else
	{
		sphereCoords(ax,ay, ab_curr);
		if(ab_curr == ab_start)
		{ 
			// avoid potential rare divide by tiny
			quatCopy(ab_quat,ab_last);
			return;
		}

		// use a dot product to get the angle between them
		// use a cross product to get the vector to rotate around

		cosa   = ab_start.dot(ab_curr);
		sina2  = sqrt((1.0 - cosa)*0.5);
		cosa2  = sqrt((1.0 + cosa)*0.5);
		angle = acos(cosa2)*180.0/PI;

		p = (ab_start*ab_curr);
		p.normalize();
		p *=sina2;
		Quat.set(cosa2, p.x, p.y, p.z);

		quatToMatrix(ab_next, Quat);

		// update the rotation matrix
		quatNext(ab_quat,ab_last,ab_next);
	}
}



/** reset the rotation matrix*/
void ArcBall::quatIdentity(float* q)
{
	q[0] =1; q[1] =0; q[2] =0; q[3] =0;
	q[4] =0; q[5] =1; q[6] =0; q[7] =0;
	q[8] =0; q[9] =0; q[10]=1; q[11]=0;
	q[12]=0; q[13]=0; q[14]=0; q[15]=1;
}


/** copy a rotation matrix*/
void ArcBall::quatCopy(float* dst, float* src)
{
	dst[0]=src[0]; dst[1]=src[1]; dst[2] =src[2];
	dst[4]=src[4]; dst[5]=src[5]; dst[6] =src[6];
	dst[8]=src[8]; dst[9]=src[9]; dst[10]=src[10];
}


/** convert the quaternion into a rotation matrix*/
void ArcBall::quatToMatrix(float* mat, Quaternion Qt)
{
	x2 = Qt.qx*Qt.qx;
	y2 = Qt.qy*Qt.qy;
	z2 = Qt.qz*Qt.qz;
	xy = Qt.qx*Qt.qy;
	xz = Qt.qx*Qt.qz;
	yz = Qt.qy*Qt.qz;
	wx = Qt.a*Qt.qx;
	wy = Qt.a*Qt.qy;
	wz = Qt.a*Qt.qz;

	mat[0] = (float)(1 - 2*y2 - 2*z2);
	mat[1] = (float)(2*xy + 2*wz);
	mat[2] = (float)(2*xz - 2*wy);

	mat[4] = (float)(2*xy - 2*wz);
	mat[5] = (float)(1 - 2*x2 - 2*z2);
	mat[6] = (float)(2*yz + 2*wx);

	mat[8] = (float)(2*xz + 2*wy);
	mat[9] = (float)(2*yz - 2*wx);
	mat[10]= (float)(1 - 2*x2 - 2*y2);

	mat[11] = 0.0;
	mat[12] = 0.0;
	mat[13] = 0.0;
	mat[14] = 0.0;
	mat[15] = 1.0;
}


void ArcBall::matToQuat(Quaternion &qt, float *mat)
{
	qt.a = sqrt(1.0 + mat[0] + mat[5] + mat[10]) / 2.0;
	double w4 = (4.0 * qt.a);
	qt.qx = (mat[8] - mat[6]) / w4;
	qt.qy = (mat[2] - mat[8]) / w4;
	qt.qz = (mat[4] - mat[1]) / w4;
}


void ArcBall::matToQuat(Quaternion &qt, double *mat)
{
	qt.a = sqrt(1.0 + mat[0] + mat[5] + mat[10]) / 2.0;
	double w4 = (4.0 * qt.a);
	qt.qx = (mat[8] - mat[6]) / w4;
	qt.qy = (mat[2] - mat[8]) / w4;
	qt.qz = (mat[4] - mat[1]) / w4;
}


/** multiply two rotation matrices*/
void ArcBall::quatNext(float* dest, float* left, float* right)
{
	dest[0] = left[0]*right[0] + left[1]*right[4] + left[2] *right[8];
	dest[1] = left[0]*right[1] + left[1]*right[5] + left[2] *right[9];
	dest[2] = left[0]*right[2] + left[1]*right[6] + left[2] *right[10];
	dest[4] = left[4]*right[0] + left[5]*right[4] + left[6] *right[8];
	dest[5] = left[4]*right[1] + left[5]*right[5] + left[6] *right[9];
	dest[6] = left[4]*right[2] + left[5]*right[6] + left[6] *right[10];
	dest[8] = left[8]*right[0] + left[9]*right[4] + left[10]*right[8];
	dest[9] = left[8]*right[1] + left[9]*right[5] + left[10]*right[9];
	dest[10]= left[8]*right[2] + left[9]*right[6] + left[10]*right[10];
	dest[11] = 0.0;
	dest[12] = 0.0;
	dest[13] = 0.0;
	dest[14] = 0.0;
	dest[15] = 1.0;
}


/** reset the arcball*/
void ArcBall::reset()
{
	quatIdentity(ab_quat);
	quatIdentity(ab_last);
}


void ArcBall::rotateCrossPoint()
{
	aa.set(1.0, 0.0, 0.0);

	cosa   = aa.dot(ab_curr);
	sina2  = sqrt((1.0 - cosa)*0.5);
	cosa2  = sqrt((1.0 + cosa)*0.5);
	angle = 2.0*acos(cosa2)*180.0/PI;

	p = aa * ab_curr;
	p.normalize();
}


void ArcBall::quat(double roll, double pitch, double yaw, Quaternion &qt)
{
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
	qt.set(w,x,y,z);
}



void ArcBall::setQuat(Quaternion Qt)
{
	if(qAbs(Qt.a)<=1.0) angle = 2.0*acos(Qt.a) *  180.0/PI;
	Quat.a  = Qt.a;

	Quat.qx = Qt.qx;
	Quat.qy = Qt.qy;
	Quat.qz = Qt.qz;

	quatToMatrix(ab_quat, Quat);
}


void ArcBall::setQuat(double r, double qx, double qy, double qz)
{
	if(qAbs(r)<=1.0) angle = 2.0*acos(r) *  180.0/PI;
	Quat.a  = r;

	Quat.qx = qx;
	Quat.qy = qy;
	Quat.qz = qz;

	quatToMatrix(ab_quat, Quat);
}


void ArcBall::setZoom(double radius, Vector3d eye, Vector3d up)
{
	ab_eye     = eye; // store eye vector
	ab_zoom2   = ab_eye.dot(ab_eye);
	ab_zoom    = sqrt(ab_zoom2); // store eye distance
	ab_sphere  = radius;         // sphere radius
	ab_sphere2 = ab_sphere * ab_sphere;
	ab_eyedir  = ab_eye * (1.0 / ab_zoom); // distance to eye
	ab_edge    = ab_sphere2 / ab_zoom; // plane of visible edge

	if(ab_sphere <= 0.0) // trackball mode
	{
		ab_planar = true;
		ab_up = up;
		ab_out = (ab_eyedir * ab_up);
		ab_planedist = (0.0 - ab_sphere) * ab_zoom;
	}
	else
		ab_planar = false;
}


/** begin arcball rotation*/
void ArcBall::start(double ax, double ay)
{
	// saves a copy of the current rotation for comparison
	quatCopy(ab_last,ab_quat);
	if(ab_planar)
		planarCoords(ax, ay, ab_start);
	else
		sphereCoords(ax, ay, ab_start);
	ab_curr = ab_start;
}


void ArcBall::planarCoords(double const &ax, double const &ay, Vector3d &V)
{
	m.set(ax- ab_eye.x, ay- ab_eye.y, az- ab_eye.z);
	// intersect the point with the trackball plane
	t = (ab_planedist - ab_zoom)*1.0 / (ab_eyedir.dot(m));
	d = ab_eye + m*t;

	V.set(d.dot(ab_up),d.dot(ab_out),0.0);
}


void ArcBall::sphereCoords(double const &ax, double const &ay, Vector3d &V)
{
	// find the intersection with the sphere

	if(ab_sphere2>ax*ax+ay*ay) V.set(ax,ay,sqrt(ab_sphere2-ax*ax-ay*ay));
	else                       V.set(ax,ay,0.0);
//	else return EdgeCoords(ax, ay);

	V.normalize();
}







