/****************************************************************************

    gl3dTestView Class
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


#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>

#include "gl3dtestglview.h"
#include <xfl3d/controls/w3dprefs.h>

gl3dTestGLView::gl3dTestGLView(QWidget *pParent) : gl3dView(pParent)
{
    setWindowTitle("Test GL");
    setMouseTracking(true);
    setReferenceLength(1.5);
    reset3dScale();
    m_bInitialized = false;
}


void gl3dTestGLView::glRenderView()
{
    double theta = 2.0*PI/3.0;
    paintSphere(Vector3d(        0.0,         0.0, 0.0), 0.2f, Qt::darkCyan,   true);
    paintSphere(Vector3d(        1.0,         0.0, 0.0), 0.1f, Qt::darkGreen,  true);
    paintSphere(Vector3d( cos(theta),  sin(theta), 0.0), 0.1f, Qt::darkYellow, true);
    paintSphere(Vector3d( cos(theta), -sin(theta), 0.0), 0.1f, Qt::darkRed,    true);
//    paintCube(0.0, 0.0, 0.5, 0.25, Qt::darkRed, true);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dTestGLView::showEvent(QShowEvent *pEvent)
{
    reset3dScale();
    setFocus();
    gl3dView::showEvent(pEvent);
}

