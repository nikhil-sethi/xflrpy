/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QCheckBox>

#include <xfl3d/testgl/gl3dsurface.h>



class Stream2d;

class gl3dSurfacePlot: public gl3dSurface
{
    Q_OBJECT
    public:
        gl3dSurfacePlot(QWidget *pParent=nullptr);
        ~gl3dSurfacePlot() override;

        void clearVertices() {m_Vertices.clear();}
        void addVertex(Vector3d vtx) {m_Vertices.push_back(vtx);}
        void addVertex(double x, double y, double z) {m_Vertices.push_back({x,y,z});}

        void glMakePolygon();
        void glRenderView() override;
        void glMake3dObjects() override;
        void paintPolygon();

        void setTriangle(Vector3d* vertices);


    private:
        Vector3d m_TriangleVertex[3];
        QVector<Vector3d> m_Vertices;

        QOpenGLBuffer m_vboTriangle;
        QOpenGLBuffer m_vboPolygon;

        GLfloat m_panel[9]  = {
            -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f};

        double m_ValMin, m_ValMax;


        QCheckBox *m_pchGrid;
        QCheckBox *m_pchContour;
};

