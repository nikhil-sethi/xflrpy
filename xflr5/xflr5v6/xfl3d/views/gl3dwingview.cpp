/****************************************************************************

    gl3dWingView Class
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

#include <QPainter>

#include <QOpenGLPaintDevice>

#include "gl3dwingview.h"
#include <xflobjects/editors/wingdlg.h>
#include <xfl3d/controls/w3dprefs.h>
#include <xflobjects/objects3d/wing.h>
#include <xflobjects/objects3d/surface.h>


gl3dWingView::gl3dWingView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pGL3dWingDlg = dynamic_cast<WingDlg*>(pParent);
    m_pWing = nullptr;


    m_bResetglSectionHighlight = true;
    m_bResetglWing             = true;
}


void gl3dWingView::setWing(Wing const*pWing)
{
    m_pWing = pWing;
/*    qDebug()<<m_pWing->NWingSection()<<m_pWing->surfaceCount();
    for(int is=0; is<m_pWing->NWingSection(); is++)
    {
        WingSection const *pWS = m_pWing->m_Section.at(is);
        qDebug()<<pWS->rightFoilName()<<pWS->leftFoilName()<< pWS->m_NXPanels << pWS->m_NYPanels << pWS->m_XPanelDist << pWS->m_YPanelDist <<pWS-> m_Chord;
    }*/
}


bool gl3dWingView::intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    return m_pGL3dWingDlg->intersectObject(AA, U, I);
}


void gl3dWingView::glMakeWingSectionHighlight(Wing const*pWing, int iSectionHighLight, bool bRightSide)
{
    Vector3d Point, Normal;

    int CHORDPOINTS = W3dPrefs::chordwiseRes();
    int iSection = 0;
    int jSurf = 0;
    for(int jSection=0; jSection<pWing->NWingSection(); jSection++)
    {
        if(jSection==iSectionHighLight) break;
        if(jSection<pWing->NWingSection()-1 && fabs(pWing->YPosition(jSection+1)-pWing->YPosition(jSection)) > Wing::minPanelSize())
            iSection++;
    }

    m_HighlightLineSize = CHORDPOINTS * 2;
    int bufferSize = m_HighlightLineSize *2 *3;
    QVector<float> pHighlightVertexArray(bufferSize);

    m_nHighlightLines = 0;
    int iv=0;
    if(iSection==0)
    {
        m_nHighlightLines++;
        //define the inner left side surface
        if(!pWing->isFin())  jSurf = pWing->m_Surface.size()/2 - 1;
        else                 jSurf = pWing->m_Surface.size()   - 1;
        Surface const *pSurf =  pWing->surface(jSurf);
//        if(pSurf)
        {
            for (int lx=0; lx<CHORDPOINTS; lx++)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pSurf->getSidePoint(xRel, true, xfl::TOPSURFACE, Point, Normal);

                pHighlightVertexArray[iv++] = Point.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf();
            }
            for (int lx=CHORDPOINTS-1; lx>=0; lx--)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pSurf->getSidePoint(xRel, true, xfl::BOTSURFACE, Point, Normal);
                pHighlightVertexArray[iv++] = Point.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf();
            }
        }
    }
    else
    {
        if((pWing->isSymetric() || bRightSide) && !pWing->isFin())
        {
            m_nHighlightLines++;
            jSurf = pWing->m_Surface.size()/2 + iSection -1;

            Surface const *pSurf =  pWing->surface(jSurf);
//            if(pSurf)
            {
                for (int lx=0; lx<CHORDPOINTS; lx++)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pSurf->getSidePoint(xRel, true, xfl::TOPSURFACE, Point, Normal);
                    pHighlightVertexArray[iv++] = Point.xf();
                    pHighlightVertexArray[iv++] = Point.yf();
                    pHighlightVertexArray[iv++] = Point.zf();
                }
                for (int lx=CHORDPOINTS-1; lx>=0; lx--)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pSurf->getSidePoint(xRel, true, xfl::BOTSURFACE, Point, Normal);
                    pHighlightVertexArray[iv++] = Point.xf();
                    pHighlightVertexArray[iv++] = Point.yf();
                    pHighlightVertexArray[iv++] = Point.zf();
                }
            }
        }

        if(pWing->isSymetric() || !bRightSide)
        {
            m_nHighlightLines++;
            if(!pWing->isFin()) jSurf = pWing->m_Surface.size()/2 - iSection;
            else                jSurf = pWing->m_Surface.size()   - iSection;
            Surface const *pSurf =  pWing->surface(jSurf);
//            if(pSurf)
            {
                //plot A side outline
                for (int lx=0; lx<CHORDPOINTS; lx++)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pSurf->getSidePoint(xRel, false, xfl::TOPSURFACE, Point, Normal);
                    pHighlightVertexArray[iv++] = Point.xf();
                    pHighlightVertexArray[iv++] = Point.yf();
                    pHighlightVertexArray[iv++] = Point.zf();
                }

                for (int lx=CHORDPOINTS-1; lx>=0; lx--)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pSurf->getSidePoint(xRel, false, xfl::BOTSURFACE, Point, Normal);
                    pHighlightVertexArray[iv++] = Point.xf();
                    pHighlightVertexArray[iv++] = Point.yf();
                    pHighlightVertexArray[iv++] = Point.zf();
                }
            }
        }
    }

    m_vboHighlight.destroy();
    m_vboHighlight.create();
    m_vboHighlight.bind();
    m_vboHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboHighlight.release();

}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dWingView::glMake3dObjects()
{
    if(m_bResetglSectionHighlight || m_bResetglWing)
    {
        if(m_pGL3dWingDlg->m_iSection>=0)
        {
            glMakeWingSectionHighlight(m_pWing, m_pGL3dWingDlg->m_iSection, m_pGL3dWingDlg->m_bRightSide);
            m_bResetglSectionHighlight = false;
        }
    }

    if(m_bResetglWing)
    {
        m_bResetglWing = false;
        glMakeWingSurface(m_pWing, nullptr, m_vboSurface);
        glMakeWingOutline(m_pWing, nullptr, m_vboOutline);
//        glMakeWingGeometry(0, m_pWing, nullptr);
        glMakeWingEditMesh(m_vboEditWingMesh[0], m_pWing);
    }
}


void gl3dWingView::glRenderView()
{
    if(m_pWing)
    {
        // paintWing(0, m_pWing);

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
        m_shadLine.release();

        if(m_bSurfaces)
            paintTriangles3VtxTexture(m_vboSurface, m_pWing->color(), false, true, nullptr);

        if(m_bOutline)
            paintSegments(m_vboOutline, W3dPrefs::s_OutlineStyle, false);

        if(m_bFoilNames)  paintFoilNames(m_pWing);
        if(m_bVLMPanels)  paintEditWingMesh(m_vboEditWingMesh[0]);
        if(m_bShowMasses)
            paintMasses(m_pWing->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pWing->m_PointMass);
        if(m_pGL3dWingDlg->iSection()>=0) paintSectionHighlight();
    }
}
