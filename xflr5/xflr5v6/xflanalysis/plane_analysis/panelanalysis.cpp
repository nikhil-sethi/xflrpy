/****************************************************************************

    PanelAnalysis Class

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

#include <QElapsedTimer>
#include <QTime>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

#include <xflcore/matrix.h>
#include "panelanalysis.h"
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/body.h>
#include <xflobjects/objects3d/surface.h>



bool PanelAnalysis::s_bCancel = false;
bool PanelAnalysis::s_bWarning = false;
bool PanelAnalysis::s_bKeepOutOpp = false;
bool PanelAnalysis::s_bTrefftz = true;
int PanelAnalysis::s_MaxWakeIter = 1;


/**
* The public constructor
*/
PanelAnalysis::PanelAnalysis()
{
    m_nRHS = 0;
    s_MaxRHSSize = VLMMAXRHS;
    m_MaxMatSize = 0;


    m_Progress = m_TotalTime = 0.0;

    m_Ai = m_Cl = m_ICd = nullptr;
    m_F  = nullptr;
    m_Vd = nullptr;

    m_aij = m_aijWake = nullptr;
    m_uRHS = m_vRHS = m_wRHS = m_pRHS = m_qRHS = m_rRHS = nullptr;
    m_cRHS = m_uWake = m_wWake = nullptr;

    m_Index = nullptr;

    m_RHS = m_RHSRef = m_SigmaRef = m_Sigma = m_Mu = m_Cp = nullptr;
    m_3DQInf = nullptr;

    m_pWingList[0] = m_pWingList[1] = m_pWingList[2] = m_pWingList[3] = nullptr;

    m_pWPolar = nullptr;
    m_pPlane  = nullptr;
    m_ppSurface = nullptr;

    m_pPanel         = nullptr;
    m_pWakePanel     = nullptr;
    m_pRefWakePanel  = nullptr;
    m_pMemPanel      = nullptr;
    m_pNode          = nullptr;
    m_pMemNode       = nullptr;
    m_pWakeNode      = nullptr;
    m_pRefWakeNode   = nullptr;
    m_pTempWakeNode  = nullptr;

    m_Alpha   = 0.0;
    m_AlphaEq = 0.0;
    m_QInf    = 0.0;
    m_OpAlpha = 0.0;
    m_OpBeta  = 0.0;
    m_Ctrl    = 0.0;
    m_vMin = m_vMax = m_vDelta = 0.0;

    m_Mass = 0.0;
    m_CoG.set(0.0,0.0,0.0);
    m_Inertia[0]=m_Inertia[1]=m_Inertia[2]=m_Inertia[3]=0.0;

    m_bSequence      = false;

    m_MatSize = m_nNodes = 0;
    m_CL  = m_CX  = m_CY  = 0.0;
    m_GCm = m_GRm = m_GYm = m_VCm = m_ICm = m_VYm = m_IYm = 0.0;
    m_CP.set( 0.0, 0.0, 0.0);
    m_ViscousDrag = m_InducedDrag = 0.0;

    m_MatSize        = 0;
    m_nNodes         = 0;
    m_NWakeColumn    = 0;

    m_nWakeNodes = 0;
    m_WakeSize   = 0;

    Theta0 = 0.0;
    u0     = 0.0;

    //Dimensional stability derivatives
    Xu = Xw = Zu = Zw = Zq = Mu = Mw = Mq = Zwp = Mwp = 0.0;
    Yv = Yp = Yr = Lv = Lp = Lr = Nv = Np = Nr = 0.0;

    // Non dimensional stability derivatives
    CXu = CZu = Cmu = CXq = CZq = Cmq = CXa = CZa = Cma = 0.0;
    CYb = CYp = CYr = Clb = Clp = Clr = Cnb = Cnp = Cnr = 0.0;
    CXe = CYe = CZe = Cle = Cme = Cne = 0.0;

    memset(m_ALong, 0, 16*sizeof(double));
    memset(m_ALat,  0, 16*sizeof(double));
    memset(m_BLong, 0,  4*sizeof(double));
    memset(m_BLat,  0,  4*sizeof(double));
    memset(m_R,     0,  9*sizeof(double));
    memset(m_Is,    0,  9*sizeof(double));
    memset(m_Ib,    0,  9*sizeof(double));
    memset(m_rLong, 0,  8*sizeof(double));
    memset(m_rLat,  0,  8*sizeof(double));
    memset(m_vLong, 0, 32*sizeof(double));
    memset(m_vLat,  0, 32*sizeof(double));
}


/**
 * The public destructor.
 *
 */
PanelAnalysis::~PanelAnalysis()
{
    releaseArrays();
    delete [] m_Ai;
    delete [] m_Cl;
    delete [] m_ICd;
    delete [] m_F;
    delete [] m_Vd;
    /*    for (int i=m_PlaneOppList.size()-1; i>=0; i--)
    {
        PlaneOpp *pObj = m_PlaneOppList.at(i);
        m_PlaneOppList.removeAt(i);
        delete pObj;
    }*/
}


/**
 * Reserves the memory necessary to matrix arrays.
 *@return true if the memory could be allocated, false otherwise.
 */
bool PanelAnalysis::allocateMatrix(int matSize, int &memsize)
{
    QString strange;

    if(matSize<=m_MaxMatSize) return true;  //current analysis requires smaller size than that currently allocated

    releaseArrays();

    //    Trace("PanelAnalysis::Allocating matrix arrays");

    int size2 = matSize * matSize;
    try
    {
        m_aij      = new double[ulong(size2)];
        m_aijWake  = new double[ulong(size2)];

        m_uRHS  = new double[ulong(matSize)];
        m_vRHS  = new double[ulong(matSize)];
        m_wRHS  = new double[ulong(matSize)];
        m_pRHS  = new double[ulong(matSize)];
        m_qRHS  = new double[ulong(matSize)];
        m_rRHS  = new double[ulong(matSize)];
        m_cRHS  = new double[ulong(matSize)];
        m_uWake = new double[ulong(matSize)];
        m_wWake = new double[ulong(matSize)];

        m_uVl.resize(matSize);
        m_wVl.resize(matSize);
        m_Index = new int[ulong(matSize)];
    }
    catch(std::exception & )
    {
        releaseArrays();
        m_MaxMatSize = 0;
        //        Trace(e.what());
        strange = "Memory allocation error: the request for additional memory has been denied.\nPlease reduce the model's size.";
        //        Trace(strange);
        return false;
    }

    m_MaxMatSize = matSize;

    memsize  = int(sizeof(double))  * 2 * size2; //bytes
    memsize += int(sizeof(double))  * 9 * matSize; //bytes
    memsize += int(sizeof(Vector3d)) * 3 * matSize;
    memsize += int(sizeof(int))     * 1 * matSize;

    strange = QString("PanelAnalysis::Memory allocation for the matrix arrays is %1 MB").arg(double(memsize)/1024./1024., 7, 'f', 2);
    //    Trace(strange);

    memset(m_aij,     0, uint(size2) * sizeof(double));
    memset(m_aijWake, 0, uint(size2) * sizeof(double));

    memset(m_uRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_vRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_wRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_pRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_qRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_rRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_cRHS,  0, ulong(matSize)*sizeof(double));
    memset(m_uWake, 0, ulong(matSize)*sizeof(double));
    memset(m_wWake, 0, ulong(matSize)*sizeof(double));

    m_uVl.fill(Vector3d());
    m_wVl.fill(Vector3d());

    memset(m_Index, 0, ulong(matSize)*sizeof(int));

    int RHSSize = 0;

    if(!allocateRHS(matSize, RHSSize))
    {
        strange = "Memory allocation error: the request for additional memory has been denied.\nPlease educe the model's size.";
        traceLog(strange);
        return false;
    }

    memsize += RHSSize;

    strange = QString("PanelAnalysis::Memory allocation for the analysis arrays is %1 MB").arg(double(memsize)/1024./1024., 7, 'f', 2);
    //    Trace(strange);

    return true;
}


/**
 * Reserves the memory necessary to RHS arrays.
 *@return true if the memory could be allocated, false otherwise.
 */
bool PanelAnalysis::allocateRHS(int matSize, int &memsize)
{
    //    Trace("PanelAnalysis::Allocating RHS arrays");
    uint size = uint(matSize * s_MaxRHSSize);

    if(size==0) return false;

    try
    {
        m_RHS      = new double[size];
        m_RHSRef   = new double[size];
        m_SigmaRef = new double[size];
        m_Sigma    = new double[size];
        m_Mu       = new double[size];
        m_Cp       = new double[size];

        m_3DQInf   = new double[uint(s_MaxRHSSize)];
    }
    catch(std::exception &)
    {
        releaseArrays();
        //        Trace(e.what());
        return false;
    }

    memsize = int(sizeof(double) * 6 * size);

    memset(m_RHS,       0, size*sizeof(double));
    memset(m_RHSRef,    0, size*sizeof(double));
    memset(m_Sigma,     0, size*sizeof(double));
    memset(m_SigmaRef,  0, size*sizeof(double));
    memset(m_Mu,        0, size*sizeof(double));
    memset(m_Cp,        0, size*sizeof(double));

    memset(m_3DQInf, 0, uint(s_MaxRHSSize)*sizeof(double));

    //    QString strange = QString("PanelAnalysis::Memory allocation for the RHS arrays is %1 MB").arg((double)memsize/1024./1024., 7, 'f', 2);
    //    Trace(strange);


    return true;
}


/**
 * Releases the memory reserved for matrix and RHS arrays
 */
void PanelAnalysis::releaseArrays()
{
    if(m_aij)     delete [] m_aij;
    if(m_aijWake) delete [] m_aijWake;
    m_aij = m_aijWake = nullptr;

    if(m_RHS)      delete [] m_RHS;
    if(m_RHSRef)   delete [] m_RHSRef;
    if(m_SigmaRef) delete [] m_SigmaRef;
    if(m_Sigma)    delete [] m_Sigma;
    if(m_Mu)       delete [] m_Mu;
    if(m_Cp)       delete [] m_Cp;
    m_RHS = m_RHSRef = m_SigmaRef = m_Sigma = m_Mu = m_Cp = nullptr;

    if(m_3DQInf) delete [] m_3DQInf;
    m_3DQInf = nullptr;

    if(m_uRHS)  delete [] m_uRHS;
    if(m_vRHS)  delete [] m_vRHS;
    if(m_wRHS)  delete [] m_wRHS;
    if(m_rRHS)  delete [] m_rRHS;
    if(m_pRHS)  delete [] m_pRHS;
    if(m_qRHS)  delete [] m_qRHS;
    if(m_cRHS)  delete [] m_cRHS;
    if(m_uWake) delete [] m_uWake;
    if(m_wWake) delete [] m_wWake;
    m_uRHS = m_vRHS = m_wRHS = m_pRHS = m_qRHS = m_rRHS = m_cRHS = m_uWake = m_wWake = nullptr;

    if(m_Index) delete [] m_Index;
    m_Index = nullptr;

    m_MaxMatSize = 0;
}


void PanelAnalysis::setObjectPointers(Plane *pPlane, QVector<Surface*>*pSurfaceList)
{
    m_pPlane    = pPlane;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        m_pWingList[iw] = m_pPlane->wing(iw);
    }

    m_ppSurface = pSurfaceList;
}


void PanelAnalysis::setRange(double vMin, double vMax, double vDelta, bool bSequence)
{
    if(vMax<vMin) vDelta = -qAbs(vDelta);

    m_bSequence = bSequence;
    m_vMin= vMin;
    m_vMax = vMax;
    m_vDelta = vDelta;

    m_nRHS = int(qAbs((m_vMax-m_vMin)*1.0001/m_vDelta) +1);

    if(!m_bSequence) m_nRHS = 1;
    else if(m_nRHS>=VLMMAXRHS)
    {
        QString strange = QString("The number of points to be calculated will be limited to %1\n\n").arg(VLMMAXRHS);
        traceLog(strange);
        m_nRHS = VLMMAXRHS-1;
        s_MaxRHSSize = int(double(m_nRHS) * 1.2);
    }


    //ESTIMATED UNIT TIMES FOR OPERATIONS

    if(m_pWPolar->bTilted() || m_pWPolar->polarType()==xfl::BETAPOLAR)
    {

        //ESTIMATED UNIT TIMES FOR OPERATIONS
        //BuildInfluenceMatrix :     10 x MatSize/400
        //CreateRHS :                10
        //CreateWakeContribution :    1
        //SolveUnitRHS :            400 x MatSize/400
        //ComputeFarField :          10 x MatSize/400x nrhs
        //ComputeOnBodyCp :           1 x nrhs
        //RelaxWake :                20 x nrhs x MaxWakeIter *
        //ComputeAeroCoefs :          5 x nrhs

        m_TotalTime      =  10.*double(m_MatSize)/400.
                +  10.
                + 400.*double(m_MatSize)/400.
                +  10.*double(m_MatSize)/400
                +   1.
                +   5.;

        m_TotalTime *= double(m_nRHS);

    }
    else if(m_pWPolar->polarType()<xfl::FIXEDAOAPOLAR)
    {
        //BuildInfluenceMatrix :     10 x MatSize/400
        //CreateRHS :                10
        //CreateWakeContribution :    1
        //SolveUnitRHS :            400 x MatSize/400
        //ComputeFarField :          10 x MatSize/400x nrhs
        //ComputeOnBodyCp :           1 x nrhs
        //RelaxWake :                20 x nrhs x MaxWakeIter *
        //ComputeAeroCoefs :          5 x nrhs
        int MaxWakeIter = 1;

        if(!m_pWPolar->bThinSurfaces()) m_TotalTime +=1.0; //for wake contribution

        if(m_pWPolar->bWakeRollUp()) m_TotalTime += 20*m_nRHS*MaxWakeIter;

        m_TotalTime      = 10.0*double(m_MatSize)/400.
                + 10.
                + 400.*double(m_MatSize)/400.
                + 10*double(m_MatSize)/400*double(m_nRHS)
                + 1*double(m_nRHS)
                + 5*double(m_nRHS) ;
    }
    else if(m_pWPolar->polarType()==xfl::FIXEDAOAPOLAR)
    {
        //BuildInfluenceMatrix :     10 x m_MatSize/400
        //CreateRHS :                10
        //CreateWakeContribution :    1
        //SolveUnitRHS :            400 x MatSize/400
        //ComputeFarField :          10 x MatSize/400x 1
        //ComputeOnBodyCp :           1 x nrhs
        //RelaxWake :                20 x nrhs x MaxWakeIter *
        //ComputeAeroCoefs :          5 x nrhs

        m_TotalTime      = 10.0*double(m_MatSize)/400.
                + 10.
                + 400.*double(m_MatSize)/400.
                + 10*double(m_MatSize)/400*1.
                + 1*double(m_nRHS)
                + 5*double(m_nRHS) ;
    }
    else if(m_pWPolar->polarType()==xfl::STABILITYPOLAR)
    {
        if(!m_bSequence) m_nRHS = 1;
        else if(m_nRHS==0) m_nRHS = 1;//compute at least nominal control positions, even if none is active nor defined

        m_TotalTime      = 10.0*double(m_MatSize)/400.       //BuildInfluenceMatrix
                + 10.                               //CreateRHS
                + 400.*double(m_MatSize)/400.       //SolveUnitRHS
                + 10*double(m_MatSize)/400          //ComputeFarField
                + 2+5*6                             //ComputeStabDer
                + 1                                 //ComputeOnBodyCp
                + 5;                                //ComputeAeroCoefs
        m_TotalTime *= double(m_nRHS);
    }
}


void PanelAnalysis::setArrayPointers(Panel *pPanel, Panel *pMemPanel, Panel *pWakePanel, Panel *pRefWakePanel,
                                     Vector3d *pNode,  Vector3d *pMemNode,  Vector3d *pWakeNode,  Vector3d *pRefWakeNode, Vector3d *pTempWakeNode)
{
    m_pPanel        = pPanel;
    m_pMemPanel     = pMemPanel;
    m_pWakePanel    = pWakePanel;
    m_pRefWakePanel = pRefWakePanel;

    m_pNode         = pNode;
    m_pMemNode      = pMemNode;
    m_pWakeNode     = pWakeNode;
    m_pRefWakeNode  = pRefWakeNode;
    m_pTempWakeNode = pTempWakeNode;
}


void PanelAnalysis::setArraySize(int MatSize, int WakeSize, int nNodes, int nWakeNodes, int NWakeColumn)
{
    m_MatSize     = MatSize;
    m_WakeSize    = WakeSize;
    m_nNodes      = nNodes;
    m_nWakeNodes  = nWakeNodes;
    m_NWakeColumn = NWakeColumn;
}


/**
* Initializes the analysis
*/
bool PanelAnalysis::initializeAnalysis()
{
    if(!m_pPlane) return false;
    s_bCancel = false;

    QString strange;

    strange = "Launching the 3D Panel Analysis....\n";
    traceLog(strange);

    traceLog(m_pPlane->name()+"\n");

    if(m_pWPolar->isFixedSpeedPolar())     strange = "Type 1 - Fixed speed polar";
    else if(m_pWPolar->isFixedLiftPolar()) strange = "Type 2 - Fixed lift polar";
    else if(m_pWPolar->isFixedaoaPolar())  strange = "Type 4 - Fixed angle of attack polar";
    else if(m_pWPolar->isBetaPolar())      strange = "Type 5 - Sideslip variation polar";
    else if(m_pWPolar->isStabilityPolar()) strange = "Type 7 - Stability polar";
    traceLog(strange+"\n\n");

    if(m_pWPolar->bThinSurfaces())
    {
        strange = "Wings as thin surfaces";
        traceLog(strange+"\n");

        if(m_pWPolar->bVLM1()) strange = "Using horseshoe vortices- VLM1";
        else                   strange = "Using ring vortices - VLM2";
        traceLog(strange+"\n");

        strange = "Using Neumann boundary conditions for wings";
        traceLog(strange+"\n");
    }
    else
    {
        strange = "Wings as thick surfaces";
        traceLog(strange+"\n");
        if(m_pWPolar->boundaryCondition()==xfl::DIRICHLET) strange = "Using Dirichlet boundary conditions for wings";
        else                                               strange = "Using Neumann boundary conditions for wings";
        traceLog(strange+"\n");
    }

    if(m_pPlane->body())
    {
        if(m_pWPolar->bDirichlet())  strange = "Using Dirichlet boundary conditions for the body";
        else                         strange = "Using Neumann boundary conditions for the body";
        traceLog(strange+"\n");
    }
    traceLog("\n");

    strange = QString::fromUtf8("Density   = %1kg/m3").arg(m_pWPolar->density(), 11, 'g', 5);
    traceLog(strange+"\n");
    strange = QString::fromUtf8("Viscosity = %1m²/s").arg(m_pWPolar->viscosity(), 11, 'g', 5);
    traceLog(strange+"\n\n");

    // make sure the polar is up to date with the latest plane data;
    // should have been updated at the time when the polar was set
    if(m_pWPolar->bAutoInertia())
    {
        m_pWPolar->setMass(m_pPlane->totalMass());
        m_pWPolar->setCoG(m_pPlane->CoG());
        m_pWPolar->setCoGIxx(m_pPlane->CoGIxx());
        m_pWPolar->setCoGIyy(m_pPlane->CoGIyy());
        m_pWPolar->setCoGIzz(m_pPlane->CoGIzz());
        m_pWPolar->setCoGIxz(m_pPlane->CoGIxz());
    }

    if(m_pWPolar->referenceDim()!=xfl::MANUALREFDIM)
    {
        if(m_pWPolar->referenceDim()==xfl::PLANFORMREFDIM)
        {
            m_pWPolar->setReferenceArea(m_pPlane->planformArea());
            m_pWPolar->setReferenceSpanLength(m_pPlane->planformSpan());
        }
        else if(m_pWPolar->referenceDim()==xfl::PLANFORMREFDIM)
        {
            m_pWPolar->setReferenceArea(m_pPlane->projectedArea());
            m_pWPolar->setReferenceSpanLength(m_pPlane->projectedSpan());
        }
    }
    strange = QString::fromUtf8("Reference Area   = %1m²").arg(m_pWPolar->referenceArea(), 11, 'g', 5);
    traceLog(strange+"\n");
    strange = QString::fromUtf8("Reference length = %1m").arg(m_pWPolar->referenceSpanLength(), 11, 'g', 5);
    traceLog(strange+"\n\n");

    m_NSpanStations = 0;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingList[iw] != nullptr) m_NSpanStations += m_pWingList[iw]->m_NStation;
    }

    m_PlaneOppList.clear();

    if(m_Ai)  delete [] m_Ai;
    if(m_Cl)  delete [] m_Cl;
    if(m_ICd) delete [] m_ICd;
    if(m_F)   delete [] m_F;
    if(m_Vd)  delete [] m_Vd;
    m_Ai   = new double[ MAXWINGS*VLMMAXRHS * uint(m_NSpanStations)];
    m_Cl   = new double[ MAXWINGS*VLMMAXRHS * uint(m_NSpanStations)];
    m_ICd  = new double[ MAXWINGS*VLMMAXRHS * uint(m_NSpanStations)];
    m_F    = new Vector3d[ MAXWINGS*VLMMAXRHS * uint(m_NSpanStations)];
    m_Vd   = new Vector3d[ MAXWINGS*VLMMAXRHS * uint(m_NSpanStations)];

    m_Progress = 0.0;

    m_bPointOut = false;
    s_bCancel   = false;
    s_bWarning  = false;

    QString str = QString("Counted %1 panel elements\n").arg(m_MatSize,4);
    traceLog(str+"\n");

    /*    for (int p=0; p<m_MatSize; p++)
    {
        Panel const &p4 = m_pPanel[p];
        qDebug("index=%3d   iPU=%3d   iPD=%3d   iPL=%3d   iPR=%3d", p4.m_iElement, p4.m_iPU, p4.m_iPD, p4.m_iPL, p4.m_iPR);
    }*/

    return true;
}


bool PanelAnalysis::loop()
{
    if(m_pWPolar->polarType()<xfl::FIXEDAOAPOLAR)
    {
        if(m_pWPolar->bTilted() || fabs(m_pWPolar->Beta())>PRECISION) return unitLoop();
        else                                                          return alphaLoop();
    }
    else if(m_pWPolar->polarType()==xfl::FIXEDAOAPOLAR)
    {
        if(m_pWPolar->bTilted() || fabs(m_pWPolar->Beta())>PRECISION) return unitLoop();
        else                                                          return QInfLoop();
    }
    else if(m_pWPolar->polarType()==xfl::BETAPOLAR)
    {
        return unitLoop();
    }
    else if(m_pWPolar->polarType()==xfl::STABILITYPOLAR)
    {
        return controlLoop();
    }

    restorePanels();

    return false;
}


/**
* Launches a calculation over the input sequence of aoa.
* Used for type 1 & 2 analysis, without tilted geometry.
*
* The calculation is performed for two unit RHS, and all the Operating POints are calculated by linear combination.
* The two unit RHS are for a unit velocity along the x-axis, and for a unit velocity along the z-axis.
*@return true if all the aoa were computed successfully, false otherwise. Interpolation issues are not counted as unsuccessful.
*/
bool PanelAnalysis::alphaLoop()
{
    QString str;
    if(!m_bSequence) m_nRHS = 1;

    setInertia(0.0, 0.0, 0.0);

    m_Progress = 0.0;

    str = QString("   Solving the problem... \n");
    traceLog(str);

    buildInfluenceMatrix();
    if (s_bCancel) return true;
    //display_vec(m_aij, 2*m_MatSize);

    createUnitRHS();
    if (s_bCancel) return true;

    if(!m_pWPolar->bThinSurfaces())
    {
        //compute wake contribution
        createWakeContribution();
        //        display_vec(m_aijWake+17*m_MatSize, m_MatSize);

        //add wake contribution to matrix and RHS
        for(int p=0; p<m_MatSize; p++)
        {
            m_uRHS[p]+= m_uWake[p];
            m_wRHS[p]+= m_wWake[p];
            for(int pp=0; pp<m_MatSize; pp++)
            {
                m_aij[p*m_MatSize+pp] += m_aijWake[p*m_MatSize+pp];
            }
        }
    }
    //display_vec(m_aijWake, 2*m_MatSize);
    if (s_bCancel) return true;

    if (!solveUnitRHS())
    {
        s_bWarning = true;
        return true;
    }
    //for(int i=0; i<m_MatSize; i++) displayDouble(m_uRHS[i], m_wRHS[i]);

    if (s_bCancel) return true;

    createSourceStrength(m_vMin, m_vDelta, m_nRHS);
    if (s_bCancel) return true;

    createDoubletStrength(m_vMin, m_vDelta, m_nRHS);
    if (s_bCancel) return true;

    computeFarField(1.0, m_vMin, m_vDelta, m_nRHS);
    if (s_bCancel) return true;

    for(int q=0; q<m_nRHS; q++)
        computeBalanceSpeeds(m_vMin+q*m_vDelta, q);

    scaleResultstoSpeed(m_nRHS);
    if (s_bCancel) return true;

    computeOnBodyCp(m_vMin, m_vDelta, m_nRHS);
    if (s_bCancel) return true;
    //for(int i=0; i<m_MatSize; i++)    displayDouble(m_Cp[i]);

    computeAeroCoefs(m_vMin, m_vDelta, m_nRHS);

    return true;
}



/**
* Builds the influence matrix, both for VLM or Panel calculations.
*/
void PanelAnalysis::buildInfluenceMatrix()
{
    Vector3d C, CC, V;
    int m, mm, p, pp;
    double phi;

    int Size = m_MatSize;
    //    if(m_b3DSymetric) Size = m_SymSize;

    traceLog("      Creating the influence matrix...");
    traceLog("\n");
    m=0;
    for(p=0; p<m_MatSize; p++)
    {
        if(s_bCancel) return;
        //        if(!m_b3DSymetric || m_pPanel[p].m_bIsLeftPanel)
        //        {
        //for each Boundary Condition point
        if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE)
        {
            //Thick surfaces, 3D-panel type BC, use collocation point
            C = m_pPanel[p].CollPt;
        }
        else
        {
            //Thin surface, VLM type BC, use control point
            C = m_pPanel[p].CtrlPt;
        }
        CC.x =  C.x;//symmetric point, just in case
        CC.y = -C.y;
        CC.z =  C.z;

        mm = 0;
        for(pp=0; pp<m_MatSize; pp++)
        {
            if(s_bCancel) return;
            //                if(!m_b3DSymetric || m_pPanel[pp].m_bIsLeftPanel)
            //                {
            //for each panel, get the unit doublet or vortex influence at the boundary condition pt
            getDoubletInfluence(C, m_pPanel+pp, V, phi);

            /*                    if(m_b3DSymetric && m_pPanel[pp].m_iSym>=0) // add symmetric contribution
                    {
                        GetDoubletInfluence(CC, m_pPanel+pp, VS, phiSym);
                        V.x += VS.x;
                        V.y -= VS.y;
                        V.z += VS.z;
                        phi += phiSym;
                    }*/

            if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE) m_aij[m*Size+mm] = V.dot(m_pPanel[p].Normal);
            else if(m_pWPolar->bDirichlet())                              m_aij[m*Size+mm] = phi;

            mm++;
            //                }
        }
        m++;
        //        }
        m_Progress += 10.0*double(Size)/400./double(Size);
    }
}


/**
 * Creates the source strengths for all requested RHS in a Panel analysis, using the specified boundary conditions (BC).
 * BC may be of the Neumann or Dirichlet type depending on the analysis type and on the geometry
 *
 * Uses NASA 4023 equation (20) & (22)
 *
 * The computation is performed for a unit speed. The results are scaled to speed later depending on the polar type.
 *
 *@param Alpha0 the first aoa in the sequence
 *@param AlphaDeltathe aoa increment
 *@param the total number of aoa and of source arrays to build
*/
void PanelAnalysis::createSourceStrength(double Alpha0, double AlphaDelta, int nval)
{
    int p, pp, q;
    double alpha;
    Vector3d WindDirection;

    traceLog("      Creating source strengths...\n");

    p=0;
    for (q=0; q<nval;q++)
    {
        alpha = Alpha0+q*AlphaDelta;
        WindDirection.set(cos(alpha*PI/180.0), 0.0, sin(alpha*PI/180.0));

        for (pp=0; pp<m_MatSize; pp++)
        {
            if(s_bCancel) return;
            if(m_pPanel[pp].m_Pos!=xfl::MIDSURFACE) m_Sigma[p] = -1.0/4.0/PI* WindDirection.dot(m_pPanel[pp].Normal);
            else                               m_Sigma[p] =  0.0;
            p++;
        }
    }
}


/**
* Creates a RHS vector array for a given field of velocity vectors.
* If the input pointer VField is NULL, a uniform field of freestream velocities is considered.
* If the pointer is not NULL, it points to an array of velocity vectors on the panels. Used for evaluating the stability derivatives for the rotation dof.
* @param RHS A pointer to the RHS to build
* @param VField a pointer to the array of the freesteam velocity vectors on the panels.
* @param VInf the freestream velocity vector
*/
void PanelAnalysis::createRHS(double *RHS, Vector3d VInf, double *VField)
{
    double  phi=0, sigmapp=0;
    Vector3d V, C, VPanel;

    int m = 0;

    for (int p=0; p<m_MatSize; p++)
    {
        if(s_bCancel) return;
        if(VField)
        {
            VPanel.x = *(VField             +p);
            VPanel.y = *(VField+  m_MatSize +p);
            VPanel.z = *(VField+2*m_MatSize +p);
        }
        else VPanel = VInf;

        //        if(!m_b3DSymetric || m_pPanel[p].m_bIsLeftPanel)
        //        {
        if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
        {
            // first term of RHS is -V.n
            RHS[m] = - m_pPanel[p].Normal.dot(VPanel);
            //                if(RHS[m]<PRECISION) RHS[m] = 0.0;
        }
        else if(m_pWPolar->bDirichlet()) RHS[m] = 0.0;
        if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE) C = m_pPanel[p].CollPt;
        else                              C = m_pPanel[p].CtrlPt;

        for (int pp=0; pp<m_MatSize; pp++)
        {
            // Consider only the panels positioned on thick surfaces,
            // since the source strength is zero on thin surfaces
            if(m_pPanel[pp].m_Pos!=xfl::MIDSURFACE)
            {
                // Define the source strength on panel pp
                sigmapp = -1.0/4.0/PI * m_pPanel[pp].Normal.dot(VPanel);

                // Add to RHS the source influence of panel pp on panel p
                getSourceInfluence(C, m_pPanel+pp, V, phi);

                if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                {
                    // Apply Neumann B.C.
                    // NASA4023 eq. (22) and (23)
                    // The RHS term is sigma[pp]*DJK = nj.Vjk
                    RHS[m] -= V.dot(m_pPanel[p].Normal) * sigmapp;
                }
                else if(m_pWPolar->bDirichlet())
                {
                    //NASA4023 eq. (20)
                    RHS[m] -= (phi * sigmapp);
                }
            }
        }
        m++;
        //        }
        m_Progress += 5.0/double(m_MatSize);
    }
}


/**
*Builds the two unit RHS for freestream velocities directed along x and z
*/
void PanelAnalysis::createUnitRHS()
{
    traceLog("      Creating the unit RHS vectors...\n");

    Vector3d VInf;

    VInf.set(1.0, 0.0, 0.0);
    createRHS(m_uRHS,  VInf);

    VInf.set(0.0, 0.0, 1.0);
    createRHS(m_wRHS, VInf);
}


/**
* In the case of a panel analysis, adds the contribution of the wake columns to the coefficients of the influence matrix
* Method :
*     - follow the method described in NASA 4023 eq. (44)
*    - add the wake's doublet contribution to the matrix
*    - add the difference in potential at the trailing edge panels to the RHS
* Only a flat wake is considered. Wake roll-up has been tested but did not prove robust enough for implementation.
*/
void PanelAnalysis::createWakeContribution()
{
    int kw=0, lw=0, pw=0, p=0, pp=0, Size=0;

    Vector3d V, C, CC, TrPt;
    double phi=0;
    QVector<double> PHC(m_NWakeColumn);
    QVector<Vector3d> VHC(m_NWakeColumn);

    traceLog("      Adding the wake's contribution...\n");

    Size = m_MatSize;
    //    if(m_b3DSymetric) Size = m_SymSize;

    int m(0), mm(0);

    m=0;
    for(p=0; p<m_MatSize; p++)
    {
        if(s_bCancel) return;
        //        if(!m_b3DSymetric || m_pPanel[p].m_bIsLeftPanel)
        {
            m_uWake[m] = m_wWake[m] = 0.0;
            C    = m_pPanel[p].CollPt;
            CC.x =  C.x;//symmetric point, just in case
            CC.y = -C.y;
            CC.z =  C.z;

            //____________________________________________________________________________
            //build the contributions of each wake column at point C
            //we have m_NWakeColum to consider
            pw=0;
            for (kw=0; kw<m_NWakeColumn; kw++)
            {
                PHC[kw] = 0.0;
                VHC[kw].set(0.0,0.0,0.0);
                //each wake column has m_NXWakePanels
                for(lw=0; lw<m_pWPolar->m_NXWakePanels; lw++)
                {
                    getDoubletInfluence(C, m_pWakePanel+pw, V, phi, true, true);

                    PHC[kw] += phi;
                    VHC[kw] += V;

                    pw++;
                }
            }

            //____________________________________________________________________________
            //Add the contributions of the trailing panels to the matrix coefficients and to the RHS
            mm = 0;
            for(pp=0; pp<m_MatSize; pp++) //for each matrix column
            {
                if(s_bCancel) return;
                m_aijWake[m*Size+mm] = 0.0;
                // Is the panel pp shedding a wake ?
                if(m_pPanel[pp].m_bIsTrailing)
                {
                    // If so, we need to add the contributions of the wake column
                    // shedded by this panel to the RHS and to the Matrix
                    // Get trailing point where the jup in potential is evaluated v6.02
                    TrPt = (m_pNode[m_pPanel[pp].m_iTA] + m_pNode[m_pPanel[pp].m_iTB])/2.0;

                    if(m_pPanel[pp].m_Pos==xfl::MIDSURFACE)
                    {
                        //The panel shedding a wake is on a thin surface
                        if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                        {
                            //then add the velocity contribution of the wake column to the matrix coefficient
                            m_aijWake[m*Size+mm] += VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                            //we do not add the term Phi_inf_KWPUM - Phi_inf_KWPLM (eq. 44) since it is 0, thin edge
                        }
                        else if(m_pWPolar->bDirichlet())
                        {
                            //then add the potential contribution of the wake column to the matrix coefficient
                            m_aijWake[m*Size+mm] += PHC[m_pPanel[pp].m_iWakeColumn];
                            //we do not add the term Phi_inf_KWPUM - Phi_inf_KWPLM (eq. 44) since it is 0, thin edge
                        }
                    }
                    else if(m_pPanel[pp].m_Pos==xfl::BOTSURFACE)
                    {
                        //the panel sedding a wake is on the bottom side, substract
                        if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                        {
                            //use Neumann B.C.
                            m_aijWake[m*Size+mm] -= VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                            //corrected in v6.02;
                            m_uWake[m] -= TrPt.x  * VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                            m_wWake[m] -= TrPt.z  * VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                        }
                        else if(m_pWPolar->bDirichlet())
                        {
                            m_aijWake[m*Size+mm] -= PHC[m_pPanel[pp].m_iWakeColumn];
                            m_uWake[m] +=  TrPt.x * PHC[m_pPanel[pp].m_iWakeColumn];
                            m_wWake[m] +=  TrPt.z * PHC[m_pPanel[pp].m_iWakeColumn];
                        }
                    }
                    else if(m_pPanel[pp].m_Pos==xfl::TOPSURFACE)
                    {
                        //the panel sedding a wake is on the top side, add
                        if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                        {
                            //use Neumann B.C.
                            m_aijWake[m*Size+mm] += VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                            //corrected in v6.02;
                            m_uWake[m] += TrPt.x * VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                            m_wWake[m] += TrPt.z * VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                        }
                        else if(m_pWPolar->bDirichlet())
                        {
                            m_aijWake[m*Size+mm] += PHC[m_pPanel[pp].m_iWakeColumn];
                            m_uWake[m] -= TrPt.x * PHC[m_pPanel[pp].m_iWakeColumn];
                            m_wWake[m] -= TrPt.z * PHC[m_pPanel[pp].m_iWakeColumn];
                        }
                    }
                }
                mm++;
                //                }
            }
            m++;
        }
        m_Progress += 1.0/double(m_MatSize);
    }
}


/**
* In the case of a panel analysis, adds the contribution of the wake columns to the coefficients of the influence matrix
* Method :
*    - follow the method described in NASA 4023 eq. (44)
*    - add the wake's doublet contribution to the matrix
*    - add the potential difference at the trailing edge panels to the RHS ; the potential's origin
*     is set arbitrarily to the geometrical orgin so that phi = V.dot(WindDirectio) x point_position
* Only a flat wake is considered. Wake roll-up has been tested but did not prove robust enough for implementation.
*/
void PanelAnalysis::createWakeContribution(double *pWakeContrib, const Vector3d &WindDirection)
{
    Vector3d V, C, CC, TrPt;
    double phi=0.0;
    QVector<double> PHC(m_NWakeColumn);
    QVector<Vector3d> VHC(m_NWakeColumn);

    traceLog("      Adding the wake's contribution...\n");

    //    if(m_b3DSymetric) Size = m_SymSize;
    //    else              Size = m_MatSize;

    int m(0), mm(0);
    m = 0;

    for(int p=0; p<m_MatSize; p++)
    {
        if(s_bCancel) return;
        //        if(!m_b3DSymetric || m_pPanel[p].m_bIsLeftPanel)
        //        {
        pWakeContrib[m] = 0.0;
        C    = m_pPanel[p].CollPt;
        CC.x =  C.x;//symmetric point, just in case
        CC.y = -C.y;
        CC.z =  C.z;

        //____________________________________________________________________________
        //build the contributions of each wake column at point C
        //we have m_NWakeColum to consider
        int pw=0;
        for (int kw=0; kw<m_NWakeColumn; kw++)
        {
            PHC[kw] = 0.0;
            VHC[kw].set(0.0,0.0,0.0);
            //each wake column has m_NXWakePanels
            for(int lw=0; lw<m_pWPolar->m_NXWakePanels; lw++)
            {
                getDoubletInfluence(C, m_pWakePanel+pw, V, phi, true, true);

                PHC[kw] += phi;
                VHC[kw] += V;

                /*                    if(m_b3DSymetric && m_pPanel[p].m_bIsLeftPanel)
                    {
                        GetDoubletInfluence(CC, m_pWakePanel+pw, VS, phiSym, true, true);
                        PHC[kw]    +=  phiSym;
                        VHC[kw].x  +=  VS.x;
                        VHC[kw].y  -=  VS.y;
                        VHC[kw].z  +=  VS.z;
                    }*/
                pw++;
            }
        }
        //____________________________________________________________________________
        //Add the contributions to the matrix coefficients and to the RHS
        mm = 0;
        for(int pp=0; pp<m_MatSize; pp++) //for each matrix column
        {
            //                if(!m_b3DSymetric || m_pPanel[pp].m_bIsLeftPanel)
            //                {
            if(s_bCancel) return;

            // Is the panel pp shedding a wake ?
            if(m_pPanel[pp].m_bIsTrailing)
            {
                // Get trailing point where the jump in potential is evaluated
                TrPt = (m_pNode[m_pPanel[pp].m_iTA] + m_pNode[m_pPanel[pp].m_iTB])/2.0;
                // If so, we need to add the contributions of the wake column
                // shedded by this panel to the RHS and to the Matrix
                if(m_pPanel[pp].m_Pos==xfl::MIDSURFACE)
                {
                    //The panel shedding a wake is on a thin surface
                    if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                    {
                        //then add the velocity contribution of the wake column to the matrix coefficient
                        //we do not add the term Phi_inf_KWPUM - Phi_inf_KWPLM (eq. 44) since it is 0, thin edge
                    }
                    else if(m_pWPolar->bDirichlet())
                    {
                        //then add the potential contribution of the wake column to the matrix coefficient
                        //we do not add the term Phi_inf_KWPUM - Phi_inf_KWPLM (eq. 44) since it is 0, thin edge
                    }
                }
                else if(m_pPanel[pp].m_Pos==xfl::BOTSURFACE)//bottom side, substract
                {
                    //evaluate the potential on the bottom side panel pp which is shedding a wake
                    if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                    {
                        //use Neumann B.C.
                        pWakeContrib[m] -= TrPt.dot(WindDirection) * VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                    }
                    else if(m_pWPolar->bDirichlet())
                    {
                        pWakeContrib[m] += TrPt.dot(WindDirection) * PHC[m_pPanel[pp].m_iWakeColumn];
                    }
                }
                else if(m_pPanel[pp].m_Pos==xfl::TOPSURFACE)  //top side, add
                {
                    //evaluate the potential on the top side panel pp which is shedding a wake
                    if(!m_pWPolar->bDirichlet() || m_pPanel[p].m_Pos==xfl::MIDSURFACE)
                    {
                        //use Neumann B.C.
                        pWakeContrib[m] += TrPt.dot(WindDirection) * VHC[m_pPanel[pp].m_iWakeColumn].dot(m_pPanel[p].Normal);
                    }
                    else if(m_pWPolar->bDirichlet())
                    {
                        pWakeContrib[m] -= TrPt.dot(WindDirection) * PHC[m_pPanel[pp].m_iWakeColumn];
                    }
                }
            }
            mm++;
            //                }
        }
        m++;
        //        }
        m_Progress += 1.0/double(m_MatSize);
    }
}


/**
* This method performs the computation in the far-field (Trefftz) plane.
* For each of the wings, calculates, the resulting Force vector and induced drag, and the coefficients for each chordwise strip.
* The calculations are made based on the source and doublet strengths or on the vortex circulations which have been previously computed.
* The results are used a first time to calculate the balance velocities, and a second time for the calculation of aero coefficients
* @param QInf the freestream velocity
* @param Alpha0 the first aoa in the sequence
* @param AlphaDelta the aoa increment
* @param nval the number of aoa in the sequence
*/
void PanelAnalysis::computeFarField(double QInf, double Alpha0, double AlphaDelta, int nval)
{
    QString strong;

    double alpha=0, IDrag=0;
    double const *Mu=nullptr;
    double const *Sigma=nullptr;
    double ThinSize = 0.0;
    Vector3d WindNormal, WingForce;

    traceLog("      Calculating aerodynamic coefficients in the far field plane\n");

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingList[iw]) ThinSize += double(m_pWingList[iw]->m_nPanels);
    }

    for (int q=0; q<nval;q++)
    {
        if(m_pWPolar->bTilted()) alpha = m_OpAlpha;
        else
        {
            if(m_pWPolar->polarType()==xfl::FIXEDAOAPOLAR)       alpha = m_OpAlpha;
            else if(m_pWPolar->polarType()==xfl::BETAPOLAR)      alpha = m_OpAlpha;
//            else if(m_pWPolar->polarType()==XFLR5::STABILITYPOLAR) alpha = m_OpAlpha;
            else if(fabs(m_pWPolar->Beta())>PRECISION)             alpha = m_OpAlpha;
            else                                                   alpha = Alpha0 + q*AlphaDelta;
        }
        WindNormal.set(-sin(alpha*PI/180.0), 0.0, cos(alpha*PI/180.0));

        int pos = 0;
        Mu     = m_Mu    + q*m_MatSize;
        Sigma  = m_Sigma + q*m_MatSize;

        strong = "        Calculating point " + QString("%1").arg(alpha,7,'f',2)+QString::fromUtf8("°....\n");
        traceLog(strong);

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pWingList[iw])
            {
                WingForce.set(0.0, 0.0, 0.0);
                panelTrefftz(m_pWingList[iw], QInf, alpha, Mu, Sigma, pos, WingForce, IDrag, m_pWPolar, m_pWakePanel, m_pWakeNode);

                //save the results... will save another FF calculation when computing the operating point
                m_WingForce[q*MAXWINGS+iw] = WingForce;  // N/q
                m_WingIDrag[q*MAXWINGS+iw] = IDrag;

                memcpy(m_Cl  + (q*MAXWINGS+iw)*m_NSpanStations, m_pWingList[iw]->m_Cl,  uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                memcpy(m_ICd + (q*MAXWINGS+iw)*m_NSpanStations, m_pWingList[iw]->m_ICd, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                memcpy(m_Ai  + (q*MAXWINGS+iw)*m_NSpanStations, m_pWingList[iw]->m_Ai,  uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                memcpy(m_F   + (q*MAXWINGS+iw)*m_NSpanStations, m_pWingList[iw]->m_F,   uint(m_pWingList[iw]->m_NStation)*sizeof(Vector3d));
                memcpy(m_Vd  + (q*MAXWINGS+iw)*m_NSpanStations, m_pWingList[iw]->m_Vd,  uint(m_pWingList[iw]->m_NStation)*sizeof(Vector3d));

                pos += m_pWingList[iw]->m_nPanels;

                m_Progress += 10.0 * double(m_pWingList[iw]->m_nPanels)/ThinSize *double(m_MatSize)/400.;
                if(s_bCancel)return;
            }
        }
    }
}


/**
* In the case of Type 2 or Type 7 polars, computes the velocity for which the lift force balances the plane's weight.
* Assumes the lift force on each lifting wing has been calculated for a unit velocity.
* @param Alpha the angle of attack to calculate
* @param q the index of the aoa in the sequence
*/
void PanelAnalysis::computeBalanceSpeeds(double Alpha, int q)
{
    QString strong;
    Vector3d Force, WindNormal;
    WindNormal.set(-sin(Alpha*PI/180.0),   0.0, cos(Alpha*PI/180.0));

    Force.set(0.0,0.0,0.0);

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingList[iw]) Force += m_WingForce[q*MAXWINGS+iw];
    }
    if (m_pWPolar->isFixedSpeedPolar() || m_pWPolar->isBetaPolar())
    {
        m_3DQInf[q] = m_pWPolar->m_QInfSpec;
    }
    else if(m_pWPolar->isFixedLiftPolar())
    {
        double Lift =  Force.dot(WindNormal) ;      //N/q, for 1/ms
        double TempCl = Lift/m_pWPolar->referenceArea();
        if(Lift<=0.0)
        {
            strong = "           "+QString("Found a negative lift for Alpha=%1.... skipping the angle...\n").arg(Alpha, 5,'f',2);
            traceLog(strong);
            m_bPointOut = true;
            s_bWarning  = true;
            m_3DQInf[q] = -100.0;
        }
        else
        {
            m_3DQInf[q] =  sqrt(2.0* 9.81 * m_Mass/m_pWPolar->density()/TempCl/m_pWPolar->referenceArea());
            strong = QString("           Alpha=%1   QInf=%2m/s").arg(Alpha, 5,'f',2).arg(m_3DQInf[q],5,'f',2);
            strong+="\n";
            traceLog(strong);
        }
    }
}


/**
* Scales the unit results using the specified freestream velocity.
* The velocity may have been specified directly in the case of a type 1 polar, or may have been calculated to balance the plane's weight.
* The velocity for each aoa are provided in the member variable array m_3DQInf
* @param nval the number of aoa in the sequence
*/
void PanelAnalysis::scaleResultstoSpeed(int nval)
{
    //______________________________________________________________________________________
    // Scale RHS and Sigma i.a.w. speeds (so far we have unit doublet and source strengths)

    QString strong="\n";
    traceLog(strong);

    memcpy(m_SigmaRef, m_Sigma, uint(nval*m_MatSize)*sizeof(double));
    memcpy(m_RHSRef,   m_Mu,    uint(nval*m_MatSize)*sizeof(double));

    if(m_pWPolar->polarType()!=xfl::FIXEDAOAPOLAR)
    {
        int p=0;
        for (int q=0; q<nval;q++)
        {
            for(int pp=0; pp<m_MatSize; pp++)
            {
                m_Mu[p]     *= m_3DQInf[q];
                m_Sigma[p]  *= m_3DQInf[q];
                p++;
            }
        }
    }
    else
    {
        //type 4, we scale the first single rhs for all specified speed values
        int p=0;
        for (int q=0; q<nval;q++)
        {
            for(int pp=0; pp<m_MatSize; pp++)
            {
                m_Mu[p]    = m_RHSRef[pp]   * m_3DQInf[q];
                m_Sigma[p] = m_SigmaRef[pp] * m_3DQInf[q];
                p++;
            }
        }
    }

    //scale the strip force and downwash fields
    if(m_pWPolar->polarType()!=xfl::FIXEDAOAPOLAR)
    {
        for (int q=0; q<nval;q++)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(m_pWingList[iw])
                {
                    for(int m=0; m<m_pWingList[iw]->m_NStation; m++)
                    {
                        m_F[ m+ (q*MAXWINGS+iw)*m_NSpanStations] *= m_3DQInf[q] * m_3DQInf[q];
                        m_Vd[m+ (q*MAXWINGS+iw)*m_NSpanStations] *= m_3DQInf[q] ;
                    }
                }
            }
        }
    }
    else
    {
        for (int q=0; q<nval;q++)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(m_pWingList[iw])
                {
                    for(int m=0; m<m_pWingList[iw]->m_NStation; m++)
                    {
                        m_F[ m+ (q*MAXWINGS+iw)*m_NSpanStations] = m_F[ m] * m_3DQInf[q] * m_3DQInf[q];
                        m_Vd[m+ (q*MAXWINGS+iw)*m_NSpanStations] = m_Vd[m] * m_3DQInf[q];
                    }
                    m_WingForce[q*MAXWINGS+iw] = m_WingForce[iw];
                    m_WingIDrag[q*MAXWINGS+iw] = m_WingIDrag[iw];
                }
            }
        }
    }
}


/**
* Computes the  viscous and inviscid aerodynamic coefficients.
* @param V0 the initial aoa or velocity, depending on the polar type
* @param VDelta the aoa or velocity increment, depending on the polar type
* @param nrhs the number of points in the sequence
*/
void PanelAnalysis::computeAeroCoefs(double V0, double VDelta, int nrhs)
{
    QString str;

    if(m_pWPolar->isFixedaoaPolar())
    {
        for (int q=0; q<nrhs; q++)
        {
            if(s_bCancel) return;
            str = QString("      Computing Plane for QInf=%1m/s").arg((V0+q*VDelta),7,'f',2);
            traceLog(str);
            computePlane(m_OpAlpha, V0+q*VDelta, q);
            m_Progress += 5.0*double(nrhs) /double(nrhs);
        }
    }
    else if(m_pWPolar->isBetaPolar())
    {
        for (int q=0; q<nrhs; q++)
        {
            if(s_bCancel) return;
            str = QString("      Computing Plane for beta=%1").arg((m_OpBeta),0,'f',1);
            str += QString::fromUtf8("°\n");
            traceLog(str);
            computePlane(m_OpAlpha, m_3DQInf[q], q);
            m_Progress += 5.0*double(nrhs) /double(nrhs);
        }
    }
    else
    {
        for (int q=0; q<nrhs; q++)
        {
            if(s_bCancel) return;
            if(m_3DQInf[q]>0.0)
            {
                if(!m_pWPolar->bTilted()) str = QString("      Computing Plane for alpha=%1").arg(V0+q*VDelta,7,'f',2);
                else                      str = QString("      Computing Plane for alpha=%1").arg(m_OpAlpha,7,'f',2);
                str += QString::fromUtf8("°\n");
                traceLog(str);
                computePlane(V0+q*VDelta, m_3DQInf[q], q);
            }
            m_Progress += 5.0*double(nrhs)/double(nrhs);
        }
    }
}



/**
 * This method calculates the all the viscous and inviscid aerodynamic coefficients for one aoa.
 * The method calls the method in the QMiarex class which creates the operating points, updates the polar data, and updates the graphs.
 * The method uses in input the result data stored in the member variables.
 * @param Alpha the aoa of this calculation
 * @param QInf the freesteam velocity of this calculation
 * @param qrhs the index of the current right hand side calculation
*/
void PanelAnalysis::computePlane(double Alpha, double QInf, int qrhs)
{
    int pos(0);
    double Lift(0), IDrag(0), VDrag(0), XCP(0), YCP(0), ZCP(0), WingVDrag(0);
    Vector3d WindNormal, WindDirection, WindSide;
    Vector3d Force;
    QString OutString;

    if(m_pWPolar->bTilted() || m_pWPolar->isBetaPolar() || fabs(m_pWPolar->Beta())>PRECISION)
    {
        // the analysis is performed at aoa = 0.0 on a rotated geometry
        Alpha = m_OpAlpha;
        WindNormal.set(0,0,1);
        WindDirection.set(1,0,0);
    }
    else
    {
        m_OpAlpha = Alpha;
        //   Define wind (stability) axis
        double cosa = cos(Alpha*PI/180.0);
        double sina = sin(Alpha*PI/180.0);

        WindNormal.set(  -sina, 0.0, cosa);
        WindDirection.set(cosa, 0.0, sina);
        WindSide = WindNormal * WindDirection;
    }

    double const*Mu     = m_Mu    + qrhs*m_MatSize;
    double const*Sigma  = m_Sigma + qrhs*m_MatSize;

    m_QInf      = QInf;

    for(int iw=0; iw<MAXWINGS; iw++)
        if(m_pWingList[iw])m_pWingList[iw]->m_bWingOut = false;
    //    m_pWing->m_bWingOut                = false;

    if(QInf >0.0)
    {
        traceLog("       Calculating aerodynamic coefficients...\n");
        m_bPointOut = false;
        Force.set(0.0, 0.0, 0.0);

        Lift = IDrag = VDrag = XCP = YCP = ZCP = 0.0;
        m_GCm = m_VCm = m_ICm = m_GRm = m_GYm = m_VYm = m_IYm = 0.0;

        pos = 0;

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pWingList[iw])
            {
                traceLog("         Calculating wing..." + m_pWingList[iw]->m_Name+"\n");
                //restore the saved FF results
                if(m_pWPolar->polarType()!=xfl::FIXEDAOAPOLAR)
                {
                    memcpy(m_pWingList[iw]->m_Cl,  m_Cl  + (qrhs*MAXWINGS+iw)*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                    memcpy(m_pWingList[iw]->m_ICd, m_ICd + (qrhs*MAXWINGS+iw)*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                    memcpy(m_pWingList[iw]->m_Ai,  m_Ai  + (qrhs*MAXWINGS+iw)*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                    memcpy(m_pWingList[iw]->m_F,   m_F   + (qrhs*MAXWINGS+iw)*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(Vector3d));
                    memcpy(m_pWingList[iw]->m_Vd,  m_Vd  + (qrhs*MAXWINGS+iw)*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(Vector3d));
                }
                else
                {
                    memcpy(m_pWingList[iw]->m_Cl,  m_Cl +iw*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                    memcpy(m_pWingList[iw]->m_ICd, m_ICd+iw*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                    memcpy(m_pWingList[iw]->m_Ai,  m_Ai +iw*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(double));
                    memcpy(m_pWingList[iw]->m_F,   m_F  +iw*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(Vector3d));
                    memcpy(m_pWingList[iw]->m_Vd,  m_Vd +iw*m_NSpanStations, uint(m_pWingList[iw]->m_NStation)*sizeof(Vector3d));
                }

                Force += m_WingForce[qrhs*MAXWINGS+iw];
                IDrag += m_WingIDrag[qrhs*MAXWINGS+iw];

                //Get viscous interpolations
                m_pWingList[iw]->panelComputeViscous(QInf, m_pWPolar, WingVDrag, m_pWPolar->bViscous(), OutString);
                VDrag += WingVDrag;

                traceLog(OutString);
                if(m_pWingList[iw]->m_bWingOut)  m_bPointOut = true;


                //Compute moment coefficients
                m_pWingList[iw]->panelComputeOnBody(QInf, Alpha, m_Cp+qrhs*m_MatSize+pos, m_Mu+qrhs*m_MatSize+pos,
                                                    XCP, YCP, ZCP, m_GCm, m_VCm, m_ICm, m_GRm, m_GYm, m_VYm, m_IYm,
                                                    m_pWPolar, m_CoG, m_pPanel);


                m_pWingList[iw]->panelComputeBending(m_pPanel, m_pWPolar->bThinSurfaces());

                pos += m_pWingList[iw]->m_nPanels;
            }
        }

        if(m_pPlane->body() && m_pWPolar->analysisMethod()==xfl::PANEL4METHOD && !m_pWPolar->bIgnoreBodyPanels())
        {
            double ICm = 0.0;
            traceLog("       Calculating body...\n");
            m_pPlane->body()->computeAero(m_Cp+qrhs*m_MatSize+pos, XCP, YCP, ZCP, ICm, m_GRm, m_GYm, Alpha, m_CoG, m_pPanel);
            m_ICm += ICm;
            m_GCm += ICm;

            //the body does not shed any wake --> no induced lift or drag
        }

        if(!s_bTrefftz) sumPanelForces(m_Cp+qrhs*m_MatSize, Alpha, Lift, IDrag);

        m_CL          =       Force.dot(WindNormal)    /m_pWPolar->referenceArea();
        m_CX          =       Force.dot(WindDirection) /m_pWPolar->referenceArea();
        m_CY          =       Force.dot(WindSide)      /m_pWPolar->referenceArea();

        m_InducedDrag =  1.0*IDrag/m_pWPolar->referenceArea();
        m_ViscousDrag =  1.0*VDrag/m_pWPolar->referenceArea();


        if(fabs(Force.dot(WindNormal))>0.0)
        {
            m_CP.x         = XCP/Force.dot(WindNormal);
            m_CP.y         = YCP/Force.dot(WindNormal);
            m_CP.z         = ZCP/Force.dot(WindNormal);
        }
        else
        {
            m_CP.set(0.0,0.0,0.0);
        }


        m_GCm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceChordLength();
        m_VCm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceChordLength();
        m_ICm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceChordLength();

        m_GRm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceSpanLength();

        m_GYm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceSpanLength();
        m_VYm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceSpanLength();
        m_IYm *= 1.0 / m_pWPolar->referenceArea() /m_pWPolar->referenceSpanLength();


        if(m_pWPolar->isStabilityPolar()) computePhillipsFormulae();

        if(m_bPointOut) s_bWarning = true;

        if(m_pWPolar->isStabilityPolar()) m_Alpha = m_AlphaEq; // so it is set by default at the end of the analyis

        PlaneOpp *pPOpp = createPlaneOpp(m_Cp+qrhs*m_MatSize, Mu, Sigma);
        m_PlaneOppList.append(pPOpp);

        traceLog("\n");
    }
    else m_bPointOut = true;
}


/**
* Returns the estimation of the panel's lift coeficient based on the vortex circulation.
* @param p the index of the panel
* @param Gamma the pointer to the array of vortex circulations
* @param Cp a pointer to the array of resulting panel lift coefficients
* @param VInf the freestream velocity vector
*/
void PanelAnalysis::getVortexCp(int p, double const *Gamma, double *Cp, Vector3d const &VInf) const
{
    Vector3d PanelForce, Force;
    // for each panel along the chord, add the lift coef
    PanelForce  = VInf * m_pPanel[p].Vortex;
    PanelForce *= Gamma[p] * m_pWPolar->density();                 //Newtons

    if(!m_pWPolar->bVLM1() && !m_pPanel[p].m_bIsLeading)
    {
        Force       = VInf* m_pPanel[p].Vortex;
        Force      *= Gamma[p+1] * m_pWPolar->density();           //Newtons
        PanelForce -= Force;
    }

    Cp[p]  = -2.0 * PanelForce.dot(m_pPanel[p].Normal) /m_pPanel[p].Area/m_pWPolar->density();
}


/**
* This method calculates the Cp coefficient on a panel based on the distribution of doublet strengths.
* This calculation follows the method provided in document NASA 4023.
* @param p the index of the panel for which the calculation is performed
* @param Mu the array of doublet strength values
* @param Cp a reference to the Cp variable to evaluate
* @param VLocal a reference to the Vector3d holding the local velocity on the panel
* @param QInf the freestream velocity
* @param Vx the x-component of the freestream velocity vector
* @param Vz the z-component of the freestream velocity vector
*/
void PanelAnalysis::getDoubletDerivative(const int &p, double const*Mu, double &Cp, Vector3d &VLocal, double QInf, double Vx, double Vy, double Vz) const
{
    double DELQ=0, DELP=0, mu0=0, mu1=0, mu2=0, x0=0, x1=0, x2=0, Speed2=0;
    Vector3d VTot;//total local panel speed
    Vector3d S2, Sl2;

    int PL = m_pPanel[p].m_iPL;
    int PR = m_pPanel[p].m_iPR;
    int PU = m_pPanel[p].m_iPU;
    int PD = m_pPanel[p].m_iPD;

    if(PL>=0 && PR>=0)
    {
        //we have two side neighbours
        x1  = 0.0;
        x0  = x1 - m_pPanel[p].SMQ - m_pPanel[PL].SMQ;
        x2  = x1 + m_pPanel[p].SMQ + m_pPanel[PR].SMQ;
        mu0 = Mu[PL];
        mu1 = Mu[p];
        mu2 = Mu[PR];
        DELQ =      mu0 *(x1-x2)       /(x0-x1)/(x0-x2)
                + mu1 *(2.0*x1-x0-x2)/(x1-x0)/(x1-x2)
                + mu2 *(x1-x0)       /(x2-x0)/(x2-x1);
    }
    else if(PL>=0 && PR<0)
    {
        // no right neighbour
        // do we have two left neighbours ?
        if(m_pPanel[PL].m_iPL>=0)
        {
            x2  = 0.0;
            x1  = x2 - m_pPanel[p].SMQ  - m_pPanel[PL].SMQ;
            x0  = x1 - m_pPanel[PL].SMQ - m_pPanel[m_pPanel[PL].m_iPL].SMQ;

            mu0 = Mu[m_pPanel[PL].m_iPL];
            mu1 = Mu[PL];
            mu2 = Mu[p];
            DELQ =      mu0 *(x2-x1)       /(x0-x1)/(x0-x2)
                    + mu1 *(x2-x0)       /(x1-x0)/(x1-x2)
                    + mu2 *(2.0*x2-x0-x1)/(x2-x0)/(x2-x1);
        }
        else
        {
            //calculate the derivative on two panels only
            DELQ = -(Mu[PL]-Mu[p])/(m_pPanel[p].SMQ  + m_pPanel[PL].SMQ);
        }
    }
    else if(PL<0 && PR>=0)
    {
        // no left neighbour
        // do we have two right neighbours ?
        if(m_pPanel[PR].m_iPR>=0){
            x0  = 0.0;
            x1  = x0 + m_pPanel[p].SMQ  + m_pPanel[PR].SMQ;
            x2  = x1 + m_pPanel[PR].SMQ + m_pPanel[m_pPanel[PR].m_iPR].SMQ;
            mu0 = Mu[p];
            mu1 = Mu[PR];
            mu2 = Mu[m_pPanel[PR].m_iPR];
            DELQ =      mu0 *(2.0*x0-x1-x2)/(x0-x1)/(x0-x2)
                    + mu1 *(x0-x2)       /(x1-x0)/(x1-x2)
                    + mu2 *(x0-x1)       /(x2-x0)/(x2-x1);
        }
        else
        {
            //calculate the derivative on two panels only
            DELQ = (Mu[PR]-Mu[p])/(m_pPanel[p].SMQ  + m_pPanel[PR].SMQ);
        }
    }
    else
    {
        DELQ = 0.0;
        //Cannot calculate a derivative on one panel only
    }

    if(PU>=0 && PD>=0)
    {
        //we have one upstream and one downstream neighbour
        x1  = 0.0;
        x0  = x1 - m_pPanel[p].SMP - m_pPanel[PU].SMP;
        x2  = x1 + m_pPanel[p].SMP + m_pPanel[PD].SMP;
        mu0 = Mu[PU];
        mu1 = Mu[p];
        mu2 = Mu[PD];
        DELP =      mu0 *(x1-x2)       /(x0-x1)/(x0-x2)
                + mu1 *(2.0*x1-x0-x2)/(x1-x0)/(x1-x2)
                + mu2 *(x1-x0)       /(x2-x0)/(x2-x1);
    }
    else if(PU>=0 && PD<0)
    {
        // no downstream neighbour
        // do we have two upstream neighbours ?
        if(m_pPanel[PU].m_iPU>=0)
        {
            x2  = 0.0;
            x1  = x2 - m_pPanel[p ].SMP  - m_pPanel[PU].SMP;
            x0  = x1 - m_pPanel[PU].SMP  - m_pPanel[m_pPanel[PU].m_iPU].SMP;
            mu0 = Mu[m_pPanel[PU].m_iPU];
            mu1 = Mu[PU];
            mu2 = Mu[p];
            DELP =      mu0 *(x2-x1)       /(x0-x1)/(x0-x2)
                    + mu1 *(x2-x0)       /(x1-x0)/(x1-x2)
                    + mu2 *(2.0*x2-x0-x1)/(x2-x0)/(x2-x1);
        }
        else
        {
            //calculate the derivative on two panels only
            DELP = -(Mu[PU]-Mu[p])/(m_pPanel[p].SMP  + m_pPanel[PU].SMP);
        }
    }
    else if(PU<0 && PD>=0)
    {
        // no upstream neighbour
        // do we have two downstream neighbours ?
        if(m_pPanel[PD].m_iPD>=0)
        {
            x0  = 0.0;
            x1  = x0 + m_pPanel[p].SMP  + m_pPanel[PD].SMP;
            x2  = x1 + m_pPanel[PD].SMP + m_pPanel[m_pPanel[PD].m_iPD].SMP;
            mu0 = Mu[p];
            mu1 = Mu[PD];
            mu2 = Mu[m_pPanel[PD].m_iPD];
            DELP =      mu0 *(2.0*x0-x1-x2)/(x0-x1)/(x0-x2)
                    + mu1 *(x0-x2)       /(x1-x0)/(x1-x2)
                    + mu2 *(x0-x1)       /(x2-x0)/(x2-x1);
        }
        else
        {
            //calculate the derivative on two panels only
            DELP = (Mu[PD]-Mu[p])/(m_pPanel[p].SMP  + m_pPanel[PD].SMP);
        }
    }
    else
    {
        DELP = 0.0;
    }

    //find middle of side 2
    S2 = (m_pNode[m_pPanel[p].m_iTA] + m_pNode[m_pPanel[p].m_iTB])/2.0 - m_pPanel[p].CollPt;
    //convert to local coordinates
    Sl2   = m_pPanel[p].globalToLocal(S2);
    VTot  = m_pPanel[p].globalToLocal(Vx, Vy, Vz);

    //in panel referential
    VLocal.x = -4.0*PI*(m_pPanel[p].SMP*DELP - Sl2.y*DELQ)/Sl2.x;
    VLocal.y = -4.0*PI*DELQ;
    //    Vl.z =  4.0*PI*Sigma[p];

    VTot +=VLocal;
    VTot.z = 0.0;

    Speed2 = VTot.x*VTot.x + VTot.y*VTot.y;

    Cp  = 1.0-Speed2/QInf/QInf;
}


/**
* Calculates the Cp coefficient on each panel, using hte vortex circulations or the doublet strengths, depending on the analysis method.
* @param V0 the first value in the sequence, either aoa for type 1 & 2 polars or velocity for type 4
* @param VDelta the increment value of the input parameter, either aoa for type 1 & 2 polars or velocity for type 4
* @param nval the number of values in the sequence
*/
void PanelAnalysis::computeOnBodyCp(double V0, double VDelta, int nval)
{
    //following VSAERO theory manual
    //the on-body tangential perturbation speed is the derivative of the doublet strength

    double Alpha, *Mu, *Cp;
    Vector3d WindDirection, VInf, VLocal;
    double Speed2, cosa, sina;
    //______________________________________________________________________________________
    traceLog("      Computing On-Body Speeds...\n");

    if(m_pWPolar->polarType() != xfl::FIXEDAOAPOLAR)
    {
        for (int q=0; q<nval; q++)
        {
            //   Define wind axis
            Alpha = V0 + double(q) * VDelta;
            cosa = cos(Alpha*PI/180.0);
            sina = sin(Alpha*PI/180.0);
            WindDirection.set(cosa, 0.0, sina);
            VInf = WindDirection * m_3DQInf[q];

            Mu     = m_Mu    + q * m_MatSize;
            //            Sigma  = m_Sigma + q * m_MatSize;
            Cp     = m_Cp    + q * m_MatSize;

            for (int p=0; p<m_MatSize; p++)
            {
                if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE)
                {
                    m_pPanel[p].globalToLocal(VInf, VLocal);
                    VLocal += m_uVl[p]*cosa*m_3DQInf[q] + m_wVl[p]*sina*m_3DQInf[q];
                    Speed2 = VLocal.x*VLocal.x + VLocal.y*VLocal.y;
                    Cp[p]  = 1.0-Speed2/m_3DQInf[q]/m_3DQInf[q];
                }
                else getVortexCp(p, Mu, Cp, WindDirection);

                if(s_bCancel) return;
            }
            if(s_bCancel) return;
            m_Progress += 1.0 *double(nval)/double(nval);
        }
    }
    else //FIXEDAOAPOLAR
    {
        //   Define wind axis
        WindDirection.set(cos(m_Alpha*PI/180.0), 0.0, sin(m_Alpha*PI/180.0));
        for (int q=0; q<nval; q++)
        {
            VInf = WindDirection * m_3DQInf[q];

            Mu     = m_Mu    + q * m_MatSize;
            //            Sigma  = m_Sigma + q * m_MatSize;
            Cp     = m_Cp    + q * m_MatSize;

            for (int p=0; p<m_MatSize; p++)
            {
                if(s_bCancel) break;

                if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE) getDoubletDerivative(p, Mu, Cp[p], VLocal, m_3DQInf[q], VInf.x, VInf.y, VInf.z);
                else                              getVortexCp(p, Mu, Cp, WindDirection);
            }

            m_Progress += 1.0  *double(nval)/double(nval);
        }
        for (int q=1; q<nval; q++)
        {
            if(s_bCancel) return;
            for (int p=0; p<m_MatSize; p++)
            {
                m_Cp[p+q*m_MatSize] = m_Cp[p];
            }
        }
    }
}



/**
* Returns the influence at point C of the panel pPanel.
* If the panel pPanel is located on a thin surface, then its the influence of a vortex.
* If it is on a thick surface, then its a doublet.
*
*@param C the point where the influence is to be evaluated
*@param pPanel a pointer to the Panel with the doublet strength
*@param V the perturbation velocity at point C
*@param phi the potential at point C
*@param bWake true if the panel is located on the wake
*@param bAll true if the influence of the bound vortex should be evaluated, in the case of a VLM analysis.
*/
void PanelAnalysis::getDoubletInfluence(Vector3d const &C, Panel const *pPanel, Vector3d &V, double &phi, bool bWake, bool bAll) const
{
    if(pPanel->m_Pos!=xfl::MIDSURFACE || pPanel->m_bIsWakePanel)
        pPanel->doubletNASA4023(C, V, phi, bWake);
    else
    {
        VLMGetVortexInfluence(pPanel, C, V, bAll);
        phi = 0.0;
    }

    if(m_pWPolar->bGround())
    {
        double phiG = 0.0;
        Vector3d VG;
        Vector3d CG(C.x, C.y, -C.z-2.0*m_pWPolar->m_Height);

        if(pPanel->m_Pos!=xfl::MIDSURFACE || pPanel->m_bIsWakePanel)    pPanel->doubletNASA4023(CG, VG, phiG, bWake);
        else
        {
            VLMGetVortexInfluence(pPanel, CG, VG, bAll);
            phiG = 0.0;
        }
        V.x += VG.x;
        V.y += VG.y;
        V.z -= VG.z;
        phi += phiG;
    }
}


/**
* Returns the influence at point C of a uniform source distribution on the panel pPanel
* The panel is necessarily located on a thick surface, else the source strength is zero
* @param C the point where the influence is to be evaluated
* @param pPanel a pointer to the Panel with the doublet strength
* @param V the perturbation velocity at point C
* @param phi the potential at point C
*/
void PanelAnalysis::getSourceInfluence(Vector3d const &C, Panel *pPanel, Vector3d &V, double &phi) const
{
    pPanel->sourceNASA4023(C, V, phi);

    if(m_pWPolar->bGround())
    {
        double phiG = 0.0;
        Vector3d VG;
        Vector3d CG(C.x, C.y, -C.z-2.0*m_pWPolar->m_Height);
        pPanel->sourceNASA4023(CG, VG, phiG);
        V.x += VG.x;
        V.y += VG.y;
        V.z -= VG.z;
        phi += phiG;
    }
}


/**
* Returns the perturbation velocity vector at a given point, due to the distribution of source and doublet/circulation strengths.
* @param C the point where the influence is to be evaluated
* @param Mu a pointer to the array of doublet strength or vortex circulations
* @param sigma a pointer to the array of source strengths
* @param VT the resulting perturbation velocity
* @param bAll true if the influence of the bound vortex should be included, in the case of a VLM analysis
*/
void PanelAnalysis::getSpeedVector(Vector3d const &C, double const *Mu, double const *Sigma, Vector3d &VT, bool bAll) const
{
    Vector3d V;
    double phi(0), sign(0);

    VT.set(0.0,0.0,0.0);

    for (int pp=0; pp<m_MatSize;pp++)
    {
        if(s_bCancel) return;

        if(m_pPanel[pp].m_Pos!=xfl::MIDSURFACE) //otherwise Sigma[pp] =0.0, so contribution is zero also
        {
            getSourceInfluence(C, m_pPanel+pp, V, phi);
            VT += V * Sigma[pp] ;
        }
        getDoubletInfluence(C, m_pPanel+pp, V, phi, false, bAll);

        VT += V * Mu[pp];

        // Is the panel pp shedding a wake ?
        if(m_pPanel[pp].m_bIsTrailing && m_pPanel[pp].m_Pos!=xfl::MIDSURFACE)
        {
            //If so, we need to add the contribution of the wake column shedded by this panel
            if(m_pPanel[pp].m_Pos==xfl::BOTSURFACE) sign=-1.0; else sign=1.0;
            int pw = m_pPanel[pp].m_iWake;
            for(int lw=0; lw<m_pWPolar->m_NXWakePanels; lw++)
            {
                getDoubletInfluence(C, m_pWakePanel+pw+lw, V, phi, true, bAll);
                VT += V * Mu[pp]*sign;
            }
        }
    }
}





/**
* Launches a calculation over the input sequence of velocity.
* Used for type 4 analysis, without tilted geometry.
*
* The calculation is performed for two unit RHS, and all the Operating POints are calculated by linear combination.
* The two unit RHS are for a unit velocity along the x-axis, and for a unit velocity along the z-axis.
*@return true if all the aoa were computed successfully, false otherwise. Interpolation issues are not counted as unsuccessful.
*/
bool PanelAnalysis::QInfLoop()
{
    QString str;
    double Alpha = 0.0;

    setInertia(0.0, 0.0, 0.0);

    m_QInf = m_vMin;

    if(!m_pWPolar->bThinSurfaces()) m_TotalTime +=1.0; //for wake contribution

    m_Progress = 0.0;

    if(m_pWPolar->bTilted())
    {
        //reset the initial geometry before a new angle is processed
        memcpy(m_pPanel,        m_pMemPanel,     uint(m_MatSize) * sizeof(Panel));
        memcpy(m_pNode,         m_pMemNode,      uint(m_nNodes) * sizeof(Vector3d));
        memcpy(m_pWakePanel,    m_pRefWakePanel, uint(m_WakeSize) * sizeof(Panel));
        memcpy(m_pWakeNode,     m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));
        memcpy(m_pTempWakeNode, m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));

        // Rotate the wing panels and translate the wake to the new T.E. position
        Vector3d O;
        rotateGeomY(m_pWPolar->m_AlphaSpec, O, m_pWPolar->m_NXWakePanels);

        m_OpAlpha = m_pWPolar->m_AlphaSpec;
        Alpha = 0.0;
    }
    else
    {
        Alpha = m_Alpha;
        m_OpAlpha = m_Alpha;
    }

    str = QString("   Solving the problem... \n");
    traceLog("\n"+str);

    buildInfluenceMatrix();
    if (s_bCancel) return true;

    createUnitRHS();
    if (s_bCancel) return true;

    createSourceStrength(m_Alpha, 0.0, 1);
    if (s_bCancel) return true;

    if(!m_pWPolar->bThinSurfaces())
    {
        //compute wake contribution
        createWakeContribution();

        //add wake contribution to matrix and RHS
        for(int p=0; p<m_MatSize; p++)
        {
            m_uRHS[p]+= m_uWake[p];
            m_wRHS[p]+= m_wWake[p];
            for(int pp=0; pp<m_MatSize; pp++)
            {
                m_aij[p*m_MatSize+pp] += m_aijWake[p*m_MatSize+pp];
            }
        }
    }
    if (s_bCancel) return true;

    if (!solveUnitRHS())
    {
        s_bWarning = true;
        return true;
    }
    if (s_bCancel) return true;

    createDoubletStrength(Alpha, m_vDelta, 1);
    if (s_bCancel) return true;


    computeFarField(1.0, m_OpAlpha, 0.0, 1);
    if (s_bCancel) return true;


    for(int q=0; q<m_nRHS; q++)
        m_3DQInf[q] = m_QInf+q*m_vDelta;

    scaleResultstoSpeed(m_nRHS);
    if (s_bCancel) return true;


    computeOnBodyCp(m_QInf, m_vDelta, m_nRHS);
    if (s_bCancel) return true;

    computeAeroCoefs(m_QInf, m_vDelta, m_nRHS);
    if (s_bCancel) return true;

    return true;
}


/**
* Solves the linear system for the two unit RHS, using LU decomposition.
* Calculates the local velocities on each panel for the two unit RHS
*/
bool PanelAnalysis::solveUnitRHS()
{
    double taskTime = 400.0;
    int Size = m_MatSize;

    QElapsedTimer t;
    t.start();

    memcpy(m_RHS,      m_uRHS, uint(Size) * sizeof(double));
    memcpy(m_RHS+Size, m_wRHS, uint(Size) * sizeof(double));

    traceLog("      Performing LU Matrix decomposition...\n");

    if(!Crout_LU_Decomposition_with_Pivoting(m_aij, m_Index, Size, &s_bCancel, taskTime*double(m_MatSize)/400.0, m_Progress))
    {
        traceLog("      Singular Matrix.... Aborting calculation...\n");
        return false;
    }

    traceLog("      Solving the LU system...\n");
    Crout_LU_with_Pivoting_Solve(m_aij, m_uRHS, m_Index, m_RHS,      Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_wRHS, m_Index, m_RHS+Size, Size, &s_bCancel);

    QString strange;
    strange = QString::asprintf("      Time for linear system solve: %.3f s\n", double(t.elapsed())/1000.0);
    //    qDebug(strange.toStdString().c_str());
    traceLog(strange);

    memcpy(m_uRHS, m_RHS,           uint(m_MatSize)*sizeof(double));
    memcpy(m_wRHS, m_RHS+m_MatSize, uint(m_MatSize)*sizeof(double));

    //   Define unit local velocity vector, necessary for moment calculations in stability analysis of 3D panels
    Vector3d u(1.0, 0.0, 0.0);
    Vector3d w(0.0, 0.0, 1.0);
    double Cp;

    //for(int i4=0; i4<m_MatSize; i4++) displayDouble(m_uRHS[i4], m_wRHS[i4]);

    for (int p=0; p<m_MatSize; p++)
    {
        if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE)
        {
            getDoubletDerivative(p, m_uRHS, Cp, m_uVl[p], 1.0, u.x, u.y, u.z);
            getDoubletDerivative(p, m_wRHS, Cp, m_wVl[p], 1.0, w.x, w.y, w.z);
        }
        if(s_bCancel) return false;
    }

    //for(int p=0; p<m_MatSize; p++) displayDouble('local', m_uVl[p].x, m_uVl[p].y, m_uVl[p].z, m_wVl[p].x, m_wVl[p].y, m_wVl[p].z);

    return true;
}



/**
*
* Creates the doublet strength or the vortex circulations for all the operating points from the unit sine and cosine unit results.
* The doublet and source strengths are for a unit speed.
* The scaling to speed is performed at the next step, depending on the polar type.
*
* @param Alpha0 the first aoa in the sequence
* @param AlphaDelta the aoa increment
* @param nval the number of aoa to calculate
*/
void PanelAnalysis::createDoubletStrength(double Alpha0, double AlphaDelta, int nval)
{
    traceLog("      Calculating doublet strength...\n");

    //______________________________________________________________________________________
    //    reconstruct all results from cosine and sine unit vectors
    for (int q=0; q<nval;q++)
    {
        double alpha = Alpha0 + q * AlphaDelta;
        double cosa = cos(alpha*PI/180.0);
        double sina = sin(alpha*PI/180.0);
        for(int p=0; p<m_MatSize; p++)
        {
            m_Mu[p+q*m_MatSize] = cosa*m_uRHS[p] + sina*m_wRHS[p];
        }
    }
}



/**
* Performs the summation of on-body forces to calculate the total lift and drag forces
* @param Cp a pointer to the array of previously calculated Cp coefficients
* @param Alpha the aoa for which this calculation is performed
* @param Lift the resulting lift force
* @param Drag the resulting drag force
*/
void PanelAnalysis::sumPanelForces(double const *Cp, double Alpha, double &Lift, double &Drag)
{
    Vector3d PanelForce;

    for(int p=0; p<m_MatSize; p++)
    {
        PanelForce += m_pPanel[p].Normal * (-Cp[p]) * m_pPanel[p].Area;
    }

    Lift = PanelForce.z * cos(Alpha*PI/180.0) - PanelForce.x * sin(Alpha*PI/180.0);
    Drag = PanelForce.x * cos(Alpha*PI/180.0) + PanelForce.z * sin(Alpha*PI/180.0);
}



/**
* Launches the calculation of operating points, when linear combination is not an option.
* This is the case for analysis of tilted geometries, or with wake roll-up, of for Beta-type polars
*
*@return true if the aoa was computed successfully, false otherwise.
*/
bool PanelAnalysis::unitLoop()
{
    QString str;
    Vector3d O(0.0,0.0,0.0);

    int MaxWakeIter=0;

    if(!m_pWPolar->bWakeRollUp()) MaxWakeIter = 1;
    else                          MaxWakeIter = qMax(s_MaxWakeIter, 1);

    m_Progress = 0.0;

    str = QString("   Solving the problem...\n");
    traceLog("\n"+str);

    for (int n=0; n<m_nRHS; n++)
    {
        switch(m_pWPolar->polarType())
        {
            case xfl::BETAPOLAR:
                m_OpAlpha = m_pWPolar->m_AlphaSpec;
                m_OpBeta  = m_vMin+n*m_vDelta;
                break;

            case xfl::FIXEDSPEEDPOLAR:
            case xfl::FIXEDLIFTPOLAR:
                m_OpAlpha = m_vMin+n*m_vDelta;
                m_OpBeta  = m_pWPolar->Beta();
                break;

            case xfl::FIXEDAOAPOLAR:
                m_OpAlpha = m_pWPolar->Alpha();
                m_OpBeta  = m_pWPolar->Beta();
                m_QInf      = m_vMin+n*m_vDelta;
                m_3DQInf[n] = m_vMin+n*m_vDelta;
                break;

            default:
                m_OpAlpha = m_vMin+n*m_vDelta;
                m_OpBeta  = m_pWPolar->Beta();
                break;
        }

        setInertia(0.0, m_OpAlpha, m_OpBeta);

        if(m_pWPolar->polarType()!=xfl::BETAPOLAR) str = QString("      \n    Processing Alpha= %1\n").arg(m_OpAlpha,0,'f',1);
        else                                         str = QString("      \n    Processing Beta= %1\n").arg(m_OpBeta,0,'f',1);
        traceLog(str);

        //reset the initial geometry before a new angle is processed
        memcpy(m_pPanel,         m_pMemPanel,     uint(m_MatSize)    * sizeof(Panel));
        memcpy(m_pNode,          m_pMemNode,      uint(m_nNodes)     * sizeof(Vector3d));
        memcpy(m_pWakePanel,     m_pRefWakePanel, uint(m_WakeSize)   * sizeof(Panel));
        memcpy(m_pWakeNode,      m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));
        memcpy(m_pTempWakeNode,  m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));

        // Rotate the wing panels and translate the wake to the new T.E. position
        rotateGeomY(m_OpAlpha, O, m_pWPolar->m_NXWakePanels);

        //        if(m_pWPolar->polarType()==XFLR5::BETAPOLAR)
        if(fabs(m_OpBeta)>PRECISION)
        {
            rotateGeomZ(m_OpBeta, O, m_pWPolar->m_NXWakePanels);
        }

        buildInfluenceMatrix();
        if (s_bCancel) return true;

        createUnitRHS();
        if (s_bCancel) return true;


        createSourceStrength(0.0, m_vDelta, 1);
        if (s_bCancel) return true;

        for (int nWakeIter = 0; nWakeIter<MaxWakeIter; nWakeIter++)
        {
            if(m_pWPolar->bWakeRollUp())
            {
                str = QString("      Wake iteration %1\n").arg(nWakeIter+1,3);
                traceLog(str);
            }

            if (s_bCancel) return true;

            /** @todo : check... may not be quite correct */
            if(!m_pWPolar->bThinSurfaces())
            {
                //compute wake contribution
                createWakeContribution();
                //add wake contribution to matrix and RHS
                for(int p=0; p<m_MatSize; p++)
                {
                    m_uRHS[p]+= m_uWake[p];
                    m_wRHS[p]+= m_wWake[p];
                    for(int pp=0; pp<m_MatSize; pp++)
                    {
                        m_aij[p*m_MatSize+pp] += m_aijWake[p*m_MatSize+pp];
                    }
                }
            }

            if (s_bCancel) return true;

            if (!solveUnitRHS())
            {
                s_bWarning = true;
                return true;
            }
            if (s_bCancel) return true;

            createDoubletStrength(0.0, m_vDelta, 1);
            if (s_bCancel) return true;

            computeFarField(1.0, 0.0, m_vDelta, 1);
            if (s_bCancel) return true;

            computeBalanceSpeeds(0.0, 0);
            if (s_bCancel) return true;

            scaleResultstoSpeed(1);
            if (s_bCancel) return true;

            computeOnBodyCp(0.0, m_vDelta, 1);
            if (s_bCancel) return true;

//            if(MaxWakeIter>0 && m_pWPolar->bWakeRollUp()) relaxWake();
        }

        switch(m_pWPolar->polarType())
        {
            case xfl::BETAPOLAR:
                computeAeroCoefs(0.0, m_vDelta, 1);
                break;

            case xfl::FIXEDSPEEDPOLAR:
            case xfl::FIXEDLIFTPOLAR:
                computeAeroCoefs(m_vMin, m_vDelta, 1);
                break;

            case xfl::FIXEDAOAPOLAR:
                computeAeroCoefs(m_QInf, m_vDelta, 1);
                break;

            default:
                break;
        }
    }

    //leave things as they were
    memcpy(m_pPanel,         m_pMemPanel,     uint(m_MatSize)    * sizeof(Panel));
    memcpy(m_pNode,          m_pMemNode,      uint(m_nNodes)     * sizeof(Vector3d));
    memcpy(m_pWakePanel,     m_pRefWakePanel, uint(m_WakeSize)   * sizeof(Panel));
    memcpy(m_pWakeNode,      m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));
    memcpy(m_pTempWakeNode,  m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));

    return true;
}


/**
* Returns the perturbation velocity created at a point C by a horseshoe or quad vortex with unit circulation located on a panel pPanel
* @param pPanel a pointer to the Panel where the vortex is located
* @param C the point where the perrturbation is evaluated
* @param V a reference to the resulting perturbation velocity vector
* @param bAll true if the influence of the bound vector should be included. Not necessary in the case of a far-field evaluation.
*/
void PanelAnalysis::VLMGetVortexInfluence(Panel const *pPanel, Vector3d const &C, Vector3d &V, bool bAll) const
{
    int lw=0, pw=0, p=0;
    Vector3d AA1, BB1, VT;

    p = pPanel->m_iElement;

    V.x = V.y = V.z = 0.0;

    if(m_pWPolar->bVLM1())
    {
        //just get the horseshoe vortex's influence
        VLMCmn(pPanel->VA, pPanel->VB, C, V, bAll);
    }
    else
    {
        // we have quad vortices
        // so we follow Katz and Plotkin's lead
        if(!pPanel->m_bIsTrailing)
        {
            if(bAll)
            {
                VLMQmn(pPanel->VA, pPanel->VB, m_pPanel[p-1].VA, m_pPanel[p-1].VB, C, V);
            }
        }
        else
        {
            // then panel p is trailing and shedding a wake
            if(!m_pWPolar->bWakeRollUp())
            {
                // since Panel p+1 does not exist...
                // we define the points AA=A+1 and BB=B+1
                AA1.x = m_pNode[pPanel->m_iTA].x + (m_pNode[pPanel->m_iTA].x-pPanel->VA.x)/3.0;
                AA1.y = m_pNode[pPanel->m_iTA].y;
                AA1.z = m_pNode[pPanel->m_iTA].z;
                BB1.x = m_pNode[pPanel->m_iTB].x + (m_pNode[pPanel->m_iTB].x-pPanel->VB.x)/3.0;
                BB1.y = m_pNode[pPanel->m_iTB].y;
                BB1.z = m_pNode[pPanel->m_iTB].z;

                // first we get the quad vortex's influence
                if (bAll)
                {
                    VLMQmn(pPanel->VA, pPanel->VB, AA1, BB1, C, V);
                }

                //we just add a trailing horseshoe vortex's influence to simulate the wake
                VLMCmn(AA1,BB1,C,VT,bAll);

                V.x += VT.x;
                V.y += VT.y;
                V.z += VT.z;
            }
            else
            {
                // if there is a wake roll-up required
                pw = pPanel->m_iWake;
                // first close the wing's last vortex ring at T.E.
                if (bAll)
                {
                    VLMQmn(pPanel->VA, pPanel->VB, m_pWakePanel[pw].VA, m_pWakePanel[pw].VB, C, V);
                }

                //each wake panel has the same vortex strength than the T.E. panel
                //so we just cumulate their unit influences
                if(bAll)
                {
                    for (lw=0; lw<m_pWPolar->m_NXWakePanels-1; lw++)
                    {
                        VLMQmn(m_pWakePanel[pw  ].VA, m_pWakePanel[pw  ].VB,
                               m_pWakePanel[pw+1].VA, m_pWakePanel[pw+1].VB, C, VT);
                        V += VT;

                        pw++;
                    }
                }
                //
                // For the very last wake panel downstream, just add a horseshoe vortex influence
                //
                // TODO : check influence on results
                //
                //                VLMCmn(m_pWakePanel[pw].A, m_pWakePanel[pw].B,C,VT,bAll);
                //                V += VT;
                //
                // simple really !
                //
            }
        }
    }
}

/**
 * Sets the values of mass, CoG and inertia tensor as a function of the input control parameter
 * The value of a variable is v = Mean_Value + ctrl * Gain
 * Mean_Value is the default value of the variable in the WPolar object
 * Gain is the gain parameter of the variable in the WPolar object
 *
 * @param ctrl
 */
void PanelAnalysis::setInertia(double ctrl, double alpha, double beta)
{
    m_Mass       = m_pWPolar->mass()   + ctrl*m_pWPolar->m_inertiaGain[0];
    m_CoG.x      = m_pWPolar->CoG().x  + ctrl*m_pWPolar->m_inertiaGain[1];
    m_CoG.y      = m_pWPolar->CoG().y;
    m_CoG.z      = m_pWPolar->CoG().z  + ctrl*m_pWPolar->m_inertiaGain[2];
    m_Inertia[0] = m_pWPolar->CoGIxx() + ctrl*m_pWPolar->m_inertiaGain[3];
    m_Inertia[1] = m_pWPolar->CoGIyy() + ctrl*m_pWPolar->m_inertiaGain[4];
    m_Inertia[2] = m_pWPolar->CoGIzz() + ctrl*m_pWPolar->m_inertiaGain[5];
    m_Inertia[3] = m_pWPolar->CoGIxz() + ctrl*m_pWPolar->m_inertiaGain[6];


    m_Ib[0][0] = m_Inertia[0];
    m_Ib[1][1] = m_Inertia[1];
    m_Ib[2][2] = m_Inertia[2];
    m_Ib[0][2] = m_Ib[2][0] = m_Inertia[3];
    m_Ib[1][0] = m_Ib[1][2] = m_Ib[0][1] = m_Ib[2][1] = 0.0;


    QString str;
    str = QString("   Mass=%1 kg").arg(m_Mass, 12,'f',3)+"\n";
    traceLog(str);

    str = "\n   ___Center of Gravity Position - Body axis____\n";
    traceLog(str);
    str = QString("    CoG_x=%1 m").arg(m_CoG.x, 12,'f',4)+"\n";
    traceLog(str);
    str = QString("    CoG_y=%1 m").arg(m_CoG.y, 12,'f',4)+"\n";
    traceLog(str);
    str = QString("    CoG_z=%1 m").arg(m_CoG.z, 12,'f',4)+"\n";
    traceLog(str);

    str = "\n   ___Inertia - Body Axis - CoG Origin____\n";
    traceLog(str);
    str = QString::fromUtf8("    Ibxx=%1 kg.m²").arg(m_Ib[0][0], 12,'g',4);
    traceLog(str+"\n");
    str = QString::fromUtf8("    Ibyy=%1 kg.m²").arg(m_Ib[1][1], 12,'g',4);
    traceLog(str+"\n");
    str = QString::fromUtf8("    Ibzz=%1 kg.m²").arg(m_Ib[2][2], 12,'g',4);
    traceLog(str+"\n");
    str = QString::fromUtf8("    Ibxz=%1 kg.m²").arg(m_Ib[0][2], 12,'g',4);
    traceLog(str+"\n\n");


    // case of tilted geometry, unit loops:
    m_CoG.rotateY(Vector3d(0.0,0.0,0.0), alpha);
    m_CoG.rotateZ(Vector3d(0.0,0.0,0.0), beta);
}


/**
* Performs a stability analysis for each requested position of the control surfaces.
*
*/
bool PanelAnalysis::controlLoop()
{
    //
    //    Loop for each control value
    //          Update the geometry, design variables
    //          Build the influence matrix
    //          Perform LU matrix decomposition
    //          Solve a first time the VLM problem to find the trimmed conditions:
    //              - solve for unit RHS
    //              - iterate to find equilibrium aoa such that Cm=0 in steady level flight or banked turn
    //          Build the rotation matrix from body axes to stability axes
    //          Build the RHS for unit velocity fields in stability axes
    //          Build the RHS for unit control rotations (Normals only) in stability axes
    //          Solve the VLM problem for RHS in stability axes
    //          Compute stability derivatives
    //          Compute control derivatives
    //          Compute inertia in stability axis
    //          Longitudinal stability :
    //             - Build longitudinal state matrix
    //             - Solve for eigenvalues and eigenvectors
    //          Lateral stability
    //             - Build lateral state matrix
    //             - Solve for eigenvalues and eigenvectors
    //          Compute aero coeffs for alpha_eq
    //          Store OpPoint and polar data
    //    end loop
    //
    int i;
    QString str, outString;

    m_Progress = 0.0;

    m_bTrace = true;

    str = QString("   Solving the problem... \n\n");
    traceLog("\n"+str);

    for (i=0; i<m_nRHS; i++)
    {
        // create the geometry for the control parameter
        // so first restore the initial geometry
        memcpy(m_pPanel, m_pMemPanel, uint(m_MatSize) * sizeof(Panel));
        memcpy(m_pNode,  m_pMemNode,  uint(m_nNodes)  * sizeof(Vector3d));

        m_OpAlpha = 0.0;
        //define the control position for this iteration
        m_Ctrl = m_vMin +double(i) *m_vDelta;
        str = QString("      Calculation for control position %1\n").arg(m_Ctrl ,5,'f',2);
        traceLog(str);
        outString.clear();

        setInertia(m_Ctrl, 0.0, 0.0);

        setControlPositions(m_Ctrl, m_NCtrls, outString, true);

        traceLog(outString);
        if(s_bCancel) break;

        // next find the balanced and trimmed conditions
        if(!computeTrimmedConditions())
        {
            if(s_bCancel) break;
            //no zero moment alpha
            str = QString("      Unsuccessful attempt to trim the model for control position=%1 - skipping.\n\n\n").arg(m_Ctrl,5,'f',2);
            traceLog(str);
            s_bWarning = true;
        }
        else
        {
            m_3DQInf[i] = u0;
            m_QInf      = u0;

            if (s_bCancel) return true;

            //Build the rotation matrix from body axes to stability axes
            buildRotationMatrix();
            if(s_bCancel) break;

            // Compute inertia in stability axes
            computeStabilityInertia();
            if(s_bCancel) break;

            str = "\n      ___Inertia - Stability Axis - CoG Origin____\n";
            traceLog(str);
            str = QString("      Isxx=%1 ").arg(m_Is[0][0], 12,'g',4);
            traceLog(str+"\n");
            str = QString("      Isyy=%1 ").arg(m_Is[1][1], 12,'g',4);
            traceLog(str+"\n");
            str = QString("      Iszz=%1 ").arg(m_Is[2][2], 12,'g',4);
            traceLog(str+"\n");
            str = QString("      Isxz=%1 ").arg(m_Is[0][2], 12,'g',4);
            traceLog(str+"\n\n");

            // Compute stability and control derivatives in stability axes
            // viscous or not viscous ?
            computeStabilityDerivatives();
            if(s_bCancel) break;

            computeControlDerivatives(); //single derivative, wrt the polar's control variable
            if(s_bCancel) break;

            computeNDStabDerivatives();

            // Construct the state matrices - longitudinal and lateral
            buildStateMatrices();

            // Solve for eigenvalues
            if(!solveEigenvalues())
            {
                str = QString("      Unsuccessful attempt to compute eigenvalues for Control=%1 - skipping.\n\n\n").arg(m_Ctrl,10,'f',3);
                traceLog(str);
                s_bWarning = true;
            }
            else
            {
                // Compute aero coefficients for trimmed conditions
                computeFarField(m_QInf, m_AlphaEq, 0.0, 1);
                if (s_bCancel) return true;

                computeOnBodyCp(m_AlphaEq, 0.0, 1);
                if (s_bCancel) return true;


                str = QString("      Computing Plane for alpha=%1").arg(m_AlphaEq,7,'f',2);
                str += QString::fromUtf8("°\n");
                traceLog(str);
                computePlane(m_AlphaEq, u0, 0);

                if (s_bCancel) return true;
            }
            str = QString("\n     ______Finished operating point calculation for control position %1________\n\n\n\n\n").arg(m_Ctrl, 5,'f',2);
            traceLog(str);
        }
        if(s_bCancel) break;
    }
    return true;
}



/**
* Extracts the eigenvalues and eigenvectors from the state matrices.
* Stores the results in the memeber variables if successful.
* @return true if the extraction was successful.
*/
bool PanelAnalysis::solveEigenvalues()
{
    // Finds the eigenvalues and eigenvectors of the state matrices ALong and ALat
    double pLong[5], pLat[5];//the coefficients of the characteristic polynomial
    int i;
    QString str;

    CharacteristicPol(m_ALong, pLong);

    if(!LinBairstow(pLong, m_rLong, 4))
    {
        traceLog("\n       Error extracting longitudinal eigenvalues\n");
        return false;
    }

    //sort them
    ComplexSort(m_rLong, 4);

    for(i=0; i<4; i++)
    {
        if(!Eigenvector(m_ALong, m_rLong[i], m_vLong+i*4))
        {
            str = QString("Error extracting longitudinal eigenvector for mode %1\n").arg(i);
            return false;
        }
    }

    str = "\n\n      ___Longitudinal modes____\n\n";
    traceLog(str);

    str = QString("      Eigenvalue:  %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
            .arg(m_rLong[0].real(),9, 'g', 4).arg(m_rLong[0].imag(),9, 'g', 4)
            .arg(m_rLong[1].real(),9, 'g', 4).arg(m_rLong[1].imag(),9, 'g', 4)
            .arg(m_rLong[2].real(),9, 'g', 4).arg(m_rLong[2].imag(),9, 'g', 4)
            .arg(m_rLong[3].real(),9, 'g', 4).arg(m_rLong[3].imag(),9, 'g', 4);
    traceLog(str);
    str=("                    _____________________________________________________________________________________________________\n");
    traceLog(str);

    i=0;
    str = QString("      Eigenvector: %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
            .arg(m_vLong[i].real(),    9, 'g', 4).arg(m_vLong[i].imag(),    9, 'g', 4)
            .arg(m_vLong[i+4].real(),  9, 'g', 4).arg(m_vLong[i+4].imag(),  9, 'g', 4)
            .arg(m_vLong[i+8].real(),  9, 'g', 4).arg(m_vLong[i+8].imag(),  9, 'g', 4)
            .arg(m_vLong[i+12].real(), 9, 'g', 4).arg(m_vLong[i+12].imag(), 9, 'g', 4);
    traceLog(str);
    for (i=1; i<4; i++)
    {
        str = QString("                   %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
                .arg(m_vLong[i].real(),    9, 'g', 4).arg(m_vLong[i].imag(),    9, 'g', 4)
                .arg(m_vLong[i+4].real(),  9, 'g', 4).arg(m_vLong[i+4].imag(),  9, 'g', 4)
                .arg(m_vLong[i+8].real(),  9, 'g', 4).arg(m_vLong[i+8].imag(),  9, 'g', 4)
                .arg(m_vLong[i+12].real(), 9, 'g', 4).arg(m_vLong[i+12].imag(), 9, 'g', 4);
        traceLog(str);
    }
    str = "\n";
    traceLog(str);

    CharacteristicPol(m_ALat, pLat);

    if(!LinBairstow(pLat, m_rLat, 4))
    {
        traceLog("\n       Error extracting lateral eigenvalues\n");
        return false;
    }

    //sort them
    ComplexSort(m_rLat, 4);

    for(i=0; i<4; i++)
    {
        if(!Eigenvector(m_ALat, m_rLat[i], m_vLat+i*4))
        {
            str = QString("Error extracting lateral eigenvector for mode %1\n").arg(i);
            return false;
        }
    }


    str = "\n\n      ___Lateral modes____\n\n";
    traceLog(str);

    str = QString("      Eigenvalue:  %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
            .arg(m_rLat[0].real(),9, 'g', 4).arg(m_rLat[0].imag(),9, 'g', 4)
            .arg(m_rLat[1].real(),9, 'g', 4).arg(m_rLat[1].imag(),9, 'g', 4)
            .arg(m_rLat[2].real(),9, 'g', 4).arg(m_rLat[2].imag(),9, 'g', 4)
            .arg(m_rLat[3].real(),9, 'g', 4).arg(m_rLat[3].imag(),9, 'g', 4);
    traceLog(str);
    str=("                    _____________________________________________________________________________________________________\n");
    traceLog(str);

    i=0;
    str = QString("      Eigenvector: %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
            .arg(m_vLat[i].real(),    9, 'g', 4).arg(m_vLat[i].imag(),    9, 'g', 4)
            .arg(m_vLat[i+4].real(),  9, 'g', 4).arg(m_vLat[i+4].imag(),  9, 'g', 4)
            .arg(m_vLat[i+8].real(),  9, 'g', 4).arg(m_vLat[i+8].imag(),  9, 'g', 4)
            .arg(m_vLat[i+12].real(), 9, 'g', 4).arg(m_vLat[i+12].imag(), 9, 'g', 4);
    traceLog(str);
    for (i=1; i<4; i++)
    {
        str = QString("                   %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
                .arg(m_vLat[i].real(),    9, 'g', 4).arg(m_vLat[i].imag(),    9, 'g', 4)
                .arg(m_vLat[i+4].real(),  9, 'g', 4).arg(m_vLat[i+4].imag(),  9, 'g', 4)
                .arg(m_vLat[i+8].real(),  9, 'g', 4).arg(m_vLat[i+8].imag(),  9, 'g', 4)
                .arg(m_vLat[i+12].real(), 9, 'g', 4).arg(m_vLat[i+12].imag(), 9, 'g', 4);
        traceLog(str);
    }
    str = "\n";
    traceLog(str);

    return true;
}

/**
* Computes the non-dimensional stability derivatives.
* Outputs the results to the log file
*/
void PanelAnalysis::computeNDStabDerivatives()
{
    QString str;
    double b, S, mac, q, theta0, Cw0;
    double rho = m_pWPolar->density();

    q = 1./2. * m_pWPolar->density() * u0 * u0;
    b   = m_pWPolar->referenceSpanLength();
    S   = m_pWPolar->referenceArea();
    mac = m_pPlane->mac();
    theta0 = 0.0;//steady level flight only ?

    Cw0 = m_Mass * 9.81/q/S; //E&R p.127
    //    Cx0 =  Cw0 * sin(theta0); //E&R p.119
    //    Cz0 = -Cw0 * cos(theta0); //E&R p.118

    //E&R p. 118, table 4.4
    CXu = (Xu - rho * u0*S*Cw0*sin(theta0))/(0.5*rho*u0*S);
    CZu = (Zu + rho * u0*S*Cw0*cos(theta0))/(0.5*rho*u0*S);
    Cmu = Mu /(0.5*rho*u0*mac*S);
    CXa = Xw /(0.5*rho*u0*S);
    CZa = Zw /(0.5*rho*u0*S);
    Cma = Mw /(0.5*rho*u0*mac*S);
    CXq = Xq /(.25*rho*u0*mac*S);
    CZq = Zq /(.25*rho*u0*mac*S);
    Cmq = Mq /(.25*rho*u0*mac*mac*S);

    XNP = m_CoG.x + Cma/CZa * mac; //E&R (eq. 2.3.5 p.29)

    CYb = Yv*    u0     /(q*S);
    CYp = Yp* 2.*u0     /(q*S*b);
    CYr = Yr* 2.*u0     /(q*S*b);
    Clb = Lv*    u0     /(q*S*b);
    Clp = Lp*(2.*u0/b)  /(q*S*b);
    Clr = Lr*(2.*u0/b)  /(q*S*b);
    Cnb = Nv*    u0     /(q*S*b);
    Cnp = Np*(2.*u0/b)  /(q*S*b);
    Cnr = Nr*(2.*u0/b)  /(q*S*b);

    CXe = Xde/(q*S);
    CYe = Yde/(q*S);
    CZe = Zde/(q*S);
    Cle = Lde/(q*S*b);
    Cme = Mde/(q*S*mac);
    Cne = Nde/(q*S*b);

    // no OpPoint, we output the data to the log file
    str = "      Longitudinal derivatives\n";
    traceLog(str);
    str = QString("      Xu=%1         Cxu=%2\n").arg(Xu,12,'g',5).arg(CXu, 12, 'g', 5);
    traceLog(str);
    str = QString("      Xw=%1         Cxa=%2\n").arg(Xw,12,'g',5).arg(CXa, 12, 'g', 5);
    traceLog(str);
    str = QString("      Zu=%1         Czu=%2\n").arg(Zu,12,'g',5).arg(CZu, 12, 'g', 5);
    traceLog(str);
    str = QString("      Zw=%1         CLa=%2\n").arg(Zw,12,'g',5).arg(-CZa,12, 'g', 5);
    traceLog(str);
    str = QString("      Zq=%1         CLq=%2\n").arg(Zq,12,'g',5).arg(-CZq,12, 'g', 5);
    traceLog(str);
    str = QString("      Mu=%1         Cmu=%2\n").arg(Mu,12,'g',5).arg(Cmu, 12, 'g', 5);
    traceLog(str);
    str = QString("      Mw=%1         Cma=%2\n").arg(Mw,12,'g',5).arg(Cma, 12, 'g', 5);
    traceLog(str);
    str = QString("      Mq=%1         Cmq=%2\n").arg(Mq,12,'g',5).arg(Cmq, 12, 'g', 5);
    traceLog(str);

    str = QString("      Neutral Point position=%1 m").arg(XNP, 10,'f',5);
    str +="\n\n";
    traceLog(str);

    str = "\n      Lateral derivatives\n";
    traceLog(str);
    str = QString("      Yv=%1         CYb=%2\n").arg(Yv,12,'g',5).arg(CYb,12,'g',5);
    traceLog(str);
    str = QString("      Yp=%1         CYp=%2\n").arg(Yp,12,'g',5).arg(CYp,12,'g',5);
    traceLog(str);
    str = QString("      Yr=%1         CYr=%2\n").arg(Yr,12,'g',5).arg(CYr,12,'g',5);
    traceLog(str);
    str = QString("      Lv=%1         Clb=%2\n").arg(Lv,12,'g',5).arg(Clb,12,'g',5);
    traceLog(str);
    str = QString("      Lp=%1         Clp=%2\n").arg(Lp,12,'g',5).arg(Clp,12,'g',5);
    traceLog(str);
    str = QString("      Lr=%1         Clr=%2\n").arg(Lr,12,'g',5).arg(Clr,12,'g',5);
    traceLog(str);
    str = QString("      Nv=%1         Cnb=%2\n").arg(Nv,12,'g',5).arg(Cnb,12,'g',5);
    traceLog(str);
    str = QString("      Np=%1         Cnp=%2\n").arg(Np,12,'g',5).arg(Cnp,12,'g',5);
    traceLog(str);
    str = QString("      Nr=%1         Cnr=%2\n\n").arg(Nr,12,'g',5).arg(Cnr,12,'g',5);
    traceLog(str);


    //output control derivatives
    bool bActive = false;
    for(int c=0; c<m_NCtrls; c++)
    {
        if(fabs(m_pWPolar->m_ControlGain[c])>PRECISION)
        {
            bActive = true;
            break;
        }
    }
    if(!bActive) return;

    str = QString("      Control derivatives \n");
    traceLog(str);

    str = QString("      Xde=%1        CXde=%2\n").arg(Xde,12,'g',5).arg(Xde/(q*S),12,'g',5);
    traceLog(str);

    str = QString("      Yde=%1        CYde=%2\n").arg(Yde,12,'g',5).arg(Yde/(q*S),12,'g',5);
    traceLog(str);

    str = QString("      Zde=%1        CZde=%2\n").arg(Zde,12,'g',5).arg(Zde/(q*S),12,'g',5);
    traceLog(str);

    str = QString("      Lde=%1        CLde=%2\n").arg(Lde,12,'g',5).arg(Lde/(q*S*b),12,'g',5);
    traceLog(str);

    str = QString("      Mde=%1        CMde=%2\n").arg(Mde,12,'g',5).arg(Mde/(q*S*mac),12,'g',5);
    traceLog(str);

    str = QString("      Nde=%1        CNde=%2\n").arg(Nde,12,'g',5).arg(Nde/(q*S*b),12,'g',5);
    traceLog(str+"\n");

    str ="\n";
    traceLog(str);

}


/**
* Calculates the forces using a farfield method.
* Calculates the moments by a near field method, i.e. direct summation on the panels.
* Downwash is evaluated at a distance 100 times the span downstream (i.e. infinite)
* @param Mu a pointer to the array of doublet strengths or vortex circulations
* @param Sigma a pointer to the array of source strengths
* @param *VInf a pointer to the array of the velocity vectors on the panels
* @param Force the resulting force vector
* @param Moment the resulting moment vector
*/
void PanelAnalysis::forces(double *Mu, double *Sigma, double alpha, Vector3d Vinc, double *VInf, Vector3d &Force, Vector3d &Moment)
{
    if(!m_pPanel || !m_pWPolar) return;

    bool bOutRe(false), bError(false), bOut(false);
    int p(0), pp(0), m(0), nw(0), iTA(0), iTB(0);
    double cosa(0), sina(0), Re(0), PCd(0), Cl(0), Cp(0), tau(0), StripArea(0), ViscousDrag(0), ExtraDrag(0);
    double QInf(0), QInfStrip(0), qdyn(0), GammaStrip(0);
    Vector3d  C, PtC4, LeverArm, WindDirection, WindNormal, PanelLeverArm, Wg;
    Vector3d Velocity, StripForce, ViscousMoment, dF, PanelForce, PanelForcep1;

    int coef = 2;
    if (m_pWPolar->bThinSurfaces()) coef = 1;

    cosa = cos(alpha*PI/180.0);
    sina = sin(alpha*PI/180.0);

    //   Define the wind axis
    WindNormal.set(   -sina, 0.0, cosa);
    WindDirection.set( cosa, 0.0, sina);

    p=m=0;

    Force.set( 0.0, 0.0, 0.0);
    Moment.set(0.0, 0.0, 0.0);
    ViscousDrag = 0.0;
    ViscousMoment.set(0.0,0.0,0.0);

    ExtraDrag = 0.0;

    for(int j=0; j<m_ppSurface->size(); j++)
    {
        if(m_ppSurface->at(j)->m_bIsTipLeft && !m_pWPolar->bThinSurfaces()) p+=m_ppSurface->at(j)->m_NXPanels;//tip patch panels

        for(int k=0; k<m_ppSurface->at(j)->m_NYPanels; k++)
        {
            //Get the strip area
            pp=p;
            StripArea = 0.0;
            for (int l=0; l<coef*m_ppSurface->at(j)->m_NXPanels; l++)
            {
                StripArea  += m_pPanel[pp].Area;
                pp++;
            }

            //Get the strip's lifting force
            if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE)
            {
                StripArea /=2.0;
                //FF force
                nw  = m_pPanel[p].m_iWake;
                iTA = m_pWakePanel[nw].m_iTA;
                iTB = m_pWakePanel[nw].m_iTB;
                C = (m_pWakeNode[iTA] + m_pWakeNode[iTB])/2.0;
                getSpeedVector(C, Mu, Sigma, Wg, false);
                Wg.x += VInf[p            ];
                Wg.y += VInf[p+m_MatSize  ];
                Wg.z += VInf[p+2*m_MatSize];

                GammaStrip = (-Mu[p+coef*m_ppSurface->at(j)->m_NXPanels-1] + Mu[p]) *4.0*PI;

                StripForce  = m_pPanel[p].Vortex * Wg;
                StripForce *= GammaStrip;                            //Newtons/rho
                Force += StripForce;

                Velocity.x = *(VInf               +p);
                Velocity.y = *(VInf +   m_MatSize +p);
                Velocity.z = *(VInf + 2*m_MatSize +p);
                QInfStrip = Velocity.norm(); //used for viscous drag at the next step

                p+=m_ppSurface->at(j)->m_NXPanels*coef;
            }
            else
            {
                //iPos=0, VLM type panel
                StripForce.set(0.0,0.0,0.0);
                for(int l=0; l<m_ppSurface->at(j)->m_NXPanels; l++)
                {
                    Velocity.x = *(VInf               +p);
                    Velocity.y = *(VInf +   m_MatSize +p);
                    Velocity.z = *(VInf + 2*m_MatSize +p);
                    QInfStrip = Velocity.norm();

                    //FF force
                    if(m_pWPolar->bVLM1() || m_pPanel[p].m_bIsTrailing)
                    {
                        C = m_pPanel[p].CtrlPt;
                        C.x = m_pPlane->planformSpan() * 100.0;

                        getSpeedVector(C, Mu, Sigma, Wg, false);

                        // The trailing point sees both the upstream and downstream parts of the trailing vortices
                        // Hence it sees twice the downwash.
                        // So divide by 2 to account for this.
                        Wg *= 1.0/2.0;
                        Wg += Velocity; //total speed vector

                        //induced force
                        dF  = Wg * m_pPanel[p].Vortex;    // Kutta-Joukowski theorem
                        dF *=  Mu[p];       // N/rho

                        Force += dF;        // N/rho
                        StripForce += dF;
                    }

                    //On-Body moment
                    PanelForce  = Velocity * m_pPanel[p].Vortex;
                    PanelForce *= Mu[p];                                 //Newtons/rho

                    if(!m_pWPolar->bVLM1() && !m_pPanel[p].m_bIsLeading)
                    {
                        PanelForcep1  = Velocity * m_pPanel[p].Vortex;
                        PanelForcep1 *= Mu[p+1];                          //Newtons/rho

                        PanelForce -= PanelForcep1;
                    }

                    PanelLeverArm = m_pPanel[p].VortexPos - m_CoG;
                    Moment += PanelLeverArm * PanelForce;                     // N.m/rho
                    p++;
                }

            }
            if(m_pWPolar->bViscous())
            {
                //add the viscous drag component to force and moment
                qdyn = 0.5 * m_pWPolar->density() * QInfStrip * QInfStrip;
                m_ppSurface->at(j)->getC4(k, PtC4, tau);
                Re = m_ppSurface->at(j)->chord(tau) * QInfStrip /m_pWPolar->viscosity();
                Cl = StripForce.dot(WindNormal)*m_pWPolar->density()/qdyn/StripArea;
                PCd    = Wing::getInterpolatedVariable(2, m_ppSurface->at(j)->m_pFoilA, m_ppSurface->at(j)->m_pFoilB, Re, Cl, tau, bOutRe, bError);
                PCd   *= StripArea * 1./2.*QInfStrip*QInfStrip;             // Newtons/rho
                bOut = bOut || bOutRe || bError;
                ViscousDrag += PCd ;                                         // Newtons/rho

                LeverArm   = PtC4 - m_CoG;

                ViscousMoment.x += PCd * (WindDirection.y*LeverArm.z - WindDirection.z*LeverArm.y);   // N.m/rho
                ViscousMoment.y += PCd * (WindDirection.z*LeverArm.x - WindDirection.x*LeverArm.z);
                ViscousMoment.z += PCd * (WindDirection.x*LeverArm.y - WindDirection.y*LeverArm.x);
            }

            m++;
        }
    }

    if(!m_pWPolar->bThinSurfaces())
    {
        //On-Body moment
        // same as before, except that we take into account tip patches
        Vector3d VLocal;
        Moment.set(0.0,0.0,0.0);
        for(p=0; p<m_MatSize; p++)
        {
            Velocity.x = *(VInf               +p);
            Velocity.y = *(VInf +   m_MatSize +p);
            Velocity.z = *(VInf + 2*m_MatSize +p);
            QInf = Velocity.norm();

            getDoubletDerivative(p, Mu, Cp, VLocal, QInf, Velocity.x, Velocity.y, Velocity.z);
            PanelForce = m_pPanel[p].Normal * (-Cp) * m_pPanel[p].Area *1/2.*QInf*QInf;      // Newtons/rho

            PanelLeverArm = m_pPanel[p].CollPt - m_CoG;
            Moment += PanelLeverArm * PanelForce;                     // N.m/rho
        }
    }

    //    if(m_pWPolar->bThinSurfaces()) Force -= WindDirection*Force.dot(WindDirection)/2.0;


    if(m_pWPolar->bViscous())
    {
        Force += WindDirection * ViscousDrag;
        Moment += ViscousMoment;
    }

    Force  *= m_pWPolar->density();                          // N

    for(int iex=0; iex<MAXEXTRADRAG; iex++)
    {
        ExtraDrag += m_pWPolar->m_ExtraDragArea[iex] * m_pWPolar->m_ExtraDragCoef[iex];
    }
    ExtraDrag *= 1./2.*m_pWPolar->density()*Vinc.norm()*Vinc.norm();   // N
    Force += WindDirection*ExtraDrag;

    Moment *= m_pWPolar->density();                          // N.m
}


#define CM_ITER_MAX 50
/**
* Finds the zero-pitching-moment aoa such that Cm=0.
* Proceeds by iteration between -PI/4 and PI/4
* @return true if an equlibrium angle was found false otherwise.
*/
bool PanelAnalysis::getZeroMomentAngle()
{
    double tmp=0;
    double eps = 1.e-7;

    int iter = 0;
    double a0 = -PI/4.0;
    double a1 =  PI/4.0;

    double a = 0.0;
    double Cm0 = computeCm(a0*180.0/PI);
    double Cm1 = computeCm(a1*180.0/PI);
    double Cm = 1.0;

    //are there two initial values of opposite signs ?
    while(Cm0*Cm1>0.0 && iter <20)
    {
        a0 *=0.9;
        a1 *=0.9;
        Cm0 = computeCm(a0*180.0/PI);
        Cm1 = computeCm(a1*180.0/PI);
        iter++;
        if(s_bCancel) break;
    }
    if(iter>=20 || s_bCancel) return false;

    iter = 0;

    //Cm0 and Cm1 are of opposite sign
    if(Cm0>Cm1)
    {
        tmp = Cm1;
        Cm1 = Cm0;
        Cm0 = tmp;
        tmp = a0;
        a0  =  a1;
        a1  = tmp;
    }

    while (qAbs(Cm)>eps && iter<=CM_ITER_MAX)
    {
        a = a0 - (a1-a0) * Cm0/(Cm1-Cm0);
        Cm = computeCm(a*180.0/PI);
        if(Cm>0.0)
        {
            a1  = a;
            Cm1 = Cm;
        }
        else
        {
            a0  = a;
            Cm0 = Cm;
        }
        iter++;
        if(s_bCancel) break;
    }

    if(iter>=CM_ITER_MAX || s_bCancel) return false;

    m_AlphaEq = a*180.0/PI;
    //    Cm = computeCm(m_AlphaEq);// for information only, should be zero

    return true;
}


/**
 * Creates the longitudinal and lateral state matrices
 * from the derivatives and inertias calculated previously

 * Creates the control state matrix from the control derivatives
*/
void PanelAnalysis::buildStateMatrices()
{
    QString strange;

    //use inertia measured in stability axis, CoG origin
    double Ixx = m_Is[0][0];
    double Iyy = m_Is[1][1];
    double Izz = m_Is[2][2];
    double Izx = m_Is[0][2];

    //____________________Longitudinal stability_____________
    m_ALong[0][0] = Xu/m_Mass;
    m_ALong[0][1] = Xw/m_Mass;
    m_ALong[0][2] = 0.0;
    m_ALong[0][3] = -9.81*cos(Theta0*PI/180.0);

    m_ALong[1][0] =  Zu                              /(m_Mass-Zwp);
    m_ALong[1][1] =  Zw                              /(m_Mass-Zwp);
    m_ALong[1][2] = (Zq+m_Mass*u0)                   /(m_Mass-Zwp);
    m_ALong[1][3] = -9.81*m_Mass*sin(Theta0*PI/180.0)/(m_Mass-Zwp);

    m_ALong[2][0] = (Mu + Mwp*Zu/(m_Mass-Zwp))                   /Iyy;
    m_ALong[2][1] = (Mw + Mwp*Zw/(m_Mass-Zwp))                   /Iyy;
    m_ALong[2][2] = (Mq + Mwp*(Zq+m_Mass*u0)/(m_Mass-Zwp))       /Iyy;
    m_ALong[2][3] = (Mwp*(-m_Mass*9.81*sin(Theta0))/(m_Mass-Zwp))/Iyy;

    m_ALong[3][0] = 0.0;
    m_ALong[3][1] = 0.0;
    m_ALong[3][2] = 1.0;
    m_ALong[3][3] = 0.0;

    strange = "      _____State matrices__________\n";
    traceLog(strange);
    strange = "       Longitudinal state matrix\n";
    traceLog(strange);
    for (int i=0; i<4; i++)
    {
        strange = QString("        %1      %2      %3      %4\n")
                .arg(m_ALong[i][0], 14, 'g', 6)
                .arg(m_ALong[i][1], 14, 'g', 6)
                .arg(m_ALong[i][2], 14, 'g', 6)
                .arg(m_ALong[i][3], 14, 'g', 6);
        traceLog(strange);
    }

    //____________________Lateral stability_____________
    double Ipxx = (Ixx * Izz - Izx*Izx)/Izz;
    double Ipzz = (Ixx * Izz - Izx*Izx)/Ixx;
    double Ipzx =  Izx/(Ixx * Izz - Izx*Izx);

    m_ALat[0][0] = Yv/m_Mass;
    m_ALat[0][1] = Yp/m_Mass;
    m_ALat[0][2] = Yr/m_Mass - u0;
    m_ALat[0][3] = 9.81 * cos(Theta0*PI/180.0);

    m_ALat[1][0] = Lv/Ipxx+Ipzx*Nv;
    m_ALat[1][1] = Lp/Ipxx+Ipzx*Np;
    m_ALat[1][2] = Lr/Ipxx+Ipzx*Nr;
    m_ALat[1][3] = 0.0;

    m_ALat[2][0] = Lv*Ipzx+ Nv/Ipzz;
    m_ALat[2][1] = Lp*Ipzx+ Np/Ipzz;
    m_ALat[2][2] = Lr*Ipzx+ Nr/Ipzz;
    m_ALat[2][3] = 0.0;

    m_ALat[3][0] = 0.0;
    m_ALat[3][1] = 1.0;
    m_ALat[3][2] = tan(Theta0*PI/180.0);
    m_ALat[3][3] = 0.0;

    strange = "       Lateral state matrix\n";
    traceLog(strange);
    for (int i=0; i<4; i++)
    {
        strange = QString("        %1      %2      %3      %4\n")
                .arg(m_ALat[i][0], 14, 'g', 6)
                .arg(m_ALat[i][1], 14, 'g', 6)
                .arg(m_ALat[i][2], 14, 'g', 6)
                .arg(m_ALat[i][3], 14, 'g', 6);
        traceLog(strange);
    }

    strange ="\n";
    traceLog(strange);

    bool bActive = false;
    for(int c=0; c<m_NCtrls; c++)
    {
        if(fabs(m_pWPolar->m_ControlGain[c])>PRECISION)
        {
            bActive = true;
            break;
        }
    }
    if(!bActive)
    {
        m_BLong[0] = m_BLong[1] = m_BLong[2] = m_BLong[3] = 0.0;
        m_BLat[0]  = m_BLat[1]  = m_BLat[2]  = m_BLat[3]  = 0.0;
    }
    else
    {
        //build the control matrix
        // per radian
        m_BLong[0] = Xde/m_Mass;
        m_BLong[1] = Zde/m_Mass;
        m_BLong[2] = Mde/Iyy;
        m_BLong[3] = 0.0;

        m_BLat[0] = Yde/m_Mass;
        m_BLat[1] = Lde/Ipxx+Nde*Ipzx;
        m_BLat[2] = Lde*Ipzx+Nde/Ipzz;
        m_BLat[3] = 0.0;

        strange = "      _____Control Matrices__________\n";
        traceLog(strange);
        strange = "       Longitudinal control matrix\n";
        traceLog(strange);

        strange = QString("      %1\n      %2\n      %3\n      %4\n\n")
                .arg(m_BLong[0], 13, 'g', 7)
                .arg(m_BLong[1], 13, 'g', 7)
                .arg(m_BLong[2], 13, 'g', 7)
                .arg(m_BLong[3], 13, 'g', 7);
        traceLog(strange);
        strange = "       Lateral control matrix\n";
        traceLog(strange);

        strange = QString("      %1\n      %2\n      %3\n      %4\n\n")
                .arg(m_BLat[0], 13, 'g', 7)
                .arg(m_BLat[1], 13, 'g', 7)
                .arg(m_BLat[2], 13, 'g', 7)
                .arg(m_BLat[3], 13, 'g', 7);
        traceLog(strange);
    }

}


/**
 *  This methods builds the rotation matrix from geometrical axis to stability axis,
 *  i.e. if V is a vector defined in the body axis, its coordinates in stability axis are v = R.V
 *  In XFLR5, the x-body-axis is pointing backward, and the convention
 *  for stability axes is with the x-axis pointing forward.
 *  The rotation matrix is set in accordance.
*/
void PanelAnalysis::buildRotationMatrix()
{
    m_R[0][0] = -cos(m_AlphaEq*PI/180.0);
    m_R[1][0] =  0.0;
    m_R[2][0] =  sin(m_AlphaEq*PI/180.0);
    m_R[0][1] =  0.0;
    m_R[1][1] =  1.0;
    m_R[2][1] =  0.0;
    m_R[0][2] = -sin(m_AlphaEq*PI/180.0);
    m_R[1][2] =  0.0;
    m_R[2][2] = -cos(m_AlphaEq*PI/180.0);
}


/**
 * Computes the trimmed condition for a stability analysis
 * Method :
 *   - For level flight, find the a.o.a. such that Cm=0
 *   - Set trimmed parameters for level flight or other
*/
bool PanelAnalysis::computeTrimmedConditions()
{
    QString strong, strange;

    double Lift(0), phi(0);
    Vector3d VInf, Force, Moment, WindNormal;

    // find aoa such that Cm=0;

    //Build the unit RHS vectors along x and z in Body Axis
    createUnitRHS();
    if (s_bCancel) return false;

    // build the influence matrix in Body Axis
    buildInfluenceMatrix();
    if (s_bCancel) return false;

    if(!m_pWPolar->bThinSurfaces())
    {
        //compute wake contribution
        createWakeContribution();
        //add wake contribution to matrix and RHS
        for(int p=0; p<m_MatSize; p++)
        {
            m_uRHS[p]+= m_uWake[p];
            m_wRHS[p]+= m_wWake[p];
            for(int pp=0; pp<m_MatSize; pp++)
            {
                m_aij[p*m_MatSize+pp] += m_aijWake[p*m_MatSize+pp];
            }
        }
    }

    if (!solveUnitRHS())    //solve for the u,w unit vectors
    {
        s_bWarning = true;
        return false;
    }

    strong ="      Searching for zero-moment angle... ";

    if(!getZeroMomentAngle())
    {
        strong += "none found\n";
        traceLog(strong);
        return false;
    }

    strong += QString("Alpha=%1").arg(m_AlphaEq,0,'f',5) + QChar(0260) +"\n";
    traceLog(strong);

    createSourceStrength(m_AlphaEq, 0.0, 1);
    if (s_bCancel) return true;

    //reconstruct doublet strengths from unit cosine and sine vectors
    createDoubletStrength(m_AlphaEq, 0.0, 1.0);
    if(s_bCancel) return false;

    //______________________________________________________________________________________
    // Calculate the trimmed conditions for this control setting and calculated Alpha_eq
    /*        phi = bank angle
        V   = sqrt(2 m g / rho S CL cos(phi))    (airspeed)
        R   = V^2 / g tan(phi)        (turn radius, positive for right turn)
        W   = V / R                   (turn rate, positive for right turn)
        p   = 0                       (roll rate, zero for steady turn)
        q   = W sin(phi)              (pitch rate, positive nose upward)
        r   = W cos(phi)              (yaw rate, positive for right turn) */

    //so far we have a unit Vortex Strength
    // find the speed which will create a lift equal to the weight

    traceLog("      Calculating speed to balance the weight...");

    WindNormal.set(-sin(m_AlphaEq*PI/180.0), 0.0, cos(m_AlphaEq*PI/180.0));
    VInf.set(       cos(m_AlphaEq*PI/180.0), 0.0, sin(m_AlphaEq*PI/180.0));
    for(int p=0; p<m_MatSize; p++)
    {
        m_RHS[50*m_MatSize+p] = VInf.x;
        m_RHS[51*m_MatSize+p] = VInf.y;
        m_RHS[52*m_MatSize+p] = VInf.z;
    }

    u0 = 1.0;

    //extra drag is useless when calculating lifting velocity
    forces(m_Mu, m_Sigma, m_AlphaEq, Vector3d(0.0,0.0,0.0), m_RHS+50*m_MatSize, Force, Moment);

    phi = m_pWPolar->m_BankAngle *PI/180.0;
    Lift   = Force.dot(WindNormal);        //N/rho ; bank effect not included
    //    VerticalCl = Lift*2.0/m_pWPolar->referenceArea() * cos(phi)/m_pWPolar->density();
    if(Lift<=0.0)
    {
        u0 = -100.0;
        strong = QString("  Found a negative lift for Alpha=%1.... skipping the angle...\n").arg(m_AlphaEq,0,'f',5);
        if(m_bTrace) traceLog(strong);
        m_bPointOut = true;
        s_bWarning = true;
        return false;
    }
    else
    {
        //        u0 =  sqrt( 2.0* 9.81 * m_Mass /m_pWPolar->density()/m_pWPolar->referenceArea() / VerticalCl );
        u0 = sqrt(9.81 * m_Mass / Force.z);
        strong = QString("VInf = %1 m/s").arg(u0,0,'f',5);
        strong+= strange + "\n";
        if(m_bTrace) traceLog(strong);

        if(qAbs(m_pWPolar->m_BankAngle)>PRECISION)
        {
            double radius = u0*u0/9.81/tan(phi);
            double W      = u0/radius;
            double p      = 0.0;
            double q      = W * sin(phi);
            double r      = W * cos(phi);

            strong = QString("      Phi         =%1").arg(m_pWPolar->m_BankAngle,5,'f',2);
            strong += QChar(0260);
            if(m_bTrace) traceLog(strong);

            strong = QString("      Turn radius =%1").arg(radius,5,'f',2);
            if(m_bTrace) traceLog(strong);

            strong = QString("      Turn rate   =%1").arg(W,5,'f',2);
            if(m_bTrace) traceLog(strong);

            strong = QString("      Roll rate   =%1").arg(p,5,'f',2);
            if(m_bTrace) traceLog(strong);

            strong = QString("      Pitch rate  =%1").arg(q,5,'f',2);
            if(m_bTrace) traceLog(strong);

            strong = QString("      Yaw rate    =%1").arg(r,5,'f',2);
            if(m_bTrace) traceLog(strong);
        }
    }

    //______________________________________________________________________________________
    // Scale circulations to speeds

    for(int p=0; p<m_MatSize; p++)
    {
        m_Mu[p]    *= u0;
        m_Sigma[p] *= u0;
    }
    VInf *= u0;

    // Store the force and moment acting on the surfaces
    // Will be of use later for stability and control derivatives
    // need to re-calculate because of viscous drag which is not linear

    for(int p=0; p<m_MatSize; p++)
    {
        m_RHS[50*m_MatSize+p] = VInf.x;
        m_RHS[51*m_MatSize+p] = VInf.y;
        m_RHS[52*m_MatSize+p] = VInf.z;
    }

    // Force0 and Moment0 are the reference values for differentiation
    // Stability derivatives are inviscid
    forces(m_Mu, m_Sigma, m_AlphaEq, VInf, m_RHS+50*m_MatSize, Force0, Moment0);

    return true;
}


/**
* Calculates the stability derivatives.
* @todo implement automatic differentiation. Considerable task.
 The stability derivatives are estimated by forward difference at U=(U0,0,0).
 The reference condition has been saved during the calculation of the trimmed condition.
*/
void PanelAnalysis::computeStabilityDerivatives()
{
    Vector3d Forcem, Momentm, Vim, Vjm, Vkm, Rism, Rjsm, Rksm;
    Vector3d Forcep, Momentp, Vip, Vjp, Vkp, Risp, Rjsp, Rksp;
    Vector3d V0, is, js, ks, CGM, WindDirection, WindNormal;

    double alpha(0), sina(0), cosa(0), deltaspeed(0), deltarotation(0);
    QString strong;
    // Compute stability and control derivatives
    Xu = Xw = Zu = Zw = Mu = Mw = Mq = Zwp = Mwp = 0.0;
    Yv = Yp = Yr = Lv = Lp = Lr = Nv = Np  = Nr  = 0.0;

    int Size= m_MatSize;
    //    if(m_b3DSymetric) Size = m_SymSize;

    strong = "      Calculating the stability derivatives\n";
    traceLog(strong);

    deltaspeed    = 0.001;         //  m/s   for forward difference estimation
    deltarotation = 0.001;        //  rad/s for forward difference estimation

    // Define the stability axes
    cosa = cos(m_AlphaEq*PI/180);
    sina = sin(m_AlphaEq*PI/180);
    WindDirection.set(cosa, 0.0, sina);
    WindNormal.set(-sina, 0.0, cosa);

    is.set(-cosa, 0.0, -sina);
    js.set(  0.0, 1.0,   0.0);
    ks.set( sina, 0.0, -cosa);

    V0 = is * (-u0); //is the steady state velocity vector, if no sideslip

    //______________________________________________________________________________
    // RHS for unit speed vectors
    // The change in wind velocity is opposite to the change in plane velocity
    Vim = V0 - is * deltaspeed; //a positive increase in axial speed is a positive increase in wind speed
    Vjm = V0 - js * deltaspeed; //a plane movement to the right is a wind flow to the left, i.e. negative y
    Vkm = V0 - ks * deltaspeed; //a plane movement downwards (Z_stability>0) is a positive increase of V in geometry axes
    Vip = V0 + is * deltaspeed; //a positive increase in axial speed is a positive increase in wind speed
    Vjp = V0 + js * deltaspeed; //a plane movement to the right is a wind flow to the left, i.e. negative y
    Vkp = V0 + ks * deltaspeed; //a plane movement downwards (Z_stability>0) is a positive increase of V in geometry axes

    strong = "         Creating the RHS translation vectors\n";
    traceLog(strong);

    for (int p=0; p<m_MatSize; p++)
    {
        //re-use existing memory to define the velocity field
        m_RHS[50*m_MatSize+p] = Vip.x;
        m_RHS[51*m_MatSize+p] = Vip.y;
        m_RHS[52*m_MatSize+p] = Vip.z;
        m_RHS[53*m_MatSize+p] = Vjp.x;
        m_RHS[54*m_MatSize+p] = Vjp.y;
        m_RHS[55*m_MatSize+p] = Vjp.z;
        m_RHS[56*m_MatSize+p] = Vkp.x;
        m_RHS[57*m_MatSize+p] = Vkp.y;
        m_RHS[58*m_MatSize+p] = Vkp.z;

        m_RHS[60*m_MatSize+p] = Vim.x;
        m_RHS[61*m_MatSize+p] = Vim.y;
        m_RHS[62*m_MatSize+p] = Vim.z;
        m_RHS[63*m_MatSize+p] = Vjm.x;
        m_RHS[64*m_MatSize+p] = Vjm.y;
        m_RHS[65*m_MatSize+p] = Vjm.z;
        m_RHS[66*m_MatSize+p] = Vkm.x;
        m_RHS[67*m_MatSize+p] = Vkm.y;
        m_RHS[68*m_MatSize+p] = Vkm.z;

        if(!m_pWPolar->bThinSurfaces())
        {
            m_Sigma[p]             = -1.0/4.0/PI* (  m_RHS[50*m_MatSize+p] *m_pPanel[p].Normal.x
                    + m_RHS[51*m_MatSize+p] *m_pPanel[p].Normal.y
                    + m_RHS[52*m_MatSize+p] *m_pPanel[p].Normal.z);
            m_Sigma[p+ m_MatSize]  = -1.0/4.0/PI* (  m_RHS[53*m_MatSize+p] *m_pPanel[p].Normal.x
                    + m_RHS[54*m_MatSize+p] *m_pPanel[p].Normal.y
                    + m_RHS[55*m_MatSize+p] *m_pPanel[p].Normal.z);
            m_Sigma[p+2*m_MatSize] = -1.0/4.0/PI* (  m_RHS[56*m_MatSize+p] *m_pPanel[p].Normal.x
                    + m_RHS[57*m_MatSize+p] *m_pPanel[p].Normal.y
                    + m_RHS[58*m_MatSize+p] *m_pPanel[p].Normal.z);
            m_Sigma[p+3*m_MatSize] = -1.0/4.0/PI* (  m_RHS[60*m_MatSize+p] *m_pPanel[p].Normal.x
                    + m_RHS[61*m_MatSize+p] *m_pPanel[p].Normal.y
                    + m_RHS[62*m_MatSize+p] *m_pPanel[p].Normal.z);
            m_Sigma[p+4*m_MatSize] = -1.0/4.0/PI* (  m_RHS[63*m_MatSize+p] *m_pPanel[p].Normal.x
                    + m_RHS[64*m_MatSize+p] *m_pPanel[p].Normal.y
                    + m_RHS[65*m_MatSize+p] *m_pPanel[p].Normal.z);
            m_Sigma[p+5*m_MatSize] = -1.0/4.0/PI* (  m_RHS[66*m_MatSize+p] *m_pPanel[p].Normal.x
                    + m_RHS[67*m_MatSize+p] *m_pPanel[p].Normal.y
                    + m_RHS[68*m_MatSize+p] *m_pPanel[p].Normal.z);
        }
    }

    createRHS(m_uRHS, Vip);
    createRHS(m_vRHS, Vjp);
    createRHS(m_wRHS, Vkp);
    createRHS(m_pRHS, Vim);
    createRHS(m_qRHS, Vjm);
    createRHS(m_rRHS, Vkm);

    if(!m_pWPolar->bThinSurfaces())
    {
        // As long as the wing has a thin trailing edge, this contribution is zero anyway
        // Compute the wake's contribution
        // We ignore the perturbations and consider only the potential of the steady state flow
        // Clearly an approximation which is also implicit in the VLM formulation
        createWakeContribution(m_uWake,  WindDirection);// re-use m_uWake memory, which is re-calculated anyway at the next control iteration

        //add wake contribution to all 6 RHS
        for(int p=0; p<m_MatSize; p++)
        {
            m_uRHS[p]+= m_uWake[p]*u0;
            m_vRHS[p]+= m_uWake[p]*u0;
            m_wRHS[p]+= m_uWake[p]*u0;

            m_pRHS[p]+= m_uWake[p]*u0;
            m_qRHS[p]+= m_uWake[p]*u0;
            m_rRHS[p]+= m_uWake[p]*u0;
        }
    }

    strong = "         LU solving for RHS - longitudinal\n";
    traceLog(strong);

    Crout_LU_with_Pivoting_Solve(m_aij, m_uRHS, m_Index, m_RHS+0*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_vRHS, m_Index, m_RHS+1*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_wRHS, m_Index, m_RHS+2*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_pRHS, m_Index, m_RHS+3*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_qRHS, m_Index, m_RHS+4*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_rRHS, m_Index, m_RHS+5*m_MatSize, Size, &s_bCancel);

    memcpy(m_uRHS, m_RHS+0*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_vRHS, m_RHS+1*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_wRHS, m_RHS+2*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_pRHS, m_RHS+3*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_qRHS, m_RHS+4*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_rRHS, m_RHS+5*m_MatSize, uint(m_MatSize)*sizeof(double));

    //________________________________________________
    // 1st ORDER STABILITY DERIVATIVES
    // x-derivatives________________________

    strong = "         Calculating forces and derivatives - lateral\n";
    traceLog(strong);

    alpha = atan2(Vim.z, Vim.x) * 180.0/PI;// =m_AlphaEq....
    alpha = m_AlphaEq;

    forces(m_uRHS, m_Sigma+0*m_MatSize, alpha, Vim, m_RHS+50*m_MatSize, Forcep, Momentp);
    forces(m_pRHS, m_Sigma+3*m_MatSize, alpha, Vim, m_RHS+60*m_MatSize, Forcem, Momentm);
    Xu = (Forcem - Forcep).dot(is)   /deltaspeed/2.0;
    Zu = (Forcem - Forcep).dot(ks)   /deltaspeed/2.0;
    Mu = (Momentm- Momentp).dot(js)  /deltaspeed/2.0;

    // y-derivatives________________________
    alpha = atan2(Vjm.z, Vjm.x)*180.0/PI;// =m_AlphaEq....
    alpha = m_AlphaEq;
    forces(m_vRHS, m_Sigma+1*m_MatSize, alpha, V0, m_RHS+53*m_MatSize, Forcep, Momentp);
    forces(m_qRHS, m_Sigma+4*m_MatSize, alpha, V0, m_RHS+63*m_MatSize, Forcem, Momentm);
    Yv = (Forcem - Forcep).dot(js)   /deltaspeed/2.0;
    Lv = (Momentm - Momentp).dot(is) /deltaspeed/2.0;
    Nv = (Momentm - Momentp).dot(ks) /deltaspeed/2.0;

    // z-derivatives________________________
    alpha = atan2(Vkm.z, Vkm.x)* 180.0/PI;
    alpha = m_AlphaEq;
    forces(m_wRHS, m_Sigma+2*m_MatSize, alpha, V0, m_RHS+56*m_MatSize, Forcep, Momentp);
    forces(m_rRHS, m_Sigma+5*m_MatSize, alpha, V0, m_RHS+66*m_MatSize, Forcem, Momentm);
    Xw = (Forcem - Forcep).dot(is)   /deltaspeed/2.0;
    Zw = (Forcem - Forcep).dot(ks)   /deltaspeed/2.0;
    Mw = (Momentm - Momentp).dot(js) /deltaspeed/2.0;

    m_Progress +=1;

    //______________________________________________________________________________
    // RHS for unit rotation vectors around Stability axis
    // stability axis origin is CoG

    strong = "         Creating the RHS rotation vectors\n";
    traceLog(strong);

    for (int p=0; p<m_MatSize; p++)
    {
        //re-use existing memory to define the velocity field
        if(m_pPanel[p].m_Pos==xfl::MIDSURFACE) CGM = m_pPanel[p].VortexPos - m_CoG;
        else                              CGM = m_pPanel[p].CollPt    - m_CoG;

        // a rotation of the plane about a vector is the opposite of a rotation of the freestream about this vector
        Risp = is*CGM * (+deltarotation) + V0;
        Rjsp = js*CGM * (+deltarotation) + V0;
        Rksp = ks*CGM * (+deltarotation) + V0;
        Rism = is*CGM * (-deltarotation) + V0;
        Rjsm = js*CGM * (-deltarotation) + V0;
        Rksm = ks*CGM * (-deltarotation) + V0;

        m_RHS[50*m_MatSize+p] = Risp.x;
        m_RHS[51*m_MatSize+p] = Risp.y;
        m_RHS[52*m_MatSize+p] = Risp.z;
        m_RHS[53*m_MatSize+p] = Rjsp.x;
        m_RHS[54*m_MatSize+p] = Rjsp.y;
        m_RHS[55*m_MatSize+p] = Rjsp.z;
        m_RHS[56*m_MatSize+p] = Rksp.x;
        m_RHS[57*m_MatSize+p] = Rksp.y;
        m_RHS[58*m_MatSize+p] = Rksp.z;

        m_RHS[60*m_MatSize+p] = Rism.x;
        m_RHS[61*m_MatSize+p] = Rism.y;
        m_RHS[62*m_MatSize+p] = Rism.z;
        m_RHS[63*m_MatSize+p] = Rjsm.x;
        m_RHS[64*m_MatSize+p] = Rjsm.y;
        m_RHS[65*m_MatSize+p] = Rjsm.z;
        m_RHS[66*m_MatSize+p] = Rksm.x;
        m_RHS[67*m_MatSize+p] = Rksm.y;
        m_RHS[68*m_MatSize+p] = Rksm.z;

        if(!m_pWPolar->bThinSurfaces())
        {
            m_Sigma[p+0*m_MatSize] = -1.0/4.0/PI* (  m_RHS[50*m_MatSize+p]*m_pPanel[p].Normal.x
                    + m_RHS[51*m_MatSize+p]*m_pPanel[p].Normal.y
                    + m_RHS[52*m_MatSize+p]*m_pPanel[p].Normal.z);
            m_Sigma[p+1*m_MatSize] = -1.0/4.0/PI* (  m_RHS[53*m_MatSize+p]*m_pPanel[p].Normal.x
                    + m_RHS[54*m_MatSize+p]*m_pPanel[p].Normal.y
                    + m_RHS[55*m_MatSize+p]*m_pPanel[p].Normal.z);
            m_Sigma[p+2*m_MatSize] = -1.0/4.0/PI* (  m_RHS[56*m_MatSize+p]*m_pPanel[p].Normal.x
                    + m_RHS[57*m_MatSize+p]*m_pPanel[p].Normal.y
                    + m_RHS[58*m_MatSize+p]*m_pPanel[p].Normal.z);

            m_Sigma[p+3*m_MatSize] = -1.0/4.0/PI* (  m_RHS[60*m_MatSize+p]*m_pPanel[p].Normal.x
                    + m_RHS[61*m_MatSize+p]*m_pPanel[p].Normal.y
                    + m_RHS[62*m_MatSize+p]*m_pPanel[p].Normal.z);
            m_Sigma[p+4*m_MatSize] = -1.0/4.0/PI* (  m_RHS[63*m_MatSize+p]*m_pPanel[p].Normal.x
                    + m_RHS[64*m_MatSize+p]*m_pPanel[p].Normal.y
                    + m_RHS[65*m_MatSize+p]*m_pPanel[p].Normal.z);
            m_Sigma[p+5*m_MatSize] = -1.0/4.0/PI* (  m_RHS[66*m_MatSize+p]*m_pPanel[p].Normal.x
                    + m_RHS[67*m_MatSize+p]*m_pPanel[p].Normal.y
                    + m_RHS[68*m_MatSize+p]*m_pPanel[p].Normal.z);
        }
    }

    createRHS(m_uRHS, WindDirection, m_RHS+50*m_MatSize);
    createRHS(m_vRHS, WindDirection, m_RHS+53*m_MatSize);
    createRHS(m_wRHS, WindDirection, m_RHS+56*m_MatSize);

    createRHS(m_pRHS, WindDirection, m_RHS+60*m_MatSize);
    createRHS(m_qRHS, WindDirection, m_RHS+63*m_MatSize);
    createRHS(m_rRHS, WindDirection, m_RHS+66*m_MatSize);

    if(!m_pWPolar->bThinSurfaces())
    {
        // Compute the wake's contribution
        // We ignore the perturbations and consider only the potential of the steady state flow
        // Clearly an approximation which is also implicit in the VLM formulation
        createWakeContribution(m_uWake,  WindDirection);// re-use m_uWake memory, which is re-calculated anyway at the next control iteration

        //add wake contribution to all 6 RHS
        for(int p=0; p<m_MatSize; p++)
        {
            m_uRHS[p]+= m_uWake[p]*u0;
            m_vRHS[p]+= m_uWake[p]*u0;
            m_wRHS[p]+= m_uWake[p]*u0;

            m_pRHS[p]+= m_uWake[p]*u0;
            m_qRHS[p]+= m_uWake[p]*u0;
            m_rRHS[p]+= m_uWake[p]*u0;
        }
    }

    // The LU matrix is unchanged, so back-substitute for unit vortex circulations

    strong = "         LU solving for RHS - lateral\n";
    traceLog(strong);

    Crout_LU_with_Pivoting_Solve(m_aij, m_uRHS, m_Index, m_RHS+0*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_vRHS, m_Index, m_RHS+1*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_wRHS, m_Index, m_RHS+2*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_pRHS, m_Index, m_RHS+3*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_qRHS, m_Index, m_RHS+4*m_MatSize, Size, &s_bCancel);
    Crout_LU_with_Pivoting_Solve(m_aij, m_rRHS, m_Index, m_RHS+5*m_MatSize, Size, &s_bCancel);

    memcpy(m_uRHS, m_RHS+0*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_vRHS, m_RHS+1*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_wRHS, m_RHS+2*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_pRHS, m_RHS+3*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_qRHS, m_RHS+4*m_MatSize, uint(m_MatSize)*sizeof(double));
    memcpy(m_rRHS, m_RHS+5*m_MatSize, uint(m_MatSize)*sizeof(double));


    //________________________________________________
    // 1st ORDER STABILITY DERIVATIVES

    strong = "         Calculating forces and derivatives - lateral\n";
    traceLog(strong);

    // p-derivatives
    forces(m_uRHS, m_Sigma+0*m_MatSize, m_AlphaEq, V0, m_RHS+50*m_MatSize, Forcep, Momentp);
    forces(m_pRHS, m_Sigma+3*m_MatSize, m_AlphaEq, V0, m_RHS+60*m_MatSize, Forcem, Momentm);
    Yp = (Forcem-Forcep).dot(js)   /deltarotation/2.0;
    Lp = (Momentm-Momentp).dot(is) /deltarotation/2.0;
    Np = (Momentm-Momentp).dot(ks) /deltarotation/2.0;

    // q-derivatives
    forces(m_vRHS, m_Sigma+1*m_MatSize, m_AlphaEq, V0, m_RHS+53*m_MatSize, Forcep, Momentp);
    forces(m_qRHS, m_Sigma+4*m_MatSize, m_AlphaEq, V0, m_RHS+63*m_MatSize, Forcem, Momentm);
    Xq = (Forcem-Forcep).dot(is)   /deltarotation/2.0;
    Zq = (Forcem-Forcep).dot(ks)   /deltarotation/2.0;
    Mq = (Momentm-Momentp).dot(js) /deltarotation/2.0;

    // r-derivatives
    forces(m_wRHS, m_Sigma+2*m_MatSize, m_AlphaEq, V0, m_RHS+56*m_MatSize, Forcep, Momentp);
    forces(m_rRHS, m_Sigma+5*m_MatSize, m_AlphaEq, V0, m_RHS+66*m_MatSize, Forcem, Momentm);
    Yr = (Forcem-Forcep).dot(js)   /deltarotation/2.0;
    Lr = (Momentm-Momentp).dot(is) /deltarotation/2.0;
    Nr = (Momentm-Momentp).dot(ks) /deltarotation/2.0;

    m_Progress +=1;

    //________________________________________________
    // 2nd ORDER STABILITY DERIVATIVES
    // Zwp & Mwp ... ?
    // M. Drela's answer to the question posted on Yahoo Groups:

    /*    May I take this opportunity to ask you about the stability derivatives w.r.t. alpha dot ?
    Are they ignored in AVL and if so, is it a safe assumption ?

    Yes, the _alphadot derivatives are ignored, for the simple reason that there's no way to do it
    in an algorithmic way for a general configuration. The usual estimates in Etkin or Nelson assume
     a "typical" tailed airplane configuration, with a small tail and a well-defined tail arm.
    These estimates won't work for other configurations like canard, tandem, or flying wing.

    Normally, only the Cm_alphadot derivative is significant, and slightly augments the pitch-damping
    derivative Cm_q. Leaving it out therefore underpredicts pitch damping slightly, so this is a
    conservative approximation. And pitch damping is not a major concern in any case.
    Simple pitch stability is more important, and that's not affected by alphadot.

    All this is for a conventional configuration. Not sure what the impact is on the pitch damping of flying wings. */
}

/**
 * Returns the inertia tensor in stability axis

 * R  is the rotation matrix from Body Frame to stability frame
 * Ib is the inertia tensor in the body frame, with origin at the CoG

 * The body frame is the one in which the geometry has been defined,
 * i.e. the x axis points backwards, y is starboard and z is upwards

 * Is is the inertia tensor in stability axes with origin at the CoG
 *                               Is = tR.Ib.R
*/
void PanelAnalysis::computeStabilityInertia()
{
    double tR[3][3], tmp[3][3];

    tR[0][0] = m_R[0][0];
    tR[0][1] = m_R[1][0];
    tR[0][2] = m_R[2][0];
    tR[1][0] = m_R[0][1];
    tR[2][0] = m_R[0][2];

    tR[1][1] = m_R[1][1];
    tR[1][2] = m_R[2][1];
    tR[2][1] = m_R[1][2];

    tR[2][2] = m_R[2][2];

    // tmp = Ib.R
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            tmp[i][j] = m_Ib[i][0]*m_R[0][j] + m_Ib[i][1]*m_R[1][j] + m_Ib[i][2]*m_R[2][j];
        }
    }

    // Is = tR.tmp
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            m_Is[i][j] = tR[i][0]*tmp[0][j] + tR[i][1]*tmp[1][j] + tR[i][2]*tmp[2][j];
        }
    }
}


/**
 * Calculates the control derivatives for small control deflections, using forward derivatives
 * The geometry has been previously modified to set the control in the position defined for this iteration of the analysis
 * The problem is not linear, since we take into account viscous forces
 * Therefore to get the derivative, we need to make the difference between two states
 * We use forward differences between the reference state corresponding to the trimmed conditions
 * and the same state + application of a delta angle to all the controls

 * AVL ignores the modification of the matrix terms and therefore requires only a single LU decomposition throughout the problem.

 * The same method is applied here
 *  - Save the reference geometry
 *  - Change the control point positions and the normals of the flaps
 *  - re-generate the RHS vector
 *  - the geometry is reset at the next iteration loop
*/
void PanelAnalysis::computeControlDerivatives()
{
    Vector3d WindDirection, H, Force, Moment, V0, is, js, ks;

    double SignedDeltaAngle(0);

    QString str;
    Quaternion Quat;

    //    q = 1./2. * m_pWPolar->density() * u0 * u0;
    //    b   = m_pWPolar->m_WSpan;
    //    S   = m_pWPolar->m_WArea;
    //    mac = m_pPlane->mac();

    Xde = Yde =  Zde = Lde = Mde = Nde= 0.0;

    bool bActive = false;
    for(int c=0; c<m_NCtrls; c++)
    {
        if(qAbs(m_pWPolar->m_ControlGain[c])>PRECISION)
        {
            bActive = true;
            break;
        }
    }
    if(!bActive)
    {
        str = "\n      No active control - skipping control derivatives\n\n\n";
        traceLog(str);
        return;
    }

    // Define the stability axes and the freestream velocity field
    double cosa = cos(m_AlphaEq*PI/180);
    double sina = sin(m_AlphaEq*PI/180);
    V0.set(u0*cosa, 0.0, u0*sina);
    WindDirection.set(cosa, 0.0, sina);
    is.set(-cosa, 0.0, -sina);
    js.set(  0.0, 1.0,   0.0);
    ks.set( sina, 0.0, -cosa);

    double DeltaAngle = 0.001;

    int pos = 0;

    int NCtrls = 0;

    if(!m_pPlane->isWing())
    {
        //Wing tilt
        if(qAbs(m_pWPolar->m_ControlGain[0])>PRECISION)
        {
            //rotate the normals and control point positions
            H.set(0.0, 1.0, 0.0);
            if(qAbs(m_pWPolar->m_ControlGain[0])>PRECISION)
                SignedDeltaAngle = DeltaAngle * m_pWPolar->m_ControlGain[0]/qAbs(m_pWPolar->m_ControlGain[0]);
            else SignedDeltaAngle = DeltaAngle;

            Quat.set(SignedDeltaAngle*180.0/PI, H);

            for(int p=0; p<m_pWingList[0]->nPanels(); p++)
            {
                Panel &panel = m_pPanel[m_pWingList[0]->firstPanelIndex()+p];
                panel.rotateBC(m_pPlane->wingLE(0), Quat);
            }
        }

        pos = m_pWingList[0]->m_nPanels;
        NCtrls = 1;
    }

    if(m_pPlane && m_pWingList[2])
    {
        //Elevator tilt
        if (qAbs(m_pWPolar->m_ControlGain[1])>PRECISION)
        {
            H.set(0.0, 1.0, 0.0);

            if(qAbs(m_pWPolar->m_ControlGain[1])>PRECISION)
                SignedDeltaAngle = DeltaAngle * m_pWPolar->m_ControlGain[1]/qAbs(m_pWPolar->m_ControlGain[1]);
            else SignedDeltaAngle = DeltaAngle;

            Quat.set(SignedDeltaAngle*180.0/PI, H);

            for(int p=0; p<m_pWingList[2]->m_nPanels; p++)
            {
                Panel &panel = m_pPanel[m_pWingList[2]->firstPanelIndex()+p];
                panel.rotateBC(m_pPlane->wingLE(2), Quat);
            }
        }
        pos += m_pWingList[2]->m_nPanels;
        NCtrls = 2;
    }

    //flap tilt
    /** @todo : exclude fin flaps, or add them to StabPolarDlg*/
    for (int j=0; j<m_ppSurface->size(); j++)
    {
        if(m_ppSurface->at(j)->m_bTEFlap)
        {
            if (qAbs(m_pWPolar->m_ControlGain[NCtrls])>PRECISION)
            {
                //Add delta rotations to initial control setting and to wing or flap delta rotation
                if(qAbs(m_pWPolar->m_ControlGain[NCtrls])>PRECISION)
                    SignedDeltaAngle = DeltaAngle * m_pWPolar->m_ControlGain[NCtrls]/qAbs(m_pWPolar->m_ControlGain[NCtrls]);
                else SignedDeltaAngle = DeltaAngle;

                Quat.set(SignedDeltaAngle*180.0/PI, m_ppSurface->at(j)->m_HingeVector);
                for(int p=0; p<m_MatSize;p++)
                {
                    if(m_ppSurface->at(j)->isFlapPanel(p))
                    {
                        m_pPanel[p].rotateBC(m_ppSurface->at(j)->m_HingePoint, Quat);
                    }
                }
            }
            NCtrls++;
        }
    }

    //create the RHS
    createRHS(m_cRHS, V0);

    if(!m_pWPolar->bThinSurfaces())
    {
        createWakeContribution(m_uWake,  WindDirection);// re-use m_uWake memory, which is re-calculated anyway at the next control iteration
        for(int p=0; p<m_MatSize; p++)    m_cRHS[p]+= m_uWake[p]*u0;
    }

    //Solve the system
    for (int p=0; p<m_MatSize; p++)
    {
        m_RHS[50*m_MatSize+p] = V0.x;
        m_RHS[51*m_MatSize+p] = V0.y;
        m_RHS[52*m_MatSize+p] = V0.z;
    }

    QString strong = "      Calculating the control derivatives\n\n";
    traceLog(strong);

    Crout_LU_with_Pivoting_Solve(m_aij, m_cRHS, m_Index, m_RHS, m_MatSize, &s_bCancel);
    memcpy(m_cRHS, m_RHS, uint(m_MatSize)*sizeof(double));

    forces(m_cRHS, m_Sigma, m_AlphaEq, V0, m_RHS+50*m_MatSize, Force, Moment);

    // make the forward difference with nominal results
    // which gives the stability derivative for a rotation of control ic
    Xde = (Force-Force0).dot(is)/DeltaAngle;
    Yde = (Force-Force0).dot(js)/DeltaAngle;
    Zde = (Force-Force0).dot(ks)/DeltaAngle;
    Lde = (Moment - Moment0).dot(is) /DeltaAngle;  // N.m/rad
    Mde = (Moment - Moment0).dot(js) /DeltaAngle;
    Nde = (Moment - Moment0).dot(ks) /DeltaAngle;
}


/**
* Returns the geometric pitching moment coefficient for the specified angle of attack
* The effect of the viscous drag is not included.
*@param Alpha the aoa for which Cm is calculated
*/
double PanelAnalysis::computeCm(double Alpha) const
{
    Vector3d VInf, Force, PanelLeverArm, ForcePt, PanelForce, WindDirection, VLocal;

    // Define the wind axis
    double cosa = cos(Alpha*PI/180.0);
    double sina = sin(Alpha*PI/180.0);
    WindDirection.set( cosa, 0.0, sina);
    VInf.set(cosa, 0.0, sina);

    double Cm = 0.0;
    for(int p=0; p<m_MatSize; p++)
    {
        //write vector operations in-line, more efficient
        if(m_pPanel[p].m_Pos!=xfl::MIDSURFACE)
        {
            //first calculate Cp for this angle
            m_pPanel[p].globalToLocal(VInf, VLocal);
            VLocal += m_uVl[p]*cosa + m_wVl[p]*sina;
            double Speed2 = VLocal.x*VLocal.x + VLocal.y*VLocal.y;
            double Cp  = 1.0-Speed2; // QInf=unit, /1.0/1.0;
            m_Cp[p] = Cp; /** @todo : remove, for information only*/

            //next calculate the force acting on the panel
            ForcePt = m_pPanel[p].CollPt;
            PanelForce = m_pPanel[p].Normal * (-Cp) * m_pPanel[p].Area;      // Newtons/q
        }
        else
        {
            // for each panel along the chord, add the lift coef
            double Gamma = m_uRHS[p]  *cosa + m_wRHS[p]  *sina;
            ForcePt = m_pPanel[p].VortexPos;
            PanelForce  = WindDirection * m_pPanel[p].Vortex;
            PanelForce *= 2.0 * Gamma;                                       //Newtons/q   (QInf = unit)
            if(!m_pWPolar->bVLM1() && !m_pPanel[p].m_bIsLeading)
            {
                Q_ASSERT(p+1<m_MatSize);
                double Gammap1 = m_uRHS[p+1]*cosa + m_wRHS[p+1]*sina;
                Force       = WindDirection * m_pPanel[p].Vortex;
                Force      *= 2.0 * Gammap1;       //Newtons/q/QInf
                PanelForce -= Force;
            }
            m_Cp[p] = PanelForce.dot(m_pPanel[p].Normal)/m_pPanel[p].Area; //todo : remove, for information only
        }
        PanelLeverArm.x = ForcePt.x - m_CoG.x;
        PanelLeverArm.y = ForcePt.y - m_CoG.y;
        PanelLeverArm.z = ForcePt.z - m_CoG.z;
        Cm += -PanelLeverArm.x * PanelForce.z + PanelLeverArm.z*PanelForce.x; //N.m/rho
    }

    Cm *= m_pWPolar->density();
    return Cm;
}


void PanelAnalysis::restorePanels()
{
    if(m_pWPolar && (m_pWPolar->polarType()==xfl::STABILITYPOLAR || m_pWPolar->bTilted() || m_pWPolar->bWakeRollUp()))
    {
        //restore the panels and nodes;
        memcpy(m_pPanel,        m_pMemPanel,     uint(m_MatSize) * sizeof(Panel));
        memcpy(m_pNode,         m_pMemNode,      uint(m_nNodes)  * sizeof(Vector3d));
        memcpy(m_pWakePanel,    m_pRefWakePanel, uint(m_WakeSize) * sizeof(Panel));
        memcpy(m_pWakeNode,     m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));
        memcpy(m_pTempWakeNode, m_pRefWakeNode,  uint(m_nWakeNodes) * sizeof(Vector3d));
    }
}


/**
* Creates the plane's operating point,
* fills it with the input resulting from the  analysis, and returns a pointer to the PlaneOpp object which has been created.

*@param bPointOut: if true, part of the oppoint viscous properties could not be interpolated
*@param Cp: the array of Cp value for each panel
*@param Gamma: the array of circulation or doublet strengths Gamma for each panel
*@param Sigma: the array of source strengths for a panel analysis
*@return a pointer to the PlaneOpp object which has been created
*/
PlaneOpp* PanelAnalysis::createPlaneOpp(double *Cp, double const *Gamma, double const *Sigma)
{
    double Cb = 0.0;

    PlaneOpp *pPOpp = new PlaneOpp(m_pPlane, m_pWPolar, m_MatSize);
    if(!pPOpp) return nullptr;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingList[iw])
        {
            pPOpp->addWingOpp(iw, m_pWingList[iw]->m_nPanels);
            pPOpp->m_pWOpp[iw]->createWOpp(m_pWingList[iw], m_pWPolar);
        }
    }

    pPOpp->m_bOut  = m_bPointOut;
    pPOpp->setAlpha(m_Alpha);
    pPOpp->setQInf(m_QInf);

    WingOpp *pWOpp = pPOpp->m_pWOpp[0];

    for (int l=0; l<m_pPlane->m_Wing[0].m_NStation; l++)
    {
        if(qAbs(m_pPlane->m_Wing[0].m_BendingMoment[l])>qAbs(Cb))    Cb = m_pPlane->m_Wing[0].m_BendingMoment[l];
    }


    pWOpp->m_MaxBending = Cb;

    if(m_pWPolar->analysisMethod()==xfl::VLMMETHOD)
    {
        traceLog("OldVLM polar\n");
    }
    else if(m_pWPolar->analysisMethod()==xfl::PANEL4METHOD)
    {
        //get the data from the PanelAnalysis dialog, and from the plane object
        pPOpp->m_NPanels             = m_MatSize;

        pPOpp->setAlpha(m_OpAlpha);
        pPOpp->m_QInf                = m_QInf;
        pPOpp->m_CL                  = m_CL;
        pPOpp->m_CX                  = m_CX;
        pPOpp->m_CY                  = m_CY;
        pPOpp->m_ICD                 = m_InducedDrag;
        pPOpp->m_VCD                 = m_ViscousDrag;

        pPOpp->m_GCm                 = m_GCm;
        pPOpp->m_VCm                 = m_VCm;
        pPOpp->m_ICm                 = m_ICm;

        pPOpp->m_GRm                 = m_GRm;

        pPOpp->m_GYm                 = m_GYm;
        pPOpp->m_VYm                 = m_VYm;
        pPOpp->m_IYm                 = m_IYm;

        pPOpp->m_CP                  = m_CP;

        if(m_pWPolar->polarType()!=xfl::BETAPOLAR) pPOpp->m_Beta = m_pWPolar->m_BetaSpec;
        else                                         pPOpp->m_Beta = m_OpBeta;

        if(m_pWPolar->polarType()==xfl::STABILITYPOLAR)
        {
            pPOpp->setAlpha(m_AlphaEq);
            pPOpp->m_QInf             = u0;
            pPOpp->m_Ctrl             = m_Ctrl;

            pPOpp->m_Ctrl             = m_Ctrl;
            pWOpp->m_QInf             = u0;
            pWOpp->m_Alpha            = m_AlphaEq;

            for(int i=0; i<4; i++)
            {
                pPOpp->m_EigenValue[i]   = m_rLong[i];
                pPOpp->m_EigenValue[i+4] = m_rLat[i];
                for(int l=0;l<4; l++)
                {
                    pPOpp->m_EigenVector[i][l]   = m_vLong[4*i+l];
                    pPOpp->m_EigenVector[i+4][l] = m_vLat[4*i+l];
                }
            }


            for(int i=0; i<4; i++)
            {
                pPOpp->m_EigenValue[i]   = m_rLong[i];
                pPOpp->m_EigenValue[i+4] = m_rLat[i];
                for(int l=0;l<4; l++)
                {
                    pPOpp->m_EigenVector[i][l]   = m_vLong[4*i+l];
                    pPOpp->m_EigenVector[i+4][l] = m_vLat[4*i+l];
                }
            }

            pPOpp->m_XNP = XNP;

            pPOpp->CXu =  CXu;
            pPOpp->CZu =  CZu;
            pPOpp->Cmu =  Cmu;

            pPOpp->CXa =  CXa;
            pPOpp->CLa = -CZa;
            pPOpp->Cma =  Cma;

            pPOpp->CXq =  CXq;
            pPOpp->CLq = -CZq;
            pPOpp->Cmq =  Cmq;

            pPOpp->CYb =  CYb;
            pPOpp->Clb =  Clb;
            pPOpp->Cnb =  Cnb;

            pPOpp->CYp =  CYp;
            pPOpp->Clp =  Clp;
            pPOpp->Cnp =  Cnp;

            pPOpp->CYr =  CYr;
            pPOpp->Clr =  Clr;
            pPOpp->Cnr =  Cnr;

            //Only one control derivative for all the controls of the polar
            pPOpp->m_pWOpp[0]->m_nControls = 1;
            pPOpp->m_nControls = 1;
            pPOpp->CXe = CXe;
            pPOpp->CYe = CYe;
            pPOpp->CZe = CZe;
            pPOpp->CLe = Cle;
            pPOpp->CMe = Cme;
            pPOpp->CNe = Cne;

            memcpy(pPOpp->m_ALong, m_ALong, 16*sizeof(double));
            memcpy(pPOpp->m_ALat,  m_ALat,  16*sizeof(double));

            memcpy(pPOpp->m_BLong, m_BLong, 4*sizeof(double));
            memcpy(pPOpp->m_BLat,  m_BLat,  4*sizeof(double));
        }
        else
        {
            pPOpp->m_Ctrl = 0.0;
            memset(pPOpp->m_EigenValue, 0, sizeof(pPOpp->m_EigenValue));
            memset(pPOpp->m_EigenVector, 0, sizeof(pPOpp->m_EigenVector));
            memset(pPOpp->m_ALong, 0, 16*sizeof(double));
            memset(pPOpp->m_ALat,  0,  16*sizeof(double));
        }

    }

    for(int iw=0;iw<MAXWINGS; iw++)
    {
        if(pPOpp->m_pWOpp[iw])
        {
            pPOpp->m_pWOpp[iw]->m_Alpha = pPOpp->alpha();
            pPOpp->m_pWOpp[iw]->m_QInf  = pPOpp->m_QInf;


            pPOpp->m_pWOpp[iw]->m_FlapMoment.clear();
            for (int i=0; i<m_pPlane->m_Wing[iw].m_nFlaps; i++)
            {
                pPOpp->m_pWOpp[iw]->m_FlapMoment.append(m_pPlane->m_Wing[iw].m_FlapMoment.at(i));
            }
        }
    }

    memcpy(pPOpp->m_dCp,    Cp,    uint(pPOpp->m_NPanels)*sizeof(double));
    memcpy(pPOpp->m_dG,     Gamma, uint(pPOpp->m_NPanels)*sizeof(double));
    memcpy(pPOpp->m_dSigma, Sigma, uint(pPOpp->m_NPanels)*sizeof(double));

    int pos = 0;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingList[iw])
        {
            if(pPOpp->m_pWOpp[iw])
            {
                pPOpp->m_pWOpp[iw]->m_dCp    = pPOpp->m_dCp    + pos;
                pPOpp->m_pWOpp[iw]->m_dG     = pPOpp->m_dG     + pos;
                pPOpp->m_pWOpp[iw]->m_dSigma = pPOpp->m_dSigma + pos;
            }
            pos +=m_pWingList[iw]->m_nPanels;
        }
    }

    pPOpp->m_phiPH = m_phiPH;
    pPOpp->m_phiDR = m_phiDR;

    //add the data to the polar object
    if(PlaneOpp::s_bKeepOutOpps || !pPOpp->isOut())
        m_pWPolar->addPlaneOpPoint(pPOpp);

    return pPOpp;
}


/**
 * Adds the input message to the output message.
 * The message is read and cleared from the calling dialog class.
 * @param str the message to output
*/
void PanelAnalysis::traceLog(QString str) const
{
    emit outputMsg(str);
}



void PanelAnalysis::computePhillipsFormulae()
{
    //    Phugoid Approximation for Conventional Airplanes, JOURNAL OF AIRCRAFT,     Vol. 37, No. 1, January– February 2000
    //    Improved Closed-Form Approximation for Dutch Roll, JOURNAL OF AIRCRAFT,    Vol. 37, No. 3, May– June 2000

    double MTOW_des(0), AWing(0), Vmax_C(0), Span(0), Chord(0), CL_C(0), CDtot_C(0), rho_C(0);
    double Ixx(0),Iyy(0),Izz(0);
    double Rg(0),Rgy(0),Rza(0);
    double RDc(0), RDp(0), RDs(0), Rs(0), Rd(0), Rp(0);
    double rlDR(0), ilDR(0), rlPH(0), ilPH(0);
    double Rxa(0), Rma(0), Rmq(0), RYb(0), Rlb(0), Rnb(0), Rlp(0), Rnp(0), RYr(0), Rlr(0), Rnr(0);
    double Fdr(0),zeta_dr(0),Fph(0),zeta_ph(0);

    MTOW_des = m_pWPolar->mass();
    AWing    = m_pPlane->planformArea();
    Vmax_C   = m_QInf;
    Span     = m_pPlane->planformSpan();
    Chord    = m_pPlane->mac();
    rho_C    = m_pWPolar->density();
    Ixx      = m_Is[0][0];
    Iyy      = m_Is[1][1];
    Izz      = m_Is[2][2];
    CL_C     = m_CL;
    CDtot_C  = m_CX;
    for(int i=0; i<MAXEXTRADRAG; i++)
    {
        if(fabs(m_pWPolar->m_ExtraDragCoef[i])>PRECISION && fabs(m_pWPolar->m_ExtraDragArea[i])>PRECISION)
        {
            CDtot_C += m_pWPolar->m_ExtraDragCoef[i] * m_pWPolar->m_ExtraDragArea[i] / AWing;
        }
    }

    //    Rx  =-CDtot_C*rho_C*AWing*Chord/(2*MTOW_des); //unused
    //    Rz  =-CL_C*rho_C*AWing*Chord/(2*MTOW_des); //unused
    Rg  = 9.81*Chord/(2*Vmax_C*Vmax_C);
    Rgy = 9.81*Span/(2*Vmax_C*Vmax_C);
    Rza = (rho_C*AWing*Chord/(4*MTOW_des))*(+CZa-CDtot_C); /** @todo check issue with sign of CZa */

    Rxa = (rho_C*AWing*Chord/(4*MTOW_des))*CXa;
    Rma = (rho_C*AWing*Chord*Chord*Chord/(8*Iyy))*Cma;
    Rmq = (rho_C*AWing*Chord*Chord*Chord/(8*Iyy))*Cmq;
    RYb = (rho_C*AWing*Span/(4*MTOW_des))*CYb;
    Rlb = (rho_C*AWing*Span*Span*Span/(8*Ixx))*Clb;
    Rnb = (rho_C*AWing*Span*Span*Span/(8*Izz))*Cnb;
    Rlp = (rho_C*AWing*Span*Span*Span/(8*Ixx))*Clp;
    Rnp = (rho_C*AWing*Span*Span*Span/(8*Izz))*Cnp;
    RYr = (rho_C*AWing*Span/(4*MTOW_des))*CYr;
    Rlr = (rho_C*AWing*Span*Span*Span/(8*Ixx))*Clr;
    Rnr = (rho_C*AWing*Span*Span*Span/(8*Izz))*Cnr;

    RDc = Rlr*Rnp/Rlp;
    RDs = (Rlb*(Rgy-(1-RYr)*Rnp)-RYb*Rlr*Rnp)/Rlp;
    RDp = (Rgy*(Rlr*Rnb-Rlb*Rnr))/(Rlp*(Rnb+RYb*Rnr))-RDs/Rlp;
    Rs  = Rma/(Rma-Rza*Rmq);
    Rd  = Rxa*Rmq/(Rma-Rza*Rmq);
    Rp  = Rg*Rs*((Rza+Rmq)/(Rma-Rza*Rmq));

    rlDR = (2*Vmax_C/Span)*(RYb+Rnr-RDc+RDp)/2;
    ilDR = (2*Vmax_C/Span)*sqrt(fabs((1-RYr)*Rnb+RYb*Rnr+RDs-((RYb+Rnr)/2)*((RYb+Rnr)/2)));
    rlPH = (9.81/Vmax_C)*(-CDtot_C/CL_C-Rd+Rp);
    ilPH = (9.81/Vmax_C)*sqrt(fabs(2*Rs-((CDtot_C/CL_C)+Rd)*((CDtot_C/CL_C)+Rd)));

    Fdr = sqrt(rlDR*rlDR+ilDR*ilDR)/(2.*PI);
    zeta_dr = -rlDR/ilDR;
    Fph = sqrt(rlPH*rlPH+ilPH*ilPH)/(2.*PI);
    zeta_ph = -rlPH/ilPH;

    m_phiPH = complex<double>(rlPH, ilPH);
    m_phiDR = complex<double>(rlDR, ilDR);

    QString strange;
    traceLog("\n");
    traceLog("   Phillips formulae:\n");
    strange = QString::asprintf("       Phugoid eigenvalue:     %9.5f+%9.5fi",rlPH, ilPH);
    traceLog(strange+"\n");
    strange = QString::asprintf("               frequency:%7.3f Hz", Fph);
    traceLog(strange+"\n");
    strange = QString::asprintf("               damping:  %7.3f", zeta_ph);
    traceLog(strange+"\n");
    strange = QString::asprintf("       Dutch-Roll eigenvalue:  %9.5f+%9.5fi",rlDR, ilDR);
    traceLog(strange+"\n");
    strange = QString::asprintf("               frequency:%7.3f Hz", Fdr);
    traceLog(strange+"\n");
    strange = QString::asprintf("               damping:  %7.3f", zeta_dr);
    traceLog(strange+"\n");
}


/**
 * Rotates all the panels about the y-axis
 * @param Angle the rotation angle in degrees
 * @param P the point of origin of the rotation
 */
void PanelAnalysis::rotateGeomY(double Alpha, Vector3d const &P, int NXWakePanels)
{
    Vector3d LATB, TALB, Pt, Trans;

    for (int n=0; n<m_nNodes; n++)
    {
        m_pNode[n].rotateY(P, Alpha);
    }

    for (int p=0; p<m_MatSize; p++)
    {
        int iLA = m_pPanel[p].m_iLA;
        int iLB = m_pPanel[p].m_iLB;
        int iTA = m_pPanel[p].m_iTA;
        int iTB = m_pPanel[p].m_iTB;

        LATB = m_pNode[iLA] - m_pNode[iTB];
        TALB = m_pNode[iTA] - m_pNode[iLB];

        if(m_pPanel[p].m_Pos==xfl::MIDSURFACE || m_pPanel[p].m_Pos==xfl::TOPSURFACE) m_pPanel[p].setPanelFrame(m_pNode[iLA], m_pNode[iLB], m_pNode[iTA], m_pNode[iTB]);
        else if (m_pPanel[p].m_Pos==xfl::BOTSURFACE)                                 m_pPanel[p].setPanelFrame(m_pNode[iLB], m_pNode[iLA], m_pNode[iTB], m_pNode[iTA]);
    }

    // the wake array is not rotated but translated to remain at the wing's trailing edge
    int pw=0;

    for (int kw=0; kw<m_NWakeColumn; kw++)
    {
        //consider the first panel of the column;
        Pt = m_pWakeNode[m_pWakePanel[pw].m_iLA];
        Pt.rotateY(P, Alpha);
        //define the translation to be applied to the column's left points
        Trans = Pt - m_pWakeNode[m_pWakePanel[pw].m_iLA];

        //each wake column has m_NXWakePanels ... translate all left A nodes
        for(int lw=0; lw<NXWakePanels; lw++)
        {
            if(lw==0) m_pWakeNode[m_pWakePanel[pw].m_iLA] += Trans;
            m_pWakeNode[m_pWakePanel[pw].m_iTA] += Trans;
            pw++;
        }
    }

    //do the same for the very last right wake node column
    pw -= NXWakePanels;
    Pt = m_pWakeNode[m_pWakePanel[pw].m_iLB];
    Pt.rotateY(P, Alpha);
    //define the translation to be applied to the column's left points
    Trans = Pt-m_pWakeNode[m_pWakePanel[pw].m_iLB];

    //each wake column has m_NXWakePanels ... translate all right B nodes
    for(int lw=0; lw<NXWakePanels; lw++)
    {
        if(lw==0) m_pWakeNode[m_pWakePanel[pw].m_iLB] += Trans;
        m_pWakeNode[m_pWakePanel[pw].m_iTB] += Trans;
        pw++;
    }

    //Reset panel frame : CollPt has been translated
    for (pw=0; pw<m_WakeSize; pw++)
    {
        int iLA = m_pWakePanel[pw].m_iLA;
        int iLB = m_pWakePanel[pw].m_iLB;
        int iTA = m_pWakePanel[pw].m_iTA;
        int iTB = m_pWakePanel[pw].m_iTB;
        m_pWakePanel[pw].setPanelFrame(m_pWakeNode[iLA], m_pWakeNode[iLB], m_pWakeNode[iTA], m_pWakeNode[iTB]);
    }
}



/**
 * Rotates all the panels about the z-axis
 * @param pPanel a pointer to the array of surface mesh panels
 * @param pNode  a pointer to the array of surface panel nodes
 * @param pWakePanel a pointer to the array of wake mesh panels
 * @param pWakeNode  a pointer to the array of wake panel nodes
 * @param beta the rotation angle in degrees
 * @param P the point of origin of the rotation
 */
void PanelAnalysis::rotateGeomZ(double Beta, Vector3d const &P, int NXWakePanels)
{
    int iLA(0), iLB(0), iTA(0), iTB(0);
    Vector3d Pt, Trans;

    for (int n=0; n<m_nNodes; n++)    m_pNode[n].rotateZ(P, Beta);

    for (int p=0; p<m_MatSize; p++)
    {
        // get the index of the panel's four corner points
        iLA = m_pPanel[p].m_iLA; iLB = m_pPanel[p].m_iLB;
        iTA = m_pPanel[p].m_iTA; iTB = m_pPanel[p].m_iTB;

        //set the new panel geometry
        if(m_pPanel[p].m_Pos>=xfl::MIDSURFACE)       m_pPanel[p].setPanelFrame(m_pNode[iLA], m_pNode[iLB], m_pNode[iTA], m_pNode[iTB]);
        else if (m_pPanel[p].m_Pos==xfl::BOTSURFACE) m_pPanel[p].setPanelFrame(m_pNode[iLB], m_pNode[iLA], m_pNode[iTB], m_pNode[iTA]);
    }

    // the wake array is not rotated but translated to remain at the wing's trailing edge and aligned with the freestream velocity vector
    int pw=0;

    for (int kw=0; kw<m_NWakeColumn; kw++)
    {
        //consider the first panel of the column;
        Pt = m_pWakeNode[m_pWakePanel[pw].m_iLA];
        Pt.rotateZ(P, Beta);
        //define the translation to be applied to the column's left points
        Trans = Pt-m_pWakeNode[m_pWakePanel[pw].m_iLA] ;

        //each wake column has m_NXWakePanels ... translate all left A nodes
        for(int lw=0; lw<NXWakePanels; lw++)
        {
            if(lw==0) m_pWakeNode[m_pWakePanel[pw].m_iLA] += Trans;
            m_pWakeNode[m_pWakePanel[pw].m_iTA] += Trans;
            pw++;
        }
    }

    //last column, process B side of the column
    pw -= NXWakePanels;
    //consider the first panel of the column;
    Pt = m_pWakeNode[m_pWakePanel[pw].m_iLB];
    Pt.rotateZ(P, Beta);
    //define the translation to be applied to the column's left points
    Trans = Pt - m_pWakeNode[m_pWakePanel[pw].m_iLB];

    //each wake column has m_NXWakePanels ... translate all left A nodes
    for(int lw=0; lw<NXWakePanels; lw++)
    {
        if(lw==0) m_pWakeNode[m_pWakePanel[pw].m_iLB] += Trans;
        m_pWakeNode[m_pWakePanel[pw].m_iTB] += Trans;
        pw++;
    }

    //Reset panel frame : CollPt has been translated
    for (pw=0; pw< m_WakeSize; pw++)
    {
        // get the index of the panel's four corner points
        iLA = m_pWakePanel[pw].m_iLA; iLB = m_pWakePanel[pw].m_iLB;
        iTA = m_pWakePanel[pw].m_iTA; iTB = m_pWakePanel[pw].m_iTB;

        //set the new panel geometry
        m_pWakePanel[pw].setPanelFrame(m_pWakeNode[iLA], m_pWakeNode[iLB], m_pWakeNode[iTA], m_pWakeNode[iTB]);
    }
}


/**
 * Called by a stability analysis to modify the panels and nodes by setting the control positions to the specified position t
 * The panels and nodes to be rotated are pointed by pPanel and pNode
 * @param pPanel a pointer to the array of surface panels to be rotated
 * @param pNode  a pointer to the array of mesh nodes to be rotated
 * @param t the value of the control parameter which defines the amount of rotation
 * @param &NCtrls counts tha active controls
 * @param &out the output message for the log file
 * @param bBCOnly, if true, then only the control points and normal vector and rotates; if not,the whole geometry is rotated
 */
void PanelAnalysis::setControlPositions(double t, int &NCtrls, QString &out, bool bBCOnly)
{
    QString strange;
    Quaternion Quat;
    int nFlap(0);
    double angle(0), TotalAngle(0);
    Wing *pWing(nullptr);
    Vector3d YVector(0.0, 1.0, 0.0);
    Vector3d W;

    // update the variables & geometry
    // if plane : WingTilt, elevator Tilt
    // if flaps : wing flaps, elevator flaps

    //the CG position is fixed for this analysis

    NCtrls = 0;

    Wing *pWingList[MAXWINGS];
    pWingList[0] = m_pPlane->wing();
    pWingList[1] = m_pPlane->wing2();
    pWingList[2] = m_pPlane->stab();
    pWingList[3] = m_pPlane->fin();


    if(!m_pPlane->isWing())
    {
        //wing incidence
        if(qAbs(m_pWPolar->m_ControlGain[0])>PRECISION)
        {
            //wing tilt
            angle = m_pWPolar->m_ControlGain[0] * t; //maxcontrol is the gain
            TotalAngle = m_pPlane->wingTiltAngle(0) + angle;
            strange = QString::fromUtf8("    Rotating the wing by %1°, total angle is %2°").arg(angle, 5, 'f',2).arg(TotalAngle, 5, 'f',2);

            strange += "\n";
            out +=strange;

            Quat.set(angle, YVector);

            if(bBCOnly)
            {
                for(int p=0; p<m_pPlane->wing()->nPanels(); p++)
                {
                    memcpy(m_pPanel+p, m_pMemPanel+p, sizeof(Panel));
                    (m_pPanel+p)->rotateBC(m_pPlane->wingLE(0), Quat);
                }
            }
            else
            {
                for(int n=0; n<m_nNodes; n++)
                {
                    if(m_pPlane->wing()->isWingNode(n, m_pPanel))
                    {
                        m_pNode[n].copy(m_pMemNode[n]);
                        W = m_pNode[n] - m_pPlane->wingLE(0);
                        Quat.conjugate(W);
                        m_pNode[n] = W + m_pPlane->wingLE(0);
                    }
                }
                for(int p=0; p<m_MatSize; p++)
                {
                    if(m_pPlane->wing()->isWingPanel(p, m_pPanel)) m_pPanel[p].setPanelFrame();
                }
            }
        }
        NCtrls=1;

        if(m_pPlane->stab())
        {
            //elevator incidence
            if(qAbs(m_pWPolar->m_ControlGain[1])>PRECISION)
            {
                //Elevator tilt
                angle = m_pWPolar->m_ControlGain[1] * t; //maxcontrol is the gain
                TotalAngle = m_pPlane->wingTiltAngle(2) + angle;
                strange = QString::fromUtf8("         Rotating the elevator by %1°, total angle is %2°").arg(angle, 5, 'f',2).arg(TotalAngle, 5, 'f',2);

                strange += "\n";
                out +=strange;

                Quat.set(angle, YVector);
                if(!bBCOnly)
                {
                    for(int n=0; n<m_nNodes; n++)
                    {
                        if(m_pPlane->stab()->isWingNode(n, m_pPanel))
                        {
                            m_pNode[n].copy(m_pMemNode[n]);
                            W = m_pNode[n] - m_pPlane->wingLE(2);
                            Quat.conjugate(W);
                            m_pNode[n] = W + m_pPlane->wingLE(2);
                        }
                    }
                    for(int p=0; p<m_MatSize; p++)
                    {
                        if(pWingList[2]->isWingPanel(p, m_pPanel)) m_pPanel[p].setPanelFrame();
                    }
                }
                else
                {
                    for(int p=0; p<m_pPlane->stab()->m_nPanels; p++)
                    {
                        Panel &panel = m_pPanel[m_pPlane->stab()->firstPanelIndex()+p];
                        panel.rotateBC(m_pPlane->wingLE(2), Quat);
                    }
                }
            }
            NCtrls = 2;
        }
    }

    // flap controls
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        pWing = pWingList[iw];
        if(pWing)
        {
            nFlap = 0;
            for (int j=0; j<pWing->m_Surface.size(); j++)
            {
                if(pWing->surface(j)->m_bTEFlap)
                {
                    if(qAbs(m_pWPolar->m_ControlGain[NCtrls])>PRECISION)
                    {
                        angle = m_pWPolar->m_ControlGain[NCtrls] * t; //maxcontrol is the gain

                        if(fabs(pWing->surface(j)->m_pFoilA->m_TEFlapAngle)>0.0 &&
                           fabs(pWing->surface(j)->m_pFoilA->m_TEFlapAngle)>0.0)
                            TotalAngle = angle + (pWing->surface(j)->m_pFoilA->m_TEFlapAngle + pWing->surface(j)->m_pFoilB->m_TEFlapAngle)/2.0;
                        else
                            TotalAngle = angle;

                        strange = QString::fromUtf8("         Rotating the flap by %1°, total angle is %2°").arg(angle, 5, 'f',2).arg(TotalAngle, 5, 'f',2);

                        strange += "\n";
                        out +=strange;

                        if(qAbs(angle)>PRECISION)
                        {
                            //                            pWing->surface(j)->RotateFlap(angle, bBCOnly);
                            Quat.set(angle, pWing->surface(j)->m_HingeVector);
                            if(bBCOnly)
                            {
                                for(int p=0; p<m_MatSize; p++)
                                {
                                    if(pWing->surface(j)->isFlapPanel(p))
                                    {
                                        memcpy(m_pPanel+p, m_pMemPanel+p, sizeof(Panel));
                                        m_pPanel[p].rotateBC(pWing->surface(j)->m_HingePoint, Quat);
                                    }
                                }
                            }
                            else
                            {
                                for(int n=0; n<m_nNodes; n++)
                                {
                                    if(pWing->surface(j)->isFlapNode(n))
                                    {
                                        m_pNode[n].copy(m_pMemNode[n]);
                                        W = m_pNode[n] - pWing->surface(j)->m_HingePoint;
                                        Quat.conjugate(W);
                                        m_pNode[n] = W + pWing->surface(j)->m_HingePoint;
                                    }
                                }
                                for(int p=0; p<m_MatSize; p++)
                                {
                                    if(pWing->surface(j)->isFlapPanel(p)) m_pPanel[p].setPanelFrame();
                                }
                            }
                        }
                    }
                    nFlap++;
                    NCtrls++;
                }
            }
        }
    }
}


/**
* HORSESHOE VORTEX FORMULATION/
*
*    LA__________LB               |
*    |           |                |
*    |           |                | freestream speed
*    |           |                |
*    |           |                \/
*    |           |
*    \/          \/
*
*
* Returns the velocity greated by a horseshoe vortex with unit circulation at a distant point
*
* Notes :
*  - The geometry has been rotated by the sideslip angle, hence, there is no need to align the trailing vortices with sideslip
*  - Vectorial operations are written inline to save computing times -->longer code, but 4x more efficient....
*
* @param A the left point of the bound vortex
* @param B the right point of the bound vortex
* @param C the point where the velocity is calculated
* @param V the resulting velocity vector at point C
* @param bAll true if the influence of the bound vortex should be evaluated; false for a distant point in the far field.
*/
void PanelAnalysis::VLMCmn(Vector3d const &A, Vector3d const &B, Vector3d const &C, Vector3d &V, bool bAll) const
{
    //we use a default core size, unless the user has specified one
    double CoreSize = 0.0001;
    if(fabs(Panel::coreSize())>PRECISION) CoreSize = Panel::coreSize();
    double ftmp(0), Omega(0), Psi_x(0), Psi_y(0), Psi_z(0), r0_x(0), r0_y(0), r0_z(0), r1_x(0), r1_y(0), r1_z(0), r2_x(0), r2_y(0), r2_z(0);
    double Far_x(0), Far_y(0), Far_z(0), t_x(0), t_y(0), t_z(0), h_x(0), h_y(0), h_z(0);
    V.x = 0.0;
    V.y = 0.0;
    V.z = 0.0;

    if(bAll)
    {
        r0_x = B.x - A.x;
        r0_y = B.y - A.y;
        r0_z = B.z - A.z;

        r1_x = C.x - A.x;
        r1_y = C.y - A.y;
        r1_z = C.z - A.z;

        r2_x = C.x - B.x;
        r2_y = C.y - B.y;
        r2_z = C.z - B.z;

        Psi_x = r1_y*r2_z - r1_z*r2_y;
        Psi_y =-r1_x*r2_z + r1_z*r2_x;
        Psi_z = r1_x*r2_y - r1_y*r2_x;

        ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

        //get the distance of the TestPoint to the panel's side
        t_x =  r1_y*r0_z - r1_z*r0_y;
        t_y = -r1_x*r0_z + r1_z*r0_x;
        t_z =  r1_x*r0_y - r1_y*r0_x;

        if ((t_x*t_x+t_y*t_y+t_z*t_z)/(r0_x*r0_x+r0_y*r0_y+r0_z*r0_z) >CoreSize * CoreSize)
        {
            Psi_x /= ftmp;
            Psi_y /= ftmp;
            Psi_z /= ftmp;

            Omega = (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
                    -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

            V.x = Psi_x * Omega/4.0/PI;
            V.y = Psi_y * Omega/4.0/PI;
            V.z = Psi_z * Omega/4.0/PI;
        }
    }
    // We create Far points to align the trailing vortices with the reference axis
    // The trailing vortex legs are not aligned with the free-stream, i.a.w. the small angle approximation
    // If this approximation is not valid, then the geometry should be tilted in the polar definition

    // calculate left contribution
    Far_x = A.x +  1.0e10;
    Far_y = A.y;
    Far_z = A.z;// + (Far_x-A.x) * tan(m_Alpha*PI/180.0);

    r0_x = A.x - Far_x;
    r0_y = A.y - Far_y;
    r0_z = A.z - Far_z;

    r1_x = C.x - A.x;
    r1_y = C.y - A.y;
    r1_z = C.z - A.z;

    r2_x = C.x - Far_x;
    r2_y = C.y - Far_y;
    r2_z = C.z - Far_z;

    Psi_x = r1_y*r2_z - r1_z*r2_y;
    Psi_y =-r1_x*r2_z + r1_z*r2_x;
    Psi_z = r1_x*r2_y - r1_y*r2_x;

    ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

    t_x=1.0; t_y=0.0; t_z=0.0;

    h_x =  r1_y*t_z - r1_z*t_y;
    h_y = -r1_x*t_z + r1_z*t_x;
    h_z =  r1_x*t_y - r1_y*t_x;

    //Next add 'left' semi-infinite contribution
    //eq.6-56

    if ((h_x*h_x+h_y*h_y+h_z*h_z) > CoreSize * CoreSize)
    {
        Psi_x /= ftmp;
        Psi_y /= ftmp;
        Psi_z /= ftmp;

        Omega =  (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
                -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

        V.x += Psi_x * Omega/4.0/PI;
        V.y += Psi_y * Omega/4.0/PI;
        V.z += Psi_z * Omega/4.0/PI;
    }

    // calculate right vortex contribution
    Far_x = B.x +  1.0e10;
    Far_y = B.y ;
    Far_z = B.z;// + (Far_x-B.x) * tan(m_Alpha*PI/180.0);

    r0_x = Far_x - B.x;
    r0_y = Far_y - B.y;
    r0_z = Far_z - B.z;

    r1_x = C.x - Far_x;
    r1_y = C.y - Far_y;
    r1_z = C.z - Far_z;

    r2_x = C.x - B.x;
    r2_y = C.y - B.y;
    r2_z = C.z - B.z;

    Psi_x = r1_y*r2_z - r1_z*r2_y;
    Psi_y =-r1_x*r2_z + r1_z*r2_x;
    Psi_z = r1_x*r2_y - r1_y*r2_x;

    ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

    //Last add 'right' semi-infinite contribution
    h_x =  r2_y*t_z - r2_z*t_y;
    h_y = -r2_x*t_z + r2_z*t_x;
    h_z =  r2_x*t_y - r2_y*t_x;

    if ((h_x*h_x+h_y*h_y+h_z*h_z) > CoreSize * CoreSize)
    {
        Psi_x /= ftmp;
        Psi_y /= ftmp;
        Psi_z /= ftmp;

        Omega =  (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
                -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

        V.x += Psi_x * Omega/4.0/PI;
        V.y += Psi_y * Omega/4.0/PI;
        V.z += Psi_z * Omega/4.0/PI;
    }
}



/**
* QUAD VORTEX FORMULATION
*
* LA, LB, TA, TB are the vortex's four corners
* LA and LB are at the 3/4 point of panel nx
* TA and TB are at the 3/4 point of panel nx+1
*
*    LA__________LB               |
*    |           |                |
*    |           |                | freestream speed
*    |           |                |
*    |           |                \/
*    |           |
*    TA__________TB
*
*
* Returns the velocity greated by a ring vortex with unit circulation at a distant point
*
* Notes :
*  - The geometry has been rotated by the sideslip angle, hence, there is no need to align the trailing vortices with sideslip
*  - Vectorial operations are written inline to save computing times -->longer code, but 4x more efficient....
*
* @param LA the leading left point of the quad vortex
* @param LB the leading right point of the quad vortex
* @param TA the trailing left point of the quad vortex
* @param TB the trailing right point of the quad vortex
* @param C the point where the velocity is calculated
* @param V the resulting velocity vector at point C
*/
void PanelAnalysis::VLMQmn(Vector3d const &LA, Vector3d const&LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, Vector3d &V) const
{
    //
    // C is the point where the induced speed is calculated
    // V is the resulting speed
    //
    // Vectorial operations are written explicitly to save computing times (4x more efficient)
    //

    Vector3d const *m_pR[5];
    double ftmp(0), Omega(0), Psi_x(0), Psi_y(0), Psi_z(0), r0_x(0), r0_y(0), r0_z(0), r1_x(0), r1_y(0), r1_z(0), r2_x(0), r2_y(0), r2_z(0);
    double r1v(0), r2v(0), t_x(0), t_y(0), t_z(0);


    //we use a default core size, unless the user has specified one
    double CoreSize = 0.0001;
    if(fabs(Panel::coreSize())>PRECISION) CoreSize = Panel::coreSize();


    V.x = 0.0;
    V.y = 0.0;
    V.z = 0.0;

    m_pR[0] = &LB;
    m_pR[1] = &TB;
    m_pR[2] = &TA;
    m_pR[3] = &LA;
    m_pR[4] = &LB;


    for (int i=0; i<4; i++)
    {
        r0_x = m_pR[i+1]->x - m_pR[i]->x;
        r0_y = m_pR[i+1]->y - m_pR[i]->y;
        r0_z = m_pR[i+1]->z - m_pR[i]->z;
        r1_x = C.x - m_pR[i]->x;
        r1_y = C.y - m_pR[i]->y;
        r1_z = C.z - m_pR[i]->z;
        r2_x = C.x - m_pR[i+1]->x;
        r2_y = C.y - m_pR[i+1]->y;
        r2_z = C.z - m_pR[i+1]->z;

        Psi_x = r1_y*r2_z - r1_z*r2_y;
        Psi_y =-r1_x*r2_z + r1_z*r2_x;
        Psi_z = r1_x*r2_y - r1_y*r2_x;

        ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

        r1v = sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z));
        r2v = sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

        //get the distance of the TestPoint to the panel's side
        t_x =  r1_y*r0_z - r1_z*r0_y;
        t_y = -r1_x*r0_z + r1_z*r0_x;
        t_z =  r1_x*r0_y - r1_y*r0_x;

        if ((t_x*t_x+t_y*t_y+t_z*t_z)/(r0_x*r0_x+r0_y*r0_y+r0_z*r0_z) > CoreSize * CoreSize)
        {
            Psi_x /= ftmp;
            Psi_y /= ftmp;
            Psi_z /= ftmp;

            Omega = (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/r1v - (r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/r2v;
            V.x += Psi_x * Omega/4.0/PI;
            V.y += Psi_y * Omega/4.0/PI;
            V.z += Psi_z * Omega/4.0/PI;
        }
    }
}



void PanelAnalysis::clearPOppList()
{
    for(int ip=m_PlaneOppList.count()-1; ip>=0; ip--)
    {
        delete m_PlaneOppList.at(ip);
        m_PlaneOppList.removeAt(ip);
    }
}


void PanelAnalysis::onCancel()
{
    s_bCancel = true;
    traceLog("Cancelling the panel analysis\n");
}


/**
* Calculates the induced lift and drag from the vortices or wake panels strength using a farfield method
* Downwash is evaluated at a distance 100 times the span downstream (i.e. infinite)
*/
void PanelAnalysis::panelTrefftz(Wing *pWing, double QInf, double Alpha, double const*Mu, double const*Sigma, int pos,
                                 Vector3d &Force, double &WingIDrag,
                                 WPolar const*pWPolar, Panel const*pWakePanel, Vector3d const*pWakeNode) const
{
    int nw(0), iTA(0), iTB(0);
    int pp(0);
    double InducedAngle(0), cosa(0), sina(0);
    QVector<double> GammaStrip;
    Vector3d C, Wg, dF, StripForce, WindDirection, WindNormal, VInf;

    /*    if(pWPolar->m_bTiltedGeom)
    {
        cosa = 1.0;
        sina = 0.0;
    }
    else
    {*/
    cosa = cos(Alpha*PI/180.0);
    sina = sin(Alpha*PI/180.0);
    //    }

    //   Define wind axis
    WindNormal.set(   -sina, 0.0, cosa);
    WindDirection.set( cosa, 0.0, sina);

    VInf = WindDirection * QInf;

    //dynamic pressure, kg/m3
    double q = 0.5 * pWPolar->density() * QInf * QInf;

    pWing->m_WingCL = 0.0;
    WingIDrag = 0.0;

    int coef = 2;
    if (pWPolar->bThinSurfaces()) coef = 1;

    int NSurfaces = pWing->m_Surface.size();

    int p=0;
    int m=0;
    for (int j=0; j<NSurfaces; j++)
    {
        Surface const *pSurf = pWing->surface(j);
        if(pSurf->isTipLeft() && !pWPolar->bThinSurfaces()) p += pSurf->nXPanels();         //tip patch panels

        Vector3d surfaceNormal(pSurf->normal());
        for (int k=0; k<pSurf->nYPanels(); k++)
        {
            pp = p;
            pWing->m_StripArea[m] = 0.0;
            StripForce.set(0.0,0.0,0.0);
            for (int l=0; l<coef*pSurf->nXPanels(); l++)
            {
                pWing->m_StripArea[m] += m_pPanel[pWing->firstPanelIndex()+pp].area();
                pp++;
            }
            pWing->m_StripArea[m] /= double(coef);

            Panel const &panel = m_pPanel[pWing->firstPanelIndex()+p];

            if(!pWPolar->bThinSurfaces())
            {
                // ____________________________
                // Downwash calculation
                //
                // Since we place the trailing point at the end of the wake panels, it sees only the effect
                // of the upstream part of the wake because the downstream part isn't modelled.
                // The downwash in this plane is directly the wing's downwash
                // If we were to model the downstream part, the total induced speed would be twice larger,
                // so just add a factor 2 to account for this.
                nw  = panel.m_iWake;
                iTA = pWakePanel[nw].m_iTA;
                iTB = pWakePanel[nw].m_iTB;
                C = (pWakeNode[iTA] + pWakeNode[iTB])/2.0;
                getSpeedVector(C, Mu, Sigma, Wg, false);

                pWing->m_Vd[m] = Wg;
                InducedAngle = atan2(Wg.dot(surfaceNormal), QInf);
                pWing->m_Ai[m] = InducedAngle*180.0/PI;
                //    qDebug("%13.7g %13.7g %13.7g %13.7g %13.7g %13.7g %13.7g ", C.x, C.y, C.z, Wg.x, Wg.y, Wg.z, pWing->m_Ai[m]);

                // ____________________________
                // Lift calculation
                //
                // Method 1 : Sum panel pressure forces over the top and bottom strip.
                // The induced drag is calculated by projection of the strip force on the wind direction
                // General experience in published literature shows that this isn't such a good idea

                // Method 2 : Far-field plane integration
                // This is the method generally recommended
                GammaStrip.append((-Mu[pos+p+coef*pSurf->m_NXPanels-1] + Mu[pos+p]) *4.0*PI);
                Wg += VInf;

                StripForce  = panel.Vortex * Wg;
                StripForce *= GammaStrip.at(m) * pWPolar->density() / q;  // N/q

                //____________________________
                // Project on wind axes
                pWing->m_Cl[m]    = StripForce.dot(surfaceNormal)   /pWing->m_StripArea[m];
                pWing->m_ICd[m]   = StripForce.dot(WindDirection)   /pWing->m_StripArea[m];
                WingIDrag += StripForce.dot(WindDirection);          // N/q
            }
            else
            {
                pp=p;
                for(int l=0; l<pSurf->m_NXPanels; l++)
                {

                    Panel const &panel_pp = m_pPanel[pWing->firstPanelIndex()+pp];

                    if(pWPolar->bVLM1() || panel_pp.m_bIsTrailing)
                    {
                        C = panel_pp.CtrlPt;
                        C.x = pWing->m_PlanformSpan * 1000.0;

                        getSpeedVector(C, Mu, Sigma, Wg, false);

                        // The trailing point sees both the upstream and downstream parts of the trailing vortices
                        // Hence it sees twice the downwash.
                        // So divide by 2 to account for this.
                        Wg *= 1.0/2.0;

                        if(panel_pp.m_bIsTrailing)
                        {
                            pWing->m_Vd[m]      = Wg;
                            InducedAngle = atan2(Wg.dot(surfaceNormal), QInf);
                            pWing->m_Ai[m]      = InducedAngle*180/PI;
                        }

                        Wg += VInf; //total speed vector

                        //induced force
                        dF  = Wg * panel_pp.Vortex;
                        dF *= Mu[pp+pos];       // N/rho
                        StripForce += dF;       // N/rho
                    }

                    pp++;
                }
                StripForce *= 2./QInf/QInf; //N/q

                //____________________________
                // Project on wind axes
                pWing->m_Cl[m]    = StripForce.dot(surfaceNormal)   /pWing->m_StripArea[m];
                pWing->m_ICd[m]   = StripForce.dot(WindDirection)/pWing->m_StripArea[m];
                pWing->m_WingCL  += StripForce.dot(WindNormal);                // N/q
                WingIDrag += StripForce.dot(WindDirection);          // N/q
            }
            p += coef*pSurf->m_NXPanels;

            // Calculate resulting vector force
            Force     += StripForce;                          // N/q
            pWing->m_F[m]     = StripForce * q;                        // Newtons

            m++;
        }

        if(pSurf->isTipRight() && !pWPolar->bThinSurfaces()) p += pSurf->nXPanels();//tip patch panels
    }

    pWing->m_CDi = WingIDrag; // save this wing's induced drag (unused though...)
}


