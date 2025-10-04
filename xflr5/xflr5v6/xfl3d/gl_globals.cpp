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

#include <QVector>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>

#include "gl_globals.h"

#include <xfl3d/controls/w3dprefs.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflgeom/geom3d/segment3d.h>
#include <xflgeom/geom3d/triangle3d.h>

#define PIf    3.141592654f
#define NPOINTS 300

double t_lmin(0), t_range(0);
QVector<QVector<Segment3d>> t_futuresegs;

void glMakeTetra(Vector3d const &pt, double side, QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges)
{
    Vector3d vtx[4];
    vtx[0] = { sqrt(8.0/9.0)*side,                   0, -1.0/3.0*side};
    vtx[1] = {-sqrt(2.0/9.0)*side,  sqrt(2.0/3.0)*side, -1.0/3.0*side};
    vtx[2] = {-sqrt(2.0/9.0)*side, -sqrt(2.0/3.0)*side, -1.0/3.0*side};
    vtx[3] = { 0.0,                                0.0,          side};

    for(int i=0; i<4; i++) vtx[i] += pt;

    Vector3d N;

    GLfloat b[72];
    int iv=0;
    int i(0), j(2), k(1);
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    i=0; j=3; k=2;
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    i=2; j=3; k=1;
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    i=1; j=3; k=0;
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k].coord(l));
    for(int l=0; l<3; l++) b[iv++]=float(N.coord(l));

    if(vboFaces.isCreated()) vboFaces.destroy(); // recreate it in the VAO
    vboFaces.create();
    vboFaces.bind();
    {
        vboFaces.allocate(b, 72*sizeof(float));
    }
    vboFaces.release();

    int segsize = 4 * 2 *3; //4 segments x 2 vertices x3 components;
    iv=0;
    // first
    b[iv++] = vtx[0].xf();
    b[iv++] = vtx[0].yf();
    b[iv++] = vtx[0].zf();
    b[iv++] = vtx[1].xf();
    b[iv++] = vtx[1].yf();
    b[iv++] = vtx[1].zf();
    // second
    b[iv++] = vtx[1].xf();
    b[iv++] = vtx[1].yf();
    b[iv++] = vtx[1].zf();
    b[iv++] = vtx[2].xf();
    b[iv++] = vtx[2].yf();
    b[iv++] = vtx[2].zf();
    // third
    b[iv++] = vtx[2].xf();
    b[iv++] = vtx[2].yf();
    b[iv++] = vtx[2].zf();
    b[iv++] = vtx[3].xf();
    b[iv++] = vtx[3].yf();
    b[iv++] = vtx[3].zf();
    // fourth
    b[iv++] = vtx[0].xf();
    b[iv++] = vtx[0].yf();
    b[iv++] = vtx[0].zf();
    b[iv++] = vtx[3].xf();
    b[iv++] = vtx[3].yf();
    b[iv++] = vtx[3].zf();

    if(vboEdges.isCreated()) vboEdges.destroy(); // recreate it in the VAO
    vboEdges.create();
    vboEdges.bind();
    {
        vboEdges.allocate(b, segsize*sizeof(float));
    }
    vboEdges.release();
}



void glMakeCube(Vector3d const &pt, double dx, double dy, double dz,
                QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges)
{
    // 12 triangles
    // 3 vertices/triangle
    // (3 position + 3 normal) components/vertex
    int buffersize = 12 *3 * 6;
    QVector<GLfloat> CubeVertexArray(buffersize, 0);

    // 8 vertices
    Vector3d T000 = {pt.x-dx/2, pt.y-dy/2, pt.z-dz/2};
    Vector3d T001 = {pt.x-dx/2, pt.y-dy/2, pt.z+dz/2};
    Vector3d T010 = {pt.x-dx/2, pt.y+dy/2, pt.z-dz/2};
    Vector3d T011 = {pt.x-dx/2, pt.y+dy/2, pt.z+dz/2};
    Vector3d T100 = {pt.x+dx/2, pt.y-dy/2, pt.z-dz/2};
    Vector3d T101 = {pt.x+dx/2, pt.y-dy/2, pt.z+dz/2};
    Vector3d T110 = {pt.x+dx/2, pt.y+dy/2, pt.z-dz/2};
    Vector3d T111 = {pt.x+dx/2, pt.y+dy/2, pt.z+dz/2};

    Vector3d N;

    int iv = 0;
    // X- face
    N.set(-1,0,0);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    //   second triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();


    // X+ face
    N.set(1,0,0);
    //   first triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    //   second triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Y- face
    N.set(0,-1,0);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Y+ face
    N.set(0,1,0);
    //   first triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();


    // Z- face
    N.set(0,0,-1);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Z+ face
    N.set(0,0,1);
    //   first triangle
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    Q_ASSERT(iv==buffersize);

    vboFaces.destroy();
    vboFaces.create();
    vboFaces.bind();
    vboFaces.allocate(CubeVertexArray.data(), buffersize* int(sizeof(GLfloat)));
    vboFaces.release();

    buffersize = 12 * 2 *3; //12 edges x2 vertices x3 components
    QVector<float> EdgeVertexArray(buffersize);
    iv=0;

    //bottom face
    {
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();

        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();
        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();

        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();
        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();

        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
    }

    //top face
    {
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();
        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();

        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();
        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();

        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();
        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();

        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();
    }

    //lateral edges
    {
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();

        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();
        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();

        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();
        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();

        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();
        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();
    }

    Q_ASSERT(iv==buffersize);

    vboEdges.destroy();
    vboEdges.create();
    vboEdges.bind();
    vboEdges.allocate(EdgeVertexArray.data(), buffersize* int(sizeof(GLfloat)));
    vboEdges.release();
}

void glMakeTriangle(Triangle3d const &t3d, QOpenGLBuffer &vbo){glMakeTriangle(t3d.vertexAt(0), t3d.vertexAt(1), t3d.vertexAt(2), vbo);}
void glMakeTriangle(Vector3d *V, QOpenGLBuffer &vbo){glMakeTriangle(V[0], V[1], V[2], vbo);}



void glMakeTriangle(Vector3d const &V0, Vector3d const &V1, Vector3d const &V2, QOpenGLBuffer &vbo)
{
    QVector<GLfloat> TriangleVertexArray(12);

    int iv = 0;

    TriangleVertexArray[iv++] = V0.xf();
    TriangleVertexArray[iv++] = V0.yf();
    TriangleVertexArray[iv++] = V0.zf();

    TriangleVertexArray[iv++] = V1.xf();
    TriangleVertexArray[iv++] = V1.yf();
    TriangleVertexArray[iv++] = V1.zf();

    TriangleVertexArray[iv++] = V2.xf();
    TriangleVertexArray[iv++] = V2.yf();
    TriangleVertexArray[iv++] = V2.zf();

    //close the triangle
    TriangleVertexArray[iv++] = V0.xf();
    TriangleVertexArray[iv++] = V0.yf();
    TriangleVertexArray[iv++] = V0.zf();

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(TriangleVertexArray.data(), 12 * sizeof(GLfloat));
    vbo.release();
}


void glMakeQuadContoursOnGrid(QOpenGLBuffer &vbo, int nrows, int ncols,
                                           QVector<Vector3d> const&node, QVector<double> const &value,
                                           bool bMultithreaded)
{
    if(node.count()<4)            {vbo.destroy();  return;}
    if(node.size()!=nrows*ncols)  {vbo.destroy();  return;}
    if(value.size()!=nrows*ncols) {vbo.destroy();  return;}

    // find min and max Cp for scale set
    float lmin =  1000000.0f;
    float lmax = -1000000.0f;

    float coef = 1.0f;

    for(int p=0; p<value.count(); p++)
    {
        lmin = std::min(lmin, float(value.at(p))*coef);
        lmax = std::max(lmax, float(value.at(p))*coef);
    }

    float range = lmax - lmin;

    //define the threshold values for the contours
    int nContours = 20;
    QVector<float> contour(nContours);
    for(int ic=0; ic<nContours; ic++) contour[ic] = lmin + float(ic)/float(nContours-1)*range;

    QVector<Segment3d> segs;

    t_futuresegs.resize(nContours);

    if(bMultithreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for(int ic=0; ic<nContours; ic++)
        {
            t_futuresegs[ic].clear();
            t_futuresegs[ic].reserve((nrows-1)*(ncols-1)*4);

            futureSync.addFuture(QtConcurrent::run(&makeQuadContour,
                                                   contour.at(ic), nrows, node, value,
                                                   ic));
        }
        futureSync.waitForFinished();
        for(int iseg=0; iseg<t_futuresegs.size(); iseg++)
            segs.append(t_futuresegs.at(iseg));
    }
    else
    {
        for(int ic=0; ic<nContours; ic++)
        {
            t_futuresegs[ic].clear();
            t_futuresegs[ic].reserve((nrows-1)*(ncols-1)*4);
            makeQuadContour(contour.at(ic), nrows, node, value, ic);
            segs.append(t_futuresegs[ic]);
        }
    }

    // vertex array size
    // nsegs
    // x 2 vertices
    // x 3 components
    int nodeVertexSize = segs.size() * 2 * 3;
    QVector<float> nodeVertexArray(nodeVertexSize);

    int iv=0;
    for(int is=0; is<segs.size(); is++)
    {
        Node const & n0 = segs.at(is).vertexAt(0);
        Node const & n1 = segs.at(is).vertexAt(1);
        nodeVertexArray[iv++] = n0.xf();
        nodeVertexArray[iv++] = n0.yf();
        nodeVertexArray[iv++] = n0.zf();
        nodeVertexArray[iv++] = n1.xf();
        nodeVertexArray[iv++] = n1.yf();
        nodeVertexArray[iv++] = n1.zf();
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void makeQuadContour(double threshold, int nrows,
                                        QVector<Vector3d> const&node, QVector<double> const &value,
                                        int ic)
{
    QVector<Segment3d> &contoursegs = t_futuresegs[ic];

    int idx[] = {0,0,0,0};
    int ncols = node.size()/nrows;
    double tau = 0.0;
    int ik[] = {-1, -1, -1, -1, -1, -1, -1, -1};
    Vector3d I[2]; // crossover points on edge

    int i0=0, i1=0, i2=0, i3=0;
    int key=0, k=1;

    for(int r=0; r<nrows-1; r++)
    {
        for(int c=0; c<ncols-1; c++)
        {
            // for each cell
            idx[0] =  r   *ncols+c;
            idx[1] =  r   *ncols+c+1;
            idx[2] = (r+1)*ncols+c+1;
            idx[3] = (r+1)*ncols+c;

            // check for crossover of contour value
            // use base 2 key as table index
            key = 0;
            k=1;
            for(int i=0; i<4; i++)
            {
                if(value[idx[i%4]]-threshold<0) // true if there is a crossover
                {
                    key += k;
                }
                k *= 2;
            }

            lookUpQuadKey(key, ik);

            for(int jk=0; jk<2; jk++)
            {
                i0 = ik[4*jk+0];
                i1 = ik[4*jk+1];
                i2 = ik[4*jk+2];
                i3 = ik[4*jk+3];
                if(i0>=0 && i1>=0 && i2>=0 && i3>=0)
                {
                    tau = (threshold - value[idx[i0]]) /(value[idx[i1]]-value[idx[i0]]);
                    I[0].x = node[idx[i0]].x*(1-tau) + node[idx[i1]].x*tau;
                    I[0].y = node[idx[i0]].y*(1-tau) + node[idx[i1]].y*tau;
                    I[0].z = node[idx[i0]].z*(1-tau) + node[idx[i1]].z*tau;
                    tau = (threshold - value[idx[i2]]) /(value[idx[i3]]-value[idx[i2]]);
                    I[1].x = node[idx[i2]].x*(1-tau) + node[idx[i3]].x*tau;
                    I[1].y = node[idx[i2]].y*(1-tau) + node[idx[i3]].y*tau;
                    I[1].z = node[idx[i2]].z*(1-tau) + node[idx[i3]].z*tau;
                    contoursegs.push_back({I[0], I[1]});
                }
            }
        }
    }
}



void lookUpQuadKey(int key, int *i)
{
    // 2^4 = 16 crossover configurations
    //
    //    N3------N2           2^3------2^2
    //    |       |             |       |
    //    |       |             |       |
    //    |       |             |       |
    //    N0------N1           2^0------2^1
    //
    i[0] = i[1] = i[2] = i[3] = i[4] = i[5] = i[6] = i[7] = -1;

    switch(key)
    {
        case 0: break;
        case 1:  // 2^0
        {
            i[0]=0;
            i[1]=1;

            i[2]=0;
            i[3]=3;
            break;
        }
        case 2:  // 2^1
        {
            i[0]=0;
            i[1]=1;

            i[2]=1;
            i[3]=2;
            break;
        }
        case 3:  // =2^0 + 2^1
        {
            i[0]=0;
            i[1]=3;

            i[2]=1;
            i[3]=2;
            break;
        }
        case 4:  // =2^2
        {
            i[0]=1;
            i[1]=2;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 5:  // =2^0 + 2^2
        {
            i[0]=0;
            i[1]=1;

            i[2]=3;
            i[3]=0;

            i[4]=1;
            i[5]=2;

            i[6]=2;
            i[7]=3;
            break;
        }
        case 6:  // =2^1 + 2^2
        {
            i[0]=0;
            i[1]=1;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 7:  // =2^0 + 2^1 + 2^2
        {
            i[0]=2;
            i[1]=3;

            i[2]=3;
            i[3]=0;
            break;
        }
        case 8:  // =2^3
        {
            i[0]=2;
            i[1]=3;

            i[2]=3;
            i[3]=0;
            break;

        }
        case 9:  // =2^0 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 10: // =2^1 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=1;
            i[3]=2;

            i[4]=0;
            i[5]=3;

            i[6]=2;
            i[7]=3;
            break;
        }
        case 11: // =2^0 + 2^1 + 2^3
        {
            i[0]=1;
            i[1]=2;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 12: // =2^2 + 2^3
        {
            i[0]=0;
            i[1]=3;

            i[2]=1;
            i[3]=2;

            break;
        }
        case 13: // =2^0 + 2^2 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=1;
            i[3]=2;
            break;
        }
        case 14: // =2^1 + 2^2 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=0;
            i[3]=3;
            break;
        }
        case 15: break;// =2^0 + 2^1 + 2^2 + 2^3
    }
}



void glMakeEllipseLineStrip(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS;
    arcbuffersize *= 3; // 3 components per vertex for the surface shader

    // NPOINTS-1 triangles

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);

    Vector3d C(O.x+a*e, O.y, 0.0);
    double b = a * sqrt(1.0-e*e);
    int iv = 0;

    for(int i=0; i<NPOINTS; i++)
    {
        float t = float(i)/float(NPOINTS-1);
        float x = float(a * cosf(2.0*PIf*t));
        float y = float(b * sinf(2.0*PIf*t));
        ArcVertexArray[iv++] = C.xf()+x;
        ArcVertexArray[iv++] = C.yf()+y;
        ArcVertexArray[iv++] = C.zf();
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


void glMakeEllipseFan(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS;
    arcbuffersize += 1; // 1 central vertex at the focus point
    arcbuffersize *= 6; // 3+3 components per vertex for the surface shader

    // NPOINTS-1 triangles

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);

    Vector3d C(O.x+a*e, O.y, 0.0);
    double b = a * sqrt(1.0-e*e);
    int iv = 0;
    //set the fixed central vertex at the focus point
    ArcVertexArray[iv++] = C.xf();
    ArcVertexArray[iv++] = C.yf();
    ArcVertexArray[iv++] = C.zf();
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 1.0f;

    for(int i=0; i<NPOINTS; i++)
    {
        float t = float(i)/float(NPOINTS-1);
        float x = float(a * cosf(2.0f*PIf*t));
        float y = float(b * sinf(2.0f*PIf*t));
        ArcVertexArray[iv++] = C.xf()+x;
        ArcVertexArray[iv++] = C.yf()+y;
        ArcVertexArray[iv++] = C.zf();
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 1.0f;
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


void glMakeTriangles3Vtx(QVector<Triangle3d> const &triangles, bool bFlatNormals, QOpenGLBuffer &vbo)
{
    //Make surface triangulation
    int bufferSize = triangles.count();
    bufferSize *= 3;    // 4 vertices for each triangle
    bufferSize *= 6;    // (3 coords+3 normal components) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N;

    int iv = 0;
    for(int it=0; it<triangles.size(); it++)
    {
        Triangle3d const &t3d = triangles.at(it);
        N.set(t3d.normal());

        meshvertexarray[iv++] = t3d.vertexAt(0).xf();
        meshvertexarray[iv++] = t3d.vertexAt(0).yf();
        meshvertexarray[iv++] = t3d.vertexAt(0).zf();
        if(bFlatNormals)
        {
            meshvertexarray[iv++] = N.xf();
            meshvertexarray[iv++] = N.yf();
            meshvertexarray[iv++] = N.zf();
        }
        else
        {
            meshvertexarray[iv++] = t3d.vertexAt(0).normal().xf();
            meshvertexarray[iv++] = t3d.vertexAt(0).normal().yf();
            meshvertexarray[iv++] = t3d.vertexAt(0).normal().zf();
        }

        meshvertexarray[iv++] = t3d.vertexAt(1).xf();
        meshvertexarray[iv++] = t3d.vertexAt(1).yf();
        meshvertexarray[iv++] = t3d.vertexAt(1).zf();
        if(bFlatNormals)
        {
            meshvertexarray[iv++] = N.xf();
            meshvertexarray[iv++] = N.yf();
            meshvertexarray[iv++] = N.zf();
        }
        else
        {
            meshvertexarray[iv++] = t3d.vertexAt(1).normal().xf();
            meshvertexarray[iv++] = t3d.vertexAt(1).normal().yf();
            meshvertexarray[iv++] = t3d.vertexAt(1).normal().zf();
        }


        meshvertexarray[iv++] = t3d.vertexAt(2).xf();
        meshvertexarray[iv++] = t3d.vertexAt(2).yf();
        meshvertexarray[iv++] = t3d.vertexAt(2).zf();
        if(bFlatNormals)
        {
            meshvertexarray[iv++] = N.xf();
            meshvertexarray[iv++] = N.yf();
            meshvertexarray[iv++] = N.zf();
        }
        else
        {
            meshvertexarray[iv++] = t3d.vertexAt(2).normal().xf();
            meshvertexarray[iv++] = t3d.vertexAt(2).normal().yf();
            meshvertexarray[iv++] = t3d.vertexAt(2).normal().zf();
        }
    }

    Q_ASSERT(iv==bufferSize);

    if(vbo.isCreated()) vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void glMakeTrianglesOutline(QVector<Triangle3d> const &triangles, Vector3d const &position, QOpenGLBuffer &vbo)
{
    int nPanel3 = triangles.size();
    // vertices array size:
    //		n triangular Panels
    //      x3 edges
    //      x2 nodes per edges
    //		x3 vertex components

    int buffersize = nPanel3*3*2*3;

    QVector<float> nodeVertexArray(buffersize);

    int iv = 0;
    for (int i3=0; i3<nPanel3; i3++)
    {
        Triangle3d const &t3d = triangles.at(i3);

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = t3d.vertexAt(i).xf() + position.xf();
            nodeVertexArray[iv++] = t3d.vertexAt(i).yf() + position.yf();
            nodeVertexArray[iv++] = t3d.vertexAt(i).zf() + position.zf();

            nodeVertexArray[iv++] = t3d.vertexAt(i+1).xf() + position.xf();
            nodeVertexArray[iv++] = t3d.vertexAt(i+1).yf() + position.yf();
            nodeVertexArray[iv++] = t3d.vertexAt(i+1).zf() + position.zf();
        }
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void glMakeQuadTex(double side, QOpenGLBuffer &vbo)
{
    Node vtx[4];

    vtx[0].set(-side,-side, 0);  vtx[0].setNormal(0,0,1);
    vtx[1].set( side,-side, 0);  vtx[2].setNormal(0,0,1);
    vtx[2].set( side, side, 0);  vtx[1].setNormal(0,0,1);
    vtx[3].set(-side, side, 0);  vtx[3].setNormal(0,0,1);
    QVector<Triangle3d> triangles(2);
    triangles.first().setTriangle(vtx[0], vtx[1], vtx[2]);
    triangles.last().setTriangle(vtx[0], vtx[2], vtx[3]);

    //Make surface triangulation
    int bufferSize = triangles.count();
    bufferSize *= 3;    // 4 vertices for each triangle
    bufferSize *= 8;    // (3 coords+3 normal components+2UVcomponents) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N;
    bool bFlatNormals = true;
    int iv = 0;
    for(int it=0; it<triangles.size(); it++)
    {
        Triangle3d const &t3d = triangles.at(it);
        N.set(t3d.normal());

        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Node const &vertex = t3d.vertexAt(ivtx);

            meshvertexarray[iv++] = vertex.xf();
            meshvertexarray[iv++] = vertex.yf();
            meshvertexarray[iv++] = vertex.zf();
            if(bFlatNormals)
            {
                meshvertexarray[iv++] = N.xf();
                meshvertexarray[iv++] = N.yf();
                meshvertexarray[iv++] = N.zf();
            }
            else
            {
                meshvertexarray[iv++] = vertex.normal().xf();
                meshvertexarray[iv++] = vertex.normal().yf();
                meshvertexarray[iv++] = vertex.normal().zf();
            }
            meshvertexarray[iv++] = (side-vertex.xf())/side/2.0; // U component, equal to x
            meshvertexarray[iv++] = (vertex.yf()-side)/side/2.0; // V component, equal to y
        }
    }

    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void glMakeCircle(double radius, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS-1; // 1 segment less than the number of points
    arcbuffersize *= 2; // two vertices per segment
    arcbuffersize *= 3; // three components per vertex

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);


    int iv = 0;

    for(int i=0; i<NPOINTS-1; i++)
    {
        float theta  = float(i)   * 2.0f*PIf/float(NPOINTS-1);
        float theta1 = float(i+1) * 2.0f*PIf/float(NPOINTS-1);
        ArcVertexArray[iv++] = O.xf()+radius*cosf(theta);
        ArcVertexArray[iv++] = O.yf()+radius*sinf(theta);
        ArcVertexArray[iv++] = O.zf();
        ArcVertexArray[iv++] = O.xf()+radius*cosf(theta1);
        ArcVertexArray[iv++] = O.yf()+radius*sinf(theta1);
        ArcVertexArray[iv++] = O.zf();
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


void glMakeDisk(double radius, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS;
    arcbuffersize += 1; // 1 central vertex
    arcbuffersize *= 6; // 3+3 components per vertex

    // NPOINTS-1 triangles

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);

    int iv = 0;
    //set the fixed central vertex
    ArcVertexArray[iv++] = O.xf();
    ArcVertexArray[iv++] = O.yf();
    ArcVertexArray[iv++] = O.zf();
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 1.0f;
    for(int i=0; i<NPOINTS; i++)
    {
        float theta  = float(i)   * 2.0f*PI/float(NPOINTS-1);
        ArcVertexArray[iv++] = O.xf()+radius*cosf(theta);
        ArcVertexArray[iv++] = O.yf()+radius*sinf(theta);
        ArcVertexArray[iv++] = O.zf();
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 1.0f;
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}

