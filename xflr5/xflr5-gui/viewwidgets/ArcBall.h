/****************************************************************************

    CArcBall Class
	Copyright (C)  Bradley Smith, March 24, 2006
	Hideously modified in 2008-2009 by Andre Deperrois adeperrois@xflr5.com for miserable selfish purposes

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

/* Arcball, written by Bradley Smith, March 24, 2006
 *
 * Using the arcball:
 *   Call arcball_setzoom after setting up the projection matrix.
 *
 *     The arcball, by default, will act as if a sphere with the given
 *     radius, centred on the origin, can be directly manipulated with
 *     the mouse. Clicking on a point should drag that point to rest under
 *     the current mouse position. eye is the position of the eye relative
 *     to the origin. up is unused.
 *
 *     Alternatively, pass the value: (-radius/|eye|)
 *     This puts the arcball in a mode where the distance the mouse moves
 *     is equivalent to rotation along the axes. This acts much like a
 *     trackball. (It is for this mode that the up vector is required,
 *     which must be a unit vector.)
 *
 *     You should call arcball_setzoom after use of gluLookAt.
 *     gluLookAt(eye.x,eye.y,eye.z, ?,?,?, up.x,up.y,up.z);
 *     The arcball derives its transformation information from the
 *     openGL projection and viewport matrices. (modelview is ignored)
 *
 *     If looking at a point different from the origin, the arcball will still
 *     act as if it centred at (0,0,0). (You can use this to translate
 *     the arcball to some other part of the screen.)
 *
 *   Call arcball_start with a mouse position, and the arcball will
 *     be ready to manipulate. (Call on mouse button down.)
 *   Call arcball_move with a mouse position, and the arcball will
 *     find the rotation necessary to move the start mouse position to
 *     the current mouse position on the sphere. (Call on mouse move.)
 *   Call arcball_rotate after resetting the modelview matrix in your
 *     drawing code. It will call glRotate with its current rotation.
 *   Call arcball_reset if you wish to reset the arcball rotation.
 */



/**
 *@file
 * This file contains the description of the Arcball class used for the calculation of rotations in 3D display.
 * Based on the code provided by Bradley Smith http://rainwarrior.ca/dragon/arcball.html
 *
 */


#ifndef ARBCALL_H
#define ARBCALL_H



#include <objects2d/Vector3d.h>
#include <objects3d/Quaternion.h>
#include <QRect>



/**
*@class ArcBall
*@brief
 * This class defines the Arcball object used for the calculation of rotations in 3D display.
 *
   Based on the code provided by Bradley Smith http://rainwarrior.ca/dragon/arcball.html
*/
class ArcBall
{
public:
	ArcBall(void);

	void setQuat(Quaternion Qt);
	void setQuat(double r, double qx, double qy, double qz);
	void setZoom(double radius, Vector3d eye, Vector3d up);
	void rotate();
	void reset();
	void start(double ax, double ay);
	void move(double ax, double ay);
	void getMatrix();
	void sphereCoords(double const &ax, double const &ay, Vector3d &V);// find the intersection with the sphere
	void planarCoords(double const &ax, double const &ay, Vector3d &V);// get intersection with plane for "trackball" style rotation
	void edgeCoords(Vector3d m, Vector3d &V);	// find the intersection with the plane through the visible edge
	void rotateCrossPoint();
	void quatIdentity(float* q);	// reset the rotation matrix
	void quatCopy(float* dst, float* src);// copy a rotation matrix
	void quatToMatrix(float* q, Quaternion Qt);// convert the quaternion into a rotation matrix
	void quatNext(float* dest, float* left, float* right);// multiply two rotation matrices

	float ab_quat[16];
	float ab_last[16];
	float ab_next[16];
	float ab_crosspoint[16];

	// the distance from the origin to the eye
	double ab_zoom;
	double ab_zoom2;
	// the radius of the arcball
	double ab_sphere;
	double ab_sphere2;
	// the distance from the origin of the plane that intersects
	// the edge of the visible sphere (tangent to a ray from the eye)
	double ab_edge;
	// whether we are using a sphere or plane
	bool ab_planar;
	double ab_planedist;

	Vector3d ab_start;
	Vector3d ab_curr;
	Vector3d ab_eye;
	Vector3d ab_eyedir;
	Vector3d ab_up;
	Vector3d ab_out;

	Quaternion Quat;
	double angle, cosa2, sina2, cosa;

//	double ab_glp[16];
//	double ab_glm[16];
//	int ab_glv[4];

	//	object offset
//	double *m_pOffx, *m_pOffy;
//	double *m_pTransx, *m_pTransy;
//	QRect *m_pRect;

	//avoid lengthy recurring memory allocations
	Vector3d aa, c, m, ec, sc, p, d;
	double t, ac, c2, q, b, delta, a;
	double x2, y2, z2, xy, xz, yz, wx, wy, wz;
	double ax,ay,az;
};
#endif
 

