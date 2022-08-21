/****************************************************************************

    gl3dBodyView Class
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
#include <QOpenGLTexture>

#include <QPainter>

#include "gl3dbodyview.h"

#include <globals/mainframe.h>
#include <xfl3d/globals/w3dprefs.h>
#include <xfl3d/globals/gl_globals.h>
#include <xflcore/displayoptions.h>
#include <xflobjects/editors/bodydlg.h>
#include <xflobjects/objects3d/body.h>

gl3dBodyView::gl3dBodyView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pBody = nullptr;
    m_bResetglFrameHighlight   = true;
    m_bResetglBody        = true;
    m_bNormals = false;
}


void gl3dBodyView::glRenderView()
{
    if(m_pBody)
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

        if(m_bVLMPanels)
        {
            paintMesh(m_vboEditBodyMesh);
            paintSegments(m_vboEditBodyMeshEdges, W3dPrefs::s_VLMStyle);
        }

        if(m_bSurfaces)
        {
            paintTriangles3VtxTexture(m_vboFuseLeft,  m_pBody->color(), false, true, nullptr);
            paintTriangles3VtxTexture(m_vboFuseRight, m_pBody->color(), false, true, nullptr);
        }
        if(m_bOutline)
        {
            if(m_pBody->isFlatPanelType())
                paintSegments(m_vboFuseOutline, W3dPrefs::s_OutlineStyle);
            else
                paintLineStrip(m_vboFuseOutline, W3dPrefs::s_OutlineStyle);
        }

        if(m_bNormals)
            paintNormals(m_vboNormals);

        if(m_pBody->activeFrame()) paintSectionHighlight();
        if(m_bShowMasses)
            paintMasses(m_pBody->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pBody->m_PointMass);
    }
}


/**
* Overrides the contextMenuEvent method of the base class.
* Dispatches the handling to the active child application.
*/
void gl3dBodyView::contextMenuEvent (QContextMenuEvent *)
{
    m_bArcball = false;
    update();
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dBodyView::glMake3dObjects()
{
    if(m_bResetglFrameHighlight || m_bResetglBody)
    {
        if(m_pBody->activeFrame())
        {
            glMakeBodyFrameHighlight(m_pBody,Vector3d(0.0,0.0,0.0), m_pBody->m_iActiveFrame);
            m_bResetglFrameHighlight = false;
        }
    }

    if(m_bResetglBody)
    {
        m_bResetglBody = false;
        if(m_pBody)
        {
            if(m_pBody->isSplineType())
            {
                glMakeFuseSplines(m_pBody);
                glMakeFuseSplinesOutline(m_pBody);
            }
            else if(m_pBody->isFlatPanelType())
            {
                glMakeFuseFlatPanels(m_pBody);
                glMakeFuseFlatPanelsOutline(m_pBody);
            }
            QVector<Panel> panels;
            QVector<Vector3d> nodes;
            m_pBody->makePanels(0, Vector3d(), panels, nodes);
            glMakePanels(m_vboEditBodyMesh, nodes, panels, DisplayOptions::backgroundColor());
            glMakePanelEdges(m_vboEditBodyMeshEdges, nodes, panels);
            glMakePanelNormals(panels, 0.1f, m_vboNormals);

        }
    }
}


bool gl3dBodyView::intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I)
{
    return m_pBody->intersectFlatPanels(AA, BB, I);
}


void gl3dBodyView::on3dReset()
{
    setReferenceLength(m_pBody->length());
    gl3dXflView::on3dReset();
}




