/****************************************************************************

    gl3dBodyView Class
    Copyright (C) 2016-2019 Andre Deperrois

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

#include "gl3dbodyview.h"
#include <misc/options/settings.h>
#include <miarex/view/w3drefsdlg.h>
#include <miarex/design/gl3dbodydlg.h>
#include <globals/mainframe.h>
#include <objects/objects3d/body.h>


gl3dBodyView::gl3dBodyView(QWidget *pParent) : gl3dView(pParent)
{
//    m_pglBodyDlg = dynamic_cast<GL3dBodyDlg*>(pParent);
    m_pBody = nullptr;
    m_bResetglFrameHighlight   = true;
    m_bResetglBody        = true;//otherwise endless repaint if no body present
}


void gl3dBodyView::glRenderView()
{
    if(m_pBody)
    {
        if(m_bVLMPanels) paintEditBodyMesh(m_pBody);
        paintBody(m_pBody);

        if(m_pBody->activeFrame()) paintSectionHighlight();
        if(m_bShowMasses) paintMasses(m_pBody->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pBody->m_PointMass);
    }
}


/**
* Overrides the contextMenuEvent method of the base class.
* Dispatches the handling to the active child application.
*/
void gl3dBodyView::contextMenuEvent (QContextMenuEvent * event)
{
    Q_UNUSED(event);
    m_bArcball = false;
    update();

    //    GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
    //    pDlg->showContextMenu(event);
}


void gl3dBodyView::on3DReset()
{
    startResetTimer(m_pBody->length());
}


void gl3dBodyView::paintGL()
{
    glMake3DObjects();

    paintGL3();
    paintOverlay();
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dBodyView::glMake3DObjects()
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
        if(m_pBody->isSplineType())         glMakeBodySplines(m_pBody);
        else if(m_pBody->isFlatPanelType()) glMakeBody3DFlatPanels(m_pBody);
        glMakeEditBodyMesh(m_pBody, Vector3d(0.0,0.0,0.0));
    }
}


void gl3dBodyView::set3DRotationCenter(QPoint point)
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


    if(m_pBody->intersectFlatPanels(AA, AA+m_transIncrement*10, I))
    {
        bIntersect = true;
        PP.set(I);
    }


    if(bIntersect)
    {
        startTranslationTimer(PP);
    }
}


void gl3dBodyView::paintOverlay()
{
    //    QOpenGLPaintDevice device(size() * devicePixelRatio());
    //    QPainter painter(&device);

    /*    EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
        painter.drawPixmap(0,0, pDlg->m_PixText);
        painter.drawPixmap(0,0, m_PixTextOverlay);
        m_PixTextOverlay.fill(Qt::transparent);
    */
}









