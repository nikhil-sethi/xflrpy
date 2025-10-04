/****************************************************************************

    PlaneOpp Class
    Copyright (C) 2006-2019 André Deperrois

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

#include <QRandomGenerator>

#include "planeopp.h"
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflcore/mathelem.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>

bool  PlaneOpp::s_bStoreOpps=true;
bool  PlaneOpp::s_bKeepOutOpps=false;

/**
*The public constructor
*/
PlaneOpp::PlaneOpp(Plane *pPlane, WPolar *pWPolar, int PanelArraySize)
{
    m_PlaneName   = "";
    m_WPlrName    = "";

    m_NStation    = 0;
    m_NPanels     = 0;

    m_WPolarType     = xfl::FIXEDSPEEDPOLAR;
    m_AnalysisMethod = xfl::VLMMETHOD;

    m_Weight = 0.0;

    m_theStyle.m_Stipple     = Line::SOLID;
    m_theStyle.m_Width       = 1;
    m_theStyle.m_Symbol  = Line::NOSYMBOL;
    m_theStyle.m_bIsVisible  = true;

    m_theStyle.m_Color.setHsv(QRandomGenerator::global()->bounded(360),
               QRandomGenerator::global()->bounded(55)+30,
               QRandomGenerator::global()->bounded(55)+150);

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

    m_phiPH = std::complex<double>(0.0, 0.0);
    m_phiDR = std::complex<double>(0.0, 0.0);

    CXu = CZu = Cmu = 0.0;
    CXa = CLa = Cma = CXq = CLq = Cmq = CYb = CYp = CYr = Clb = Clp = Clr = Cnb = Cnp = Cnr = 0.0;

    memset(m_ALong, 0, 16*sizeof(double));
    memset(m_ALat, 0, 16*sizeof(double));
    memset(m_BLat, 0, 4*sizeof(double));
    memset(m_BLong, 0, 4*sizeof(double));

    CXe = CYe = CZe = CLe = CMe = CNe = 0;

    //    for (int iw=0; iw<MAXWINGS; iw++) m_bWing[iw] = false;
    //    m_bWing[0] = true;

    for (int iw=0; iw<MAXWINGS; iw++) m_pWOpp[iw] = nullptr;

    m_dCp = m_dG = m_dSigma = nullptr;
    allocateMemory(PanelArraySize);


    if(pPlane)
    {
        m_PlaneName = pPlane->name();
        m_MAChord   = pPlane->mac();
        m_Span      = pPlane->span();
        m_NStation  = pPlane->m_Wing[0].m_NStation;
    }

    if(pWPolar)
    {
        m_WPlrName        = pWPolar->polarName();
        m_bVLM1           = pWPolar->bVLM1();
        m_WPlrName        = pWPolar->polarName();
        m_bThinSurface    = pWPolar->bThinSurfaces();
        m_bTiltedGeom     = pWPolar->bTilted();
        m_WPolarType      = pWPolar->polarType();
        m_AnalysisMethod  = pWPolar->analysisMethod();
    }
}


QString PlaneOpp::name() const
{
    QString strange;
    QString format("%1");

    if     (isT7Polar()) strange = QString(format).arg(ctrl(),  7, 'f', 3);
    else if(isT5Polar()) strange = QString(format).arg(beta(),  7, 'f', 3);
    else if(isT4Polar()) strange = QString(format).arg(QInf()*Units::mstoUnit(),  7, 'f', 3);
    else                 strange = QString(format).arg(alpha(), 7, 'f', 3);
    return strange;
}


/**
 * Adds a WingOpp to the PlaneOpp and initializes the data
 * @param iw the index of the wing for which a WingOpp is added
 * @param PanelArraySize the number of panels on the wing
 */
void PlaneOpp::addWingOpp(int iw, int PanelArraySize)
{
    //    m_bWing[iw] = true;
    m_pWOpp[iw] = new WingOpp(PanelArraySize);
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
    m_dCp    = new double[ulong(PanelArraySize)];
    m_dSigma = new double[ulong(PanelArraySize)];
    m_dG     = new double[ulong(PanelArraySize)];
    memset(m_dG,     0, ulong(PanelArraySize) * sizeof(double));
    memset(m_dSigma, 0, ulong(PanelArraySize) * sizeof(double));
    memset(m_dCp,    0, ulong(PanelArraySize) * sizeof(double));
}


/**
 * Releases memory allocated on the heap
 */
void PlaneOpp::releaseMemory()
{
    if(m_dCp)    delete [] m_dCp;
    if(m_dSigma) delete [] m_dSigma;
    if(m_dG)     delete [] m_dG;

    m_dCp = nullptr;
    m_dSigma = nullptr;
    m_dG = nullptr;

    for (int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWOpp[iw] != nullptr) delete m_pWOpp[iw];
        m_pWOpp[iw] = nullptr;
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
    int ArchiveFormat(0);
    int a(0), k(0);
    float f(0), h(0);

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
        xfl::readCString(ar, m_PlaneName);
        xfl::readCString(ar, m_WPlrName);

        //always a main wing
        if(m_pWOpp[0]!=nullptr) delete m_pWOpp[0];
        m_pWOpp[0] = new WingOpp();

        if(ArchiveFormat>=1005)
        {
            ar >> a;
            if (a!=0 && a!=1) return false;
            if(a)
            {
                if(m_pWOpp[1]!=nullptr) delete m_pWOpp[1];
                m_pWOpp[1] = new WingOpp();
            }
        }

        ar >> a;
        if (a!=0 && a!=1) return false;
        if(a)
        {
            if(m_pWOpp[2]!=nullptr) delete m_pWOpp[2];
            m_pWOpp[2] = new WingOpp();
        }

        ar >> a;
        if (a!=0 && a!=1) return false;
        if(a)
        {
            if(m_pWOpp[3]!=nullptr) delete m_pWOpp[3];
            m_pWOpp[3] = new WingOpp();
        }


        ar >> a;
        if (a!=0 && a!=1) return false;
        if(a) m_theStyle.m_bIsVisible = true; else m_theStyle.m_bIsVisible = false;

        ar >> a; m_theStyle.setPointStyle(a);

        ar >> a;
        if (a!=0 && a!=1) return false;

        if(a) m_bOut = true; else m_bOut = false;

        ar >> a;
        if (a!=0 && a!=1) return false;

        if(a) m_bVLM1 = true; else m_bVLM1 = false;

        ar >> a;
        if (a!=0 && a!=1) return false;
        //        if(a) m_bMiddle = true; else m_bMiddle = false;

        ar >> k;   m_theStyle.setStipple(k);
        ar >> m_theStyle.m_Width;

        int r,g,b;
        xfl::readCOLORREF(ar, r,g,b);

        ar >>k;
        if(k==1)      m_WPolarType = xfl::FIXEDSPEEDPOLAR;
        else if(k==2) m_WPolarType = xfl::FIXEDLIFTPOLAR;
        else if(k==4) m_WPolarType = xfl::FIXEDAOAPOLAR;
        else if(k==5) m_WPolarType = xfl::BETAPOLAR;
        else if(k==7) m_WPolarType = xfl::STABILITYPOLAR;
        else return false;


        ar >> m_NStation;
        ar >> f;        m_Alpha = double(f);
        ar >> f;        m_QInf  = double(f);
        ar >> f;//        m_Weight = f;

        if(ArchiveFormat>=1007)
        {
            ar>>f; m_Beta = double(f);
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
                    //                m_Cp[k] = f;
                }
            }
        }
        if(ArchiveFormat>=1003 && ArchiveFormat<=1007)
        {
            for (k=0; k<=m_NPanels; k++)
            {
                ar >> f;
                //                if(ArchiveFormat<1004)    m_G[k] = f/1000.0;
                //                else                     m_G[k] = f;
            }
        }

        if(ArchiveFormat>=1006 && ArchiveFormat<=1007)
        {
            for (k=0; k<=m_NPanels; k++)
            {
                ar >> f;
                //                m_Sigma[k] = f;
            }
        }

        if(ArchiveFormat>=1009)
        {
            if(m_dG!=nullptr)     delete [] m_dG;
            if(m_dSigma!=nullptr) delete [] m_dSigma;
            if(m_dCp!=nullptr)    delete [] m_dCp;

            m_dG     = new double[ulong(m_NPanels)];
            m_dSigma = new double[ulong(m_NPanels)];
            m_dCp    = new double[ulong(m_NPanels)];

            for (k=0; k<m_NPanels; k++)
            {
                ar >> f >> g >> h;
                m_dCp[k]    = double(f);
                m_dSigma[k] = double(g);
                m_dG[k]     = double(h);
            }
        }

        ar >> k; //VLMType

        if (!m_pWOpp[0]->serializeWingOppWPA(ar, bIsStoring))
        {
            return false;
        }


        if(ArchiveFormat>=1005)
        {
            if(m_pWOpp[1])
            {
                if (!m_pWOpp[1]->serializeWingOppWPA(ar, bIsStoring))
                {
                    return false;
                }
            }
        }
        if(m_pWOpp[2])
        {
            if (!m_pWOpp[2]->serializeWingOppWPA(ar, bIsStoring))
            {
                return false;
            }

        }
        if(m_pWOpp[3])
        {
            if (!m_pWOpp[3]->serializeWingOppWPA(ar, bIsStoring))
            {
                return false;
            }
        }


        m_CL = m_pWOpp[0]->m_CL;
        m_CX = m_pWOpp[0]->m_CX;
        m_CY = m_pWOpp[0]->m_CY;

        m_ICD = m_pWOpp[0]->m_ICD;
        m_VCD = m_pWOpp[0]->m_VCD;

        m_ICm = m_pWOpp[0]->m_ICm;
        m_VCm = m_pWOpp[0]->m_VCm;
        m_GCm = m_pWOpp[0]->m_GCm;

        m_GRm = m_pWOpp[0]->m_GRm;

        m_IYm = m_pWOpp[0]->m_IYm;
        m_VYm = m_pWOpp[0]->m_VYm;
        m_GYm = m_pWOpp[0]->m_GYm;

        m_CP.copy(m_pWOpp[0]->m_CP);

        m_Ctrl = m_pWOpp[0]->m_oldCtrl;

        memcpy(m_EigenValue,  m_pWOpp[0]->m_oldEigenValue,  16*sizeof(double));
        memcpy(m_EigenVector, m_pWOpp[0]->m_oldEigenVector, 64*sizeof(double));

        int pos = 0;
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pWOpp[iw])
            {
                m_pWOpp[iw]->m_dCp    = m_dCp    + pos;
                m_pWOpp[iw]->m_dG     = m_dG     + pos;
                m_pWOpp[iw]->m_dSigma = m_dSigma + pos;
                pos +=m_pWOpp[iw]->m_NVLMPanels;
            }
        }

        if(ArchiveFormat>=1020)
        {
            // Non dimensional stability derivatives
            ar>>f;   CXa= double(f);
            ar>>f;   CXq= double(f);
            ar>>f;   CXu= double(f);
            ar>>f;   CZu= double(f);
            ar>>f;   Cmu= double(f);
        }
        if(ArchiveFormat>=1017)
        {
            // Non dimensional stability derivatives
            ar>>f;   CLa= double(f);
            ar>>f;   CLq= double(f);
            ar>>f;   Cma= double(f);
            ar>>f;   Cmq= double(f);
            ar>>f;   CYb= double(f);
            ar>>f;   CYp= double(f);
            ar>>f;   CYr= double(f);
            ar>>f;   Clb= double(f);
            ar>>f;   Clp= double(f);
            ar>>f;   Clr= double(f);
            ar>>f;   Cnb= double(f);
            ar>>f;   Cnp= double(f);
            ar>>f;   Cnr= double(f);
        }

        int n=0;
        float f0=0, f1=0,f2=0,f3=0;
        if(ArchiveFormat>=1018)
        {
            ar >> m_nControls;

            if(ArchiveFormat<1022) n = m_nControls;
            else                   n =1;
            for(k=0; k<n; k++)
            {
                ar >>f;   if(k==0) CXe=double(f);
                ar >>f;   if(k==0) CYe=double(f);
                ar >>f;   if(k==0) CZe=double(f);
                ar >>f;   if(k==0) CLe=double(f);
                ar >>f;   if(k==0) CMe=double(f);
                ar >>f;   if(k==0) CNe=double(f);

                ar >>f0>>f1>>f2>>f3;
                if(k==0) { m_BLat[0]= double(f0); m_BLat[1]= double(f1); m_BLat[2] = double(f2); m_BLat[3] = double(f3);}
                ar >>f0>>f1>>f2>>f3;
                m_BLong[0]=double(f0); m_BLong[1]=double(f1); m_BLong[2]= double(f2); m_BLong[3]= double(f3);
            }

            for(k=0; k<4; k++)
            {
                ar >>f0>>f1>>f2>>f3;
                m_ALong[k][0]= double(f0); m_ALong[k][1]= double(f1); m_ALong[k][2]= double(f2); m_ALong[k][3] = double(f3);
                ar >>f0>>f1>>f2>>f3;
                m_ALat[k][0] = double(f0); m_ALat[k][1] = double(f1); m_ALat[k][2] = double(f2); m_ALat[k][3] = double(f3);
            }
        }

        if(ArchiveFormat>=1019)
        {
            ar>>f;
            m_XNP = double(f);
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
    bool boolean(false);
    int k(0), n(0);
    float f0(0), f1(0), f2(0);
    double dble(0), dbl1(0), dbl2(0);

    // 200002: added new format LineStyle
    int ArchiveFormat = 200002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        //200001 : first go at the new format

        //write variables
        ar << m_PlaneName;
        ar << m_WPlrName;
/*
        ar << m_Style << m_Width;
        writeQColor(ar, m_Color.red(), m_Color.green(), m_Color.blue(), m_Color.alpha());
        ar << m_bIsVisible << false;
*/
        m_theStyle.serializeXfl(ar, bIsStoring);

        ar << m_bOut;
        ar << m_bVLM1;

        ar << m_bThinSurface << m_bTiltedGeom;

        if(m_WPolarType==xfl::FIXEDSPEEDPOLAR)      ar<<1;
        else if(m_WPolarType==xfl::FIXEDLIFTPOLAR)  ar<<2;
        else if(m_WPolarType==xfl::FIXEDAOAPOLAR)   ar<<4;
        else if(m_WPolarType==xfl::BETAPOLAR)       ar<<5;
        else if(m_WPolarType==xfl::STABILITYPOLAR)  ar<<7;
        else ar << 1;

        if(m_AnalysisMethod==xfl::LLTMETHOD)         ar<<1;
        else if(m_AnalysisMethod==xfl::VLMMETHOD)    ar<<2;
        else if(m_AnalysisMethod==xfl::PANEL4METHOD) ar<<3;
        else if(m_AnalysisMethod==xfl::TRILINMETHOD) ar<<4;
        else if(m_AnalysisMethod==xfl::TRIUNIMETHOD) ar<<5;
        else                                           ar<<0;

        ar << m_NPanels;
        ar << m_NStation;
        ar << m_Alpha << m_QInf;
        ar << m_Beta;
        ar << m_Ctrl;

        ar << m_Weight;

        if(m_AnalysisMethod!=xfl::LLTMETHOD)
        {
            for (k=0; k<m_NPanels; k++) ar<<float(m_dCp[k])<<float(m_dSigma[k])<<float(m_dG[k]);
        }

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pWOpp[iw])  ar << 1; else ar<<0;

            if(m_pWOpp[iw])
            {
                m_pWOpp[iw]->serializeWingOppXFL(ar, bIsStoring);
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
        ar << k; //m_theStyle.m_Symbol;

        ar << m_MAChord<<m_Span;
        ar << m_phiPH.real() << m_phiPH.imag();
        ar << m_phiDR.real() << m_phiDR.imag();
        for (int i=6; i<50; i++) ar << 0.0;
    }
    else
    {
        ar >> ArchiveFormat;
        if (ArchiveFormat<200000 || ArchiveFormat>200100 ) return false;

        ar >> m_PlaneName;
        ar >> m_WPlrName;

        if(ArchiveFormat<200002)
        {
            ar >> n; m_theStyle.setStipple(n);
            ar >> m_theStyle.m_Width;
            int a(0),r(0),g(0),b(0);
            xfl::readQColor(ar, r, g, b, a);
            m_theStyle.m_Color = {r,g,b,a};

            ar >> m_theStyle.m_bIsVisible >> boolean;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        ar >> m_bOut;
        ar >> m_bVLM1;

        ar >> m_bThinSurface >> m_bTiltedGeom;

        ar >> n;
        if(n==1)      m_WPolarType=xfl::FIXEDSPEEDPOLAR;
        else if(n==2) m_WPolarType=xfl::FIXEDLIFTPOLAR;
        else if(n==4) m_WPolarType=xfl::FIXEDAOAPOLAR;
        else if(n==5) m_WPolarType=xfl::BETAPOLAR;
        else if(n==7) m_WPolarType=xfl::STABILITYPOLAR;

        ar >> n;
        if(n==1)      m_AnalysisMethod=xfl::LLTMETHOD;
        else if(n==2) m_AnalysisMethod=xfl::VLMMETHOD;
        else if(n==3) m_AnalysisMethod=xfl::PANEL4METHOD;
        else if(n==4) m_AnalysisMethod=xfl::TRILINMETHOD;
        else if(n==5) m_AnalysisMethod=xfl::TRIUNIMETHOD;

        ar >> m_NPanels;
        ar >> m_NStation;
        ar >> m_Alpha >> m_QInf;
        ar >> m_Beta;
        ar >> m_Ctrl;

        ar >> m_Weight;

        if(m_dG!=nullptr)     delete [] m_dG;
        if(m_dSigma!=nullptr) delete [] m_dSigma;
        if(m_dCp!=nullptr)    delete [] m_dCp;

        m_dG     = new double[ulong(m_NPanels)];
        m_dSigma = new double[ulong(m_NPanels)];
        m_dCp    = new double[ulong(m_NPanels)];

        if(m_AnalysisMethod!=xfl::LLTMETHOD)
        {
            for (k=0; k<m_NPanels; k++)
            {
                ar >> f0 >> f1 >> f2;
                m_dCp[k]    = double(f0);
                m_dSigma[k] = double(f1);
                m_dG[k]     = double(f2);
            }
        }

        int pos = 0;
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            ar >> n;
            if(n) m_pWOpp[iw] = new WingOpp();
            else  m_pWOpp[iw] = nullptr;

            if(m_pWOpp[iw])
            {
                m_pWOpp[iw]->serializeWingOppXFL(ar, bIsStoring);

                m_pWOpp[iw]->m_dCp    = m_dCp    + pos;
                m_pWOpp[iw]->m_dG     = m_dG     + pos;
                m_pWOpp[iw]->m_dSigma = m_dSigma + pos;
                pos +=m_pWOpp[iw]->m_NVLMPanels;
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
        if(m_WPolarType!=xfl::STABILITYPOLAR) m_XNP = 0.0;

        for(int kv=0; kv<8;kv++)
        {
            ar >> dbl1 >> dbl2;
            m_EigenValue[kv] = std::complex<double>(dbl1, dbl2);

            for(int lv=0; lv<4; lv++)
            {
                ar >> dbl1 >> dbl2;
                m_EigenVector[kv][lv] = std::complex<double>(dbl1, dbl2);
            }
        }

        // space allocation
        for (int i=0; i<19; i++) ar >> k;
        ar >> k; //m_Symbol;

        ar>>m_MAChord>>m_Span;

        double real, imag;
        ar >> real >> imag;
        m_phiPH = std::complex<double>(real, imag);
        ar >> real >> imag;
        m_phiDR = std::complex<double>(real, imag);

        for (int i=6; i<50; i++) ar >> dble;
    }
    return true;
}


void PlaneOpp::getProperties(QString &planeOppProperties, QString lengthUnitLabel, QString massUnitLabel, QString speedUnitLabel,
                                     double mtoUnit, double kgtoUnit, double mstoUnit)
{
    QString strong, strange;

    planeOppProperties.clear();

    if     (m_WPolarType==xfl::FIXEDSPEEDPOLAR) strong += "Type 1 ("+QObject::tr("Fixed speed") +")\n";
    else if(m_WPolarType==xfl::FIXEDLIFTPOLAR)  strong += "Type 2 ("+QObject::tr("Fixed lift") +")\n";
    else if(m_WPolarType==xfl::FIXEDAOAPOLAR)   strong += "Type 4 ("+QObject::tr("Fixed angle of attack") +")\n";
    else if(m_WPolarType==xfl::BETAPOLAR)       strong += "Type 5 ("+QObject::tr("Beta range") +")\n";
    else if(m_WPolarType==xfl::STABILITYPOLAR)  strong += "Type 7 ("+QObject::tr("Stability analysis") +")\n";
    planeOppProperties += strong;

    //    WOppProperties += QObject::tr("Method")+" = ";
    if     (m_AnalysisMethod==xfl::LLTMETHOD)                         planeOppProperties +=QObject::tr("LLT");
    else if(m_AnalysisMethod==xfl::PANEL4METHOD && !m_bThinSurface)   planeOppProperties +=QObject::tr("3D-Panels");
    else if(m_AnalysisMethod==xfl::PANEL4METHOD && m_bVLM1)           planeOppProperties +=QObject::tr("3D-Panels/VLM1");
    else if(m_AnalysisMethod==xfl::PANEL4METHOD && !m_bVLM1)          planeOppProperties +=QObject::tr("3D-Panels/VLM2");
    planeOppProperties +="\n";


    if(m_bTiltedGeom) planeOppProperties += QObject::tr("Tilted geometry")+"\n";

    if(m_bOut) planeOppProperties += "Point is out of the flight envelope\n";

    strong  = QString(QObject::tr("VInf")+"  =%1 ").arg(m_QInf*mstoUnit,7,'f',3);
    planeOppProperties += "\n"+strong + speedUnitLabel+"\n";

    strong  = QString(QObject::tr("Alpha")+" =%1").arg(m_Alpha,7,'f',2);
    planeOppProperties += strong +QChar(0260)+"\n";

    strong  = QString(QObject::tr("Mass")+"  = %1 ").arg(m_Weight*kgtoUnit,7,'f',3);
    planeOppProperties += strong + massUnitLabel + "\n";

    if(qAbs(m_Beta)>PRECISION)
    {
        strong  = QString(QObject::tr("Beta")+"  = %1").arg(m_Beta,7,'f',2);
        planeOppProperties += strong +QChar(0260)+"\n\n";
    }

    if(m_WPolarType==xfl::STABILITYPOLAR)
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

    /*    if(m_nFlaps>0)
    {
        WingOppProperties += "\n";
        for(int ip=0; ip<m_nFlaps; ip++)
        {
            strong = QString("Flap Moment[%1] = %2").arg(ip+1).arg(m_FlapMoment[ip]*UnitsDlg::NmtoUnit(), 7,'g',3);
            UnitsDlg::GetMomentUnit(strange, UnitsDlg::momentUnit());
            WingOppProperties += strong + strange +"\n";
        }
    }*/


    if(m_WPolarType==xfl::STABILITYPOLAR)
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

        std::complex<double> c, angle;
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
                strange = QString::asprintf(   "  Time to double            = %8.3fs", log(2)/fabs(c.real()));
                planeOppProperties += strange +"\n";
                if(c.real()<0.0)
                {
                    strange = QString::asprintf("  Time constant              =%8.3f", -1.0/c.real());
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
        if(c.imag()>=0.0) strange = QString::asprintf("  Eigenvalue    = %9.5f+%9.5fi", m_phiPH.real(), m_phiPH.imag());
        else              strange = QString::asprintf("  Eigenvalue    = %9.5f-%9.5fi", m_phiPH.real(), m_phiPH.imag());
        planeOppProperties += strange +"\n";

        strange = QString::asprintf("     Undamped Natural Frequency = %8.3f Hz",OmegaN/2.0/PI);
        planeOppProperties += strange +"\n";

        strange = QString::asprintf("     Damped Natural Frequency   = %8.3f Hz",Omega1/2.0/PI);
        planeOppProperties += strange +"\n";

        strange = QString::asprintf("     Damping Ratio              = %8.3f ", Dsi);
        planeOppProperties += strange +"\n";


        modeProperties(m_phiDR, OmegaN, Omega1, Dsi);

        planeOppProperties += "Phillips Dutch-Roll eq. 28 JOURNAL OF AIRCRAFT Vol. 37, No. 3, May–June 2000\n";
        if(c.imag()>=0.0) strange = QString::asprintf("  Eigenvalue    = %9.5f+%9.5fi", m_phiDR.real(), m_phiDR.imag());
        else              strange = QString::asprintf("  Eigenvalue    = %9.5f-%9.5fi", m_phiDR.real(), m_phiDR.imag());
        planeOppProperties += strange +"\n";

        strange = QString::asprintf("     Undamped Natural Frequency = %8.3f Hz",OmegaN/2.0/PI);
        planeOppProperties += strange +"\n";

        strange = QString::asprintf("     Damped Natural Frequency   = %8.3f Hz",Omega1/2.0/PI);
        planeOppProperties += strange +"\n";

        strange = QString::asprintf("     Damping Ratio              = %8.3f ", Dsi);
        planeOppProperties += strange +"\n";
    }
}

