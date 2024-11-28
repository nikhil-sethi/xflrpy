/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QVector>

#include <xflgeom/geom2d/vector2d.h>

class Triangle2d;

class Segment2d
{
    public:
        Segment2d();
        Segment2d(Vector2d vtx0, Vector2d vtx1);
        Segment2d(double x0, double y0, double x1, double y1);

        void setVertices(Vector2d vtx0, Vector2d vtx1) {m_Vtx0=vtx0; m_Vtx1=vtx1;}
        void setVertices(Vector2d *vtx) {m_Vtx0=vtx[0]; m_Vtx1=vtx[1];}
        void setVertex(int ivtx, Vector2d pt) {if(ivtx==0) m_Vtx0=pt; else if(ivtx==1) m_Vtx1=pt;}

        Vector2d const &vertexAt(int i) const {return i==0 ? m_Vtx0 : m_Vtx1;}
        Vector2d vertex(int i) {return i==0 ? m_Vtx0 : m_Vtx1;}

        Vector2d midPoint()    const {return (m_Vtx0+m_Vtx1)/2.0;}
        Vector2d unitDir()     const {return (m_Vtx1-m_Vtx0).normalized();}
        Vector2d segment()     const {return (m_Vtx1-m_Vtx0);}
        double length()        const {return (m_Vtx1-m_Vtx0).norm();}

        bool isSame(Segment2d otheredge, double precision = 1.e-6) const;

        bool isEncroachedBy(Vector2d const &pt) const;
        bool isEncroachedBy(Triangle2d const &t2d) const;

        QVector<Segment2d> split() const;

        bool intersects(const Vector2d &Pt0, const Vector2d &Pt1, Vector2d &IPt, bool bPointsInside, double precision) const;
        bool intersects(const Segment2d &seg, Vector2d &IPt, bool bPointsInside, double precision) const;

        bool isOnSegment(const Vector2d &pt, double precision=1.e-6) const;
        bool isOnSegment(double x, double y, double precision=1.e-6) const;

        bool isEndPoint(Vector2d const&pt, double precision=1.e-6) const;

        bool isSplittable() const {return m_bSplittable;}
        void setSplittable(bool bsplittable) {m_bSplittable=bsplittable;}

        void displayNodes() const;
        QString properties(bool bLong=true, QString prefix=QString()) const;

    private:
        Vector2d m_Vtx0, m_Vtx1;

        bool m_bSplittable; /** if true, the segment can be split during the refinement of the PSLG */
};












