/****************************************************************************

    PlaneTask Class

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


#include <QDebug>


#include "planetask.h"
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects3d/surface.h>


bool PlaneTask::s_bCancel = false;

PlaneTask::PlaneTask()
{
    m_pPlane = nullptr;
    m_pWPolar = nullptr;

    m_vMin = m_vMax = m_vInc = 0.0;
    m_MaxPanelSize = 0;
    m_bSequence = true;
    m_bIsFinished = false;

    m_WakeSize = 0;

    m_NWakeColumn = 0;
}


PlaneTask::~PlaneTask()
{
    releasePanelMemory();
}


void PlaneTask::initializeTask(Plane *pPlane, WPolar *pWPolar, double vMin, double vMax, double VInc, bool bSequence)
{
    m_pPlane = pPlane;
    m_pWPolar = pWPolar;

    m_vMin = vMin;
    m_vMax = vMax;
    m_vInc = VInc;
    m_bSequence = bSequence;
}


void PlaneTask::initializeTask(PlaneAnalysis *pAnalysis)
{
    m_pPlane  = pAnalysis->pPlane;
    m_pWPolar = pAnalysis->pWPolar;

    m_vMin = pAnalysis->vMin;
    m_vMax = pAnalysis->vMax;
    m_vInc = pAnalysis->vInc;
    m_bSequence = true;
}


void PlaneTask::run()
{
    if(s_bCancel || !m_pPlane || !m_pWPolar)
    {
        m_bIsFinished = true;
        return;
    }

    if(m_pWPolar->isLLTMethod())
    {
        LLTAnalyze();
    }
    else if(m_pWPolar->isQuadMethod())
    {
        PanelAnalyze();
    }

    m_bIsFinished = true;

    emit taskFinished();
}


bool PlaneTask::isLLTTask() const
{
    return (m_pWPolar && m_pWPolar->isLLTMethod());
}


bool PlaneTask::isPanelTask() const
{
    return (m_pWPolar && m_pWPolar->isQuadMethod());
}


/**
 * Sets the active plane
 * Constructs the surface
 * Calculates the inertia
 * @param PlaneName the name of the plane to be set as active
 */
Plane *PlaneTask::setPlaneObject(Plane *pPlane)
{
    m_pPlane = pPlane;
    if(!pPlane) return nullptr;

    double dx(0), dz(0);

    if(pPlane->body())
    {
        dx = pPlane->bodyPos().x;
        dz = pPlane->bodyPos().z;
        pPlane->body()->setNURBSKnots();
        pPlane->body()->setPanelPos();
    }

    Body *pCurBody = pPlane->body();

    //create the array of wing Surface
    m_SurfaceList.clear();
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pPlane->wing(iw))
        {
            if(iw<3)         pPlane->wing(iw)->createSurfaces(pPlane->wingLE(iw),   0.0, pPlane->wingTiltAngle(iw));
            else if(iw==3)   pPlane->wing(iw)->createSurfaces(pPlane->wingLE(iw), -90.0, pPlane->wingTiltAngle(iw));

            for (int j=0; j<pPlane->wing(iw)->m_Surface.size(); j++)
            {
                pPlane->wing(iw)->surface(j)->setMeshSidePoints(pCurBody, dx, dz);
                m_SurfaceList.append(pPlane->wing(iw)->surface(j));
            }
//            pPlane->wing(iw)->computeBodyAxisInertia(); // redundant with inertia calculation at plane level
        }
    }

    pPlane->computeBodyAxisInertia();

    return pPlane;
}


/**
 * Sets the active polar
 * Builds the array of panels depending on the polar type
 * @param bCurrent if true, the active polar is set anew
 * @param WPlrName the name of the polar to set for the active wing or plane
 */
WPolar* PlaneTask::setWPolarObject(Plane *pCurPlane, WPolar *pCurWPolar)
{
    int m=0, NStation=0;
    double SpanPos=0;

    if(!pCurPlane)
    {
        releasePanelMemory();
        return nullptr;
    }

    m_pWPolar = pCurWPolar;
    m_pPlane = pCurPlane;

    if(!m_pWPolar)
    {
        releasePanelMemory();
        return nullptr;
    }

    Wing *pWingList[MAXWINGS];
    pWingList[0] = pCurPlane->wing();
    pWingList[1] = pCurPlane->wing2();
    pWingList[2] = pCurPlane->stab();
    pWingList[3] = pCurPlane->fin();

    if(!m_pWPolar || m_pWPolar->analysisMethod()>xfl::LLTMETHOD)
    {
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(pWingList[iw])
            {
                pWingList[iw]->computeChords();
//for(int i=0; i<pWingList[iw]->m_NStation; i++) qDebug()<<"twist"<<pWingList[iw]->m_Twist[i];
                NStation = 0;
                m=0;
                SpanPos = 0;
                for (int j=0; j<pWingList[iw]->m_Surface.size(); j++)    NStation += pWingList[iw]->surface(j)->m_NYPanels;

                for (int j=int(pWingList[iw]->m_Surface.size()/2); j<pWingList[iw]->m_Surface.size(); j++)
                {
                    for(int k=0; k<pWingList[iw]->surface(j)->m_NYPanels; k++)
                    {
                        pWingList[iw]->m_SpanPos[m+NStation/2] = SpanPos + pWingList[iw]->surface(j)->stripSpanPos(k);
                        m++;
                    }
                    SpanPos += pWingList[iw]->surface(j)->m_Length;
                }

                for(m=0; m<NStation/2; m++) pWingList[iw]->m_SpanPos[m] = -pWingList[iw]->m_SpanPos[NStation-m-1];
            }
        }
    }
    else if(m_pWPolar->analysisMethod()==xfl::LLTMETHOD)
    {
//            pCurPlane->m_Wing[0].m_NStation  = m_NStation;
//            pCurPlane->m_Wing[0].m_bLLT      = true;
    }

    if(m_pWPolar->isPanel4Method())
    {
        // if more than one wing, force thin surfaces
        if(!m_pPlane->isWing()) m_pWPolar->setThinSurfaces(true);
    }

    if(!initializePanels()) return nullptr;

    if(!m_pWPolar) return nullptr;

    stitchSurfaces();

    //initialize the analysis pointers.
    //do it now, in case the user asks for streamlines from an existing file
    m_ptheLLTAnalysis->setWPolar(m_pWPolar);
    m_ptheLLTAnalysis->setPlane(pCurPlane);

    m_pthePanelAnalysis->setWPolar(m_pWPolar);
    m_pthePanelAnalysis->setObjectPointers(pCurPlane, &m_SurfaceList);
    m_pthePanelAnalysis->setArrayPointers(m_Panel.data(), m_MemPanel.data(), m_WakePanel.data(), m_RefWakePanel.data(),
                                          m_Node.data(), m_MemNode.data(),
                                          m_WakeNode.data(), m_RefWakeNode.data(), m_TempWakeNode.data());
    m_pthePanelAnalysis->setArraySize(m_Panel.size(), m_WakeSize, m_Node.size(), m_WakeNode.size(), m_NWakeColumn);

    /** @todo restore */
    //set sideslip
/*    Vector3d RefPoint(0.0, 0.0, 0.0);
    if(fabs(m_pWPolar->m_BetaSpec)>0.001 && !m_pWPolar->isBetaPolar())
    {
        // Standard Convention in mechanic of flight is to have Beta>0 with nose to the left
        // The yaw moement has the opposite convention...
        m_pthePanelAnalysis->rotateGeomZ(m_pWPolar->m_BetaSpec, RefPoint, m_pWPolar->m_NXWakePanels);
    }*/



    /** @todo need to cancel results too if we modify the inertia */
    if(m_pWPolar->m_bAutoInertia)
    {
        if(pCurPlane)
        {
            m_pWPolar->setMass(pCurPlane->totalMass());
            m_pWPolar->setCoG(pCurPlane->CoG());
            m_pWPolar->setCoGIxx(pCurPlane->CoGIxx());
            m_pWPolar->setCoGIyy(pCurPlane->CoGIyy());
            m_pWPolar->setCoGIzz(pCurPlane->CoGIzz());
            m_pWPolar->setCoGIxz(pCurPlane->CoGIxz());
        }
    }

    return m_pWPolar;
}


/**
 * Following the selection of a wing or a plane, this subroutine creates the panels
 * associated to all of the surface objects.
 *
 * m_Panel is the array of panels in the following order
 *         main wing left side, main wing right side
 *         second wing
 *         elevator
 *         fin
 *         body
 *
 * A copy of the panels is saved to the MemPanel and MemNode arrays
 *@return true if successful, false if the panels could not be properly created ot if no object is active
*/
bool PlaneTask::initializePanels()
{
    if(!m_pPlane) return false;

    // first check that the total number of panels that will be created does not exceed
    // the currently allocated memory size for the influence atrix.

    int PanelArraySize = calculateMatSize();
    int memsize = 0;

    //    if(PanelArraySize>m_MaxPanelSize)
    {
//        Trace(QString("PlaneTask::Requesting additional memory for %1 panels").arg(PanelArraySize));

        // allocate 10% more than needed to avoid repeating the operation if the user requirement increases sightly again.
        m_MaxPanelSize = int(double(PanelArraySize) *1.1);
        releasePanelMemory();

        if(!allocatePanelArrays(memsize))
        {
            m_MaxPanelSize = 0;
            return false;
        }
    }

    //if a WPolar is defined, allocate the matrix
    if(m_pWPolar)
    {
        int MatrixSize=0;

        if(!m_pthePanelAnalysis->allocateMatrix(m_MaxPanelSize, MatrixSize))
        {
            releasePanelMemory();
            return false;
        }

//        Trace("");
        memsize += MatrixSize;
    }

    // all set to create the panels
    m_NWakeColumn = 0;
    m_WakeSize    = 0;

    m_Node.fill(Vector3d());

    Wing *pWingList[MAXWINGS];
    pWingList[0] = m_pPlane->wing();
    pWingList[1] = m_pPlane->wing2();
    pWingList[2] = m_pPlane->stab();
    pWingList[3] = m_pPlane->fin();

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        Wing *pWing = pWingList[iw];
        if(!pWing) continue;

        int Nel = 0;
        pWing->setFirstPanelIndex(m_Panel.size());
        for(int jSurf=0; jSurf<pWing->surfaceCount(); jSurf++)
        {
            Surface *pSurf = pWing->surface(jSurf);
            pSurf->resetFlap();
            Nel += createSurfaceElements(m_pPlane, m_pWPolar, pSurf);
        }
        pWing->setPanelCoun(Nel);

//        qDebug()<<pWing->name()<<pWing->firstPanelIndex()<<pWing->nPanels();
    }

    bool bBodyEl = false;
    if(m_pPlane && m_pPlane->body())
    {
        if(!m_pWPolar) bBodyEl = true;//no risk...
        else if(m_pWPolar->analysisMethod()==xfl::PANEL4METHOD && !m_pWPolar->bIgnoreBodyPanels())
        {
            bBodyEl = true;
        }
    }

    if(bBodyEl)
    {
        if(m_pPlane && m_pPlane->body())
        {
            m_pPlane->body()->setFirstPanelIndex(m_Panel.size());
            createBodyElements(m_pPlane);
//            qDebug()<<m_pPlane->body()->name()<<m_pPlane->body()->firstPanelIndex()<<m_pPlane->body()->nPanels();
        }
    }

    //back-up the current geometry
    m_MemPanel     = m_Panel;
    m_RefWakePanel = m_WakePanel;
    m_RefWakeNode  = m_WakeNode;
    m_MemNode      = m_Node;

    return true;
}


/**
 * Creates the body panels for the active Body object
 * The panels are created in the following order
 *    - for the port side  first, then for the starboard side
 *    - from bottom to top
 *    - from tip to tail
 * The panels are appended to the existing array of panels
 * @return the number of panels which have been created and appended
 * @todo use Body::makePanels() instead
 */
int PlaneTask::createBodyElements(Plane *pCurPlane)
{
    if(!pCurPlane) return 0;
    if(!pCurPlane->body()) return 0;

    Body *pCurBody = pCurPlane->body();

    double uk(0), uk1(0), v(0), dj(0), dj1(0), dl1(0);
    double dpx(0), dpz(0);
    Vector3d LATB, TALB;
    Vector3d LA, LB, TA, TB;
    Vector3d PLA, PTA, PLB, PTB;

    int n0(0), n1(0), n2(0), n3(0), lnx(0), lnh(0);
    int nx = pCurBody->nxPanels();
    int nh = pCurBody->nhPanels();
    int p = 0;

    int m_MatSize = m_Panel.size();
    int InitialSize = m_Panel.size();
    int FullSize =0;

    lnx = 0;

    if(pCurPlane && pCurPlane->body())
    {
        dpx = pCurPlane->bodyPos().x;
        dpz = pCurPlane->bodyPos().z;
    }
    else dpx=dpz=0.0;

    if(pCurBody->isFlatPanelType())
    {
        nx = 0;
        for(int i=0; i<pCurBody->frameCount()-1; i++) nx+=pCurBody->m_xPanels.at(i);
        nh = 0;
        for(int i=0; i<pCurBody->sideLineCount()-1; i++) nh+=pCurBody->m_hPanels.at(i);
        FullSize = nx*nh*2;
        pCurBody->setNXPanels(nx);
        pCurBody->setNHPanels(nh);

        for (int i=0; i<pCurBody->frameCount()-1; i++)
        {
            for (int j=0; j<pCurBody->m_xPanels[i]; j++)
            {
                dj  = double( j) /double(pCurBody->m_xPanels.at(i));
                dj1 = double(j+1)/double(pCurBody->m_xPanels.at(i));

                //body left side
                lnh = 0;
                for (int k=0; k<pCurBody->sideLineCount()-1; k++)
                {
                    //build the four corner points of the strips
                    PLB.x =  (1.0- dj) * pCurBody->framePosition(i)                +  dj * pCurBody->framePosition(i+1)                 +dpx;
                    PLB.y = -(1.0- dj) * pCurBody->frame(i)->m_CtrlPoint.at(k).y   -  dj * pCurBody->frame(i+1)->m_CtrlPoint.at(k).y;
                    PLB.z =  (1.0- dj) * pCurBody->frame(i)->m_CtrlPoint.at(k).z   +  dj * pCurBody->frame(i+1)->m_CtrlPoint.at(k).z    +dpz;

                    PTB.x =  (1.0-dj1) * pCurBody->framePosition(i)                + dj1 * pCurBody->framePosition(i+1)                 +dpx;
                    PTB.y = -(1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint.at(k).y   - dj1 * pCurBody->frame(i+1)->m_CtrlPoint.at(k).y;
                    PTB.z =  (1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint.at(k).z   + dj1 * pCurBody->frame(i+1)->m_CtrlPoint.at(k).z    +dpz;

                    PLA.x =  (1.0- dj) * pCurBody->framePosition(i)                +  dj * pCurBody->framePosition(i+1)                 +dpx;
                    PLA.y = -(1.0- dj) * pCurBody->frame(i)->m_CtrlPoint.at(k+1).y -  dj * pCurBody->frame(i+1)->m_CtrlPoint.at(k+1).y;
                    PLA.z =  (1.0- dj) * pCurBody->frame(i)->m_CtrlPoint.at(k+1).z +  dj * pCurBody->frame(i+1)->m_CtrlPoint.at(k+1).z  +dpz;

                    PTA.x =  (1.0-dj1) * pCurBody->framePosition(i)                + dj1 * pCurBody->framePosition(i+1)                 +dpx;
                    PTA.y = -(1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint.at(k+1).y - dj1 * pCurBody->frame(i+1)->m_CtrlPoint.at(k+1).y;
                    PTA.z =  (1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint.at(k+1).z + dj1 * pCurBody->frame(i+1)->m_CtrlPoint.at(k+1).z  +dpz;

                    LB = PLB;
                    TB = PTB;

                    for (int l=0; l<pCurBody->m_hPanels.at(k); l++)
                    {
                        dl1  = double(l+1) / double(pCurBody->m_hPanels.at(k));
                        LA = PLB * (1.0- dl1) + PLA * dl1;
                        TA = PTB * (1.0- dl1) + PTA * dl1;

                        n0 = isNode(LA);
                        n1 = isNode(TA);
                        n2 = isNode(LB);
                        n3 = isNode(TB);

                        m_Panel.push_back(Panel());
                        Panel &panel = m_Panel.back();

                        if(n0>=0) {
                            panel.m_iLA = n0;
                        }
                        else {
                            panel.m_iLA = m_Node.size();
                            m_Node.push_back(LA);
                        }

                        if(n1>=0) {
                            panel.m_iTA = n1;
                        }
                        else {
                            panel.m_iTA = m_Node.size();
                            m_Node.push_back(TA);
                        }

                        if(n2>=0) {
                            panel.m_iLB = n2;
                        }
                        else {
                            panel.m_iLB = m_Node.size();
                            m_Node.push_back(LB);
                        }

                        if(n3 >=0) {
                            panel.m_iTB = n3;
                        }
                        else {
                            panel.m_iTB = m_Node.size();
                            m_Node.push_back(TB);
                        }

                        LATB = TB - LA;
                        TALB = LB - TA;
                        panel.Normal = LATB * TALB;
                        panel.Area =  panel.Normal.norm()/2.0;
                        panel.Normal.normalize();

                        panel.m_bIsInSymPlane  = false;
                        panel.m_bIsLeading     = false;
                        panel.m_bIsTrailing    = false;
                        panel.m_Pos = xfl::BODYSURFACE;
                        panel.m_iElement = m_MatSize;
                        panel.m_bIsLeftPanel  = true;
                        panel.setPanelFrame(LA, LB, TA, TB);

                        // set neighbour panels

                        panel.m_iPD = m_MatSize + nh;
                        panel.m_iPU = m_MatSize - nh;

                        if(lnx==0)      panel.m_iPU = -1;// no panel downstream
                        if(lnx==nx-1)   panel.m_iPD = -1;// no panel upstream

                        panel.m_iPL = m_MatSize + 1;
                        panel.m_iPR = m_MatSize - 1;

                        if(lnh==0)     panel.m_iPR = InitialSize + FullSize - p - 1;
                        if(lnh==nh-1)  panel.m_iPL = InitialSize + FullSize - p - 1;

                        m_MatSize++;
                        p++;
                        LB = LA;
                        TB = TA;
                        lnh++;
                    }
                }
                lnx++;
            }
        }
    }
    else if(pCurBody->isSplineType())
    {
        FullSize = 2*nx*nh;
        //start with left side... same as for wings
        for (int k=0; k<nx; k++)
        {
            uk  = pCurBody->m_XPanelPos.at(k);
            uk1 = pCurBody->m_XPanelPos.at(k+1);

            pCurBody->getPoint(uk,  0, false, LB);
            pCurBody->getPoint(uk1, 0, false, TB);

            LB.x += dpx;
            LB.z += dpz;
            TB.x += dpx;
            TB.z += dpz;

            for (int l=0; l<nh; l++)
            {
                //start with left side... same as for wings
                v = double(l+1) / double(nh);
                pCurBody->getPoint(uk,  v, false, LA);
                pCurBody->getPoint(uk1, v, false, TA);

                LA.x += dpx;
                LA.z += dpz;
                TA.x += dpx;
                TA.z += dpz;

                n0 = isNode(LA);
                n1 = isNode(TA);
                n2 = isNode(LB);
                n3 = isNode(TB);

                m_Panel.push_back(Panel());
                Panel &panel = m_Panel.back();

                if(n0>=0) {
                    panel.m_iLA = n0;
                }
                else {
                    panel.m_iLA = m_Node.size();
                    m_Node.push_back(LA);
                }

                if(n1>=0) {
                    panel.m_iTA = n1;
                }
                else {
                    panel.m_iTA = m_Node.size();
                    m_Node.push_back(TA);
                }

                if(n2>=0) {
                    panel.m_iLB = n2;
                }
                else {
                    panel.m_iLB = m_Node.size();
                    m_Node.push_back(LB);
                }

                if(n3 >=0) {
                    panel.m_iTB = n3;
                }
                else {
                    panel.m_iTB = m_Node.size();
                    m_Node.push_back(TB);
                }

                LATB = TB - LA;
                TALB = LB - TA;
                panel.Normal = LATB * TALB;
                panel.Area =  panel.Normal.norm()/2.0;
                panel.Normal.normalize();

                panel.m_bIsInSymPlane  = false;
                panel.m_bIsLeading     = false;
                panel.m_bIsTrailing    = false;
                panel.m_Pos = xfl::BODYSURFACE;
                panel.m_iElement = m_MatSize;
                panel.m_bIsLeftPanel  = true;
                panel.setPanelFrame(LA, LB, TA, TB);

                // set neighbour panels

                panel.m_iPD = m_MatSize + nh;
                panel.m_iPU = m_MatSize - nh;

                if(k==0)    panel.m_iPU = -1;// no panel downstream
                if(k==nx-1) panel.m_iPD = -1;// no panel upstream

                panel.m_iPL = m_MatSize + 1;
                panel.m_iPR = m_MatSize - 1;

                if(l==0)     panel.m_iPR = InitialSize + FullSize - p - 1;
                if(l==nh-1)  panel.m_iPL = InitialSize + FullSize - p - 1;

                LB = LA;
                TB = TA;
                m_MatSize++;
                p++;
            }
        }
    }

    //right side next
    int i = m_MatSize;

    for (int k=nx-1; k>=0; k--)
    {
        for (int l=nh-1; l>=0; l--)
        {
            i--;
            LA = m_Node[m_Panel[i].m_iLB];
            TA = m_Node[m_Panel[i].m_iTB];
            LB = m_Node[m_Panel[i].m_iLA];
            TB = m_Node[m_Panel[i].m_iTA];

            LA.y = -LA.y;
            LB.y = -LB.y;
            TA.y = -TA.y;
            TB.y = -TB.y;

            n0 = isNode(LA);
            n1 = isNode(TA);
            n2 = isNode(LB);
            n3 = isNode(TB);

            m_Panel.push_back(Panel());
            Panel &panel = m_Panel.back();

            if(n0>=0) {
                panel.m_iLA = n0;
            }
            else {
                panel.m_iLA = m_Node.size();
                m_Node.push_back(LA);
            }

            if(n1>=0) {
                panel.m_iTA = n1;
            }
            else {
                panel.m_iTA = m_Node.size();
                m_Node.push_back(TA);
            }

            if(n2>=0) {
                panel.m_iLB = n2;
            }
            else {
                panel.m_iLB = m_Node.size();
                m_Node.push_back(LB);
            }

            if(n3 >=0) {
                panel.m_iTB = n3;
            }
            else {
                panel.m_iTB = m_Node.size();
                m_Node.push_back(TB);
            }

            LATB = TB - LA;
            TALB = LB - TA;
            panel.Normal = LATB * TALB;
            panel.Area =  panel.Normal.norm()/2.0;
            panel.Normal.normalize();

            panel.m_bIsInSymPlane  = false;
            panel.m_bIsLeading     = false;
            panel.m_bIsTrailing    = false;
            panel.m_Pos = xfl::BODYSURFACE;
            panel.m_iElement = m_MatSize;
            panel.m_bIsLeftPanel  = false;
            panel.setPanelFrame(LA, LB, TA, TB);

            // set neighbour panels
            // valid only for Panel Analysis

            panel.m_iPD = m_MatSize - nh;
            panel.m_iPU = m_MatSize + nh;

            if(k==0)    panel.m_iPU = -1;// no panel downstream
            if(k==nx-1)    panel.m_iPD = -1;// no panel upstream

            panel.m_iPL = m_MatSize + 1;
            panel.m_iPR = m_MatSize - 1;

            if(l==0)     panel.m_iPL = InitialSize + FullSize - p - 1;
            if(l==nh-1)  panel.m_iPR = InitialSize + FullSize - p - 1;

            LB = LA;
            TB = TA;
            m_MatSize++;
            p++;
        }
    }
    pCurBody->m_NElements = m_MatSize-InitialSize;
    return pCurBody->m_NElements;
}


/**
* Creates the panel elements for the active surface.
* The panels are appended at the end of the existing array of panel
* The panels are created from left to right on a surface
* The panels are created depending on the current WPolar:
*   No WPolar --> panel elements on top & bottom surfaces, just for cosmetics
*   VLM       --> panel elements on mid camber line from T.E. to L.E.
*   3D Panels --> panel elements on left tip surface
*                 panel elements on each strip, starting on the bottom T.E. to the L.E back to the opt T.E
*                 panel elements on right tip surface
*
*@param a pointer to the surface for which the panels will be created
*@return the number of panels which have been created and appended
*/
int PlaneTask::createSurfaceElements(Plane const*pPlane, WPolar const*pWPolar, Surface *pSurface)
{
    //TODO : for  a gap at the wing's center, need to separate m_iPL and m_iPR at the tips;
    bool bNoJoinFlap(true);
    int n0(0), n1(0), n2(0), n3(0);

    int matsize = m_Panel.size();
    int initialsize = matsize;
    xfl::enumSurfacePosition side = xfl::MIDSURFACE;
    Vector3d LA, LB, TA, TB;

    bool bThickSurfaces = true;
    if(!pPlane->isWing()) bThickSurfaces= false;
    if(pWPolar)
    {
        if(pWPolar->analysisMethod() == xfl::LLTMETHOD) bThickSurfaces = false;
        if(pWPolar->analysisMethod() == xfl::VLMMETHOD) bThickSurfaces = false;
        if(pWPolar->bThinSurfaces()) bThickSurfaces = false;
    }

    if (bThickSurfaces && pWPolar && pSurface->isTipLeft())
    {
        //then left tip surface, add side panels
        for (int l=0; l<pSurface->nXPanels(); l++)
        {
            m_Panel.append(Panel());
            Panel &panel = m_Panel.last();
            panel.m_bIsLeading     = false;
            panel.m_bIsTrailing    = false;
            panel.m_bIsWakePanel   = false;
            panel.m_bIsInSymPlane  = false; //even if part of a fin

            pSurface->getPanel(0, l, xfl::BOTSURFACE);
            LA.copy(pSurface->LA);
            TA.copy(pSurface->TA);

            pSurface->getPanel(0, l, xfl::TOPSURFACE);
            LB.copy(pSurface->LA);
            TB.copy(pSurface->TA);

            n0 = isNode(LA);
            if(n0>=0)     panel.m_iLA = n0;
            else {
                panel.m_iLA = m_Node.size();
                m_Node.push_back(LA);
            }

            n1 = isNode(TA);
            if(n1>=0)     panel.m_iTA = n1;
            else {
                panel.m_iTA = m_Node.size();
                m_Node.push_back(TA);
            }

            n2 = isNode(LB);
            if(n2>=0)     panel.m_iLB = n2;
            else {
                panel.m_iLB = m_Node.size();
                m_Node.push_back(LB);
            }

            n3 = isNode(TB);
            if(n3>=0)     panel.m_iTB = n3;
            else {
                panel.m_iTB = m_Node.size();
                m_Node.push_back(TB);
            }

            panel.m_Pos = xfl::SIDESURFACE;
            panel.m_iElement = matsize;
            panel.m_bIsLeftPanel  = pSurface->isLeftSurf();
            panel.setPanelFrame(LA, LB, TA, TB);
            panel.m_iWake = -1;

            panel.m_iPD = matsize-1;
            panel.m_iPU = matsize+1;
            if(l==0)                      panel.m_iPD = -1;// no panel downstream
            if(l==pSurface->nXPanels()-1) panel.m_iPU = -1;// no panel upstream
            panel.m_iPL = -1;
            panel.m_iPR = -1;

            matsize++;
        }
    }

    for (int k=0; k<pSurface->nYPanels(); k++)
    {
        //add "horizontal" panels, mid side, or following a strip from bot to top if 3D Panel
        if(bThickSurfaces)   side = xfl::BOTSURFACE;  //start with lower surf, as recommended by K&P
        else                 side = xfl::MIDSURFACE;
        //from T.E. to L.E.
        for (int l=0; l<pSurface->nXPanels(); l++)
        {
            pSurface->getPanel(k,l,side);
            Q_ASSERT(!isnan(pSurface->LA.x));
            n0 = isNode(pSurface->LA);
            n1 = isNode(pSurface->TA);
            n2 = isNode(pSurface->LB);
            n3 = isNode(pSurface->TB);

            m_Panel.push_back(Panel());
            Panel &panel = m_Panel.back();

            if(l==0)                      panel.m_bIsTrailing = true;
            if(l==pSurface->nXPanels()-1) panel.m_bIsLeading  = true;
            panel.m_bIsWakePanel   = false;
            panel.m_bIsInSymPlane  = pSurface->isInSymPlane();

            bNoJoinFlap = side==0 &&  l<pSurface->nXFlap() && k==0;

            if(n0>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
            {
                panel.m_iLA = n0;
            }
            else {
                panel.m_iLA = m_Node.size();
                m_Node.push_back(pSurface->LA);
            }

            if(n1>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
            {
                panel.m_iTA = n1;
            }
            else {
                panel.m_iTA = m_Node.size();
                m_Node.push_back(pSurface->TA);
            }

            bNoJoinFlap = side==0 &&  l<pSurface->nXFlap() && k==pSurface->nYPanels()-1;

            if(n2>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
            {
                panel.m_iLB = n2;
            }
            else {
                panel.m_iLB = m_Node.size();
                m_Node.push_back(pSurface->LB);
            }

            if(n3>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
            {
                panel.m_iTB = n3;
            }
            else {
                panel.m_iTB = m_Node.size();
                m_Node.push_back(pSurface->TB);
            }

            panel.m_Pos = side;
            panel.m_iElement = matsize;
            panel.m_bIsLeftPanel  = pSurface->isLeftSurf();

            if(side==xfl::MIDSURFACE)        panel.setPanelFrame(pSurface->LA, pSurface->LB, pSurface->TA, pSurface->TB);
            else if (side==xfl::BOTSURFACE)  panel.setPanelFrame(pSurface->LB, pSurface->LA, pSurface->TB, pSurface->TA);

            // set neighbour panels
            // valid only for Panel 2-sided Analysis
            // we are on the bottom or middle surface
            panel.m_iPD = matsize-1;
            panel.m_iPU = matsize+1;
            if(l==0)                                         panel.m_iPD = -1;// no panel downstream
            if(l==pSurface->nXPanels()-1 && side==xfl::MIDSURFACE) panel.m_iPU = -1;// no panel upstream

            if(side!=xfl::MIDSURFACE)
            {
                //wings are modelled as thick surfaces
                panel.m_iPL = matsize + 2*pSurface->nXPanels();
                panel.m_iPR = matsize - 2*pSurface->nXPanels();
                //todo : do not link to right wing if there is a gap in between
                if(k==0                      && pSurface->isTipLeft())  panel.m_iPR = -1;
                if(k==pSurface->nYPanels()-1 && pSurface->isTipRight()) panel.m_iPL = -1;
            }
            else
            {
                //wings are modelled as thin surfaces
                panel.m_iPR = matsize + pSurface->nXPanels();
                panel.m_iPL = matsize - pSurface->nXPanels();
                if(k==0                      && pSurface->isTipLeft())  panel.m_iPL = -1;
                if(k==pSurface->nYPanels()-1 && pSurface->isTipRight()) panel.m_iPR = -1;
            }

            //do not link to next surfaces... will be done in JoinSurfaces() if surfaces are continuous
            if(k==0)                      panel.m_iPR = -1;
            if(k==pSurface->nYPanels()-1) panel.m_iPL = -1;

            if(pWPolar && panel.m_bIsTrailing && pWPolar->analysisMethod()==xfl::PANEL4METHOD)
            {
                panel.m_iWake = m_WakeSize;//next wake element
                panel.m_iWakeColumn = m_NWakeColumn;
                if(pWPolar->bThinSurfaces())
                {
                    createWakeElems(matsize, pPlane, pWPolar);
                    m_NWakeColumn++;
                }
            }

            if(l<pSurface->nXFlap()) pSurface->addFlapPanel(m_Panel.at(matsize));

            matsize++;
        }

        if (bThickSurfaces)
        {
            //add top side if 3D Panels
            side = xfl::TOPSURFACE; //next upper surf, as recommended by K&P
            //from L.E. to T.E.
            for (int l=pSurface->nXPanels()-1;l>=0; l--)
            {
                pSurface->getPanel(k,l,side);
                n0 = isNode(pSurface->LA);
                n1 = isNode(pSurface->TA);
                n2 = isNode(pSurface->LB);
                n3 = isNode(pSurface->TB);


                m_Panel.push_back(Panel());
                Panel &panel = m_Panel.back();

                if(l==0)                      panel.m_bIsTrailing = true;
                if(l==pSurface->nXPanels()-1) panel.m_bIsLeading  = true;
                panel.m_bIsWakePanel   = false;
                panel.m_bIsInSymPlane  = pSurface->isInSymPlane();

                if(n0>=0)
                    panel.m_iLA = n0;
                else {
                    panel.m_iLA = m_Node.size();
                    m_Node.push_back(pSurface->LA);
                }

                if(n1>=0)
                    panel.m_iTA = n1;
                else {
                    panel.m_iTA = m_Node.size();
                    m_Node.push_back(pSurface->TA);
                }

                if(n2>=0)
                    panel.m_iLB = n2;
                else {
                    panel.m_iLB = m_Node.size();
                    m_Node.push_back(pSurface->LB);
                }

                if(n3 >=0)
                    panel.m_iTB = n3;
                else {
                    panel.m_iTB = m_Node.size();
                    m_Node.push_back(pSurface->TB);
                }

                panel.m_Pos = side;
                panel.m_iElement = matsize;
                panel.m_bIsLeftPanel  = pSurface->isLeftSurf();

                panel.setPanelFrame(pSurface->LA, pSurface->LB, pSurface->TA, pSurface->TB);

                // set neighbour panels
                // valid only for Panel 2-sided Analysis
                // we are on the top surface
                panel.m_iPD = matsize+1;
                panel.m_iPU = matsize-1;
                if(l==0)                      panel.m_iPD = -1;// no panel downstream
//                if(l==pSurface->NXPanels()-1) panel.m_iPU = -1;// no panel upstream

                panel.m_iPL = matsize - 2*pSurface->nXPanels();//assuming all wing panels have same chordwise distribution
                panel.m_iPR = matsize + 2*pSurface->nXPanels();//assuming all wing panels have same chordwise distribution

                if(k==0                      && pSurface->isTipLeft())    panel.m_iPL = -1;
                if(k==pSurface->nYPanels()-1 && pSurface->isTipRight())   panel.m_iPR = -1;

                //do not link to next surfaces... will be done in JoinSurfaces() if surfaces are continuous
                if(k==0)                      panel.m_iPL = -1;
                if(k==pSurface->nYPanels()-1) panel.m_iPR = -1;


                if(pWPolar && panel.m_bIsTrailing && pWPolar->analysisMethod()==xfl::PANEL4METHOD)
                {
                    panel.m_iWake = m_WakeSize;//next wake element
                    panel.m_iWakeColumn = m_NWakeColumn;
                    createWakeElems(matsize, pPlane, pWPolar);
                }

                if(l<pSurface->nXFlap()) pSurface->addFlapPanel(m_Panel.at(matsize));
                matsize++;
            }
            m_NWakeColumn++;
        }
    }

    if (bThickSurfaces && pWPolar && pSurface->isTipRight())
    {    //right tip surface
        int k = pSurface->nYPanels()-1;
        for (int l=0; l< pSurface->nXPanels(); l++)
        {
            m_Panel.push_back(Panel());
            Panel &panel = m_Panel.back();

            panel.m_bIsTrailing    = false;
            panel.m_bIsLeading     = false;
            panel.m_bIsWakePanel   = false;
            panel.m_bIsInSymPlane  = false;//even if part of a fin

            pSurface->getPanel(k,l,xfl::TOPSURFACE);
            LA.copy(pSurface->LB);
            TA.copy(pSurface->TB);

            pSurface->getPanel(k,l,xfl::BOTSURFACE);
            LB.copy(pSurface->LB);
            TB.copy(pSurface->TB);

            n0 = isNode(LA);//answer should be yes
            if(n0>=0)                 panel.m_iLA = n0;
            else {
                panel.m_iLA = m_Node.size();
                m_Node.push_back(LA);
            }
            n1 = isNode(TA);//answer should be yes
            if(n1>=0)                 panel.m_iTA = n1;
            else {
                panel.m_iTA = m_Node.size();
                m_Node.push_back(TA);
            }
            n2 = isNode(LB);//answer should be yes
            if(n2>=0)                 panel.m_iLB = n2;
            else {
                panel.m_iLB = m_Node.size();
                m_Node.push_back(LB);
            }
            n3 = isNode(TB);//answer should be yes
            if(n3>=0)                panel.m_iTB = n3;
            else {
                panel.m_iTB = m_Node.size();
                m_Node.push_back(TB);
            }


            panel.m_iPD = matsize-1;
            panel.m_iPU = matsize+1;
            if(l==0)                      panel.m_iPD = -1;// no panel downstream
            if(l==pSurface->nXPanels()-1) panel.m_iPU = -1;// no panel upstream
            panel.m_iPL = -1;
            panel.m_iPR = -1;

            panel.m_Pos = xfl::SIDESURFACE;
            panel.m_iElement = matsize;
            panel.m_bIsLeftPanel  = pSurface->isLeftSurf();
            panel.setPanelFrame(LA, LB, TA, TB);
            panel.m_iWake = -1;
            matsize++;
        }
    }

    pSurface->setNElements(matsize-initialsize);
    return pSurface->NElements();
}


/**
* Creates a column of wake elements shed from a panel at the trailing edge of the wing's surface
* @param PanelIndex the index of the panel on the trailing edge of the surface which will shed the column of wake panels
*/
bool PlaneTask::createWakeElems(int PanelIndex, Plane const*pPlane, WPolar const*pWPolar)
{
    if(!pWPolar) return false;
    if(!m_Panel[PanelIndex].m_bIsTrailing) return false;

    int n0(0), n1(0), n2(0), n3(0);
    int mw = m_WakeSize;// number of wake panels
    Vector3d LATB, TALB;
    Vector3d LA, LB, TA,TB;//wake panel's corner points

    int NXWakePanels=0;
    double WakePanelFactor=0;
    double TotalWakeLength=0;

    NXWakePanels    = pWPolar->m_NXWakePanels;
    WakePanelFactor = pWPolar->m_WakePanelFactor;
    TotalWakeLength = pWPolar->m_TotalWakeLength;

    TA = m_Node[m_Panel[PanelIndex].m_iTA];
    TB = m_Node[m_Panel[PanelIndex].m_iTB];
    double dxA = TotalWakeLength*pPlane->mac() - m_Node[m_Panel[PanelIndex].m_iTA].x;
    double dxB = TotalWakeLength*pPlane->mac() - m_Node[m_Panel[PanelIndex].m_iTB].x;

    if(WakePanelFactor==1.0)
    {
        dxA /= NXWakePanels;
        dxB /= NXWakePanels;
    }
    else
    {
        dxA *= (1.0-WakePanelFactor)/(1.0-pow(WakePanelFactor, NXWakePanels));
        dxB *= (1.0-WakePanelFactor)/(1.0-pow(WakePanelFactor, NXWakePanels));
    }

    for (int l=0; l<NXWakePanels; l++)
    {
        Panel &wakepanel = m_WakePanel[mw];
        LA = TA;
        LB = TB;
        TA.x += dxA;
        TB.x += dxB;
//        TA += TE * dxA;
//        TB += TE * dxB;
        dxA *= WakePanelFactor;
        dxB *= WakePanelFactor;

        n0 = isWakeNode(LA);
        n1 = isWakeNode(TA);
        n2 = isWakeNode(LB);
        n3 = isWakeNode(TB);

        if(n0>=0) {
            wakepanel.m_iLA = n0;
        }
        else {
            wakepanel.m_iLA = m_WakeNode.size();
            m_WakeNode.push_back(LA);
        }

        if(n1>=0) {
            wakepanel.m_iTA = n1;
        }
        else {
            wakepanel.m_iTA = m_WakeNode.size();
            m_WakeNode.push_back(TA);
        }

        if(n2>=0) {
            wakepanel.m_iLB = n2;
        }
        else {
            wakepanel.m_iLB = m_WakeNode.size();
            m_WakeNode.push_back(LB);
        }

        if(n3 >=0) {
            wakepanel.m_iTB = n3;
        }
        else {
            wakepanel.m_iTB = m_WakeNode.size();
            m_WakeNode.push_back(TB);
        }

        LATB = TB - LA;
        TALB = LB - TA;

        wakepanel.Normal = LATB * TALB;

        wakepanel.m_Pos = xfl::MIDSURFACE;
        wakepanel.m_bIsWakePanel = true;
        wakepanel.Area =  wakepanel.Normal.norm()/2.0;
        wakepanel.Normal.normalize();
        wakepanel.setPanelFrame(LA,LB, TA, TB);
        wakepanel.m_bIsLeftPanel  = false;

        if     (l==0)               wakepanel.m_iPD = -1;// no panel downstream
        else if(l==NXWakePanels)    wakepanel.m_iPU = -1;// no panel upstream
        else                        wakepanel.m_iPD = mw+1;

        wakepanel.m_iPL = -1;
        wakepanel.m_iPR = -1;

        mw++;
    }

    m_WakeSize = mw;

    return true;
}


int PlaneTask::calculateMatSize()
{
    int PanelArraySize = 0;

    if(!m_pPlane) return 0;

    //Count the wing panels
    for (int js=0; js<m_SurfaceList.size(); js++)
    {
        PanelArraySize += m_SurfaceList.at(js)->nXPanels() * m_SurfaceList.at(js)->nYPanels();
    }

    if(m_pPlane->isWing() && (!m_pWPolar || !m_pWPolar->bThinSurfaces()))
    {
        PanelArraySize *= 2;
        for (int js=0; js<m_SurfaceList.size(); js++)
        {
            if(m_SurfaceList.at(js)->isTipLeft() || m_SurfaceList.at(js)->isTipRight())
                PanelArraySize += m_SurfaceList.at(js)->nXPanels();//tip patches
        }
    }

    // add the number of body panels
    //create the body elements only if there is a body, and the analysis is not of the VLM Type
    if(m_pPlane && m_pPlane->body())
    {
        Body *pCurBody = m_pPlane->body();

        if(m_pWPolar && m_pWPolar->analysisMethod()==xfl::PANEL4METHOD && m_pWPolar->bIgnoreBodyPanels())
        {
        }
        else
        {
            if(pCurBody->m_LineType==xfl::BODYPANELTYPE)
            {
                int nx = 0;
                for(int i=0; i<pCurBody->frameCount()-1; i++) nx+=pCurBody->m_xPanels[i];
                int nh = 0;
                for(int i=0; i<pCurBody->sideLineCount()-1; i++) nh+=pCurBody->m_hPanels[i];
                PanelArraySize += nx*nh*2;
            }
            else PanelArraySize += 2 * pCurBody->nxPanels() * pCurBody->nhPanels();
        }
    }
    return PanelArraySize;
}


/**
 * Releases the memory allocated to the Panel and node arrays.
 * Sets the pointers to NULL and the matrixsize to 0.
 */
void PlaneTask::releasePanelMemory()
{
    m_Node.clear();
    m_MemNode.clear();
    m_WakeNode.clear();
    m_RefWakeNode.clear();
    m_TempWakeNode.clear();

    m_Panel.clear();
    m_MemPanel.clear();
    m_WakePanel.clear();
    m_RefWakePanel.clear();
}


/**
 * Checks if the input point is close to a wake node within the tolerances set in the Vector3d class
 * Returns the index of a node if found, else returns -1
 *@param Pt : the point to identify
 *@return the index of the node with coordinates equal to the input Pt
*/
int PlaneTask::isWakeNode(Vector3d &Pt)
{
    for (int in=0; in<m_WakeNode.size(); in++)
    {
        if(Pt.isSame(m_WakeNode.at(in))) return in;
    }
    return -1;
}


/**
 * Checks if the input point is close to a mesh node within the tolerances set in the Vector3d class
 * Returns the index of a node if found, else returns -1
 *@param Pt : the point to identify
 *@return the index of the node with coordinates equal to the input Pt
*/
int PlaneTask::isNode(Vector3d &Pt)
{
    for (int in=m_Node.size()-1; in>=0; in--)
    {
        if(Pt.isSame(m_Node.at(in))) return in;
    }
    return -1;
}


/**
 * Reserves the memory necessary to all the arrays used in a Panel analysis.
 * @return true if the memory could be allocated, false otherwise.
 */
bool PlaneTask::allocatePanelArrays(int &memsize)
{
    try
    {
        m_Node.resize(2*m_MaxPanelSize);
        m_MemNode.resize(2*m_MaxPanelSize);

        //Wake Node size
        m_NWakeColumn = 0;
        int WakeNodeSize = 0;
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pPlane->wing(iw))
            {
//                qDebug()<<"---------"<<iw<<"m_pPlane->wing(iw)->m_NStation"<<m_pPlane->wing(iw)->m_NStation;
                //calculate chords to initialize Station count
                m_pPlane->wing(iw)->computeChords();
//qDebug()<<"chords "<<m_pPlane->wing(iw)->m_NStation;
//qDebug()<<"count"<<m_pPlane->wing(iw)->NYPanels() ;
                m_NWakeColumn += m_pPlane->wing(iw)->m_NStation;
                //add 2 columns for tip and body connection

//                WakeNodeSize  += (m_pPlane->wing(iw)->m_NStation + 2) * (m_pWPolar->m_NXWakePanels + 1);
                for(int j=0; j<m_pPlane->wing(iw)->m_Surface.size(); j++)
                    WakeNodeSize += m_pPlane->wing(iw)->surface(j)->nYPanels()+1;
            }
        }
        WakeNodeSize *=  (m_pWPolar->m_NXWakePanels + 1);
        int WakePanelSize = m_NWakeColumn * m_pWPolar->m_NXWakePanels;

        m_WakeNode.resize(WakeNodeSize);
        m_RefWakeNode.resize(WakeNodeSize);
        m_TempWakeNode.resize(WakeNodeSize);

        m_WakePanel.resize(WakePanelSize);
        m_RefWakePanel.resize(WakePanelSize);

    }
    catch(std::exception &)
    {
        releasePanelMemory();
        m_MaxPanelSize = 0;

/*        Trace(e.what());
        QString strange = "Memory allocation error: the request for additional memory has been denied.\nPlease reduce the model's size.";
        Trace(strange);*/
        return false;
    }

    memsize  = int(sizeof(Vector3d)) * 8 * 2 * m_MaxPanelSize; //bytes
    memsize += int(sizeof(Panel))   * 8 * 2 * m_MaxPanelSize; //bytes

//    Trace(QString("Objects3D::   ...Allocated %1MB for the panel and node arrays").arg((double)memsize/1024./1024.));

    Panel::s_pNode = m_Node.constData();
    Panel::s_pWakeNode = m_WakeNode.constData();

    Surface::setPanelPointers(m_Panel.data(), m_Node.data());

//    QMiarex::s_pPanel = m_Panel;
//    QMiarex::s_pNode = m_Node;

    return true;
}


void PlaneTask::stitchSurfaces()
{
    //Stitch surfaces together
    int pl = 0;
    int pr = m_SurfaceList.at(0)->m_NElements;
    for (int i=0; i<m_SurfaceList.size()-1; i++)
    {
        if(!m_SurfaceList.at(i)->m_bIsTipRight)
        {
            if(m_SurfaceList.at(i)->m_bJoinRight)
                joinSurfaces(m_pWPolar, m_SurfaceList.at(i), m_SurfaceList.at(i+1), pl, pr);
        }
        pl  = pr;
        pr += m_SurfaceList.at(i+1)->m_NElements;
    }
}


/**
 * At panels on the side of the surfaces, connects the element to the next surface
 *
 * In the case where the number of chordwise panels is different between two adjacent surfaces,
 * We need to correct the ideal connection that was set in the CreateElements() method.
 * This is the case for instance for a flap.
 * The algorithm below is artisanal and not robust... ideally the connections should be set manually
 *
 * Uses VSAERO method
 *
*/
void PlaneTask::joinSurfaces(WPolar*pWPolar, Surface *pLeftSurf, Surface *pRightSurf, int pl, int pr)
{
    if(!pWPolar || pWPolar->analysisMethod()!=xfl::PANEL4METHOD) return;//panel analysis only

    //pl and pr are respectively the left surface's and the right surface's first panel index
    int ppl=0, ppr=0;
    double dist=0, x=0,y=0,z=0, mindist=0;
    int lclose=0;
    Vector3d MidNormal = pLeftSurf->m_Normal + pRightSurf->m_Normal;
    MidNormal.normalize();

    int coef = 1;
    if(pWPolar && pWPolar->analysisMethod()==xfl::PANEL4METHOD && !pWPolar->bThinSurfaces()) coef = 2;

    //left surface's right side
    ppl = pl;
    ppr = pr;
    if(pLeftSurf->m_bIsTipLeft && !pWPolar->bThinSurfaces()) ppl+= pLeftSurf->m_NXPanels;//left tip patch
    ppl += (pLeftSurf->m_NYPanels-1) * coef*pLeftSurf->m_NXPanels;
    //ppl is now set at left surface's first bottom panel of tip right strip

    //Process left bottom side first
    for (int ls=0; ls<pLeftSurf->m_NXPanels; ls++)
    {
        if(ls>=pLeftSurf->m_NXFlap) //flaps are not connected
        {
            mindist = 1.0e100;
            for (int lr=0; lr<pRightSurf->m_NXPanels; lr++)
            {
                //get distance from panel's normal plane as per NASA 4023 VSAERO fig.10
                x = m_Panel[ppr+lr].CollPt.x - m_Panel[ppl+ls].CollPt.x;
                y = m_Panel[ppr+lr].CollPt.y - m_Panel[ppl+ls].CollPt.y;
                z = m_Panel[ppr+lr].CollPt.z - m_Panel[ppl+ls].CollPt.z;
                dist = qAbs(x*m_Panel[ppl+ls].l.x + y*m_Panel[ppl+ls].l.y + z*m_Panel[ppl+ls].l.z);
                if(dist<mindist)
                {
                    lclose = lr;
                    mindist = dist ;
                }
            }
            if(lclose>=pRightSurf->m_NXFlap)
            {
                m_Panel[ppl+ls].m_iPL = ppr+lclose;

            }
            else
                m_Panel[ppl+ls].m_iPL = -1;
        }
        else
            m_Panel[ppl+ls].m_iPL = -1;//flap is not connected
    }

    //Process left top side next
    for (int ls=pLeftSurf->m_NXPanels;ls<coef*pLeftSurf->m_NXPanels; ls++)
    {
        if(ls < coef*pLeftSurf->m_NXPanels-pLeftSurf->m_NXFlap)
        {
            mindist = 1.0e100;
            for (int lr=pRightSurf->m_NXPanels; lr<coef*pRightSurf->m_NXPanels; lr++)
            {
                //get distance from panel's normal plane as per NASA 4023 VSAERO fig.10
                x = m_Panel[ppr+lr].CollPt.x - m_Panel[ppl+ls].CollPt.x;
                y = m_Panel[ppr+lr].CollPt.y - m_Panel[ppl+ls].CollPt.y;

                z = m_Panel[ppr+lr].CollPt.z - m_Panel[ppl+ls].CollPt.z;
                dist = qAbs(x*m_Panel[ppl+ls].l.x + y*m_Panel[ppl+ls].l.y + z*m_Panel[ppl+ls].l.z);
                if(dist<mindist)
                {
                    lclose = lr;
                    mindist = dist ;
                }
            }
            if(lclose< coef*pRightSurf->m_NXPanels-pRightSurf->m_NXFlap)
            {
                m_Panel[ppl+ls].m_iPR = ppr+lclose;
            }
            else
                m_Panel[ppl+ls].m_iPR = -1;
        }
        else
            m_Panel[ppl+ls].m_iPR = -1;
    }

    //Move on to right surface's left connections
    //ppr is set at right surface's first bottom panel of tip left strip
    ppl = pl;
    if(pLeftSurf->m_bIsTipLeft && !pWPolar->bThinSurfaces()) ppl+= pLeftSurf->m_NXPanels;//left tip patch
    ppl += (pLeftSurf->m_NYPanels-1) * coef*pLeftSurf->m_NXPanels;

    //Process right bottom side
    for (int lr=0;lr<pRightSurf->m_NXPanels; lr++)
    {
        if(lr>=pRightSurf->m_NXFlap)
        {
            mindist = 1.0e100;
            for (int ls=0; ls<pLeftSurf->m_NXPanels; ls++)
            {
                //get distance from panel's normal plane as per NASA 4023 VSAERO fig.10
                x = m_Panel[ppl+ls].CollPt.x - m_Panel[ppr+lr].CollPt.x;
                y = m_Panel[ppl+ls].CollPt.y - m_Panel[ppr+lr].CollPt.y;
                z = m_Panel[ppl+ls].CollPt.z - m_Panel[ppr+lr].CollPt.z;
                dist = qAbs(x*m_Panel[ppr+lr].l.x + y*m_Panel[ppr+lr].l.y + z*m_Panel[ppr+lr].l.z);
                if(dist<mindist)
                {
                    lclose = ls;
                    mindist = dist ;
                }
            }
            if(lclose>=pLeftSurf->m_NXFlap)
            {
                m_Panel[ppr+lr].m_iPR = ppl+lclose;
            }
            else
                m_Panel[ppr+lr].m_iPR = -1;
        }
        else
            m_Panel[ppr+lr].m_iPR = -1;
    }


    //Process right top side
    for (int lr=pRightSurf->m_NXPanels;lr<coef*pRightSurf->m_NXPanels; lr++)
    {
        if(lr < 2*pRightSurf->m_NXPanels-pRightSurf->m_NXFlap)
        {
            mindist = 1.0e100;
            for (int ls=pLeftSurf->m_NXPanels; ls<2*pLeftSurf->m_NXPanels; ls++)
            {
                //get distance from panel's normal plane as per NASA 4023 VSAERO fig.10
                x = m_Panel[ppl+ls].CollPt.x - m_Panel[ppr+lr].CollPt.x;
                y = m_Panel[ppl+ls].CollPt.y - m_Panel[ppr+lr].CollPt.y;
                z = m_Panel[ppl+ls].CollPt.z - m_Panel[ppr+lr].CollPt.z;
                dist = qAbs(x*m_Panel[ppr+lr].l.x + y*m_Panel[ppr+lr].l.y + z*m_Panel[ppr+lr].l.z);
                if(dist<mindist)
                {
                    lclose =  ls;
                    mindist = dist ;
                }
            }
            if(lclose < 2*pLeftSurf->m_NXPanels-pLeftSurf->m_NXFlap)
            {
                m_Panel[ppr+lr].m_iPL = ppl+lclose;
            }
            else
                m_Panel[ppr+lr].m_iPL = -1;
        }
        else
            m_Panel[ppr+lr].m_iPL = -1;
    }
}


void PlaneTask::LLTAnalyze()
{
    if(!m_ptheLLTAnalysis || !m_ptheLLTAnalysis->m_pWing || !m_ptheLLTAnalysis->m_pWPolar) return;
    //all set to launch the analysis

    m_bIsFinished   = false;

    m_ptheLLTAnalysis->setWPolar(m_pWPolar);
    m_ptheLLTAnalysis->setLLTRange(m_vMin, m_vMax, m_vInc, m_bSequence);

    m_ptheLLTAnalysis->initializeAnalysis();
    m_ptheLLTAnalysis->loop();

    m_bIsFinished = true;
}


void PlaneTask::PanelAnalyze()
{
    if(!m_pthePanelAnalysis->m_pPlane || !m_pthePanelAnalysis->m_pWPolar) return;

    m_bIsFinished   = false;

    m_pthePanelAnalysis->setRange(m_vMin, m_vMax, m_vInc, m_bSequence);

    m_pthePanelAnalysis->m_OpBeta = m_pWPolar->Beta();

    if(m_pWPolar->polarType()==xfl::FIXEDAOAPOLAR)
    {
        m_pthePanelAnalysis->m_Alpha  = m_pWPolar->Alpha();
    }
    else if(m_pWPolar->polarType()==xfl::STABILITYPOLAR)
    {
        m_pthePanelAnalysis->m_Alpha  = m_pWPolar->Alpha();
    }
    else
    {
        m_pthePanelAnalysis->m_QInf   = m_pWPolar->velocity();
    }

    m_pthePanelAnalysis->initializeAnalysis();
    m_pthePanelAnalysis->loop();

    m_bIsFinished = true;
}









