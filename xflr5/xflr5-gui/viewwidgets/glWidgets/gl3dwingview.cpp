/****************************************************************************

    gl3dWingView Class
    Copyright (C) 2016 Andre Deperrois

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

#include "gl3dwingview.h"
#include <QOpenGLPaintDevice>
#include <miarex/design/gl3dwingdlg.h>
#include <miarex/view/w3drefsdlg.h>
#include <objects/objects3d/wing.h>
#include <objects/objects3d/surface.h>

gl3dWingView::gl3dWingView(QWidget *pParent) : gl3dView(pParent)
{
    m_pGL3dWingDlg = dynamic_cast<GL3dWingDlg*>(pParent);
    m_pWing = nullptr;
}



void gl3dWingView::glRenderView()
{
    if(m_pWing)
    {
        paintWing(0, m_pWing);
        if(m_bFoilNames)  paintFoilNames(m_pWing);
        if(m_bVLMPanels)  paintEditWingMesh(m_vboEditWingMesh[0]);
        if(m_bShowMasses)
            paintMasses(m_pWing->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pWing->m_PointMass);
        if(m_pGL3dWingDlg->iSection()>=0) paintSectionHighlight();
    }
}


void gl3dWingView::paintGL()
{
    m_pGL3dWingDlg->glMake3DObjects();

    paintGL3();
    paintOverlay();
}


void gl3dWingView::on3DReset()
{
    startResetTimer(m_pWing->planformSpan());
}


void gl3dWingView::set3DRotationCenter(QPoint point)
{
    //adjusts the new rotation center after the user has picked a point on the screen
    //finds the closest panel under the point,
    //and changes the rotation vector and viewport translation
    Vector3d I, A, B, AA, BB, PP;

    screenToViewport(point, B);
    B.z = -1.0;
    A.set(B.x, B.y, +1.0);

    viewportToWorld(A, AA);
    viewportToWorld(B, BB);

    m_transIncrement.set(BB.x-AA.x, BB.y-AA.y, BB.z-AA.z);
    m_transIncrement.normalize();

    bool bIntersect = false;

    if(m_pGL3dWingDlg->intersectObject(AA, m_transIncrement, I))
    {
        bIntersect = true;
        PP.set(I);
    }


    if(bIntersect)
    {
        startTranslationTimer(PP);
    }
}


void gl3dWingView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);

    if(!m_PixTextOverlay.isNull())
    {
        painter.drawPixmap(0,0, m_PixTextOverlay);
        m_PixTextOverlay.fill(Qt::transparent);
    }
}


void gl3dWingView::glMakeWingSectionHighlight(Wing *pWing, int iSectionHighLight, bool bRightSide)
{
    Vector3d Point, Normal;

    int CHORDPOINTS = W3dPrefsDlg::chordwiseRes();
    int iSection = 0;
    int jSurf = 0;
    for(int jSection=0; jSection<pWing->NWingSection(); jSection++)
    {
        if(jSection==iSectionHighLight) break;
        if(jSection<pWing->NWingSection()-1 &&    fabs(pWing->YPosition(jSection+1)-pWing->YPosition(jSection)) > Wing::minPanelSize())
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
        Surface const *pSurf =  pWing->m_Surface.at(jSurf);

        for (int lx=0; lx<CHORDPOINTS; lx++)
        {
            double xRel = double(lx)/double(CHORDPOINTS-1);
            pSurf->getSidePoint(xRel, true, TOPSURFACE, Point, Normal);

            pHighlightVertexArray[iv++] = Point.xf();
            pHighlightVertexArray[iv++] = Point.yf();
            pHighlightVertexArray[iv++] = Point.zf();
        }
        for (int lx=CHORDPOINTS-1; lx>=0; lx--)
        {
            double xRel = double(lx)/double(CHORDPOINTS-1);
            pSurf->getSidePoint(xRel, true, BOTSURFACE, Point, Normal);
            pHighlightVertexArray[iv++] = Point.xf();
            pHighlightVertexArray[iv++] = Point.yf();
            pHighlightVertexArray[iv++] = Point.zf();
        }
    }
    else
    {
        if((pWing->isSymetric() || bRightSide) && !pWing->isFin())
        {
            m_nHighlightLines++;
            jSurf = pWing->m_Surface.size()/2 + iSection -1;
            Surface const *pSurf =  pWing->m_Surface.at(jSurf);

            for (int lx=0; lx<CHORDPOINTS; lx++)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pSurf->getSidePoint(xRel, true, TOPSURFACE, Point, Normal);
                pHighlightVertexArray[iv++] = Point.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf();
            }
            for (int lx=CHORDPOINTS-1; lx>=0; lx--)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pSurf->getSidePoint(xRel, true, BOTSURFACE, Point, Normal);
                pHighlightVertexArray[iv++] = Point.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf();
            }
        }

        if(pWing->isSymetric() || !bRightSide)
        {
            m_nHighlightLines++;
            if(!pWing->isFin()) jSurf = pWing->m_Surface.size()/2 - iSection;
            else                jSurf = pWing->m_Surface.size()   - iSection;
            Surface const *pSurf =  pWing->m_Surface.at(jSurf);

            //plot A side outline
            for (int lx=0; lx<CHORDPOINTS; lx++)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pSurf->getSidePoint(xRel, false, TOPSURFACE, Point, Normal);
                pHighlightVertexArray[iv++] = Point.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf();
            }

            for (int lx=CHORDPOINTS-1; lx>=0; lx--)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pSurf->getSidePoint(xRel, false, BOTSURFACE, Point, Normal);
                pHighlightVertexArray[iv++] = Point.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf();
            }
        }
    }

    m_vboHighlight.destroy();
    m_vboHighlight.create();
    m_vboHighlight.bind();
    m_vboHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboHighlight.release();

}








