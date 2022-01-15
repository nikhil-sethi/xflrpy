/****************************************************************************

    GL_Globals
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
#include <QOpenGLBuffer>

class Segment3d;
class Triangle3d;
class Vector3d;

void glMakeCircle(double radius, Vector3d const &O, QOpenGLBuffer &vbo);
void glMakeDisk(double radius, Vector3d const &O, QOpenGLBuffer &vbo);

void glMakeTriangles3Vtx(const QVector<Triangle3d> &triangles, bool bFlatNormals, QOpenGLBuffer &vbo);
void glMakeTrianglesOutline(QVector<Triangle3d> const &triangles, const Vector3d &position, QOpenGLBuffer &vbo);

void glMakeTetra(Vector3d const &pt, double side, QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges);

void glMakeCube(Vector3d const &pt, double dx, double dy, double dz, QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges);

void glMakeTriangle(Vector3d const &V0, const Vector3d &V1, const Vector3d &V2, QOpenGLBuffer &vbo);
void glMakeTriangle(Triangle3d const &t3d, QOpenGLBuffer &vbo);
void glMakeTriangle(Vector3d *V, QOpenGLBuffer &vbo);

void glMakeEllipseLineStrip(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo);
void glMakeEllipseFan(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo);

void glMakeQuadContoursOnGrid(QOpenGLBuffer &vbo, int nrows, int ncols,
                              QVector<Vector3d> const&node, QVector<double> const &value,
                              bool bMultithreaded);

void makeQuadContour(double threshold, int nrows,
                     QVector<Vector3d> const&node, QVector<double> const &value,
                     int ic);

void lookUpQuadKey(int key, int *i);
void glMakeQuadTex(double side, QOpenGLBuffer &vbo);

/* external temp variables for multithreading  */
extern double t_lmin, t_range;
extern QVector<QVector<Segment3d>> t_futuresegs;
