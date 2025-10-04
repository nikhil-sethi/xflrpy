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

#include <xfl3d/controls/w3dprefs.h>
#include <xflobjects/editors/bodydlg.h>
#include <globals/mainframe.h>
#include <xflobjects/objects3d/body.h>
#include <xflcore/displayoptions.h>

gl3dBodyView::gl3dBodyView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pBody = nullptr;
    m_bResetglFrameHighlight   = true;
    m_bResetglBody        = true;//otherwise endless repaint if no body present
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
            paintMesh(m_vboEditBodyMesh, true);

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
            glMakePanels(m_vboEditBodyMesh, panels.size(), nodes.constData(), panels.constData());
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


void gl3dBodyView::glMakePanels(QOpenGLBuffer &vbo, int nPanels, const Vector3d *pNode, const Panel *pPanel)
{
    if(!pPanel || !pNode || !nPanels) return;


    Vector3d TA,LA, TB, LB;

    QColor clr = DisplayOptions::backgroundColor();

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
        // write the first
        nodeVertexArray[iv++] = LB.xf();
        nodeVertexArray[iv++] = LB.yf();
        nodeVertexArray[iv++] = LB.zf();
        nodeVertexArray[iv++] = float(clr.redF());
        nodeVertexArray[iv++] = float(clr.greenF());
        nodeVertexArray[iv++] = float(clr.blueF());

        nodeVertexArray[iv++] = LA.xf();
        nodeVertexArray[iv++] = LA.yf();
        nodeVertexArray[iv++] = LA.zf();
        nodeVertexArray[iv++] = float(clr.redF());
        nodeVertexArray[iv++] = float(clr.greenF());
        nodeVertexArray[iv++] = float(clr.blueF());

        nodeVertexArray[iv++] = TA.xf();
        nodeVertexArray[iv++] = TA.yf();
        nodeVertexArray[iv++] = TA.zf();
        nodeVertexArray[iv++] = float(clr.redF());
        nodeVertexArray[iv++] = float(clr.greenF());
        nodeVertexArray[iv++] = float(clr.blueF());


        // write the second one
        nodeVertexArray[iv++] = TA.xf();
        nodeVertexArray[iv++] = TA.yf();
        nodeVertexArray[iv++] = TA.zf();
        nodeVertexArray[iv++] = float(clr.redF());
        nodeVertexArray[iv++] = float(clr.greenF());
        nodeVertexArray[iv++] = float(clr.blueF());

        nodeVertexArray[iv++] = TB.xf();
        nodeVertexArray[iv++] = TB.yf();
        nodeVertexArray[iv++] = TB.zf();
        nodeVertexArray[iv++] = float(clr.redF());
        nodeVertexArray[iv++] = float(clr.greenF());
        nodeVertexArray[iv++] = float(clr.blueF());

        nodeVertexArray[iv++] = LB.xf();
        nodeVertexArray[iv++] = LB.yf();
        nodeVertexArray[iv++] = LB.zf();
        nodeVertexArray[iv++] = float(clr.redF());
        nodeVertexArray[iv++] = float(clr.greenF());
        nodeVertexArray[iv++] = float(clr.blueF());
    }

    Q_ASSERT(iv==nodeVertexSize);
    Q_ASSERT(iv==nPanels*2*3*6);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}




