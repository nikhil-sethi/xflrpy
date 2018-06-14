/****************************************************************************

	PlaneOpp Class
	Copyright (C) 2006-2016 Andre Deperrois 

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

#include "PlaneOpp.h"
#include "Plane.h"
#include "WPolar.h"
#include "objects_global.h"

bool  PlaneOpp::s_bStoreOpps=true;
bool  PlaneOpp::s_bKeepOutOpps=false;

/**
*The public constructor
*/
PlaneOpp::PlaneOpp(void *pPlanePtr, void *pWPolarPtr, int PanelArraySize)
{
	m_PlaneName   = "";
	m_WPlrName    = "";


	m_NStation    = 0;
	m_NPanels     = 0;

	m_WPolarType     = XFLR5::FIXEDSPEEDPOLAR;
	m_AnalysisMethod = XFLR5::VLMMETHOD;

	m_Weight = 0.0;

	m_Style       = 0;
	m_Width       = 1;
	m_PointStyle  = 0;
	m_bIsVisible  = true;

	m_Color.setRed((int)(((double)qrand()/(double)RAND_MAX)*155)+100);
	m_Color.setGreen((int)(((double)qrand()/(double)RAND_MAX)*155)+100);
	m_Color.setBlue((int)(((double)qrand()/(double)RAND_MAX)*155)+100);

	m_bVLM1 = false;
	m_bThinSurface = true;
	m_bTiltedGeom = false;

	m_nControls = 0;

	m_Span = m_MAChord = 0.0;

	m_bOut        = false;
	m_bVLM1       = true;

	m_Alpha               = 0.0;
	m_Beta                = 0.0;
	m_Bank                = 0.0;
	m_QInf                = 0.0;
	m_Ctrl                = 0.0;


	m_CL=m_CX=m_CY=m_VCD=m_ICD=m_GCm=m_GRm=m_VCm=m_ICm=m_GYm=m_VYm=m_IYm=0.0;

	m_XNP = 0.0;

	memset(m_EigenValue, 0, sizeof(m_EigenValue)); //four longitudinal and four lateral modes
	memset(m_EigenVector, 0, sizeof(m_EigenVector));

	m_phiPH = complex<double>(0.0, 0.0);
	m_phiDR = complex<double>(0.0, 0.0);

	CXu = CZu = Cmu = 0.0;
	CXa = CLa = Cma = CXq = CLq = Cmq = CYb = CYp = CYr = Clb = Clp = Clr = Cnb = Cnp = Cnr = 0.0;

	memset(m_ALong, 0, 16*sizeof(double));
	memset(m_ALat, 0, 16*sizeof(double));
	memset(m_BLat, 0, 4*sizeof(double));
	memset(m_BLong, 0, 4*sizeof(double));

	CXe = CYe = CZe = CLe = CMe = CNe = 0;

//	for (int iw=0; iw<MAXWINGS; iw++) m_bWing[iw] = false;
//	m_bWing[0] = true;

	for (int iw=0; iw<MAXWINGS; iw++) m_pPlaneWOpp[iw] = NULL;

	m_dCp = m_dG = m_dSigma = NULL;
	allocateMemory(PanelArraySize);


	if(pPlanePtr)
	{
		Plane * pPlane = (Plane*)pPlanePtr;
		m_PlaneName = pPlane->planeName();
		m_MAChord   = pPlane->mac();
		m_Span      = pPlane->span();
		m_NStation  = pPlane->m_Wing[0].m_NStation;
	}

	if(pWPolarPtr)
	{
		WPolar *pWPolar = (WPolar*)pWPolarPtr;
		m_WPlrName        = pWPolar->polarName();
		m_bVLM1           = pWPolar->bVLM1();
		m_WPlrName        = pWPolar->polarName();
		m_bThinSurface    = pWPolar->bThinSurfaces();
		m_bTiltedGeom     = pWPolar->bTilted();
		m_WPolarType      = pWPolar->polarType();
		m_AnalysisMethod  = pWPolar->analysisMethod();
	}
}


/**
 * Adds a WingOpp to the PlaneOpp and initializes the data
 * @param iw the index of the wing for which a WingOpp is added
 * @param PanelArraySize the number of panels on the wing
 */
void PlaneOpp::addWingOpp(int iw, int PanelArraySize)
{
//	m_bWing[iw] = true;
	m_pPlaneWOpp[iw] = new WingOpp(PanelArraySize);
}



/**
 * The public destructor
 */
PlaneOpp::~PlaneOpp()
{
	releaseMemory();
}



/** Allocate memory to the arrays */
void PlaneOpp::allocateMemory(int PanelArraySize)
{
	releaseMemory();

	m_NPanels = PanelArraySize;
	m_dCp    = new double[PanelArraySize];
	m_dSigma = new double[PanelArraySize];
	m_dG     = new double[PanelArraySize];
	memset(m_dG,     0, PanelArraySize * sizeof(double));
	memset(m_dSigma, 0, PanelArraySize * sizeof(double));
	memset(m_dCp,    0, PanelArraySize * sizeof(double));
}


/**
 * Releases memory allocated on the heap
 */
void PlaneOpp::releaseMemory()
{
	if(m_dCp)    delete [] m_dCp;
	if(m_dSigma) delete [] m_dSigma;
	if(m_dG)     delete [] m_dG;

	m_dCp = NULL;
	m_dSigma = NULL;
	m_dG = NULL;

	for (int iw=0; iw<MAXWINGS; iw++)
	{
		if(m_pPlaneWOpp[iw] != NULL) delete m_pPlaneWOpp[iw];
		m_pPlaneWOpp[iw] = NULL;
	}
}


/**
 * Loads or saves the data of this operating point to a binary file.
 * This method serializes the data associated to the plane, then calls the serialization methods
 * of the wings.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool PlaneOpp::serializePOppWPA(QDataStream &ar, bool bIsStoring)
{
	int ArchiveFormat;
	int a, k;
	float f, h;

	if(bIsStoring)
	{
		// not storing to .wpa format anymore
	}
	else
	{
		//1009 : restored the serialization of Cp, Gamma, Sigma arrays...
		//1008 : removed the serialization of Cp, Gamma, Sigma arrays
		//1007 : added Sideslip Beta
		//1006 : added Panel's source strengths Sigma
		//1005 : added second wing results for a biplane
		//1004 : converted units to SI
		//1003 : added vortices strengths
		//1002 : added Cp, suppressed Cl and other parameters
		//1001 : suppressed Ai, Cd, SpanPos,
		ar >> ArchiveFormat;
		if(ArchiveFormat<1000 || ArchiveFormat>1100) return false;
		//read variables
		readCString(ar, m_PlaneName);
		readCString(ar, m_WPlrName);

		//always a main wing
		if(m_pPlaneWOpp[0]!=NULL) delete m_pPlaneWOpp[0];
		m_pPlaneWOpp[0] = new WingOpp();

		if(ArchiveFormat>=1005)
		{
			ar >> a;
			if (a!=0 && a!=1) return false;
			if(a)
			{
				if(m_pPlaneWOpp[1]!=NULL) delete m_pPlaneWOpp[1];
				m_pPlaneWOpp[1] = new WingOpp();
			}
		}

		ar >> a;
		if (a!=0 && a!=1) return false;
		if(a)
		{
			if(m_pPlaneWOpp[2]!=NULL) delete m_pPlaneWOpp[2];
			m_pPlaneWOpp[2] = new WingOpp();
		}

		ar >> a;
		if (a!=0 && a!=1) return false;
		if(a)
		{
			if(m_pPlaneWOpp[3]!=NULL) delete m_pPlaneWOpp[3];
			m_pPlaneWOpp[3] = new WingOpp();
		}


		ar >> a;
		if (a!=0 && a!=1) return false;
		if(a) m_bIsVisible = true; else m_bIsVisible = false;

		ar >> a; m_PointStyle = a;

		ar >> a;
		if (a!=0 && a!=1) return false;

		if(a) m_bOut = true; else m_bOut = false;

		ar >> a;
		if (a!=0 && a!=1) return false;

		if(a) m_bVLM1 = true; else m_bVLM1 = false;

		ar >> a;
		if (a!=0 && a!=1) return false;
//		if(a) m_bMiddle = true; else m_bMiddle = false;

		ar >> m_Style >> m_Width;

        int r,g,b;
        readCOLORREF(ar, r,g,b);

		ar >>k;
		if(k==1)      m_WPolarType = XFLR5::FIXEDSPEEDPOLAR;
		else if(k==2) m_WPolarType = XFLR5::FIXEDLIFTPOLAR;
		else if(k==4) m_WPolarType = XFLR5::FIXEDAOAPOLAR;
		else if(k==5) m_WPolarType = XFLR5::BETAPOLAR;
		else if(k==7) m_WPolarType = XFLR5::STABILITYPOLAR;
		else return false;


		ar >> m_NStation;
		ar >> f;		m_Alpha = f;
		ar >> f;        m_QInf  = f;
		ar >> f;//        m_Weight = f;

		if(ArchiveFormat>=1007)
		{
			ar>>f; m_Beta = f;
		}
		if(ArchiveFormat<1002)
		{
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
			ar >> f;
		}
		if(ArchiveFormat<1001)
		{
			for (k=0; k<=m_NStation; k++)
			{
				ar >> f;
			}
			for (k=0; k<=m_NStation; k++)
			{
				ar >> f;
			}
			for (k=0; k<=m_NStation; k++)
			{
				ar >> f;
			}
		}
		if(ArchiveFormat>=1002 )
		{
			ar >>m_NPanels;

			if(ArchiveFormat<=1007)
			{
				for (k=0; k<=m_NPanels; k++)
				{
					ar >> f;
	//				m_Cp[k] = f;
				}
			}
		}
		if(ArchiveFormat>=1003 && ArchiveFormat<=1007)
		{
			for (k=0; k<=m_NPanels; k++)
			{
				ar >> f;
//				if(ArchiveFormat<1004)	m_G[k] = f/1000.0;
//				else 					m_G[k] = f;
			}
		}

		if(ArchiveFormat>=1006 && ArchiveFormat<=1007)
		{
			for (k=0; k<=m_NPanels; k++)
			{
				ar >> f;
//				m_Sigma[k] = f;
			}
		}

		if(ArchiveFormat>=1009)
		{
			if(m_dG!=NULL)     delete [] m_dG;
			if(m_dSigma!=NULL) delete [] m_dSigma;
			if(m_dCp!=NULL)    delete [] m_dCp;

			m_dG     = new double[m_NPanels];
			m_dSigma = new double[m_NPanels];
			m_dCp    = new double[m_NPanels];

			for (k=0; k<m_NPanels; k++)
			{
				ar >> f >> g >> h;
				m_dCp[k]    = (double)f;
				m_dSigma[k] = (double)g;
				m_dG[k]     = (double)h;
			}
		}

		ar >> k; //VLMType

		if (!m_pPlaneWOpp[0]->serializeWingOppWPA(ar, bIsStoring))
		{
			return false;
		}


		if(ArchiveFormat>=1005)
		{
			if(m_pPlaneWOpp[1])
			{
				if (!m_pPlaneWOpp[1]->serializeWingOppWPA(ar, bIsStoring))
				{
					return false;
				}
			}
		}
		if(m_pPlaneWOpp[2])
		{
			if (!m_pPlaneWOpp[2]->serializeWingOppWPA(ar, bIsStoring))
			{
				return false;
			}

		}
		if(m_pPlaneWOpp[3])
		{
			if (!m_pPlaneWOpp[3]->serializeWingOppWPA(ar, bIsStoring))
			{
				return false;
			}
		}


		m_CL = m_pPlaneWOpp[0]->m_CL;
		m_CX = m_pPlaneWOpp[0]->m_CX;
		m_CY = m_pPlaneWOpp[0]->m_CY;

		m_ICD = m_pPlaneWOpp[0]->m_ICD;
		m_VCD = m_pPlaneWOpp[0]->m_VCD;

		m_ICm = m_pPlaneWOpp[0]->m_ICm;
		m_VCm = m_pPlaneWOpp[0]->m_VCm;
		m_GCm = m_pPlaneWOpp[0]->m_GCm;

		m_GRm = m_pPlaneWOpp[0]->m_GRm;

		m_IYm = m_pPlaneWOpp[0]->m_IYm;
		m_VYm = m_pPlaneWOpp[0]->m_VYm;
		m_GYm = m_pPlaneWOpp[0]->m_GYm;

		m_CP.copy(m_pPlaneWOpp[0]->m_CP);

		m_Ctrl = m_pPlaneWOpp[0]->m_oldCtrl;

		memcpy(m_EigenValue,  m_pPlaneWOpp[0]->m_oldEigenValue,  16*sizeof(double));
		memcpy(m_EigenVector, m_pPlaneWOpp[0]->m_oldEigenVector, 64*sizeof(double));

		int pos = 0;
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pPlaneWOpp[iw])
			{
				m_pPlaneWOpp[iw]->m_dCp    = m_dCp    + pos;
				m_pPlaneWOpp[iw]->m_dG     = m_dG     + pos;
				m_pPlaneWOpp[iw]->m_dSigma = m_dSigma + pos;
				pos +=m_pPlaneWOpp[iw]->m_NVLMPanels;
			}
		}


		if(ArchiveFormat>=1020)
		{
			// Non dimensional stability derivatives
			ar>>f;   CXa= f;
			ar>>f;   CXq= f;
			ar>>f;   CXu= f;
			ar>>f;   CZu= f;
			ar>>f;   Cmu= f;
		}
		if(ArchiveFormat>=1017)
		{
			// Non dimensional stability derivatives
			ar>>f;   CLa= f;
			ar>>f;   CLq= f;
			ar>>f;   Cma= f;
			ar>>f;   Cmq= f;
			ar>>f;   CYb= f;
			ar>>f;   CYp= f;
			ar>>f;   CYr= f;
			ar>>f;   Clb= f;
			ar>>f;   Clp= f;
			ar>>f;   Clr= f;
			ar>>f;   Cnb= f;
			ar>>f;   Cnp= f;
			ar>>f;   Cnr= f;
		}

		int n;
		float f0, f1,f2,f3;
		if(ArchiveFormat>=1018)
		{
			ar >> m_nControls;

			if(ArchiveFormat<1022) n = m_nControls;
			else                   n =1;
			for(k=0; k<n; k++)
			{
				ar >>f;   if(k==0) CXe=f;
				ar >>f;   if(k==0) CYe=f;
				ar >>f;   if(k==0) CZe=f;
				ar >>f;   if(k==0) CLe=f;
				ar >>f;   if(k==0) CMe=f;
				ar >>f;   if(k==0) CNe=f;

				ar >>f0>>f1>>f2>>f3;
				if(k==0) m_BLat[0]= f0; m_BLat[1]= f1; m_BLat[2] = f2; m_BLat[3] = f3;
				ar >>f0>>f1>>f2>>f3;
				m_BLong[0]=f0; m_BLong[1]=f1; m_BLong[2]= f2; m_BLong[3]= f3;
			}

			for(k=0; k<4; k++)
			{
				ar >>f0>>f1>>f2>>f3;
				m_ALong[k][0]= f0; m_ALong[k][1]= f1; m_ALong[k][2]= f2; m_ALong[k][3] = f3;
				ar >>f0>>f1>>f2>>f3;
				m_ALat[k][0] = f0; m_ALat[k][1] = f1; m_ALat[k][2] = f2; m_ALat[k][3] = f3;
			}
		}

		if(ArchiveFormat>=1019)
		{
			ar>>f;
			m_XNP = f;
		}
		else m_XNP = 0.0;
	}
	return true;
}


/**
 * Loads or saves the data of this operating point to a binary file.
 * This method serializes the data associated to the plane, then calls the serialization methods
 * of the wings.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool PlaneOpp::serializePOppXFL(QDataStream &ar, bool bIsStoring)
{
	int ArchiveFormat;
	bool boolean;
	int k, n;
	float f0, f1, f2;
	double dble, dbl1, dbl2;

	if(bIsStoring)
	{
		ar << 200001;
		//200001 : first go at the new format

		//write variables
		ar << m_PlaneName;
		ar << m_WPlrName;

		ar << m_Style << m_Width;
		writeQColor(ar, m_Color.red(), m_Color.green(), m_Color.blue(), m_Color.alpha());
		ar << m_bIsVisible << false;

		ar << m_bOut;
		ar << m_bVLM1;

		ar << m_bThinSurface << m_bTiltedGeom;

		if(m_WPolarType==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
		else if(m_WPolarType==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
		else if(m_WPolarType==XFLR5::FIXEDAOAPOLAR)   ar<<4;
		else if(m_WPolarType==XFLR5::BETAPOLAR)       ar<<5;
		else if(m_WPolarType==XFLR5::STABILITYPOLAR)  ar<<7;
		else ar << 1;

		if(m_AnalysisMethod==XFLR5::LLTMETHOD)         ar<<1;
		else if(m_AnalysisMethod==XFLR5::VLMMETHOD)    ar<<2;
		else if(m_AnalysisMethod==XFLR5::PANEL4METHOD) ar<<3;
		else if(m_AnalysisMethod==XFLR5::TRILINMETHOD) ar<<4;
		else if(m_AnalysisMethod==XFLR5::TRICSTMETHOD) ar<<5;
		else                                           ar<<0;

		ar << m_NPanels;
		ar << m_NStation;
		ar << m_Alpha << m_QInf;
		ar << m_Beta;
		ar << m_Ctrl;

		ar << m_Weight;

		if(m_AnalysisMethod!=XFLR5::LLTMETHOD)
		{
			for (k=0; k<m_NPanels; k++) ar<<(float)m_dCp[k]<<(float)m_dSigma[k]<<(float)m_dG[k];
		}

		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pPlaneWOpp[iw])  ar << 1; else ar<<0;

			if(m_pPlaneWOpp[iw])
			{
				m_pPlaneWOpp[iw]->serializeWingOppXFL(ar, bIsStoring);
			}
		}

		ar << m_CL << m_CX << m_CY;
		ar << m_VCD << m_ICD << m_VCm << m_ICm;
		ar << m_GRm;
		ar << m_GYm << m_VYm << m_IYm;
		ar << m_CP.x << m_CP.y << m_CP.z;

		ar << CXa << CXq << CXu << CZu <<Cmu;
		ar << CLa << CLq << Cma << Cmq;
		ar << CYb << CYp << CYr << Clb << Clp << Clr << Cnb << Cnp << Cnr;

		ar << m_nControls;
		ar<<CXe<<CYe<<CZe;
		ar<<CLe<<CMe<<CNe;

		ar << m_BLat[0] << m_BLat[1] << m_BLat[2] << m_BLat[3];
		ar << m_BLong[0]<< m_BLong[1]<< m_BLong[2]<< m_BLong[3];

		for(k=0; k<4; k++)
		{
			ar << m_ALong[k][0]<< m_ALong[k][1]<< m_ALong[k][2]<< m_ALong[k][3];
			ar << m_ALat[k][0] << m_ALat[k][1] << m_ALat[k][2] << m_ALat[k][3];
		}

		ar << m_XNP;

		for(int kv=0; kv<8;kv++)
		{
			ar << m_EigenValue[kv].real() << m_EigenValue[kv].imag();
			for(int lv=0; lv<4; lv++)
			{
				ar << m_EigenVector[kv][lv].real() << m_EigenVector[kv][lv].imag();
			}
		}

		// space allocation for the future storage of more data, without need to change the format
		for (int i=0; i<19; i++) ar << 0;
		ar << m_PointStyle;

		ar << m_MAChord<<m_Span;
		ar << m_phiPH.real() << m_phiPH.imag();
		ar << m_phiDR.real() << m_phiDR.imag();
		for (int i=6; i<50; i++) ar << 0.0;
	}
	else
	{
		ar >> ArchiveFormat;
		if (ArchiveFormat<200000 || ArchiveFormat>200003 ) return false;

		ar >> m_PlaneName;
		ar >> m_WPlrName;

		ar >> m_Style >> m_Width;

		int a,r,g,b;
		readQColor(ar, r, g, b, a);
		m_Color.setColor(r,g,b,a);

		ar >> m_bIsVisible >> boolean;

		ar >> m_bOut;
		ar >> m_bVLM1;

		ar >> m_bThinSurface >> m_bTiltedGeom;

		ar >> n;
		if(n==1)      m_WPolarType=XFLR5::FIXEDSPEEDPOLAR;
		else if(n==2) m_WPolarType=XFLR5::FIXEDLIFTPOLAR;
		else if(n==4) m_WPolarType=XFLR5::FIXEDAOAPOLAR;
		else if(n==5) m_WPolarType=XFLR5::BETAPOLAR;
		else if(n==7) m_WPolarType=XFLR5::STABILITYPOLAR;

		ar >> n;
		if(n==1)      m_AnalysisMethod=XFLR5::LLTMETHOD;
		else if(n==2) m_AnalysisMethod=XFLR5::VLMMETHOD;
		else if(n==3) m_AnalysisMethod=XFLR5::PANEL4METHOD;
		else if(n==4) m_AnalysisMethod=XFLR5::TRILINMETHOD;
		else if(n==5) m_AnalysisMethod=XFLR5::TRICSTMETHOD;

		ar >> m_NPanels;
		ar >> m_NStation;
		ar >> m_Alpha >> m_QInf;
		ar >> m_Beta;
		ar >> m_Ctrl;

		ar >> m_Weight;

		if(m_dG!=NULL)     delete [] m_dG;
		if(m_dSigma!=NULL) delete [] m_dSigma;
		if(m_dCp!=NULL)    delete [] m_dCp;

		m_dG     = new double[m_NPanels];
		m_dSigma = new double[m_NPanels];
		m_dCp    = new double[m_NPanels];

		if(m_AnalysisMethod!=XFLR5::LLTMETHOD)
		{
			for (k=0; k<m_NPanels; k++)
			{
				ar >> f0 >> f1 >> f2;
				m_dCp[k]    = (double)f0;
				m_dSigma[k] = (double)f1;
				m_dG[k]     = (double)f2;
			}
		}

		int pos = 0;
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			ar >> n;
			if(n) m_pPlaneWOpp[iw] = new WingOpp();
			else  m_pPlaneWOpp[iw] = NULL;

			if(m_pPlaneWOpp[iw])
			{
				m_pPlaneWOpp[iw]->serializeWingOppXFL(ar, bIsStoring);

				m_pPlaneWOpp[iw]->m_dCp    = m_dCp    + pos;
				m_pPlaneWOpp[iw]->m_dG     = m_dG     + pos;
				m_pPlaneWOpp[iw]->m_dSigma = m_dSigma + pos;
				pos +=m_pPlaneWOpp[iw]->m_NVLMPanels;
			}
		}


		ar >> m_CL >> m_CX >> m_CY;
		ar >> m_VCD >> m_ICD >> m_VCm >> m_ICm;
		ar >> m_GRm;
		ar >> m_GYm >> m_VYm >> m_IYm;
		ar >> m_CP.x >> m_CP.y >> m_CP.z;

		m_GCm = m_ICm + m_VCm;
		m_GYm = m_IYm + m_VYm;


		ar >> CXa >> CXq >> CXu >> CZu >> Cmu;
		ar >> CLa >> CLq >> Cma >> Cmq;
		ar >> CYb >> CYp >> CYr >> Clb >> Clp >> Clr >> Cnb >> Cnp >> Cnr;

		ar >> m_nControls;
		ar >> CXe >> CYe >> CZe;
		ar >> CLe >> CMe >> CNe;
		ar >> m_BLat[0] >> m_BLat[1] >> m_BLat[2] >> m_BLat[3];
		ar >> m_BLong[0]>> m_BLong[1]>> m_BLong[2]>> m_BLong[3];

		for(k=0; k<4; k++)
		{
			ar >> m_ALong[k][0]>> m_ALong[k][1]>> m_ALong[k][2]>> m_ALong[k][3];
			ar >> m_ALat[k][0] >> m_ALat[k][1] >> m_ALat[k][2] >> m_ALat[k][3];
		}

		ar >> m_XNP;
		if(m_WPolarType!=XFLR5::STABILITYPOLAR) m_XNP = 0.0;

		for(int kv=0; kv<8;kv++)
		{
			ar >> dbl1 >> dbl2;
			m_EigenValue[kv] = complex<double>(dbl1, dbl2);

			for(int lv=0; lv<4; lv++)
			{
				ar >> dbl1 >> dbl2;
				m_EigenVector[kv][lv] = complex<double>(dbl1, dbl2);
			}
		}

		// space allocation
		for (int i=0; i<19; i++) ar >> k;
		ar >> m_PointStyle;

		ar>>m_MAChord>>m_Span;

		double real, imag;
		ar >> real >> imag;
		m_phiPH = complex<double>(real, imag);
		ar >> real >> imag;
		m_phiDR = complex<double>(real, imag);

		for (int i=6; i<50; i++) ar >> dble;
	}
	return true;
}



void PlaneOpp::getPlaneOppProperties(QString &planeOppProperties, QString lengthUnitLabel, QString massUnitLabel, QString speedUnitLabel,
									 double mtoUnit, double kgtoUnit, double mstoUnit)
{
	QString strong, strange;

	planeOppProperties.clear();

	if(m_WPolarType==XFLR5::FIXEDSPEEDPOLAR)     strong += "Type 1 ("+QObject::tr("Fixed speed") +")\n";
	else if(m_WPolarType==XFLR5::FIXEDLIFTPOLAR) strong += "Type 2 ("+QObject::tr("Fixed lift") +")\n";
	else if(m_WPolarType==XFLR5::FIXEDAOAPOLAR)  strong += "Type 4 ("+QObject::tr("Fixed angle of attack") +")\n";
	else if(m_WPolarType==XFLR5::BETAPOLAR)      strong += "Type 5 ("+QObject::tr("Beta range") +")\n";
	else if(m_WPolarType==XFLR5::STABILITYPOLAR) strong += "Type 7 ("+QObject::tr("Stability analysis") +")\n";
	planeOppProperties += strong;

//	WOppProperties += QObject::tr("Method")+" = ";
	if(m_AnalysisMethod==XFLR5::LLTMETHOD)                             planeOppProperties +=QObject::tr("LLT");
	else if(m_AnalysisMethod==XFLR5::PANEL4METHOD && !m_bThinSurface)   planeOppProperties +=QObject::tr("3D-Panels");
	else if(m_AnalysisMethod==XFLR5::PANEL4METHOD && m_bVLM1)           planeOppProperties +=QObject::tr("3D-Panels/VLM1");
	else if(m_AnalysisMethod==XFLR5::PANEL4METHOD && !m_bVLM1)          planeOppProperties +=QObject::tr("3D-Panels/VLM2");
	planeOppProperties +="\n";


	if(m_bTiltedGeom) planeOppProperties += QObject::tr("Tilted geometry")+"\n";

	if(m_bOut) planeOppProperties += "Point is out of the flight envelope\n";

	strong  = QString(QObject::tr("VInf")+"  =%1 ").arg(m_QInf*mstoUnit,7,'f',3);
	planeOppProperties += "\n"+strong + speedUnitLabel+"\n";

	strong  = QString(QObject::tr("Alpha")+" =%1").arg(m_Alpha,7,'f',2);
	planeOppProperties += strong +QString::fromUtf8("°")+"\n";

	strong  = QString(QObject::tr("Mass")+"  = %1 ").arg(m_Weight*kgtoUnit,7,'f',3);
	planeOppProperties += strong + massUnitLabel + "\n";

	if(qAbs(m_Beta)>PRECISION)
	{
		strong  = QString(QObject::tr("Beta")+"  = %1").arg(m_Beta,7,'f',2);
		planeOppProperties += strong +QString::fromUtf8("°")+"\n\n";
	}

	if(m_WPolarType==XFLR5::STABILITYPOLAR)
	{
		strong  = QString(QObject::tr("Control value")+" = %1").arg(m_Ctrl,5,'f',2);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("XNP")+" = %1 ").arg(m_XNP*mtoUnit,7,'f',3);
		planeOppProperties += "\n"+strong +lengthUnitLabel+"\n";
	}
	strong  = QString(QObject::tr("XCP")+" = %1 ").arg(m_CP.x*mtoUnit,7,'f',3);
	planeOppProperties += strong +lengthUnitLabel+"\n";
	strong  = QString(QObject::tr("YCP")+" = %1 ").arg(m_CP.y*mtoUnit,7,'f',3);
	planeOppProperties += strong +lengthUnitLabel+"\n";
	strong  = QString(QObject::tr("ZCP")+" = %1 ").arg(m_CP.z*mtoUnit,7,'f',3);
	planeOppProperties += strong +lengthUnitLabel+"\n\n";


	strong  = QString(QObject::tr("CL")+"  = %1").arg(m_CL,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("CD")+"  = %1").arg(m_ICD+m_VCD,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("VCD")+" = %1").arg(m_VCD,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("ICD")+" = %1").arg(m_ICD,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("CX")+"  = %1").arg(m_CX,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("CY")+"  = %1").arg(m_CY,9,'f',5);
	planeOppProperties += strong +"\n";

	strong  = QString(QObject::tr("Cl")+"  = %1").arg(m_GRm,9,'f',5);
	planeOppProperties += strong +"\n";

	strong  = QString(QObject::tr("Cm")+"  = %1").arg(m_GCm,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("ICm")+" = %1").arg(m_ICm,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("VCm")+" = %1").arg(m_VCm,9,'f',5);
	planeOppProperties += strong +"\n";

	strong  = QString(QObject::tr("Cn")+"  = %1").arg(m_GYm,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("ICn")+" = %1").arg(m_IYm,9,'f',5);
	planeOppProperties += strong +"\n";
	strong  = QString(QObject::tr("VCn")+" = %1").arg(m_VYm,9,'f',5);
	planeOppProperties += strong +"\n";

/*	if(m_nFlaps>0)
	{
		WingOppProperties += "\n";
		for(int ip=0; ip<m_nFlaps; ip++)
		{
			strong = QString("Flap Moment[%1] = %2").arg(ip+1).arg(m_FlapMoment[ip]*UnitsDlg::NmtoUnit(), 7,'g',3);
			UnitsDlg::GetMomentUnit(strange, UnitsDlg::momentUnit());
			WingOppProperties += strong + strange +"\n";
		}
	}*/


	if(m_WPolarType==XFLR5::STABILITYPOLAR)
	{
		planeOppProperties += "\n\n";
		planeOppProperties += QObject::tr("Non-dimensional Stability Derivatives:")+"\n";
		strong  = QString(QObject::tr("CXu")+"  = %1").arg(CXu,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("CLu")+"  = %1").arg(-CZu,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Cmu")+"  = %1").arg(Cmu,9,'f',5);
		planeOppProperties += strong +"\n\n";
		strong  = QString(QObject::tr("CXa")+"  = %1").arg(CXa,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("CLa")+"  = %1").arg(CLa,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Cma")+"  = %1").arg(Cma,9,'f',5);
		planeOppProperties += strong +"\n\n";
		strong  = QString(QObject::tr("CXq")+"  = %1").arg(CXq,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("CLq")+"  = %1").arg(CLq,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Cmq")+"  = %1").arg(Cmq,9,'f',5);
		planeOppProperties += strong +"\n\n";
		strong  = QString(QObject::tr("CYb")+"  = %1").arg(CYb,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Clb")+"  = %1").arg(Clb,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Cnb")+"  = %1").arg(Cnb,9,'f',5);
		planeOppProperties += strong +"\n\n";
		strong  = QString(QObject::tr("CYp")+"  = %1").arg(CYp,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Clp")+"  = %1").arg(Clp,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Cnp")+"  = %1").arg(Cnp,9,'f',5);
		planeOppProperties += strong +"\n\n";
		strong  = QString(QObject::tr("CYr")+"  = %1").arg(CYr,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Clr")+"  = %1").arg(Clr,9,'f',5);
		planeOppProperties += strong +"\n";
		strong  = QString(QObject::tr("Cnr")+"  = %1").arg(Cnr,9,'f',5);
		planeOppProperties += strong +"\n\n";

		if(m_nControls>0)
		{
			// (only one)
			planeOppProperties += QObject::tr("Non-dimensional Control Derivatives:")+"\n";
			strong  = QString(QObject::tr("CXd")+"  = %1").arg(CXe,9,'f',5);
			planeOppProperties += strong +"\n";
			strong  = QString(QObject::tr("CYd")+"  = %1").arg(CYe,9,'f',5);
			planeOppProperties += strong +"\n";
			strong  = QString(QObject::tr("CZd")+"  = %1").arg(CZe,9,'f',5);
			planeOppProperties += strong +"\n";
			strong  = QString(QObject::tr("Cld")+"  = %1").arg(CLe,9,'f',5);
			planeOppProperties += strong +"\n";
			strong  = QString(QObject::tr("Cmd")+"  = %1").arg(CMe,9,'f',5);
			planeOppProperties += strong +"\n";
			strong  = QString(QObject::tr("Cnd")+"  = %1").arg(CNe,9,'f',5);
			planeOppProperties += strong +"\n\n";
		}

		complex<double> c, angle;
		double OmegaN, Omega1, Dsi, u0, mac, span;
		u0   = m_QInf;
		mac  = m_MAChord;
		span = m_Span;


		planeOppProperties += "\nLongitudinal modes:\n";
		for(int im=0; im<4; im++)
		{
			c = m_EigenValue[im];
            modeProperties(c, OmegaN, Omega1, Dsi);

			if(c.imag()>=0.0) strange = QString("  Eigenvalue    = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("  Eigenvalue    = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			strange = QString("  Undamped Natural Frequency = %1 Hz").arg(OmegaN/2.0/PI, 8,'f',3);
			planeOppProperties += strange +"\n";

			strange = QString("  Damped Natural Frequency   = %1 Hz").arg(Omega1/2.0/PI, 8,'f',3);
			planeOppProperties += strange +"\n";

			strange = QString("  Damping Ratio              = %1 ").arg(Dsi, 8,'f',3);
			planeOppProperties += strange +"\n";

			planeOppProperties += "  Normalized Eigenvector:\n";
			angle = m_EigenVector[im][3];
			c = m_EigenVector[im][0]/u0;
			if(c.imag()>=0.0) strange = QString("    u/u0          = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    u/u0          = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			c = m_EigenVector[im][1]/u0;
			if(c.imag()>=0.0) strange = QString("    w/u0          = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    w/u0          = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			c = m_EigenVector[im][2]/(2.0*u0/mac);
			if(c.imag()>=0.0) strange = QString("    q/(2.u0.MAC)  = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    q/(2.u0.MAC)  = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			c = m_EigenVector[im][3]/angle;
			if(c.imag()>=0.0) strange = QString("    theta(rad)    = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    theta(rad)    = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n\n";
		}

		planeOppProperties += "\nLateral modes:\n";
		for(int im=4; im<8; im++)
		{
			c = m_EigenValue[im];
            modeProperties(c, OmegaN, Omega1, Dsi);

			if(c.imag()>=0.0) strange = QString("  Eigenvalue    = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("  Eigenvalue    = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			strange = QString("  Undamped Natural Frequency = %1 Hz").arg(OmegaN/2.0/PI, 8,'f',3);
			planeOppProperties += strange +"\n";

			strange = QString("  Damped Natural Frequency   = %1 Hz").arg(Omega1/2.0/PI, 8,'f',3);
			planeOppProperties += strange +"\n";

			strange = QString("  Damping Ratio              = %1 ").arg(Dsi, 8,'f',3);
			planeOppProperties += strange +"\n";


			if(fabs(c.real())>PRECISION && fabs(c.imag())<PRECISION)
			{
				strange.sprintf(   "  Time to double            = %8.3fs", log(2)/fabs(c.real()));
				planeOppProperties += strange +"\n";
				if(c.real()<0.0)
				{
					strange.sprintf("  Time constant              =%8.3f", -1.0/c.real());
					planeOppProperties += strange +"\n";
				}
			}


			planeOppProperties += "  Normalized Eigenvector:\n";

			angle = m_EigenVector[im][3];

			c = m_EigenVector[im][0]/u0;
			if(c.imag()>=0.0) strange = QString("    v/u0          = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    v/u0          = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			c = m_EigenVector[im][1]/(2.0*u0/span);
			if(c.imag()>=0.0) strange = QString("    p/(2.u0.Span) = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    p/(2.u0.Span) = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			c = m_EigenVector[im][2]/(2.0*u0/span);
			if(c.imag()>=0.0) strange = QString("    r/(2.u0.Span) = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    r/(2.u0.Span) = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n";

			c = m_EigenVector[im][3]/angle;
			if(c.imag()>=0.0) strange = QString("    phi(rad)      = %1+%2i").arg(c.real(),10,'f',5).arg(c.imag(),10,'f',5);
			else              strange = QString("    phi(rad)      = %1-%2i").arg(c.real(),10,'f',5).arg(qAbs(c.imag()),10,'f',5);
			planeOppProperties += strange +"\n\n";
		}

        modeProperties(m_phiPH, OmegaN, Omega1, Dsi);

		planeOppProperties += "Phillips Phugoid eq.38 JOURNAL OF AIRCRAFT Vol. 37, No. 1, January–February 2000\n";
		if(c.imag()>=0.0) strange.sprintf("  Eigenvalue    = %9.5f+%9.5fi", m_phiPH.real(), m_phiPH.imag());
		else              strange.sprintf("  Eigenvalue    = %9.5f-%9.5fi", m_phiPH.real(), m_phiPH.imag());
		planeOppProperties += strange +"\n";

		strange.sprintf("     Undamped Natural Frequency = %8.3f Hz",OmegaN/2.0/PI);
		planeOppProperties += strange +"\n";

		strange.sprintf("     Damped Natural Frequency   = %8.3f Hz",Omega1/2.0/PI);
		planeOppProperties += strange +"\n";

		strange.sprintf("     Damping Ratio              = %8.3f ", Dsi);
		planeOppProperties += strange +"\n";


        modeProperties(m_phiDR, OmegaN, Omega1, Dsi);

		planeOppProperties += "Phillips Dutch-Roll eq. 28 JOURNAL OF AIRCRAFT Vol. 37, No. 3, May–June 2000\n";
		if(c.imag()>=0.0) strange.sprintf("  Eigenvalue    = %9.5f+%9.5fi", m_phiDR.real(), m_phiDR.imag());
		else              strange.sprintf("  Eigenvalue    = %9.5f-%9.5fi", m_phiDR.real(), m_phiDR.imag());
		planeOppProperties += strange +"\n";

		strange.sprintf("     Undamped Natural Frequency = %8.3f Hz",OmegaN/2.0/PI);
		planeOppProperties += strange +"\n";

		strange.sprintf("     Damped Natural Frequency   = %8.3f Hz",Omega1/2.0/PI);
		planeOppProperties += strange +"\n";

		strange.sprintf("     Damping Ratio              = %8.3f ", Dsi);
		planeOppProperties += strange +"\n";
	}
}

