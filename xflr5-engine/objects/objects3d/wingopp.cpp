/****************************************************************************

    WingOpp Class
    Copyright (C) 2005-2016 Andre Deperrois 

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

#include "wingopp.h"

#include "plane.h"
#include "objects_global.h"
#include <objects/objects3d/wpolar.h>
#include <math.h>
#include <QTextStream>



/**
 * The public constructor.
 */
WingOpp::WingOpp(int PanelArraySize)
{
    m_AnalysisMethod = XFLR5::LLTMETHOD;

    m_NVLMPanels   = PanelArraySize;
    m_dCp = m_dG = m_dSigma = nullptr;

    m_bOut         = false;

    m_nWakeNodes     = 0;
    m_NXWakePanels   = 0;
    m_FirstWakePanel = 0.0;
    m_WakeFactor     = 1.0;

    m_Span    = 0.0;
    m_MAChord = 0.0;

    m_NStation     = 0;
    m_nFlaps       = 0;
    m_nControls    = 0;

    m_Alpha               = 0.0;
    m_QInf                = 0.0;
    m_Beta                = 0.0;
    m_Phi                 = 0.0;
    m_CL                  = 0.0;
    m_CY                  = 0.0;
    m_CX                  = 0.0;
    m_VCD                 = 0.0;
    m_ICD                 = 0.0;
    m_GCm = m_VCm = m_ICm = m_GRm = m_GYm = m_VYm = m_IYm = 0.0;
    m_CP.set(0.0,0.0,0.0);

    m_oldCtrl = 0.0;

    memset(m_Ai,            0, sizeof(m_Ai));
    memset(m_Twist,         0, sizeof(m_Twist));
    memset(m_Cl,            0, sizeof(m_Cl));
    memset(m_PCd,           0, sizeof(m_PCd));
    memset(m_ICd,           0, sizeof(m_ICd));
    memset(m_Cm,            0, sizeof(m_Cm));
    memset(m_CmAirf,        0, sizeof(m_CmAirf));
    memset(m_XCPSpanRel,    0, sizeof(m_XCPSpanRel));
    memset(m_XCPSpanAbs,    0, sizeof(m_XCPSpanAbs));
    memset(m_Chord,         0, sizeof(m_Chord));
    memset(m_SpanPos,       0, sizeof(m_SpanPos));
    memset(m_StripArea,     0, sizeof(m_StripArea));
    memset(m_Re,            0, sizeof(m_Re));
    memset(m_Twist,         0, sizeof(m_Twist));
    memset(m_XTrTop,        0, sizeof(m_XTrTop));
    memset(m_XTrBot,        0, sizeof(m_XTrBot));
    memset(m_BendingMoment, 0, sizeof(m_BendingMoment));
    memset(m_Vd,            0, sizeof(m_Vd));
    memset(m_F,             0, sizeof(m_F));

    memset(m_oldEigenValue, 0, sizeof(m_oldEigenValue)); //four longitudinal and four lateral modes
    memset(m_oldEigenVector, 0, sizeof(m_oldEigenVector));

    m_FlapMoment.clear();
}


/**
 * This course of action will lead us to destruction.
 */
WingOpp::~WingOpp()
{
}




/**
 * Exports the data of the WingOpp to a text file
 * @param out the instance of output QtextStream
 * @param FileType TXT if the data is separated by spaces, CSV for a comma separator
 * @return true if the export was successful, false otherwise.
 */
bool WingOpp::exportWOpp(QTextStream &out, bool bCSV)
{
    QString Header, strong, Format;
    int k;

    if(!bCSV) Header = "  y-span        Chord      Ai         Cl        PCd          ICd        CmGeom    CmAirf@chord/4    XTrtop    XTrBot      XCP       BM\n";
    else      Header = "  y-span,Chord,Ai,Cl,PCd,ICd,CmGeom,CmAirf@chord/4,XTrtop,XTrBot,XCP,BM\n";
    out << Header;

    int nStart;
    if(m_AnalysisMethod==XFLR5::LLTMETHOD) nStart = 1;
    else                                   nStart = 0;

    if(!bCSV) Format = "%1  %2   %3   %4   %5   %6   %7   %8    %9   %10   %11   %12\n";
    else      Format = "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12\n";
    for (k=nStart; k<m_NStation; k++)
    {
        strong = QString(Format)
            .arg(m_SpanPos[k],10,'f',4)
            .arg(m_Chord[k],9,'f',4)
            .arg(m_Ai[k],7,'f',3)
            .arg(m_Cl[k],9,'f',6)
            .arg(m_PCd[k],9,'f',6)
            .arg(m_ICd[k],9,'f',6)
            .arg(m_Cm[k],9,'f',6)
            .arg(m_CmAirf[k],9,'f',6)
            .arg(m_XTrTop[k],7,'f',4)
            .arg(m_XTrBot[k],7,'f',4)
            .arg(m_XCPSpanRel[k],7,'f',4)
            .arg(m_BendingMoment[k],7,'f',4);
        out << strong;
    }
    out << "\n\n";

    return true;
}


/**
 * Returns the maximum value of the local lift along the span.
 * Used to calibrate the display of the optimal elliptic curve in hte WingOpp graph.
 * @return the maximum local lift.
*/
double WingOpp::maxLift()
{
    int i,nStart;
    if(m_AnalysisMethod==XFLR5::LLTMETHOD) nStart = 1;
    else                                   nStart = 0;

    double maxlift = 0.0;
    for (i=nStart; i<m_NStation; i++)
    {
        if(m_Cl[i] * m_Chord[i]/m_MAChord>maxlift)
        {
            maxlift = m_Cl[i] * m_Chord[i]/m_MAChord;
        }
    }
    return maxlift;
}





/**
* Fills the operating point associated to a plane's wing with analysis results.
* Uses the wing's geometric data and the current instance of the LLT, VLM or Panel analysis dialog box.
*@param pWOpp  a pointer to the CWOpp object which is to be filled with the results of the analysis which has been completed
*@param to the wing object for which the CWOpp will be created
*/
void WingOpp::createWOpp(void *pWingPtr, void *pWPolarPtr)
{
    Wing *pWing = (Wing*)pWingPtr;
    WPolar *pWPolar = (WPolar*)pWPolarPtr;


    m_WingName            = pWing->m_WingName;
    m_NVLMPanels          = pWing->m_MatSize;
    m_NStation            = pWing->m_NStation;
    m_nFlaps              = pWing->m_nFlaps;

    m_PlrName             = pWPolar->polarName();
    m_AnalysisMethod      = pWPolar->analysisMethod();
    m_Beta                = pWPolar->m_BetaSpec;
    m_Phi                 = pWPolar->m_BankAngle;
    m_Weight              = pWPolar->mass();
    m_Span                = pWPolar->referenceSpanLength();

    m_MAChord             = pWing->m_MAChord;
    m_CL                  = pWing->m_WingCL;
    m_ICD                 = pWing->m_CDi;
    m_VCD                 = pWing->m_CDv;

    m_GCm                 = pWing->m_GCm;
    m_VCm                 = pWing->m_VCm;
    m_ICm                 = pWing->m_ICm;
    m_GRm                 = pWing->m_GRm;
    m_GYm                 = pWing->m_GYm;
    m_VYm                 = pWing->m_VYm;
    m_IYm                 = pWing->m_IYm;


    /**< @todo check if m_CP !=0 */
    m_CP                  = pWing->m_CP;

    double Cb =0.0;

    for(int l=0; l<pWing->m_NStation; l++)
    {
        m_Ai[l] =            pWing->m_Ai[l];
        m_Cl[l] =            pWing->m_Cl[l];
        m_PCd[l] =           pWing->m_PCd[l];
        m_ICd[l] =           pWing->m_ICd[l];
        m_Cm[l] =            pWing->m_Cm[l];
        m_CmAirf[l] =        pWing->m_CmPressure[l];
        m_XCPSpanRel[l] =    pWing->m_XCPSpanRel[l];
        m_XCPSpanAbs[l] =    pWing->m_XCPSpanAbs[l];
        m_Re[l] =            pWing->m_Re[l];
        m_Chord[l] =         pWing->m_Chord[l];
        m_Twist[l] =         pWing->m_Twist[l];
        m_XTrTop[l] =        pWing->m_XTrTop[l];
        m_XTrBot[l] =        pWing->m_XTrBot[l];
        m_BendingMoment[l] = pWing->m_BendingMoment[l];
        m_Vd[l] =            pWing->m_Vd[l];
        m_F[l] =             pWing->m_F[l];

        m_SpanPos[l]   = pWing->m_SpanPos[l];
        m_StripArea[l] = pWing->m_StripArea[l];
        Cb = qMax(Cb, pWing->m_BendingMoment[l]);
    }
    m_MaxBending = Cb;
}


/**
 * Loads or saves the data of this WingOpp to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WingOpp::serializeWingOppXFL(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;
    int k, n;
    double dble;

    if(bIsStoring)
    {
        ar << 200001;
        //200001
        //write variables
        ar << m_WingName;
        ar << m_PlrName;

        if(m_AnalysisMethod==XFLR5::LLTMETHOD)         ar<<1;
        else if(m_AnalysisMethod==XFLR5::VLMMETHOD)    ar<<2;
        else if(m_AnalysisMethod==XFLR5::PANEL4METHOD)  ar<<3;
        else ar << 2;

        ar << m_bOut;

        ar << m_NStation;
        ar << m_NVLMPanels;
        ar << m_nWakeNodes << m_NXWakePanels << m_FirstWakePanel << m_WakeFactor;

        ar << m_Alpha << m_Beta << m_QInf << m_Weight << m_Span << m_MAChord;

        ar << m_CL << m_VCD << m_ICD ;
        ar << m_CX << m_CY;

        ar << m_GCm << m_GRm << m_GYm;
        ar << m_VYm << m_IYm;

        ar << m_CP.x << m_CP.y << m_CP.z;

        for (k=0; k<m_NStation; k++)
        {
            ar << m_Re[k] << m_Chord[k] << m_Twist[k];
            ar << m_Ai[k] << m_Cl[k] << m_PCd[k] << m_ICd[k];
            ar << m_Cm[k] << m_CmAirf[k];
            ar << m_XCPSpanRel[k]<< m_XCPSpanAbs[k];
            ar << m_XTrTop[k] << m_XTrBot[k];
            ar << m_BendingMoment[k];
            ar << m_Vd[k].x << m_Vd[k].y << m_Vd[k].z;
            ar << m_F[k].x << m_F[k].y << m_F[k].z;
            ar << m_SpanPos[k] << m_StripArea[k];
        }

        if(m_AnalysisMethod!=XFLR5::LLTMETHOD)
        {
            ar << m_nFlaps;
            for(k=0; k<m_nFlaps; k++)
            {
                ar << m_FlapMoment[k];
            }
        }
        else
        {
            ar <<0;
        }

        // space allocation for the future storage of more data, without need to change the format
        for (int i=0; i<20; i++) ar << 0;
        for (int i=0; i<50; i++) ar << 0.0;
    }
    else
    {
        ar >> ArchiveFormat;

        ar >> m_WingName;
        ar >> m_PlrName;

        ar >> n;
        if(n==1)      m_AnalysisMethod=XFLR5::LLTMETHOD;
        else if(n==2) m_AnalysisMethod=XFLR5::VLMMETHOD;
        else if(n==3) m_AnalysisMethod=XFLR5::PANEL4METHOD;

        ar >> m_bOut;

        ar >> m_NStation;
        ar >> m_NVLMPanels;
        ar >> m_nWakeNodes >> m_NXWakePanels >> m_FirstWakePanel >> m_WakeFactor;

        ar >> m_Alpha >> m_Beta >> m_QInf >> m_Weight >> m_Span >> m_MAChord;

        ar >> m_CL >> m_VCD >> m_ICD;
        ar >> m_CX >> m_CY;

        ar >> m_GCm >> m_GRm >> m_GYm;
        ar >> m_VYm >> m_IYm;

        ar >> m_CP.x >> m_CP.y >> m_CP.z;

        for (k=0; k<m_NStation; k++)
        {
            ar >> m_Re[k] >> m_Chord[k] >> m_Twist[k];
            ar >> m_Ai[k] >> m_Cl[k] >> m_PCd[k] >> m_ICd[k];
            ar >> m_Cm[k] >> m_CmAirf[k];
            ar >> m_XCPSpanRel[k]>> m_XCPSpanAbs[k];
            ar >> m_XTrTop[k] >> m_XTrBot[k];
            ar >> m_BendingMoment[k];
            ar >> m_Vd[k].x >> m_Vd[k].y >> m_Vd[k].z;
            ar >> m_F[k].x >> m_F[k].y >> m_F[k].z;
            ar >> m_SpanPos[k] >> m_StripArea[k];
        }

        ar >> m_nFlaps;
        m_FlapMoment.clear();
        for(k=0; k<m_nFlaps; k++)
        {
            ar >> dble;
            m_FlapMoment.append(dble);
        }


        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}

/**
 * Loads or saves the data of this WingOpp to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool WingOpp::serializeWingOppWPA(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;
    int a,p,k,l,n;
    float f, f0, f1, f2, f3;

    if(bIsStoring)
    {
        // we don't store any more in wpa format
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1001|| ArchiveFormat>1100) return false;
        //read variables
        readCString(ar, m_WingName);
        readCString(ar, m_PlrName);

        ar >> a;
        if (a!=0 && a!=1) return false;

//        if(a) m_bIsVisible = true; else m_bIsVisible = false;
        ar >> a;
        if (a!=0 && a!=1) return false;
//        if(a) m_bShowPoints = true; else m_bShowPoints = false;

        ar >> a;
        if (a!=0 && a!=1) return false;

        if(a) m_bOut = true; else m_bOut = false;

        ar>>k;
        if(k==1)      m_AnalysisMethod=XFLR5::LLTMETHOD;
        else if(k==2) m_AnalysisMethod=XFLR5::VLMMETHOD;
        else if(k==3) m_AnalysisMethod=XFLR5::PANEL4METHOD;
        else if(k==4) m_AnalysisMethod=XFLR5::VLMMETHOD;
        else return false;

        if(ArchiveFormat>=1005)
        {
            ar >> a;
            if (a!=0 && a!=1) return false;

//            if(a) m_bVLM1 = true; else m_bVLM1 = false;

            ar >> a;
            if (a!=0 && a!=1) return false;
//            if(a) m_bThinSurface = true; else m_bThinSurface = false;
        }
        if(ArchiveFormat>=1013)
        {
            ar >> a;
            if (a!=0 && a!=1) return false;
//            if(a) m_bTiltedGeom = true; else m_bTiltedGeom = false;
        }

//        ar >> m_Style >> m_Width;
        ar>> k>>l;
        int r,g,b;
        readCOLORREF(ar,r,g,b);

        ar >>k;
/*        if(k==1)      m_WPolarType = FIXEDSPEEDPOLAR;
        else if(k==2) m_WPolarType = FIXEDLIFTPOLAR;
        else if(k==4) m_WPolarType = FIXEDAOAPOLAR;
        else if(k==6) m_WPolarType = STABILITYPOLAR; //former control polars
        else if(k==7) m_WPolarType = STABILITYPOLAR;
        else return false;*/

        ar >> m_NStation;
        ar >> f; m_Alpha =f;
        ar >> f; m_QInf =f;
        ar >> f; m_Weight =f;
        ar >> f; m_Span =f;
        ar >> f; m_MAChord =f;
        ar >> f; m_CL =f;
        ar >> f; m_VCD =f;
        ar >> f; m_ICD =f;
        if(ArchiveFormat>=1015)
        {
            ar >> f; m_Beta=f;
            ar >> f; m_CX =f;
            ar >> f; m_CY =f;
        }
        ar >> f; m_GCm =f;
        ar >> f; m_GRm =f;
        ar >> f; m_GYm =f;
        ar >> f; //m_VCm =f;
        ar >> f; m_VYm =f;
        ar >> f; m_IYm =f;

        if(ArchiveFormat<1014 && m_AnalysisMethod>XFLR5::LLTMETHOD)
        {
            m_GCm = m_GRm = m_GYm = m_VYm = m_IYm = 0.0;
        }

        ar >> f; m_CP.x =f;
        ar >> f; m_CP.y =f;

        for (k=0; k<m_NStation; k++)
        {
            ar >> f; m_Re[k] =f;
            ar >> f; m_Chord[k] =f;
            ar >> f; m_Twist[k] =f;
            ar >> f; m_Ai[k] =f;
            ar >> f; m_Cl[k] =f;
            ar >> f; m_PCd[k] =f;
            ar >> f; m_ICd[k] =f;
            ar >> f; m_Cm[k] =f;
            ar >> f; m_CmAirf[k] =f;
            ar >> f; //f=0.0;
            ar >> f; m_XCPSpanRel[k] =f;
            if(ArchiveFormat>=1007){ar >> f; m_XCPSpanAbs[k] =f;}
            ar >> f; m_XTrTop[k] =f;
            ar >> f; m_XTrBot[k] =f;
            if(ArchiveFormat>=1002)    {ar >> f; m_BendingMoment[k]=f;}
            else m_BendingMoment[k] = 0.0;
            if(ArchiveFormat>=1005)
            {
                ar >> f; m_Vd[k].x=f;
                ar >> f; m_Vd[k].y=f;
                ar >> f; m_Vd[k].z=f;
            }
            else
            {
                m_Vd[k].x = 0.0;
                m_Vd[k].y = 0.0;
                m_Vd[k].z = 0.0;
            }
            if(ArchiveFormat>=1006)
            {
                ar >> f; m_F[k].x=f;
                ar >> f; m_F[k].y=f;
                ar >> f; m_F[k].z=f;
            }
            else {
                m_F[k].x = 0.0;
                m_F[k].y = 0.0;
                m_F[k].z = 0.0;
            }
        }

        m_MaxBending = 0.0;
        for (k=0; k<m_NStation; k++)
        {
            m_MaxBending = qMax(m_MaxBending, m_BendingMoment[k]);
        }
        for (k=0; k<=m_NStation; k++)
        {
            ar >> f1;
            if(m_AnalysisMethod==XFLR5::LLTMETHOD  && ArchiveFormat<=1004) m_SpanPos[k] = -f1;
            else                                                    m_SpanPos[k] =  f1;
            if(ArchiveFormat>=1012)
            {
                ar >> f2;
                m_StripArea[k] = f2;
            }
            else m_StripArea[k] = 0.0;

        }
        if(ArchiveFormat>=1003)
        {
            ar>> m_NVLMPanels;

            if(ArchiveFormat<1023 || m_AnalysisMethod !=XFLR5::LLTMETHOD)
            {
                for (p=0; p<m_NVLMPanels;p++)
                {
                    ar >> f;// Cp =f; // not stored... Cp array is stored at PlaneOpp level
                }
            }
        }

        if(ArchiveFormat>=1009)
        {
            if(ArchiveFormat<1023 || m_AnalysisMethod !=XFLR5::LLTMETHOD)
            {
                for (p=0; p<m_NVLMPanels;p++)
                {
                    ar >> f;
//                    if(ArchiveFormat<1010) Gamma[p] =  f/1000.0;
                }
            }
        }
        if(ArchiveFormat>1010)
        {
            if(m_AnalysisMethod==XFLR5::PANEL4METHOD)
            {
                for (p=0; p<m_NVLMPanels;p++)
                {
                    ar >> f;
                }
            }
        }
        if(ArchiveFormat>=1004)
        {
            ar>> k; //m_WingType;
        }
        if(ArchiveFormat>=1008)
        {
            ar >> m_nWakeNodes >> m_NXWakePanels >> m_FirstWakePanel >> m_WakeFactor;
        }
        if(ArchiveFormat>=1010)
        {
            ar >> m_nFlaps;
            m_FlapMoment.clear();
            for(k=0; k<m_nFlaps; k++)
            {
                ar >> f;
                m_FlapMoment.append(f);
            }
        }

        if(ArchiveFormat<1010)
        {
            for(k=0; k<m_NStation; k++)
            {
                m_Chord[k]      /=1000.0;
                m_XCPSpanAbs[k] /=1000.0;
                m_SpanPos[k]    /=1000.0;
            }
            m_MAChord /=1000.0;
            m_Span    /=1000.0;
            m_CP.x    /=1000.0;
            m_CP.y    /=1000.0;
        }
        if(ArchiveFormat>=1016)
        {
            for(k=0; k<8;k++)
            {
                ar >> f1 >> f2;
                m_oldEigenValue[k] = complex<double>(f1, f2);

                for(l=0; l<4; l++)
                {
                    ar >> f1 >> f2;
                    m_oldEigenVector[k][l] = complex<double>(f1, f2);
                }
            }
        }
        if(ArchiveFormat>=1020)
        {
            // Non dimensional stability derivatives
            ar>>f; //  CXa= f;
            ar>>f; //  CXq= f;
            ar>>f; //  CXu= f;
            ar>>f; //  CZu= f;
            ar>>f; //  Cmu= f;
        }
        if(ArchiveFormat>=1017)
        {
            // Non dimensional stability derivatives
            ar>>f; //  CLa= f;
            ar>>f; //  CLq= f;
            ar>>f; //  Cma= f;
            ar>>f; //  Cmq= f;
            ar>>f; //  CYb= f;
            ar>>f; //  CYp= f;
            ar>>f; //  CYr= f;
            ar>>f; //  Clb= f;
            ar>>f; //  Clp= f;
            ar>>f; //  Clr= f;
            ar>>f; //  Cnb= f;
            ar>>f; //  Cnp= f;
            ar>>f; //  Cnr= f;
        }

        if(ArchiveFormat>=1018)
        {
            ar >> m_nControls;

            if(ArchiveFormat<1022) n = m_nControls;
            else                   n =1;
            for(k=0; k<n; k++)
            {
                ar >>f; //  if(k==0) CXe=f;
                ar >>f; //  if(k==0) CYe=f;
                ar >>f; //  if(k==0) CZe=f;
                ar >>f; //  if(k==0) CLe=f;
                ar >>f; //  if(k==0) CMe=f;
                ar >>f; //  if(k==0) CNe=f;

                ar >>f0>>f1>>f2>>f3;
//                if(k==0) m_BLat[0]= f0; m_BLat[1]= f1; m_BLat[2] = f2; m_BLat[3] = f3;
                ar >>f0>>f1>>f2>>f3;
//                m_BLong[0]=f0; m_BLong[1]=f1; m_BLong[2]= f2; m_BLong[3]= f3;
            }

            for(k=0; k<4; k++)
            {
                ar >>f0>>f1>>f2>>f3;
//                m_ALong[k][0]= f0; m_ALong[k][1]= f1; m_ALong[k][2]= f2; m_ALong[k][3] = f3;
                ar >>f0>>f1>>f2>>f3;
//                m_ALat[k][0] = f0; m_ALat[k][1] = f1; m_ALat[k][2] = f2; m_ALat[k][3] = f3;
            }
        }

        if(ArchiveFormat>=1019)
        {
            ar>>f;
//            m_XNP = f;
        }
//        else m_XNP = 0.0;

        if(ArchiveFormat>=1019)
        {
            ar>>f;    m_oldCtrl=f;

            //provision
            for(int i=1; i<20; i++)
            {
                ar>>f;
            }
            for(int i=0; i<20; i++)
            {
                ar>>k;
            }
        }
        if(ArchiveFormat>=1021)
        {
            ar >> f; m_CP.z=f;
        }


    }
    return true;
}

