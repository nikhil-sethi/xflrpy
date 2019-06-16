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
#include <miarex/design/GL3dWingDlg.h>

#include <objects/objects3d/Wing.h>

gl3dWingView::gl3dWingView(QWidget *pParent) : gl3dView(pParent)
{
    m_pParent = pParent;
    m_pWing = nullptr;
}



void gl3dWingView::glRenderView()
{
    GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;

    if(m_pWing)
    {
        paintWing(0, m_pWing);
        if(m_bFoilNames)  paintFoilNames(m_pWing);
        if(m_bVLMPanels)  paintEditWingMesh(m_vboEditWingMesh[0]);
        if(m_bShowMasses)
            paintMasses(m_pWing->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pWing->m_PointMass);
        if(pDlg->iSection()>=0) paintSectionHighlight();
    }
}


void gl3dWingView::paintGL()
{
    GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
    pDlg->glMake3DObjects();

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


    GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
    if(pDlg->intersectObject(AA, m_transIncrement, I))
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










