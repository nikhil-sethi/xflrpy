/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/

#include "triangle2d.h"

#include <xflcore/constants.h>
#include <xflcore/matrix.h>
#include <xflgeom/geom_globals.h>

double Triangle2d::s_QualityBound=sqrt(2.0);

Triangle2d::Triangle2d()
{
    initialize();
}

/** The copy constructor, slow and useless */
/*Triangle2d::Triangle2d(Triangle2d const &triangle)
{
    initialize();
    m_S[0] = triangle.vertex(0);
    m_S[1] = triangle.vertex(1);
    m_S[2] = triangle.vertex(2);
    setTriangle();
}*/


Triangle2d::Triangle2d(Vector2d const &vtx0, Vector2d const &vtx1, Vector2d const &vtx2)
{
    initialize();
    m_S[0] = vtx0;
    m_S[1] = vtx1;
    m_S[2] = vtx2;
    setTriangle();
}


/** constructs the triangle's properties from its vertices; leave the orientation unchanged */
void Triangle2d::setTriangle(const Vector2d &S0, const Vector2d &S1, const Vector2d &S2)
{
    m_S[0].copy(S0);
    m_S[1].copy(S1);
    m_S[2].copy(S2);
    setTriangle();
}


void Triangle2d::setTriangle()
{
    S01 = m_S[1]-m_S[0];
    S02 = m_S[2]-m_S[0];
    S12 = m_S[2]-m_S[1];

    // the index of an edge is the index of the opposite vertex
    m_Edge[0].setVertices(m_S[1],m_S[2]);
    m_Edge[1].setVertices(m_S[2],m_S[0]);
    m_Edge[2].setVertices(m_S[0],m_S[1]);

    CoG_G = (m_S[0]+m_S[1]+m_S[2])/3.0;

    m_CF.setOrigin(CoG_G);
    Vector2d u = m_Edge[2].unitDir();
    m_CF.setI(u);


    if(S01.norm()<LENGTHPRECISION || S02.norm()<LENGTHPRECISION || S12.norm()<LENGTHPRECISION)
    {
        // one null side
        m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }

    m_bNullTriangle = false;

    m_SignedArea = (S01.x*S02.y - S01.y*S02.x) / 2.0; /** @todo check sign */

    m_Angle[0] = acos( S01.dot(S02)/S01.norm()/S02.norm()) * 180.0/PI;
    m_Angle[1] = acos(-S01.dot(S12)/S01.norm()/S12.norm()) * 180.0/PI;
//    m_Angle[2] = acos( S02.dot(S12)/S02.norm()/S12.norm()) * 180.0/PI;
    m_Angle[2] = 180.0-m_Angle[0]-m_Angle[1];

    if(fabs(m_SignedArea)<LENGTHPRECISION*LENGTHPRECISION)
    {
        // the three sides are colinear, flat triangle
        m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }
    // calculate the matrix to transform local coordinates in homogeneous barycentric coordinates
    gmat[0] = 1.0;     gmat[1] = m_S[0].x;     gmat[2] = m_S[0].y;
    gmat[3] = 1.0;     gmat[4] = m_S[1].x;     gmat[5] = m_S[1].y;
    gmat[6] = 1.0;     gmat[7] = m_S[2].x;     gmat[8] = m_S[2].y;
    transpose33(gmat);
    invert33(gmat);
}


/** Initializes member variables to zero */
void Triangle2d::initialize()
{
    m_Neighbour[0] = m_Neighbour[1] = m_Neighbour[2] = -1;
    m_NodeIndex[0] = m_NodeIndex[1] = m_NodeIndex[2] = -1;

    m_bNullTriangle = true;
    m_SignedArea  = 0.0;
    m_NodeIndex[0] = m_NodeIndex[1] = m_NodeIndex[2] = -1;
    memset(gmat, 0, 9*sizeof(double));
}


/**
 * Calculates the barycentric coordinates of a point P.
 * @param Ptl is the point in global coordinates
 * @param g  a pointer to the array of barycentric coordinates
*/
void Triangle2d::barycentricCoords(const Vector2d ptGlobal, double *g) const
{
    g[0] = gmat[0] + gmat[1]*ptGlobal.x+ gmat[2]*ptGlobal.y;
    g[1] = gmat[3] + gmat[4]*ptGlobal.x+ gmat[5]*ptGlobal.y;
    g[2] = gmat[6] + gmat[7]*ptGlobal.x+ gmat[8]*ptGlobal.y;
}

void Triangle2d::barycentricCoords(double xg, double yg, double *g) const
{
    g[0] = gmat[0] + gmat[1]*xg+ gmat[2]*yg;
    g[1] = gmat[3] + gmat[4]*xg+ gmat[5]*yg;
    g[2] = gmat[6] + gmat[7]*xg+ gmat[8]*yg;
}


Vector2d Triangle2d::point(double const *g, double &x, double &y) const
{
    x = m_S[0].x*g[0] + m_S[1].x*g[1] + m_S[2].x*g[2];
    y = m_S[0].y*g[0] + m_S[1].y*g[1] + m_S[2].y*g[2];
    return {x,y};
}


#define PREC 0.0001

/**
 * RECOMMENDED WAY TO TEST POINT POSITION
 */
TRIANGLE::enumPointPosition Triangle2d::pointPosition(double xg, double yg, int &iVertex, int &iEdge) const
{
    iVertex = iEdge = -1;
    double g[3];
    barycentricCoords(xg, yg, g);
    if ((g[0]>PREC) && (g[0]<1.0-PREC) && (g[1]>PREC) && (g[1]<1.0-PREC)  && (g[2]>PREC) && (g[2]<1.0-PREC))
    {
        return TRIANGLE::Inside;
    }
    else if (fabs(g[0]-1.0)<PREC && fabs(g[1])<PREC && fabs(g[2])<PREC)
    {
        iVertex = 0;
        return TRIANGLE::OnVertex;
    }
    else if (fabs(g[0])<PREC && fabs(g[1]-1)<PREC && fabs(g[2])<PREC)
    {
        iVertex = 1;
        return TRIANGLE::OnVertex;
    }
    else if (fabs(g[0])<PREC && fabs(g[1])<PREC && fabs(g[2]-1)<PREC)
    {
        iVertex = 2;
        return TRIANGLE::OnVertex;
    }
    else if(fabs(g[0])<PREC)
    {
        iEdge = 0;
        return TRIANGLE::OnEdge;
    }
    else if(fabs(g[1])<PREC)
    {
        iEdge = 1;
        return TRIANGLE::OnEdge;
    }
    else if(fabs(g[2])<PREC)
    {
        iEdge = 2;
        return TRIANGLE::OnEdge;
    }

    return TRIANGLE::Outside;
}


/** Edges and vertices INCLUDED. */
bool Triangle2d::isInside(Vector2d ptg) const
{
    return isInside(ptg.x, ptg.y);
}


/** Edges and vertices INCLUDED.  Not reliable: beware equality of floats*/
bool Triangle2d::isInside(double xg, double yg) const
{
    double g[] = {0,0,0};
    barycentricCoords(xg, yg, g);
    return (g[0]>=0.0) && (g[0]<=1.0) && (g[1]>=0.0) && (g[1]<=1.0)  && (g[2]>=0.0) && (g[2]<=1.0);
}


/** Edges and vertices EXCLUDED. */
bool Triangle2d::isStrictlyInside(double xg, double yg) const
{
    double g[] = {0,0,0};
    barycentricCoords(xg, yg, g);
    return (g[0]>PREC) && (g[0]<1.0-PREC) && (g[1]>PREC) && (g[1]<1.0-PREC)  && (g[2]>PREC) && (g[2]<1.0-PREC);
}


bool Triangle2d::isOnEdge(Vector2d ptg, int &iEdge) const
{
    return isOnEdge(ptg.x, ptg.y, iEdge);
}


bool Triangle2d::isOnEdge(double xg, double yg, int &iEdge) const
{
    for(int i=0; i<3; i++)
    {
        if(m_Edge[i].isOnSegment(xg, yg))
        {
            iEdge=i;
            return true;
        }
    }
    iEdge = -1;
    return false;
}


/**
 * returns true if the point is in the triangle's circum circle
 */
bool Triangle2d::hasInCircumCircle(Vector2d pt) const
{
    bool isPositiveOrientation = (m_S[1].x - m_S[0].x)*(m_S[2].y - m_S[0].y)-(m_S[2].x - m_S[0].x)*(m_S[1].y - m_S[0].y) > 0;
    double ax = m_S[0].x-pt.x;
    double ay = m_S[0].y-pt.y;
    double bx = m_S[1].x-pt.x;
    double by = m_S[1].y-pt.y;
    double cx = m_S[2].x-pt.x;
    double cy = m_S[2].y-pt.y;
    double det = (ax*ax + ay*ay) * (bx*cy-cx*by) - (bx*bx + by*by) * (ax*cy-cx*ay) + (cx*cx + cy*cy) * (ax*by-bx*ay);
    if(isPositiveOrientation) return det>0.0;
    else                      return det<0.0;
}


bool Triangle2d::segmentIntersects(Segment2d const & seg, Vector2d &IPt) const
{
    double precision = 1.e-5;
    if(seg.intersects( m_S[0], m_S[1], IPt, false, precision)) return true;
    if(seg.intersects( m_S[1], m_S[2], IPt, false, precision)) return true;
    if(seg.intersects( m_S[2], m_S[0], IPt, false, precision)) return true;
    return false;
}


bool Triangle2d::hasCommonEdge(Triangle2d const &t2d) const
{
    int iEdge=-1;
    if(isEdge(t2d.vertex(0), t2d.vertex(1), iEdge)) return true;
    if(isEdge(t2d.vertex(1), t2d.vertex(2), iEdge)) return true;
    if(isEdge(t2d.vertex(2), t2d.vertex(0), iEdge)) return true;
    return  false;
}


bool Triangle2d::isEdge(Segment2d const &seg, int &iEdge) const
{
    return isEdge(seg.vertexAt(0), seg.vertexAt(1), iEdge);
}


bool Triangle2d::isEdge(Vector2d const &vtx0, Vector2d const &vtx1, int &iEdge) const
{
    if(m_S[0].isSame(vtx0))
    {
        if      (m_S[1].isSame(vtx1)) {iEdge=2; return true;}
        else if (m_S[2].isSame(vtx1)) {iEdge=1; return true;}
        return false;
    }
    else if(m_S[0].isSame(vtx1))
    {
        if      (m_S[1].isSame(vtx0)) {iEdge=2; return true;}
        else if (m_S[2].isSame(vtx0)) {iEdge=1; return true;}
        return false;
    }
    else if(m_S[1].isSame(vtx0) && m_S[2].isSame(vtx1)) {iEdge=0; return true;}
    else if(m_S[1].isSame(vtx1) && m_S[2].isSame(vtx0)) {iEdge=0; return true;}

    iEdge=-1;
    return false;
}


/**
 * A triangle is said to be skinny if the circumradius-to-shortest edge ratio is greater than B
 * Note: this only works if the panel is in the xy plane
 */

/**
 * A triangle is said to be skinny if the circumradius-to-shortest edge ratio is greater than B
 */
bool Triangle2d::isSkinny() const
{
    double r=0.0, e=0.0;
    return qualityFactor(r,e) > s_QualityBound;
}


double Triangle2d::qualityFactor(double &r, double &shortestEdge) const
{
    shortestEdge = 1.e100;
    double a = m_Edge[0].length();
    double b = m_Edge[1].length();
    double c = m_Edge[2].length();
    shortestEdge = std::min(shortestEdge, a);
    shortestEdge = std::min(shortestEdge, b);
    shortestEdge = std::min(shortestEdge, c);

    // find circumradius
    r = a*b*c / sqrt((a+b+c)*(b+c-a)*(c+a-b)*(a+b-c));

    return r/shortestEdge;
}


bool Triangle2d::isLong(double size) const
{
    if(S01.norm()>size) return true;
    if(S02.norm()>size) return true;
    if(S12.norm()>size) return true;
    return false;
}

/*
double Triangle2d::edgeLength(int iEdge)
{
    switch(iEdge)
    {
        case 0: return S01.VAbs(); break;
        case 1: return S02.VAbs(); break;
        case 2: return S12.VAbs(); break;
        default: break;
    }
    return -1.0;
}
*/

void Triangle2d::circumCenter(double &r, Vector2d &CC) const
{
    double xc, yc, m1, m2, mx1, mx2, my1, my2, dx, dy;
    double ax = m_S[0].x, ay = m_S[0].y;
    double bx = m_S[1].x, by = m_S[1].y;
    double cx = m_S[2].x, cy = m_S[2].y;

    double dy1 = fabs(ay-by);
    double dy2 = fabs(by-cy);

    /* Check for coincident points */
    if(dy1<PRECISION && dy2<PRECISION) return;

    if(dy1<PRECISION)
    {
        m2  = -((cx-bx)/(cy-by));
        mx2 = (bx+cx)/2.0;
        my2 = (by+cy)/2.0;
        xc  = (bx+ax)/2.0;
        yc  = m2 * (xc-mx2)+my2;
    }
    else if(dy2<PRECISION)
    {
        m1  = -((bx-ax)/(by-ay));
        mx1 = (ax+bx)/2.0;
        my1 = (ay+by)/2.0;
        xc  = (cx+bx)/2.0;
        yc  = m1 * (xc-mx1)+my1;
    }
    else
    {
        m1  = -((bx-ax)/(by-ay));
        m2  = -((cx-bx)/(cy-by));
        mx1 = (ax+bx)/2.0;
        mx2 = (bx+cx)/2.0;
        my1 = (ay+by)/2.0;
        my2 = (by+cy)/2.0;
        xc  = (m1 * mx1-m2 * mx2+my2-my1)/(m1-m2);
        yc  = (dy1 > dy2)? m1 * (xc-mx1)+my1 : m2 * (xc-mx2)+my2;
    }

    dx = bx-xc;
    dy = by-yc;
    r = sqrt(dx*dx+dy*dy);
    CC.set(xc,yc);
}


bool Triangle2d::contains(Vector2d pt, bool bEdgesInside) const
{
    return contains(pt.x, pt.y, bEdgesInside);
}


/**
 * Determines if the point with ccordinates (x,y) is inside the triangle or not.
 * @param bEdgesInside if true, points on the edges are considered to be inside the triangle
 * @return  true if the point is inside the triangle, false otherwise
 */
bool Triangle2d::contains(double x, double y, bool bEdgesInside) const
{
/*	The interior of the triangle is the set of all points inside a triangle,
    i.e., the set of all points in the convex hull of the triangle's vertices.
    The simplest way to determine if a point lies inside a triangle is to check
    the number of points in the convex hull of the vertices of the triangle adjoined
    with the point in question. If the hull has three points, the point lies in the
    triangle's interior; if it is four, it lies outside the triangle.
    To determine if a given point v lies in the interior of a given triangle,
    consider an individual vertex, denoted v_0, and let v_1 and v_2 be the vectors
    from v_0 to the other two vertices.
    Expressing the vector from v_0 to v in terms of v_1 and v_2 then gives v = v_0+ a v_1 + b v_2 */

/*    double vv1 =       x*S01.y -       y*S01.x;
    double vv2 =       x*S02.y -       y*S02.x;
    double v0v1 = S[0].x*S01.y -  S[0].y*S01.x;
    double v0v2 = S[0].x*S02.y -  S[0].y*S02.x;
    double v1v2 = S01.x*S02.y - S01.y*S02.x;
    double a =  (vv2-v0v2)/v1v2;
    double b = -(vv1-v0v1)/v1v2;
    if(bEdgesInside) return a>=-LENGTHPRECISION && b>=-LENGTHPRECISION && a+b<=1.0+LENGTHPRECISION;
    else             return a>= LENGTHPRECISION && b>= LENGTHPRECISION && a+b<=1.0-LENGTHPRECISION;*/

    // check if inside triangle
    double g[] = {0,0,0};
    barycentricCoords(x, y, g);
    if(bEdgesInside)
        return -LENGTHPRECISION<g[0] && g[0]<1.0+LENGTHPRECISION &&
               -LENGTHPRECISION<g[1] && g[1]<1.0+LENGTHPRECISION &&
               -LENGTHPRECISION<g[2] && g[2]<1.0+LENGTHPRECISION;
    else
        return LENGTHPRECISION<g[0] && g[0]<1.0-LENGTHPRECISION &&
               LENGTHPRECISION<g[1] && g[1]<1.0-LENGTHPRECISION &&
               LENGTHPRECISION<g[2] && g[2]<1.0-LENGTHPRECISION;
}


void Triangle2d::displayNodes(QString strong) const
{
    QString str;
    str = QString::asprintf("Vector2d( %17.9g, %17.9g ), Vector2d( %17.9g, %17.9g ), Vector2d( %17.9g, %17.9g )", m_S[0].x, m_S[0].y, m_S[1].x, m_S[1].y, m_S[2].x, m_S[2].y);
    strong = strong + " " +str;
    qDebug("%s", strong.toStdString().c_str());
}


bool Triangle2d::hasVertex(Vector2d vtx) const
{
    if     (m_S[0].isSame(vtx)) return true;
    else if(m_S[1].isSame(vtx)) return true;
    else if(m_S[2].isSame(vtx)) return true;
    return false;
}


Segment2d Triangle2d::edge(int i) const
{
    if     (i==2) return Segment2d(m_S[0], m_S[1]);
    else if(i==0) return Segment2d(m_S[1], m_S[2]);
    else if(i==1) return Segment2d(m_S[2], m_S[0]);
    else
    {
        qDebug("Panel3::edge   Problem");
        Segment2d anEdge;
        return anEdge;
    }
}


Vector2d const &Triangle2d::oppositeVertex(int iedge) const
{
    return m_S[iedge]; // since by construction the index of an edge is the index of its opposite vertex
}


int Triangle2d::neighbourCount() const
{
    int count = 0;
    for(int in=0; in<3; in++)
    {
        if(m_Neighbour[in]>=0) count++;
    }
    return count;
}


void Triangle2d::splitEdge(int iEdge, Triangle2d &T1, Triangle2d &T2) const
{
    Vector2d midpt = edge(iEdge).midPoint();
    if (iEdge==2)
    {
        T1.setTriangle(m_S[0], m_S[2], midpt);
        T2.setTriangle(m_S[1], m_S[2], midpt);
    }
    else if (iEdge==0)
    {
        T1.setTriangle(m_S[1], m_S[0], midpt);
        T2.setTriangle(m_S[2], m_S[0], midpt);
    }
    else if (iEdge==1)
    {
        T1.setTriangle(m_S[0], m_S[1], midpt);
        T2.setTriangle(m_S[2], m_S[1], midpt);
    }
}


void Triangle2d::splitAtEdgeMidPoints(QVector<Triangle2d> &splittriangles) const
{
    Vector2d vtx0 = edge(0).midPoint();
    Vector2d vtx1 = edge(1).midPoint();
    Vector2d vtx2 = edge(2).midPoint();
    splittriangles.clear();
    splittriangles.append({vtx0, m_S[2], vtx1});
    splittriangles.append({vtx1, m_S[0], vtx2});
    splittriangles.append({vtx2, m_S[1], vtx0});
    splittriangles.append({vtx0, vtx1, vtx2});
}


void Triangle2d::splitAtCoG(QVector<Triangle2d> &splittriangles) const
{
    splittriangles.clear();
    splittriangles.append({m_S[0], CoG_G, m_S[1]});
    splittriangles.append({m_S[1], CoG_G, m_S[2]});
    splittriangles.append({m_S[2], CoG_G, m_S[0]});
}


double Triangle2d::minEdgeLength() const
{
    double l=1.e10;
    for (int ie=0; ie<3; ie++)
    {
        l = std::min(l, edge(ie).length());
    }
    return l;
}


double Triangle2d::maxEdgeLength() const
{
    double l=0;
    for (int ie=0; ie<3; ie++)
    {
        l = std::max(l, edge(ie).length());
    }
    return l;
}


void Triangle2d::longestEdge(int &iEdge, double &length) const
{
    iEdge  = -1;
    length =  0;
    for (int ie=0; ie<3; ie++)
    {
        double l = edge(ie).length();
        if(l>length)
        {
            iEdge = ie;
            length = l;
        }
    }
}
