/****************************************************************************

    gl3dPlaneView Class
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


#include <QOpenGLPaintDevice>
#include <QPainter>

#include "gl3dplaneview.h"
#include <xflobjects/objects3d/plane.h>
#include <xfl3d/controls/w3dprefs.h>

gl3dPlaneView::gl3dPlaneView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pPlane = nullptr;

    m_bResetglSectionHighlight = true;
    m_bResetglPlane            = true;
    m_bResetglBody             = true;
}


void gl3dPlaneView::glRenderView()
{
    if(m_pPlane)
    {
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

        Body const*pBody = m_pPlane->body();
        if(pBody)
        {
            paintTriangles3VtxTexture(m_vboFuseLeft,  pBody->color(), false, true, nullptr);
            paintTriangles3VtxTexture(m_vboFuseRight, pBody->color(), false, true, nullptr);

            if(pBody->isFlatPanelType())
                paintSegments(m_vboFuseOutline, W3dPrefs::s_OutlineStyle);
            else
                paintLineStrip(m_vboFuseOutline, W3dPrefs::s_OutlineStyle);
        }

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            Wing const* pWing = m_pPlane->wingAt(iw);
            if(pWing)
            {
                if(m_bSurfaces)
                    paintTriangles3VtxTexture(m_vboWingSurface[iw], pWing->color(), false, true, nullptr);

                if(m_bOutline)
                    paintSegments(m_vboWingOutline[iw], W3dPrefs::s_OutlineStyle, false);

                if(m_bFoilNames) paintFoilNames(pWing);
            }
        }
        if(m_bShowMasses) paintMasses(m_pPlane);
    }
}


bool gl3dPlaneView::intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    Wing const*pWingList[MAXWINGS] = {m_pPlane->wing(), m_pPlane->wing2(), m_pPlane->stab(), m_pPlane->fin()};

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if (pWingList[iw] && pWingList[iw]->intersectWing(AA, U, I)) return true;
    }

    if(m_pPlane->body())
    {
        if(m_pPlane->body()->intersectFlatPanels(AA, AA+U*10, I)) return true;
    }
    return false;
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dPlaneView::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);


    double w = double(width);
    double h = double(height);
    double s = 1.0;

    if(w>h) m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

    if(!m_PixTextOverlay.isNull())    m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size()*devicePixelRatio());
    if(!m_PixTextOverlay.isNull())    m_PixTextOverlay.fill(Qt::transparent);

//    m_pEditPlaneDlg->resize3DView();
}


void gl3dPlaneView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);

//    if(!m_pEditPlaneDlg->m_PixText.isNull())   painter.drawPixmap(0,0, m_pEditPlaneDlg->m_PixText);
    if(!m_PixTextOverlay.isNull())  painter.drawPixmap(0,0, m_PixTextOverlay);
    m_PixTextOverlay.fill(Qt::transparent);
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dPlaneView::glMake3dObjects()
{
    if(!m_pPlane) return;

    if(m_bResetglPlane)
    {
        Body TranslatedBody;
        if(m_pPlane->body())
        {
            TranslatedBody.duplicate(m_pPlane->body());
            TranslatedBody.translate(m_pPlane->bodyPos());
            if(m_bResetglPlane || m_bResetglBody)
            {
                if(TranslatedBody.isSplineType())
                {
                    glMakeFuseSplines(&TranslatedBody);
                    glMakeFuseSplinesOutline(&TranslatedBody);
                }
                else if(TranslatedBody.isFlatPanelType())
                {
                    glMakeFuseFlatPanels(&TranslatedBody);
                    glMakeFuseFlatPanelsOutline(&TranslatedBody);
                }
                m_bResetglBody = false;
            }
        }
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pPlane->wingAt(iw))
            {
                if(m_pPlane->body())
                {
    //                    glMakeWingGeometry(iw, pCurPlane->wing(iw), &translatedBody);
                    glMakeWingSurface(m_pPlane->wingAt(iw), &TranslatedBody, m_vboWingSurface[iw]);
                    glMakeWingOutline(m_pPlane->wingAt(iw), &TranslatedBody, m_vboWingOutline[iw]);
                }
                else
                {
    //                    glMakeWingGeometry(iw, pCurPlane->wing(iw), nullptr);
                    glMakeWingSurface(m_pPlane->wingAt(iw), nullptr, m_vboWingSurface[iw]);
                    glMakeWingOutline(m_pPlane->wingAt(iw), nullptr, m_vboWingOutline[iw]);
                }
            }
        }

        m_bResetglPlane = false;
    }
}


