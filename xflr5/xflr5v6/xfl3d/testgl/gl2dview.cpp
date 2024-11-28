/****************************************************************************

    Xfl3d
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <cmath>

#include <QApplication>
#include <QOpenGLPaintDevice>
#include <QMatrix4x4>

#include "gl2dview.h"

#include <xfl3d/globals/gl_globals.h>
#include <xfl3d/globals/w3dprefs.h>
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xflcore/constants.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/wt_globals.h>



gl2dView::gl2dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setFocusPolicy(Qt::WheelFocus);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);

    m_bDynTranslation = false;
    m_bDynScaling     = false;
    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));

    m_nRoots = 0;

    m_Scale = 1.0f;
}


void gl2dView::onDynamicIncrement()
{
    if(m_bDynTranslation)
    {
        double dist = sqrt(m_Trans.x()*m_Trans.x()+m_Trans.y()*m_Trans.y())*m_Scale;
        if(dist<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_ptOffset += m_Trans/10.0;

        m_Trans *= (1.0-W3dPrefs::spinDamping());
    }

    if(m_bDynScaling)
    {
        if(abs(m_ZoomFactor)<10)
        {
            stopDynamicTimer();
            update();
            return;
        }

        double scalefactor(1.0-DisplayOptions::scaleFactor()/3.0 * m_ZoomFactor/120);

        m_Scale *= scalefactor;
        m_ZoomFactor *= (1.0-W3dPrefs::spinDamping());
    }

    update();
}


void gl2dView::startDynamicTimer()
{
    m_DynTimer.start(17);
//    setMouseTracking(false);
}


void gl2dView::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
    }
    m_bDynTranslation  = m_bDynScaling = false;
//    setMouseTracking(true);
}


/* triangle strip */
void gl2dView::makeQuad()
{
    QVector<GLfloat> QuadVertexArray(12, 0);

    int iv = 0;
    QuadVertexArray[iv++] = 0.0f;
    QuadVertexArray[iv++] = 0.0f;

    QuadVertexArray[iv++] = m_rectView.left();
    QuadVertexArray[iv++] = m_rectView.top();

    QuadVertexArray[iv++] = m_rectView.right();
    QuadVertexArray[iv++] = m_rectView.top();

    QuadVertexArray[iv++] = m_rectView.right();
    QuadVertexArray[iv++] = m_rectView.bottom();

    QuadVertexArray[iv++] = m_rectView.left();
    QuadVertexArray[iv++] = m_rectView.bottom();

    QuadVertexArray[iv++] = m_rectView.left();
    QuadVertexArray[iv++] = m_rectView.top();

    Q_ASSERT(iv==QuadVertexArray.size());

    m_vboQuad.destroy();
    m_vboQuad.create();
    m_vboQuad.bind();
    m_vboQuad.allocate(QuadVertexArray.data(), QuadVertexArray.size() * sizeof(GLfloat));
    m_vboQuad.release();
}


void gl2dView::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(windowFlags()&Qt::FramelessWindowHint)
            {
                setWindowFlags(Qt::Window);
                show(); //Note: This function calls setParent() when changing the flags for a window, causing the widget to be hidden. You must call show() to make the widget visible again..
                return;
            }
            break;
        }
        case Qt::Key_R:
        {
            stopDynamicTimer();
            m_ptOffset = defaultOffset();
            m_Scale = 1.0f;
            update();
            break;
        }
    }

    QOpenGLWidget::keyPressEvent(pEvent);
}


void gl2dView::showEvent(QShowEvent *pEvent)
{
    setFocus();
    m_ptOffset = defaultOffset();
    QOpenGLWidget::showEvent(pEvent);
}


void gl2dView::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usable on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(W3dPrefs::bSpinAnimation() && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {
        float zoomfactor(1.0f);
        if(pEvent->angleDelta().y()>0) zoomfactor = 1.0/(1.0+DisplayOptions::scaleFactor());
        else                           zoomfactor = 1.0+DisplayOptions::scaleFactor();

        m_Scale *= zoomfactor;

        int a = rect().center().x();
        int b = rect().center().y();
        m_ptOffset.rx() = a + (m_ptOffset.x()-a);
        m_ptOffset.ry() = b + (m_ptOffset.y()-b);
        update();
    }

    pEvent->accept();
}


void gl2dView::initializeGL()
{
    QString strange, vsrc, gsrc, fsrc;

    //--------- setup the shader to paint stippled thick lines -----------
    vsrc = ":/shaders/line/line_VS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Line vertex shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }


    gsrc = ":/shaders/line/line_GS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Geometry, gsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Line geometry shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }



    fsrc = ":/shaders/line/line_FS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Stipple fragment shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadLine.link();
    m_shadLine.bind();
    {
        m_locLine.m_attrVertex    = m_shadLine.attributeLocation("vertexPosition_modelSpace");
        m_locLine.m_attrColor = m_shadLine.attributeLocation("vertexColor");
        m_locLine.m_vmMatrix     = m_shadLine.uniformLocation("vmMatrix");
        m_locLine.m_pvmMatrix    = m_shadLine.uniformLocation("pvmMatrix");
        m_locLine.m_HasUniColor  = m_shadLine.uniformLocation("HasUniColor");
        m_locLine.m_UniColor     = m_shadLine.uniformLocation("UniformColor");
        m_locLine.m_ClipPlane    = m_shadLine.uniformLocation("clipPlane0");
        m_locLine.m_Thickness    = m_shadLine.uniformLocation("Thickness");
        m_locLine.m_Viewport     = m_shadLine.uniformLocation("Viewport");
        m_locLine.m_Pattern      = m_shadLine.uniformLocation("pattern");
        m_locLine.m_nPatterns    = m_shadLine.uniformLocation("nPatterns");
        GLint nPatterns = 300; // number of patterns per unit projected length (viewport half width = 1)
        m_shadLine.setUniformValue(m_locLine.m_nPatterns, nPatterns);
    }
    m_shadLine.release();

    vsrc = ":/shaders/point/point_VS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point vertex shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    gsrc = ":/shaders/point/point_GS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Geometry, gsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point geometry shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/point/point_FS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point fragment shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadPoint.link();
    m_shadPoint.bind();
    {
        m_locPoint.m_attrVertex = m_shadPoint.attributeLocation("vertexPosition_modelSpace");
        m_locPoint.m_State      = m_shadPoint.attributeLocation("PointState");
        m_locPoint.m_vmMatrix   = m_shadPoint.uniformLocation("vmMatrix");
        m_locPoint.m_pvmMatrix  = m_shadPoint.uniformLocation("pvmMatrix");
        m_locPoint.m_ClipPlane  = m_shadPoint.uniformLocation("clipPlane0");
        m_locPoint.m_UniColor   = m_shadPoint.uniformLocation("Color");
        m_locPoint.m_Thickness  = m_shadPoint.uniformLocation("Thickness");
        m_locPoint.m_Shape      = m_shadPoint.uniformLocation("Shape");
        m_locPoint.m_Viewport   = m_shadPoint.uniformLocation("Viewport");
        m_locPoint.m_Light      = m_shadPoint.uniformLocation("LightOn");
        m_locPoint.m_TwoSided   = m_shadPoint.uniformLocation("TwoSided");
    }
    m_shadPoint.release();
    makeQuad();
}


void gl2dView::resizeGL(int width, int height)
{
    QOpenGLWidget::resizeGL(width, height);

//    int side = std::min(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);

    double w = double(width);
    double h = double(height);
    double s = 1.0;

    if(w>h)	m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);
}



void gl2dView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(W3dPrefs::bSpinAnimation())
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<300 && !m_PressedPoint.isNull())
        {
            if(pEvent->button()==Qt::LeftButton)
            {
                m_Trans = (pEvent->pos() - m_PressedPoint)/m_Scale;
                startDynamicTimer();
                m_bDynTranslation = true;
            }
        }
    }
    QApplication::restoreOverrideCursor();
}


void gl2dView::mousePressEvent(QMouseEvent *pEvent)
{
    m_LastPoint = pEvent->pos();
    m_PressedPoint = m_LastPoint;

    stopDynamicTimer();
    m_MoveTime.restart();
    if(hasFocus()) QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}


void gl2dView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!hasFocus()) setFocus();

    QPoint point = pEvent->pos();

    if(pEvent->buttons() & Qt::LeftButton)
    {
        //translate the view

        QPoint delta = point - m_LastPoint;
        m_ptOffset.setX(m_ptOffset.x() + double(delta.x())/m_Scale);
        m_ptOffset.setY(m_ptOffset.y() + double(delta.y())/m_Scale);

        m_LastPoint.rx() = point.x();
        m_LastPoint.ry() = point.y();

        update();
        return;
    }
    else if(pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        float zoomfactor=1.0f;

        if(point.y()-m_LastPoint.y()<0) zoomfactor = 1.0/(1.0+DisplayOptions::scaleFactor());
        else                            zoomfactor = 1.0+DisplayOptions::scaleFactor();

        m_Scale *= zoomfactor;
        update();
    }
    m_LastPoint = point;

    pEvent->accept();
}


void gl2dView::screenToViewport(QPoint const &point, QVector2D &real) const
{
    double h2 = double(geometry().height()) /2.0;
    double w2 = double(geometry().width())  /2.0;

    real.setX( (double(point.x()) - w2) / w2);
    real.setY(-(double(point.y()) - h2) / w2);
}


void gl2dView::screenToWorld(QPoint const &screenpt, QVector2D &pt)
{
    QMatrix4x4 m_matView, m;
    QVector4D in, out;
    QVector2D real;
    double w = m_rectView.width();
    QVector2D off((-m_ptOffset.x())/width()*w, m_ptOffset.y()/width()*w);

    screenToViewport(screenpt, real);
    in.setX(float(real.x()));
    in.setY(float(real.y()));
    in.setZ(0.0f);
    in.setW(1.0f);

    m_matView.scale(m_Scale, m_Scale, m_Scale);
    m_matView.translate(-off.x(), -off.y(), 0.0f);

    bool bInverted=false;
    QMatrix4x4 vmMatrix = m_matView;
    m = vmMatrix.inverted(&bInverted);
    out = m * in;

    if(fabs(double(out[3]))>PRECISION)
    {
        pt.setX(double(out[0]/out[3]));
        pt.setY(double(out[1]/out[3]));
    }
    else
    {
        pt.setX(double(out[0]));
        pt.setY(double(out[0]));
    }
}


void gl2dView::paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadPoint.bind();
    {
        // iShape 0: Pentagon, 1: Icosahedron, 2: Cube
        m_shadPoint.setUniformValue(m_locPoint.m_Shape, iShape);
        m_shadPoint.setUniformValue(m_locPoint.m_UniColor, clr); // only used if vertex state is invalid 0< or >1
        m_shadPoint.setUniformValue(m_locPoint.m_Thickness, width);
        if(bLight)m_shadPoint.setUniformValue(m_locPoint.m_Light, 1);
        else      m_shadPoint.setUniformValue(m_locPoint.m_Light, 0);

        if(vbo.bind())
        {
            m_shadPoint.enableAttributeArray(m_locPoint.m_attrVertex);
            m_shadPoint.setAttributeBuffer(m_locPoint.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(float));
            m_shadPoint.enableAttributeArray(m_locPoint.m_State);
            m_shadPoint.setAttributeBuffer(m_locPoint.m_State, GL_FLOAT, 3*sizeof(float), 1, stride*sizeof(float));
            int npts = vbo.size()/stride/int(sizeof(float));
            glDrawArrays(GL_POINTS, 0, npts);// 4 vertices defined but only 3 are used
            m_shadPoint.disableAttributeArray(m_locPoint.m_attrVertex);
            m_shadPoint.disableAttributeArray(m_locPoint.m_State);
        }
        vbo.release();
    }
    m_shadPoint.release();
}



void gl2dView::paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh)
{
    paintSegments(vbo, ls.m_Color, ls.m_Width, ls.m_Stipple, bHigh);
}


void gl2dView::paintSegments(QOpenGLBuffer &vbo, QColor const &clr, float thickness, Line::enumLineStipple stip, bool bHigh)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    int stride = 4; // 3 position components
    m_shadLine.bind();
    {
        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 4, stride*sizeof(GLfloat));

            int nSegs = vbo.size()/2/stride/int(sizeof(float));

            if(bHigh)
            {
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));
                m_shadLine.setUniformValue(m_locLine.m_UniColor, Qt::red);

                m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness+2);
            }
            else
            {
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(stip));
                m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);
                m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness);
            }

            glDrawArrays(GL_LINES, 0, nSegs*2);// 4 vertices defined but only 3 are used
            glDisable(GL_LINE_STIPPLE);
        }
        vbo.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}



