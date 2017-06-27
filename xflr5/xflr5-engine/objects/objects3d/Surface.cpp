/****************************************************************************

	Surface Class
	Copyright (C) 2005-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <QtDebug>
#include <math.h>
#include "Surface.h"
#include <objects3d/Quaternion.h>
#include <objects2d/Foil.h>
#include "Body.h"
#include <objects2d/Vector3d.h>
#include "WingSection.h"

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
	m_XDistType = XFLR5::COSINE;
	m_YDistType = XFLR5::UNIFORM;

	m_pLeftSurface = m_pRightSurface = NULL;
	m_pFoilA   = NULL;
	m_pFoilB   = NULL;
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

}


/**
 * Adds the reference of thE input panel to the array of flap panel indexes.
 * @param pPanel the pointer of the panel to add to the flap panel list.
 */
void Surface::addFlapPanel(Panel *pPanel)
{
	bool bFound = false;
	int i;

	//Add Nodes

	for (i=0; i<m_nFlapNodes; i++)
	{
		bFound = bFound && pPanel->m_iLA==m_FlapNode[i];
		if(pPanel->m_iLA== m_FlapNode[i])
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		m_FlapNode[m_nFlapNodes] = pPanel->m_iLA;
		m_nFlapNodes++;
	}

	bFound = false;
	for (i=0; i< m_nFlapNodes; i++)
	{
		if(pPanel->m_iLB== m_FlapNode[i])
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		m_FlapNode[m_nFlapNodes] = pPanel->m_iLB;
		m_nFlapNodes++;
	}

	for (i=0; i< m_nFlapNodes; i++)
	{
		if(pPanel->m_iTA== m_FlapNode[i])
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		m_FlapNode[m_nFlapNodes] = pPanel->m_iTA;
		m_nFlapNodes++;
	}

	bFound = false;
	for (i=0; i< m_nFlapNodes; i++)
	{
		if(pPanel->m_iTB== m_FlapNode[i])
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		m_FlapNode[m_nFlapNodes] = pPanel->m_iTB;
		m_nFlapNodes++;
	}

	//Add panel;
	bFound=false;
	for(i=0; i<m_nFlapPanels; i++)
	{
		if(pPanel->m_iElement==m_FlapPanel[i])
		{
			bFound =true;
			break;
		}
	}
	if(!bFound)
	{
		m_FlapPanel[m_nFlapPanels] = pPanel->m_iElement;
		m_nFlapPanels++;
	}
}

/**
 * Copy the data from another Surface object to this Surface
 * @param Surface the source Surface from which the data shall be duplicated
 */
void Surface::copy(Surface *pSurface)
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

	Normal  = pSurface->Normal;
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
void Surface::getC4(int k, Vector3d &Pt, double &tau)
{
	getPanel(k,m_NXPanels-1,MIDSURFACE);
	double xl = (LA.x+LB.x)/2.0;
	double yl = (LA.y+LB.y)/2.0;
	double zl = (LA.z+LB.z)/2.0;
	getPanel(k,0,MIDSURFACE);
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
double Surface::chord(int k)
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
double Surface::chord(double tau)
{
	//assumes LA-TB have already been loaded
    Vector3d V1, V2;
    double ChordA, ChordB;

	V1 = m_TA-m_LA;
	V2 = m_TB-m_LB;

	ChordA = V1.VAbs();
	ChordB = V2.VAbs();

	return ChordA + (ChordB-ChordA) * qAbs(tau);
}


/**
 * Returns the strip's offset in the x direction at the specified relative span position.
 * @param tau the relative percentage of the Surface's span length
 * @return the offset in the x-direction
 */
double Surface::offset(double tau)
{
	//chord spacing
	return m_LA.x + (m_LB.x-m_LA.x) * qAbs(tau);
}


/**
 * Returns the area of the virtual foil at a specified relative span position. Used in Inertia calaculations.
 * @param tau the relative percentage of the Surface's span length
 * @return the cross area at the specified location
 */
double Surface::foilArea(double tau)
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
void Surface::getNormal(double yrel, Vector3d &N)
{
	N = NormalA * (1.0-yrel) + NormalB * yrel;
	N.normalize();
}



/**
 * Returns the leading point of the specified strip
 * @param k the 0-based index of the strip for which the leading point shall be returned.
 * @param C the strip's leading point.
 */
void Surface::getLeadingPt(int k, Vector3d &C)
{
	getPanel(k,m_NXPanels-1, MIDSURFACE);

	C.x    = (LA.x+LB.x)/2.0;
	C.y    = (LA.y+LB.y)/2.0;
	C.z    = (LA.z+LB.z)/2.0;
}



/**
 * Returns the trailing point of the specified strip
 * @param k the 0-based index of the strip for which the trailing point shall be returned.
 * @param C the strip's leading point.
 */
void Surface::getTrailingPt(int k, Vector3d &C)
{
	getPanel(k,0,MIDSURFACE);

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
void Surface::getPanel(int const &k, int const &l, enumPanelPosition pos)
{
    double y1, y2;
	getYDist(k,y1,y2);
	if(pos==MIDSURFACE)
	{
		LA.x = SideA[l+1].x * (1.0-y1) + SideB[l+1].x* y1;
		LA.y = SideA[l+1].y * (1.0-y1) + SideB[l+1].y* y1;
		LA.z = SideA[l+1].z * (1.0-y1) + SideB[l+1].z* y1;
		TA.x = SideA[l].x   * (1.0-y1) + SideB[l].x  * y1;
		TA.y = SideA[l].y   * (1.0-y1) + SideB[l].y  * y1;
		TA.z = SideA[l].z   * (1.0-y1) + SideB[l].z  * y1;
		LB.x = SideA[l+1].x * (1.0-y2) + SideB[l+1].x* y2;
		LB.y = SideA[l+1].y * (1.0-y2) + SideB[l+1].y* y2;
		LB.z = SideA[l+1].z * (1.0-y2) + SideB[l+1].z* y2;
		TB.x = SideA[l].x   * (1.0-y2) + SideB[l].x  * y2;
		TB.y = SideA[l].y   * (1.0-y2) + SideB[l].y  * y2;
		TB.z = SideA[l].z   * (1.0-y2) + SideB[l].z  * y2;
	}
	else if (pos==BOTSURFACE)
	{
		LA = SideA_B[l+1] * (1.0-y1) + SideB_B[l+1]* y1;
		TA = SideA_B[l]   * (1.0-y1) + SideB_B[l]  * y1;
		LB = SideA_B[l+1] * (1.0-y2) + SideB_B[l+1]* y2;
		TB = SideA_B[l]   * (1.0-y2) + SideB_B[l]  * y2;

		LA.x = SideA_B[l+1].x * (1.0-y1) + SideB_B[l+1].x* y1;
		LA.y = SideA_B[l+1].y * (1.0-y1) + SideB_B[l+1].y* y1;
		LA.z = SideA_B[l+1].z * (1.0-y1) + SideB_B[l+1].z* y1;
		TA.x = SideA_B[l].x   * (1.0-y1) + SideB_B[l].x  * y1;
		TA.y = SideA_B[l].y   * (1.0-y1) + SideB_B[l].y  * y1;
		TA.z = SideA_B[l].z   * (1.0-y1) + SideB_B[l].z  * y1;
		LB.x = SideA_B[l+1].x * (1.0-y2) + SideB_B[l+1].x* y2;
		LB.y = SideA_B[l+1].y * (1.0-y2) + SideB_B[l+1].y* y2;
		LB.z = SideA_B[l+1].z * (1.0-y2) + SideB_B[l+1].z* y2;
		TB.x = SideA_B[l].x   * (1.0-y2) + SideB_B[l].x  * y2;
		TB.y = SideA_B[l].y   * (1.0-y2) + SideB_B[l].y  * y2;
		TB.z = SideA_B[l].z   * (1.0-y2) + SideB_B[l].z  * y2;
	}
	else if (pos==TOPSURFACE)
	{
		LA.x = SideA_T[l+1].x * (1.0-y1) + SideB_T[l+1].x* y1;
		LA.y = SideA_T[l+1].y * (1.0-y1) + SideB_T[l+1].y* y1;
		LA.z = SideA_T[l+1].z * (1.0-y1) + SideB_T[l+1].z* y1;
		TA.x = SideA_T[l].x   * (1.0-y1) + SideB_T[l].x  * y1;
		TA.y = SideA_T[l].y   * (1.0-y1) + SideB_T[l].y  * y1;
		TA.z = SideA_T[l].z   * (1.0-y1) + SideB_T[l].z  * y1;
		LB.x = SideA_T[l+1].x * (1.0-y2) + SideB_T[l+1].x* y2;
		LB.y = SideA_T[l+1].y * (1.0-y2) + SideB_T[l+1].y* y2;
		LB.z = SideA_T[l+1].z * (1.0-y2) + SideB_T[l+1].z* y2;
		TB.x = SideA_T[l].x   * (1.0-y2) + SideB_T[l].x  * y2;
		TB.y = SideA_T[l].y   * (1.0-y2) + SideB_T[l].y  * y2;
		TB.z = SideA_T[l].z   * (1.0-y2) + SideB_T[l].z  * y2;
	}
}


/**
 * Returns the strip width at a specified index
 * @param k the index of the strip 0<=k<m_NYPanels
 * @return the strip width
 */
double Surface::stripWidth(int k)
{
	getPanel(k, 0, MIDSURFACE);
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
void Surface::getSidePoint(double xRel, bool bRight, enumPanelPosition pos, Vector3d &Point, Vector3d &PtNormal)
{
	Vector3d foilPt(xRel,0.0,0.0);

    if(!bRight)
    {
        if(pos==MIDSURFACE && m_pFoilA)      foilPt = m_pFoilA->midYRel(xRel);
		else if(pos==TOPSURFACE && m_pFoilA) foilPt = m_pFoilA->upperYRel(xRel, PtNormal.x, PtNormal.z);
		else if(pos==BOTSURFACE && m_pFoilA) foilPt = m_pFoilA->lowerYRel(xRel, PtNormal.x, PtNormal.z);

        Point = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        Point +=  Normal * foilPt.y*chord(0.0);
    }
    else
    {
        if(pos==MIDSURFACE && m_pFoilB)      foilPt = m_pFoilB->midYRel(xRel);
		else if(pos==TOPSURFACE && m_pFoilB) foilPt = m_pFoilB->upperYRel(xRel, PtNormal.x, PtNormal.z);
		else if(pos==BOTSURFACE && m_pFoilB) foilPt = m_pFoilB->lowerYRel(xRel, PtNormal.x, PtNormal.z);

        Point = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        Point +=  Normal * foilPt.y*chord(1.0);
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
void Surface::getSidePoints(enumPanelPosition pos,
							Body * pBody,
							Vector3d *PtA, Vector3d *PtB, Vector3d *NA, Vector3d *NB, int nPoints)
{
	double xRel;
	Vector3d A4, B4, TA4, TB4;

/*	double cosdA = Normal.dot(NormalA);
	double cosdB = Normal.dot(NormalB);

	if(cosdA>1.0) cosdA = 1.0;
	if(cosdB>1.0) cosdB = 1.0;
	if(cosdA<-1.0) cosdA = -1.0;
	if(cosdB<-1.0) cosdB = -1.0;

	Vector3d x(1.0,0.0,0.0);
	double alpha_dA = atan2((NormalA * Normal).dot(x), Normal.dot(NormalA))*180.0/PI;
	double alpha_dB = atan2((NormalB * Normal).dot(x), Normal.dot(NormalB))*180.0/PI;*/

	Vector3d V = Normal * NormalA;
	Vector3d U = (m_TA - m_LA).normalized();
//	double sindA = -V.dot(Vector3d(1.0,0.0,0.0));
	double sindA = -V.dot(U);
	if(sindA> 1.0) sindA = 1.0;
	if(sindA<-1.0) sindA = -1.0;
	double alpha_dA = asin(sindA);
	double cosdA = cos(alpha_dA);
	alpha_dA *= 180.0/PI;

	V = Normal * NormalB;
	U = (m_TB-m_LB).normalized();
//	double sindB = -V.dot(Vector3d(1.0,0.0,0.0));
	double sindB = -V.dot(U);
	if(sindB> 1.0) sindB = 1.0;
	if(sindB<-1.0) sindB = -1.0;
	double alpha_dB = asin(sindB);
	double cosdB = cos(alpha_dB);
	alpha_dB *= 180.0/PI;


	double delta = -atan(Normal.y / Normal.z)*180.0/PI;
//	double delta = -atan2(Normal.y,  Normal.z)*180.0/PI;

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
		xRel  = 1.0/2.0*(1.0-cos( (double)i*PI   /(double)(nPoints-1)));

		NA[i].set(0.0,0.0,0.0);
		getSidePoint(xRel, false, pos, PtA[i], NA[i]);

		//scale the thickness
		double Ox = xRel;
		double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
		double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
		PtA[i].y   = Oy +(PtA[i].y - Oy)/cosdA;
		PtA[i].z   = Oz +(PtA[i].z - Oz)/cosdA;
		PtA[i].rotate(m_LA, m_LA-m_TA, +alpha_dA);
		NA[i].rotate(Vector3d(1.0,0.0,0.0), delta);

		NB[i].set(0.0,0.0,0.0);
		getSidePoint(xRel, true,  pos, PtB[i], NB[i]);
		Ox = xRel;
		Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
		Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
		PtB[i].y   = Oy +(PtB[i].y - Oy)/cosdB;
		PtB[i].z   = Oz +(PtB[i].z - Oz)/cosdB;
		PtB[i].rotate(m_LB, m_LB-m_TB, +alpha_dB);
		NB[i].rotate(Vector3d(1.0,0.0,0.0), delta);



/*		double sweep_i = -atan2((PtB[i].x-PtA[i].x), (PtB[i].y-PtA[i].y)) * 180.0/PI;
		NA[i].rotate(Vector3d(0.0,0.0,1.0), sweep_i);
		NB[i].rotate(Vector3d(0.0,0.0,1.0), sweep_i);*/

		if(pBody && m_bIsCenterSurf && m_bIsLeftSurf)
		{
			pBody->intersect(PtA[i], PtB[i], PtB[i], false);
		}
		else if(pBody && m_bIsCenterSurf && m_bIsRightSurf)
		{
			pBody->intersect(PtA[i], PtB[i], PtA[i], true);
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
void Surface::getSurfacePoint(double xArel, double xBrel, double yrel, enumPanelPosition pos, Vector3d &Point, Vector3d &PtNormal)
{
    Vector3d APt, BPt, foilPt;
	double nx, ny;
    if(pos==MIDSURFACE && m_pFoilA && m_pFoilB)
    {
        foilPt = m_pFoilA->midYRel(xArel);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  Normal * foilPt.y*chord(0.0);

        foilPt = m_pFoilB->midYRel(xBrel);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  Normal * foilPt.y*chord(1.0);

    }
    else if(pos==TOPSURFACE && m_pFoilA && m_pFoilB)
    {
		foilPt = m_pFoilA->upperYRel(xArel, nx, ny);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  Normal * foilPt.y*chord(0.0);

		foilPt = m_pFoilB->upperYRel(xBrel, nx, ny);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  Normal * foilPt.y*chord(1.0);

    }
    else if(pos==BOTSURFACE && m_pFoilA && m_pFoilB)
    {
		foilPt = m_pFoilA->lowerYRel(xArel, nx, ny);
        APt = m_LA * (1.0-foilPt.x) + m_TA * foilPt.x;
        APt +=  Normal * foilPt.y*chord(0.0);

		foilPt = m_pFoilB->lowerYRel(xBrel, nx, ny);
        BPt = m_LB * (1.0-foilPt.x) + m_TB * foilPt.x;
        BPt +=  Normal * foilPt.y*chord(1.0);
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
void Surface::getSection(double const &tau, double &Chord, double &Area, Vector3d &PtC4)
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
		getPanel(k,l, MIDSURFACE);
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
double Surface::twist(int k)
{
	getPanel(k, 0, MIDSURFACE);
	double y = (LA.y+LB.y+TA.y+TB.y)/4.0;
	return  m_TwistA + (m_TwistB-m_TwistA) *(y-m_LA.y)/(m_LB.y-m_LA.y);
}


/**
 * Returns the relative left and right span positions of a given strip
 * @param k the 0-based index of the strip.
 * @param y1 a reference to the relative left span position.
 * @param y2 a reference to the relative left span position.
 */
void Surface::getYDist(int const &k, double &y1, double &y2)
{
	//leading edge

	double YPanels, dk;
	YPanels = (double)m_NYPanels;
	dk      = (double)k;

	if(m_YDistType==XFLR5::COSINE)
	{
		//cosine case
		y1  = 1.0/2.0*(1.0-cos( dk*PI   /YPanels));
		y2  = 1.0/2.0*(1.0-cos((dk+1)*PI/YPanels));
	}
	else if(m_YDistType== XFLR5::INVERSESINE)
	{
		//sine case
		y1  = 1.0*(sin( dk*PI   /2.0/YPanels));
		y2  = 1.0*(sin((dk+1)*PI/2.0/YPanels));
	}
	else if(m_YDistType==XFLR5::SINE)
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
//	Vector3d DL, DC;
//	DL.set(m_LB.x-m_LA.x, m_LB.y-m_LA.y, m_LB.z-m_LA.z);
//	DC.set(m_TA.x-m_LA.x, m_TA.y-m_LA.y, m_TA.z-m_LA.z);
//	Length = DL.VAbs();
//	Chord  = DC.VAbs();
//	u.Set(DC.x/Chord,  DC.y/Chord,  DC.z/Chord);
//	v.Set(DL.x/Length, DL.y/Length, DL.z/Length);

	m_bIsTipLeft   = false;
	m_bIsTipRight  = false;
	m_bIsLeftSurf  = false;
	m_bIsRightSurf = false;

/*	Vector3d LATB, TALB;

	LATB = m_TB - m_LA;
	TALB = m_LB - m_TA;
	Normal = LATB * TALB;
    Normal.normalize();*/
}


void Surface::setCornerPoints(Vector3d PLA, Vector3d PTA, Vector3d PLB, Vector3d PTB)
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
bool Surface::isFlapPanel(int p)
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
bool Surface::isFlapPanel(Panel *pPanel)
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
bool Surface::isFlapNode(int nNode)
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

	int k,l,p;
	double alpha0;
	Quaternion Quat;
	Vector3d R, S;
	
	p = 0;

	if(m_pFoilA && m_pFoilB)
	{
		//get the approximate initial angle
		if(qAbs(m_pFoilA->m_TEFlapAngle - m_pFoilB->m_TEFlapAngle)>0.1)
		{
			QString error = QObject::tr("Continuous foils for surface do not have the same initial flap angle... aborting\n");

			return false;
		}
		alpha0 = (m_pFoilA->m_TEFlapAngle + m_pFoilB->m_TEFlapAngle)/2.0;

		Quat.set(Angle-alpha0, m_HingeVector);


		for (k=0; k<m_nFlapNodes; k++)
		{
			R.x = s_pNode[m_FlapNode[k]].x - m_HingePoint.x;
			R.y = s_pNode[m_FlapNode[k]].y - m_HingePoint.y;
			R.z = s_pNode[m_FlapNode[k]].z - m_HingePoint.z;
			Quat.Conjugate(R,S);

			s_pNode[m_FlapNode[k]].x = S.x + m_HingePoint.x;
			s_pNode[m_FlapNode[k]].y = S.y + m_HingePoint.y;
			s_pNode[m_FlapNode[k]].z = S.z + m_HingePoint.z;
		}

		for(l=0; l<m_nFlapPanels; l++)
		{
			k = m_FlapPanel[l];
			if(s_pPanel[k].m_Pos==BOTSURFACE)
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
	else p+= m_NYPanels * m_NXPanels;

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

	Normal.rotateX(XTilt);
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

	Normal.rotateY(YTilt);
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
	Normal.rotateZ(Origin, ZTilt);
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
		getSurfacePoint(m_posATE, m_posBTE, 0.0, MIDSURFACE, m_HingePoint,N);
		getSurfacePoint(m_posATE, m_posBTE, 1.0, MIDSURFACE, HB, N);
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
	Normal = LATB * TALB;
	Normal.normalize();
}


/**
 * Creates the master points on the left and right ends.
 * One of the most difficult part of the code to implement. The algorithm still isn't very robust.
 * @param pBody a pointer to the Body object, or NULL if none.
 * @param dx the x-component of the translation to apply to the body.
 * @param dz the z-component of the translation to apply to the body.
 */
void Surface::setSidePoints(Body * pBody, double dx, double dz)
{
	int l;
	double alpha_dA, alpha_dB, cosdA, cosdB;
	Vector3d N, A4, B4, TA4, TB4;
    Body TBody;
	if(pBody)
	{
		TBody.duplicate(pBody);
		TBody.translate(dx, 0.0, dz);
	}

	Vector3d V = Normal * NormalA;
	Vector3d U = (m_TA - m_LA).normalized();
//	double sindA = -V.dot(Vector3d(1.0,0.0,0.0));
	double sindA = -V.dot(U);
	if(sindA> 1.0) sindA = 1.0;
	if(sindA<-1.0) sindA = -1.0;
	alpha_dA = asin(sindA);
	cosdA = cos(alpha_dA);
	alpha_dA *= 180.0/PI;

	V = Normal * NormalB;
	U = (m_TB-m_LB).normalized();
	double sindB = -V.dot(U);
//	double sindB = V.VAbs();
	if(sindB> 1.0) sindB = 1.0;
	if(sindB<-1.0) sindB = -1.0;
	alpha_dB = asin(sindB);
	cosdB = cos(alpha_dB);
	alpha_dB *= 180.0/PI;

	chordA  = chord(0.0);
	chordB  = chord(1.0);

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

	SideA.clear();
	SideA_T.clear();
	SideA_B.clear();
	SideB.clear();
	SideB_T.clear();
	SideB_B.clear();

	for(int i=0; i<m_NXPanels+1; i++)
	{
		SideA.append(Vector3d(0,0,0));
		SideA_B.append(Vector3d(0,0,0));
		SideA_T.append(Vector3d(0,0,0));
		SideB.append(Vector3d(0,0,0));
		SideB_B.append(Vector3d(0,0,0));
		SideB_T.append(Vector3d(0,0,0));
	}

	for (l=0; l<=m_NXPanels; l++)
	{
		getSidePoint(m_xPointA[l], false, MIDSURFACE, SideA[l], N);
		getSidePoint(m_xPointA[l], false, TOPSURFACE, SideA_T[l], N);
		getSidePoint(m_xPointA[l], false, BOTSURFACE, SideA_B[l], N);

		//scale the thickness
		double Ox = m_xPointA[l];
		double Oy = m_LA.y * (1.0-Ox) +  m_TA.y * Ox;
		double Oz = m_LA.z * (1.0-Ox) +  m_TA.z * Ox;
		SideA[l].y   = Oy +(SideA[l].y   - Oy)/cosdA;
		SideA[l].z   = Oz +(SideA[l].z   - Oz)/cosdA;
		SideA_T[l].y = Oy +(SideA_T[l].y - Oy)/cosdA;
		SideA_T[l].z = Oz +(SideA_T[l].z - Oz)/cosdA;
		SideA_B[l].y = Oy +(SideA_B[l].y - Oy)/cosdA;
		SideA_B[l].z = Oz +(SideA_B[l].z - Oz)/cosdA;

		//rotate the point about the foil's neutral line to account for dihedral
		SideA[l].rotate(m_LA, m_LA-m_TA, alpha_dA);
		SideA_T[l].rotate(m_LA, m_LA-m_TA, alpha_dA);
		SideA_B[l].rotate(m_LA, m_LA-m_TA, alpha_dA);

		//set the twist
//		SideA[l].rotate(A4, TA4, m_TwistA);
//		SideA_T[l].rotate(A4, TA4, m_TwistA);
//		SideA_B[l].rotate(A4, TA4, m_TwistA);
//        NormalA.rotate(TA4, m_TwistA);

		getSidePoint(m_xPointB[l], true, MIDSURFACE, SideB[l], N);
		getSidePoint(m_xPointB[l], true, TOPSURFACE, SideB_T[l], N);
		getSidePoint(m_xPointB[l], true, BOTSURFACE, SideB_B[l], N);

		//scale the thickness
		Ox = m_xPointB[l];
		Oy = m_LB.y * (1.0-Ox) +  m_TB.y * Ox;
		Oz = m_LB.z * (1.0-Ox) +  m_TB.z * Ox;
		SideB[l].y   = Oy +(SideB[l].y   - Oy)/cosdB;
		SideB[l].z   = Oz +(SideB[l].z   - Oz)/cosdB;
		SideB_T[l].y = Oy +(SideB_T[l].y - Oy)/cosdB;
		SideB_T[l].z = Oz +(SideB_T[l].z - Oz)/cosdB;
		SideB_B[l].y = Oy +(SideB_B[l].y - Oy)/cosdB;
		SideB_B[l].z = Oz +(SideB_B[l].z - Oz)/cosdB;

		//rotate the point about the foil's neutral line to account for dihedral
		SideB[l].rotate(m_LB, m_LB-m_TB, alpha_dB);
		SideB_T[l].rotate(m_LB, m_LB-m_TB, alpha_dB);
		SideB_B[l].rotate(m_LB, m_LB-m_TB, alpha_dB);


		//set the twist
//		SideB[l].rotate(B4, TB4, m_TwistB);
//		SideB_T[l].rotate(B4, TB4, m_TwistB);
//		SideB_B[l].rotate(B4, TB4, m_TwistB);
//        NormalB.rotate(TB4, m_TwistB);


		if(pBody && m_bIsCenterSurf && m_bIsLeftSurf)
		{
			if(TBody.intersect(SideA_B[l], SideB_B[l], SideB_B[l], false)) m_bJoinRight = false;
			if(TBody.intersect(SideA_T[l], SideB_T[l], SideB_T[l], false)) m_bJoinRight = false;
			if(TBody.intersect(SideA[l],   SideB[l],   SideB[l],   false)) m_bJoinRight = false;
		}
		else if(pBody && m_bIsCenterSurf && m_bIsRightSurf)
		{
			TBody.intersect(SideA_B[l], SideB_B[l], SideA_B[l], true);
			TBody.intersect(SideA_T[l], SideB_T[l], SideA_T[l], true);
			TBody.intersect(SideA[l],   SideB[l],     SideA[l], true);
		}
	}

	//merge trailing edge nodes in case the foil has a T.E. gap

	Vector3d Node;

	Node = (SideA_B[0] + SideA_T[0])/2.0;
	SideA_B[0].set(Node);
	SideA_T[0].set(Node);

	Node = (SideB_B[0] + SideB_T[0])/2.0;
	SideB_B[0].set(Node);
	SideB_T[0].set(Node);
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
	int l;
	int NXFlapA, NXFlapB, NXLeadA, NXLeadB;
	double dl, dl2;
	double xHingeA, xHingeB;
	if(m_pFoilA && m_pFoilA->m_bTEFlap) xHingeA=m_pFoilA->m_TEXHinge/100.0; else xHingeA=1.0;
	if(m_pFoilB && m_pFoilB->m_bTEFlap) xHingeB=m_pFoilB->m_TEXHinge/100.0; else xHingeB=1.0;

	NXFlapA = (int)((1.0-xHingeA) * (double)m_NXPanels*1.000123);// to avoid numerical errors if exact division
	NXFlapB = (int)((1.0-xHingeB) * (double)m_NXPanels *1.000123);

	if(m_pFoilA && m_pFoilA->m_bTEFlap && NXFlapA==0) NXFlapA++;
	if(m_pFoilB && m_pFoilB->m_bTEFlap && NXFlapB==0) NXFlapB++;

	NXLeadA = m_NXPanels - NXFlapA;
	NXLeadB = m_NXPanels - NXFlapB;

	m_NXFlap  = qMax(NXFlapA, NXFlapB);
	if(m_NXFlap>m_NXPanels/2) m_NXFlap=(int)m_NXPanels/2;
	m_NXLead  = m_NXPanels - m_NXFlap;

	for(l=0; l<NXFlapA; l++)
	{
		dl =  (double)l;
		dl2 = (double)NXFlapA;
		if(m_XDistType==XFLR5::COSINE)
			m_xPointA[l] = 1.0 - (1.0-xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
		else
			m_xPointA[l] = 1.0 - (1.0-xHingeA) * (dl/dl2);
	}

	for(l=0; l<NXLeadA; l++)
	{
		dl =  (double)l;
		dl2 = (double)NXLeadA;
		if(m_XDistType==XFLR5::COSINE)
			m_xPointA[l+NXFlapA] = xHingeA - (xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
		else
			m_xPointA[l+NXFlapA] = xHingeA - (xHingeA) * (dl/dl2);
	}

	for(l=0; l<NXFlapB; l++)
	{
		dl =  (double)l;
		dl2 = (double)NXFlapB;
		if(m_XDistType==XFLR5::COSINE)
			m_xPointB[l] = 1.0 - (1.0-xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
		else
			m_xPointB[l] = 1.0 - (1.0-xHingeB) * (dl/dl2);
	}

	for(l=0; l<NXLeadB; l++)
	{
		dl =  (double)l;
		dl2 = (double)NXLeadB;
		if(m_XDistType==XFLR5::COSINE)
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



double Surface::spanLength()
{
	return sqrt((m_LB.y - m_LA.y)*(m_LB.y - m_LA.y) + (m_LB.z - m_LA.z)*(m_LB.z - m_LA.z));
}


void Surface::setPanelPointers(Panel *pPanel, Vector3d *pNode)
{
	s_pPanel = pPanel;
	s_pNode = pNode;

}















