/****************************************************************************

    gl3dTestView Class
    Copyright (C) 2019 Andre Deperrois

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

#include "gl3dtestview.h"


gl3dTestView::gl3dTestView(QWidget *pParent) : gl3dView (pParent)
{
    m_bInitialized = false;
    m_pContext = nullptr;

//    qDebug()<<"gl3dTestView set:"<<format().profile();
//    qDebug()<<"deprecated functs:"<<format().testOption(QSurfaceFormat::DeprecatedFunctions);
}



void gl3dTestView::glRenderView()
{
//    QOpenGLFunctions *f = context()->functions();
//    QOpenGLFunctions *g = QOpenGLContext::currentContext()->functions();

    paintSphere(Vector3d( 0.0, 0.0, 0.0), 0.2, QColor(205, 55, 35), true);
    paintSphere(Vector3d( 0.3, 0.3, 0.0), 0.1, QColor(155, 75, 15), true);
    paintSphere(Vector3d( 0.3,-0.3, 0.0), 0.1, QColor(105, 15, 35), true);
    paintSphere(Vector3d(-0.3,-0.3, 0.0), 0.1, QColor(155, 35, 75), true);
    paintSphere(Vector3d(-0.3, 0.3, 0.0), 0.1, QColor(135, 75, 75), true);

    if (!m_bInitialized) {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dTestView::on3DReset()
{
    startResetTimer(1.0);
}


void gl3dTestView::set3DRotationCenter(QPoint )
{
}
