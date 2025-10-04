/****************************************************************************

    Surface Class
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


#include "surface.h"
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects3d/body.h>
#include <xflgeom/geom3d/quaternion.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/wingsection.h>

Vector3d *Surface::s_pNode;
Panel *Surface::s_pPanel;


/**
 * The public constructor
 */
Surface::Surface()
{
    m_bTEFlap = false;

    m_Length   = 0.0;
    m_TwistA   = 0.0;
    m_TwistB   = 0.0;
    m_posATE   = 1.0;
    m_posBTE   = 1.0;
    m_NXPanels  = 1;
    m_NYPanels  = 2;
    m_NXLead    = 1;
    m_NXFlap    = 0;
    m_XDistType = xfl::COSINE;
    m_YDistType = xfl::UNIFORM;

    m_pLeftSurface = m_pRightSurface = nullptr;
    m_pFoilA   = nullptr;
    m_pFoilB   = nullptr;
    m_NElements = 0;

    m_bIsInSymPlane = false;
    m_bIsTipLeft    = false;
    m_bIsTipRight   = false;

    m_bIsLeftSurf   = false;
    m_bIsRightSurf  = false;

    m_bIsCenterSurf = false;
    m_bJoinRight    = true;

    m_nFlapNodes  = 0;
    m_nFlapPanels = 0;

    memset(m_FlapNode, 0, sizeof(m_FlapNode));
    memset(m_FlapPanel, 0, sizeof(m_FlapPanel));

    m_xPointA.clear();
    m_xPointA.insert(0, 1000, 0);

    m_xPointB.clear();
    m_xPointB.insert(0, 1000, 0);

    LA.set(0.0,0.0,0.0);
    TA.set(0.0,0.0,0.0);
    LB.set(0.0,0.0,0.0);
    TB.set(0.0,0.0,0.0);

    m_innerSection = m_outerSection = 0;
}


/**
 * Adds the reference of thE input panel to the array of flap panel indexes.
 * @param pPanel the pointer of the panel to add to the flap panel list.
 */
void Surface::addFlapPanel(Panel const &panel)
{
    bool bFound = false;

    //Add Nodes

    for (int i=0; i<m_nFlapNodes; i++)
    {
        bFound = bFound && panel.m_iLA==m_FlapNode[i];
        if(panel.m_iLA== m_FlapNode[i])
        {
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        m_FlapNode[m_nFlapNodes] = panel.m_iLA;
        m_nFlapNodes++;
    }

    bFound = false;
    for (int i=0; i< m_nFlapNodes; i++)
    {
        if(panel.m_iLB== m_FlapNode[i])
        {
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        m_FlapNode[m_nFlapNodes] = panel.m_iLB;
        m_nFlapNodes++;
    }

    for (int i=0; i< m_nFlapNodes; i++)
    {
        if(panel.m_iTA== m_FlapNode[i])
        {
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        m_FlapNode[m_nFlapNodes] = panel.m_iTA;
        m_nFlapNodes++;
    }

    bFound = false;
    for (int i=0; i< m_nFlapNodes; i++)
    {
        if(panel.m_iTB== m_FlapNode[i])
        {
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        m_FlapNode[m_nFlapNodes] = panel.m_iTB;
        m_nFlapNodes++;
    }

    //Add panel;
    bFound=false;
    for(int i=0; i<m_nFlapPanels; i++)
    {
        if(panel.m_iElement==m_FlapPanel[i])
        {
            bFound =true;
            break;
        }
    }
    if(!bFound)
    {
        m_FlapPanel[m_nFlapPanels] = panel.m_iElement;
        m_nFlapPanels++;
    }
}

/**
 * Copy the data from another Surface object to this Surface
 * @param Surface the source Surface from which the data shall be duplicated
 */
void Surface::copy(Surface const*pSurface)
{
    m_LA.copy(pSurface->m_LA);
    m_LB.copy(pSurface->m_LB);
    m_TA.copy(pSurface->m_TA);
    m_TB.copy(pSurface->m_TB);
    m_XDistType = pSurface->m_XDistType;
    m_YDistType = pSurface->m_YDistType;
    m_NElements = pSurface->m_NElements;

    m_Length    = pSurface->m_Length;
    m_NXPanels  = pSurface->m_NXPanels;
    m_NYPanels  = pSurface->m_NYPanels;
    m_pFoilA    = pSurface->m_pFoilA;
    m_pFoilB    = pSurface->m_pFoilB;
    m_TwistA    = pSurface->m_TwistA;
    m_TwistB    = pSurface->m_TwistB;

    m_Normal  = pSurface->m_Normal;
    NormalA = pSurface->NormalA;
    NormalB = pSurface->NormalB;

    m_bIsTipLeft    = pSurface->m_bIsTipLeft;
    m_bIsTipRight   = pSurface->m_bIsTipRight;
    m_bIsLeftSurf   = pSurface->m_bIsLeftSurf;
    m_bIsRightSurf  = pSurface->m_bIsRightSurf;
    m_bIsCenterSurf = pSurface->m_bIsCenterSurf;
    m_bJoinRight    = pSurface->m_bJoinRight;
    m_bIsInSymPlane = pSurface->m_bIsInSymPlane;

    m_bTEFlap       = pSurface->m_bTEFlap;
    m_nFlapNodes  = pSurface->m_nFlapNodes;
    m_nFlapPanels = pSurface->m_nFlapPanels;
    m_HingePoint  = pSurface->m_HingePoint;
    m_HingeVector = pSurface->m_HingeVector;

    memcpy(m_FlapNode, pSurface->m_FlapNode, sizeof(m_FlapNode));
    memcpy(m_FlapPanel, pSurface->m_FlapPanel, sizeof(m_FlapPanel));

    m_xPointA.clear();
    m_xPointB.clear();

    for(int ix=0; ix<pSurface->m_xPointA.size(); ix++)
        m_xPointA.append(pSurface->m_xPointA.at(ix));
    for(int ix=0; ix<pSurface->m_xPointB.size(); ix++)
        m_xPointB.append(pSurface->m_xPointB.at(ix));
}


/**
 * Returns the quarter-chord point of a specified strip
 * @param k the 0-based index of the strip for which the quarter-chord point shall be returned.
 * @param Pt the quarter-chord point
 * @param tau the relative span position of the Pt
 */
void Surface::getC4(int k, Vector3d &Pt, double &tau) const
{
    getPanel(k,m_NXPanels-1, xfl::MIDSURFACE);
    double xl = (LA.x+LB.x)/2.0;
    double yl = (LA.y+LB.y)/2.0;
    double zl = (LA.z+LB.z)/2.0;
    getPanel(k,0,xfl::MIDSURFACE);
    double xt = (TA.x+TB.x)/2.0;
    double yt = (TA.y+TB.y)/2.0;
    double zt = (TA.z+TB.z)/2.0;
    Pt.x = xl*.75 + xt*.25;
    Pt.y = yl*.75 + yt*.25;
    Pt.z = zl*.75 + zt*.25;

    tau = sqrt((Pt.y-m_LA.y)*(Pt.y-m_LA.y) + (Pt.z-m_LA.z)*(Pt.z-m_LA.z)) / m_Length;
}


/**
 * Returns the chord length of the specified strip
 * @param k the 0-based index of the strip for which the chord shall be returned.
 * @return the chord length
 */
double Surface::chord(int k) const
{
    double y1, y2;
    getYDist(k, y1, y2);
    return chord((y1+y2)/2.0);
}


/**
 * Returns the chord length at the specified relative span position.
 * @param tau the relative percentage of the Surface's span length
 * @return the chord length
 */
double Surface::chord(double tau) const
{
    //assumes LA-TB have already been loaded
    Vector3d V1, V2;
    double ChordA, ChordB;

    V1 = m_TA-m_LA;
    V2 = m_TB-m_LB;

    ChordA = V1.norm();
    ChordB = V2.norm();

    return ChordA + (ChordB-ChordA) * qAbs(tau);
}


/**
 * Returns the strip's offset in the x direction at the specified relative span position.
 * @param tau the relative percentage of the Surface's span length
 * @return the offset in the x-direction
 */
double Surface::offset(double tau) const
{
    //chord spacing
    return m_LA.x + (m_LB.x-m_LA.x) * qAbs(tau);
}


/**
 * Returns the area of the virtual foil at a specified relative span position. Used in Inertia calaculations.
 * @param tau the relative percentage of the Surface's span length
 * @return the cross area at the specified location
 */
double Surface::foilArea(double tau) const
{
    if(m_pFoilA && m_pFoilB)
    {
        return (m_pFoilA->area() + m_pFoilB->area())/2.0*chord(tau)*chord(tau);//m2
    }
    else
        return 0.0;
}


/**
 * Returns the normal vector at a specified relative span position.
 * @param tau the relative percentage of the Surface's span length
 * @return N the average normal at the specified location
 */
void Surface::getNormal(double yrel, Vector3d &N) const
{
    N = NormalA * (1.0-yrel) + NormalB * yrel;
    N.normalize();
}


/**
 * Returns the leading point of the specified strip
 * @param k the 0-based index of the strip for which the leading point shall be returned.
 * @param C the strip's leading point.
 */
void Surface::getLeadingPt(int k, Vector3d &C) const
{
    getPanel(k,m_NXPanels-1, xfl::MIDSURFACE);

    C.x    = (LA.x+LB.x)/2.0;
    C.y    = (LA.y+LB.y)/2.0;
    C.z    = (LA.z+LB.z)/2.0;
}


/**
 * Returns the trailing point of the specified strip
 * @param k the 0-based index of the strip for which the trailing point shall be returned.
 * @param C the strip's leading point.
 */
void Surface::getTrailingPt(int k, Vector3d &C) const
{
    getPanel(k,0,xfl::MIDSURFACE);

    C.x    = (TA.x+TB.x)/2.0;
    C.y    = (TA.y+TB.y)/2.0;
    C.z    = (TA.z+TB.z)/2.0;
}


/**
 * Calculates the corner points of the panel with index k in the span direction and index l in the chordwise direction.
 * The point coordinates are loaded in the memeber variables LA, LB, TA, TB.
 *
 * Assumes the side points have been set previously
 *
 * @param k the index of the strip 0<=k<m_NYPanels
 * @param l the index of the panel in the chordwise direction. 0<=l<m_NXPanels
 * @param pos defines on which surface (BOTSURFACE, TOPSURFACE, MIDSURFACE) the node positions should be calculated.
 */
void Surface::getPanel(int const &k, int const &l, xfl::enumSurfacePosition pos) const
{
    double y1=0, y2=0;
    getYDist(k,y1,y2);
    if(pos==xfl::MIDSURFACE)
    {
        LA.x = m_SideA[l+1].x * (1.0-y1) + m_SideB[l+1].x* y1;
        LA.y = m_SideA[l+1].y * (1.0-y1) + m_SideB[l+1].y* y1;
        LA.z = m_SideA[l+1].z * (1.0-y1) + m_SideB[l+1].z* y1;
        TA.x = m_SideA[l].x   * (1.0-y1) + m_SideB[l].x  * y1;
        TA.y = m_SideA[l].y   * (1.0-y1) + m_SideB[l].y  * y1;
        TA.z = m_SideA[l].z   * (1.0-y1) + m_SideB[l].z  * y1;
        LB.x = m_SideA[l+1].x * (1.0-y2) + m_SideB[l+1].x* y2;
        LB.y = m_SideA[l+1].y * (1.0-y2) + m_SideB[l+1].y* y2;
        LB.z = m_SideA[l+1].z * (1.0-y2) + m_SideB[l+1].z* y2;
        TB.x = m_SideA[l].x   * (1.0-y2) + m_SideB[l].x  * y2;
        TB.y = m_SideA[l].y   * (1.0-y2) + m_SideB[l].y  * y2;
        TB.z = m_SideA[l].z   * (1.0-y2) + m_SideB[l].z  * y2;
    }
    else if (pos==xfl::BOTSURFACE)
    {
        LA = m_SideA_B[l+1] * (1.0-y1) + m_SideB_B[l+1]* y1;
        TA = m_SideA_B[l]   * (1.0-y1) + m_SideB_B[l]  * y1;
        LB = m_SideA_B[l+1] * (1.0-y2) + m_SideB_B[l+1]* y2;
        TB = m_SideA_B[l]   * (1.0-y2) + m_SideB_B[l]  * y2;

        LA.x = m_SideA_B[l+1].x * (1.0-y1) + m_SideB_B[l+1].x* y1;
        LA.y = m_SideA_B[l+1].y * (1.0-y1) + m_SideB_B[l+1].y* y1;
        LA.z = m_SideA_B[l+1].z * (1.0-y1) + m_SideB_B[l+1].z* y1;
        TA.x = m_SideA_B[l].x   * (1.0-y1) + m_SideB_B[l].x  * y1;
        TA.y = m_SideA_B[l].y   * (1.0-y1) + m_SideB_B[l].y  * y1;
        TA.z = m_SideA_B[l].z   * (1.0-y1) + m_SideB_B[l].z  * y1;
        LB.x = m_SideA_B[l+1].x * (1.0-y2) + m_SideB_B[l+1].x* y2;
        LB.y = m_SideA_B[l+1].y * (1.0-y2) + m_SideB_B[l+1].y* y2;
        LB.z = m_SideA_B[l+1].z * (1.0-y2) + m_SideB_B[l+1].z* y2;
        TB.x = m_SideA_B[l].x   * (1.0-y2) + m_SideB_B[l].x  * y2;
        TB.y = m_SideA_B[l].y   * (1.0-y2) + m_SideB_B[l].y  * y2;
        TB.z = m_SideA_B[l].z   * (1.0-y2) + m_SideB_B[l].z  * y2;
    }
    else if (pos==xfl::TOPSURFACE)
    {
        LA.x = m_SideA_T[l+1].x * (1.0-y1) + m_SideB_T[l+1].x* y1;
        LA.y = m_SideA_T[l+1].y * (1.0-y1) + m_SideB_T[l+1].y* y1;
        LA.z = m_SideA_T[l+1].z * (1.0-y1) + m_SideB_T[l+1].z* y1;
        TA.x = m_SideA_T[l].x   * (1.0-y1) + m_SideB_T[l].x  * y1;
        TA.y = m_SideA_T[l].y   * (1.0-y1) + m_SideB_T[l].y  * y1;
        TA.z = m_SideA_T[l].z   * (1.0-y1) + m_SideB_T[l].z  * y1;
        LB.x = m_SideA_T[l+1].x * (1.0-y2) + m_SideB_T[l+1].x* y2;
        LB.y = m_SideA_T[l+1].y * (1.0-y2) + m_SideB_T[l+1].y* y2;
        LB.z = m_SideA_T[l+1].z * (1.0-y2) + m_SideB_T[l+1].z* y2;
        TB.x = m_SideA_T[l].x   * (1.0-y2) + m_SideB_T[l].x  * y2;
        TB.y = m_SideA_T[l].y   * (1.0-y2) + m_SideB_T[l].y  * y2;
        TB.z = m_SideA_T[l].z   * (1.0-y2) + m_SideB_T[l].z  * y2;
    }
}


/**
 * Returns the strip width at a specified index
 * @param k the index of the strip 0<=k<m_NYPanels
 * @return the strip width
 */
double Surface::stripWidth(int k) const
{
    getPanel(k, 0, xfl::MIDSURFACE);
    return fabs(LA.y-LB.y);
}


/**
 * Returns the position of a side point at the position specified by the input parameters.
 * @param xRel the relative position along the chord
 * @param bRight the left or right side of the surface on which the point is calculated
 * @param pos the top, middle, or bottom surface on which the point is calculated
 * @param Point a reference to the requested point's position
 * @param PtNormal a reference to the vector normal to the surface at that point
 */
void Surface::getSidePoint(double xRel, bool bRight, xfl::enumSurfacePosition pos, Vector3d &Point, Vector3d &PtNormal) const
{
    Vector2d foilPt(xRel,0.0);

    if(!bRight)
    {
        if     (pos==xfl::MIDSURFACE && m_pFoilA) foilPt = m_pFoilA->midYRel(xRel);
        else if(pos==xfl::TOPSURFACE && m_pFoilA) foilPt = m_pFoilA->upperYRel(xRel, PtNormal.x, PtNormal.z);
        else if(pos==xfl::BOTSURFACE && m_pFoilA) foilPt = m_pFoilA->lowerYRel(xRel, PtNormal.x, PtNormal.z);

        Point = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        Point +=  m_Normal * foilPt.y*chord(0.0);
    }
    else
    {
        if     (pos==xfl::MIDSURFACE && m_pFoilB) foilPt = m_pFoilB->midYRel(xRel);
        else if(pos==xfl::TOPSURFACE && m_pFoilB) foilPt = m_pFoilB->upperYRel(xRel, PtNormal.x, PtNormal.z);
        else if(pos==xfl::BOTSURFACE && m_pFoilB) foilPt = m_pFoilB->lowerYRel(xRel, PtNormal.x, PtNormal.z);

        Point = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        Point +=  m_Normal * foilPt.y*chord(1.0);
    }
}


/**
 * Creates the master points on the left and right ends. Used to create the 3d OpengGL view lists.
 * @param pBody a pointer to the Body object, or NULL if none.
 * @param dx the x-component of the translation to apply to the body.
 * @param dz the z-component of the translation to apply to the body.
 * @param PtA a pointer to the array to fill with the points on the Surface's left tip
 * @param PtB a pointer to the array to fill with the points on the Surface's right tip
 * @param N a pointer to the array to fill with the vectors normal to the surface
 * @param nPoints the number of side points to define on each tip
 */
void Surface::getSidePoints(xfl::enumSurfacePosition pos,
                            Body const*pBody,
                            QVector<Vector3d> &PtA, QVector<Vector3d> &PtB, QVector<Vector3d> &NA, QVector<Vector3d> &NB, int nPoints) const
{
    double xRelA(0), xRelB(0);
    Vector3d A4, B4, TA4, TB4, I;
    Vector3d Ux(1,0,0);
    Vector3d V = m_Normal * NormalA;
    Vector3d Ua = (m_LA - m_TA).normalized();
    //    double sindA = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindA = V.dot(Ua);
    if(sindA> 1.0) sindA = 1.0;
    if(sindA<-1.0) sindA = -1.0;
    double alpha_dA = asin(sindA);
    double cosdA = cos(alpha_dA);
    alpha_dA *= 180.0/PI;

    V = m_Normal * NormalB;
    Vector3d Ub = (m_LB-m_TB).normalized();
    //    double sindB = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindB = V.dot(Ub);
    if(sindB> 1.0) sindB = 1.0;
    if(sindB<-1.0) sindB = -1.0;
    double alpha_dB = asin(sindB);
    double cosdB = cos(alpha_dB);
    alpha_dB *= 180.0/PI;


    double delta = -atan(m_Normal.y / m_Normal.z)*180.0/PI;
    //    double delta = -atan2(Normal.y,  Normal.z)*180.0/PI;

    //create the quarter chord centers of rotation for the twist
    A4 = m_LA *3.0/4.0 + m_TA * 1/4.0;
    B4 = m_LB *3.0/4.0 + m_TB * 1/4.0;

    // create the vectors perpendicular to the side Normals and to the x-axis
    TA4.x = 0.0;
    TA4.y = +NormalA.z;
    TA4.z = -NormalA.y;

    TB4.x = 0.0;
    TB4.y = +NormalB.z;
    TB4.z = -NormalB.y;

    for(int i=0; i<nPoints; i++)
    {
        if(m_NXFlap>0 && m_pFoilA && m_pFoilB)
        {
            int nPtsTr = nPoints/3;
            int nPtsLe = nPoints-nPtsTr;

            if(i<nPtsTr)
            {
                xRelA = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPtsTr-1)))* (m_pFoilA->m_TEXHinge/100.);
                xRelB = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPtsTr-1)))* (m_pFoilB->m_TEXHinge/100.);
            }
            else
            {
                int j = i-nPtsTr;
                xRelA = m_pFoilA->m_TEXHinge/100. + 1.0/2.0*(1.0-cos(PI* double(j)/double(nPtsLe-1))) * (1.-m_pFoilA->m_TEXHinge/100.);
                xRelB = m_pFoilB->m_TEXHinge/100. + 1.0/2.0*(1.0-cos(PI* double(j)/double(nPtsLe-1))) * (1.-m_pFoilB->m_TEXHinge/100.);
            }
        }
        else
        {
            xRelA  = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPoints-1)));
            xRelB  = xRelA;
        }

        NA[i].set(0.0,0.0,0.0);
        getSidePoint(xRelA, false, pos, PtA[i], NA[i]);
        //scale the thickness
        double Ox = xRelA;
        double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
        double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
        PtA[i].y = Oy +(PtA[i].y - Oy)/cosdA;
        PtA[i].z = Oz +(PtA[i].z - Oz)/cosdA;
        PtA[i].rotate(m_LA, Ua, +alpha_dA);
        NA[i].rotate(Ux, delta);

        NB[i].set(0.0,0.0,0.0);
        getSidePoint(xRelB, true,  pos, PtB[i], NB[i]);
        Ox = xRelB;
        Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
        Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
        PtB[i].y   = Oy +(PtB[i].y - Oy)/cosdB;
        PtB[i].z   = Oz +(PtB[i].z - Oz)/cosdB;
        PtB[i].rotate(m_LB, Ub, +alpha_dB);
        NB[i].rotate(Ux, delta);

        if(pBody && m_bIsCenterSurf && m_bIsLeftSurf)
        {
            if(pBody->intersect(PtA[i], PtB[i], I, false))
            {
                PtB[i] = I;
            }
        }
        else if(pBody && m_bIsCenterSurf && m_bIsRightSurf)
        {
            if(pBody->intersect(PtA[i], PtB[i], I, true))
            {
                PtA[i] = I;
            }
        }
    }
}


/**
 * Returns the position of a surface point at the position specified by the input parameters.
 * @param xArel the relative position at the left Foil
 * @param xBrel the relative position at the right Foil
 * @param yrel the relative span position
 * @param Point a reference of the requested point's position
 * @param pos defines on which surface (BOTSURFACE, TOPSURFACE, MIDSURFACE) the point is calculated
 */
void Surface::getSurfacePoint(double xArel, double xBrel, double yrel,
                              xfl::enumSurfacePosition pos, Vector3d &Point, Vector3d &PtNormal) const
{
    Vector2d foilPt;
    Vector3d APt, BPt;
    double nx(0), ny(0);
    if(pos==xfl::MIDSURFACE && m_pFoilA && m_pFoilB)
    {
        foilPt = m_pFoilA->midYRel(xArel);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  m_Normal * foilPt.y*chord(0.0);

        foilPt = m_pFoilB->midYRel(xBrel);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  m_Normal * foilPt.y*chord(1.0);

    }
    else if(pos==xfl::TOPSURFACE && m_pFoilA && m_pFoilB)
    {
        foilPt = m_pFoilA->upperYRel(xArel, nx, ny);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  m_Normal * foilPt.y*chord(0.0);

        foilPt = m_pFoilB->upperYRel(xBrel, nx, ny);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  m_Normal * foilPt.y*chord(1.0);

    }
    else if(pos==xfl::BOTSURFACE && m_pFoilA && m_pFoilB)
    {
        foilPt = m_pFoilA->lowerYRel(xArel, nx, ny);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  m_Normal * foilPt.y*chord(0.0);

        foilPt = m_pFoilB->lowerYRel(xBrel, nx, ny);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  m_Normal * foilPt.y*chord(1.0);
    }
    Point = APt * (1.0-yrel)+  BPt * yrel;
    getNormal(yrel, PtNormal);
}


/**
 * Returns the chord length, cross-section area, and quarter-chord point of a given strip,
 * @param tau the relative percentage of the Surface's span length
 * @param Chord a reference to the chord length
 * @param Area a reference to the cross-section area
 * @param PtC4 a reference to the quarter-chord point
 */
void Surface::getSection(double const &tau, double &Chord, double &Area, Vector3d &PtC4) const
{
    //explicit double calculations are much faster than vector algebra
    LA.x = m_LA.x * (1.0-tau) + m_LB.x * tau;
    LA.y = m_LA.y * (1.0-tau) + m_LB.y * tau;
    LA.z = m_LA.z * (1.0-tau) + m_LB.z * tau;
    TA.x = m_TA.x * (1.0-tau) + m_TB.x * tau;
    TA.y = m_TA.y * (1.0-tau) + m_TB.y * tau;
    TA.z = m_TA.z * (1.0-tau) + m_TB.z * tau;
    PtC4.x = .75 * LA.x + .25 * TA.x;
    PtC4.y = .75 * LA.y + .25 * TA.y;
    PtC4.z = .75 * LA.z + .25 * TA.z;

    Chord = sqrt((LA.x-TA.x)*(LA.x-TA.x) + (LA.y-TA.y)*(LA.y-TA.y) + (LA.z-TA.z)*(LA.z-TA.z));

    if(m_pFoilA && m_pFoilB)
    {
        Area = (m_pFoilA->area() * tau + m_pFoilB->area() * (1.0-tau))*Chord*Chord;//m2
    }
    else
    {
        Area = 0.0;
    }
}


/**
 * Returns the absolute position of a specified strip.
 *
 * Returns the average span position of the strip; necessary for strips 'distorted' by the fuselage;
 *
 * @param k the 0-based index of the strip for which the position shall be returned.
 * @return the absolute position of the strip
 */
double Surface::stripSpanPos(int k)
{
    double YPos = 0.0;
    double ZPos = 0.0;

    for(int l=0; l<m_NXPanels; l++)
    {
        getPanel(k,l, xfl::MIDSURFACE);
        YPos += (LA.y+LB.y+TA.y+TB.y)/4.0;
        ZPos += (LA.z+LB.z+TA.z+TB.z)/4.0;
    }

    YPos /= m_NXPanels;
    ZPos /= m_NXPanels;

    YPos -= (m_LA.y + m_TA.y)/2.0;
    ZPos -= (m_LA.z + m_TA.z)/2.0;

    return sqrt(YPos*YPos+ZPos*ZPos);
}


/**
 * Returns the twist of the specified strip
 * @param k the 0-based index of the strip for which the leading point shall be returned.
 * @return the strip's twist.
 */
double Surface::twist(int k) const
{
    /*    getPanel(k, 0, MIDSURFACE);
    double y = (LA.y+LB.y+TA.y+TB.y)/4.0;
    return  m_TwistA + (m_TwistB-m_TwistA) *(y-m_LA.y)/(m_LB.y-m_LA.y);*/

    double y1=0.0, y2=0.0;
    getYDist(k, y1, y2);
    double tau = (y1+y2)/2.0;
    return m_TwistA *(1.0-tau) + m_TwistB*tau;
}


/**
 * Returns the relative left and right span positions of a given strip
 * @param k the 0-based index of the strip.
 * @param y1 a reference to the relative left span position.
 * @param y2 a reference to the relative left span position.
 */
void Surface::getYDist(int const &k, double &y1, double &y2) const
{
    //leading edge

    double YPanels, dk;
    YPanels = double(m_NYPanels);
    dk      = double(k);

    if(m_YDistType==xfl::COSINE)
    {
        //cosine case
        y1  = 1.0/2.0*(1.0-cos( dk*PI   /YPanels));
        y2  = 1.0/2.0*(1.0-cos((dk+1)*PI/YPanels));
    }
    else if(m_YDistType== xfl::INVERSESINE)
    {
        //sine case
        y1  = 1.0*(sin( dk*PI   /2.0/YPanels));
        y2  = 1.0*(sin((dk+1)*PI/2.0/YPanels));
    }
    else if(m_YDistType==xfl::SINE)
    {
        //-sine case
        y1  = 1.0*(1.-cos( dk*PI   /2.0/YPanels));
        y2  = 1.0*(1.-cos((dk+1)*PI/2.0/YPanels));
    }
    else
    {
        //equally spaced case
        y1 =  dk     /YPanels;
        y2 = (dk+1.0)/YPanels;
    }
}


/**
 * Initializes the Surface
 */
void Surface::init()
{
    //    Vector3d DL, DC;
    //    DL.set(m_LB.x-m_LA.x, m_LB.y-m_LA.y, m_LB.z-m_LA.z);
    //    DC.set(m_TA.x-m_LA.x, m_TA.y-m_LA.y, m_TA.z-m_LA.z);
    //    Length = DL.VAbs();
    //    Chord  = DC.VAbs();
    //    u.Set(DC.x/Chord,  DC.y/Chord,  DC.z/Chord);
    //    v.Set(DL.x/Length, DL.y/Length, DL.z/Length);

    m_bIsTipLeft   = false;
    m_bIsTipRight  = false;
    m_bIsLeftSurf  = false;
    m_bIsRightSurf = false;

    /*    Vector3d LATB, TALB;

    LATB = m_TB - m_LA;
    TALB = m_LB - m_TA;
    Normal = LATB * TALB;
    Normal.normalize();*/
}


void Surface::setCornerPoints(const Vector3d &PLA, const Vector3d &PTA, const Vector3d &PLB, const Vector3d &PTB)
{
    m_LA = PLA;
    m_LB = PLB;
    m_TA = PTA;
    m_TB = PTB;
}


/**
 * Returns true if the specified panel is located on the T.E. flap
 * @param p the index of the panel
 * @return true if the panel is located on the T.E. flap
 */
bool Surface::isFlapPanel(int p) const
{
    int pp;
    for(pp=0; pp<m_nFlapPanels; pp++)
    {
        if (p==m_FlapPanel[pp]) return true;
    }
    return false;
}


/**
 * Returns true if the specified panel is located on the T.E. flap
 * @param pPanel a pointer to the panel object
 * @return true if the panel is located on the T.E. flap
 */
bool Surface::isFlapPanel(Panel const *pPanel) const
{
    int pp;
    for(pp=0; pp<m_nFlapPanels; pp++)
    {
        if (pPanel->m_iElement==m_FlapPanel[pp]) return true;
    }
    return false;
}


/**
 * Returns true if the specified node is located on the T.E. flap
 * @param nNode the index of the node
 * @return true if the node is located on the T.E. flap
 */
bool Surface::isFlapNode(int nNode) const
{
    int pp;
    for(pp=0; pp<m_nFlapPanels; pp++)
    {
        if(nNode==s_pPanel[m_FlapPanel[pp]].m_iLA) return true;
        if(nNode==s_pPanel[m_FlapPanel[pp]].m_iLB) return true;
        if(nNode==s_pPanel[m_FlapPanel[pp]].m_iTA) return true;
        if(nNode==s_pPanel[m_FlapPanel[pp]].m_iTB) return true;
    }
    return false;
}


/** Clears the array of flap panel and node references */
void Surface::resetFlap()
{
    int i;
    for(i=0; i<200; i++)
    {
        m_FlapPanel[i] = 30000;
        m_FlapNode[i]  = 30000;
    }
    m_nFlapPanels = 0;
    m_nFlapNodes = 0;
}



/**
 * Rotates a flap panels around its hinge axis.
 * @param Angle the rotation angle in degrees
 * @return false if the left and right Foil objects do not have an identical default flap angle, true otherwise.
 */
bool Surface::rotateFlap(double Angle)
{
    //The average angle between the two tip foil is cancelled
    //Instead, the Panels are rotated by Angle around the hinge point and hinge vector

    double alpha0(0);
    Quaternion Quat;
    Vector3d R, S;

    if(m_pFoilA && m_pFoilB)
    {
        //get the approximate initial angle
        if(qAbs(m_pFoilA->m_TEFlapAngle - m_pFoilB->m_TEFlapAngle)>0.1)
        {
//            QString error = QObject::tr("Continuous foils for surface do not have the same initial flap angle... aborting\n");
            return false;
        }
        alpha0 = (m_pFoilA->m_TEFlapAngle + m_pFoilB->m_TEFlapAngle)/2.0;

        Quat.set(Angle-alpha0, m_HingeVector);


        for (int k=0; k<m_nFlapNodes; k++)
        {
            R.x = s_pNode[m_FlapNode[k]].x - m_HingePoint.x;
            R.y = s_pNode[m_FlapNode[k]].y - m_HingePoint.y;
            R.z = s_pNode[m_FlapNode[k]].z - m_HingePoint.z;
            Quat.conjugate(R,S);

            s_pNode[m_FlapNode[k]].x = S.x + m_HingePoint.x;
            s_pNode[m_FlapNode[k]].y = S.y + m_HingePoint.y;
            s_pNode[m_FlapNode[k]].z = S.z + m_HingePoint.z;
        }

        for(int l=0; l<m_nFlapPanels; l++)
        {
            int k = m_FlapPanel[l];
            if(s_pPanel[k].m_Pos==xfl::BOTSURFACE)
            {
                s_pPanel[k].setPanelFrame(
                            s_pNode[s_pPanel[k].m_iLB],
                            s_pNode[s_pPanel[k].m_iLA],
                            s_pNode[s_pPanel[k].m_iTB],
                            s_pNode[s_pPanel[k].m_iTA]);
            }
            else
            {
                s_pPanel[k].setPanelFrame(
                            s_pNode[s_pPanel[k].m_iLA],
                            s_pNode[s_pPanel[k].m_iLB],
                            s_pNode[s_pPanel[k].m_iTA],
                            s_pNode[s_pPanel[k].m_iTB]);
            }
        }
    }

    return true;
}


/**
 * Rotates the entire surface around the x-axis
 * @param O a point on the axis of rotation
 * @param XTilt the rotation angle in degrees
 */
void Surface::rotateX(Vector3d const&O, double XTilt)
{
    m_LA.rotateX(O, XTilt);
    m_LB.rotateX(O, XTilt);
    m_TA.rotateX(O, XTilt);
    m_TB.rotateX(O, XTilt);
    m_HingePoint.rotateX(O, XTilt);

    m_Normal.rotateX(XTilt);
    NormalA.rotateX(XTilt);
    NormalB.rotateX(XTilt);
    m_HingeVector.rotateX(XTilt);
}


/**
 * Rotates the entire surface around the y-axis
 * @param O a point on the axis of rotation
 * @param YTilt the rotation angle in degrees
 */
void Surface::rotateY(Vector3d const &O, double YTilt)
{
    m_LA.rotateY(O, YTilt);
    m_LB.rotateY(O, YTilt);
    m_TA.rotateY(O, YTilt);
    m_TB.rotateY(O, YTilt);
    m_HingePoint.rotateY(O, YTilt);

    m_Normal.rotateY(YTilt);
    NormalA.rotateY(YTilt);
    NormalB.rotateY(YTilt);
    m_HingeVector.rotateY(YTilt);
}


/**
 * Rotates the entire surface around the z-axis
 * @param O a point on the axis of rotation
 * @param ZTilt the rotation angle in degrees
 */
void Surface::rotateZ(Vector3d const &O, double ZTilt)
{
    m_LA.rotateZ(O, ZTilt);
    m_LB.rotateZ(O, ZTilt);
    m_TA.rotateZ(O, ZTilt);
    m_TB.rotateZ(O, ZTilt);
    m_HingePoint.rotateZ(O, ZTilt);

    Vector3d Origin(0.0,0.0,0.0);
    m_Normal.rotateZ(Origin, ZTilt);
    NormalA.rotateZ(Origin, ZTilt);
    NormalB.rotateZ(Origin, ZTilt);
    m_HingeVector.rotateZ(Origin, ZTilt);
}

/**
 * Initializes the flap data
 */
void Surface::setFlap()
{
    Vector3d N;
    if(m_pFoilA && m_pFoilA->m_bTEFlap)
    {
        m_posATE = m_pFoilA->m_TEXHinge/100.0;
        if(m_posATE>1.0) m_posATE = 1.0; else if(m_posATE<0.0) m_posATE = 0.0;
    }
    else m_posATE = 1.0;

    if(m_pFoilB && m_pFoilB->m_bTEFlap)
    {
        m_posBTE = m_pFoilB->m_TEXHinge/100.0;
        if(m_posBTE>1.0) m_posBTE = 1.0; else if(m_posBTE<0.0) m_posBTE = 0.0;
    }
    else m_posBTE = 1.0;

    if(m_pFoilA && m_pFoilB) m_bTEFlap = m_pFoilA->m_bTEFlap && m_pFoilB->m_bTEFlap;
    else                     m_bTEFlap = false;


    if(m_pFoilA && m_pFoilB && m_pFoilA->m_bTEFlap && m_pFoilB->m_bTEFlap)
    {
        Vector3d HB;
        //create a hinge unit vector and initialize hinge moment
        getSurfacePoint(m_posATE, m_posBTE, 0.0, xfl::MIDSURFACE, m_HingePoint,N);
        getSurfacePoint(m_posATE, m_posBTE, 1.0, xfl::MIDSURFACE, HB, N);
        m_HingeVector = HB-m_HingePoint;
        m_HingeVector.normalize();
    }
}


/** Sets the surface average normal vector */
void Surface::setNormal()
{
    Vector3d LATB, TALB;
    LATB = m_TB - m_LA;
    TALB = m_LB - m_TA;
    m_Normal = LATB * TALB;
    m_Normal.normalize();
}


/**
 * Translates the entire Surface.
 * @param T the translation vector.
 */
void Surface::translate(Vector3d const &T)
{
    m_LA.translate(T);
    m_LB.translate(T);
    m_TA.translate(T);
    m_TB.translate(T);
    m_HingePoint.translate(T);
}

/**
 * Translates the entire Surface.
 * @param tx the x-component of the translation.
 */
void Surface::translate(double tx, double ty, double tz)
{
    m_LA.translate(tx, ty, tz);
    m_LB.translate(tx, ty, tz);
    m_TA.translate(tx, ty, tz);
    m_TB.translate(tx, ty, tz);
    m_HingePoint.translate(tx, ty, tz);
}


/**
 * Creates relative position of the master points at the left and right sides of the Surface.
 * The chordwise panel distribution is set i.a.w. with the flap hinges, if any.
 * The positions are stored in the member variables m_xPointA and m_xPointB.
 */
void Surface::createXPoints()
{
    int NXFlapA(0), NXFlapB(0), NXLeadA(0), NXLeadB(0);
    double dl(0), dl2(0);
    double xHingeA(0), xHingeB(0);
    if(m_pFoilA && m_pFoilA->m_bTEFlap) xHingeA=m_pFoilA->m_TEXHinge/100.0; else xHingeA=1.0;
    if(m_pFoilB && m_pFoilB->m_bTEFlap) xHingeB=m_pFoilB->m_TEXHinge/100.0; else xHingeB=1.0;

    NXFlapA = int((1.0-xHingeA) * double(m_NXPanels) *1.000123);// to avoid numerical errors if exact division
    NXFlapB = int((1.0-xHingeB) * double(m_NXPanels) *1.000123);

    if(m_pFoilA && m_pFoilA->m_bTEFlap && NXFlapA==0) NXFlapA++;
    if(m_pFoilB && m_pFoilB->m_bTEFlap && NXFlapB==0) NXFlapB++;

    // uniformize the number of flap panels if flaps are defined at each end
    if(NXFlapA>0 && NXFlapB>0)
    {
        int n = int((NXFlapA+NXFlapB)/2);
        NXFlapA = n;
        NXFlapB = n;
    }

    NXLeadA = m_NXPanels - NXFlapA;
    NXLeadB = m_NXPanels - NXFlapB;

    m_NXFlap  = qMax(NXFlapA, NXFlapB);
    if(m_NXFlap>m_NXPanels/2) m_NXFlap=int(m_NXPanels/2);
    m_NXLead  = m_NXPanels - m_NXFlap;

    for(int l=0; l<NXFlapA; l++)
    {
        dl =  double(l);
        dl2 = double(NXFlapA);
        if(m_XDistType==xfl::COSINE)
            m_xPointA[l] = 1.0 - (1.0-xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            m_xPointA[l] = 1.0 - (1.0-xHingeA) * (dl/dl2);
    }

    for(int l=0; l<NXLeadA; l++)
    {
        dl =  double(l);
        dl2 = double(NXLeadA);
        if(m_XDistType==xfl::COSINE)
            m_xPointA[l+NXFlapA] = xHingeA - (xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            m_xPointA[l+NXFlapA] = xHingeA - (xHingeA) * (dl/dl2);
    }

    for(int l=0; l<NXFlapB; l++)
    {
        dl =  double(l);
        dl2 = double(NXFlapB);
        if(m_XDistType==xfl::COSINE)
            m_xPointB[l] = 1.0 - (1.0-xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            m_xPointB[l] = 1.0 - (1.0-xHingeB) * (dl/dl2);
    }

    for(int l=0; l<NXLeadB; l++)
    {
        dl =  double(l);
        dl2 = double(NXLeadB);
        if(m_XDistType==xfl::COSINE)
            m_xPointB[l+NXFlapB] = xHingeB - (xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            m_xPointB[l+NXFlapB] = xHingeB - (xHingeB) * (dl/dl2);
    }

    m_xPointA[m_NXPanels] = 0.0;
    m_xPointB[m_NXPanels] = 0.0;
}


/**
 * Sets the surface twist - method 1
 */
void Surface::setTwist()
{
    Vector3d A4, B4, U, T;

    A4 = m_LA *3.0/4.0 + m_TA * 1/4.0;
    B4 = m_LB *3.0/4.0 + m_TB * 1/4.0;

    // create a vector perpendicular to NormalA and x-axis
    T.x = 0.0;
    T.y = +NormalA.z;
    T.z = -NormalA.y;
    //rotate around this axis
    U = m_LA-A4;
    U.rotate(T, m_TwistA);
    m_LA = A4+ U;

    U = m_TA-A4;
    U.rotate(T, m_TwistA);
    m_TA = A4 + U;

    NormalA.rotate(T, m_TwistA);

    // create a vector perpendicular to NormalB and x-axis
    T.x = 0.0;
    T.y = +NormalB.z;
    T.z = -NormalB.y;

    U = m_LB-B4;
    U.rotate(T, m_TwistB);
    m_LB = B4+ U;

    U = m_TB-B4;
    U.rotate(T, m_TwistB);
    m_TB = B4 + U;

    NormalB.rotate(T, m_TwistB);
}



double Surface::spanLength() const
{
    return sqrt((m_LB.y - m_LA.y)*(m_LB.y - m_LA.y) + (m_LB.z - m_LA.z)*(m_LB.z - m_LA.z));
}


void Surface::setPanelPointers(Panel *pPanel, Vector3d *pNode)
{
    s_pPanel = pPanel;
    s_pNode = pNode;
}


/**
 * Creates the master points on the left and right ends.
 * One of the most difficult part of the code to implement.
 * @param pBody a pointer to the Body object, or NULL if none.
 * @param dx the x-component of the translation to apply to the body.
 * @param dz the z-component of the translation to apply to the body.
 */
void Surface::setMeshSidePoints(Body * pBody, double dx, double dz)
{
    double alpha_dA(0), alpha_dB(0), cosdA(0), cosdB(0);
    Vector3d N, A4, B4, TA4, TB4, I;
    Vector3d V, Ua, Ub;
    Body TBody;
    if(pBody)
    {
        TBody.duplicate(pBody);
        TBody.translate(dx, 0.0, dz);
    }

    V = m_Normal * NormalA;
    Ua = (m_LA - m_TA).normalized();
    //    double sindA = -V.dot(Vector3d(1.0,0.0,0.0));
    double sindA = V.dot(Ua);
    if(sindA> 1.0) sindA = 1.0;
    if(sindA<-1.0) sindA = -1.0;
    alpha_dA = asin(sindA);
    cosdA = cos(alpha_dA);
    alpha_dA *= 180.0/PI;

    V = m_Normal * NormalB;
    Ub = (m_LB-m_TB).normalized();
    double sindB = V.dot(Ub);
    //    double sindB = V.VAbs();
    if(sindB> 1.0) sindB = 1.0;
    if(sindB<-1.0) sindB = -1.0;
    alpha_dB = asin(sindB);
    cosdB = cos(alpha_dB);
    alpha_dB *= 180.0/PI;

    //create the quarter chord centers of rotation for the twist
    A4 = m_LA *3.0/4.0 + m_TA * 1.0/4.0;
    B4 = m_LB *3.0/4.0 + m_TB * 1.0/4.0;

    // create the vectors perpendicular to the side Normals and to the x-axis
    TA4.x = 0.0;
    TA4.y = +NormalA.z;
    TA4.z = -NormalA.y;

    TB4.x = 0.0;
    TB4.y = +NormalB.z;
    TB4.z = -NormalB.y;

    m_SideA.resize(  m_NXPanels+1);
    m_SideA_B.resize(m_NXPanels+1);
    m_SideA_T.resize(m_NXPanels+1);
    m_SideB.resize(  m_NXPanels+1);
    m_SideB_B.resize(m_NXPanels+1);
    m_SideB_T.resize(m_NXPanels+1);

    for (int l=0; l<=m_NXPanels; l++)
    {
        getSidePoint(m_xPointA.at(l), false, xfl::MIDSURFACE, m_SideA[l], N);
        getSidePoint(m_xPointA.at(l), false, xfl::TOPSURFACE, m_SideA_T[l], N);
        getSidePoint(m_xPointA.at(l), false, xfl::BOTSURFACE, m_SideA_B[l], N);

        //scale the thickness
        double Ox = m_xPointA.at(l);
        double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
        double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
        m_SideA[l].y   = Oy +(m_SideA.at(l).y   - Oy)/cosdA;
        m_SideA[l].z   = Oz +(m_SideA.at(l).z   - Oz)/cosdA;
        m_SideA_T[l].y = Oy +(m_SideA_T.at(l).y - Oy)/cosdA;
        m_SideA_T[l].z = Oz +(m_SideA_T.at(l).z - Oz)/cosdA;
        m_SideA_B[l].y = Oy +(m_SideA_B.at(l).y - Oy)/cosdA;
        m_SideA_B[l].z = Oz +(m_SideA_B.at(l).z - Oz)/cosdA;

        //rotate the point about the foil's neutral line to account for dihedral
        m_SideA[  l].rotate(m_LA, Ua, alpha_dA);
        m_SideA_T[l].rotate(m_LA, Ua, alpha_dA);
        m_SideA_B[l].rotate(m_LA, Ua, alpha_dA);

        getSidePoint(m_xPointB[l], true, xfl::MIDSURFACE, m_SideB[l], N);
        getSidePoint(m_xPointB[l], true, xfl::TOPSURFACE, m_SideB_T[l], N);
        getSidePoint(m_xPointB[l], true, xfl::BOTSURFACE, m_SideB_B[l], N);

        //scale the thickness
        Ox = m_xPointB[l];
        Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
        Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
        m_SideB[l].y   = Oy +(m_SideB.at(l).y   - Oy)/cosdB;
        m_SideB[l].z   = Oz +(m_SideB.at(l).z   - Oz)/cosdB;
        m_SideB_T[l].y = Oy +(m_SideB_T.at(l).y - Oy)/cosdB;
        m_SideB_T[l].z = Oz +(m_SideB_T.at(l).z - Oz)/cosdB;
        m_SideB_B[l].y = Oy +(m_SideB_B.at(l).y - Oy)/cosdB;
        m_SideB_B[l].z = Oz +(m_SideB_B.at(l).z - Oz)/cosdB;

        //rotate the point about the foil's neutral line to account for dihedral
        m_SideB[  l].rotate(m_LB, Ub, alpha_dB);
        m_SideB_T[l].rotate(m_LB, Ub, alpha_dB);
        m_SideB_B[l].rotate(m_LB, Ub, alpha_dB);


        if(pBody && m_bIsCenterSurf && m_bIsLeftSurf)
        {
            if(TBody.intersect(m_SideA_B.at(l), m_SideB_B.at(l), I, false))
            {
                m_SideB_B[l] = I;
                m_bJoinRight = false;
            }
            if(TBody.intersect(m_SideA_T.at(l), m_SideB_T.at(l), I, false))
            {
                m_SideB_T[l] = I;
                m_bJoinRight = false;
            }
            if(TBody.intersect(m_SideA.at(l), m_SideB.at(l), I, false))
            {
                m_SideB[l] = I;
                m_bJoinRight = false;
            }
        }
        else if(pBody && m_bIsCenterSurf && m_bIsRightSurf)
        {
            if(TBody.intersect(m_SideA_B.at(l), m_SideB_B.at(l), I, true))
            {
                m_SideA_B[l] = I;
            }
            if(TBody.intersect(m_SideA_T.at(l), m_SideB_T.at(l), I, true))
            {
                m_SideA_T[l] = I;
            }
            if(TBody.intersect(m_SideA.at(l), m_SideB.at(l), I, true))
            {
                m_SideA[l] = I;
            }
        }
    }

    //merge trailing edge nodes in case the foil has a T.E. gap

    Vector3d Node;

    Node = (m_SideA_B[0] + m_SideA_T[0])/2.0;
    m_SideA_B[0].set(Node);
    m_SideA_T[0].set(Node);

    Node = (m_SideB_B[0] + m_SideB_T[0])/2.0;
    m_SideB_B[0].set(Node);
    m_SideB_T[0].set(Node);
}










