/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/




#include "segment2d.h"
#include <xflgeom/geom2d/triangle2d.h>
#include <xflcore/constants.h>
#include <xflgeom/geom_globals.h>

Segment2d::Segment2d()
{
    m_Vtx0.set(0.0,0.0);
    m_Vtx1.set(0.0,0.0);
    m_bSplittable = true;
}


Segment2d::Segment2d(Vector2d vtx0, Vector2d vtx1)
{
    m_Vtx0=vtx0;
    m_Vtx1=vtx1;
    m_bSplittable = true;
}


Segment2d::Segment2d(double x0, double y0, double x1, double y1)
{
    m_Vtx0=Vector2d(x0, y0);
    m_Vtx1=Vector2d(x1, y1);
    m_bSplittable = true;
}


bool Segment2d::isSame(Segment2d otheredge, double precision) const
{
    if(otheredge.m_Vtx0.isSame(m_Vtx0, precision) && otheredge.m_Vtx1.isSame(m_Vtx1, precision)) return true;
    if(otheredge.m_Vtx0.isSame(m_Vtx1, precision) && otheredge.m_Vtx1.isSame(m_Vtx0, precision)) return true;
    return false;
}


bool Segment2d::isEncroachedBy(Vector2d const &pt) const
{
    if(m_Vtx0.isSame(pt) || m_Vtx1.isSame(pt)) return false;

    Vector2d MidPoint = (m_Vtx0+m_Vtx1)/2.0;
    double d2 = (pt.x-MidPoint.x)*(pt.x-MidPoint.x) + (pt.y-MidPoint.y)*(pt.y-MidPoint.y);
    Vector2d R = (m_Vtx1-m_Vtx0)/2.0;
    double r2 = R.x*R.x + R.y*R.y;
    return d2<r2-PRECISION;
}

/** shold'nt be needed */
bool Segment2d::isEncroachedBy(Triangle2d const &t2d) const
{
    Vector2d CC;
    double r;
    t2d.circumCenter(r, CC);
    return isEncroachedBy(CC);
}


/** returns a list made of the two half segments */
QVector<Segment2d> Segment2d::split() const
{
    QVector<Segment2d> splitList;
    Vector2d midPoint = (m_Vtx0+m_Vtx1)/2.0;
    splitList.push_back({m_Vtx0, midPoint});
    splitList.push_back({midPoint, m_Vtx1});
    return splitList;
}


void Segment2d::displayNodes() const
{
    qDebug(" {Vector2d(  %9.5g, %9.5g  ),  Vector2d(  %9.5g, %9.5g  )} ", m_Vtx0.x, m_Vtx0.y, m_Vtx1.x, m_Vtx1.y);
}


QString Segment2d::properties(bool bLong, QString prefix) const
{
    QString props, strong;
    strong = QString::asprintf("V0 = ( %7g, %7g )\n", m_Vtx0.x, m_Vtx0.y);
    props = prefix + strong;
    strong = QString::asprintf("V1 = ( %7g, %7g )\n", m_Vtx1.x, m_Vtx1.y);
    props += prefix + strong;

    if(!bLong) return props;

    props += "\n";

    strong = QString::asprintf("CoG = ( %9g, %9g )\n", midPoint().x, midPoint().y);
    props += prefix + strong;

    strong = QString::asprintf("U   = ( %9g, %9g )\n", unitDir().x, unitDir().y);
    props += prefix + strong;

    strong = QString::asprintf("Length = %g", length());
    props += prefix + strong;

    return props;
}


// endpoints coincidences are not considered intersections
bool Segment2d::intersects(Segment2d const &seg, Vector2d &IPt, bool bPointsInside, double precision) const
{
    return intersects(seg.m_Vtx0, seg.m_Vtx1, IPt, bPointsInside, precision);
}


// endpoints coincidences are not considered intersections
bool Segment2d::intersects(Vector2d const &Pt0, Vector2d const&Pt1, Vector2d &IPt, bool bPointsInside, double precision) const
{
    return intersectSegment(m_Vtx0, m_Vtx1, Pt0, Pt1, IPt, bPointsInside, precision);
}


/**
 * returns true if the point lies on the segment, false otherwise.
 */
bool Segment2d::isOnSegment(Vector2d const &pt, double precision) const
{
    return isOnSegment(pt.x, pt.y, precision);
}


bool Segment2d::isOnSegment(double x, double y, double precision) const
{
    double dxs = m_Vtx1.x-m_Vtx0.x;
    double dys = m_Vtx1.y-m_Vtx0.y;
    double dx0 = x-m_Vtx0.x;
    double dy0 = y-m_Vtx0.y;
    double dx1 = x-m_Vtx1.x;
    double dy1 = y-m_Vtx1.y;

    // return false if the distance of Pt(x,y) to the line is greater than precision
    double l = length();
    double proj = (dxs*dx0+dys*dy0)/l;
    double rx0 = dxs/l * proj;
    double ry0 = dys/l * proj;
    double hx = dx0-rx0;
    double hy = dy0-ry0;
    if(sqrt(hx*hx+hy*hy)>precision) return false;

    //return false if the point is on the line but outside the segment
    if(dx0*dxs+dy0*dys<0.0) return false;
    if(dx1*dxs+dy1*dys>0.0) return false;

    // on the line and within the segment
    return true;
}


bool Segment2d::isEndPoint(Vector2d const&pt, double precision) const
{
    if(m_Vtx0.isSame(pt, precision)) return true;
    if(m_Vtx1.isSame(pt, precision)) return true;
    return false;
}

