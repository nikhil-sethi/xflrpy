#include "geom_globals.h"

#include <xflcore/constants.h>
#include <xflgeom/geom3d/node.h>
#include <xflgeom/geom3d/triangle3d.h>


/**
 * Returns the distance of point Pt to line AB
 */
double distanceToLine3d(Vector3d const &A, Vector3d const &B, Vector3d const &P)
{
    double dotproduct = (B.x-A.x)*(P.x-A.x) + (B.y-A.y)*(P.y-A.y) + (B.z-A.z)*(P.z-A.z);

    double AB = sqrt((B.x-A.x)*(B.x-A.x)+(B.y-A.y)*(B.y-A.y)+(B.z-A.z)*(B.z-A.z));
    double AP = sqrt((P.x-A.x)*(P.x-A.x)+(P.y-A.y)*(P.y-A.y)+(P.z-A.z)*(P.z-A.z));

    if(AP<LENGTHPRECISION)
    {
        return 0.0;
    }
    if(AB<LENGTHPRECISION)
    {
        return 0.0;
    }
    double cosa = dotproduct/AB/AP;

    double AH = AP * cosa;
    return sqrt(AP*AP-AH*AH);
}


void makeSphere(double radius, int nSplit, QVector<Triangle3d> &triangles)
{
    // make vertices
    QVector<Node> vtx(12);
    vtx[10].set(0,0, radius);   // North pole
    vtx[11].set(0,0,-radius);   // South pole
    double x=0,y=0,z=0;
    double atn= atan(0.5);
    double di=0.0;
    for(int i=0; i<5; i++)
    {
        di = double(i);
        x = radius * cos(atn)*cos(72.0*di*PI/180.0);
        y = radius * cos(atn)*sin(72.0*di*PI/180.0);
        z = radius * sin(atn);
        vtx[i].set(x,y,z);
        x =  radius * cos(atn)*cos((36+72.0*di)*PI/180.0);
        y =  radius * cos(atn)*sin((36+72.0*di)*PI/180.0);
        z = -radius * sin(atn);
        vtx[i+5].set(x,y,z);
    }

    for(int i=0; i<vtx.size(); i++) vtx[i].setNormal(vtx[i]);

    triangles.resize(20);


    //make the top five triangles from the North pole to the northern hemisphere latitude

    for(int i=0; i<5; i++)
    {
        int i1 = i;
        int i2 = (i+1)%5;
        triangles[i].setTriangle(vtx[10], vtx[i1], vtx[i2]);
    }
    //make the bottom five triangles from the South pole to the northern hemisphere latitude
    for(int i=0; i<5; i++)
    {
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;
        triangles[5+i].setTriangle(vtx[11], vtx[i2], vtx[i1]);
    }

    // make the equatorial belt
    for(int i=0; i<5; i++)
    {
        int i1 = i;
        int i2 = (i+1)%5;
        triangles[10+i].setTriangle(vtx[i1], vtx[i1+5], vtx[i2]);
    }
    for(int i=0; i<5; i++)
    {
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;
        triangles[15+i].setTriangle(vtx[i1], vtx[i2], vtx[(i1+1)%5]);
    }

    QVector<Triangle3d> triangle, split(4);

    Node M0, M1, M2; //edge mid points
    for(int iter=0; iter<nSplit; iter++)
    {
        triangle.clear();
        for(int it=0; it<triangles.size(); it++)
        {
            Triangle3d const& t3d = triangles.at(it);

            M0.set(t3d.edge(0).midPoint());
            M1.set(t3d.edge(1).midPoint());
            M2.set(t3d.edge(2).midPoint());

            // project radially the mid points on the unit sphere
            M0.normalize();
            M1.normalize();
            M2.normalize();

            M0.setNormal(M0);
            M1.setNormal(M1);
            M2.setNormal(M2);

            M0 *= radius;
            M1 *= radius;
            M2 *= radius;

            // build the subset of 4 triangles
            split[0].setTriangle(M1, t3d.vertexAt(0), M2);
            split[1].setTriangle(M2, t3d.vertexAt(1), M0);
            split[2].setTriangle(M0, t3d.vertexAt(2), M1);
            split[3].setTriangle(M2, M0, M1);

            triangle.append(split);
        }

        triangles = triangle;
    }
}


/**
 * Returns true if [A0,A1] intersects [B0,B1], false otherwise
 * Parallel overlapping segments are considered to intersect.
 * The default precision is set to 1/100 mmm because the max error returned by OCC on test cases is ~1.e-6
 */
bool intersectSegment(Vector2d const &A0, Vector2d const &A1,
                      Vector2d const &B0, Vector2d const &B1,
                      Vector2d &IPt, bool bPointsInside, double vtx_precision)
{
    double rx = A1.x-A0.x;
    double ry = A1.y-A0.y;

    double sx = B1.x-B0.x;
    double sy = B1.y-B0.y;

    double rXs = rx*sy - ry*sx; // two dimensional cross product

    double sinTheta = rXs / sqrt(rx*rx+ry*ry) / sqrt(sx*sx+sy*sy);

    if(A0.isSame(B0, vtx_precision)) return bPointsInside;
    if(A0.isSame(B1, vtx_precision)) return bPointsInside;
    if(A1.isSame(B0, vtx_precision)) return bPointsInside;
    if(A1.isSame(B1, vtx_precision)) return bPointsInside;

    if(fabs(sinTheta)<1.e-4)
    {
        // the two lines are parallel
        double qpXr = (B0.x-A0.x)*ry - (B0.y-A0.y)*rx;
        if(fabs(qpXr)<1.e-6)
        {
            //the two lines are co-linear
            double r2 = rx*rx + ry*ry;
            double t0 = ((B0.x-A0.x)*rx + (B0.y-A0.y)*ry )/r2;
            double t1 = t0 + (sx*rx+sy*ry)/r2;
            if(t0>vtx_precision && t0<1.0-vtx_precision)
            {
                return false; // overlapping
            }
            if(t1>vtx_precision && t1<1.0-vtx_precision)
            {
                return true; // overlapping
            }
            // Colinear not overlapping;
            return false;
        }
        else
        {
            //parallel;
            return false;
        }
    }
    else
    {
        double qpXr = (B0.x-A0.x)*ry - (B0.y-A0.y)*rx;
        double qpXs = (B0.x-A0.x)*sy - (B0.y-A0.y)*sx;
        double t = qpXs / rXs;
        double u = qpXr / rXs;

        IPt.x = A0.x + t*rx;
        IPt.y = A0.y + t*ry;
        // note: using t>precision etc. excludes coincident endpoints
        //       and also endpoints on segment
        // --> handle limit cases in calling functions

        if (vtx_precision<t && t<1.0-vtx_precision && vtx_precision<u && u<1.0-vtx_precision)
        {
            return true; // clearly inside on both segments
        }
        if(fabs(t)<vtx_precision || fabs(t-1.0)<vtx_precision)
        {
            // the segments meet at an end point
            // check on the other segment
            if(vtx_precision<u && u<1.0-vtx_precision)
            {
                return bPointsInside;
            }
            return false;
        }
        if(fabs(u)<vtx_precision || fabs(u-1.0)<vtx_precision)
        {
            // the segments meet at an end point
            // check on the other segment
            if(vtx_precision<t && t<1.0-vtx_precision)
            {
                return bPointsInside;
            }
            return false;
        }
        return false;
    }
}
