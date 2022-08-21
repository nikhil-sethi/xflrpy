/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QApplication>
#include <QFormLayout>
#include <QLabel>

#include "gl2dfractal.h"

#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>

#include <xfl3d/views/gl3dview.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>


int gl2dFractal::s_Hue(1000);
int gl2dFractal::s_MaxIter(1024);
float gl2dFractal::s_MaxLength(10);
QVector2D gl2dFractal::s_Seed(-0.35099f, -0.605502f);

gl2dFractal::gl2dFractal(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Mandelbrot");

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);
    m_Scale = 0.5f;

    m_nRoots = 1;
    m_iHoveredRoot = m_iSelectedRoot = -1;
    m_bResetRoots = true;

    m_locJulia = m_locParamX = m_locParamY = -1;
    m_locIters = m_locLength = m_locHue = -1;
    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;

    QFrame *m_pCmdFrame = new QFrame(this);
    {
        m_pCmdFrame->setCursor(Qt::ArrowCursor);

        m_pCmdFrame->setFrameShape(QFrame::NoFrame);
        m_pCmdFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader to compute <br>and plot the Mandelbrot and Julia sets");
            QHBoxLayout *pModeLayout = new QHBoxLayout;
            {
                m_prbMandelbrot = new QRadioButton("Mandelbrot");
                m_prbJulia = new QRadioButton("Julia");
                m_prbMandelbrot->setChecked(true);
                connect(m_prbMandelbrot, SIGNAL(clicked(bool)), SLOT(onMode()));
                connect(m_prbJulia,      SIGNAL(clicked(bool)), SLOT(onMode()));

                pModeLayout->addWidget(m_prbMandelbrot);
                pModeLayout->addWidget(m_prbJulia);
            }

            QFormLayout*pParamLayout = new QFormLayout;
            {
                m_pieMaxIter = new IntEdit(s_MaxIter);
                m_pieMaxIter->setToolTip("The number of iterations before bailing out.");

                m_pdeMaxLength = new DoubleEdit(s_MaxLength);
                m_pdeMaxLength->setToolTip("The escape amplitude of z.");

                connect(m_pieMaxIter,   SIGNAL(valueChanged()), SLOT(onMode()));
                connect(m_pdeMaxLength, SIGNAL(valueChanged()), SLOT(onMode()));

                pParamLayout->addRow("Max. iterations:", m_pieMaxIter);
                pParamLayout->addRow("Max. length:",     m_pdeMaxLength);
            }

            m_pchShowSeed = new QCheckBox("Show seed");
            connect(m_pchShowSeed, SIGNAL(clicked()), SLOT(onMode()));
            m_pchShowSeed->setToolTip("Use the mouse to drag the seed and update the corresponding Julia set");

            QHBoxLayout *pHueLayout = new QHBoxLayout;
            {
                QLabel *pLabHue = new QLabel("Hue:");
                m_pslTau = new QSlider(Qt::Horizontal);
                m_pslTau->setMinimum(0);
                m_pslTau->setMaximum(1000);
                m_pslTau->setSliderPosition(500);
                m_pslTau->setTickInterval(50);
                m_pslTau->setTickPosition(QSlider::TicksBelow);
                m_pslTau->setValue(s_Hue);
                connect(m_pslTau, SIGNAL(valueChanged(int)), SLOT(update()));
                pHueLayout->addWidget(pLabHue);
                pHueLayout->addWidget(m_pslTau);
            }


            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());

            QLabel *pRefLink = new QLabel;
            pRefLink->setText("Inspired by <a href=https://youtu.be/LqbZpur38nw>3Blue1Brown's YouTube video</a>");
            pRefLink->setOpenExternalLinks(true);
            pRefLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pRefLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

            pFrameLayout->addWidget(pLabTitle);
            pFrameLayout->addLayout(pModeLayout);
            pFrameLayout->addLayout(pParamLayout);
            pFrameLayout->addWidget(m_pchShowSeed);
            pFrameLayout->addLayout(pHueLayout);
            pFrameLayout->addWidget(m_plabScale);
            pFrameLayout->addWidget(pRefLink);
        }

        m_pCmdFrame->setLayout(pFrameLayout);

        m_pCmdFrame->setStyleSheet("QFrame{background-color: transparent; color: white}"
                              "QRadioButton{background-color: transparent; color: white}"
                              "QCheckBox{background-color: transparent; color: white}");
    }
}


void gl2dFractal::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dFractal");
    {
        s_Hue       = settings.value("Hue",       s_Hue).toInt();
        s_MaxIter   = settings.value("MaxIters",  s_MaxIter).toInt();
        s_MaxLength = settings.value("MaxLength", s_MaxLength).toFloat();
        s_Seed.setX(settings.value("SeedX", s_Seed.x()).toFloat());
        s_Seed.setY(settings.value("SeedY", s_Seed.y()).toFloat());
    }
    settings.endGroup();
}


void gl2dFractal::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dFractal");
    {
        settings.setValue("Hue",       s_Hue);
        settings.setValue("MaxIters",  s_MaxIter);
        settings.setValue("MaxLength", s_MaxLength);
        settings.setValue("SeedX", s_Seed.x());
        settings.setValue("SeedY", s_Seed.y());
    }
    settings.endGroup();
}


void gl2dFractal::onMode()
{
    m_bResetRoots = true;
    update();
}


void gl2dFractal::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/shaders/fractal/fractal_VS.glsl";
    m_shadFrac.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadFrac.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac vertex shader log:"+m_shadFrac.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/fractal/fractal_FS.glsl";
    m_shadFrac.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadFrac.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac fragment shader log:"+m_shadFrac.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadFrac.link();
    m_shadFrac.bind();
    {
        m_attrVertexPosition = m_shadFrac.attributeLocation("VertexPosition");

        m_locJulia     = m_shadFrac.uniformLocation("julia");
        m_locParamX    = m_shadFrac.uniformLocation("paramx");
        m_locParamY    = m_shadFrac.uniformLocation("paramy");
        m_locLength    = m_shadFrac.uniformLocation("maxlength");
        m_locIters     = m_shadFrac.uniformLocation("maxiters");
        m_locHue       = m_shadFrac.uniformLocation("tau");
        m_locViewTrans = m_shadFrac.uniformLocation("ViewTrans");
        m_locViewScale = m_shadFrac.uniformLocation("ViewScale");
        m_locViewRatio = m_shadFrac.uniformLocation("ViewRatio");
    }
    m_shadFrac.release();

    gl2dView::initializeGL();
}


void gl2dFractal::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    double w = m_rectView.width();
    QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);

    m_shadFrac.bind();
    {
        int stride = 2;
        s_MaxIter   = m_pieMaxIter->value();
        s_MaxLength = m_pdeMaxLength->value();
        if(m_prbJulia->isChecked()) m_shadFrac.setUniformValue(m_locJulia,  1);
        else                        m_shadFrac.setUniformValue(m_locJulia,  0);
        m_shadFrac.setUniformValue(m_locParamX,    s_Seed.x());
        m_shadFrac.setUniformValue(m_locParamY,    s_Seed.y());
        m_shadFrac.setUniformValue(m_locIters,     s_MaxIter);
        m_shadFrac.setUniformValue(m_locLength,    s_MaxLength);
        m_shadFrac.setUniformValue(m_locViewRatio, float(width())/float(height()));

        s_Hue = m_pslTau->value();
        float tau = float(s_Hue)/1000.0f;
        m_shadFrac.setUniformValue(m_locHue, tau);

        m_shadFrac.setUniformValue(m_locViewTrans,  off);
        m_shadFrac.setUniformValue(m_locViewScale,  m_Scale);

        m_vboQuad.bind();
        {
            m_shadFrac.enableAttributeArray(m_attrVertexPosition);
            m_shadFrac.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, 2, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadFrac.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
    }
    m_shadFrac.release();


    if(m_pchShowSeed->isChecked())
    {
        if(m_bResetRoots)
        {
            int nseed = 1;
            int buffersize = nseed*4;
            QVector<float> pts(buffersize);
            int iv = 0;

            pts[iv++] = s_Seed.x();
            pts[iv++] = s_Seed.y();
            pts[iv++] = 0.0f;
            if (m_iHoveredRoot==0)
                pts[iv++] = 1.0f;
            else
                pts[iv++] = -1.0f; // invalid state, use uniform

            if(m_vboRoots.isCreated()) m_vboRoots.destroy();
            m_vboRoots.create();
            m_vboRoots.bind();
            m_vboRoots.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
            m_vboRoots.release();

            if(m_prbMandelbrot->isChecked())
            {
                buffersize = s_MaxIter           // number of segments
                             *2                  // two vertices/segment
                             *4;                 // three components/vertex
                pts.resize(buffersize);

                float x(s_Seed.x());
                float y(s_Seed.y());
                float xn(0), yn(0);
                iv = 0;
                for(int is=0; is<s_MaxIter; is++)
                {
                    pts[iv++] = x;
                    pts[iv++] = y;
                    pts[iv++] = 0.0f;
                    pts[iv++] = 1.0f;

                    xn = x*x - y*y + s_Seed.x();
                    yn = 2.0*x*y   + s_Seed.y();
                    if(std::isnormal(x) && std::isnormal(y))
                    {
                        x = xn;
                        y = yn;
                    }

                    pts[iv++] = x;
                    pts[iv++] = y;
                    pts[iv++] = 0.0f;
                    pts[iv++] = 1.0f;
                }

                Q_ASSERT(iv==buffersize);

                if(m_vboSegs.isCreated()) m_vboSegs.destroy();
                m_vboSegs.create();
                m_vboSegs.bind();
                m_vboSegs.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
                m_vboSegs.release();
            }
            m_bResetRoots = false;
        }

        QMatrix4x4 matModel;
        QMatrix4x4 matView;
        QMatrix4x4 matProj;
        float s = 1.0;
        int width  = geometry().width();
        int height = geometry().height();
        matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-1.0e3*s,1.0e3*s);

        matView.scale(m_Scale, m_Scale, m_Scale);
        matView.translate(-off.x(), -off.y(), 0.0f);

        QMatrix4x4 vmMat(matView*matModel);
        QMatrix4x4 pvmMat(matProj*vmMat);

        if(m_prbMandelbrot->isChecked())
        {
            m_shadLine.bind();
            {
                float m_ClipPlanePos(500.0);
                m_shadLine.setUniformValue(m_locLine.m_ClipPlane, m_ClipPlanePos);
                m_shadLine.setUniformValue(m_locLine.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
                m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
                m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
            }
            m_shadLine.release();
            paintSegments(m_vboSegs, Qt::darkCyan, 1.0f, Line::SOLID, false);
        }

        m_shadPoint.bind();
        {
            float m_ClipPlanePos(500.0);
            m_shadPoint.setUniformValue(m_locPoint.m_ClipPlane, m_ClipPlanePos);
            m_shadPoint.setUniformValue(m_locPoint.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix,  vmMat);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
        }
        m_shadPoint.release();

        paintPoints(m_vboRoots, 1.0, 0, false, Qt::white, 4);

    }

    m_plabScale->setText(QString::asprintf("Scale = %g", m_Scale));

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}


void gl2dFractal::mousePressEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);

    int nroots = 1;
    for(int i=0; i<nroots; i++)
    {
        if(pt.distanceToPoint(s_Seed)<0.025/m_Scale)
        {
//            m_Timer.stop();
//            m_pchAnimateRoots->setChecked(false);
            m_iSelectedRoot = 0;
            return;
        }
    }

    gl2dView::mousePressEvent(pEvent);
}


void gl2dFractal::mouseMoveEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);

    if(m_iSelectedRoot>=0)
    {
        s_Seed = pt;
        m_amp0 = sqrt(s_Seed.x()*s_Seed.x()+s_Seed.y()*s_Seed.y());
        m_phi0 = atan2f(s_Seed.y(), s_Seed.x());
//        m_Time = 0;
        m_bResetRoots = true;
        update();
        return;
    }
    else
    {
        for(int i=0; i<m_nRoots; i++)
        {
            if(pt.distanceToPoint(s_Seed)<0.025/m_Scale)
            {
                m_iHoveredRoot = 0;
                m_bResetRoots = true;
                update();
                return;
            }
        }
        m_iHoveredRoot = -1;
        m_bResetRoots = true;
        update();
    }
    gl2dView::mouseMoveEvent(pEvent);
}


void gl2dFractal::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(m_iSelectedRoot>=0 || m_iHoveredRoot>=0)
    {
        m_iSelectedRoot = m_iHoveredRoot = -1;
        m_bResetRoots = true;
//        m_Time = 0;
        update();
        QApplication::restoreOverrideCursor();

        return;
    }
    gl2dView::mouseReleaseEvent(pEvent);
}

