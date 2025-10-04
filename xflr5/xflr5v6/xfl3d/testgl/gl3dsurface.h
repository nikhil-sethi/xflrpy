/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom2d/vector2d.h>
class gl3dSurface : public gl3dTestGLView
{
    public:
        gl3dSurface(QWidget *pParent=nullptr);
        QSize sizeHint() const override {return QSize(900,700);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const&, Vector3d const&, Vector3d &)  override {return false;}

    protected:
        void paintGrid();

        void glMakeSurface();
        void make3dPlane(double a, double b, double c);
        double makeTestSurface();
        double error(const Vector2d &pos, bool bMinimum) const;
        double function(double x, double y) const;

    protected:
        bool m_bGrid, m_bContour;
        double m_HalfSide;
        int m_Size_x, m_Size_y;
        QVector<Vector3d> m_PointArray; // Size x Size

        QOpenGLBuffer m_vboSurface, m_vboGrid;

        QOpenGLBuffer m_vboContourLines;

        bool m_bResetSurface;
        bool m_bDisplaySurface;
        bool m_bDoubleDipSurface;


        double m_ValMin, m_ValMax;

};


extern double c0, c1, c2; // random function coefficients



