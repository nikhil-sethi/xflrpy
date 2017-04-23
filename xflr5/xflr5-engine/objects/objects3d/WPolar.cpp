/****************************************************************************

    WPolar Class
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


#include "WPolar.h"
#include <objects3d/Surface.h>
#include <objects3d/Plane.h>
#include <math.h>
#include "objects_global.h"
#include <QtDebug>

/**
 * The public constructor
 */
WPolar::WPolar()
{
	m_bIsVisible  = true;
	m_PointStyle = 0;
	m_Style  = 0;
	m_Width  = 1;
	m_Color.setHsv((int)(((double)qrand()/(double)RAND_MAX)*360),
				  (int)(((double)qrand()/(double)RAND_MAX)*155)+100,
				  (int)(((double)qrand()/(double)RAND_MAX)*155)+100,
				   255);;

	m_bVLM1         = true;
	m_bThinSurfaces = true;
	m_bWakeRollUp   = false;
	m_bTiltedGeom   = false;
	m_bViscous      = true;
	m_bGround       = false;
//	m_bDirichlet    = true;
	m_BoundaryCondition = XFLR5::DIRICHLET;
	m_bIgnoreBodyPanels = false;
	m_bRelaxWake = false;

	m_NXWakePanels    = 1;
	m_TotalWakeLength = 100.0;
	m_WakePanelFactor =   1.1;

	m_AnalysisMethod = XFLR5::LLTMETHOD;
	m_WPolarType     = XFLR5::FIXEDSPEEDPOLAR;
	m_ReferenceDim   = XFLR5::PROJECTEDREFDIM;

	m_BankAngle = 0.0;
	m_AlphaSpec = 0.0;
	m_BetaSpec  = 0.0;
	m_QInfSpec  = 10.0;
	m_Mass      = 1.0;
	m_referenceArea     = 0.0;
	m_referenceChordLength  = 0.0;
	m_referenceSpanLength     = 0.0;
	m_Height    = 0.0;
	m_Density   = 1.225;
	m_Viscosity = 1.5e-5;//m2/s

	m_nControls = 0;
	m_ControlGain.clear();
	m_ControlGain.resize(MAXCONTROLS);

	m_XNeutralPoint = 0.0;

	memset(m_EigenValue, 0, 2*8*MAXPOLARPOINTS*sizeof(double));
	
	m_bAutoInertia = true;
	m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz =0.0;
	m_CoG.set(0.0,0.0,0.0);

	for(int i=0; i<7; i++) m_inertiaGain[i] = 0.0;
	for(int i=0; i<MAXEXTRADRAG; i++) m_ExtraDragArea[i] = 0.0;
	for(int i=0; i<MAXEXTRADRAG; i++) m_ExtraDragCoef[i] = 0.0;
}


void WPolar::replacePOppDataAt(int pos, PlaneOpp *pPOpp)
{
	if(pos<0 || pos>= dataSize()) return;

	m_Alpha[pos]      =  pPOpp->alpha();
	m_Beta[pos]       =  pPOpp->m_Beta;
	m_QInfinite[pos]  =  pPOpp->m_QInf;
	m_CL[pos]         =  pPOpp->m_CL;
	m_CY[pos]         =  pPOpp->m_CY;
	m_ICd[pos]        =  pPOpp->m_ICD;
	m_PCd[pos]        =  pPOpp->m_VCD;
	m_TCd[pos]        =  pPOpp->m_ICD + pPOpp->m_VCD;
	m_GCm[pos]        =  pPOpp->m_GCm;
	m_VCm[pos]        =  pPOpp->m_VCm;
	m_ICm[pos]        =  pPOpp->m_ICm;
	m_GRm[pos]        =  pPOpp->m_GRm;
	m_GYm[pos]        =  pPOpp->m_GYm;
	m_VYm[pos]        =  pPOpp->m_VYm;
	m_IYm[pos]        =  pPOpp->m_IYm;

	m_XCP[pos]        =  pPOpp->m_CP.x;
	m_YCP[pos]        =  pPOpp->m_CP.x;
	m_ZCP[pos]        =  pPOpp->m_CP.z;
	m_MaxBending[pos] =  pPOpp->m_pPlaneWOpp[0]->m_MaxBending;
	m_Ctrl[pos]       =  pPOpp->m_Ctrl;
	m_XNP[pos]        =  pPOpp->m_XNP;

	//store the eigenthings
	for(int l=0; l<8; l++) m_EigenValue[l][pos] = pPOpp->m_EigenValue[l];

	calculatePoint(pos);
}


void WPolar::insertPOppDataAt(int pos, PlaneOpp *pPOpp)
{
	if(pos<0 || pos> dataSize()) return; // if(pos==size), then the data is appended
	
	m_Alpha.insert(pos, pPOpp->alpha());
	m_Beta.insert(pos, pPOpp->m_Beta);
	m_QInfinite.insert(pos, pPOpp->m_QInf);
	m_CL.insert( pos,  pPOpp->m_CL);
	m_CY.insert( pos,  pPOpp->m_CY);
	m_ICd.insert(pos,  pPOpp->m_ICD);
	m_PCd.insert(pos,  pPOpp->m_VCD);
	m_TCd.insert(pos,  pPOpp->m_ICD + pPOpp->m_VCD);

	m_GCm.insert(pos,  pPOpp->m_GCm);
	m_VCm.insert(pos,  pPOpp->m_VCm);
	m_ICm.insert(pos,  pPOpp->m_ICm);
	m_GRm.insert(pos,  pPOpp->m_GRm);
	m_GYm.insert(pos,  pPOpp->m_GYm);
	m_VYm.insert(pos,  pPOpp->m_VYm);
	m_IYm.insert(pos,  pPOpp->m_IYm);

	m_XCP.insert(pos,  pPOpp->m_CP.x);
	m_YCP.insert(pos,  pPOpp->m_CP.y);
	m_ZCP.insert(pos,  pPOpp->m_CP.z);
	if(pPOpp->m_pPlaneWOpp[0])	m_MaxBending.insert(pos, pPOpp->m_pPlaneWOpp[0]->m_MaxBending);
	else                        m_MaxBending.insert(pos, 0.0);
	m_Ctrl.insert(pos, pPOpp->m_Ctrl);
	m_XNP.insert(pos,  pPOpp->m_XNP);

	m_ShortPeriodDamping.insert(pos,0.0);
	m_ShortPeriodFrequency.insert(pos,0.0);
	m_PhugoidDamping.insert(pos,0.0);
	m_PhugoidFrequency.insert(pos,0.0);
	m_DutchRollDamping.insert(pos,0.0);
	m_DutchRollFrequency.insert(pos,0.0);
	m_RollDampingT2.insert(pos,0.0);
	m_SpiralDampingT2.insert(pos,0.0);

	//make room for computed values
	m_1Cl.insert(pos,0.0);
	m_ClCd.insert(pos,0.0);
	m_Cl32Cd.insert(pos,0.0);
	m_ExtraDrag.insert(pos,0.0);
	m_Vx.insert(pos,0.0);
	m_Vz.insert(pos,0.0);
	m_FZ.insert(pos,0.0);
	m_FY.insert(pos,0.0);
	m_FX.insert(pos,0.0);
	m_Gamma.insert(pos,0.0);
	m_Rm.insert(pos, 0.0);
	m_Pm.insert(pos, 0.0);
	m_Ym.insert(pos, 0.0);
	m_VertPower.insert(pos, 0.0);
	m_HorizontalPower.insert(pos, 0.0);
	m_Oswald.insert(pos, 0.0);
	m_XCpCl.insert(pos, 0.0);
	m_SM.insert(pos, 0.0);
	m_Mass_var.insert(pos, 0.0);
	m_CoG_x.insert(pos, 0.0);
	m_CoG_z.insert(pos, 0.0);

	for(int l=0; l<8; l++)
		for(int j=dataSize(); j>pos; j--)
		{
			m_EigenValue[l][j] = m_EigenValue[l][j-1];
		}

	//store the eigenthings
	for(int l=0; l<8; l++)	m_EigenValue[l][pos] = pPOpp->m_EigenValue[l];

	calculatePoint(pos);
}



void WPolar::insertDataAt(int pos, double Alpha, double Beta, double QInf, double Ctrl, double Cl, double CY, double ICd, double PCd, double GCm,
							double ICm, double VCm, double GRm, double GYm, double IYm, double VYm, double XCP, double YCP,
							double ZCP, double Cb, double XNP)
{
	if(pos<0 || pos>dataSize()) return;
	
	m_Alpha.insert(pos, Alpha);
	m_Beta.insert(pos, Beta);
	m_CL.insert(pos, Cl);
	m_CY.insert(pos, CY);
	m_ICd.insert(pos, ICd);
	m_PCd.insert(pos, PCd);

	m_GCm.insert(pos, GCm);
	m_VCm.insert(pos, VCm);
	m_ICm.insert(pos, ICm);
	m_GRm.insert(pos, GRm);
	m_GYm.insert(pos, GYm);
	m_VYm.insert(pos, VYm);
	m_IYm.insert(pos, IYm);

	m_QInfinite.insert(pos, QInf);

	m_XCP.insert(pos, XCP);
	m_YCP.insert(pos, YCP);
	m_ZCP.insert(pos, ZCP);
	m_MaxBending.insert(pos, Cb);
	m_Ctrl.insert(pos, Ctrl);
	if(isStabilityPolar()) m_XNP.insert(pos, XNP);
	else                   m_XNP.insert(pos, 0.0);

	m_TCd.insert(pos, 0.0);
	m_PhugoidDamping.insert(pos, 0.0);
	m_PhugoidFrequency.insert(pos, 0.0);
	m_ShortPeriodDamping.insert(pos, 0.0);
	m_ShortPeriodFrequency.insert(pos, 0.0);
	m_DutchRollDamping.insert(pos, 0.0);
	m_DutchRollFrequency.insert(pos, 0.0);
	m_RollDampingT2.insert(pos, 0.0);
	m_SpiralDampingT2.insert(pos, 0.0);

	m_1Cl.insert(pos, 0.0);
	m_ClCd.insert(pos, 0.0);
	m_Cl32Cd.insert(pos, 0.0);
	m_ExtraDrag.insert(pos,0.0);
	m_Vx.insert(pos, 0.0);
	m_Vz.insert(pos, 0.0);
	m_FZ.insert(pos, 0.0);
	m_FY.insert(pos, 0.0);
	m_FX.insert(pos, 0.0);
	m_Gamma.insert(pos, 0.0);
	m_Rm.insert(pos, 0.0);
	m_Pm.insert(pos, 0.0);
	m_Ym.insert(pos, 0.0);
	m_VertPower.insert(pos, 0.0);
	m_HorizontalPower.insert(pos, 0.0);
	m_Oswald.insert(pos, 0.0);
	m_XCpCl.insert(pos, 0.0);
	m_SM.insert(pos, 0.0);
	m_Mass_var.insert(pos, 0.0);
	m_CoG_x.insert(pos, 0.0);
	m_CoG_z.insert(pos, 0.0);

}



/**
 * Adds the data from the instance of the operating point referenced by pPOpp to the polar object.
 * The index used to insert the data is the aoa, or the velocity, or the control parameter, depending on the polar type.
 * If a point with identical index exists, the data is replaced.
 * If not, the data is inserted for this index.
 *
 * @param pPOpp the plane operating point from which the data is to be extracted
 */
void WPolar::addPlaneOpPoint(PlaneOpp *pPOpp)
{
	bool bInserted = false;
	int i;
	int size = dataSize();

	if(size)
	{
		for (i=0; i<size; i++)
		{
			if(m_WPolarType<XFLR5::FIXEDAOAPOLAR)
			{
				if (qAbs(pPOpp->alpha()-m_Alpha[i]) < 0.001)
				{
					replacePOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
				else if (pPOpp->alpha() < m_Alpha[i])
				{
					insertPOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
			}
			else if(m_WPolarType==XFLR5::FIXEDAOAPOLAR)
			{
				// type 4, sort by speed
				if (qAbs(pPOpp->m_QInf - m_QInfinite[i]) < 0.001)
				{
					// then erase former result
					replacePOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
				else if (pPOpp->m_QInf < m_QInfinite[i])
				{
					// sort by crescending speed
					insertPOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
			}
			else if(m_WPolarType==XFLR5::BETAPOLAR)
			{
				// type 5, sort by sideslip angle
				if (qAbs(pPOpp->m_Beta - m_Beta[i]) < 0.001)
				{
					// then erase former result
					replacePOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
				else if (pPOpp->m_Beta < m_Beta[i])
				{
					// sort by crescending speed
					insertPOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
			}
			else if(m_WPolarType==XFLR5::STABILITYPOLAR)
			{
				// Control or stability analysis, sort by control value
				if (qAbs(pPOpp->m_Ctrl - m_Ctrl[i])<0.001)
				{
					// then erase former result
					replacePOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
				else if (pPOpp->m_Ctrl < m_Ctrl[i])
				{
					// sort by crescending control values
					insertPOppDataAt(i, pPOpp);
					bInserted = true;
					break;
				}
			}
		}
	}

	if(!bInserted)
	{
		// data is appended at the end
		int size = dataSize();
		insertPOppDataAt(size, pPOpp);
	}
}



/**
 * Calculates aerodynamic values for the i-th point in the array : glide ratio, power factor, forces and moments, power
 * for horizontal flight, efficiency coefficient, mode frequencies and amping factors.
 * @param i the index of the point for which the values are to be calculated
 */
void WPolar::calculatePoint(int iPt)
{
	//finish calculations
//	double cl = m_CL[i];
//	double tcd = m_TCd[i];
//	double Qinf =  m_QInfinite[i];
	//dynamic pressure
	double q =  0.5 * m_Density * m_QInfinite[iPt]*m_QInfinite[iPt];

	double mass = m_Mass;
	if(qAbs(m_inertiaGain[0])>PRECISION)
	{
		mass += m_Ctrl[iPt]*m_inertiaGain[0];
	}

	if(iPt>=m_CL.count()) return;

	m_FZ[iPt]  = q * m_CL[iPt]*m_referenceArea;
	m_FY[iPt]  = q * m_CY[iPt]*m_referenceArea;
	m_FX[iPt]  = q * (m_ICd[iPt]+m_PCd[iPt])*m_referenceArea;

	for(int iExtra=0; iExtra<MAXEXTRADRAG; iExtra++) m_FX[iPt] += m_ExtraDragArea[iExtra] * m_ExtraDragCoef[iExtra] *q;

	m_ExtraDrag[iPt] = 0.0;
	for(int i=0; i<MAXEXTRADRAG; i++) m_ExtraDrag[iPt] += m_ExtraDragArea[i]*m_ExtraDragCoef[i];
	m_ExtraDrag[iPt] *= q;

	m_TCd[iPt] = m_FX[iPt]/q/m_referenceArea;

	if(m_CL[iPt]>0.0) {
		m_1Cl[iPt]    = (double)(1./sqrt(m_CL[iPt]));
//		m_Cl32Cd[i] = (double)pow(m_CL[i],1.5)/m_TCd[i];
		m_Cl32Cd[iPt] = sqrt(m_CL[iPt]*m_CL[iPt]*m_CL[iPt])/m_TCd[iPt];
	}
	else {
		m_1Cl[iPt]    = -1.0;//will not be plotted
//		m_Cl32Cd[i] =  -(double)pow(-m_CL[i],1.5)/m_TCd[i];
		m_Cl32Cd[iPt] = -sqrt(-m_CL[iPt]*m_CL[iPt]*m_CL[iPt])/m_TCd[iPt];
	}


	if(qAbs(m_CL[iPt])>0.) m_Gamma[iPt]  =  atan(m_TCd[iPt]/m_CL[iPt]) * 180.0/PI;
	else m_Gamma[iPt] = 90.0;

	m_Vz[iPt] = (double)sqrt(2*mass*9.81/m_Density/m_referenceArea)/m_Cl32Cd[iPt];
	m_Vx[iPt] = m_QInfinite[iPt] * (double)cos(m_Gamma[iPt]*PI/180.0);

	m_ClCd[iPt]   =  m_CL[iPt]/m_TCd[iPt];

	m_Rm[iPt] = q * m_referenceArea * m_GRm[iPt] * m_referenceSpanLength;// in N.m
	m_Ym[iPt] = q * m_referenceArea * m_GYm[iPt] * m_referenceSpanLength;// in N.m
	m_Pm[iPt] = q * m_referenceArea * m_GCm[iPt] * m_referenceChordLength;// in N.m

	//power for horizontal flight
	m_VertPower[iPt] = mass * 9.81 * m_Vz[iPt];
	m_HorizontalPower[iPt] = m_FX[iPt] * m_Vx[iPt];

	double AR      = m_referenceSpanLength*m_referenceSpanLength/m_referenceArea;

	if(m_ICd[iPt]==0.0)	m_Oswald[iPt] = 0.0;
	else				m_Oswald[iPt] = m_CL[iPt]*m_CL[iPt]/PI/m_ICd[iPt]/AR;

	m_XCpCl[iPt]     = m_XCP[iPt] * m_CL[iPt];

	if(m_XCpCl.count()>1 && !isStabilityPolar())
	{
		m_XNeutralPoint = (m_XCpCl.last()-m_XCpCl.first()) / (m_CL.last()-m_CL.first());
	}
	else m_XNeutralPoint = 0.0;


	m_SM[iPt]        = (m_XCP[iPt]-m_CoG.x)/m_referenceChordLength *100.00;
	m_Mass_var[iPt]  = m_Mass + m_Ctrl[iPt] * m_inertiaGain[0];
	m_CoG_x[iPt]  = m_CoG.x + m_Ctrl[iPt] * m_inertiaGain[1];
	m_CoG_z[iPt]  = m_CoG.z + m_Ctrl[iPt] * m_inertiaGain[2];

	double OmegaN, Omega1, Dsi;
	if(isStabilityPolar())
	{
		modeProperties(m_EigenValue[2][iPt], Omega1, OmegaN, Dsi);
		m_PhugoidDamping[iPt]   = Dsi;
		m_PhugoidFrequency[iPt] = Omega1/2.0/PI;

		modeProperties(m_EigenValue[0][iPt], Omega1, OmegaN, Dsi);
		m_ShortPeriodFrequency[iPt] = Omega1/2.0/PI;
		m_ShortPeriodDamping[iPt]   = Dsi;

		modeProperties(m_EigenValue[5][iPt], Omega1, OmegaN, Dsi);
		m_DutchRollFrequency[iPt] = Omega1/2.0/PI;
		m_DutchRollDamping[iPt]   = Dsi;

		m_RollDampingT2[iPt]    = log(2.0)/fabs(m_EigenValue[4][iPt].real());
		m_SpiralDampingT2[iPt]  = log(2.0)/fabs(m_EigenValue[7][iPt].real());
	}
	else
	{
		m_PhugoidDamping[iPt] = m_PhugoidFrequency[iPt] = 0.0;
		m_ShortPeriodFrequency[iPt] = m_ShortPeriodDamping[iPt] = 0.0;
		m_DutchRollFrequency[iPt] = m_DutchRollDamping[iPt] = 0.0;
		m_RollDampingT2[iPt] = m_SpiralDampingT2[iPt] = 0.0;
	}
//qDebug()<<	m_EigenValue[4][iPt].real()<<m_EigenValue[7][iPt].real();
}


/*	N =  Cn.q.s.b
	L =  Ct.q.s.b
	M =  Cm.q.s.c'
*/

/**
 * Copies the polar's analysis parameters from an existing polar
 * @param pWPolar a pointer to the instance of the reference CWPolar object from which the parameters should be copied
 */
void WPolar::duplicateSpec(WPolar *pWPolar)
{
	m_PlaneName   = pWPolar->m_PlaneName;
	m_WPlrName    = pWPolar->m_WPlrName;

	m_WPolarType  = pWPolar->m_WPolarType;

	m_QInfSpec      = pWPolar->m_QInfSpec;
	m_AlphaSpec     = pWPolar->m_AlphaSpec;

	if(pWPolar->polarType()==XFLR5::BETAPOLAR) m_BetaSpec = 0.0;
	else                                m_BetaSpec = pWPolar->m_BetaSpec;

	m_Style  = pWPolar->curveStyle();
	m_Width  = pWPolar->curveWidth();
	m_Color  = pWPolar->curveColor();
	m_PointStyle = pWPolar->points();
	m_bIsVisible = pWPolar->isVisible();

	// general aerodynamic data - specific to a polar
	m_Viscosity   = pWPolar->m_Viscosity;
	m_Density     = pWPolar->density() ;
	m_Height      = pWPolar->m_Height;//for ground effect
	m_BankAngle   = pWPolar->m_BankAngle;

	m_NXWakePanels      = pWPolar->m_NXWakePanels;
	m_TotalWakeLength   = pWPolar->m_TotalWakeLength;
	m_WakePanelFactor   = pWPolar->m_WakePanelFactor;

	m_BoundaryCondition = pWPolar->m_BoundaryCondition;

	m_bGround         = pWPolar->m_bGround;
	m_bTiltedGeom     = pWPolar->m_bTiltedGeom;
	m_bViscous        = pWPolar->m_bViscous;
	m_bIgnoreBodyPanels = pWPolar->m_bIgnoreBodyPanels;
	m_bVLM1           = pWPolar->m_bVLM1;
	m_bWakeRollUp     = pWPolar->m_bWakeRollUp;
	m_AnalysisMethod  = pWPolar->analysisMethod();
	m_bThinSurfaces   = pWPolar->bThinSurfaces();
	m_bVLM1           = pWPolar->m_bVLM1;

	m_nControls       = pWPolar->m_nControls;
	for(int icg=0; icg<MAXCONTROLS; icg++)
		m_ControlGain[icg] = pWPolar->m_ControlGain.at(icg);

	m_ReferenceDim = pWPolar->m_ReferenceDim;
	m_referenceArea        = pWPolar->m_referenceArea;//for lift and drag calculations
	m_referenceChordLength = pWPolar->m_referenceChordLength;// for moment calculations
	m_referenceSpanLength  = pWPolar->m_referenceSpanLength;//for moment calculations

	//Inertia properties
	m_Mass = pWPolar->m_Mass;
	m_bAutoInertia = pWPolar->m_bAutoInertia;
	m_CoGIxx = pWPolar->m_CoGIxx;
	m_CoGIyy = pWPolar->m_CoGIyy;
	m_CoGIzz = pWPolar->m_CoGIzz;
	m_CoGIxz = pWPolar->m_CoGIxz;

	m_CoG = pWPolar->m_CoG;

	for(int i=0; i<7; i++) m_inertiaGain[i] = pWPolar->m_inertiaGain[i];
	for (int i=0; i<MAXEXTRADRAG; i++) m_ExtraDragArea[i] = pWPolar->m_ExtraDragArea[i];
	for (int i=0; i<MAXEXTRADRAG; i++) m_ExtraDragCoef[i] = pWPolar->m_ExtraDragCoef[i];

}




/**
 * Returns a pointer to the QList array of data for the variable referenced by iVar
 * @param iVar the index of the variable
 * @return a void pointer to the array of data
 */
void * WPolar::getWPlrVariable(int iVar)
{
	// returns a pointer to the variable array defined by its index iVar
	void * pVar;
	switch (iVar)
	{
		case 0:
			pVar = &m_Alpha;
			break;
		case 1:
			pVar = &m_Beta;
			break;
		case 2:
			pVar = &m_CL;
			break;
		case 3:
			pVar = &m_TCd;
			break;
		case 4:
			pVar = &m_PCd;
			break;
		case 5:
			pVar = &m_ICd;
			break;
		case 6:
			pVar = &m_CY;
			break;
		case 7:
			pVar = &m_GCm;
			break;
		case 8:
			pVar = &m_VCm;
			break;
		case 9:
			pVar = &m_ICm;
			break;
		case 10:
			pVar = &m_GRm;
			break;
		case 11:
			pVar = &m_GYm;
			break;
		case 12:
			pVar = &m_VYm;
			break;
		case 13:
			pVar = &m_IYm;
			break;
		case 14:
			pVar = &m_ClCd;
			break;
		case 15:
			pVar = &m_Cl32Cd;
			break;
		case 16:
			pVar = &m_1Cl;
			break;
		case 17:
			pVar = &m_FX;
			break;
		case 18:
			pVar = &m_FY;
			break;
		case 19:
			pVar = &m_FZ;
			break;
		case 20:
			pVar = &m_Vx;
			break;
		case 21:
			pVar = &m_Vz;
			break;
		case 22:
			pVar = &m_QInfinite;
			break;
		case 23:
			pVar = &m_Gamma;
			break;
		case 24:
			pVar = &m_Rm;
			break;
		case 25:
			pVar = &m_Pm;
			break;
		case 26:
			pVar = &m_Ym;
			break;
		case 27:
			pVar = &m_XCP;
			break;
		case 28:
			pVar = &m_YCP;
			break;
		case 29:
			pVar = &m_ZCP;
			break;
		case 30:
			pVar = &m_MaxBending;
			break;
		case 31:
			pVar = &m_VertPower;
			break;
		case 32:
			pVar = &m_Oswald;
			break;
		case 33:
			pVar = &m_XCpCl;
			break;
		case 34:
			pVar = &m_SM;
			break;
		case 35:
			pVar = &m_Ctrl;
			break;
		case 36:
			pVar = &m_XNP;
			break;
		case 37:
			pVar = &m_PhugoidFrequency;
			break;
		case 38:
			pVar = &m_PhugoidDamping;
			break;
		case 39:
			pVar = &m_ShortPeriodFrequency;
			break;
		case 40:
			pVar = &m_ShortPeriodDamping;
			break;
		case 41:
			pVar = &m_DutchRollFrequency;
			break;
		case 42:
			pVar = &m_DutchRollDamping;
			break;
		case 43:
			pVar = &m_RollDampingT2;
			break;
		case 44:
			pVar = &m_SpiralDampingT2;
			break;
		case 45:
			pVar = &m_HorizontalPower;
			break;
		case 46:
			pVar = &m_ExtraDrag;
			break;
		case 47:
			pVar = &m_Mass_var;
			break;
		case 48:
			pVar = &m_CoG_x;
			break;
		case 49:
			pVar = &m_CoG_z;
			break;
		default:
			pVar = &m_Alpha;
			break;
	}
	return pVar;
}





/**
 * Removes the data for the point with aoa alpha
 * @param alpha the aoa of the point to be deleted
 **/
void WPolar::remove(double alpha)
{
	for(int ia=0;ia<dataSize(); ia++)
	{
        if(qAbs(m_Alpha.at(ia)-alpha)<PRECISION)
		{
			remove(ia);
			break;
		}
	}
}


/**
 * Removes the data at index i of the data arrays
 * @param i the index at which the data is to be deleted
 **/
void WPolar::remove(int i)
{
	int size = dataSize();
	m_Alpha.removeAt(i);
	m_Beta.removeAt(i);
	m_CL.removeAt(i);
	m_CY.removeAt(i);
	m_ICd.removeAt(i);
	m_PCd.removeAt(i);
	m_TCd.removeAt(i);

	m_GCm.removeAt(i);
	m_VCm.removeAt(i);
	m_ICm.removeAt(i);
	m_GRm.removeAt(i);
	m_GYm.removeAt(i);
	m_VYm.removeAt(i);
	m_IYm.removeAt(i);

	m_XCP.removeAt(i);
	m_YCP.removeAt(i);
	m_ZCP.removeAt(i);
	m_MaxBending.removeAt(i);
	m_VertPower.removeAt(i);
	m_HorizontalPower.removeAt(i);

	m_Oswald.removeAt(i);
	m_XCpCl.removeAt(i);
	m_SM.removeAt(i);
	m_Mass_var.removeAt(i);
	m_CoG_x.removeAt(i);
	m_CoG_z.removeAt(i);
	m_Ctrl.removeAt(i);
	m_XNP.removeAt(i);
	m_ShortPeriodDamping.removeAt(i);
	m_ShortPeriodFrequency.removeAt(i);
	m_PhugoidFrequency.removeAt(i);
	m_PhugoidDamping.removeAt(i);

	m_DutchRollDamping.removeAt(i);
	m_DutchRollFrequency.removeAt(i);
	m_RollDampingT2.removeAt(i);
	m_SpiralDampingT2.removeAt(i);

	m_ClCd.removeAt(i);
	m_1Cl.removeAt(i);
	m_Cl32Cd.removeAt(i);
	m_ExtraDrag.removeAt(i);

	m_QInfinite.removeAt(i);
	m_Gamma.removeAt(i);
	m_FZ.removeAt(i);
	m_FY.removeAt(i);
	m_FX.removeAt(i);
	m_Vx.removeAt(i);

	m_Vz.removeAt(i);
	m_Pm.removeAt(i);
	m_Ym.removeAt(i);
	m_Rm.removeAt(i);

	for(int j=i; j<size; j++)
	{
		for(int l=0; l<8; l++)
			m_EigenValue[l][j] = m_EigenValue[l][j+1];
	}
}


/**
 *Clears the content of the data arrays
*/
void WPolar::clearData()
{
	int size = dataSize();
	m_Alpha.clear();
	m_Beta.clear();
	m_CL.clear();
	m_CY.clear();
	m_ICd.clear();
	m_PCd.clear();
	m_TCd.clear();

	m_GCm.clear();
	m_VCm.clear();
	m_ICm.clear();
	m_GRm.clear();
	m_GYm.clear();
	m_VYm.clear();
	m_IYm.clear();

	m_XCP.clear();
	m_YCP.clear();
	m_ZCP.clear();
	m_MaxBending.clear();
	m_VertPower.clear();
	m_HorizontalPower.clear();

	m_Oswald.clear();
	m_XCpCl.clear();
	m_SM.clear();
	m_Mass_var.clear();
	m_CoG_x.clear();
	m_CoG_z.clear();
	m_Ctrl.clear();
	m_XNP.clear();
	m_ShortPeriodDamping.clear();
	m_ShortPeriodFrequency.clear();
	m_PhugoidDamping.clear();
	m_PhugoidFrequency.clear();

	m_DutchRollDamping.clear();
	m_DutchRollFrequency.clear();
	m_RollDampingT2.clear();
	m_SpiralDampingT2.clear();

	m_ClCd.clear();
	m_1Cl.clear();
	m_Cl32Cd.clear();
	m_ExtraDrag.clear();

	m_QInfinite.clear();
	m_Gamma.clear();
	m_FZ.clear();
	m_FY.clear();
	m_FX.clear();
	m_Vx.clear();

	m_Vz.clear();
	m_Pm.clear();
	m_Ym.clear();
	m_Rm.clear();

	for(int l=0; l<8; l++)
		for(int j=0; j<size; j++)
			m_EigenValue[l][j] = 0.0;
}




/**
 * Maps the inertia data from the parameter object to the polar's variables
 * @param ptr a void pointer to the reference wing or plane instance
 * @param bPlane true if the reference object is a plane, false if it is a wing
 */
void WPolar::retrieveInertia(void *ptr)
{
	Plane *pPlane = (Plane*)ptr;
	m_Mass = pPlane->totalMass();
	m_CoG = pPlane->CoG();
	m_CoGIxx = pPlane->m_CoGIxx;
	m_CoGIyy = pPlane->m_CoGIyy;
	m_CoGIzz = pPlane->m_CoGIzz;
	m_CoGIxz = pPlane->m_CoGIxz;

	clearData();
}


void WPolar::copy(WPolar *pWPolar)
{
	int i;
	m_bIsVisible = pWPolar->isVisible();
	m_PointStyle = pWPolar->points();
	m_bTiltedGeom     = pWPolar->m_bTiltedGeom;
	m_bViscous        = pWPolar->m_bViscous;
	m_bVLM1           = pWPolar->m_bVLM1;
	m_bWakeRollUp     = pWPolar->m_bWakeRollUp;
	m_AnalysisMethod  = pWPolar->analysisMethod();
	m_WPolarType      = pWPolar->m_WPolarType;
	m_bThinSurfaces   = pWPolar->bThinSurfaces();
	m_nControls       = pWPolar->m_nControls;

    m_Density         = pWPolar->m_Density;
    m_Viscosity       = pWPolar->m_Viscosity;

	clearData();

	for(i=0; i<pWPolar->dataSize(); i++)
	{
		m_Alpha.append(     pWPolar->m_Alpha[i]);
		m_Beta.append(      pWPolar->m_Beta[i]);
		m_CL.append(        pWPolar-> m_CL[i]);
		m_CY.append(        pWPolar-> m_CY[i]);
		m_ICd.append(       pWPolar-> m_ICd[i]);
		m_PCd.append(       pWPolar-> m_PCd[i]);
		m_TCd.append(       pWPolar-> m_TCd[i]);

		m_GCm.append(       pWPolar-> m_GCm[i]);
		m_VCm.append(       pWPolar-> m_VCm[i]);
		m_ICm.append(       pWPolar-> m_ICm[i]);
		m_GRm.append(       pWPolar-> m_GRm[i]);
		m_GYm.append(       pWPolar-> m_GYm[i]);
		m_VYm.append(       pWPolar-> m_VYm[i]);
		m_IYm.append(       pWPolar-> m_IYm[i]);

		m_QInfinite.append( pWPolar->m_QInfinite[i]);
		m_XCP.append(       pWPolar-> m_XCP[i]);
		m_YCP.append(       pWPolar-> m_YCP[i]);
		m_ZCP.append(       pWPolar-> m_YCP[i]);

		m_MaxBending.append(pWPolar-> m_MaxBending[i]);
		m_Ctrl.append(      pWPolar-> m_Ctrl[i]);
		m_XNP.append(       pWPolar-> m_XNP[i]);

		m_PhugoidDamping.append(      pWPolar->m_PhugoidDamping[i]);
		m_PhugoidFrequency.append(    pWPolar->m_PhugoidFrequency[i]);
		m_ShortPeriodDamping.append(  pWPolar->m_ShortPeriodDamping[i]);
		m_ShortPeriodFrequency.append(pWPolar->m_ShortPeriodFrequency[i]);
		m_DutchRollDamping.append(    pWPolar->m_DutchRollDamping[i]);
		m_DutchRollFrequency.append(  pWPolar->m_DutchRollFrequency[i]);
		m_RollDampingT2.append(         pWPolar->m_RollDampingT2[i]);
		m_SpiralDampingT2.append(       pWPolar->m_SpiralDampingT2[i]);

		m_ClCd.append(      pWPolar-> m_ClCd[i]);
		m_1Cl.append(       pWPolar-> m_1Cl[i]);
		m_Cl32Cd.append(    pWPolar-> m_Cl32Cd[i]);
		m_ExtraDrag.append( pWPolar-> m_ExtraDrag[i]);

		m_Vx.append(        pWPolar-> m_Vx[i]);
		m_Vz.append(        pWPolar-> m_Vz[i]);

		m_FX.append(        pWPolar-> m_FX[i]);
		m_FY.append(        pWPolar-> m_FY[i]);
		m_FZ.append(        pWPolar-> m_FY[i]);

		m_Gamma.append(     pWPolar-> m_Gamma[i]);
		m_Pm.append(        pWPolar-> m_Pm[i]);
		m_Ym.append(        pWPolar-> m_Ym[i]);
		m_Rm.append(        pWPolar-> m_Rm[i]);
		m_VertPower.append( pWPolar-> m_VertPower[i]);
		m_HorizontalPower.append( pWPolar-> m_HorizontalPower[i]);

		m_Oswald.append(    pWPolar-> m_Oswald[i]);
		m_XCpCl.append(     pWPolar->m_XCpCl);
		m_SM.append(        pWPolar-> m_SM[i]);
		m_Mass_var.append(  pWPolar-> m_Mass_var[i]);
		m_CoG_x.append(     pWPolar-> m_CoG_x[i]);
		m_CoG_z.append(     pWPolar-> m_CoG_z[i]);

		for(int l=0; l<8; l++)
			m_EigenValue[l][i] = pWPolar->m_EigenValue[l][i];
	}
}




/**
 * Loads or saves the data of this polar to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WPolar::serializeWPlrWPA(QDataStream &ar, bool bIsStoring)
{
	int n;
	float f,r0,r1,r2,r3,i0,i1,i2,i3;
	int i, j, k;

	m_PolarFormat = 1024;
	// 1024 : added ignore body flag
	// 1023 : added ZCP position
	// 1022 : added XNP position and provision for 50 more variables
	// 1021 : XFLR5 v6.02 - deleted autoinertia for older format polars
	// 1020 : QFLR6 v0.00 - added inertia tensor values
	// 1019 : QFLR6 v0.00 - added eigenvalues
	// 1018 : QFLR5 v0.04 - replaced m_XcmRef by m_Cog
	// 1017 : QFLR5 v0.03 - added viscous and induced pitching moments
	// 1016 : added lateral force coefficient
	// 1015 : added lateral force coefficient
	// 1014 : added control results
	// 1013 : added control variables
	// 1012 : redefined the moment coefficients
	// 1011 : added wake roll-up parameters
	// 1010 : added ground effect variables langth changed length unit to m
	// 1009 : added viscous flag
	// 1008 : added Tilted Geometry flag
	// 1007 : added NXWakePanels
	// 1006 : added Wake Roll Up flag
	// 1005 : added VLM method types
	// 1004 : with corrected PCd calculation
	// 1003 : added bending moment V18
	// 1002 : added XCmRef
	// 1001 : v0.00

	if(bIsStoring)
	{
		// not storing to .wpa format anymore
		return true;
	}
	else
	{
        int r,g,b;
		//read variables
		ar >> m_PolarFormat;
		if (m_PolarFormat <=1000 || m_PolarFormat>1100)
		{
			m_WPlrName ="";
			return false;
		}
		readCString(ar, m_PlaneName);
		readCString(ar, m_WPlrName);

		ar>> f;
		m_referenceArea = f;
		if (m_referenceArea<0) return false;

		ar>> f;
		m_referenceChordLength = f;
		if (m_referenceChordLength<0) return false;

		ar>> f;
		m_referenceSpanLength = f;
		if (m_referenceSpanLength<0) return false;

		ar >> m_Style  >> m_Width;
		if (m_Style<0 || m_Style> 10) return false;

		if (m_Width<0 || m_Width> 10) return false;

        readCOLORREF(ar, r,g,b);

		ar>>k;
		if(k==1)      m_AnalysisMethod=XFLR5::LLTMETHOD;
		else if(k==2) m_AnalysisMethod=XFLR5::VLMMETHOD;
		else if(k==3) m_AnalysisMethod=XFLR5::PANELMETHOD;
		else if(k==4) m_AnalysisMethod=XFLR5::VLMMETHOD;

		if(m_AnalysisMethod==XFLR5::VLMMETHOD)
		{
			m_AnalysisMethod=XFLR5::PANELMETHOD;
			m_bThinSurfaces = true;
		}

		if(m_PolarFormat>=1005)
		{
			ar >> n;
			if (n!=0 && n!=1) return false;

			if(n) m_bVLM1 =true; else m_bVLM1 = false;
			ar >> n;
			if (n!=0 && n!=1) return false;

			if(n) m_bThinSurfaces =true; else m_bThinSurfaces = false;
		}
		if(m_PolarFormat>=1008)
		{
			ar >> n;
			if (n!=0 && n!=1) return false;
			if(n) m_bTiltedGeom =true; else m_bTiltedGeom = false;
		}

		if(m_PolarFormat>=1006)
		{
			ar >> n;
			if (n!=0 && n!=1) return false;
//			if(n) m_bDirichlet = false; else m_bDirichlet = true;
			m_BoundaryCondition = n ? XFLR5::DIRICHLET : XFLR5::NEUMANN;
		}
		if(m_PolarFormat>=1009)
		{
			ar >> n;
			if (n!=0 && n!=1) return false;
			if(n) m_bViscous =true; else m_bViscous = false;
		}
/*		if(m_PolarFormat>=1024)
		{
			ar >> n;
			if (n!=0 && n!=1) return false;
			if(n) m_bIgnoreBody =true; else m_bIgnoreBody = false;
		}*/

		if(m_PolarFormat>=1010)
		{
			ar >> n;
			if (n!=0 && n!=1) return false;

			if(n) m_bGround =true; else m_bGround = false;
			ar >> f; m_Height = f;
		}

		if(m_PolarFormat>=1007)
		{
			ar >> m_NXWakePanels;
			if (m_NXWakePanels<0 || m_NXWakePanels>1000) return false;
		}
		if(m_PolarFormat>=1011)
		{
			ar >> f; 			m_TotalWakeLength = f;
			ar >> f; 			m_WakePanelFactor = f;
		}

		ar >> n;
		if (n!=0 && n!=1) return false;
		else {
			if(n) m_bIsVisible =true; else m_bIsVisible = false;
		}
		ar >> n; m_PointStyle =n;

		ar >>k;
		if(k==1)      m_WPolarType = XFLR5::FIXEDSPEEDPOLAR;
		else if(k==2) m_WPolarType = XFLR5::FIXEDLIFTPOLAR;
		else if(k==4) m_WPolarType = XFLR5::FIXEDAOAPOLAR;
		else if(k==6) m_WPolarType = XFLR5::STABILITYPOLAR; // former control polars
		else if(k==7) m_WPolarType = XFLR5::STABILITYPOLAR;
		else return false;


		ar >> f;	m_QInfSpec = f;
		ar >> f;	m_Mass = f;
		ar >> f;	m_AlphaSpec = f;
		if(m_PolarFormat>=1015)
		{
			ar >> f;
			m_BetaSpec = f;
		}
		else m_BetaSpec = 0.0;
		if(m_PolarFormat<1018 && m_PolarFormat>=1002)
		{
			ar >> f;			m_CoG.x = f;
		}
		else if(m_PolarFormat>=1018)
		{
			ar >> f;			m_CoG.x = f;
			ar >> f;			m_CoG.y = f;
			ar >> f;			m_CoG.z = f;
		}
//		if(m_PolarFormat>=1002) ar >> f; m_XCmRef = f;
		ar >> f;	m_Density=f;
		ar >> f;	m_Viscosity=f;

		k=0;
		if(m_PolarFormat>=1016) ar >> k;
		if(k==1)      m_ReferenceDim = XFLR5::PLANFORMREFDIM;
		else if(k==2) m_ReferenceDim = XFLR5::PROJECTEDREFDIM;
		else if(k==3) m_ReferenceDim = XFLR5::MANUALREFDIM;
		else          m_ReferenceDim = XFLR5::PLANFORMREFDIM;

		ar >> n;
		if (n<0 || n> 100000) return false;

		if(m_PolarFormat<1010)
		{
			m_referenceArea    /=100.0;
			m_referenceChordLength /=1000.0;
			m_referenceSpanLength    /=1000.0;
			m_CoG.x   /=1000.0;
		}
		float Alpha, Cl, CY, ICd, PCd, GCm, GRm, GYm, VCm, ICm, VYm, IYm, QInfinite, XCP, YCP, ZCP, Ctrl, Cb, XNP;
		f = Alpha =  Cl = CY = ICd = PCd = GCm = GRm = GYm = VCm = ICm = VYm = IYm = QInfinite = XCP = YCP = ZCP = Ctrl = Cb =0.0;
//		bool bExists;
		for (i=0; i<n; i++)
		{
			ar >> Alpha >> Cl;
			if(m_PolarFormat>=1015) ar>>CY;
			ar >> ICd >> PCd;
			ar >> GCm;
			if(m_PolarFormat>=1017) ar >> VCm >> ICm;
			ar >> GRm >> GYm >> f >> VYm >> IYm;
			if(m_PolarFormat<1012) GCm = GRm = GYm = VCm = VYm = IYm = 0.0;
			ar >> QInfinite >> XCP >> YCP;
			if (m_PolarFormat>=1023) ar >> ZCP;

			if(m_PolarFormat<1010)
			{
				XCP   /=1000.0;
				YCP   /=1000.0;
			}

			if (m_PolarFormat>=1003)
				ar >> Cb;
			else
				Cb = 0.0;

			if (m_PolarFormat>=1014) ar >> Ctrl;
			else                     Ctrl = 0.0;

			if (m_PolarFormat>=1022) ar >> XNP;
			else                     XNP = 0.0;

			if(m_WPolarType!=XFLR5::FIXEDAOAPOLAR)
			{
				for (j=0; j<dataSize(); j++)
				{
					if(qAbs(Alpha-m_Alpha[j])<0.001)
					{
						break;
					}
				}
			}
			else
			{
				for (j=0; j<dataSize(); j++)
				{
					if(qAbs(QInfinite-m_QInfinite[j])<0.001)
					{
						break;
					}
				}
			}

			m_Alpha.append(Alpha);
			m_Beta.append(m_BetaSpec);
			m_CL.append(Cl);
			m_CY.append(CY);
			m_ICd.append(ICd);
			m_PCd.append(PCd);
			m_TCd.append(ICd+PCd);

			m_GCm.append(GCm);
			m_VCm.append(VCm);
			m_ICm.append(ICm);
			m_GRm.append(GRm);
			m_GYm.append(GYm);
			m_VYm.append(VYm);
			m_IYm.append(IYm);

			m_QInfinite.append(QInfinite);

			m_XCP.append(XCP);
			m_YCP.append(YCP);
			m_ZCP.append(ZCP);
			m_MaxBending.append(Cb);
			m_Ctrl.append(Ctrl);
			m_XNP.append(XNP);

			m_PhugoidDamping.append(0.0);
			m_PhugoidFrequency.append(0.0);
			m_ShortPeriodDamping.append(0.0);
			m_ShortPeriodFrequency.append(0.0);
			m_DutchRollDamping.append(0.0);
			m_DutchRollFrequency.append(0.0);
			m_RollDampingT2.append(0.0);
			m_SpiralDampingT2.append(0.0);


			m_1Cl.append(0.0);
			m_ClCd.append(0.0);
			m_Cl32Cd.append(0.0);
			m_ExtraDrag.append(0.0);
			m_Vx.append(0.0);
			m_Vz.append(0.0);
			m_FZ.append(0.0);
			m_FY.append(0.0);
			m_FX.append(0.0);
			m_Gamma.append(0.0);
			m_Rm.append(0.0);
			m_Pm.append(0.0);
			m_Ym.append(0.0);
			m_VertPower.append(0.0);
			m_HorizontalPower.append(0.0);
			m_Oswald.append(0.0);
			m_XCpCl.append(0.0);
			m_SM.append(0.0);
			m_Mass_var.append(0.0);
			m_CoG_x.append(0.0);
			m_CoG_z.append(0.0);

	 /** @todo replace with InsertDataAt(i, ...); */

		}
		if(m_PolarFormat>1012)
		{
			ar >> m_nControls;
			if(abs(m_nControls)>1000) m_nControls = 0;
//			m_ControlGain.clear();
			for(i=0; i<m_nControls; i++)
			{
				ar >> f; //m_MinControl[i] = f;
				ar >> f; m_ControlGain[i] = f;
			}
			for(i=0; i<m_nControls; i++)
			{
				ar >> n;
				if (n!=0 && n!=1) return false;
				else {
//					if(n) m_bActiveControl[i] =true; else m_bActiveControl[i] = false;
				}
			}
		}
		if(m_PolarFormat>=1019)
		{
			n = dataSize();

			for(i=0; i<n; i++)
			{
				ar>>r0>>r1>>r2>>r3;
				ar>>i0>>i1>>i2>>i3;

				m_EigenValue[0][i] = complex<double>(r0,i0);
				m_EigenValue[1][i] = complex<double>(r1,i1);
				m_EigenValue[2][i] = complex<double>(r2,i2);
				m_EigenValue[3][i] = complex<double>(r3,i3);
				ar>>r0>>r1>>r2>>r3;
				ar>>i0>>i1>>i2>>i3;
				m_EigenValue[4][i] = complex<double>(r0,i0);
				m_EigenValue[5][i] = complex<double>(r1,i1);
				m_EigenValue[6][i] = complex<double>(r2,i2);
				m_EigenValue[7][i] = complex<double>(r3,i3);
				calculatePoint(i);
			}
		}
		if(m_PolarFormat>=1020)
		{
			ar >> n;
			if(n && m_PolarFormat>1020) m_bAutoInertia =true; else m_bAutoInertia = false;
			ar>>r0>>r1>>r2>>r3;
			m_CoGIxx = r0;
			m_CoGIyy = r1;
			m_CoGIzz = r2;
			m_CoGIxz = r3;
		}
		else
		{
			m_bAutoInertia = false;
			m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;
		}

		if(m_PolarFormat>=1022)
		{
			//float provision
			for(int i=0; i<20; i++)
			{
				ar>>f;
			}

			//int provision
			ar >> n;
			if (m_PolarFormat >= 1024)
			{
				if (n!=0 && n!=1)
					return false;
				if(n) m_bIgnoreBodyPanels = true; else m_bIgnoreBodyPanels = false;
			} else m_bIgnoreBodyPanels = false;

			for(int i=1; i<20; i++)
			{
				ar>>n;
			}
		}
	}

	return true;
}




/**
 * Loads or saves the data of this polar to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WPolar::serializeWPlrXFL(QDataStream &ar, bool bIsStoring)
{
	bool boolean;
	int i, k, n;
	double dble;
	double r0, r1, r2, r3, r4, r5, r6, r7;
	double i0, i1, i2, i3, i4, i5, i6, i7;

	m_PolarFormat = 200013;
	// 200011 : v0.00

	if(bIsStoring)
	{
		//output the variables to the stream
		ar << m_PolarFormat;

		ar << m_PlaneName;
		ar << m_WPlrName;

		ar << m_referenceArea << m_referenceChordLength << m_referenceSpanLength ;
		ar << m_Style << m_Width;
		ar << m_Color;
		ar << m_bIsVisible << false;

		if(m_AnalysisMethod==XFLR5::LLTMETHOD)        ar<<1;
		else if(m_AnalysisMethod==XFLR5::VLMMETHOD)   ar<<2;
		else if(m_AnalysisMethod==XFLR5::PANELMETHOD) ar<<3;
		else                                   ar<<0;

		if(m_WPolarType==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
		else if(m_WPolarType==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
		else if(m_WPolarType==XFLR5::FIXEDAOAPOLAR)   ar<<4;
		else if(m_WPolarType==XFLR5::BETAPOLAR)       ar<<5;
		else if(m_WPolarType==XFLR5::STABILITYPOLAR)  ar<<7;
		else ar << 0;

		ar << m_bVLM1;
		ar << m_bThinSurfaces;
		ar << m_bTiltedGeom;
		ar << (m_BoundaryCondition==XFLR5::DIRICHLET);
		ar << m_bViscous;
		ar << m_bIgnoreBodyPanels;

		ar << m_bGround;
		ar << m_Height;

		ar << m_Density << m_Viscosity;

		if(m_ReferenceDim == XFLR5::PLANFORMREFDIM)       ar << 1;
		else if(m_ReferenceDim == XFLR5::PROJECTEDREFDIM) ar << 2;
		else if(m_ReferenceDim == XFLR5::MANUALREFDIM)    ar << 3;

		ar << m_bAutoInertia;
		ar << m_Mass;
		ar << m_CoG.x  << m_CoG.y  << m_CoG.z;
		ar << m_CoGIxx << m_CoGIyy << m_CoGIzz << m_CoGIxz;

		ar << m_nControls;
		for(i=0; i<m_nControls; i++)
		{
			ar << m_ControlGain[i];
		}

		ar << m_NXWakePanels << m_TotalWakeLength << m_WakePanelFactor;

		ar << m_QInfSpec;
		ar << m_AlphaSpec;
		ar << m_BetaSpec;

		// Last store the array data
		ar <<dataSize();
		for (i=0; i< dataSize(); i++)
		{
			ar << m_Alpha[i] << m_Beta[i] << m_QInfinite[i] << m_Ctrl[i];
			ar << m_CL[i] << m_CY[i] << m_ICd[i] << m_PCd[i] ;
			ar << m_GCm[i] << m_ICm[i] << m_VCm[i];
			ar << m_GRm[i];
			ar << m_GYm[i] << m_IYm[i] << m_VYm[i];
			ar << m_XCP[i] << m_YCP[i] << m_ZCP[i];
			ar << m_MaxBending[i];
			ar << m_XNP[i];

			ar <<m_EigenValue[0][i].real() <<m_EigenValue[1][i].real() <<m_EigenValue[2][i].real() <<m_EigenValue[3][i].real();
			ar <<m_EigenValue[0][i].imag() <<m_EigenValue[1][i].imag() <<m_EigenValue[2][i].imag() <<m_EigenValue[3][i].imag();
			ar <<m_EigenValue[4][i].real() <<m_EigenValue[5][i].real() <<m_EigenValue[6][i].real() <<m_EigenValue[7][i].real();
			ar <<m_EigenValue[4][i].imag() <<m_EigenValue[5][i].imag() <<m_EigenValue[6][i].imag() <<m_EigenValue[7][i].imag();
		}

		// space allocation for the future storage of more data, without need to change the format
		for (int i=0; i<19; i++) ar << 0;
		ar << m_PointStyle;
		for (int i=0; i<35; i++) ar << 0.0;
		for (int ix=0; ix<MAXEXTRADRAG; ix++) ar << m_ExtraDragArea[ix];
		for (int ix=0; ix<MAXEXTRADRAG; ix++) ar << m_ExtraDragCoef[ix];
		for (int i=0; i<7; i++) ar << m_inertiaGain[i];

		return true;
	}
	else
	{
		//input the variables from the stream
		ar >> m_PolarFormat;
		if(m_PolarFormat<200000 || m_PolarFormat>205000) return false;

		ar >> m_PlaneName;
		ar >> m_WPlrName;

		ar >> m_referenceArea >> m_referenceChordLength >> m_referenceSpanLength;
		ar >> m_Style >> m_Width;
		ar >> m_Color;
		ar >> m_bIsVisible >> boolean;

		ar >> n;
		if(n==1)      m_AnalysisMethod=XFLR5::LLTMETHOD;
		else if(n==2) m_AnalysisMethod=XFLR5::VLMMETHOD;
		else if(n==3) m_AnalysisMethod=XFLR5::PANELMETHOD;

		ar >> n;
		if(n==1)      m_WPolarType=XFLR5::FIXEDSPEEDPOLAR;
		else if(n==2) m_WPolarType=XFLR5::FIXEDLIFTPOLAR;
		else if(n==4) m_WPolarType=XFLR5::FIXEDAOAPOLAR;
		else if(n==5) m_WPolarType=XFLR5::BETAPOLAR;
		else if(n==7) m_WPolarType=XFLR5::STABILITYPOLAR;

		ar >> m_bVLM1;
		ar >> m_bThinSurfaces;
		ar >> m_bTiltedGeom;
		ar >> boolean;
		m_BoundaryCondition = boolean ? XFLR5::DIRICHLET : XFLR5::NEUMANN;
		ar >> m_bViscous;
		ar >> m_bIgnoreBodyPanels;

		ar >> m_bGround;
		ar >> m_Height;

		ar >> m_Density >> m_Viscosity;

		ar >> k;
		if(k==1)      m_ReferenceDim = XFLR5::PLANFORMREFDIM;
		else if(k==2) m_ReferenceDim = XFLR5::PROJECTEDREFDIM;
		else if(k==3) m_ReferenceDim = XFLR5::MANUALREFDIM;
		else          m_ReferenceDim = XFLR5::PLANFORMREFDIM;

		ar >> m_bAutoInertia;
		ar >> m_Mass;
		ar >> m_CoG.x  >> m_CoG.y  >> m_CoG.z;
		ar >> m_CoGIxx >> m_CoGIyy >> m_CoGIzz >> m_CoGIxz;

		ar >> m_nControls;
//		m_ControlGain.clear();
		for(int icg=0; icg<m_nControls; icg++)
		{
			ar >> dble;
			m_ControlGain[icg] = dble;
		}

		ar >> m_NXWakePanels >> m_TotalWakeLength >> m_WakePanelFactor;

		ar >> m_QInfSpec;
		ar >> m_AlphaSpec;
		ar >> m_BetaSpec;

		// Last store the array data
		// assumes the arrays have been cleared previously
		double d[20];
		clearData();

		ar >> n;
		if(qAbs(n)>10000) return false;

		for (i=0; i<n; i++)
		{
			for(int j=0; j<20; j++)
			{
				ar >> d[j];
			}
			insertDataAt(i, d[0],  d[1],  d[2],  d[3],  d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13],
							d[14], d[15], d[16], d[17], d[18], d[19]);


			ar >> r0 >> r1 >>r2 >> r3;
			ar >> i0 >> i1 >>i2 >> i3;
			ar >> r4 >> r5 >>r6 >> r7;
			ar >> i4 >> i5 >>i6 >> i7;
			m_EigenValue[0][i] = complex<double>(r0, i0);
			m_EigenValue[1][i] = complex<double>(r1, i1);
			m_EigenValue[2][i] = complex<double>(r2, i2);
			m_EigenValue[3][i] = complex<double>(r3, i3);
			m_EigenValue[4][i] = complex<double>(r4, i4);
			m_EigenValue[5][i] = complex<double>(r5, i5);
			m_EigenValue[6][i] = complex<double>(r6, i6);
			m_EigenValue[7][i] = complex<double>(r7, i7);

		}
	}

	// space allocation
	for (int i=0; i<19; i++) ar >> k;
	ar >> m_PointStyle;

	for (int i=0; i<35; i++) ar >> dble;
	for (int ix=0; ix<MAXEXTRADRAG; ix++) ar>> m_ExtraDragArea[ix];
	for (int ix=0; ix<MAXEXTRADRAG; ix++) ar>> m_ExtraDragCoef[ix];
	if(m_PolarFormat<200013)
	{
		for (int ix=0; ix<MAXEXTRADRAG; ix++)
		{
			m_ExtraDragArea[ix] = 0.0;
			m_ExtraDragCoef[ix] = 0.0;
		}
	}

	for (int i=0; i<7; i++)
	{
		ar >> m_inertiaGain[i];
		if(m_inertiaGain[i]>42 && m_inertiaGain[i]<51) m_inertiaGain[i] = 0.0; //correcting some former bad programming
	}

	for(int iPt=0; iPt<dataSize(); iPt++)	calculatePoint(iPt);

	return true;
}







