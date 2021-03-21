/****************************************************************************

    gl3dView Class
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

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QMouseEvent>

#include "gl3dview.h"
#include <analysis3d/plane_analysis/lltanalysis.h>
#include <globals/globals.h>

#include <globals/mainframe.h>
#include <miarex/miarex.h>
#include <miarex/design/editbodydlg.h>
#include <miarex/design/editplanedlg.h>
#include <miarex/design/gl3dbodydlg.h>
#include <miarex/design/gl3dwingdlg.h>
#include <miarex/objects3d.h>
#include <miarex/view/gl3dscales.h>
#include <miarex/view/gllightdlg.h>
#include <miarex/view/w3drefsdlg.h>
#include <misc/options/units.h>
#include <misc/options/settings.h>
#include <objects/objects3d/body.h>
#include <objects/objects3d/plane.h>
#include <objects/objects3d/pointmass.h>
#include <objects/objects3d/surface.h>
#include <objects/objects3d/wpolar.h>
#include <objects/objects3d/wing.h>
#include <objects/objects3d/vector3d.h>

Miarex *gl3dView::s_pMiarex;
MainFrame *gl3dView::s_pMainFrame;

GLLightDlg *gl3dView::s_pglLightDlg = nullptr;


gl3dView::gl3dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setAutoFillBackground(false);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    m_pTransitionTimer = nullptr;
    memset(ab_new, 0, 16*sizeof(float));
    memset(ab_old, 0, 16*sizeof(float));
    m_iTransitionInc = 0;

    m_bArcball = m_bCrossPoint = false;

    m_bUse120StyleShaders = true;

    m_SphereIndicesArray = nullptr;
    m_WingMeshIndicesArray = nullptr;

    m_pLeftBodyTexture = m_pRightBodyTexture= nullptr;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        m_pWingTopLeftTexture[iw] = m_pWingTopRightTexture[iw] = nullptr;
        m_pWingBotLeftTexture[iw] = m_pWingBotRightTexture[iw] = nullptr;
    }

    m_bOutline    = true;
    m_bSurfaces   = true;
    m_bVLMPanels  = false;
    m_bAxes       = true;
    m_bShowMasses = false;
    m_bFoilNames  = false;

    m_iBodyElems = 0;
    for(int iw=0; iw<MAXWINGS; iw++) m_iWingElems[iw]=0;
    m_iWingMeshElems = 0;


    m_nHighlightLines = m_HighlightLineSize = 0;

    m_glViewportTrans.x  = 0.0;
    m_glViewportTrans.y  = 0.0;
    m_glViewportTrans.z  = 0.0;

    m_glScaled = m_glScaledRef = 1.0;
    m_glScaleIncrement = 0.0;
    m_ClipPlanePos  = 500.0;

    m_bTrans = false;

    m_LastPoint.setX(0);
    m_LastPoint.setY(0);

    m_PixTextOverlay = QPixmap(107, 97);
    m_PixTextOverlay.fill(Qt::transparent);

    memset(m_Ny, 0, sizeof(m_Ny));
    memset(MatIn,  0, 16*sizeof(double));
    memset(MatOut, 0, 16*sizeof(double));
}


void gl3dView::printFormat(const QSurfaceFormat &format)
{
    Trace(QString("OpenGL version: %1.%2").arg(format.majorVersion()).arg(format.minorVersion()));

    switch (format.profile()) {
        case QSurfaceFormat::NoProfile:
            Trace("No Profile");
            break;
        case QSurfaceFormat::CoreProfile:
            Trace("Core Profile");
            break;
        case QSurfaceFormat::CompatibilityProfile:
            Trace("Compatibility Profile");
            break;
    }
    switch (format.renderableType())
    {
        case QSurfaceFormat::DefaultRenderableType:
            Trace("DefaultRenderableType: The default, unspecified rendering method");
            break;
        case QSurfaceFormat::OpenGL:
            Trace("OpenGL: Desktop OpenGL rendering");
            break;
        case QSurfaceFormat::OpenGLES:
            Trace("OpenGLES: OpenGL ES 2.0 rendering");
            break;
        case QSurfaceFormat::OpenVG:
            Trace("OpenVG: Open Vector Graphics rendering");
            break;
    }
}


gl3dView::~gl3dView()
{
    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboEditWingMesh[iWing].destroy();
    }

    if(m_SphereIndicesArray)   delete[] m_SphereIndicesArray;
    if(m_WingMeshIndicesArray) delete[] m_WingMeshIndicesArray;

    // release all OpenGL resources.
    makeCurrent();
    m_vboArcBall.destroy();
    m_vboArcPoint.destroy();
    m_vboSphere.destroy();
    m_vboBody.destroy();
    m_vboEditBodyMesh.destroy();

    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboWingSurface[iWing].destroy();
        m_vboWingOutline[iWing].destroy();
    }

    if(m_pLeftBodyTexture)     delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture)    delete m_pRightBodyTexture;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingBotLeftTexture[iw])  delete m_pWingBotLeftTexture[iw];
        if(m_pWingTopLeftTexture[iw])  delete m_pWingTopLeftTexture[iw];
        if(m_pWingBotRightTexture[iw]) delete m_pWingBotRightTexture[iw];
        if(m_pWingTopRightTexture[iw]) delete m_pWingTopRightTexture[iw];
        m_pWingBotLeftTexture[iw] = nullptr;
        m_pWingTopLeftTexture[iw] = nullptr;
        m_pWingBotRightTexture[iw] = nullptr;
        m_pWingTopRightTexture[iw] = nullptr;
    }

    doneCurrent();
}


QSize gl3dView::sizeHint() const
{
    return QSize(640, 480);
}


QSize gl3dView::minimumSizeHint() const
{
    return QSize(250, 200);
}


/**
* The user has modified the position of the clip plane in the 3D view
*@param pos the new z position in viewport coordinates of the clipping plane
*/
void gl3dView::onClipPlane(int pos)
{
    float coef = 4.0;
    float planepos =  pos/100.0f;
    m_ClipPlanePos = 5.0f*sinh(planepos*coef)/sinh(coef);
    update();
}


void gl3dView::on3DIso()
{
    Quaternion qti;

    memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));

    double yaw = -PI;
    double pitch = 0.0;
    double roll = -2.0*PI/3.0;
    m_ArcBall.quat(roll, pitch, yaw, qti);

    Quaternion qtyaw(-30.0, Vector3d(0.0,0.0,1.0));
    m_ArcBall.setQuat(qti*qtyaw);

    memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

    startRotationTimer();
    emit(viewModified());
}


void gl3dView::on3DFlip()
{
    memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));

    Quaternion qtflip(180.0, Vector3d(0.0,1.0,0.0));
    float ab_flip[16];
    memset(ab_flip, 0, 16*sizeof(float));
    m_ArcBall.quatToMatrix(ab_flip, qtflip);
    m_ArcBall.quatNext(ab_new, m_ArcBall.ab_quat, ab_flip);
    memcpy(m_ArcBall.ab_quat, ab_new, 16*sizeof(float));

    startRotationTimer();
    emit(viewModified());
}


void gl3dView::on3DTop()
{
    memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));
    m_ArcBall.setQuat(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
    memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

    startRotationTimer();
    emit(viewModified());
}


void gl3dView::on3DLeft()
{
    memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));
    m_ArcBall.setQuat(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);// rotate by 90° around x
    memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

    startRotationTimer();
    emit(viewModified());

}


void gl3dView::on3DFront()
{
    memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));
    Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
    Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

    m_ArcBall.setQuat(Qt1 * Qt2);
    memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

    startRotationTimer();
    emit(viewModified());

}


void gl3dView::onSurfaces(bool bChecked)
{
    m_bSurfaces = bChecked;
    update();
}


void gl3dView::onOutline(bool bChecked)
{
    m_bOutline = bChecked;
    update();
}


void gl3dView::onPanels(bool bChecked)
{
    m_bVLMPanels = bChecked;
    update();
}


void gl3dView::onAxes(bool bChecked)
{
    m_bAxes = bChecked;
    update();
}


void gl3dView::onFoilNames(bool bChecked)
{
    m_bFoilNames = bChecked;
    update();
}


void gl3dView::onShowMasses(bool bChecked)
{
    m_bShowMasses = bChecked;
    update();
}


void gl3dView::mousePressEvent(QMouseEvent *event)
{
    QPoint point(event->pos().x(), event->pos().y());

    bool bCtrl = false;
    if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

    //    setFocus();

    if (event->buttons() & Qt::MidButton)
    {
        m_bArcball = true;
        Vector3d real;
        QPoint pt(event->pos().x(), event->pos().y());
        screenToViewport(pt, real);
        m_ArcBall.start(real.x, real.y);
        m_bCrossPoint = true;

        reset3DRotationCenter();
        update();
    }
    else if (event->buttons() & Qt::LeftButton)
    {
        Vector3d real;
        QPoint pt(point.x(), point.y());
        screenToViewport(pt, real);
        m_ArcBall.start(real.x, real.y);
        m_bCrossPoint = true;
        reset3DRotationCenter();
        if (!bCtrl)
        {
            m_bTrans = true;
            setCursor(Qt::ClosedHandCursor);
        }
        else
        {
            m_bTrans=false;
            m_bArcball = true;
        }
        update();
    }

    m_LastPoint = point;
}


QPoint gl3dView::worldToScreen(Vector3d v)
{
    QVector4D v4(v.xf(), v.yf(), v.zf(), 1.0);
    QVector4D vS = m_pvmMatrix * v4;
    return QPoint(int((vS.x()+1.0f)*width()/2), int((1.0f-vS.y())*height()/2));
}


QPoint gl3dView::worldToScreen(QVector4D v4)
{
    QVector4D vS = m_pvmMatrix * v4;
    return QPoint(int((vS.x()+1.0f)*width()/2), int((1.0f-vS.y())*height()/2));
}



/**
*Overrides the mouseDoubleClickEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::mouseDoubleClickEvent(QMouseEvent *event)
{
    set3DRotationCenter(event->pos());
}


void gl3dView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point(event->pos().x(), event->pos().y());
    Vector3d Real;

    QPoint Delta(point.x()-m_LastPoint.x(), point.y()-m_LastPoint.y());
    screenToViewport(point, Real);

    //    if(!hasFocus()) setFocus();

    bool bCtrl = false;

    if (event->modifiers() & Qt::ControlModifier) bCtrl =true;
    if (event->buttons()   & Qt::LeftButton)
    {
        if(bCtrl)
        {
            //rotate
            m_ArcBall.move(Real.x, Real.y);
            update();
        }
        else if(m_bTrans)
        {
            //translate

            int side = qMax(geometry().width(), geometry().height());

            m_glViewportTrans.x += Delta.x()*2.0/m_glScaled/side;
            m_glViewportTrans.y += Delta.y()*2.0/m_glScaled/side;

            m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
            m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
            m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;

            update();

        }
    }
    else if (event->buttons() & Qt::MidButton)
    {
        m_ArcBall.move(Real.x, Real.y);
        update();
    }
    else if(event->modifiers().testFlag(Qt::AltModifier))
    {
        double zoomFactor=1.0;

        if(point.y()-m_LastPoint.y()<0) zoomFactor = 1./1.025;
        else                            zoomFactor = 1.025;

        m_glScaled *= zoomFactor;
        update();
    }
    m_LastPoint = point;
}


/**
*Overrides the wheelEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::wheelEvent(QWheelEvent *event)
{
    double zoomFactor=1.0;
    if(event->delta()>0)
    {
        if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
        else                          zoomFactor = 1.06;
    }
    else
    {
        if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
        else                          zoomFactor = 1./1.06;
    }
    m_glScaled *= zoomFactor;
    update();
}


void gl3dView::mouseReleaseEvent(QMouseEvent * event )
{
    setCursor(Qt::CrossCursor);

    m_bTrans      = false;
    m_bDragPoint  = false;
    m_bArcball    = false;
    m_bCrossPoint = false;

    //    We need to re-calculate the translation vector
    int i,j;
    for(i=0; i<4; i++)
        for(j=0; j<4; j++)
            MatIn[i][j] = double(m_ArcBall.ab_quat[i*4+j]);

    glInverseMatrix();
    m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
    m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
    m_glViewportTrans.z =  (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);

    update();
    event->accept();
}


/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {

        case Qt::Key_Control:
        {
            m_bArcball = true;
            update();
            break;
        }
        case Qt::Key_R:
        {
            on3DReset();
            event->accept();
            break;
        }
        case Qt::Key_X:
        {
            break;
        }
        default:
            event->ignore();
    }
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Control:
        {
            m_bArcball = false;
            update();
            break;
        }

        default:
            event->ignore();
    }
}


void gl3dView::reset3DRotationCenter()
{
    //adjust the new rotation center after a translation or a rotation

    int i,j;

    for(i=0; i<4; i++)
        for(j=0; j<4; j++)
            MatOut[i][j] =  double(m_ArcBall.ab_quat[i*4+j]);

    m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
    m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
    m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;
}


void gl3dView::glInverseMatrix()
{
    //Step 1. Transpose the 3x3 rotation portion of the 4x4 matrix to get the inverse rotation
    int i,j;

    for(i=0 ; i<3; i++)
    {
        for(j=0; j<3; j++)
        {
            MatOut[j][i] = MatIn[i][j];
        }
    }
}


/**
*Converts screen coordinates to OpenGL Viewport coordinates.
*@param point the screen coordinates.
*@param real the viewport coordinates.
*/
void gl3dView::screenToViewport(QPoint const &point, Vector3d &real)
{
    double h2, w2;
    h2 = double(geometry().height()) /2.0;
    w2 = double(geometry().width())  /2.0;

    if(w2>h2)
    {
        real.x =  (double(point.x()) - w2) / w2;
        real.y = -(double(point.y()) - h2) / w2;
    }
    else
    {
        real.x =  (double(point.x()) - w2) / h2;
        real.y = -(double(point.y()) - h2) / h2;
    }
}


/**
*Converts OpenGL Viewport coordinates to screen coordinates
*@param real the viewport coordinates.
*@param point the screen coordinates.
*/
void gl3dView::viewportToScreen(Vector3d const &real, QPoint &point)
{
    double dx, dy, h2, w2;

    h2 = m_GLViewRect.height() /2.0;
    w2 = m_GLViewRect.width()  /2.0;

    dx = ( real.x + w2)/2.0;
    dy = (-real.y + h2)/2.0;

    if(w2>h2)
    {
        point.setX(int(dx * double(geometry().width())));
        point.setY(int(dy * double(geometry().width())));
    }
    else
    {
        point.setX(int(dx * double(geometry().height())));
        point.setY(int(dy * double(geometry().height())));
    }
}


QVector4D gl3dView::worldToViewport(Vector3d v)
{
    QVector4D v4(v.xf(), v.yf(), v.zf(), 1.0f);
    return m_pvmMatrix * v4;
}


void gl3dView::viewportToWorld(Vector3d vp, Vector3d &w)
{
    //un-translate
    vp.x += - m_glViewportTrans.x*m_glScaled;
    vp.y += + m_glViewportTrans.y*m_glScaled;

    //un-scale
    vp.x *= 1.0/m_glScaled;
    vp.y *= 1.0/m_glScaled;
    vp.z *= 1.0/m_glScaled;


    //un-rotate
    w.x = double(m_ArcBall.ab_quat[0]*vp.xf() + m_ArcBall.ab_quat[1]*vp.yf() + m_ArcBall.ab_quat[2] *vp.zf());
    w.y = double(m_ArcBall.ab_quat[4]*vp.xf() + m_ArcBall.ab_quat[5]*vp.yf() + m_ArcBall.ab_quat[6] *vp.zf());
    w.z = double(m_ArcBall.ab_quat[8]*vp.xf() + m_ArcBall.ab_quat[9]*vp.yf() + m_ArcBall.ab_quat[10]*vp.zf());
}



void gl3dView::glRenderText(double x, double y, double z, const QString & str, QColor textColor)
{
    QPoint point;

    if(z>double(m_ClipPlanePos)) return;

    point = worldToScreen(Vector3d(x,y,z));
    point *= devicePixelRatio();
    if(!m_PixTextOverlay.isNull())
    {
        QPainter paint(&m_PixTextOverlay);
        paint.save();
        QPen textPen(textColor);
        paint.setPen(textPen);
        QFont font(paint.font());
        font.setPointSize(paint.font().pointSize()*devicePixelRatio());
        paint.setFont(font);
        paint.drawText(point, str);
        paint.restore();
    }
}


void gl3dView::glRenderText(int x, int y, const QString & str, QColor textColor)
{
    QPainter paint(&m_PixTextOverlay);
    paint.save();
    QPen textPen(textColor);
    paint.setPen(textPen);
    QFont font(paint.font());
    font.setPointSize(paint.font().pointSize()*devicePixelRatio());
    paint.setFont(font);
    paint.drawText(x*devicePixelRatio(),y*devicePixelRatio(), str);
    paint.restore();
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dView::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);


    double w, h, s;
    w = double(width);
    h = double(height);
    s = 1.0;


    if(w>h)    m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

    if(!m_PixTextOverlay.isNull())    m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size()*devicePixelRatio());
    if(!m_PixTextOverlay.isNull())    m_PixTextOverlay.fill(Qt::transparent);
}



void gl3dView::getGLError()
{
    switch(glGetError())
    {
        case GL_NO_ERROR:
            Trace("No error has been recorded. The value of this symbolic constant is guaranteed to be 0.");
            break;

        case GL_INVALID_ENUM:
            Trace("An unacceptable value is specified for an enumerated argument. "
                  "The offending command is ignored and has no other side effect than to set the error flag.");
            break;

        case GL_INVALID_VALUE:
            Trace("A numeric argument is out of range. The offending command is ignored and has no other "
                  "side effect than to set the error flag.");
            break;

        case GL_INVALID_OPERATION:
            Trace("The specified operation is not allowed in the current state. The offending command is "
                  "ignored and has no other side effect than to set the error flag.");
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            Trace("The command is trying to render to or read from the framebuffer while the currently "
                  "bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus "
                  "is not GL_FRAMEBUFFER_COMPLETE). The offending command is ignored and has no other side effect than "
                  "to set the error flag.");
            break;

        case GL_OUT_OF_MEMORY:
            Trace("There is not enough memory left to execute the command. The state of the GL is "
                  "undefined, except for the state of the error flags, after this error is recorded.");
            break;

        case GL_STACK_UNDERFLOW:
            Trace("An attempt has been made to perform an operation that would cause an internal stack to underflow.");
            break;

        case GL_STACK_OVERFLOW:
            Trace("An attempt has been made to perform an operation that would cause an internal stack to overflow.");
            break;
    }
}


static GLfloat const x_axis[] = {
    -1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f,   0.0f,   0.0f,
    0.95f,  0.015f, 0.015f,
    1.0f,  0.0f,    0.0f,
    0.95f,-0.015f,-0.015f
};

static GLfloat const y_axis[] = {
    0.0f,    -1.0f,    0.0f,
    0.0f,     1.0f,    0.0f,
    0.f,      1.0f,    0.0f,
    0.015f,   0.95f,   0.015f,
    0.f,      1.0f,    0.0f,
    -0.015f,   0.95f,  -0.015f
};

static GLfloat const z_axis[] = {
    0.0f,    0.0f,   -1.0f,
    0.0f,    0.0f,    1.0f,
    0.0f,    0.0f,    1.0f,
    0.015f,  0.015f,  0.95f,
    0.0f,    0.0f,    1.0f,
    -0.015f, -0.015f,  0.95f
};



#define NUMANGLES     10
#define NUMCIRCLES     6
#define NUMPERIM      35
#define NUMARCPOINTS  10




/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void gl3dView::glMakeArcPoint()
{
    float GLScale = 1.0f;
    int row=0, col=0;
    float Radius=0.1f, lat_incr=0, phi=0, theta=0;
    Vector3d eye(0.0, 0.0, 1.0);
    Vector3d up(0.0, 1.0, 0.0);
    m_ArcBall.setZoom(0.45, eye, up);

    Radius = float(m_ArcBall.ab_sphere);

    int iv=0;

    int bufferSize = NUMARCPOINTS*2*2*3;
    QVector<float> arcPointVertexArray (bufferSize);

    //ARCPOINT
    lat_incr = 30.0 / NUMARCPOINTS;

    phi = 0.0* PI/180.0;//longitude
    for (row = -NUMARCPOINTS; row < NUMARCPOINTS; row++)
    {
        theta = float(0.0f + row * lat_incr) * PIf/180.0f;
        arcPointVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
        arcPointVertexArray[iv++] = Radius*sin(theta)*GLScale;
        arcPointVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
    }

    theta = 0.0* PI/180.0;
    for(col=-NUMARCPOINTS; col<NUMARCPOINTS; col++)
    {
        phi = (0.0f + float(col)*30.0f/NUMARCPOINTS) * PIf/180.0f;
        arcPointVertexArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
        arcPointVertexArray[iv++] = Radius * sin(theta)*GLScale;
        arcPointVertexArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
    }

    Q_ASSERT(iv==bufferSize);

    m_vboArcPoint.destroy();
    m_vboArcPoint.create();
    m_vboArcPoint.bind();
    m_vboArcPoint.allocate(arcPointVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboArcPoint.release();
}


void gl3dView::glMakeBody3DFlatPanels(Body *pBody)
{
    Vector3d P1, P2, P3, P4, N, P1P3, P2P4, Tj, Tjp1;

    if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture) delete m_pRightBodyTexture;

    QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
    QString planeName;
    if(s_pMiarex && s_pMiarex->m_pCurPlane)
    {
        planeName = s_pMiarex->m_pCurPlane->planeName();
    }
    QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator();

    QImage leftTexture  = QImage(QString(texturePath+"body_left.png"));
    if(leftTexture.isNull()) leftTexture = QImage(QString(":/default_textures/body_left.png"));
    m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
    QImage rightTexture  = QImage(QString(texturePath+"body_right.png"));
    if(rightTexture.isNull()) rightTexture = QImage(QString(":/default_textures/body_right.png"));
    m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);


    int bufferSize = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
    bufferSize *= 2;  // two sides
    bufferSize *= 4;  // four vertices per quad
    bufferSize *= 8;  // 8 components per vertex
    QVector<float>pBodyVertexArray(bufferSize);

    //Create triangles
    //  indices array size:
    //    NX*NH
    //    2 triangles per/quad
    //    3 indices/triangle
    //    2 sides
    m_iBodyElems = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
    m_iBodyElems *= 2;    //two sides
    m_iBodyElems *= 2;    //two triangles per quad
    m_iBodyElems *= 3;    //three vertex per triangle

    m_BodyIndicesArray.resize(m_iBodyElems);
    m_BodyIndicesArray.fill(0);

    int iv=0;
    int ii=0;

    float fnh = pBody->sideLineCount();
    float fLength = float(pBody->length());

    float tip = 0.0;
    if(pBody->frameCount()) tip = pBody->frame(0)->m_Position.xf();

    //surfaces
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frame(j)->m_Position.x,     0.0, 0.0);
            Tjp1.set(pBody->frame(j+1)->m_Position.x, 0.0, 0.0);

            P1 = pBody->frame(j)->m_CtrlPoint[k];       P1.x = pBody->frame(j)->m_Position.x;
            P2 = pBody->frame(j+1)->m_CtrlPoint[k];     P2.x = pBody->frame(j+1)->m_Position.x;
            P3 = pBody->frame(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frame(j+1)->m_Position.x;
            P4 = pBody->frame(j)->m_CtrlPoint[k+1];     P4.x = pBody->frame(j)->m_Position.x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = P1P3 * P2P4;
            N.normalize();

            int i1 = iv/8;
            int i2 = i1+1;
            int i3 = i2+1;
            int i4 = i3+1;

            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            //first triangle
            m_BodyIndicesArray[ii]   = ushort(i1);
            m_BodyIndicesArray[ii+1] = ushort(i2);
            m_BodyIndicesArray[ii+2] = ushort(i3);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(i3);
            m_BodyIndicesArray[ii+4] = ushort(i4);
            m_BodyIndicesArray[ii+5] = ushort(i1);
            ii += 6;
        }
    }
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frame(j)->m_Position.x,     0.0, 0.0);
            Tjp1.set(pBody->frame(j+1)->m_Position.x, 0.0, 0.0);

            P1 = pBody->frame(j)->m_CtrlPoint[k];       P1.x = pBody->frame(j)->m_Position.x;
            P2 = pBody->frame(j+1)->m_CtrlPoint[k];     P2.x = pBody->frame(j+1)->m_Position.x;
            P3 = pBody->frame(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frame(j+1)->m_Position.x;
            P4 = pBody->frame(j)->m_CtrlPoint[k+1];     P4.x = pBody->frame(j)->m_Position.x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = P1P3 * P2P4;
            N.normalize();

            P1.y = -P1.y;
            P2.y = -P2.y;
            P3.y = -P3.y;
            P4.y = -P4.y;
            N.y = -N.y;

            int i1 = iv/8;
            int i2 = i1+1;
            int i3 = i2+1;
            int i4 = i3+1;

            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;
            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            //first triangle
            m_BodyIndicesArray[ii]   = ushort(i1);
            m_BodyIndicesArray[ii+1] = ushort(i2);
            m_BodyIndicesArray[ii+2] = ushort(i3);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(i3);
            m_BodyIndicesArray[ii+4] = ushort(i4);
            m_BodyIndicesArray[ii+5] = ushort(i1);
            ii += 6;
        }
    }
    Q_ASSERT(iv==bufferSize);
    Q_ASSERT(ii==m_iBodyElems);

    m_vboBody.destroy();
    m_vboBody.create();
    m_vboBody.bind();
    m_vboBody.allocate(pBodyVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboBody.release();
}


void gl3dView::glMakeBodySplines(Body *pBody)
{
    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();
    QVector<Vector3d> m_T((NXXXX+1)*(NHOOOP+1));
    Vector3d TALB, LATB;
    int j=0, k=0, l=0, p=0;

    if(!pBody)
    {
        return;
    }

    Vector3d Point;

    Vector3d N;

    if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture) delete m_pRightBodyTexture;

    QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
    QString planeName;
    if(s_pMiarex && s_pMiarex->m_pCurPlane)
    {
        planeName = s_pMiarex->m_pCurPlane->planeName();
    }
    QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator();

    QImage leftTexture  = QImage(QString(texturePath+"body_left.png"));
    if(leftTexture.isNull()) leftTexture = QImage(QString(":/default_textures/body_left.png"));
    m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
    QImage rightTexture  = QImage(QString(texturePath+"body_right.png"));
    if(rightTexture.isNull()) rightTexture = QImage(QString(":/default_textures/body_right.png"));
    m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);

    //vertices array size:
    // surface:
    //     (NX+1)*(NH+1) : from 0 to NX, and from 0 to NH
    //     x2 : 2 sides
    // outline:
    //     frameSize()*(NH+1)*2 : frames
    //     (NX+1) + (NX+1)      : top and bottom lines
    //
    // x8 : 3 vertices components, 3 normal components, 2 texture componenents
    int bodyVertexSize;
    bodyVertexSize  =   (NXXXX+1)*(NHOOOP+1) *2             // side surfaces
            + pBody->frameCount()*(NHOOOP+1)*2 // frames
            + (NXXXX+1)                       // top outline
            + (NXXXX+1);                      // bot outline

    bodyVertexSize *= 8; // 3 vertex components, 3 normal components, 2 uv components

    QVector<float> pBodyVertexArray(bodyVertexSize);

    p = 0;
    double ud=0, vd=0;
    for (k=0; k<=NXXXX; k++)
    {
        ud = double(k) / double(NXXXX);
        for (l=0; l<=NHOOOP; l++)
        {
            vd = double(l) / double(NHOOOP);
            pBody->getPoint(ud,  vd, true, m_T[p]);
            p++;
        }
    }

    int iv=0; //index of vertex components

    //right side first;
    p=0;
    for (k=0; k<=NXXXX; k++)
    {
        for (l=0; l<=NHOOOP; l++)
        {
            pBodyVertexArray[iv++] = m_T[p].xf();
            pBodyVertexArray[iv++] = m_T[p].yf();
            pBodyVertexArray[iv++] = m_T[p].zf();

            if(k==0)       N.set(-1.0, 0.0, 0.0);
            else if(k==NXXXX) N.set(1.0, 0.0, 0.0);
            else if(l==0)                N.set(0.0, 0.0, 1.0);
            else if(l==NHOOOP)                N.set(0.0,0.0, -1.0);
            else
            {
                LATB = m_T[p+NHOOOP+1] - m_T[p+1];     //    LATB = TB - LA;
                TALB = m_T[p]  - m_T[p+NHOOOP+2];      //    TALB = LB - TA;
                N = TALB * LATB;
                N.normalize();
            }

            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] =  N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            pBodyVertexArray[iv++] = float(l)/float(NHOOOP);
            p++;
        }
    }


    //left side next;
    p=0;
    for (k=0; k<=NXXXX; k++)
    {
        for (l=0; l<=NHOOOP; l++)
        {
            pBodyVertexArray[iv++] =  m_T[p].xf();
            pBodyVertexArray[iv++] = -m_T[p].yf();
            pBodyVertexArray[iv++] =  m_T[p].zf();

            if(k==0) N.set(-1.0, 0.0, 0.0);
            else if(k==NXXXX) N.set(1.0, 0.0, 0.0);
            else if(l==0)  N.set(0.0, 0.0, 1.0);
            else if(l==NHOOOP) N.set(0.0,0.0, -1.0);
            else
            {
                LATB = m_T[p+NHOOOP+1] - m_T[p+1];     //    LATB = TB - LA;
                TALB = m_T[p]  - m_T[p+NHOOOP+2];      //    TALB = LB - TA;
                N = TALB * LATB;
                N.normalize();
            }
            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] = -N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(k)/float(NXXXX);
            pBodyVertexArray[iv++] = float(l)/float(NHOOOP);
            p++;
        }
    }

    //OUTLINE
    double hinc=1./double(NHOOOP);
    double u=0, v=0;
    u=0.0; v = 0.0;

    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pBody->frameCount(); iFr++)
    {
        u = pBody->getu(pBody->frame(iFr)->m_Position.x);
        for (j=0; j<=NHOOOP; j++)
        {
            v = double(j)*hinc;
            pBody->getPoint(u,v,true, Point);
            pBodyVertexArray[iv++] = Point.xf();
            pBodyVertexArray[iv++] = Point.yf();
            pBodyVertexArray[iv++] = Point.zf();

            N = Vector3d(0.0, Point.y, Point.z);
            N.normalize();
            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] =  N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(u);
            pBodyVertexArray[iv++] = float(v);
        }

        for (j=NHOOOP; j>=0; j--)
        {
            v = double(j)*hinc;
            pBody->getPoint(u,v,false, Point);
            pBodyVertexArray[iv++] = Point.xf();
            pBodyVertexArray[iv++] = Point.yf();
            pBodyVertexArray[iv++] = Point.zf();
            N = Vector3d(0.0, Point.y, Point.z);
            N.normalize();
            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] =  N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(u);
            pBodyVertexArray[iv++] = float(v);
        }
    }

    //top line: NX+1
    v = 0.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint(double(iu)/double(NXXXX),v, true, Point);
        pBodyVertexArray[iv++] = Point.xf();
        pBodyVertexArray[iv++] = Point.yf();
        pBodyVertexArray[iv++] = Point.zf();

        pBodyVertexArray[iv++] = N.xf();
        pBodyVertexArray[iv++] = N.yf();
        pBodyVertexArray[iv++] = N.zf();

        pBodyVertexArray[iv++] = float(iu)/float(NXXXX);
        pBodyVertexArray[iv++] = float(v);
    }

    //bottom line: NX+1
    v = 1.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint(double(iu)/double(NXXXX),v, true, Point);
        pBodyVertexArray[iv++] = Point.xf();
        pBodyVertexArray[iv++] = Point.yf();
        pBodyVertexArray[iv++] = Point.zf();
        pBodyVertexArray[iv++] = N.xf();
        pBodyVertexArray[iv++] = N.yf();
        pBodyVertexArray[iv++] = N.zf();

        pBodyVertexArray[iv++] = float(iu)/float(NXXXX);
        pBodyVertexArray[iv++] = float(v);
    }
    Q_ASSERT(iv==bodyVertexSize);


    //Create triangles
    //  indices array size:
    //    NX*NH
    //    2 triangles per/quad
    //    3 indices/triangle
    //    2 sides
    m_BodyIndicesArray.resize(NXXXX*NHOOOP*2*3*2);

    int ii=0;
    int nV=0;

    //left side;
    for (int k=0; k<NXXXX; k++)
    {
        for (int l=0; l<NHOOOP; l++)
        {
            nV = k*(NHOOOP+1)+l; // id of the vertex at the bottom left of the quad
            //first triangle
            m_BodyIndicesArray[ii]   = ushort(nV);
            m_BodyIndicesArray[ii+1] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+2] = ushort(nV+1);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+4] = ushort(nV+1);
            m_BodyIndicesArray[ii+5] = ushort(nV+NHOOOP+1+1);
            ii += 6;
        }
    }

    //right side
    for (k=0; k<NXXXX; k++)
    {
        for (l=0; l<NHOOOP; l++)
        {
            nV = (NXXXX+1)*(NHOOOP+1) + k*(NHOOOP+1)+l; // id of the vertex at the bottom left of the quad
            //first triangle
            m_BodyIndicesArray[ii]   = ushort(nV);
            m_BodyIndicesArray[ii+1] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+2] = ushort(nV+1);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+4] = ushort(nV+1);
            m_BodyIndicesArray[ii+5] = ushort(nV+NHOOOP+1+1);
            ii += 6;
        }
    }
    m_iBodyElems = ii;

    pBody = nullptr;

    m_vboBody.destroy();
    m_vboBody.create();
    m_vboBody.bind();
    m_vboBody.allocate(pBodyVertexArray.data(), bodyVertexSize * int(sizeof(GLfloat)));
    m_vboBody.release();
}


void gl3dView::initializeGL()
{
    QSurfaceFormat ctxtFormat = format();
    m_bUse120StyleShaders = (ctxtFormat.majorVersion()*10+ctxtFormat.minorVersion())<33;

#ifdef QT_DEBUG
    printFormat(ctxtFormat);
#endif

    glMakeAxis();
    glMakeUnitSphere();
    glMakeArcBall();
    glMakeArcPoint();

    //setup the shader to paint lines
    QString vsrc = m_bUse120StyleShaders ? ":/shaders/line_vertexshader_120.glsl" : ":/shaders/line_vertexshader.glsl";
    QString fsrc = m_bUse120StyleShaders ? ":/shaders/line_fragmentshader_120.glsl" : ":/shaders/line_fragmentshader.glsl";

    m_ShaderProgramLine.addShaderFromSourceFile(QOpenGLShader::Vertex, (vsrc));
    if(m_ShaderProgramLine.log().length()) Trace("Line vertex shader log:"+m_ShaderProgramLine.log());
    m_ShaderProgramLine.addShaderFromSourceFile(QOpenGLShader::Fragment, (fsrc));
    if(m_ShaderProgramLine.log().length()) Trace("Line fragment shader log:"+m_ShaderProgramLine.log());
    m_ShaderProgramLine.link();
    m_ShaderProgramLine.bind();
    m_VertexLocationLine       = m_ShaderProgramLine.attributeLocation("vertex");
    m_mMatrixLocationLine      = m_ShaderProgramLine.uniformLocation("mMatrix");
    m_vMatrixLocationLine      = m_ShaderProgramLine.uniformLocation("vMatrix");
    m_pvmMatrixLocationLine    = m_ShaderProgramLine.uniformLocation("pvmMatrix");
    m_ColorLocationLine        = m_ShaderProgramLine.uniformLocation("color");
    m_ClipPlaneLocationLine    = m_ShaderProgramLine.uniformLocation("clipPlane0");
    m_ShaderProgramLine.release();


    //setup the shader to paint the Cp and other gradients
    vsrc = m_bUse120StyleShaders ? ":/shaders/gradient_vertexshader_120.glsl" : ":/shaders/gradient_vertexshader.glsl";
    fsrc = m_bUse120StyleShaders ? ":/shaders/gradient_fragmentshader_120.glsl" : ":/shaders/gradient_fragmentshader.glsl";
    m_ShaderProgramGradient.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_ShaderProgramGradient.log().length()) Trace("Gradient vertex shader log:"+m_ShaderProgramGradient.log());

    m_ShaderProgramGradient.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_ShaderProgramGradient.log().length()) Trace("Gradient fragment shader log:"+m_ShaderProgramGradient.log());

    m_ShaderProgramGradient.link();
    m_ShaderProgramGradient.bind();
    m_VertexLocationGradient = m_ShaderProgramGradient.attributeLocation("vertexPosition_modelSpace");
    m_pvmMatrixLocationGradient = m_ShaderProgramGradient.uniformLocation("pvmMatrix");
    m_ColorLocationGradient  = m_ShaderProgramGradient.attributeLocation("vertexColor");
    m_ShaderProgramGradient.release();


    //setup the shader to paint colored surfaces
    vsrc = m_bUse120StyleShaders ? ":/shaders/surface_vertexshader_120.glsl" : ":/shaders/surface_vertexshader.glsl";
    fsrc = m_bUse120StyleShaders ? ":/shaders/surface_fragmentshader_120.glsl" : ":/shaders/surface_fragmentshader.glsl";
    m_ShaderProgramSurface.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_ShaderProgramSurface.log().length()) Trace("Surface vertex shader log:"+m_ShaderProgramSurface.log());

    m_ShaderProgramSurface.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_ShaderProgramSurface.log().length()) Trace("Surface fragment shader log:"+m_ShaderProgramSurface.log());

    m_ShaderProgramSurface.link();
    m_ShaderProgramSurface.bind();
    m_VertexLocationSurface = m_ShaderProgramSurface.attributeLocation("vertexPosition_modelSpace");
    m_NormalLocationSurface = m_ShaderProgramSurface.attributeLocation("vertexNormal_modelSpace");
    m_ClipPlaneLocationSurface     = m_ShaderProgramSurface.uniformLocation("clipPlane0");
    m_pvmMatrixLocationSurface     = m_ShaderProgramSurface.uniformLocation("pvmMatrix");
    m_vMatrixLocationSurface       = m_ShaderProgramSurface.uniformLocation("vMatrix");
    m_mMatrixLocationSurface       = m_ShaderProgramSurface.uniformLocation("mMatrix");
    m_EyePosLocationSurface        = m_ShaderProgramSurface.uniformLocation("EyePosition_viewSpace");
    m_LightPosLocationSurface      = m_ShaderProgramSurface.uniformLocation("LightPosition_viewSpace");
    m_LightColorLocationSurface    = m_ShaderProgramSurface.uniformLocation("LightColor");
    m_LightAmbientLocationSurface  = m_ShaderProgramSurface.uniformLocation("LightAmbient");
    m_LightDiffuseLocationSurface  = m_ShaderProgramSurface.uniformLocation("LightDiffuse");
    m_LightSpecularLocationSurface = m_ShaderProgramSurface.uniformLocation("LightSpecular");
    m_ColorLocationSurface         = m_ShaderProgramSurface.uniformLocation("incolor");
    m_LightLocationSurface         = m_ShaderProgramSurface.uniformLocation("lightOn");
    m_SurfaceLocationSurface       = m_ShaderProgramSurface.uniformLocation("hasSurface");
    m_MaterialShininessSurface     = m_ShaderProgramSurface.uniformLocation("MaterialShininess");
    m_AttenuationConstantSurface   = m_ShaderProgramSurface.uniformLocation("Kc");
    m_AttenuationLinearSurface     = m_ShaderProgramSurface.uniformLocation("Kl");
    m_AttenuationQuadraticSurface  = m_ShaderProgramSurface.uniformLocation("Kq");
    m_ShaderProgramSurface.release();

    //setup the shader to paint textured surfaces
    vsrc = m_bUse120StyleShaders ? ":/shaders/texture_vertexshader_120.glsl" : ":/shaders/texture_vertexshader.glsl";
    fsrc = m_bUse120StyleShaders ? ":/shaders/texture_fragmentshader_120.glsl" : ":/shaders/texture_fragmentshader.glsl";
    m_ShaderProgramTexture.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_ShaderProgramTexture.log().length()) Trace("Texture vertex shader log:"+m_ShaderProgramTexture.log());

    m_ShaderProgramTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_ShaderProgramTexture.log().length()) Trace("Texture fragment shader log:"+m_ShaderProgramTexture.log());

    m_ShaderProgramTexture.link();
    m_ShaderProgramTexture.bind();
    m_VertexLocationTexture = m_ShaderProgramTexture.attributeLocation("vertexPosition_modelSpace");
    m_NormalLocationTexture = m_ShaderProgramTexture.attributeLocation("vertexNormal_modelSpace");
    m_UVLocationTexture     = m_ShaderProgramTexture.attributeLocation("vertexUV");
    m_ClipPlaneLocationTexture     = m_ShaderProgramTexture.uniformLocation("clipPlane0");
    m_pvmMatrixLocationTexture     = m_ShaderProgramTexture.uniformLocation("pvmMatrix");
    m_vMatrixLocationTexture       = m_ShaderProgramTexture.uniformLocation("vMatrix");
    m_mMatrixLocationTexture       = m_ShaderProgramTexture.uniformLocation("mMatrix");
    m_EyePosLocationTexture        = m_ShaderProgramTexture.uniformLocation("EyePosition_viewSpace");
    m_LightPosLocationTexture      = m_ShaderProgramTexture.uniformLocation("LightPosition_viewSpace");
    m_LightColorLocationTexture    = m_ShaderProgramTexture.uniformLocation("LightColor");
    m_LightAmbientLocationTexture  = m_ShaderProgramTexture.uniformLocation("LightAmbient");
    m_LightDiffuseLocationTexture  = m_ShaderProgramTexture.uniformLocation("LightDiffuse");
    m_LightSpecularLocationTexture = m_ShaderProgramTexture.uniformLocation("LightSpecular");
    m_LightLocationTexture         = m_ShaderProgramTexture.uniformLocation("lightOn");
    m_MaterialShininessTexture     = m_ShaderProgramTexture.uniformLocation("MaterialShininess");
    m_AttenuationConstantTexture   = m_ShaderProgramTexture.uniformLocation("Kc");
    m_AttenuationLinearTexture     = m_ShaderProgramTexture.uniformLocation("Kl");
    m_AttenuationQuadraticTexture  = m_ShaderProgramTexture.uniformLocation("Kq");
    m_ShaderProgramTexture.release();

    glSetupLight();
}


void gl3dView::glSetupLight()
{
    QColor LightColor;
    LightColor.setRedF(  double(GLLightDlg::s_Light.m_Red));
    LightColor.setGreenF(double(GLLightDlg::s_Light.m_Green));
    LightColor.setBlueF( double(GLLightDlg::s_Light.m_Blue));

    m_ShaderProgramSurface.bind();
    if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
    else                                 m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
    m_ShaderProgramSurface.setUniformValue(m_LightPosLocationSurface,      GLfloat(GLLightDlg::s_Light.m_X), GLfloat(GLLightDlg::s_Light.m_Y), GLfloat(GLLightDlg::s_Light.m_Z));
    m_ShaderProgramSurface.setUniformValue(m_LightColorLocationSurface,    LightColor);
    m_ShaderProgramSurface.setUniformValue(m_LightAmbientLocationSurface,  GLfloat(GLLightDlg::s_Light.m_Ambient));
    m_ShaderProgramSurface.setUniformValue(m_LightDiffuseLocationSurface,  GLfloat(GLLightDlg::s_Light.m_Diffuse));
    m_ShaderProgramSurface.setUniformValue(m_LightSpecularLocationSurface, GLfloat(GLLightDlg::s_Light.m_Specular));
    m_ShaderProgramSurface.setUniformValue(m_MaterialShininessSurface,     GLLightDlg::s_iShininess);
    m_ShaderProgramSurface.setUniformValue(m_AttenuationConstantSurface,   GLLightDlg::s_Attenuation.m_Constant);
    m_ShaderProgramSurface.setUniformValue(m_AttenuationLinearSurface,     GLLightDlg::s_Attenuation.m_Linear);
    m_ShaderProgramSurface.setUniformValue(m_AttenuationQuadraticSurface,  GLLightDlg::s_Attenuation.m_Quadratic);
    m_ShaderProgramSurface.release();

    m_ShaderProgramTexture.bind();
    if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 1);
    else                                 m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 0);
    m_ShaderProgramTexture.setUniformValue(m_LightPosLocationTexture,      GLfloat(GLLightDlg::s_Light.m_X), GLfloat(GLLightDlg::s_Light.m_Y), GLfloat(GLLightDlg::s_Light.m_Z));
    m_ShaderProgramTexture.setUniformValue(m_LightColorLocationTexture,    LightColor);
    m_ShaderProgramTexture.setUniformValue(m_LightAmbientLocationTexture,  GLfloat(GLLightDlg::s_Light.m_Ambient));
    m_ShaderProgramTexture.setUniformValue(m_LightDiffuseLocationTexture,  GLfloat(GLLightDlg::s_Light.m_Diffuse));
    m_ShaderProgramTexture.setUniformValue(m_LightSpecularLocationTexture, GLfloat(GLLightDlg::s_Light.m_Specular));
    m_ShaderProgramTexture.setUniformValue(m_MaterialShininessTexture,     GLLightDlg::s_iShininess);
    m_ShaderProgramTexture.setUniformValue(m_AttenuationConstantTexture,   GLLightDlg::s_Attenuation.m_Constant);
    m_ShaderProgramTexture.setUniformValue(m_AttenuationLinearTexture,     GLLightDlg::s_Attenuation.m_Linear);
    m_ShaderProgramTexture.setUniformValue(m_AttenuationQuadraticTexture,  GLLightDlg::s_Attenuation.m_Quadratic);
    m_ShaderProgramTexture.release();
}


void gl3dView::paintGL()
{
    paintGL3();
    paintOverlay();
}


void gl3dView::paintGL3()
{
    //    makeCurrent();
    int width, height;

    float s = 1.0f;
    double pixelRatio = devicePixelRatio();

    glClearColor(float(Settings::backgroundColor().redF()), float(Settings::backgroundColor().greenF()), float(Settings::backgroundColor().blueF()), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    m_ShaderProgramSurface.bind();
    m_ShaderProgramSurface.setUniformValue(m_EyePosLocationSurface, QVector3D(0.0,0.0,500.0f*s));
    m_ShaderProgramSurface.release();
    m_ShaderProgramTexture.bind();
    m_ShaderProgramTexture.setUniformValue(m_EyePosLocationTexture, QVector3D(0.0,0.0,500.0f*s));
    m_ShaderProgramTexture.release();

    QVector4D clipplane(0.0,0.0,-1,m_ClipPlanePos);

    m_ShaderProgramLine.bind();
    m_ShaderProgramLine.setUniformValue(m_ClipPlaneLocationLine, clipplane);
    m_ShaderProgramLine.release();

    m_ShaderProgramSurface.bind();
    m_ShaderProgramSurface.setUniformValue(m_ClipPlaneLocationSurface, clipplane);
    m_ShaderProgramSurface.release();

    m_ShaderProgramTexture.bind();
    m_ShaderProgramTexture.setUniformValue(m_ClipPlaneLocationTexture, clipplane);
    m_ShaderProgramTexture.release();

    width  = int(geometry().width() * pixelRatio);
    height = int(geometry().height() * pixelRatio);

    m_orthoMatrix.setToIdentity();
    m_orthoMatrix.ortho(-s,s,-(height*s)/width,(height*s)/width,-500.0f*s,500.0f*s);

    QMatrix4x4 matQuat(m_ArcBall.ab_quat);

    m_modelMatrix.setToIdentity();//keep identity
    m_viewMatrix = matQuat.transposed();
    m_pvmMatrix = m_orthoMatrix * m_viewMatrix * m_modelMatrix;

    m_ShaderProgramLine.bind();
    m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
    m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
    m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
    m_ShaderProgramLine.release();

    if(m_bArcball) paintArcBall();

    if(s_pMainFrame->m_glLightDlg.isVisible())
    {
        Vector3d lightPos(GLLightDlg::s_Light.m_X, GLLightDlg::s_Light.m_Y, GLLightDlg::s_Light.m_Z);
        double radius = (GLLightDlg::s_Light.m_Z+2.0)/73.0;
        QColor lightColor;
        lightColor.setRedF(GLLightDlg::s_Light.m_Red);
        lightColor.setGreenF(GLLightDlg::s_Light.m_Green);
        lightColor.setBlueF(GLLightDlg::s_Light.m_Blue);
        lightColor.setAlphaF(1.0);
        m_pvmMatrix = m_orthoMatrix;
        paintSphere(lightPos, radius, lightColor, false);
    }


    float glScalef = float(m_glScaled);

    if(m_bAxes)
    {
        // fixed scale axis for the axis
    //    m_viewMatrix.scale(0.5, 0.5, 0.5);

        m_viewMatrix.scale(glScalef, glScalef, glScalef);
        m_viewMatrix.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());
        m_viewMatrix.scale(0.3f/glScalef, 0.3f/glScalef, 0.3f/glScalef);
        m_pvmMatrix = m_orthoMatrix * m_viewMatrix;
        paintAxes();
    }

    m_viewMatrix= matQuat.transposed();
    m_viewMatrix.scale(glScalef, glScalef, glScalef);
    m_viewMatrix.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());
    m_pvmMatrix = m_orthoMatrix * m_viewMatrix;


    glRenderView();

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}


void gl3dView::setScale(double refLength)
{
    m_glScaled = 1.5/refLength;
}


void gl3dView::paintFoilNames(Wing *pWing)
{
    int j=0;
    Foil *pFoil=nullptr;

    for(j=0; j<pWing->m_Surface.size(); j++)
    {
        pFoil = pWing->m_Surface.at(j)->foilA();

        if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TA.x, pWing->m_Surface.at(j)->m_TA.y, pWing->m_Surface.at(j)->m_TA.z,
                               pFoil->foilName(),
                               QColor(Qt::cyan).lighter(175));
    }

    j = pWing->m_Surface.size()-1;
    pFoil = pWing->m_Surface.at(j)->foilB();
    if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TB.x, pWing->m_Surface.at(j)->m_TB.y, pWing->m_Surface.at(j)->m_TB.z,
                           pFoil->foilName(),
                           QColor(Qt::cyan).lighter(175));
}


/**
 * Draws the point masses, the object masses, and the CG position in the OpenGL viewport
*/
void gl3dView::glDrawMasses(Plane *pPlane)
{
    if(!pPlane) return;
    double delta = 0.02/m_glScaled;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pPlane->wing(iw))
        {
            paintMasses(pPlane->wing(iw)->m_VolumeMass, pPlane->WingLE(iw),
                        pPlane->wing(iw)->m_WingName,   pPlane->wing(iw)->m_PointMass);
        }
    }

    paintMasses(0.0, Vector3d(0.0,0.0,0.0),"",pPlane->m_PointMass);


    if(pPlane->body())
    {
        Body *pCurBody = pPlane->body();

        paintMasses(pCurBody->m_VolumeMass,
                    pPlane->bodyPos(),
                    pCurBody->m_BodyName,
                    pCurBody->m_PointMass);
    }

    //plot CG
    Vector3d Place(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z);
    paintSphere(Place, W3dPrefsDlg::s_MassRadius*2.0/m_glScaled,
                W3dPrefsDlg::s_MassColor.lighter());

    glRenderText(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z + delta,
                 "CoG "+QString("%1").arg(pPlane->totalMass()*Units::kgtoUnit(), 7,'g',3)
                 +Units::weightUnitLabel(), W3dPrefsDlg::s_MassColor.lighter(125));
}


void gl3dView::paintMasses(double volumeMass, Vector3d pos, QString tag, const QVector<PointMass*> &ptMasses)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    double delta = 0.02/m_glScaled;
    if(qAbs(volumeMass)>PRECISION)
    {
        glRenderText(pos.x, pos.y, pos.z + delta,
                     tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
    }

    for(int im=0; im<ptMasses.size(); im++)
    {
        paintSphere(ptMasses[im]->position() +pos,
                    W3dPrefsDlg::s_MassRadius/m_glScaled,
                    W3dPrefsDlg::s_MassColor.lighter(),
                    true);
        glRenderText(ptMasses[im]->position().x + pos.x,
                     ptMasses[im]->position().y + pos.y,
                     ptMasses[im]->position().z + pos.z + delta,
                     ptMasses[im]->tag()+QString(" (%1").arg(ptMasses[im]->mass()*Units::kgtoUnit(), 0,'g',3)+Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
    }
}



/** used in GL3DWingDlg and gl3dBodyView*/
void gl3dView::paintSectionHighlight()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_ShaderProgramLine.bind();
    m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
    m_vboHighlight.bind();
    m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(255,0,0));
    glLineWidth(5);

    int pos = 0;
    for(int iLines=0; iLines<m_nHighlightLines; iLines++)
    {
        glDrawArrays(GL_LINE_STRIP, pos, m_HighlightLineSize);
        pos += m_HighlightLineSize;
    }

    m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
    m_vboHighlight.release();
    m_ShaderProgramLine.release();
}



/** Default mesh, if no polar has been defined */
void gl3dView::paintEditWingMesh(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_ShaderProgramLine.bind();
    m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
    m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
    m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
    vbo.bind();
    m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
    m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);

    int nTriangles = vbo.size()/3/3/int(sizeof(float)); // three vertices and three components

    f->glLineWidth(W3dPrefsDlg::s_VLMWidth);
    int pos = 0;
    for(int p=0; p<nTriangles; p++)
    {
        f->glDrawArrays(GL_LINE_STRIP, pos, 3);
        pos +=3 ;
    }

    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, Settings::backgroundColor());

    f->glEnable(GL_POLYGON_OFFSET_FILL);
    f->glPolygonOffset(1.0, 1.0);
    f->glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
    f->glDisable(GL_POLYGON_OFFSET_FILL);

    vbo.release();
    m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
    m_ShaderProgramLine.release();

    f->glDisable(GL_POLYGON_OFFSET_FILL);
}


void gl3dView::paintArcBall()
{    
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_ShaderProgramLine.bind();
    m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
    m_vboArcBall.bind();
    m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(50,55,80,255));

    glLineWidth(1.0);
    int pos=0;
    for (int col=0; col<NUMCIRCLES*2; col++)
    {
        glDrawArrays(GL_LINE_STRIP, pos, NUMANGLES-2);
        pos += NUMANGLES-2;
    }
    glDrawArrays(GL_LINE_STRIP, pos, NUMPERIM-1);
    pos += NUMPERIM-1;
    glDrawArrays(GL_LINE_STRIP, pos, NUMPERIM-1);
    pos += NUMPERIM-1;
    m_vboArcBall.release();

    if(m_bCrossPoint)
    {
        QMatrix4x4 pvmCP(m_orthoMatrix);
        m_ArcBall.rotateCrossPoint();
        pvmCP.rotate(float(m_ArcBall.angle), m_ArcBall.p.xf(), m_ArcBall.p.yf(), m_ArcBall.p.zf());
        m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, pvmCP);

        m_vboArcPoint.bind();
        m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
        m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(70, 25, 40));
        pos=0;
        glLineWidth(3.0);
        glDrawArrays(GL_LINE_STRIP, pos, 2*NUMARCPOINTS);
        pos += 2*NUMARCPOINTS;
        glDrawArrays(GL_LINE_STRIP, pos, 2*NUMARCPOINTS);
        glLineWidth(1.0);
        m_vboArcPoint.release();
    }
    m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
    m_ShaderProgramLine.release();
}


void gl3dView::paintAxes()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
//    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    m_ShaderProgramLine.bind();

    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_3DAxisColor);
    m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
    m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
    m_vboAxis.bind();
    //draw Axis
    glLineWidth(W3dPrefsDlg::s_3DAxisWidth);

    m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
    m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);

    int nvertices = m_vboAxis.size()/int(sizeof(float))/3; // three components
    glDrawArrays(GL_LINES, 0, nvertices);

    m_vboAxis.release();

    m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
    m_ShaderProgramLine.release();

    glRenderText(1.0, 0.015, 0.015, "X", W3dPrefsDlg::s_3DAxisColor);
    glRenderText(0.015, 1.0, 0.015, "Y", W3dPrefsDlg::s_3DAxisColor);
    glRenderText(0.015, 0.015, 1.0, "Z", W3dPrefsDlg::s_3DAxisColor);
}


void gl3dView::setSpanStations(Plane *pPlane, WPolar *pWPolar, PlaneOpp *pPOpp)
{
    if(!pPlane || !pWPolar || !pPOpp) return;
    Wing *pWing;

    if(pWPolar->isLLTMethod())
    {
        if(pPOpp)
        {
            m_Ny[0] = pPOpp->m_pWOpp[0]->m_NStation-1;
        }
        else
        {
            m_Ny[0] = LLTAnalysis::nSpanStations();
        }

        m_Ny[1] = m_Ny[2] = m_Ny[3] = 0;
    }
    else
    {
        for(int iWing=0; iWing<MAXWINGS; iWing++)
        {
            pWing = pPlane->wing(iWing);
            if(pWing)
            {
                m_Ny[iWing]=0;
                for (int j=0; j<pWing->m_Surface.size(); j++)
                {
                    m_Ny[iWing] += pWing->m_Surface[j]->NYPanels();
                }
            }
        }
    }
}


void gl3dView::paintBody(Body *pBody)
{
    if(!pBody) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int pos = 0;
    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();

    bool bTextures = pBody->textures() && (m_pLeftBodyTexture && m_pRightBodyTexture);
    if(bTextures)
    {
        m_ShaderProgramTexture.bind();
        m_vboBody.bind();

        m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, m_modelMatrix);
        m_ShaderProgramTexture.setUniformValue(m_vMatrixLocationTexture, m_viewMatrix);
        m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);
        if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 1);
        else                                 m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 0);
        m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
        m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);
        m_ShaderProgramTexture.enableAttributeArray(m_UVLocationTexture);
        m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
        m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
        m_ShaderProgramTexture.setAttributeBuffer(m_UVLocationTexture,     GL_FLOAT, 6* sizeof(GLfloat), 2, 8 * sizeof(GLfloat));
    }
    else
    {
        m_ShaderProgramSurface.bind();
        m_vboBody.bind();

        m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, m_modelMatrix);
        m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
        m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix);
        if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
        else                                 m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
        m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, color(pBody->bodyColor()));
        m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
        m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
        m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
        m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
    }


    if(m_bSurfaces)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);

        if(bTextures) m_pRightBodyTexture->bind();
        glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data());
        if(bTextures) m_pRightBodyTexture->release();
        if(bTextures) m_pLeftBodyTexture->bind();
        glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data()+m_iBodyElems/2);
        if(bTextures) m_pLeftBodyTexture->release();

        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    m_vboBody.release();

    if(bTextures)
    {
        m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
        m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
        m_ShaderProgramTexture.disableAttributeArray(m_UVLocationTexture);
        m_ShaderProgramTexture.release();
    }
    else
    {
        m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
        m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
        m_ShaderProgramSurface.release();
    }


    if(m_bOutline)
    {
        m_ShaderProgramLine.bind();
        m_vboBody.bind();
        m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
        m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));
        m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);

        m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
        m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
        m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

        glLineWidth(W3dPrefsDlg::s_OutlineWidth);

        if(pBody->isSplineType())
        {
            pos = (NXXXX+1) * (NHOOOP+1) * 2;
            for(int iFr=0; iFr<pBody->frameCount(); iFr++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, (NHOOOP+1)*2);
                pos += (NHOOOP+1)*2;
            }
            glDrawArrays(GL_LINE_STRIP, pos, NXXXX+1);
            pos += NXXXX+1;
            glDrawArrays(GL_LINE_STRIP, pos, NXXXX+1);
        }
        else if(pBody->isFlatPanelType())
        {
            int pos=0;
            for(int i=0; i<m_iBodyElems/2; i++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, 4);
                pos +=4;
            }
        }

        m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
        m_vboBody.release();
        m_ShaderProgramLine.release();
    }
}


/** Default mesh, if no polar has been defined */
void gl3dView::paintEditBodyMesh(Body *pBody)
{
    if(!pBody) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_ShaderProgramLine.bind();
    m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
    m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
    m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
    m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
    m_vboEditBodyMesh.bind();
    m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));
    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);
    //    m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, Qt::red);

    if(pBody->isFlatPanelType())
    {
        f->glLineWidth(W3dPrefsDlg::s_VLMWidth);

        //        f->glPolygonOffset(1.0, 1.0);
        f->glDrawArrays(GL_LINES, 0, m_iBodyMeshLines*2);
    }
    else if(pBody->isSplineType())
    {
        int pos=0;
        int NXXXX = W3dPrefsDlg::bodyAxialRes();
        int NHOOOP = W3dPrefsDlg::bodyHoopRes();
        f->glLineWidth(W3dPrefsDlg::s_VLMWidth);

        pos=0;
        //x-lines
        for (int l=0; l<2*pBody->m_nhPanels; l++)
        {
            f->glDrawArrays(GL_LINE_STRIP, pos, NXXXX);
            pos += NXXXX;
        }

        //hoop lines;
        for (int k=0; k<2*pBody->m_nxPanels; k++)
        {
            f->glDrawArrays(GL_LINE_STRIP, pos, NHOOOP);
            pos += NHOOOP;
        }
    }

    m_vboEditBodyMesh.release();

    //mesh background
    m_ShaderProgramSurface.bind();
    m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, m_modelMatrix);
    m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
    m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix);
    m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0); // no light for the background
    m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
    m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
    m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, Settings::backgroundColor());

    m_vboEditBodyMesh.bind();
    m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 6*sizeof(GLfloat));
    m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 6*sizeof(GLfloat));

    f->glEnable(GL_POLYGON_OFFSET_FILL);
    f->glPolygonOffset(1.0, 1.0);
    f->glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data());
    f->glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data()+m_iBodyElems/2);
    f->glDisable(GL_POLYGON_OFFSET_FILL);

    m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
    m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
    m_ShaderProgramSurface.release();


    m_vboEditBodyMesh.release();
    m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
    m_ShaderProgramLine.release();
}


void gl3dView::paintWing(int iWing, Wing *pWing)
{
    if(!pWing) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int CHORDPOINTS = W3dPrefsDlg::chordwiseRes();

    if(m_bSurfaces)
    {
        QVector<ushort> const &wingIndicesArray = m_WingIndicesArray[iWing];

        int pos = 0;

        bool bTextures = pWing->textures() &&
                (m_pWingBotLeftTexture[iWing] && m_pWingBotRightTexture[iWing] && m_pWingTopLeftTexture[iWing] && m_pWingTopRightTexture[iWing]);

        if(bTextures)
        {
            m_ShaderProgramTexture.bind();
            m_vboWingSurface[iWing].bind();

            m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, m_modelMatrix);
            m_ShaderProgramTexture.setUniformValue(m_vMatrixLocationTexture, m_viewMatrix);
            m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);
            if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 1);
            else                                 m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 0);

            m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
            m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);
            m_ShaderProgramTexture.enableAttributeArray(m_UVLocationTexture);
            m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
            m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
            m_ShaderProgramTexture.setAttributeBuffer(m_UVLocationTexture,     GL_FLOAT, 6* sizeof(GLfloat), 2, 8 * sizeof(GLfloat));
        }
        else
        {
            m_ShaderProgramSurface.bind();
            m_vboWingSurface[iWing].bind();
            m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, m_modelMatrix);
            m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
            m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix);
            if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
            else                                 m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
            m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, color(pWing->wingColor()));

            m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
            m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
            m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
            m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
        }


        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);

        //indices array size:
        //  Top & bottom surfaces
        //      NSurfaces
        //      x (ChordPoints-1)quads
        //      x2 triangles per/quad
        //      x2 top and bottom surfaces
        //      x3 indices/triangle

        pos = 0;
        for (int j=0; j<pWing->m_Surface.count(); j++)
        {
            //top surface
            if(bTextures)
            {
                if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingTopLeftTexture[iWing]->bind();
                else                                     m_pWingTopRightTexture[iWing]->bind();
            }
            glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
            if(bTextures)
            {
                if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingTopLeftTexture[iWing]->release();
                else                                     m_pWingTopRightTexture[iWing]->release();
            }
            pos += (CHORDPOINTS-1)*2*3;
            // bottom surface
            if(bTextures)
            {
                if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingBotLeftTexture[iWing]->bind();
                else                                     m_pWingBotRightTexture[iWing]->bind();
            }
            glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
            if(bTextures)
            {
                if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingBotLeftTexture[iWing]->release();
                else                                     m_pWingBotRightTexture[iWing]->release();
            }
            pos += (CHORDPOINTS-1)*2*3;
        }

        for (int j=0; j<pWing->m_Surface.count(); j++)
        {
            //tip ssurface
            if(pWing->m_Surface.at(j)->isTipLeft())
            {
                glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
                pos += (CHORDPOINTS-1)*2*3;
            }

            if(pWing->m_Surface.at(j)->isTipRight())
            {
                glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
                pos += (CHORDPOINTS-1)*2*3;
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_vboWingSurface[iWing].release();
        if(bTextures)
        {
            m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
            m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
            m_ShaderProgramTexture.disableAttributeArray(m_UVLocationTexture);
            m_ShaderProgramTexture.release();
        }
        else
        {
            m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
            m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
            m_ShaderProgramSurface.release();
        }
    }

    if(m_bOutline)
    {
        m_ShaderProgramLine.bind();
        m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);
        m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
        m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
        m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

        m_vboWingOutline[iWing].bind();
        m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
        m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3* sizeof(GLfloat));

        glLineWidth(W3dPrefsDlg::s_OutlineWidth);
        glEnable (GL_LINE_STIPPLE);
        GLLineStipple(W3dPrefsDlg::s_OutlineStyle);

        glDrawArrays(GL_LINES, 0, m_iWingOutlinePoints[iWing]);
        m_vboWingOutline[iWing].release();

        m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
        m_ShaderProgramLine.release();
    }
    glDisable(GL_LINE_STIPPLE);
}


/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void gl3dView::glMakeArcBall()
{
    float GLScale = 1.0f;

    float Radius=0.1f, lat_incr, lon_incr, phi, theta;
    Vector3d eye(0.0, 0.0, 1.0);
    Vector3d up(0.0, 1.0, 0.0);
    m_ArcBall.setZoom(0.45, eye, up);

    Radius = float(m_ArcBall.ab_sphere);
    lat_incr =  90.0f / NUMANGLES;
    lon_incr = 360.0f / NUMCIRCLES;

    int iv=0;

    int bufferSize = ((NUMCIRCLES*2)*(NUMANGLES-2) + (NUMPERIM-1)*2)*3;
    QVector<float>arcBallVertexArray(bufferSize, 0);

    //ARCBALL
    for (int col=0; col<NUMCIRCLES; col++)
    {
        //first
        phi = (col * lon_incr) * PIf/180.0f;
        for (int row=1; row<NUMANGLES-1; row++)
        {
            theta = (row * lat_incr) * PIf/180.0f;
            arcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta)*GLScale;
            arcBallVertexArray[iv++] = Radius*sinf(theta)*GLScale;
            arcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta)*GLScale;
        }
    }

    for (int col=0; col<NUMCIRCLES; col++)
    {
        //Second
        phi = (col * lon_incr ) * PIf/180.0f;
        for (int row=1; row<NUMANGLES-1; row++)
        {
            theta = -(row * lat_incr) * PIf/180.0f;
            arcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta)*GLScale;
            arcBallVertexArray[iv++] = Radius*sinf(theta)*GLScale;
            arcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta)*GLScale;
        }
    }

    theta = 0.;
    for(int col=1; col<NUMPERIM; col++)
    {
        phi = (0.0f + col*360.0f/72.0f) * PIf/180.0f;
        arcBallVertexArray[iv++] = Radius * cosf(phi) * cosf(theta)*GLScale;
        arcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        arcBallVertexArray[iv++] = Radius * sinf(phi) * cosf(theta)*GLScale;
    }

    theta = 0.;
    for(int col=1; col<NUMPERIM; col++)
    {
        phi = (0.0f + col*360.0f/72.0f) * PIf/180.0f;
        arcBallVertexArray[iv++] = Radius * cosf(-phi) * cosf(theta)*GLScale;
        arcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        arcBallVertexArray[iv++] = Radius * sinf(-phi) * cosf(theta)*GLScale;
    }
    Q_ASSERT(iv==bufferSize);

    m_vboArcBall.destroy();
    m_vboArcBall.create();
    m_vboArcBall.bind();
    m_vboArcBall.allocate(arcBallVertexArray.data(), iv * int(sizeof(GLfloat)));
    m_vboArcBall.release();
}

#define NUMLONG  43
#define NUMLAT   37

/**
Creates a list for a sphere with unit radius
*/
void gl3dView::glMakeUnitSphere()
{
    float start_lat = -90.0f * PIf/180.0f;
    float start_lon = 0.0f * PIf/180.0f;

    float lat_incr = 180.0f / (NUMLAT-1) * PIf/180.0f;
    float lon_incr = 360.0f / (NUMLONG-1) * PIf/180.0f;

    int bufferSize = NUMLONG * NUMLAT * 2 *3;
    QVector<GLfloat>sphereVertexArray(bufferSize);
    m_SphereIndicesArray  = new unsigned short[(NUMLONG-1) * NUMLAT * 2];

    int iv = 0;

    for (int iLong=0; iLong<NUMLONG; iLong++)
    {
        float phi = (start_lon + iLong * lon_incr) ;
        for (int iLat=0; iLat<NUMLAT; iLat++)
        {
            float theta = (start_lat + iLat * lat_incr);
            // the point
            sphereVertexArray[iv++] = cosf(phi) * cosf(theta);//x
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta);//z
            sphereVertexArray[iv++] = sinf(theta);//y
            //the normal
            sphereVertexArray[iv++] = cos(phi) * cos(theta);//x
            sphereVertexArray[iv++] = sin(phi) * cos(theta);//z
            sphereVertexArray[iv++] = sin(theta);//y
        }
    }

    Q_ASSERT(iv==bufferSize);

    int in=0;
    for (int iLong=0; iLong<NUMLONG-1; iLong++)
    {
        for (int iLat=0; iLat<NUMLAT; iLat++)
        {
            m_SphereIndicesArray[in++] =  ushort( iLong   *NUMLAT  + iLat);
            m_SphereIndicesArray[in++] =  ushort((iLong+1)*NUMLAT  + iLat);
        }
    }
    Q_ASSERT(in==(NUMLONG-1) * NUMLAT * 2);

    m_vboSphere.create();
    m_vboSphere.bind();
    m_vboSphere.allocate(sphereVertexArray.data(), iv * int(sizeof(GLfloat)));
    m_vboSphere.release();
}


void gl3dView::paintSphere(Vector3d place, double radius, QColor sphereColor, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 mSphere; //is identity
    mSphere.translate(place.xf(), place.yf(), place.zf());
    mSphere.scale(float(radius));

    m_ShaderProgramSurface.bind();
    m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, mSphere);
    m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
    m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix * mSphere);
    if(bLight) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
    else       m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
    m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, sphereColor);

    m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
    m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);

    m_vboSphere.bind();
    m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
    m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    for(int iLong=0; iLong<NUMLONG-1; iLong++)
    {
        glDrawElements(GL_TRIANGLE_STRIP, NUMLAT*2, GL_UNSIGNED_SHORT, m_SphereIndicesArray+iLong*NUMLAT*2);
    }
    m_vboSphere.release();

    m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
    m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
    m_ShaderProgramSurface.release();
}


void gl3dView::glMakeWingGeometry(int iWing, Wing *pWing, Body *pBody)
{
    ushort CHORDPOINTS = ushort(W3dPrefsDlg::chordwiseRes());

    Vector3d N, Pt;
    QVector<Vector3d>NormalA(CHORDPOINTS);
    QVector<Vector3d>NormalB(CHORDPOINTS);
    QVector<Vector3d>PtBotLeft(pWing->m_Surface.count() * CHORDPOINTS);
    QVector<Vector3d>PtBotRight(pWing->m_Surface.count() * CHORDPOINTS);
    QVector<Vector3d>PtTopLeft(pWing->m_Surface.count() * CHORDPOINTS);
    QVector<Vector3d>PtTopRight(pWing->m_Surface.count() * CHORDPOINTS);

    QVector<double>leftV(CHORDPOINTS);
    QVector<double>rightV(CHORDPOINTS);
    double leftU=0.0, rightU=1.0;
    memset(NormalA.data(), 0, sizeof(CHORDPOINTS*sizeof(Vector3d)));
    memset(NormalB.data(), 0, sizeof(CHORDPOINTS*sizeof(Vector3d)));
    //vertices array size:
    // surface:
    //     pWing->NSurfaces
    //     xCHORDPOINTS : from 0 to CHORDPOINTS
    //     x2  for A and B sides
    //     x2  for top and bottom
    // outline
    //     2 points mLA & mLB for leading edge
    //     2 points mTA & mTB for trailing edge
    //
    // x8  : for 3 vertex components, 3 normal components, 2 texture components

    int bufferSize = pWing->m_Surface.count()*CHORDPOINTS*2*2 ;
    bufferSize *= 8;

    QVector<float>wingVertexArray(bufferSize);

    N.set(0.0, 0.0, 0.0);
    int iv=0; //index of vertex components

    //SURFACE
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        //top surface
        pWing->m_Surface.at(j)->getSidePoints(TOPSURFACE, pBody, PtTopLeft.data()+j*CHORDPOINTS, PtTopRight.data()+j*CHORDPOINTS,
                                              NormalA.data(), NormalB.data(), CHORDPOINTS);
        pWing->getTextureUV(j, leftV.data(), rightV.data(), leftU, rightU, CHORDPOINTS);

        //left side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].xf();
            wingVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].yf();
            wingVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].zf();
            wingVertexArray[iv++] = NormalA[l].xf();
            wingVertexArray[iv++] = NormalA[l].yf();
            wingVertexArray[iv++] = NormalA[l].zf();
            wingVertexArray[iv++] = float(leftU);
            wingVertexArray[iv++] = float(leftV[l]);
        }
        //right side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].xf();
            wingVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].yf();
            wingVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].zf();
            wingVertexArray[iv++] = NormalB[l].xf();
            wingVertexArray[iv++] = NormalB[l].yf();
            wingVertexArray[iv++] = NormalB[l].zf();
            wingVertexArray[iv++] = float(rightU);
            wingVertexArray[iv++] = float(rightV[l]);
        }


        //bottom surface
        pWing->m_Surface.at(j)->getSidePoints(BOTSURFACE, pBody, PtBotLeft.data()+j*CHORDPOINTS, PtBotRight.data()+j*CHORDPOINTS,
                                              NormalA.data(), NormalB.data(), CHORDPOINTS);

        //left side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].xf();
            wingVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].yf();
            wingVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].zf();
            wingVertexArray[iv++] = NormalA[l].xf();
            wingVertexArray[iv++] = NormalA[l].yf();
            wingVertexArray[iv++] = NormalA[l].zf();
            wingVertexArray[iv++] = float(1.0-leftU);
            wingVertexArray[iv++] = float(leftV[l]);
        }

        //right side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].xf();
            wingVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].yf();
            wingVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].zf();
            wingVertexArray[iv++] = NormalB[l].xf();
            wingVertexArray[iv++] = NormalB[l].yf();
            wingVertexArray[iv++] = NormalB[l].zf();
            wingVertexArray[iv++] = float(1.0-rightU);
            wingVertexArray[iv++] = float(rightV[l]);
        }
    }


    Q_ASSERT(iv==bufferSize);

    //indices array size:
    //  Top & bottom surfaces
    //      NSurfaces
    //      x (ChordPoints-1)quads
    //      x2 triangles per/quad
    //      x2 top and bottom surfaces
    //      x3 indices/triangle
    //  Tip patches
    //      (CHORDPOINTS-1) quads
    //      x2 triangles per/quad
    //      x2 tip patches
    //      x3 indices/triangle

    m_iWingElems[iWing] =  pWing->m_Surface.count()* (CHORDPOINTS-1) *2 *2 *3
                           + (CHORDPOINTS-1) *2 *2 *3;

    m_WingIndicesArray[iWing].resize(m_iWingElems[iWing]);
    QVector<ushort> &wingIndicesArray = m_WingIndicesArray[iWing];
    int ii = 0;
    ushort nV=0;
    for (int j=0; j<pWing->m_Surface.count(); j++)
    {
        //topsurface
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            Q_ASSERT(ii < m_iWingElems[iWing]);
            //first triangle
            wingIndicesArray[ii]   = nV;
            wingIndicesArray[ii+1] = nV+1;
            wingIndicesArray[ii+2] = nV+CHORDPOINTS;
            //second triangle
            wingIndicesArray[ii+3] = nV+CHORDPOINTS;
            wingIndicesArray[ii+4] = nV+1;
            wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
            ii += 6;
            nV++;
        }
        nV +=CHORDPOINTS+1;

        //botsurface
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            Q_ASSERT(ii < m_iWingElems[iWing]);
            //first triangle
            wingIndicesArray[ii]   = nV;
            wingIndicesArray[ii+1] = nV+1;
            wingIndicesArray[ii+2] = nV+CHORDPOINTS;
            //second triangle
            wingIndicesArray[ii+3] = nV+CHORDPOINTS;
            wingIndicesArray[ii+4] = nV+1;
            wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
            ii += 6;
            nV++;
        }
        nV +=CHORDPOINTS+1;
    }

    //TIP PATCHES
    nV=0;
    for (int j=0; j<pWing->m_Surface.count(); j++)
    {
        if(pWing->m_Surface.at(j)->isTipLeft())
        {
            Q_ASSERT(ii+5 < m_iWingElems[iWing]);
            for (int l=0; l<CHORDPOINTS-1; l++)
            {
                //first triangle
                wingIndicesArray[ii]   = nV;
                wingIndicesArray[ii+1] = nV+1;
                wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;
                //second triangle
                wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
                wingIndicesArray[ii+4] = nV+1;
                wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
                ii += 6;
                nV++; //move one vertex
            }
            nV++; //skip the last vertex
        }

        if(pWing->m_Surface.at(j)->isTipRight())
        {
            if(!pWing->m_Surface.at(j)->isTipLeft()) nV += CHORDPOINTS;

            Q_ASSERT(ii+5 < m_iWingElems[iWing]);
            for (int l=0; l<CHORDPOINTS-1; l++)
            {
                //first triangle
                wingIndicesArray[ii]   = nV;
                wingIndicesArray[ii+1] = nV+1;
                wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;

                //second triangle
                wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
                wingIndicesArray[ii+4] = nV+1;
                wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
                ii += 6;
                nV++; //move one vertex
            }
            nV++; //skip the last vertex;
            nV += CHORDPOINTS; //skip the bottom line of this wing section
        }

        if(pWing->m_Surface.at(j)->isTipLeft())
        {
            nV+= 3*CHORDPOINTS;
        }
        else if(pWing->m_Surface.at(j)->isTipRight())
        {
        }
        else
        {
            nV +=4*CHORDPOINTS;
        }
    }
    Q_ASSERT(ii==m_iWingElems[iWing]);

    if(m_pWingBotLeftTexture[iWing])  delete m_pWingBotLeftTexture[iWing];
    if(m_pWingTopLeftTexture[iWing])  delete m_pWingTopLeftTexture[iWing];
    if(m_pWingBotRightTexture[iWing]) delete m_pWingBotRightTexture[iWing];
    if(m_pWingTopRightTexture[iWing]) delete m_pWingTopRightTexture[iWing];


    QString planeName;
    QString textureName;

    if(s_pMiarex && s_pMiarex->m_pCurPlane)
    {
        planeName = s_pMiarex->m_pCurPlane->planeName();
        switch(pWing->wingType())
        {
            case XFLR5::MAINWING:
                textureName = "wing_";
                break;
            case XFLR5::SECONDWING:
                textureName = "wing2_";
                break;
            case XFLR5::ELEVATOR:
                textureName = "elevator_";
                break;
            case XFLR5::FIN:
                textureName = "fin_";
                break;
            default:
                textureName="wing_";
                break;
        }
    }
    else
    {
        textureName="wing_";
    }

    QImage topLeftTexture;
    getTextureFile(planeName, textureName+"top_left", topLeftTexture);
    m_pWingTopLeftTexture[iWing] = new QOpenGLTexture(topLeftTexture);

    QImage botLeftTexture;
    getTextureFile(planeName, textureName+"bottom_left", botLeftTexture);
    m_pWingBotLeftTexture[iWing] = new QOpenGLTexture(botLeftTexture);

    QImage topRightTexture;
    getTextureFile(planeName, textureName+"top_right", topRightTexture);
    m_pWingTopRightTexture[iWing] = new QOpenGLTexture(topRightTexture);

    QImage botRightTexture;
    getTextureFile(planeName, textureName+"bottom_right", botRightTexture);
    m_pWingBotRightTexture[iWing] = new QOpenGLTexture(botRightTexture);

    m_vboWingSurface[iWing].destroy();
    m_vboWingSurface[iWing].create();
    m_vboWingSurface[iWing].bind();
    m_vboWingSurface[iWing].allocate(wingVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboWingSurface[iWing].release();


    //make OUTLINE
    //vertices array size:
    // surface:
    //     pWing->NSurfaces
    //     x(CHORDPOINTS-1)*2 : segments from i to i+1, times two vertices
    //                          so that we can make only one call to GL_LINES later on
    //     x2  for A and B sides
    //     x2  for top and bottom
    // flaps
    m_iWingOutlinePoints[iWing]  = pWing->m_Surface.count()*(CHORDPOINTS-1)*2*2*2;

    // outline
    //     2 points mLA & mLB for leading edge
    //     2 points mTA & mTB for trailing edge
    m_iWingOutlinePoints[iWing] += pWing->m_Surface.size()*2*2;

    //TE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->m_Surface[j]->m_pFoilA;
        Foil const *pFoilB = pWing->m_Surface[j]->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
            m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
        }
    }
    //LE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->m_Surface[j]->m_pFoilA;
        Foil const *pFoilB = pWing->m_Surface[j]->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
        {
            m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
        }
    }

    // x3  : for 3 vertex components
    QVector<float> wingOutlineVertexArray(m_iWingOutlinePoints[iWing]*3, 0);

    iv=0; //index of vertex components

    //SECTIONS OUTLINE
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        //top surface
        //left side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].xf();
            wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].yf();
            wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].zf();
            wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l+1].xf();
            wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l+1].yf();
            wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l+1].zf();
        }
        //right side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].xf();
            wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].yf();
            wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].zf();
            wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l+1].xf();
            wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l+1].yf();
            wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l+1].zf();
        }


        //bottom surface

        //left side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].xf();
            wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].yf();
            wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].zf();
            wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l+1].xf();
            wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l+1].yf();
            wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l+1].zf();
        }

        //right side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].xf();
            wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].yf();
            wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].zf();
            wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l+1].xf();
            wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l+1].yf();
            wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l+1].zf();
        }

        //Leading edge
        wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+0].xf();
        wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+0].yf();
        wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+0].zf();
        wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+0].xf();
        wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+0].yf();
        wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+0].zf();

        //trailing edge
        wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS + CHORDPOINTS-1].xf();
        wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS + CHORDPOINTS-1].yf();
        wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS + CHORDPOINTS-1].zf();
        wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS + CHORDPOINTS-1].xf();
        wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS + CHORDPOINTS-1].yf();
        wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS + CHORDPOINTS-1].zf();
    }

    //TE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const *pSurf =  pWing->m_Surface[j];
        Foil const *pFoilA =pSurf->m_pFoilA;
        Foil const *pFoilB =pSurf->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
           pSurf->getSurfacePoint(pSurf->m_pFoilA->m_TEXHinge/100.0,
                                  pFoilA->m_TEXHinge/100.0,
                                  0.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

           pSurf->getSurfacePoint(pSurf->m_pFoilB->m_TEXHinge/100.0,
                                  pFoilB->m_TEXHinge/100.0,
                                  1.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();


           pSurf->getSurfacePoint(pSurf->m_pFoilA->m_TEXHinge/100.0,
                                  pFoilA->m_TEXHinge/100.0,
                                  0.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();


           pSurf->getSurfacePoint(pSurf->m_pFoilB->m_TEXHinge/100.0,
                                  pFoilB->m_TEXHinge/100.0,
                                  1.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();
        }
    }
    //LE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const *pSurf =  pWing->m_Surface[j];
        Foil const *pFoilA = pSurf->m_pFoilA;
        Foil const *pFoilB = pSurf->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
        {
            pSurf->getSurfacePoint(pFoilA->m_LEXHinge/100.0,
                                   pFoilA->m_LEXHinge/100.0,
                                   0.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

            pSurf->getSurfacePoint(pFoilB->m_LEXHinge/100.0,
                                   pFoilB->m_LEXHinge/100.0,
                                   1.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

            pSurf->getSurfacePoint(pFoilA->m_LEXHinge/100.0,
                                   pFoilA->m_LEXHinge/100.0,
                                   0.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

            pSurf->getSurfacePoint(pFoilB->m_LEXHinge/100.0,
                                   pFoilB->m_LEXHinge/100.0,
                                   1.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();
        }
    }

    Q_ASSERT(iv==m_iWingOutlinePoints[iWing] * 3);

    m_vboWingOutline[iWing].destroy();
    m_vboWingOutline[iWing].create();
    m_vboWingOutline[iWing].bind();
    m_vboWingOutline[iWing].allocate(wingOutlineVertexArray.data(), m_iWingOutlinePoints[iWing] * 3 * int(sizeof(GLfloat)));
    m_vboWingOutline[iWing].release();
}


void gl3dView::getTextureFile(QString planeName, QString surfaceName, QImage &textureImage)
{
    QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
    QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator()+surfaceName;

    textureImage =  QImage(QString(texturePath+".png"));
    if(textureImage.isNull())
    {
        textureImage  = QImage(QString(texturePath+".jpg"));
        if(textureImage.isNull())
        {
            textureImage  = QImage(QString(texturePath+".jpeg"));
            if(textureImage.isNull())
            {
                textureImage = QImage(QString(":/default_textures/"+surfaceName+".png"));
            }
        }
    }
}


/** Default mesh, if no polar has been defined */
void gl3dView::glMakeWingEditMesh(QOpenGLBuffer &vbo, Wing *pWing)
{
    int l,k;
    //not necessarily the same Nx for all surfaces, so we need to count the quad panels
    int bufferSize = 0;
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface *pSurf = pWing->m_Surface[j];
        //tip patches
        if(pSurf->isTipLeft())  bufferSize += (pSurf->NXPanels());
        if(pSurf->isTipRight()) bufferSize += (pSurf->NXPanels());

        // top and bottom surfaces
        bufferSize += pSurf->NXPanels()*2 * (pSurf->NYPanels());
    }
    bufferSize *=2;    // 2 triangles/quad
    bufferSize *=3;    // 3 vertex for each triangle
    bufferSize *=3;    // 3 components for each node

    QVector<float> meshVertexArray(bufferSize);

    int iv=0;

    Vector3d A,B,C,D;

    //tip patches
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface *pSurf = pWing->m_Surface[j];
        if(pSurf->isTipLeft())
        {
            for (l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(0,l,TOPSURFACE);
                A = pSurf->TA;
                B = pSurf->LA;
                pSurf->getPanel(0,l,BOTSURFACE);
                C = pSurf->LA;
                D = pSurf->TA;

                //first triangle
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
                meshVertexArray[iv++] = B.xf();
                meshVertexArray[iv++] = B.yf();
                meshVertexArray[iv++] = B.zf();
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();

                //second triangle
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
                meshVertexArray[iv++] = D.xf();
                meshVertexArray[iv++] = D.yf();
                meshVertexArray[iv++] = D.zf();
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
            }
        }
        if(pSurf->isTipRight())
        {
            for (l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(pSurf->NYPanels()-1,l,TOPSURFACE);
                A = pSurf->TB;
                B = pSurf->LB;
                pSurf->getPanel(pSurf->NYPanels()-1,l,BOTSURFACE);
                C = pSurf->LB;
                D = pSurf->TB;

                //first triangle
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
                meshVertexArray[iv++] = B.xf();
                meshVertexArray[iv++] = B.yf();
                meshVertexArray[iv++] = B.zf();
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();

                //second triangle
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
                meshVertexArray[iv++] = D.xf();
                meshVertexArray[iv++] = D.yf();
                meshVertexArray[iv++] = D.zf();
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
            }
        }
    }

    //background surface
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface *pSurf = pWing->m_Surface[j];
        for(k=0; k<pSurf->NYPanels(); k++)
        {
            for (l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(k,l,TOPSURFACE);

                // first triangle
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();
                meshVertexArray[iv++] = pSurf->TA.xf();
                meshVertexArray[iv++] = pSurf->TA.yf();
                meshVertexArray[iv++] = pSurf->TA.zf();
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();

                //second triangle
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();
                meshVertexArray[iv++] = pSurf->LB.xf();
                meshVertexArray[iv++] = pSurf->LB.yf();
                meshVertexArray[iv++] = pSurf->LB.zf();
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();
            }

            for (l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(k,l,BOTSURFACE);
                //first triangle
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();
                meshVertexArray[iv++] = pSurf->TA.xf();
                meshVertexArray[iv++] = pSurf->TA.yf();
                meshVertexArray[iv++] = pSurf->TA.zf();
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();

                //second triangle
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();
                meshVertexArray[iv++] = pSurf->LB.xf();
                meshVertexArray[iv++] = pSurf->LB.yf();
                meshVertexArray[iv++] = pSurf->LB.zf();
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();
            }
        }
    }


    Q_ASSERT(iv==bufferSize);


    Q_ASSERT(iv==bufferSize);

    //    m_iWingMeshElems = ii/3;
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dView::glMakeBodyFrameHighlight(Body *pBody, Vector3d bodyPos, int iFrame)
{
    //    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();
    int k;
    Vector3d Point;
    double hinc, u, v;
    if(iFrame<0) return;

    Frame *pFrame = pBody->frame(iFrame);
    //    xinc = 0.1;
    hinc = 1.0/double(NHOOOP-1);

    int bufferSize = 0;
    QVector<float>pHighlightVertexArray;

    m_nHighlightLines = 2; // left and right - could make one instead

    //create 3D Splines or Lines to overlay on the body
    int iv = 0;

    if(pBody->isFlatPanelType())
    {
        m_HighlightLineSize = pFrame->pointCount();
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
        pHighlightVertexArray.resize(bufferSize);
        for (k=0; k<pFrame->pointCount();k++)
        {
            pHighlightVertexArray[iv++] = pFrame->m_Position.xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].yf();
            pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].zf()+bodyPos.zf();
        }

        for (k=0; k<pFrame->pointCount();k++)
        {
            pHighlightVertexArray[iv++] =  pFrame->m_Position.xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = -pFrame->m_CtrlPoint[k].yf();
            pHighlightVertexArray[iv++] =  pFrame->m_CtrlPoint[k].zf()+bodyPos.zf();
        }
    }
    else if(pBody->isSplineType())
    {
        m_HighlightLineSize = NHOOOP;
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
        pHighlightVertexArray.resize(bufferSize);

        if(pBody->activeFrame())
        {
            u = pBody->getu(pFrame->m_Position.x);
            v = 0.0;
            for (k=0; k<NHOOOP; k++)
            {
                pBody->getPoint(u,v,true, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v += hinc;
            }

            v = 1.0;
            for (k=0; k<NHOOOP; k++)
            {
                pBody->getPoint(u,v,false, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v -= hinc;
            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    m_vboHighlight.destroy();
    m_vboHighlight.create();
    m_vboHighlight.bind();
    m_vboHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboHighlight.release();
}


void gl3dView::onRotationIncrement()
{
    if(m_iTransitionInc>=30)
    {
        m_pTransitionTimer->stop();
        delete m_pTransitionTimer;
        m_pTransitionTimer = nullptr;
        return;
    }
    for(int iq=0; iq<16; iq++)
    {
        m_ArcBall.ab_quat[iq] = ab_old[iq] + float(m_iTransitionInc)/29.0f * (ab_new[iq]-ab_old[iq]);
    }
    reset3DRotationCenter();
    update();
    m_iTransitionInc++;
}


void gl3dView::startRotationTimer()
{
    if(W3dPrefsDlg::s_bAnimateTransitions)
    {
        m_iTransitionInc = 0;
        if(m_pTransitionTimer) delete m_pTransitionTimer;
        m_pTransitionTimer = new QTimer(this);
        connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onRotationIncrement()));
        m_pTransitionTimer->start(10);//33 ms x 30 times is approximately one second
    }
    else
    {
        reset3DRotationCenter();
        update();
    }
}


void gl3dView::startTranslationTimer(Vector3d PP)
{
    glInverseMatrix();
    if(W3dPrefsDlg::s_bAnimateTransitions)
    {
        m_transIncrement.x = (-PP.x -m_glRotCenter.x)/30.0;
        m_transIncrement.y = (-PP.y -m_glRotCenter.y)/30.0;
        m_transIncrement.z = (-PP.z -m_glRotCenter.z)/30.0;

        m_iTransitionInc = 0;
        if(m_pTransitionTimer) delete m_pTransitionTimer;
        m_pTransitionTimer = new QTimer(this);
        connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onTranslationIncrement()));
        m_pTransitionTimer->start(10);//33 ms x 30 times is approximately one second
    }
    else
    {
        m_glRotCenter -= PP;
        m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
        m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
        m_glViewportTrans.z=   (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);

        update();
    }
}


void gl3dView::onTranslationIncrement()
{
    if(m_iTransitionInc>=30)
    {
        m_pTransitionTimer->stop();
        delete m_pTransitionTimer;
        m_pTransitionTimer = nullptr;
        return;
    }

    m_glRotCenter +=m_transIncrement;
    m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
    m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
    m_glViewportTrans.z=   (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);


    update();
    m_iTransitionInc++;
}


void gl3dView::onResetIncrement()
{
    if(m_iTransitionInc>=30)
    {
        m_pTransitionTimer->stop();
        delete m_pTransitionTimer;
        m_pTransitionTimer = nullptr;
        return;
    }

    m_glScaled += m_glScaleIncrement;
    m_glViewportTrans += m_transIncrement;

    reset3DRotationCenter();
    update();
    m_iTransitionInc++;
}


/**
 * Sets an automatic scale for the wing or plane in the 3D view, depending on wing span.
 */
void gl3dView::set3DScale(double length)
{
    if(length>0.0) m_glScaledRef = (4./5.*2.0/length);
    m_glScaled = m_glScaledRef;
    m_glViewportTrans.set(0.0, 0.0, 0.0);
    reset3DRotationCenter();
    update();
}


void gl3dView::startResetTimer(double length)
{
    if(W3dPrefsDlg::s_bAnimateTransitions)
    {
        m_iTransitionInc = 0;

        m_glScaleIncrement = (m_glScaledRef-m_glScaled)/30.0;
        m_transIncrement = (Vector3d(0.0,0.0,0.0)-m_glViewportTrans)/30.0;

        if(m_pTransitionTimer) delete m_pTransitionTimer;
        m_pTransitionTimer = new QTimer(this);
        connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onResetIncrement()));
        m_pTransitionTimer->start(10);//33 ms x 30 times is approximately one second
    }
    else
    {
        set3DScale(length);
        update();
    }
}


/** Default mesh, if no polar has been defined */
void gl3dView::glMakeEditBodyMesh(Body *pBody, Vector3d BodyPosition)
{
    if(!pBody) return;
    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();
    int nx=0, nh=0;
    Vector3d Pt;
    Vector3d P1, P2, P3, P4, PStart, PEnd;
    QVector<float>meshVertexArray;
    int bufferSize = 0;
    m_iBodyMeshLines = 0;

    float dx = BodyPosition.xf();
    float dy = BodyPosition.yf();
    float dz = BodyPosition.zf();

    int iv=0;

    if(pBody->isFlatPanelType()) //LINES
    {
        bufferSize = 0;
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            for (int k=0; k<pBody->sideLineCount()-1;k++)
            {
                for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
                {
                    bufferSize += 6;
                }
                for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
                {
                    bufferSize += 6;
                }
            }
        }
        bufferSize *=2;

        meshVertexArray.resize(bufferSize);

        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            for (int k=0; k<pBody->sideLineCount()-1;k++)
            {
                P1 = pBody->frame(j)->m_CtrlPoint[k];       P1.x = pBody->frame(j)->m_Position.x;
                P2 = pBody->frame(j+1)->m_CtrlPoint[k];     P2.x = pBody->frame(j+1)->m_Position.x;
                P3 = pBody->frame(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frame(j+1)->m_Position.x;
                P4 = pBody->frame(j)->m_CtrlPoint[k+1];     P4.x = pBody->frame(j)->m_Position.x;

                P1.x+=double(dx);   P2.x+=double(dx);   P3.x+=double(dx);   P4.x+=double(dx);
                P1.y+=double(dy);   P2.y+=double(dy);   P3.y+=double(dy);   P4.y+=double(dy);
                P1.z+=double(dz);   P2.z+=double(dz);   P3.z+=double(dz);   P4.z+=double(dz);

                //left side panels
                for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
                {
                    PStart = P1 + (P2-P1) * double(jp)/double(pBody->m_xPanels[j]);
                    PEnd   = P4 + (P3-P4) * double(jp)/double(pBody->m_xPanels[j]);
                    meshVertexArray[iv++] = PStart.xf();
                    meshVertexArray[iv++] = PStart.yf();
                    meshVertexArray[iv++] = PStart.zf();
                    meshVertexArray[iv++] = PEnd.xf();
                    meshVertexArray[iv++] = PEnd.yf();
                    meshVertexArray[iv++] = PEnd.zf();
                    m_iBodyMeshLines++;
                }
                for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
                {
                    PStart = P1 + (P4-P1) * double(kp)/double(pBody->m_hPanels[k]);
                    PEnd   = P2 + (P3-P2) * double(kp)/double(pBody->m_hPanels[k]);
                    meshVertexArray[iv++] = PStart.xf();
                    meshVertexArray[iv++] = PStart.yf();
                    meshVertexArray[iv++] = PStart.zf();
                    meshVertexArray[iv++] = PEnd.xf();
                    meshVertexArray[iv++] = PEnd.yf();
                    meshVertexArray[iv++] = PEnd.zf();
                    m_iBodyMeshLines++;
                }

                //right side panels
                for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
                {
                    PStart = P1 + (P2-P1) * double(jp)/double(pBody->m_xPanels[j]);
                    PEnd   = P4 + (P3-P4) * double(jp)/double(pBody->m_xPanels[j]);
                    meshVertexArray[iv++] =  PStart.xf();
                    meshVertexArray[iv++] = -PStart.yf();
                    meshVertexArray[iv++] =  PStart.zf();
                    meshVertexArray[iv++] =  PEnd.xf();
                    meshVertexArray[iv++] = -PEnd.yf();
                    meshVertexArray[iv++] =  PEnd.zf();
                    m_iBodyMeshLines++;
                }
                for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
                {
                    PStart = P1 + (P4-P1) * double(kp)/double(pBody->m_hPanels[k]);
                    PEnd   = P2 + (P3-P2) * double(kp)/double(pBody->m_hPanels[k]);
                    meshVertexArray[iv++] =  PStart.xf();
                    meshVertexArray[iv++] = -PStart.yf();
                    meshVertexArray[iv++] =  PStart.zf();
                    meshVertexArray[iv++] =  PEnd.xf();
                    meshVertexArray[iv++] = -PEnd.yf();
                    meshVertexArray[iv++] =  PEnd.zf();
                    m_iBodyMeshLines++;
                }
            }
        }
        Q_ASSERT(m_iBodyMeshLines*6==bufferSize);
        Q_ASSERT(iv==bufferSize);
    }
    else if(pBody->isSplineType()) //NURBS
    {
        nx = pBody->m_nxPanels;
        nh = pBody->m_nhPanels;

        bufferSize = 0;
        bufferSize += nh * NXXXX; // nh longitudinal lines
        bufferSize += nx * NHOOOP; // nx hoop line
        bufferSize *= 2;       // two sides
        bufferSize *= 3;       // 3 components/vertex;

        meshVertexArray.resize(bufferSize);

        pBody->setPanelPos();
        //x-lines;
        for (int l=0; l<nh; l++)
        {
            double v = double(l)/double(nh-1);
            for (int k=0; k<NXXXX; k++)
            {
                double u = double(k)/double(NXXXX-1);
                pBody->getPoint(u,  v, true, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }
        for (int l=0; l<nh; l++)
        {
            double v = double(l)/double(nh-1);
            for (int k=0; k<NXXXX; k++)
            {
                double u = double(k)/double(NXXXX-1);
                pBody->getPoint(u,  v, false, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }

        //hoop lines;
        for (int k=0; k<nx; k++)
        {
            double uk = pBody->m_XPanelPos[k];
            for (int l=0; l<NHOOOP; l++)
            {
                double v = double(l)/double(NHOOOP-1);
                pBody->getPoint(uk,  v, true, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }
        for (int k=0; k<nx; k++)
        {
            double uk = pBody->m_XPanelPos[k];
            for (int l=0; l<NHOOOP; l++)
            {
                double v = double(l)/double(NHOOOP-1);
                pBody->getPoint(uk,  v, false, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    m_vboEditBodyMesh.destroy();
    m_vboEditBodyMesh.create();
    m_vboEditBodyMesh.bind();
    m_vboEditBodyMesh.allocate(meshVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboEditBodyMesh.release();
}


/** note: glLineStipple is deprecated since OpenGL 3.1 */
void GLLineStipple(int style)
{
    if     (style == Qt::DashLine)       glLineStipple (1, 0xCFCF);
    else if(style == Qt::DotLine)        glLineStipple (1, 0x6666);
    else if(style == Qt::DashDotLine)    glLineStipple (1, 0xFF18);
    else if(style == Qt::DashDotDotLine) glLineStipple (1, 0x7E66);
    else                                 glLineStipple (1, 0xFFFF);
}


/**
 * @brief since glLineStipple is deprecated, make an array of simple lines for all 3 axis
 */
void gl3dView::glMakeAxis()
{
    QVector<GLfloat>axisVertexArray;

    int iv = 0;

    float axisLength = fabsf(x_axis[3]-x_axis[0]);

    int lineStyle = 3;
    switch(lineStyle)
    {
        case 1:
        {
            // dash
            iv = 0;
            int nVertices = 75;
            float incLine  = axisLength/float(nVertices-1) * 0.7f * 2.0f;
            float incSpace = axisLength/float(nVertices-1) * 0.3f * 2.0f;
            float s = 0.0f;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incLine;
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incSpace;
                iv+=6;
            }

            s=0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incLine;
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incSpace;
                iv+=6;
            }
            s=0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incLine;
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incSpace;
                iv+=6;
            }

            break;
        }
        case 2:
        {
            //dot
            iv = 0;
            int nVertices = 300;
            float incLine  = axisLength/float(nVertices-1) * 0.1f * 2.0f;
            float incSpace = axisLength/float(nVertices-1) * 0.9f * 2.0f;
            float s = 0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incLine;
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incSpace;
                iv+=6;
            }

            s=0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incLine;
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incSpace;
                iv+=6;
            }
            s=0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incLine;
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incSpace;
                iv+=6;
            }

            break;
        }
        case 3:
        {
            //dash-dot
            iv = 0;
            int nVertices = 50;
            float incLine1 = axisLength/float(nVertices-1) * 0.5f * 2.0f;
            float incLine2 = axisLength/float(nVertices-1) * 0.1f * 2.0f;
            float incSpace = axisLength/float(nVertices-1) * 0.2f * 2.0f;
            float s = 0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incLine1;
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incSpace;
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incLine2;
                axisVertexArray.push_back(x_axis[0]+s);
                axisVertexArray.push_back(x_axis[1]);
                axisVertexArray.push_back(x_axis[2]);
                s+=incSpace;
                iv+=12;
            }

            s=0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incLine1;
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incSpace;
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incLine2;
                axisVertexArray.push_back(y_axis[0]);
                axisVertexArray.push_back(y_axis[1]+s);
                axisVertexArray.push_back(y_axis[2]);
                s+=incSpace;
                iv+=12;
            }
            s=0.0;
            for(int jv=0; jv<nVertices/2; jv++)
            {
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incLine1;
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incSpace;
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incLine2;
                axisVertexArray.push_back(z_axis[0]);
                axisVertexArray.push_back(z_axis[1]);
                axisVertexArray.push_back(z_axis[2]+s);
                s+=incSpace;
                iv+=12;
            }

            break;
        }
/*        case 4:
        {
            //dash-dot-dot
            break;
        }*/
        default:
        {
            // solid
            // 3 axis x 3 solid lines x 2 vertices * 3 coordinates
            iv = 0;
            for(; iv<18; iv++)    axisVertexArray.push_back(x_axis[iv]);
            for(; iv<36; iv++)    axisVertexArray.push_back(y_axis[iv]);
            for(; iv<54; iv++)    axisVertexArray.push_back(z_axis[iv]);
            break;
        }
    }

    // add arrows
    for(int i=6; i<18; i++) axisVertexArray.push_back(x_axis[i]);
    for(int i=6; i<18; i++) axisVertexArray.push_back(y_axis[i]);
    for(int i=6; i<18; i++) axisVertexArray.push_back(z_axis[i]);

    m_vboAxis.destroy();
    m_vboAxis.create();
    m_vboAxis.bind();
    m_vboAxis.allocate(axisVertexArray.data(), axisVertexArray.size() * int(sizeof(GLfloat)));
    m_vboAxis.release();
}
