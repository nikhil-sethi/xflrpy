/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QApplication>
#include <QGridLayout>
#include <QLabel>

#include "gl2dfractal.h"

#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xfl3d/views/gl3dview.h> // for the static variables
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>

int gl2dFractal:: s_MaxIter = 32;
float gl2dFractal::s_MaxLength = 10;

float gl2dFractal::s_ScaleFactor = 0.06f;

gl2dFractal::gl2dFractal(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setWindowTitle("Mandelbrot");
    setFocusPolicy(Qt::WheelFocus);
    setCursor(Qt::CrossCursor);

    m_bDynTranslation = false;
    m_bDynScaling     = false;
    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));

    m_locIters = m_locLength = -1;
    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;

    m_Scale = 1.0f;

    m_rectMandelbrot = QRectF(-2.5, -1.0, 3.5, 2.0);


    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, gl3dView::textColor());
        palette.setColor(QPalette::Text,  gl3dView::textColor());
        QColor clr = gl3dView::backColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QGridLayout*pFrameLayout = new QGridLayout;
        {
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader<br>to compute and plot the Mandelbrot set");

            QLabel *plabMaxIter = new QLabel("Max. iterations:");
            m_pieMaxIter = new IntEdit(s_MaxIter);
            m_pieMaxIter->setToolTip("The number of iterations before bailing out.");

            QLabel *plabMaxLength= new QLabel("Max. length:");
            m_pdeMaxLength = new DoubleEdit(s_MaxLength);
            m_pdeMaxLength->setToolTip("The escape amplitude of z.");

            m_plabScale = new QLabel();
//            m_plabScale->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
            m_plabScale->setFont(DisplayOptions::textFont());

            connect(m_pieMaxIter,   SIGNAL(valueChanged()), SLOT(update()));
            connect(m_pdeMaxLength, SIGNAL(valueChanged()), SLOT(update()));

            pFrameLayout->addWidget(pLabTitle,       1, 1, 1, 2);
            pFrameLayout->addWidget(plabMaxIter,     2, 1);
            pFrameLayout->addWidget(m_pieMaxIter,    2, 2);
            pFrameLayout->addWidget(plabMaxLength,   3, 1);
            pFrameLayout->addWidget(m_pdeMaxLength,  3, 2);
            pFrameLayout->addWidget(m_plabScale,     4, 1, 1, 2);
        }

        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }
}


void gl2dFractal::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dFractal");
    {
        s_MaxIter   = settings.value("MaxIters", s_MaxIter).toInt();
        s_MaxLength = settings.value("MaxLength", s_MaxLength).toFloat();
    }
    settings.endGroup();
}


void gl2dFractal::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dFractal");
    {
        settings.setValue("MaxIters", s_MaxIter);
        settings.setValue("MaxLength", s_MaxLength);
    }
    settings.endGroup();
}


void gl2dFractal::onDynamicIncrement()
{
//    qDebug("onDynamicIncrement::Elapsed=%d ms", int(m_MoveTime.elapsed()));

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

        m_Trans *= (1.0-gl3dView::spinDamping());
    }

    if(m_bDynScaling)
    {
        if(abs(m_ZoomFactor)<10)
        {
            stopDynamicTimer();
            update();
            return;
        }

        double scalefactor(1.0-s_ScaleFactor/3.0 * m_ZoomFactor/120);

        m_Scale *= scalefactor;
        m_ZoomFactor *= (1.0-gl3dView::spinDamping());
    }

    update();
}


void gl2dFractal::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void gl2dFractal::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
    }
    m_bDynTranslation  = m_bDynScaling = false;
    setMouseTracking(true);
}


void gl2dFractal::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
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
            m_ptOffset = QPointF(0.25*width(),0);
            m_Scale = 1.0f;
            update();
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                close();
                return;
            }
        }
    }

    QOpenGLWidget::keyPressEvent(pEvent);
}


void gl2dFractal::showEvent(QShowEvent *pEvent)
{
    setFocus();
    m_ptOffset = QPointF(0.25*width(),0);
    QOpenGLWidget::showEvent(pEvent);
}


void gl2dFractal::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(gl3dView::bSpinAnimation())
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


void gl2dFractal::mousePressEvent(QMouseEvent *pEvent)
{
    m_LastPoint = pEvent->pos();
    m_PressedPoint = m_LastPoint;

    stopDynamicTimer();
    m_MoveTime.restart();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}


void gl2dFractal::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!hasFocus()) setFocus();

    QPoint point = pEvent->pos();

    if(pEvent->buttons() & Qt::LeftButton)
    {
        //translate the view
        m_ptOffset.rx() += (point.x() - m_LastPoint.x())/m_Scale;
        m_ptOffset.ry() += (point.y() - m_LastPoint.y())/m_Scale;

        m_LastPoint.rx() = point.x();
        m_LastPoint.ry() = point.y();

        update();
        return;
    }

    pEvent->accept();
}


void gl2dFractal::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usabel on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(gl3dView::bSpinAnimation() && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {

        float zoomfactor(1.0f);
        if(pEvent->angleDelta().y()>0) zoomfactor = 1.0/(1.0+s_ScaleFactor);
        else                           zoomfactor = 1.0+s_ScaleFactor;

        m_Scale *= zoomfactor;

        int a = rect().center().x();
        int b = rect().center().y();
        m_ptOffset.rx() = a + (m_ptOffset.x()-a);
        m_ptOffset.ry() = b + (m_ptOffset.y()-b);
        update();
    }

    pEvent->accept();
}


void gl2dFractal::screenToViewport(QPoint const &point, QVector2D &real) const
{
    float h2 = float(geometry().height()) /2.0f;
    float w2 = float(geometry().width())  /2.0f;

    real.setX ((float(point.x()) - w2) / w2);
    real.setY(-(float(point.y()) - h2) / w2);
}


/* triangle strip */
void gl2dFractal::makeQuad()
{
    QVector<GLfloat> QuadVertexArray(12, 0);

    int iv = 0;
    QuadVertexArray[iv++] = 0.0f;
    QuadVertexArray[iv++] = 0.0f;

    QuadVertexArray[iv++] = m_rectMandelbrot.left();
    QuadVertexArray[iv++] = m_rectMandelbrot.top();

    QuadVertexArray[iv++] = m_rectMandelbrot.right();
    QuadVertexArray[iv++] = m_rectMandelbrot.top();

    QuadVertexArray[iv++] =  m_rectMandelbrot.right();
    QuadVertexArray[iv++] =  m_rectMandelbrot.bottom();

    QuadVertexArray[iv++] = m_rectMandelbrot.left();
    QuadVertexArray[iv++] = m_rectMandelbrot.bottom();

    QuadVertexArray[iv++] = m_rectMandelbrot.left();
    QuadVertexArray[iv++] = m_rectMandelbrot.top();

    Q_ASSERT(iv==QuadVertexArray.size());

    m_vboQuad.destroy();
    m_vboQuad.create();
    m_vboQuad.bind();
    m_vboQuad.allocate(QuadVertexArray.data(), QuadVertexArray.size() * sizeof(GLfloat));
    m_vboQuad.release();
}


void gl2dFractal::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/resources/shaders/fractal/fractal_VS.glsl";
    m_shadFrac.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadFrac.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac vertex shader log:"+m_shadFrac.log()).toStdString().c_str());
        Trace(strange);
    }

    fsrc = ":/resources/shaders/fractal/fractal_FS.glsl";
    m_shadFrac.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadFrac.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac fragment shader log:"+m_shadFrac.log()).toStdString().c_str());
        Trace(strange);
    }

    m_shadFrac.link();
    m_shadFrac.bind();
    {
        m_attrVertexPosition = m_shadFrac.attributeLocation("VertexPosition");

        m_locLength    = m_shadFrac.uniformLocation("maxlength");
        m_locIters     = m_shadFrac.uniformLocation("maxiters");
        m_locViewTrans = m_shadFrac.uniformLocation("ViewTrans");
        m_locViewScale = m_shadFrac.uniformLocation("ViewScale");
        m_locViewRatio = m_shadFrac.uniformLocation("ViewRatio");
    }
    m_shadFrac.release();    

    makeQuad();
}


void gl2dFractal::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadFrac.bind();
    {
        s_MaxIter   = m_pieMaxIter->value();
        s_MaxLength = m_pdeMaxLength->value();
        m_shadFrac.setUniformValue(m_locIters,     s_MaxIter);
        m_shadFrac.setUniformValue(m_locLength,    s_MaxLength);
        m_shadFrac.setUniformValue(m_locViewRatio, float(width())/float(height()));

        double w = m_rectMandelbrot.width();
        QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);
        m_shadFrac.setUniformValue(m_locViewTrans,   off);
        m_shadFrac.setUniformValue(m_locViewScale,  m_Scale);

        m_vboQuad.bind();
        {
            m_shadFrac.enableAttributeArray(m_attrVertexPosition);
            m_shadFrac.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, 2, 2*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/2/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx*2);

            m_shadFrac.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
    }
    m_shadFrac.release();
    m_plabScale->setText(QString::asprintf("Scale = %g", m_Scale));
}


