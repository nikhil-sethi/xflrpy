/****************************************************************************

    gl3dMiarexView Class
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

#include <QMenu>
#include <QApplication>
#include <QProgressDialog>
#include <QOpenGLPaintDevice>
#include <QContextMenuEvent>

#include "gl3dmiarexview.h"
#include <miarex/miarex.h>
#include <globals/mainframe.h>


#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <miarex/view/gl3dscales.h>
#include <xflobjects/objects3d/surface.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xfl3d/controls/w3dprefs.h>


bool gl3dMiarexView::s_bResetglGeom = true;
bool gl3dMiarexView::s_bResetglMesh = true;
bool gl3dMiarexView::s_bResetglWake = true;
bool gl3dMiarexView::s_bResetglOpp = true;
bool gl3dMiarexView::s_bResetglLift = true;
bool gl3dMiarexView::s_bResetglDrag = true;
bool gl3dMiarexView::s_bResetglDownwash = true;
bool gl3dMiarexView::s_bResetglPanelForce = true;
bool gl3dMiarexView::s_bResetglPanelCp = true;
bool gl3dMiarexView::s_bResetglStream = true;
bool gl3dMiarexView::s_bResetglLegend = true;
bool gl3dMiarexView::s_bResetglBody = true;
bool gl3dMiarexView::s_bResetglSurfVelocities = true;

double gl3dMiarexView::s_LiftScale     = 1.0;
double gl3dMiarexView::s_VelocityScale = 1.0;
double gl3dMiarexView::s_DragScale     = 1.0;


bool gl3dMiarexView::s_bAutoCpScale = true;
double gl3dMiarexView::s_LegendMin = -1.0;
double gl3dMiarexView::s_LegendMax =  1.0;


gl3dMiarexView::gl3dMiarexView(QWidget *parent) : gl3dXflView(parent)
{
    m_bStream             = false;
    m_bSurfVelocities     = false;
    m_bStreamlinesDone    = false;
    m_bSurfVelocitiesDone = false;
    m_NStreamLines = 0;
}


gl3dMiarexView::~gl3dMiarexView()
{
    m_vboPanelCp.destroy();
    m_vboPanelForces.destroy();
    m_vboSurfaceVelocities.destroy();
    m_vboLiftForce.destroy();
    m_vboMoments.destroy();
    m_vboMesh.destroy();
    m_vboLegendColor.destroy();
    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboLiftStrips[iWing].destroy();
        m_vboICd[iWing].destroy();
        m_vboVCd[iWing].destroy();
        m_vboTransitions[iWing].destroy();
        m_vboDownwash[iWing].destroy();
    }
}


void gl3dMiarexView::glRenderView()
{
    if(!isVisible()) return;
    if(s_pMainFrame->m_iApp!=xfl::MIAREX) return;
    if(s_pMiarex->m_iView!=xfl::W3DVIEW) return;

    WPolar const*pWPolar = s_pMiarex->curWPolar();
    PlaneOpp const *pPOpp = s_pMiarex->curPOpp();

    QMatrix4x4 modeMatrix;

    if(pWPolar && pWPolar->isStabilityPolar())
    {
        if(pPOpp && pPOpp->polarType()==xfl::STABILITYPOLAR)
        {
            QString strong = QString(tr("Time =")+"%1s").arg(s_pMiarex->m_ModeTime,6,'f',3);
            glRenderText(10, 15, strong, DisplayOptions::textColor());
        }

        modeMatrix.translate(float(s_pMiarex->m_ModeState[0]), float(s_pMiarex->m_ModeState[1]), float(s_pMiarex->m_ModeState[2]));
        modeMatrix.rotate(float(s_pMiarex->m_ModeState[3])*180.0f/PIf, 1.0, 0.0 ,0.0);
        modeMatrix.rotate(float(s_pMiarex->m_ModeState[4])*180.0f/PIf, 0.0, 1.0 ,0.0);
        modeMatrix.rotate(float(s_pMiarex->m_ModeState[5])*180.0f/PIf, 0.0, 0.0 ,1.0);
    }
    m_matModel = modeMatrix;

    if(pPOpp)    m_matModel.rotate(float(pPOpp->alpha()),0.0,1.0,0.0);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }

    if(!s_pMiarex->m_pCurPlane) return;

    m_matModel = modeMatrix;

    // We use the model matrix to apply alpha and beta rotations to the geometry.
    if(pPOpp)
    {
        //apply aoa rotation
        m_matModel.rotate(float(pPOpp->alpha()),0.0,1.0,0.0);

        /* CP position alredy includes the sideslip geometry, shond't be rotated by sideslip*/
        if(s_pMiarex->m_bXCP)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(s_pMiarex->m_pCurPlane->wing(iw)) paintLift(iw);
            }
        }

        // apply sideslip
        if(fabs(pPOpp->beta())>PRECISION)
            m_matModel.rotate(float(pPOpp->beta()), 0.0, 0.0, 1.0);
    }
    else
    {
        if(pWPolar && fabs(pWPolar->Beta())>0.001)
            m_matModel.rotate(float(pWPolar->Beta()), 0.0, 0.0, 1.0);
    }

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }

    if(m_bVLMPanels)
    {
        if(pWPolar && pWPolar->isLLTMethod())
        {
            // no mesh
        }
        else
        {
            if(!pWPolar)
            {
                // display something
                for(int iw=0; iw<MAXWINGS; iw++)
                {
                    if(s_pMiarex->m_pCurPlane->wing(iw))
                        paintEditWingMesh(m_vboEditWingMesh[iw]);
                }

                if(s_pMiarex->m_pCurPlane->body())
                    paintMesh(m_vboEditBodyMesh, true);
            }
            else
            {
                bool bBackground = (!s_pMiarex->m_pCurPOpp || !s_pMiarex->m_b3DCp);
                paintMesh(m_vboMesh, bBackground);
            }
        }
    }

    if(pPOpp)
    {
        if(s_pMiarex->m_b3DCp && pPOpp->analysisMethod()>=xfl::VLMMETHOD)
        {
            paintPanelCp(s_pMiarex->matSize());
        }
        if(s_pMiarex->m_bPanelForce && pPOpp->analysisMethod()>=xfl::VLMMETHOD)
        {
            paintPanelForces(s_pMiarex->matSize());
        }
    }

    //streamlines and velocities are rotated by aoa when constructed
    if(pPOpp && m_bStream && pPOpp && !pPOpp->isLLTMethod() && !s_bResetglStream)
        paintStreamLines();

    if(pPOpp && m_bSurfVelocities && !pPOpp->isLLTMethod())
        paintSurfaceVelocities(s_pMiarex->matSize());

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();

    if(m_bShowMasses) paintMasses(s_pMiarex->m_pCurPlane);


    for(int iw=0; iw<MAXWINGS; iw++)
    {
        Wing const *pWing = s_pMiarex->m_pCurPlane->wing(iw);
        if(pWing)
        {
            if(m_bSurfaces)
                paintTriangles3VtxTexture(m_vboWingSurface[iw], pWing->color(), false, true, nullptr);

            if(m_bOutline)
                paintSegments(m_vboWingOutline[iw], W3dPrefs::s_OutlineStyle, false);

            if(m_bFoilNames) paintFoilNames(pWing);
        }
    }

    Body const *pBody = s_pMiarex->m_pCurPlane->body();
    if(pBody)
    {
        if(m_bSurfaces)
        {
            paintTriangles3VtxTexture(m_vboFuseLeft,  pBody->color(), false, true, nullptr);
            paintTriangles3VtxTexture(m_vboFuseRight, pBody->color(), false, true, nullptr);
        }

        if(m_bOutline)
        {
            if(pBody->isFlatPanelType())
                paintSegments(m_vboFuseOutline, W3dPrefs::s_OutlineStyle);
            else
                paintLineStrip(m_vboFuseOutline, W3dPrefs::s_OutlineStyle);
        }
    }

    if(pPOpp)
    {
        if(s_pMiarex->m_bMoments)
        {
            paintMoments();
        }
        if(s_pMiarex->m_bDownwash)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(s_pMiarex->m_pCurPlane->wing(iw)) paintDownwash(iw);
            }
        }
        if(s_pMiarex->m_bICd || s_pMiarex->m_bVCd)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(s_pMiarex->m_pCurPlane->wing(iw)) paintDrag(iw);
            }
        }
        if(s_pMiarex->m_bXTop || s_pMiarex->m_bXBot)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(s_pMiarex->m_pCurPlane->wing(iw)) paintTransitions(iw);
            }
        }

        if (s_pMiarex->m_b3DCp && pPOpp && pPOpp->analysisMethod()>=xfl::VLMMETHOD)
        {
            //                paintCpLegendClr();
        }
        else if (s_pMiarex->m_bPanelForce && pPOpp && pPOpp->analysisMethod()>=xfl::VLMMETHOD)
        {
            //                paintCpLegendClr();
        }
    }
}


/**
*Overrides the contextMenuEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dMiarexView::contextMenuEvent(QContextMenuEvent * pEvent)
{
    QPoint ScreenPt = pEvent->globalPos();
    m_bArcball = false;
    update();

    if (s_pMiarex->m_iView==xfl::W3DVIEW)
    {
        if(s_pMiarex->m_pCurWPolar && s_pMiarex->m_pCurWPolar->polarType()==xfl::STABILITYPOLAR)
            s_pMainFrame->m_pW3DStabCtxMenu->exec(ScreenPt);
        else s_pMainFrame->m_pW3DCtxMenu->exec(ScreenPt);
    }
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dMiarexView::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    double w = double(width);
    double h = double(height);
    double s = 1.0;

    if(w>h)    m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

    if(!m_PixTextOverlay.isNull()) m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size()*devicePixelRatio());
    if(!m_PixTextOverlay.isNull()) m_PixTextOverlay.fill(Qt::transparent);

    s_pMiarex->m_bResetTextLegend = true;
}


void gl3dMiarexView::paintOverlay()
{
    gl3dView::paintOverlay();
    QOpenGLPaintDevice device(size() * devicePixelRatio());

    QPainter painter(&device);
/*
bool bExport(false);
if(bExport)
{
    qDebug()<<device.size();
    s_pMiarex->m_PixText.save(QDir::homePath()+"/miarex.png", "PNG");
    m_PixTextOverlay.save(QDir::homePath()+"/overlay.png", "PNG");
}*/
/*    if(!m_PixTextOverlay.isNull())
    {
        painter.drawPixmap(0,0, m_PixTextOverlay);
        m_PixTextOverlay.fill(Qt::transparent);
    }*/
    if(!s_pMiarex->m_PixText.isNull())
        painter.drawPixmap(0,0, s_pMiarex->m_PixText);
}


void gl3dMiarexView::glMakeCpLegendClr()
{
    QFont fnt(DisplayOptions::textFont()); //valgrind
    QFontMetrics fm(fnt);
    float fmw = float(fm.averageCharWidth());

    float fi, ZPos,dz,Right1, Right2;
    float color = 0.0;

    float w = float(rect().width());
    float h = float(rect().height());
    float XPos;

    if(w>h)
    {
        XPos  = 1.0f;
        dz    = h/w /float(MAXCPCOLORS)/2.0f;
        ZPos  = h/w/10.0f - 12.0f*dz;
    }
    else
    {
        XPos  = w/h;
        dz    = 1.f /float(MAXCPCOLORS)/2.0f;
        ZPos  = 1.f/10.f - 12.0f*dz;
    }

    Right1  = XPos - 8 * fmw/w;
    Right2  = XPos - 3 * fmw/w;

    int colorLegendSize = MAXCPCOLORS*2*6;

    int bufferSize = MAXCPCOLORS*2*6;
    QVector<float> pColorVertexArray(bufferSize);

    int iv = 0;
    for (int i=0; i<MAXCPCOLORS; i++)
    {
        fi = float(i)*dz;
        color = float(i)/float(MAXCPCOLORS-1);

        pColorVertexArray[iv++] = Right1;
        pColorVertexArray[iv++] = ZPos+2*fi;
        pColorVertexArray[iv++] = 0.0;
        pColorVertexArray[iv++] = xfl::GLGetRed(color);
        pColorVertexArray[iv++] = xfl::GLGetGreen(color);
        pColorVertexArray[iv++] = xfl::GLGetBlue(color);

        pColorVertexArray[iv++] = Right2;
        pColorVertexArray[iv++] = ZPos+2*fi;
        pColorVertexArray[iv++] = 0.0;
        pColorVertexArray[iv++] = xfl::GLGetRed(color);
        pColorVertexArray[iv++] = xfl::GLGetGreen(color);
        pColorVertexArray[iv++] = xfl::GLGetBlue(color);
    }
    Q_ASSERT(iv==bufferSize);

    m_vboLegendColor.destroy();
    m_vboLegendColor.create();
    m_vboLegendColor.bind();
    m_vboLegendColor.allocate(pColorVertexArray.data(), colorLegendSize * int(sizeof(GLfloat)));
    m_vboLegendColor.release();
}


bool gl3dMiarexView::glMakeStreamLines(Wing const *PlaneWing[MAXWINGS], Vector3d const *pNode,
                                       WPolar const *pWPolar, PlaneOpp const *pPOpp)
{

    if(!isVisible()) return false;
    if(s_pMainFrame->m_iApp!=xfl::MIAREX) return false;
    if(s_pMiarex->m_iView!=xfl::W3DVIEW) return false;
    if(!pPOpp || !pWPolar || pWPolar->isLLTMethod()) return false;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    double memcoresize = Panel::coreSize();
    Panel::setCoreSize(0.0005); //mm, just for the time needed to build the streamlines which are very sensitive to trailing vortex interference

    bool bFound(false);
    double ds(0);

    Vector3d C, D, D1, VA, VAT, VB, VBT, VT, VInf, TC, TD;
    Vector3d RefPoint(0.0,0.0,0.0);

    D1.set(987654321.0, 0.0, 0.0);

    double const *Mu    = pPOpp->m_dG;
    double const *Sigma = pPOpp->m_dSigma;

    VInf.set(pPOpp->m_QInf,0.0,0.0);

    int i=0;
    int m = 0;

    m_NStreamLines = 0;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(PlaneWing[iw]) m_NStreamLines += m_Ny[iw]+20; //in case there is a body in the middle, other wise Ny+1
    }

    int streamArraySize = m_NStreamLines * int(GL3DScales::s_NX) * 3;
    QVector<float> StreamVertexArray(streamArraySize, 0.0f);

    int p0=0;
    int iv = 0;
    for (int iWing=0; iWing<MAXWINGS; iWing++)
    {
        if(PlaneWing[iWing])
        {
            Wing const *pWing = PlaneWing[iWing];

            int nVertex = 0;
            for (int p=0; p<pWing->nPanels(); p++)
            {
                bFound = false;
                int iPanel = pWing->firstPanelIndex() + p;
                Panel const &panel = s_pMiarex->m_theTask.m_Panel.at(iPanel);

                if(GL3DScales::s_pos==0 && panel.m_bIsLeading && panel.m_Pos<=xfl::MIDSURFACE)
                {
                    C.set(pNode[panel.m_iLA]);
                    D.set(pNode[panel.m_iLB]);
                    bFound = true;
                }
                else if(GL3DScales::s_pos==1 && panel.m_bIsTrailing && panel.m_Pos<=xfl::MIDSURFACE)
                {
                    C.set(pNode[panel.m_iTA]);
                    D.set(pNode[panel.m_iTB]);
                    bFound = true;
                }
                else if(GL3DScales::s_pos==2 && panel.m_bIsLeading && panel.m_Pos<=xfl::MIDSURFACE)
                {
                    C.set(0.0, pNode[panel.m_iLA].y, 0.0);
                    D.set(0.0, pNode[panel.m_iLB].y, 0.0);
                    bFound = true;
                }

                if(bFound)
                {
                    TC = C;
                    TD = D;

                    //Tilt the geometry w.r.t. sideslip and aoa
                    TC.rotateZ(RefPoint, pPOpp->beta());
                    TD.rotateZ(RefPoint, pPOpp->beta());
                    TC.rotateY(RefPoint, pPOpp->alpha());
                    TD.rotateY(RefPoint, pPOpp->alpha());

                    TC -= C;
                    TD -= D;
                    if(GL3DScales::s_pos==1 && qAbs(GL3DScales::s_XOffset)<0.001 && qAbs(GL3DScales::s_ZOffset)<0.001)
                    {
                        //apply Kutta's condition : initial speed vector is parallel to the T.E. bisector angle
                        VA.set(pNode[panel.m_iTA] - pNode[panel.m_iLA]);
                        VA. normalize();
                        VB.set(pNode[panel.m_iTB] - pNode[panel.m_iLB]);
                        VB. normalize();
                        if(panel.m_Pos==xfl::BOTSURFACE)
                        {
                            //corresponding upper panel is the next one coming up
                            int ip = 0;
                            for (i=p; i<pWing->nPanels(); i++)
                            {
                                ip = pWing->firstPanelIndex() + i;
                                Panel const &panel_i = s_pMiarex->m_theTask.m_Panel.at(ip);
                                if(panel_i.m_Pos>xfl::MIDSURFACE && panel_i.m_bIsTrailing) break;
                            }

                            Panel const &panel_i = s_pMiarex->m_theTask.m_Panel.at(ip);

                            VAT = pNode[panel_i.m_iTA] - pNode[panel_i.m_iLA];
                            VAT.normalize();
                            VA = VA+VAT;
                            VA.normalize();//VA is parallel to the bisector angle

                            VBT = pNode[panel_i.m_iTB] - pNode[panel_i.m_iLB];
                            VBT.normalize();
                            VB = VB+VBT;
                            VB.normalize();//VB is parallel to the bisector angle
                        }

                        //Tilt the geometry w.r.t. sideslip and aoa
//                        VA.rotateZ(pPOpp->beta());
//                        VB.rotateZ(pPOpp->beta());
                        VA.rotateY(pPOpp->alpha());
                        VB.rotateY(pPOpp->alpha());
                    }

                    if(!C.isSame(D1))
                    {
                        // we plot the left trailing point only for the extreme left trailing panel
                        // and only right trailing points afterwards
                        C.x += GL3DScales::s_XOffset;
                        C.z += GL3DScales::s_ZOffset;

                        ds = GL3DScales::s_DeltaL;

                        // One very special case is where we initiate the streamlines exactly at the T.E.
                        // without offset either in X ou Z directions
                        //                            V1.Set(0.0,0.0,0.0);

                        StreamVertexArray[iv++] = C.xf()+TC.xf();
                        StreamVertexArray[iv++] = C.yf()+TC.yf();
                        StreamVertexArray[iv++] = C.zf()+TC.zf();
                        C   += VA *ds;
                        StreamVertexArray[iv++] = C.xf()+TC.xf();
                        StreamVertexArray[iv++] = C.yf()+TC.yf();
                        StreamVertexArray[iv++] = C.zf()+TC.zf();
                        ds *= GL3DScales::s_XFactor;
                        nVertex +=2;

                        for (i=2; i<GL3DScales::s_NX ;i++)
                        {
                            s_pMiarex->m_thePanelAnalysis.getSpeedVector(C, Mu, Sigma, VT);

                            VT += VInf;
                            VT.normalize();
                            C   += VT* ds;
                            StreamVertexArray[iv++] = C.xf()+TC.xf();
                            StreamVertexArray[iv++] = C.yf()+TC.yf();
                            StreamVertexArray[iv++] = C.zf()+TC.zf();
                            nVertex +=1;
                            ds *= GL3DScales::s_XFactor;
                        }
                    }

                    // right trailing point
                    D1 = D;
                    D.x += GL3DScales::s_XOffset;
                    D.z += GL3DScales::s_ZOffset;

                    ds = GL3DScales::s_DeltaL;

                    //                    V1.Set(0.0,0.0,0.0);

                    StreamVertexArray[iv++] = D.xf()+TD.xf();
                    StreamVertexArray[iv++] = D.yf()+TD.yf();
                    StreamVertexArray[iv++] = D.zf()+TD.zf();
                    D   += VB *ds;
                    StreamVertexArray[iv++] = D.xf()+TD.xf();
                    StreamVertexArray[iv++] = D.yf()+TD.yf();
                    StreamVertexArray[iv++] = D.zf()+TD.zf();
                    ds *= GL3DScales::s_XFactor;
                    nVertex +=2;

                    for (int i=2; i<GL3DScales::s_NX; i++)
                    {
                        s_pMiarex->m_theTask.m_pthePanelAnalysis->getSpeedVector(D, Mu, Sigma, VT);

                        VT += VInf;
                        VT.normalize();
                        D   += VT* ds;
                        StreamVertexArray[iv++] = D.xf()+TD.xf();
                        StreamVertexArray[iv++] = D.yf()+TD.yf();
                        StreamVertexArray[iv++] = D.zf()+TD.zf();
                        ds *= GL3DScales::s_XFactor;
                        nVertex +=1;
                    }
                }

                m++;
            }

            p0+=pWing->m_nPanels;
        }
    }


    m_NStreamLines = iv / GL3DScales::s_NX / 3;

    //restore things as they were
    Panel::setCoreSize(memcoresize);
    QApplication::restoreOverrideCursor();

    m_vboStreamLines.destroy();
    m_vboStreamLines.create();
    m_vboStreamLines.bind();
    m_vboStreamLines.allocate(StreamVertexArray.data(), streamArraySize*int(sizeof(float)));
    m_vboStreamLines.release();
    m_bStreamlinesDone = true;

    return true;
}


void gl3dMiarexView::glMakeSurfVelocities(Panel const*pPanel, WPolar const *pWPolar, PlaneOpp const *pPOpp, int nPanels)
{
    if(!isVisible()) return;
    if(s_pMainFrame->m_iApp!=xfl::MIAREX) return;
    if(s_pMiarex->m_iView!=xfl::W3DVIEW) return;
    if(!pWPolar || !pPOpp || pPOpp->isLLTMethod() || !pPanel)
        return;

    float length=0.0, sinT=0.0, cosT=0.0;

    float x1=0.0, x2=0.0, y1=0.0, y2=0.0, z1=0.0, z2=0.0, xe=0.0, ye=0.0, ze=0.0, dlx=0.0, dlz=0.0;
    Vector3d C, V, VT;
    Vector3d RefPoint(0.0,0.0,0.0);

    float factor = float(s_VelocityScale)/100.0f;

//    QProgressDialog dlg(tr("Velocities calculation"), tr("Abort"), 0, nPanels);
//    dlg.setWindowModality(Qt::WindowModal);
//    dlg.show();


    double const *Mu    = pPOpp->m_dG;
    double const *Sigma = pPOpp->m_dSigma;

    // vertices array size:
    //        nPanels x 1 arrow
    //      x3 lines per arrow
    //      x2 vertices per line
    //        x3 = 3 vertex components

    int velocityVertexSize = nPanels * 3 * 2 * 3;
    QVector<float> velocityVertexArray(velocityVertexSize, 0.0f);

    int iv=0;
    for (int p=0; p<nPanels; p++)
    {
        VT.set(pPOpp->m_QInf,0.0,0.0);

        if(pWPolar->analysisMethod()==xfl::PANEL4METHOD)
        {
            if(pPanel[p].m_Pos==xfl::MIDSURFACE) C.copy(pPanel[p].CtrlPt);
            else                            C.copy(pPanel[p].CollPt);
            s_pMiarex->m_theTask.m_pthePanelAnalysis->getSpeedVector(C, Mu, Sigma, V);

            VT += V;

            //Tilt the geometry w.r.t. sideslip and aoa
            C.rotateZ(RefPoint, pPOpp->beta());
            C.rotateY(RefPoint, pPOpp->alpha());

        }
        length = float(VT.norm())*factor;
        xe     = C.xf()+factor*VT.xf();
        ye     = C.yf()+factor*VT.yf();
        ze     = C.zf()+factor*VT.zf();
        if(length>0.0f)
        {
            cosT   = (xe-C.xf())/length;
            sinT   = (ze-C.zf())/length;
            dlx    = 0.15f*length;
            dlz    = 0.07f*length;
        }
        else
        {
            cosT   = 0.0;
            sinT   = 0.0;
            dlx    = 0.0;
            dlz    = 0.0;
        }

        x1 = xe -dlx*cosT - dlz*sinT;
        y1 = ye;
        z1 = ze -dlx*sinT + dlz*cosT;

        x2 = xe -dlx*cosT + dlz*sinT;
        y2 = ye;
        z2 = ze -dlx*sinT - dlz*cosT;

        velocityVertexArray[iv++] = C.xf();
        velocityVertexArray[iv++] = C.yf();
        velocityVertexArray[iv++] = C.zf();
        velocityVertexArray[iv++] = xe;
        velocityVertexArray[iv++] = ye;
        velocityVertexArray[iv++] = ze;

        velocityVertexArray[iv++] = xe;
        velocityVertexArray[iv++] = ye;
        velocityVertexArray[iv++] = ze;
        velocityVertexArray[iv++] = x1;
        velocityVertexArray[iv++] = y1;
        velocityVertexArray[iv++] = z1;

        velocityVertexArray[iv++] = xe;
        velocityVertexArray[iv++] = ye;
        velocityVertexArray[iv++] = ze;
        velocityVertexArray[iv++] = x2;
        velocityVertexArray[iv++] = y2;
        velocityVertexArray[iv++] = z2;
    }


/*    if(dlg.wasCanceled())
    {
        memset(velocityVertexArray.data(), 0, size_t(velocityVertexSize) * sizeof(float));
    }
    else
    {
        Q_ASSERT(iv==velocityVertexSize);
    }*/
    m_vboSurfaceVelocities.destroy();
    m_vboSurfaceVelocities.create();
    m_vboSurfaceVelocities.bind();
    m_vboSurfaceVelocities.allocate(velocityVertexArray.data(), velocityVertexSize * int(sizeof(GLfloat)));
    m_vboSurfaceVelocities.release();

    m_bSurfVelocitiesDone = true; // vbo is ready for rendering
    m_bSurfVelocities = true;
    update();
}



void gl3dMiarexView::glMakeTransitions(int iWing, Wing const *pWing, WPolar const *pWPolar, WingOpp const*pWOpp)
{
    if(!pWing || !pWPolar || !pWOpp) return;
    Vector3d Pt, N;

    int bufferSize = int(m_Ny[iWing]*6);
    QVector<float> pTransVertexArray(bufferSize);
    int iv=0;
    if(pWPolar->isLLTMethod())
    {
        for (int i=1; i<pWOpp->m_NStation; i++)
        {
            pWing->surfacePoint(pWOpp->m_XTrTop[i], pWOpp->m_SpanPos[i], xfl::TOPSURFACE, Pt, N);

            pTransVertexArray[iv++] = Pt.xf();
            pTransVertexArray[iv++] = Pt.yf();
            pTransVertexArray[iv++] = Pt.zf();
        }

    }
    else
    {
        if(!pWing->isFin())
        {
            int m = 0;
            for(int j=0; j<pWing->m_Surface.size(); j++)
            {
                for(int k=0; k<pWing->surface(j)->nYPanels(); k++)
                {
                    double yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                    pWing->surface(j)->getSurfacePoint(pWOpp->m_XTrTop[m],pWOpp->m_XTrTop[m],yrel,xfl::TOPSURFACE,Pt,N);

                    pTransVertexArray[iv++] = Pt.xf();
                    pTransVertexArray[iv++] = Pt.yf();
                    pTransVertexArray[iv++] = Pt.zf();
                    m++;
                }
            }
        }
        else
        {
            int m = 0;
            for(int j=0; j<pWing->m_Surface.size(); j++)
            {
                for(int k=0; k<pWing->surface(j)->nYPanels(); k++)
                {
                    double yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                    pWing->surface(j)->getSurfacePoint(pWOpp->m_XTrTop[m],pWOpp->m_XTrTop[m],yrel,xfl::TOPSURFACE,Pt,N);
                    pTransVertexArray[iv++] = Pt.xf();
                    pTransVertexArray[iv++] = Pt.yf();
                    pTransVertexArray[iv++] = Pt.zf();

                    m++;
                }
            }
        }
    }

    if(pWPolar->isLLTMethod())
    {
        for (int i=1; i<pWOpp->m_NStation; i++)
        {
            pWing->surfacePoint(pWOpp->m_XTrBot[i], pWOpp->m_SpanPos[i], xfl::BOTSURFACE, Pt, N);
            pTransVertexArray[iv++] = Pt.xf();
            pTransVertexArray[iv++] = Pt.yf();
            pTransVertexArray[iv++] = Pt.zf();
        }
    }
    else
    {
        if(!pWing->isFin())
        {
            int m = 0;
            for(int j=0; j<pWing->m_Surface.size(); j++)
            {
                for(int k=0; k<pWing->surface(j)->nYPanels(); k++)
                {
                    double yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                    pWing->surface(j)->getSurfacePoint(pWOpp->m_XTrBot[m],pWOpp->m_XTrBot[m],yrel,xfl::BOTSURFACE,Pt,N);
                    pTransVertexArray[iv++] = Pt.xf();
                    pTransVertexArray[iv++] = Pt.yf();
                    pTransVertexArray[iv++] = Pt.zf();
                    m++;
                }
            }
        }
        else
        {
            int m = 0;
            for(int j=0; j<pWing->m_Surface.size(); j++)
            {
                for(int k=0; k<pWing->surface(j)->nYPanels(); k++)
                {
                    double yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                    pWing->surface(j)->getSurfacePoint(pWOpp->m_XTrBot[m],pWOpp->m_XTrBot[m],yrel,xfl::BOTSURFACE,Pt,N);
                    pTransVertexArray[iv++] = Pt.xf();
                    pTransVertexArray[iv++] = Pt.yf();
                    pTransVertexArray[iv++] = Pt.zf();
                    m++;
                }
            }
        }
    }

    Q_ASSERT(iv==m_Ny[iWing]*6);


    m_vboTransitions[iWing].destroy();
    m_vboTransitions[iWing].create();
    m_vboTransitions[iWing].bind();
    m_vboTransitions[iWing].allocate(pTransVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboTransitions[iWing].release();
}



void gl3dMiarexView::paintDownwash(int iWing)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_DownwashStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_DownwashStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_DownwashStyle.m_Width);

        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_vboDownwash[iWing].bind();
        {
            glEnable(GL_DEPTH_TEST);
            glEnable (GL_LINE_STIPPLE);

            GLLineStipple(W3dPrefs::s_DownwashStyle.m_Stipple);

            glLineWidth(GLfloat(W3dPrefs::s_DownwashStyle.m_Width));

            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            glDrawArrays(GL_LINES, 0, m_Ny[iWing]*6);
        }
        m_vboDownwash[iWing].release();

        glDisable (GL_LINE_STIPPLE);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dMiarexView::glMakeLiftForce(const WPolar *pWPolar, const PlaneOpp *pPOpp)
{
    if(!pWPolar || !pPOpp) return;

    float forcez=0,forcex=0,glx=0, gly=0, glz=0;
    float sign=0;

    float force = 0.5f*float(pWPolar->density() * pWPolar->referenceArea() *pPOpp->m_QInf*pPOpp->QInf() *pPOpp->m_CL);

    force *= float(s_LiftScale/500.0);

    forcez =  force * cosf(float(pPOpp->alpha()* PI/180.0));
    forcex = -force * sinf(float(pPOpp->alpha()* PI/180.0));

    if (force>0.0f) sign = 1.0; else sign = -1.0;

    glx = pPOpp->m_CP.xf();
    gly = pPOpp->m_CP.yf();
    glz = pPOpp->m_CP.zf();

    float *liftForceVertexArray = new float[18];
    int iv=0;
    liftForceVertexArray[iv++] = glx;
    liftForceVertexArray[iv++] = gly;
    liftForceVertexArray[iv++] = glz;
    liftForceVertexArray[iv++] = glx+forcex;
    liftForceVertexArray[iv++] = gly;
    liftForceVertexArray[iv++] = glz+forcez;


    liftForceVertexArray[iv++] = glx+forcex;
    liftForceVertexArray[iv++] = gly;
    liftForceVertexArray[iv++] = glz+forcez;
    liftForceVertexArray[iv++] = glx+forcex+0.008f;
    liftForceVertexArray[iv++] = gly+0.008f;
    liftForceVertexArray[iv++] = glz+forcez-0.012f*sign;

    liftForceVertexArray[iv++] = glx+forcex;
    liftForceVertexArray[iv++] = gly;
    liftForceVertexArray[iv++] = glz+forcez;
    liftForceVertexArray[iv++] = glx+forcex-0.008f;
    liftForceVertexArray[iv++] = gly-0.008f;
    liftForceVertexArray[iv++] = glz+forcez-0.012f*sign;

    m_vboLiftForce.destroy();
    m_vboLiftForce.create();
    m_vboLiftForce.bind();
    m_vboLiftForce.allocate(liftForceVertexArray, 18*sizeof(float));
    m_vboLiftForce.release();
    delete [] liftForceVertexArray;
}


void gl3dMiarexView::glMakeMoments(Wing const *pWing, const WPolar *pWPolar, const PlaneOpp *pPOpp)
{
    //    The most common aeronautical convention defines
    //    - the roll as acting about the longitudinal axis, positive with the starboard wing down.
    //    - The yaw is about the vertical body axis, positive with the nose to starboard.
    //    - Pitch is about an axis perpendicular to the longitudinal plane of symmetry, positive nose up.
    //    -- Wikipedia flight dynamics --
    if(!pWing || !pWPolar) return;

    int i=0;

    float ampL=0.0f, ampM=0.0f, ampN=0.0f;
    float sign=0.0f;
    float angle=0.0f;//radian
    float endx, endy=0.0f, endz=0.0f, dx=0.0f, dy=0.0f, dz=0.0f, xae=0.0f, yae=0.0f, zae=0.0f;
    float factor = 10.0f;
    float radius= float(pWing->m_PlanformSpan)/4.0f;

    m_iMomentPoints = 0;

    ampL = 0.5f*float(pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceChordLength()
                      *pPOpp->m_QInf*pPOpp->m_QInf * pPOpp->m_GRm * s_LiftScale)*factor;
    ampM = 0.5f*float(pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceSpanLength()
                      *pPOpp->m_QInf*pPOpp->m_QInf * pPOpp->m_GCm * s_LiftScale)*factor;
    ampN = 0.5f*float(pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceSpanLength()
                      *pPOpp->m_QInf*pPOpp->m_QInf*(pPOpp->m_GYm) * s_LiftScale)*factor;

    if(fabsf(ampL)>0.000001f)
    {
        m_iMomentPoints += int(qAbs(ampL))  *2;
        m_iMomentPoints += 4;
    }
    if(fabs(ampM)>0.000001f)
    {
        m_iMomentPoints += int(qAbs(ampM))  *2;
        m_iMomentPoints += 4;
    }
    if(fabs(ampN)>0.000001f)
    {
        m_iMomentPoints += int(qAbs(ampN))  *2;
        m_iMomentPoints += 4;
    }

    QVector<float> momentVertexArray(m_iMomentPoints*3);
    int iv = 0;

    //ROLLING MOMENT
    if(fabs(ampL)>0.000001f)
    {
        if (ampL>0.0f) sign = -1.0f; else sign = 1.0f;
        for (i=0; i<int(qAbs(ampL)); i++)
        {
            angle = sign*float(i)*3.1416f/180.0f     / factor;
            momentVertexArray[iv++] = 0.0;
            momentVertexArray[iv++] = radius*cosf(angle);
            momentVertexArray[iv++] = radius*sinf(angle);
            angle = sign*float(i+1)*3.1416f/180.0f / factor;
            momentVertexArray[iv++] = 0.0;
            momentVertexArray[iv++] = radius*cosf(angle);
            momentVertexArray[iv++] = radius*sinf(angle);
        }

        endy = radius*cos(angle);
        endz = radius*sin(angle);

        dy = 0.03f;
        dz = 0.03f*sign;

        yae = (radius-dy)*cos(angle) +dz *sin(angle);
        zae = (radius-dy)*sin(angle) -dz *cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = endy;
        momentVertexArray[iv++] = endz;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = yae;
        momentVertexArray[iv++] = zae;

        yae = (radius+dy)*cos(angle) +dz *sin(angle);
        zae = (radius+dy)*sin(angle) -dz *cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = endy;
        momentVertexArray[iv++] = endz;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = yae;
        momentVertexArray[iv++] = zae;
    }

    //PITCHING MOMENT
    if(fabs(ampM)>0.000001f)
    {
        if (ampM>0.0f) sign = -1.0; else sign = 1.0;
        for (i=0; i<int(qAbs(ampM)); i++)
        {
            angle = sign*float(i)*3.1416f/180.0f     / factor;
            momentVertexArray[iv++] = radius*cos(angle);
            momentVertexArray[iv++] = 0.0;
            momentVertexArray[iv++] = radius*sin(angle);
            angle = sign*float(i+1)*3.1416f/180.0f / factor;
            momentVertexArray[iv++] = radius*cos(angle);
            momentVertexArray[iv++] = 0.0;
            momentVertexArray[iv++] = radius*sin(angle);
        }
        endx = radius*cos(angle);
        endz = radius*sin(angle);

        dx = 0.03f;
        dz = 0.03f*sign;

        xae = (radius-dx)*cos(angle) +dz *sin(angle);
        zae = (radius-dx)*sin(angle) -dz *cos(angle);
        momentVertexArray[iv++] = endx;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = endz;
        momentVertexArray[iv++] = xae;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = zae;

        xae = (radius+dx)*cos(angle) +dz *sin(angle);
        zae = (radius+dx)*sin(angle) -dz *cos(angle);
        momentVertexArray[iv++] = endx;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = endz;
        momentVertexArray[iv++] = xae;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = zae;
    }


    //YAWING MOMENT
    if(fabs(ampN)>0.000001f)
    {
        if (ampN>0.0f) sign = -1.0; else sign = 1.0;
        angle = 0.0;
        for (i=0; i<int(qAbs(ampN)); i++)
        {
            angle = sign*float(i)*3.1416f/180.0f     / factor;
            momentVertexArray[iv++] = -radius*cos(angle);
            momentVertexArray[iv++] = -radius*sin(angle);
            momentVertexArray[iv++] = 0.0;
            angle = sign*float(i+1)*3.1416f/180.0f / factor;
            momentVertexArray[iv++] = -radius*cos(angle);
            momentVertexArray[iv++] = -radius*sin(angle);
            momentVertexArray[iv++] = 0.0;
        }

        endx = -radius*cos(angle);
        endy = -radius*sin(angle);

        dx =   0.03f;
        dy =  -0.03f*sign;

        xae = (-radius+dx)*cos(angle) +dy *sin(angle);
        yae = (-radius+dx)*sin(angle) -dy *cos(angle);
        momentVertexArray[iv++] = endx;
        momentVertexArray[iv++] = endy;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = xae;
        momentVertexArray[iv++] = yae;
        momentVertexArray[iv++] = 0.0;

        xae = (-radius-dx)*cos(angle) +dy *sin(angle);
        yae = (-radius-dx)*sin(angle) -dy *cos(angle);
        momentVertexArray[iv++] = endx;
        momentVertexArray[iv++] = endy;
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = xae;
        momentVertexArray[iv++] = yae;
        momentVertexArray[iv++] = 0.0;
    }


    Q_ASSERT(iv==m_iMomentPoints*3);
    m_vboMoments.destroy();
    m_vboMoments.create();
    m_vboMoments.bind();
    m_vboMoments.allocate(momentVertexArray.data(), m_iMomentPoints*3*int(sizeof(float)));
    m_vboMoments.release();

}


void gl3dMiarexView::glMakeLiftStrip(int iWing, const Wing *pWing, const WPolar *pWPolar, const WingOpp *pWOpp)
{
    if(!pWing || !pWPolar || !pWOpp) return;
    int i=0,j=0,k=0;
    Vector3d C, CL, Pt, PtNormal;

    float amp=0, dih=0;
    float cosa =  cosf(float(pWOpp->m_Alpha) * PIf/180.0f);
    float sina = -sinf(float(pWOpp->m_Alpha) * PIf/180.0f);

    //LIFTLINE
    //dynamic pressure x area
    float q0 = 0.5f * float(pWPolar->density() * pWOpp->m_QInf * pWOpp->m_QInf);

    QVector<float> pLiftVertexArray(m_Ny[iWing]*9);

    int iv=0;
    if(pWPolar->isLLTMethod())
    {
        iv=0;
        for (i=1; i<pWOpp->m_NStation; i++)
        {
            double yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
            pWing->surfacePoint(pWOpp->m_XCPSpanRel[i], pWOpp->m_SpanPos[i], xfl::MIDSURFACE, Pt, PtNormal);

            dih = -float(pWing->getDihedral(yob))*PIf/180.0f;
            amp = q0*float(pWOpp->m_Cl[i]*pWing->getChord(yob)/pWOpp->m_MAChord);
            amp *= float(s_LiftScale)/1000.0f;

            pLiftVertexArray[iv++] = Pt.xf();
            pLiftVertexArray[iv++] = Pt.yf();
            pLiftVertexArray[iv++] = Pt.zf();
            pLiftVertexArray[iv++] = Pt.xf() + amp * cos(dih)*sina;
            pLiftVertexArray[iv++] = Pt.yf() + amp * sin(dih);
            pLiftVertexArray[iv++] = Pt.zf() + amp * cos(dih)*cosa;
        }

        for (i=1; i<pWOpp->m_NStation; i++)
        {
            double yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
            pWing->surfacePoint(pWOpp->m_XCPSpanRel[i], pWOpp->m_SpanPos[i], xfl::MIDSURFACE, Pt, PtNormal);

            dih = -float(pWing->getDihedral(yob)*PI/180.0);
            amp = q0*float(pWOpp->m_Cl[i]*pWing->getChord(yob)/pWOpp->m_MAChord);
            amp *= float(s_LiftScale)/1000.0f;

            pLiftVertexArray[iv++] = Pt.xf() + amp * cos(dih)*sina;
            pLiftVertexArray[iv++] = Pt.yf() + amp * sin(dih);
            pLiftVertexArray[iv++] = Pt.zf() + amp * cos(dih)*cosa;
        }
    }
    else
    {
        i = 0;
        iv=0;
        //lift lines
        for (j=0; j<pWing->m_Surface.size(); j++)
        {
            for (k=0; k< pWing->surface(j)->nYPanels(); k++)
            {
                pWing->surface(j)->getLeadingPt(k, C);
                amp = float(pWing->surface(j)->chord(k) / pWOpp->m_StripArea[i] / pWing->m_MAChord);
                amp *= float(s_LiftScale)/1000.0f;
                C.x += pWOpp->m_XCPSpanRel[i] * pWing->surface(j)->chord(k);

                pLiftVertexArray[iv++] = C.xf();
                pLiftVertexArray[iv++] = C.yf();
                pLiftVertexArray[iv++] = C.zf();

                pLiftVertexArray[iv++] = C.xf()+ pWOpp->m_F[i].xf()*amp;
                pLiftVertexArray[iv++] = C.yf() + pWOpp->m_F[i].yf()*amp;
                pLiftVertexArray[iv++] = C.zf() + pWOpp->m_F[i].zf()*amp;
                i++;
            }
        }
        //Lift strip on each surface
        i = 0;
        for (j=0; j<pWing->m_Surface.size(); j++)
        {
            /*            if(j>0 && pWing->m_Surface[j-1]->m_bJoinRight)
            {
                //then connect strip to previous surface's last point
                pLiftVertexArray[iv++] = CL.xf();
                pLiftVertexArray[iv++] = CL.yf();
                pLiftVertexArray[iv++] = CL.zf();

                k=0;
                pWing->surface(j)->getLeadingPt(k, C);
                amp = pWing->surface(j)->chord(k) / pWOpp->m_StripArea[i] / pWing->m_MAChord * QMiarex::s_LiftScale/1000.0;
                C.xf()+= pWOpp->m_XCPSpanRel[i] * pWing->surface(j)->chord(k);

                pLiftVertexArray[iv++] = C.xf()+ pWOpp->m_F[i].xf()*amp;
                pLiftVertexArray[iv++] = C.yf() + pWOpp->m_F[i].yf()*amp;
                pLiftVertexArray[iv++] = C.zf() + pWOpp->m_F[i].zf()*amp;
            }*/

            for (k=0; k< pWing->surface(j)->nYPanels(); k++)
            {
                pWing->surface(j)->getLeadingPt(k, C);
                amp = float(pWing->surface(j)->chord(k) / pWOpp->m_StripArea[i] / pWing->m_MAChord);
                amp *= float(s_LiftScale)/1000.0f;
                C.x += pWOpp->m_XCPSpanRel[i] * pWing->surface(j)->chord(k);
                CL.x = C.x + pWOpp->m_F[i].x * double(amp);
                CL.y = C.y + pWOpp->m_F[i].y * double(amp);
                CL.z = C.z + pWOpp->m_F[i].z * double(amp);

                pLiftVertexArray[iv++] = CL.xf();
                pLiftVertexArray[iv++] = CL.yf();
                pLiftVertexArray[iv++] = CL.zf();
                i++;
            }
        }
    }
    Q_ASSERT(iv==m_Ny[iWing]*9);

    m_vboLiftStrips[iWing].destroy();
    m_vboLiftStrips[iWing].create();
    m_vboLiftStrips[iWing].bind();
    m_vboLiftStrips[iWing].allocate(pLiftVertexArray.data(), m_Ny[iWing]*9 * int(sizeof(GLfloat)));
    m_vboLiftStrips[iWing].release();
}


void gl3dMiarexView::paintLift(int iWing)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_XCPStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_XCPStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_XCPStyle.m_Width);

        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

        glEnable(GL_DEPTH_TEST);
        glEnable (GL_LINE_STIPPLE);

        GLLineStipple(W3dPrefs::s_XCPStyle.m_Stipple);

        glLineWidth(GLfloat(W3dPrefs::s_XCPStyle.m_Width));

        m_vboLiftStrips[iWing].bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);
            glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
            glDrawArrays(GL_LINE_STRIP, m_Ny[iWing]*2, m_Ny[iWing]);
        }
        m_vboLiftStrips[iWing].release();

        m_vboLiftForce.bind();
        {
            glLineWidth(GLfloat(W3dPrefs::s_XCPStyle.m_Width)*2.0f);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);
            glDrawArrays(GL_LINES, 0, 6);
        }
        m_vboLiftForce.release();

        glDisable (GL_LINE_STIPPLE);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dMiarexView::paintMoments()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_MomentStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_MomentStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_MomentStyle.m_Width);
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

        m_vboMoments.bind();
        {
            glEnable(GL_DEPTH_TEST);
            glEnable (GL_LINE_STIPPLE);

            GLLineStipple(W3dPrefs::s_MomentStyle.m_Stipple);

            glLineWidth(GLfloat(W3dPrefs::s_MomentStyle.m_Width));

            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            glDrawArrays(GL_LINES, 0, m_iMomentPoints);
        }
        m_vboMoments.release();

        glDisable (GL_LINE_STIPPLE);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dMiarexView::glMakeDownwash(int iWing, const Wing *pWing, const WPolar *pWPolar, const WingOpp *pWOpp)
{
    if(!pWing || !pWPolar || !pWOpp) return;

    int i=0,j=0,k=0,p=0;
    float dih=0, yob=0;
    float y1=0, y2=0, z1=0, z2=0, xs=0, ys=0, zs=0;
    Vector3d C, Pt, PtNormal;
    float amp=0.0f;

    float sina = -sinf(float(pWOpp->m_Alpha)*PIf/180.0f);
    float cosa =  cosf(float(pWOpp->m_Alpha)*PIf/180.0f);
    float factor = float(s_VelocityScale)/5.0f;

    int bufferSize = m_Ny[iWing]*18;
    QVector<float> pDownWashVertexArray(bufferSize);
    int iv = 0;
    if(pWPolar->isLLTMethod())
    {
        for (i=1; i<pWOpp->m_NStation; i++)
        {
            yob = 2.0f*float(pWOpp->m_SpanPos[i]/pWOpp->m_Span);
            pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], xfl::MIDSURFACE, Pt, PtNormal);

            dih = -float(pWing->getDihedral(double(yob)))*PIf/180.0f;
            amp = float(pWOpp->m_QInf*sin(pWOpp->m_Ai[i]*PI/180.0));
            amp *= factor;
            pDownWashVertexArray[iv++] = Pt.xf();
            pDownWashVertexArray[iv++] = Pt.yf();
            pDownWashVertexArray[iv++] = Pt.zf();

            pDownWashVertexArray[iv++] = Pt.xf() + amp * cos(dih)* sina;
            pDownWashVertexArray[iv++] = Pt.yf() + amp * sin(dih);
            pDownWashVertexArray[iv++] = Pt.zf() + amp * cos(dih)* cosa;

            xs = Pt.xf() + amp * cos(dih) * sina;
            ys = Pt.yf() + amp * sin(dih);
            zs = Pt.zf() + amp * cos(dih) * cosa;
            y1 = ys - 0.085f*amp * sin(dih)        + 0.05f*amp * cos(dih) * cosa;
            z1 = zs - 0.085f*amp * cos(dih) * cosa - 0.05f*amp * sin(dih);
            y2 = ys - 0.085f*amp * sin(dih)        - 0.05f*amp * cos(dih) * cosa;
            z2 = zs - 0.085f*amp * cos(dih) * cosa + 0.05f*amp * sin(dih);

            pDownWashVertexArray[iv++] = xs;
            pDownWashVertexArray[iv++] = ys;
            pDownWashVertexArray[iv++] = zs;
            pDownWashVertexArray[iv++] = xs;
            pDownWashVertexArray[iv++] = y1;
            pDownWashVertexArray[iv++] = z1;

            pDownWashVertexArray[iv++] = xs;
            pDownWashVertexArray[iv++] = ys;
            pDownWashVertexArray[iv++] = zs;
            pDownWashVertexArray[iv++] = xs;
            pDownWashVertexArray[iv++] = y2;
            pDownWashVertexArray[iv++] = z2;
        }
    }
    else
    {
        p = 0;
        i = 0;
        iv = 0;
        for (j=0; j<pWing->m_Surface.size(); j++)
        {
            for (k=0; k< pWing->surface(j)->nYPanels(); k++)
            {
                //                m_pSurface[j+surf0]->GetTrailingPt(k, C);
                pWing->surface(j)->getTrailingPt(k, C);
                //                if (pWOpp->m_Vd[i].z>0) sign = 1.0; else sign = -1.0;
                pDownWashVertexArray[iv++] = C.xf();
                pDownWashVertexArray[iv++] = C.yf();
                pDownWashVertexArray[iv++] = C.zf();
                pDownWashVertexArray[iv++] = C.xf()+factor*pWOpp->m_Vd[i].zf() * sina;
                pDownWashVertexArray[iv++] = C.yf()+factor*pWOpp->m_Vd[i].yf();
                pDownWashVertexArray[iv++] = C.zf()+factor*pWOpp->m_Vd[i].zf() * cosa;

                xs = C.xf()+factor*pWOpp->m_Vd[i].zf()*sina;
                ys = C.yf()+factor*pWOpp->m_Vd[i].yf();
                zs = C.zf()+factor*pWOpp->m_Vd[i].zf()*cosa;
                y1 = ys - 0.085f*factor*pWOpp->m_Vd[i].yf()      + 0.05f*factor*pWOpp->m_Vd[i].zf()*cosa;
                z1 = zs - 0.085f*factor*pWOpp->m_Vd[i].zf()*cosa - 0.05f*factor*pWOpp->m_Vd[i].yf();
                y2 = ys - 0.085f*factor*pWOpp->m_Vd[i].yf()      - 0.05f*factor*pWOpp->m_Vd[i].zf()*cosa;
                z2 = zs - 0.085f*factor*pWOpp->m_Vd[i].zf()*cosa + 0.05f*factor*pWOpp->m_Vd[i].yf();

                pDownWashVertexArray[iv++] = xs;
                pDownWashVertexArray[iv++] = ys;
                pDownWashVertexArray[iv++] = zs;
                pDownWashVertexArray[iv++] = xs;
                pDownWashVertexArray[iv++] = y1;
                pDownWashVertexArray[iv++] = z1;

                pDownWashVertexArray[iv++] = xs;
                pDownWashVertexArray[iv++] = ys;
                pDownWashVertexArray[iv++] = zs;
                pDownWashVertexArray[iv++] = xs;
                pDownWashVertexArray[iv++] = y2;
                pDownWashVertexArray[iv++] = z2;

                i++;
            }
            p++;
        }
    }

    Q_ASSERT(iv==bufferSize);

    m_vboDownwash[iWing].destroy();
    m_vboDownwash[iWing].create();
    m_vboDownwash[iWing].bind();
    m_vboDownwash[iWing].allocate(pDownWashVertexArray.constData(), bufferSize * int(sizeof(GLfloat)));
    m_vboDownwash[iWing].release();
}


void gl3dMiarexView::glMakeDragStrip(int iWing, const Wing *pWing, const WPolar *pWPolar, const WingOpp *pWOpp, double beta)
{
    if(!pWing || !pWPolar || !pWOpp) return;
    Vector3d C, Pt, PtNormal;
    int i=0,j=0,k=0;

    float coef = 5.0;
    float amp=0, amp1=0, amp2=0, yob=0, dih=0;
    float cosa =  float(cos(pWOpp->m_Alpha * PI/180.0));
    float sina = -float(sin(pWOpp->m_Alpha * PI/180.0));
    float cosb =  float(cos(-beta*PI/180.0));
    float sinb =  float(sin(-beta*PI/180.0));

    int bufferSize = m_Ny[iWing]*9;
    QVector<float> pICdVertexArray(bufferSize);
    QVector<float> pVCdVertexArray(bufferSize);

    //DRAGLINE
    float q0 = float(0.5 * pWPolar->density() * pWPolar->referenceArea() * pWOpp->m_QInf * pWOpp->m_QInf);

    int ii=0, iv=0;
    if(pWPolar->isLLTMethod())
    {
        ii=0;
        iv=0;
        for (i=1; i<pWOpp->m_NStation; i++)
        {
            yob = float(2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span);

            pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], xfl::MIDSURFACE, Pt, PtNormal);
            dih = float(pWing->getDihedral(double(yob)))*PIf/180.0f;
            amp1 = q0*float(pWOpp->m_ICd[i]*pWing->getChord(double(yob))/pWOpp->m_MAChord*s_DragScale)/coef;
            amp2 = q0*float(pWOpp->m_PCd[i]*pWing->getChord(double(yob))/pWOpp->m_MAChord*s_DragScale)/coef;
            if(s_pMiarex->m_bICd)
            {
                pICdVertexArray[ii++] = Pt.xf();
                pICdVertexArray[ii++] = Pt.yf();
                pICdVertexArray[ii++] = Pt.zf();
                pICdVertexArray[ii++] = Pt.xf()+ amp1 * cos(dih)*cosa;
                pICdVertexArray[ii++] = Pt.yf();
                pICdVertexArray[ii++] = Pt.zf() - amp1 * cos(dih)*sina;
            }
            if(s_pMiarex->m_bVCd)
            {
                if(!s_pMiarex->m_bICd)
                {
                    pVCdVertexArray[iv++] = Pt.xf();
                    pVCdVertexArray[iv++] = Pt.yf();
                    pVCdVertexArray[iv++] = Pt.zf();
                    pVCdVertexArray[iv++] = Pt.xf()+ amp2 * cos(dih)*cosa;
                    pVCdVertexArray[iv++] = Pt.yf();
                    pVCdVertexArray[iv++] = Pt.zf() - amp2 * cos(dih)*sina;
                }
                else
                {
                    pVCdVertexArray[iv++] = Pt.xf()+ amp1 * cos(dih)*cosa;
                    pVCdVertexArray[iv++] = Pt.yf();
                    pVCdVertexArray[iv++] = Pt.zf() - amp1 * cos(dih)*sina;

                    pVCdVertexArray[iv++] = Pt.xf()+ (amp1+amp2) * cos(dih)*cosa;
                    pVCdVertexArray[iv++] = Pt.yf();
                    pVCdVertexArray[iv++] = Pt.zf() - (amp1+amp2) * cos(dih)*sina;
                }
            }
        }
        if(s_pMiarex->m_bICd)
        {
            for (i=1; i<pWOpp->m_NStation; i++)
            {
                yob = 2.0f*float(pWOpp->m_SpanPos[i]/pWOpp->m_Span);
                pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], xfl::MIDSURFACE, Pt, PtNormal);

                dih = float(pWing->getDihedral(double(yob))*PI/180.0);
                amp  = q0*float(pWOpp->m_ICd[i]*pWing->getChord(double(yob))/pWOpp->m_MAChord);
                amp *= float(s_DragScale)/coef;

                pICdVertexArray[ii++] = Pt.xf()+ amp * cos(dih)*cosa;
                pICdVertexArray[ii++] = Pt.yf();
                pICdVertexArray[ii++] = Pt.zf() - amp * cos(dih)*sina;
            }
        }
        if(s_pMiarex->m_bVCd)
        {
            for (i=1; i<pWOpp->m_NStation; i++)
            {
                yob = float(2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span);
                pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], xfl::MIDSURFACE, Pt, PtNormal);

                dih = float(pWing->getDihedral(double(yob))*PI/180.0);
                amp=0.0;
                if(s_pMiarex->m_bICd) amp += float(pWOpp->m_ICd[i]);
                amp += float(pWOpp->m_PCd[i]);
                amp *= q0*float(pWing->getChord(double(yob))/pWOpp->m_MAChord);
                amp *= float(s_DragScale)/coef;

                pVCdVertexArray[iv++] = Pt.xf()+ amp * cos(dih)*cosa;
                pVCdVertexArray[iv++] = Pt.yf();
                pVCdVertexArray[iv++] = Pt.zf() - amp * cos(dih)*sina;
            }
        }
    }
    else
    {
        //Panel type drag
        i = 0;
        ii=0;
        iv=0;
        for (j=0; j<pWing->m_Surface.size(); j++)
        {
            //All surfaces
            for (k=0; k< pWing->surface(j)->nYPanels(); k++)
            {
                pWing->surface(j)->getTrailingPt(k, C);
                amp1 = q0*float(pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord*s_DragScale)/coef;
                amp2 = q0*float(pWOpp->m_PCd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord*s_DragScale)/coef;
                if(s_pMiarex->m_bICd)
                {
                    pICdVertexArray[ii++] = C.xf();
                    pICdVertexArray[ii++] = C.yf();
                    pICdVertexArray[ii++] = C.zf();
                    pICdVertexArray[ii++] = C.xf() + amp1*cosa * cosb;
                    pICdVertexArray[ii++] = C.yf() + amp1*cosa * sinb;
                    pICdVertexArray[ii++] = C.zf() - amp1*sina;
                }
                if(s_pMiarex->m_bVCd)
                {
                    if(!s_pMiarex->m_bICd)
                    {
                        pVCdVertexArray[iv++] = C.xf();
                        pVCdVertexArray[iv++] = C.yf();
                        pVCdVertexArray[iv++] = C.zf();
                        pVCdVertexArray[iv++] = C.xf()+ amp2*cosa * cosb;
                        pVCdVertexArray[iv++] = C.yf() + amp2*cosa * sinb;
                        pVCdVertexArray[iv++] = C.zf() - amp2*sina;
                    }
                    else
                    {
                        pVCdVertexArray[iv++] = C.xf()+ amp1*cosa*cosb;
                        pVCdVertexArray[iv++] = C.yf() + amp1*cosa*sinb;
                        pVCdVertexArray[iv++] = C.zf() - amp1*sina;
                        pVCdVertexArray[iv++] = C.xf()+ (amp1+amp2)*cosa*cosb;
                        pVCdVertexArray[iv++] = C.yf() + (amp1+amp2)*cosa*sinb;
                        pVCdVertexArray[iv++] = C.zf() - (amp1+amp2)*sina;
                    }
                }

                i++;
            }
        }
        if(!pWing->isFin())
        {
            if(s_pMiarex->m_bICd)
            {
                i = 0;
                for (j=0; j<pWing->m_Surface.size(); j++)
                {
                    for (k=0; k< pWing->surface(j)->nYPanels(); k++)
                    {
                        pWing->surface(j)->getTrailingPt(k, C);
                        amp = q0*float(pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord);
                        amp *= float(s_DragScale)/coef;
                        pICdVertexArray[ii++] = C.xf()+ amp*cosa * cosb;
                        pICdVertexArray[ii++] = C.yf() + amp*cosa * sinb;
                        pICdVertexArray[ii++] = C.zf() - amp*sina;
                        i++;
                    }
                }
            }
            if(s_pMiarex->m_bVCd)
            {
                i = 0;
                for (j=0; j<pWing->m_Surface.size(); j++)
                {
                    for (k=0; k< pWing->surface(j)->nYPanels(); k++)
                    {
                        pWing->surface(j)->getTrailingPt(k, C);
                        amp=0.0;
                        if(s_pMiarex->m_bICd) amp += float(pWOpp->m_ICd[i]);
                        amp += float(pWOpp->m_PCd[i]);
                        amp *= q0*float(pWOpp->m_Chord[i]/pWing->m_MAChord);
                        amp *= float(s_DragScale)/coef;

                        pVCdVertexArray[iv++] = C.xf()+ amp*cosa*cosb;
                        pVCdVertexArray[iv++] = C.yf() + amp*cosa*sinb;
                        pVCdVertexArray[iv++] = C.zf() - amp*sina;

                        i++;
                    }
                }
            }
        }
        else
        {
            if(s_pMiarex->m_bICd)
            {
                i = 0;
                for (j=0; j<pWing->m_Surface.size(); j++)
                {
                    for (k=0; k< pWing->surface(j)->nYPanels(); k++)
                    {
                        pWing->surface(j)->getTrailingPt(k, C);
                        amp = q0*float(pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord);
                        amp *= float(s_DragScale)/coef;
                        pICdVertexArray[ii++] = C.xf()+ amp*cosa * cosb;
                        pICdVertexArray[ii++] = C.yf() + amp*cosa * sinb;
                        pICdVertexArray[ii++] = C.zf() - amp*sina;
                        i++;
                    }
                }
            }
            if(s_pMiarex->m_bVCd)
            {
                i = 0;
                for (j=0; j<pWing->m_Surface.size(); j++)
                {
                    for (k=0; k< pWing->surface(j)->nYPanels(); k++)
                    {
                        pWing->surface(j)->getTrailingPt(k, C);
                        amp=0.0;
                        if(s_pMiarex->m_bICd) amp+=float(pWOpp->m_ICd[i]);
                        amp += float(pWOpp->m_PCd[i]);
                        amp *= q0*float(pWOpp->m_Chord[i]/pWing->m_MAChord);
                        amp *= float(s_DragScale)/coef;

                        pVCdVertexArray[iv++] = C.xf()+ amp*cosa*cosb;
                        pVCdVertexArray[iv++] = C.yf() + amp*cosa*sinb;
                        pVCdVertexArray[iv++] = C.zf() - amp*sina;
                        i++;
                    }
                }
            }
        }
    }
    if(s_pMiarex->m_bICd) Q_ASSERT(ii==bufferSize);
    if(s_pMiarex->m_bVCd) Q_ASSERT(iv==bufferSize);


    m_vboICd[iWing].destroy();
    m_vboICd[iWing].create();
    m_vboICd[iWing].bind();
    m_vboICd[iWing].allocate(pICdVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboICd[iWing].release();

    m_vboVCd[iWing].destroy();
    m_vboVCd[iWing].create();
    m_vboVCd[iWing].bind();
    m_vboVCd[iWing].allocate(pVCdVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboVCd[iWing].release();
}


void gl3dMiarexView::paintDrag(int iWing)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

        glEnable(GL_DEPTH_TEST);
        glEnable (GL_LINE_STIPPLE);

        // Induced drag
        if(s_pMiarex->m_bICd)
        {
            m_vboICd[iWing].bind();
            {
                m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_IDragStyle.m_Color);
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_IDragStyle.m_Stipple));
                m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_IDragStyle.m_Width);
                GLLineStipple(W3dPrefs::s_IDragStyle.m_Stipple);

                glLineWidth(GLfloat(W3dPrefs::s_IDragStyle.m_Width));
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);


                glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
                glDrawArrays(GL_LINE_STRIP, m_Ny[iWing]*2, m_Ny[iWing]);
            }
            m_vboICd[iWing].release();
        }

        //Viscous drag
        if(s_pMiarex->m_bVCd)
        {
            m_vboVCd[iWing].bind();
            {
                m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_VDragStyle.m_Color);
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_VDragStyle.m_Stipple));
                m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_VDragStyle.m_Width);
                GLLineStipple(W3dPrefs::s_VDragStyle.m_Stipple);

                glLineWidth(GLfloat(W3dPrefs::s_VDragStyle.m_Width));

                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

                glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
                glDrawArrays(GL_LINE_STRIP, 2*m_Ny[iWing], m_Ny[iWing]);
            }
            m_vboVCd[iWing].release();
        }

        glDisable (GL_LINE_STIPPLE);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dMiarexView::paintStreamLines()
{
    if(!m_bStreamlinesDone)
    {
        return; // don't render until the vbo is ready
    }
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView);

        m_vboStreamLines.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

            glEnable(GL_DEPTH_TEST);
            glEnable (GL_LINE_STIPPLE);

            m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_StreamStyle.m_Color);
            m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_StreamStyle.m_Stipple));
            m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_StreamStyle.m_Width);

            GLLineStipple(W3dPrefs::s_StreamStyle.m_Stipple);

            glLineWidth(GLfloat(W3dPrefs::s_StreamStyle.m_Width));

            int pos=0;

            for(int il=0; il<m_NStreamLines; il++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, GL3DScales::s_NX);
                pos += GL3DScales::s_NX;
            }

            glDisable (GL_LINE_STIPPLE);
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        m_vboStreamLines.release();
    }
    m_shadLine.release();

}


void gl3dMiarexView::paintTransitions(int iWing)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

        m_vboTransitions[iWing].bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            glEnable(GL_DEPTH_TEST);
            glEnable (GL_LINE_STIPPLE);

            if(s_pMiarex->m_bXTop)
            {
                m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_TopStyle.m_Color);
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_TopStyle.m_Stipple));
                m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_TopStyle.m_Width);
                GLLineStipple(W3dPrefs::s_TopStyle.m_Stipple);

                glLineWidth(GLfloat(W3dPrefs::s_TopStyle.m_Width));
                glDrawArrays(GL_LINE_STRIP, 0, m_Ny[iWing]);
            }


            if(s_pMiarex->m_bXBot)
            {
                m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_BotStyle.m_Color);
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_BotStyle.m_Stipple));
                m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_BotStyle.m_Width);
                GLLineStipple(W3dPrefs::s_BotStyle.m_Stipple);
                glLineWidth(GLfloat(W3dPrefs::s_BotStyle.m_Width));
                glDrawArrays(GL_LINE_STRIP, m_Ny[iWing], m_Ny[iWing]);
            }
        }
        m_vboTransitions[iWing].release();

        glDisable (GL_LINE_STIPPLE);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dMiarexView::paintSurfaceVelocities(int nPanels)
{
    if(!m_bSurfVelocitiesDone) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 idMatrix;
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj * m_matView);

        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_WakeStyle.m_Color);
        m_vboSurfaceVelocities.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));


            glLineWidth(W3dPrefs::s_WakeStyle.m_Width);
            glEnable(GL_DEPTH_TEST);
            glDrawArrays(GL_LINES, 0, nPanels*3*2);

            m_shadLine.disableAttributeArray(m_locLine.m_UniColor);
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        m_vboSurfaceVelocities.release();
    }
    m_shadLine.release();
}


void gl3dMiarexView::paintPanelCp(int nPanels)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {        
        m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 0); // using attribute buffer instead
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrColor);
        m_vboPanelCp.bind();
        {
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrColor,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
            glDrawArrays(GL_TRIANGLES, 0, nPanels*2*3);
            glDisable(GL_POLYGON_OFFSET_FILL);

        }
        m_vboPanelCp.release();
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrColor);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
    }
    m_shadSurf.release();
}


void gl3dMiarexView::paintPanelForces(int nPanels)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 0); // using attribute buffer instead
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrColor);
        m_vboPanelForces.bind();
        {
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrColor,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

            glLineWidth(W3dPrefs::s_XCPStyle.m_Width);
            glEnable(GL_DEPTH_TEST);
            glDrawArrays(GL_LINES, 0, nPanels*3*2);

        }
        m_vboPanelForces.release();
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrColor);
    }
    m_shadSurf.release();
}


bool gl3dMiarexView::intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    return s_pMiarex->intersectObject(AA, U, I);
}


void gl3dMiarexView::glMakePanelForces(int nPanels, Panel const *pPanel, WPolar const *pWPolar, PlaneOpp const *pPOpp)
{
    if( !pPOpp || !pWPolar || !pPanel || !nPanels) return;

    int p=0;
    double *Cp=nullptr;
    float force=0, cosa=0, sina2=0, cosa2=0, color=0;
    float rmin=0, rmax=0, range=0;

    Quaternion Qt; // Quaternion operator to align the reference arrow to the panel's normal
    Vector3d Omega; // rotation vector to align the reference arrow to the panel's normal
    Vector3d O;
    //The vectors defining the reference arrow
    Vector3d R(0.0,0.0,1.0);
    Vector3d R1( 0.05, 0.0, -0.1);
    Vector3d R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    //define the range of values to set the colors in accordance
    rmin = 1.e10;
    rmax = -rmin;
    float coef = .0005f;

    Cp = pPOpp->m_dCp;

    for (int p=0; p<nPanels; p++)
    {
        rmax = qMax(rmax, float(Cp[p]) );
        rmin = qMin(rmin, float(Cp[p]) );
    }

    float qdyn = 0.5f*float(pWPolar->density() *pPOpp->m_QInf*pPOpp->m_QInf);
    rmin *= qdyn;
    rmax *= qdyn;
    range = rmax - rmin;

    // vertices array size:
    //        nPanels x 1 arrow
    //      x3 lines per arrow
    //      x2 vertices per line
    //        x6 = 3 vertex components + 3 color components

    int forceVertexSize = nPanels * 3 * 2 * 6;
    QVector<float> forceVertexArray(forceVertexSize);

    int iv=0;
    for (p=0; p<nPanels; p++)
    {
        force = qdyn * float(Cp[p]);
        color = (force-rmin)/range;

        //scale force for display
        force *= float(s_LiftScale) *coef;

        float r = xfl::GLGetRed(color);
        float g= xfl::GLGetGreen(color);
        float b= xfl::GLGetBlue(color);

        if(pPanel->m_Pos==xfl::MIDSURFACE) O = pPanel[p].CtrlPt;
        else                          O = pPanel[p].CollPt;

        // Rotate the reference arrow to align it with the panel normal
        if(R.isSame(P))
        {
            Qt.set(0.0, 0.0,0.0,1.0); //Null quaternion
        }
        else
        {
            cosa   = float(R.dot(pPanel[p].Normal));
            sina2  = sqrtf((1.0f - cosa)*0.5f);
            cosa2  = sqrtf((1.0f + cosa)*0.5f);

            Omega = R * pPanel[p].Normal;//crossproduct
            Omega.normalize();
            Omega *= double(sina2);
            Qt.set(double(cosa2), Omega.x, Omega.y, Omega.z);
        }

        Qt.conjugate(R,  P);
        Qt.conjugate(R1, P1);
        Qt.conjugate(R2, P2);

        // Scale the pressure vector
        P  *= double(force);
        P1 *= double(force);
        P2 *= double(force);

        // Plot
        if(pPanel[p].m_Pos==xfl::MIDSURFACE)
        {
            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            if(force>0)
            {
                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf()+P1.xf();
                forceVertexArray[iv++] = O.yf()+P.yf()+P1.yf();
                forceVertexArray[iv++] = O.zf()+P.zf()+P1.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;

                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf()+P2.xf();
                forceVertexArray[iv++] = O.yf()+P.yf()+P2.yf();
                forceVertexArray[iv++] = O.zf()+P.zf()+P2.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
            }
            else
            {
                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf()+P1.xf();
                forceVertexArray[iv++] = O.yf()+P.yf()+P1.yf();
                forceVertexArray[iv++] = O.zf()+P.zf()+P1.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;

                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf()+P2.xf();
                forceVertexArray[iv++] = O.yf()+P.yf()+P2.yf();
                forceVertexArray[iv++] = O.zf()+P.zf()+P2.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
            }
        }
        else
        {
            if(Cp[p]>0)
            {
                // compression, point towards the surface
                forceVertexArray[iv++] = O.xf();
                forceVertexArray[iv++] = O.yf();
                forceVertexArray[iv++] = O.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;

                forceVertexArray[iv++] = O.xf();
                forceVertexArray[iv++] = O.yf();
                forceVertexArray[iv++] = O.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()-P1.xf();
                forceVertexArray[iv++] = O.yf()-P1.yf();
                forceVertexArray[iv++] = O.zf()-P1.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;

                forceVertexArray[iv++] = O.xf();
                forceVertexArray[iv++] = O.yf();
                forceVertexArray[iv++] = O.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()-P2.xf();
                forceVertexArray[iv++] = O.yf()-P2.yf();
                forceVertexArray[iv++] = O.zf()-P2.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
            }
            else
            {
                // depression, point outwards from the surface
                P.set(-P.x, -P.y, -P.z);

                forceVertexArray[iv++] = O.xf();
                forceVertexArray[iv++] = O.yf();
                forceVertexArray[iv++] = O.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;

                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf()-P1.xf();
                forceVertexArray[iv++] = O.yf()+P.yf()-P1.yf();
                forceVertexArray[iv++] = O.zf()+P.zf()-P1.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;

                forceVertexArray[iv++] = O.xf()+P.xf();
                forceVertexArray[iv++] = O.yf()+P.yf();
                forceVertexArray[iv++] = O.zf()+P.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
                forceVertexArray[iv++] = O.xf()+P.xf()-P2.xf();
                forceVertexArray[iv++] = O.yf()+P.yf()-P2.yf();
                forceVertexArray[iv++] = O.zf()+P.zf()-P2.zf();
                forceVertexArray[iv++] = r;
                forceVertexArray[iv++] = g;
                forceVertexArray[iv++] = b;
            }
        }
    }
    Q_ASSERT(iv==forceVertexSize);

    m_vboPanelForces.destroy();
    m_vboPanelForces.create();
    m_vboPanelForces.bind();
    m_vboPanelForces.allocate(forceVertexArray.data(), forceVertexSize * int(sizeof(GLfloat)));
    m_vboPanelForces.release();
}


void gl3dMiarexView::glMakePanels(QOpenGLBuffer &vbo, int nPanels, int , const Vector3d *pNode, const Panel *pPanel, const PlaneOpp *pPOpp)
{
    if(!pPanel || !pNode || !nPanels) return;

    float color(0);
    float range(0);

    Vector3d TA,LA, TB, LB;

    float lmin =  10000.0;
    float lmax = -10000.0;
    // find min and max Cp for scale set
    if(pPOpp)
    {
        for (int pp=0; pp< nPanels; pp++)
        {
            lmin = std::min(lmin, float(pPOpp->m_dCp[pp]));
            lmax = std::max(lmax, float(pPOpp->m_dCp[pp]));
        }

        if(gl3dMiarexView::s_bAutoCpScale)
        {
            gl3dMiarexView::s_LegendMin = double(lmin);
            gl3dMiarexView::s_LegendMax = double(lmax);
        }
        else
        {
            lmin = float(gl3dMiarexView::s_LegendMin);
            lmax = float(gl3dMiarexView::s_LegendMax);
        }
    }
    range = lmax - lmin;

    // unfortunately we can't just use nodes and colors, because the trailing edges are merged
    // and the colors would be mixed
    // so write as many nodes as there are triangles.
    //
    // vertices array size:
    //      nPanels
    //      x2 triangles per panels
    //      x3 nodes per triangle
    //      x6 = 3 vertex components + 3 color components

    int nodeVertexSize = nPanels * 2 * 3 * 6;
    QVector<float>nodeVertexArray(nodeVertexSize);

    Q_ASSERT(nPanels==nPanels);

    int iv=0;
    for (int p=0; p<nPanels; p++)
    {
        TA.copy(pNode[pPanel[p].m_iTA]);
        TB.copy(pNode[pPanel[p].m_iTB]);
        LA.copy(pNode[pPanel[p].m_iLA]);
        LB.copy(pNode[pPanel[p].m_iLB]);
        // each quad is two triangles
        // write the first one
        nodeVertexArray[iv++] = TA.xf();
        nodeVertexArray[iv++] = TA.yf();
        nodeVertexArray[iv++] = TA.zf();
        if(pPOpp)
        {
            color = (float(pPOpp->m_dCp[p])-lmin)/range;
            nodeVertexArray[iv++] = xfl::GLGetRed(color);
            nodeVertexArray[iv++] = xfl::GLGetGreen(color);
            nodeVertexArray[iv++] = xfl::GLGetBlue(color);
        }
        else
        {
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().redF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().greenF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().blueF());
        }

        nodeVertexArray[iv++] = LA.xf();
        nodeVertexArray[iv++] = LA.yf();
        nodeVertexArray[iv++] = LA.zf();
        if(pPOpp)
        {
            color = (float(pPOpp->m_dCp[p])-lmin)/range;
            nodeVertexArray[iv++] = xfl::GLGetRed(color);
            nodeVertexArray[iv++] = xfl::GLGetGreen(color);
            nodeVertexArray[iv++] = xfl::GLGetBlue(color);
        }
        else
        {
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().redF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().greenF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().blueF());
        }

        nodeVertexArray[iv++] = LB.xf();
        nodeVertexArray[iv++] = LB.yf();
        nodeVertexArray[iv++] = LB.zf();
        if(pPOpp)
        {
            color = (float(pPOpp->m_dCp[p])-lmin)/range;
            nodeVertexArray[iv++] = xfl::GLGetRed(color);
            nodeVertexArray[iv++] = xfl::GLGetGreen(color);
            nodeVertexArray[iv++] = xfl::GLGetBlue(color);
        }
        else
        {
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().redF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().greenF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().blueF());
        }

        // write the second one
        nodeVertexArray[iv++] = LB.xf();
        nodeVertexArray[iv++] = LB.yf();
        nodeVertexArray[iv++] = LB.zf();

        if(pPOpp)
        {
            color = (float(pPOpp->m_dCp[p])-lmin)/range;
            nodeVertexArray[iv++] = xfl::GLGetRed(color);
            nodeVertexArray[iv++] = xfl::GLGetGreen(color);
            nodeVertexArray[iv++] = xfl::GLGetBlue(color);
        }
        else
        {
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().redF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().greenF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().blueF());
        }

        nodeVertexArray[iv++] = TB.xf();
        nodeVertexArray[iv++] = TB.yf();
        nodeVertexArray[iv++] = TB.zf();
        if(pPOpp)
        {
            color = (float(pPOpp->m_dCp[p])-lmin)/range;
            nodeVertexArray[iv++] = xfl::GLGetRed(color);
            nodeVertexArray[iv++] = xfl::GLGetGreen(color);
            nodeVertexArray[iv++] = xfl::GLGetBlue(color);
        }
        else
        {
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().redF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().greenF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().blueF());
         }

        nodeVertexArray[iv++] = TA.xf();
        nodeVertexArray[iv++] = TA.yf();
        nodeVertexArray[iv++] = TA.zf();
        if(pPOpp)
        {
            color = (float(pPOpp->m_dCp[p])-lmin)/range;
            nodeVertexArray[iv++] = xfl::GLGetRed(color);
            nodeVertexArray[iv++] = xfl::GLGetGreen(color);
            nodeVertexArray[iv++] = xfl::GLGetBlue(color);
        }
        else
        {
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().redF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().greenF());
            nodeVertexArray[iv++] = float(DisplayOptions::backgroundColor().blueF());
         }
    }

    Q_ASSERT(iv==nodeVertexSize);
    Q_ASSERT(iv==nPanels*2*3*6);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dMiarexView::glMake3dObjects()
{
    Plane *pCurPlane = s_pMiarex->m_pCurPlane;
    WPolar *pCurWPolar = s_pMiarex->m_pCurWPolar;
    PlaneOpp *pCurPOpp = s_pMiarex->m_pCurPOpp;

    if(!pCurPlane) return;

    PlaneTask const & theTask = s_pMiarex->m_theTask;

    Body *pCurBody = pCurPlane->body();

    if(pCurWPolar) setSpanStations(pCurPlane, pCurWPolar, pCurPOpp);

    if(s_bResetglBody && pCurBody)
    {
        Body translatedBody;
        translatedBody.duplicate(pCurBody);
        translatedBody.translate(pCurPlane->bodyPos());
        if(pCurBody->isSplineType())
        {
            glMakeFuseSplines(&translatedBody);
            glMakeFuseSplinesOutline(&translatedBody);
        }
        else if(pCurBody->isFlatPanelType())
        {
            glMakeFuseFlatPanels(&translatedBody);
            glMakeFuseFlatPanelsOutline(&translatedBody);
        }
        s_bResetglBody = false;
    }

    if(s_bResetglGeom  && pCurPlane)
    {
        Body TranslatedBody;
        if(pCurPlane->body())
        {
            TranslatedBody.duplicate(pCurPlane->body());
            TranslatedBody.translate(pCurPlane->bodyPos());
        }

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(pCurPlane->wingAt(iw))
            {
                if(pCurPlane->body())
                {
//                    glMakeWingGeometry(iw, pCurPlane->wing(iw), &translatedBody);
                    glMakeWingSurface(pCurPlane->wingAt(iw), &TranslatedBody, m_vboWingSurface[iw]);
                    glMakeWingOutline(pCurPlane->wingAt(iw), &TranslatedBody, m_vboWingOutline[iw]);
                }
                else
                {
//                    glMakeWingGeometry(iw, pCurPlane->wing(iw), nullptr);
                    glMakeWingSurface(pCurPlane->wingAt(iw), nullptr, m_vboWingSurface[iw]);
                    glMakeWingOutline(pCurPlane->wingAt(iw), nullptr, m_vboWingOutline[iw]);
                }
            }
        }
        s_bResetglGeom = false;
    }

    if(s_bResetglMesh)
    {
        if(!pCurWPolar || pCurWPolar->isLLTMethod())
        {
            //make a generic quad mesh
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                if(pCurPlane->wing(iw))
                    glMakeWingEditMesh(m_vboEditWingMesh[iw], pCurPlane->wing(iw));
                else m_vboEditWingMesh[iw].destroy();
            }
            if(pCurPlane->body())
            {
                QVector<Panel> panels;
                QVector<Vector3d> nodes;
                pCurPlane->body()->makePanels(0, pCurPlane->bodyPos(), panels, nodes);
                glMakePanels(m_vboEditBodyMesh, panels.size(), nodes.size(), nodes.constData(), panels.constData(), nullptr);
            }
        }
        else
            glMakePanels(m_vboMesh, theTask.matSize(), theTask.nNodes(), theTask.m_Node.constData(), theTask.m_Panel.constData(), nullptr);
        s_bResetglMesh = false;
    }

    if(s_bResetglPanelCp || s_bResetglOpp)
    {
        if(pCurWPolar && pCurWPolar->analysisMethod()!=xfl::LLTMETHOD)
            glMakePanels(m_vboPanelCp, theTask.matSize(), theTask.nNodes(), theTask.m_Node.constData(), theTask.m_Panel.constData(), pCurPOpp);
        s_bResetglPanelCp = false;
    }


    if((s_bResetglPanelForce || s_bResetglOpp)
            && pCurWPolar && pCurWPolar->analysisMethod()!=xfl::LLTMETHOD)
    {
        if (pCurPlane && pCurPOpp)
        {
            glMakePanelForces(theTask.matSize(), theTask.m_Panel.constData(), pCurWPolar, pCurPOpp);
        }
        s_bResetglPanelForce = false;
    }


    if((s_bResetglLift || s_bResetglOpp))
    {
        if (pCurPOpp)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                Wing *pWing = pCurPlane->wing(iw);
                if(pWing)
                {
                    WingOpp *pWOpp = pCurPOpp->m_pWOpp[iw];
                    glMakeLiftStrip( iw, pWing, pCurWPolar, pWOpp);
                    glMakeTransitions(iw, pWing, pCurWPolar, pWOpp);
                }
            }
            glMakeLiftForce(pCurWPolar, pCurPOpp);
            glMakeMoments(pCurPlane->mainWing(), pCurWPolar, pCurPOpp);
        }
        s_bResetglLift = false;
    }

    if((s_bResetglDrag || s_bResetglOpp))
    {
        if (pCurPOpp)
        {
            for(int iw=0; iw<MAXWINGS; iw++)
            {
                Wing *pWing = pCurPlane->wing(iw);
                if(pWing)
                {
                    WingOpp *pWOpp = pCurPOpp->m_pWOpp[iw];
                    glMakeDragStrip( iw, pWing, pCurWPolar, pWOpp, pCurPOpp->beta());
                }
            }
        }
        s_bResetglDrag = false;
    }

    if(pCurPOpp && (s_bResetglDownwash || s_bResetglOpp))
    {
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            Wing *pWing = pCurPlane->wing(iw);
            WingOpp *pWOpp = pCurPOpp->m_pWOpp[iw];
            if(pWing &&pWOpp)
            {
                glMakeDownwash(iw, pWing, pCurWPolar, pWOpp);
            }
        }

        s_bResetglDownwash = false;
    }

    if((s_pMiarex->m_bResetTextLegend || s_bResetglLegend || s_bResetglOpp || s_bResetglGeom))
    {
        if(pCurPOpp)
        {
            glMakeCpLegendClr();
        }
        s_pMiarex->drawTextLegend();
        s_pMiarex->m_bResetTextLegend = false;
        s_bResetglLegend = false;
    }

    if (s_bResetglStream &&m_bStream)
    {
        m_bStream = false; //Disable temporarily during calculation
        m_bStreamlinesDone = false; // don't render until the vbo is built
        //no need to recalculate if not showing
        if(pCurPlane && pCurPOpp && pCurPOpp->analysisMethod()>=xfl::VLMMETHOD)
        {
            Wing const *pWingList[MAXWINGS];
            for(int iw=0; iw<MAXWINGS;iw++) pWingList[iw]=pCurPlane->wing(iw);
            if(!glMakeStreamLines(pWingList, theTask.m_Node.constData(), pCurWPolar, pCurPOpp))
            {
                m_bStream  = false;
                s_bResetglStream = true;
                s_pMiarex->m_pchStream->blockSignals(true);
                s_pMiarex->m_pchStream->setChecked(false);
                s_pMiarex->m_pchStream->blockSignals(false);
            }
            else
            {
                m_bStream  = true;
                s_bResetglStream = false;
                update(); // make sure the streamlines are displayed
            }
        }
    }

    if((s_bResetglSurfVelocities) && m_bSurfVelocities)
    {
        s_bResetglSurfVelocities = false; // prevent double repaints if calculations is not done yet
        m_bSurfVelocitiesDone  = false; // prevent painting if vbo is not yet built

        m_bSurfVelocities = false;

        if(pCurPlane && pCurPOpp && pCurPOpp->analysisMethod()>=xfl::VLMMETHOD)
        {
            glMakeSurfVelocities(theTask.m_Panel.constData(), pCurWPolar, pCurPOpp, theTask.matSize());
        }
    }

    s_bResetglOpp = false;
}
