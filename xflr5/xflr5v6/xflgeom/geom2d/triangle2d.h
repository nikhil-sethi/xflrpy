

#pragma once

#include <QString>


#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom2d/segment2d.h>
#include <xflgeom/geom2d/cartesianframe2d.h>

namespace TRIANGLE {
    typedef enum {OnVertex, OnEdge, Inside, Outside}	enumPointPosition;
}


class Triangle2d
{
    public:
        Triangle2d();
//        Triangle2d(Triangle2d const &triangle);
        Triangle2d(const Vector2d &vtx0, const Vector2d &vtx1, const Vector2d &vtx2);

        void setTriangle();
        void setTriangle(Vector2d const &S0, Vector2d const &S1, Vector2d const &S2);
        void initialize();

        CartesianFrame2d const & frame() const {return m_CF;}

        Segment2d edge(int i) const;
        bool hasInCircumCircle(Vector2d pt) const;
        bool segmentIntersects(Segment2d const &seg, Vector2d &IPt) const;
        bool isInside(Vector2d ptg) const;
        bool isInside(double xg, double yg) const;
        bool isStrictlyInside(double xg, double yg) const;
        bool isOnEdge(Vector2d ptg, int &iEdge) const;
        bool isOnEdge(double xg, double yg, int &iEdge) const;

        bool isEdge(Vector2d const &vtx0, Vector2d const &vtx1, int &iEdge) const;
        bool isEdge(Segment2d const &seg, int &iEdge) const;

        bool hasCommonEdge(Triangle2d const &t2d) const;


        TRIANGLE::enumPointPosition pointPosition(double xg, double yg, int &iVertex, int &iEdge) const;

        double minEdgeLength() const;
        double maxEdgeLength() const;
        void longestEdge(int &iEdge, double &length) const;


        double qualityFactor(double &r, double &shortestEdge) const;
        bool isSkinny() const;
        bool isLong(double size) const;
        void circumCenter(double &r, Vector2d &CC) const;
        bool contains(double x, double y, bool bEdgesInside) const;
        bool contains(Vector2d pt, bool bEdgesInside) const;

        void clearConnections() {m_Neighbour[0] = m_Neighbour[1] = m_Neighbour[2] = -1;}
        void setNeighbour(int ic, int it) {m_Neighbour[ic]=it;}
        int neighbour(int idx) const {return m_Neighbour[idx];}
        int neighbourCount() const;

        //	double edgeLength(int iEdge);
        bool hasVertex(Vector2d vtx) const;
        Vector2d const &oppositeVertex(int iedge) const;

        Vector2d vertex(int ivtx) const {return m_S[ivtx%3];}  // so that the fourth vertex is also the first
        void setVertex(int ivtx, double x, double y) {m_S[ivtx%3].x=x; m_S[ivtx%3].y=y;}

        bool isNullTriangle() const {return m_bNullTriangle;}

        double area() const {return fabs(m_SignedArea);}

        Vector2d CoG() const {return CoG_G;}

        void splitEdge(int iEdge, Triangle2d &T1, Triangle2d &T2) const;
        void splitAtEdgeMidPoints(QVector<Triangle2d> &splittriangles) const;
        void splitAtCoG(QVector<Triangle2d> &splittriangles) const;


        void barycentricCoords(const Vector2d ptGlobal, double *g) const;
        void barycentricCoords(double xg, double yg, double *g) const;
        Vector2d point(double const *g, double &x, double &y) const;

        double angle(int iVtx) const {return m_Angle[iVtx];}

        void displayNodes(QString strong) const;

        static double qualityBound() {return s_QualityBound;}
        static void setQualityBound(double bound) {s_QualityBound=bound;}

    private:
        Vector2d m_S[3];              /**< the three triangle vertices, in global coordinates; the index of a vertex is the index of the opposite edge*/
        Vector2d S01, S02, S12;
        Segment2d m_Edge[3];         /**< the three sides, in global coordinates; the index of an edge is the index of the opposite vertex*/
        int m_Neighbour[3];          /**< the indexes of the three neighbour triangle sharing one of the edge, or -1 if none */
        double m_SignedArea;          /**< The panel's signed area; */
        bool m_bNullTriangle;
        double m_Angle[3];          /**< the internal angles */
        int m_NodeIndex[3];         /**< the index of the three vertices in the global node array */
        Vector2d CoG_G;
        double gmat[9];             /**< the transformation matrix from local coordinates to barycentric coordinates;*/

        CartesianFrame2d m_CF;

        static double s_QualityBound;

};

