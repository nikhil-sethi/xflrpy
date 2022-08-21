/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QApplication>
#include <QRandomGenerator>
#include <QFormLayout>
#include <QLabel>

#include "gl2dnewton.h"

#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xfl3d/views/gl3dview.h> // for the static variables
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>

int gl2dNewton::s_MaxIter = 64;
float gl2dNewton::s_Tolerance = 1.e-6f;
QColor gl2dNewton::s_Colors[5] = {QColor(77,27,21), QColor(75,111,117), QColor(11,47,77), QColor(77,77,77), QColor(131,120,107)};


gl2dNewton::gl2dNewton(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Newton");

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);

    m_bResetRoots = true;
    m_nRoots = 3;
    m_iHoveredRoot = m_iSelectedRoot = -1;

    m_Time = 0;
    for(int i=0; i<2*MAXROOTS; i++)
    {
        m_omega[i] = 2.0*QRandomGenerator::global()->bounded(1.0)-1.0;
    }

    m_locIters = m_locTolerance = -1;
    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;
    m_locNRoots    = -1;
    for(int i=0; i<MAXROOTS; i++) m_locColor[i] = m_locRoot[i] = -1;

    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader to compute<br> and plot a polynomial Newton fractal.");

            QFormLayout*pParamLayout = new QFormLayout;
            {
                m_pieMaxIter = new IntEdit(s_MaxIter);
                m_pieMaxIter->setToolTip("The number of iterations before bailing out.");

                m_pdeTolerance = new DoubleEdit(s_Tolerance);
                m_pdeTolerance->setToolTip("The escape amplitude of z.");
                connect(m_pieMaxIter,   SIGNAL(valueChanged()), SLOT(update()));
                connect(m_pdeTolerance, SIGNAL(valueChanged()), SLOT(update()));

                pParamLayout->addRow("Max. iterations:", m_pieMaxIter);
                pParamLayout->addRow("Tolerance:",       m_pdeTolerance);
            }

            QHBoxLayout *pNRootsLayout = new QHBoxLayout;
            {
                m_prb3roots = new QRadioButton("3 roots");
                m_prb5roots = new QRadioButton("5 roots");
                m_prb3roots->setChecked(true);
                connect(m_prb3roots, SIGNAL(clicked(bool)), SLOT(onNRoots()));
                connect(m_prb5roots, SIGNAL(clicked(bool)), SLOT(onNRoots()));
                pNRootsLayout->addWidget(m_prb3roots);
                pNRootsLayout->addWidget(m_prb5roots);
                pNRootsLayout->addStretch();
            }

            m_pchShowRoots = new QCheckBox("Show roots");
            connect(m_pchShowRoots, SIGNAL(clicked(bool)), SLOT(update()));
            m_pchAnimateRoots = new QCheckBox("Animate roots");
            connect(m_pchAnimateRoots, SIGNAL(clicked(bool)), SLOT(onAnimate(bool)));
            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());


            QLabel *pRefLink = new QLabel;
            pRefLink->setText("Inspired by <a href=https://youtu.be/-RdOwhmqP5s>3Blue1Brown's YouTube video</a>");
            pRefLink->setOpenExternalLinks(true);
            pRefLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pRefLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

            pFrameLayout->addWidget(pLabTitle);
            pFrameLayout->addLayout(pParamLayout);
            pFrameLayout->addLayout(pNRootsLayout);
            pFrameLayout->addWidget(m_pchShowRoots);
            pFrameLayout->addWidget(m_pchAnimateRoots);
            pFrameLayout->addWidget(m_plabScale);
            pFrameLayout->addWidget(pRefLink);
        }

        pFrame->setLayout(pFrameLayout);

        QPalette palette;
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Text,  Qt::white);
        QColor clr = gl3dView::backColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);

        pFrame->setStyleSheet("QFrame{background-color: transparent; color: white}"
                              "QRadioButton{background-color: transparent; color: white}"
                              "QCheckBox{background-color: transparent; color: white}");
//        setWidgetStyle(pFrame, palette);
//        pFrame->setPalette(palette);
    }

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMoveRoots()));

    onNRoots();
}


void gl2dNewton::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dNewton");
    {
        s_MaxIter   = settings.value("MaxIters", s_MaxIter).toInt();
        s_Tolerance = settings.value("MaxLength", s_Tolerance).toFloat();
    }
    settings.endGroup();
}


void gl2dNewton::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dNewton");
    {
        settings.setValue("MaxIters", s_MaxIter);
        settings.setValue("MaxLength", s_Tolerance);
    }
    settings.endGroup();
}


void gl2dNewton::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/shaders/newton/newton_VS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton vertex shader log:"+m_shadNewton.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/newton/newton_FS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton fragment shader log:"+m_shadNewton.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadNewton.link();

    m_shadNewton.bind();
    {
        m_attrVertexPosition = m_shadNewton.attributeLocation("VertexPosition");
        m_locViewTrans = m_shadNewton.uniformLocation("ViewTrans");
        m_locViewScale = m_shadNewton.uniformLocation("ViewScale");
        m_locViewRatio = m_shadNewton.uniformLocation("ViewRatio");
        m_locNRoots    = m_shadNewton.uniformLocation("nroots");
        m_locTolerance = m_shadNewton.uniformLocation("tolerance");
        m_locIters     = m_shadNewton.uniformLocation("maxiters");
        for(int i=0; i<MAXROOTS; i++)
            m_locColor[i] = m_shadNewton.uniformLocation(QString::asprintf("color%d",i));
        for(int i=0; i<MAXROOTS; i++)
            m_locRoot[i]  = m_shadNewton.uniformLocation(QString::asprintf("root%d",i));
    }
    m_shadNewton.release();

    gl2dView::initializeGL();
}


void gl2dNewton::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    float ratio = float(width())/float(height());
    double w = m_rectView.width();
    QVector2D off((-m_ptOffset.x())/width()*w, m_ptOffset.y()/width()*w);

    m_shadNewton.bind();
    {
        int stride = 2;
        s_MaxIter   = m_pieMaxIter->value();
        s_Tolerance = m_pdeTolerance->value();
        m_shadNewton.setUniformValue(m_locIters,     s_MaxIter);
        m_shadNewton.setUniformValue(m_locTolerance, s_Tolerance);
        m_shadNewton.setUniformValue(m_locViewRatio, ratio);

        m_shadNewton.setUniformValue(m_locNRoots, m_nRoots);
        for(int i=0; i<m_nRoots; i++)
        {
            m_shadNewton.setUniformValue(m_locColor[i], s_Colors[i]);
            m_shadNewton.setUniformValue(m_locRoot[i], m_Root[i]);
        }

        m_shadNewton.setUniformValue(m_locViewTrans, off);
        m_shadNewton.setUniformValue(m_locViewScale, m_Scale);

        m_vboQuad.bind();
        {
            m_shadNewton.enableAttributeArray(m_attrVertexPosition);
            m_shadNewton.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, 2, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadNewton.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
    }
    m_shadNewton.release();

    if(m_pchShowRoots->isChecked())
    {
        if(m_bResetRoots)
        {
            int buffersize = m_nRoots*4;
            QVector<float> pts(buffersize);
            int iv = 0;
            for(int is=0; is<m_nRoots; is++)
            {
                pts[iv++] = m_Root[is].x();
                pts[iv++] = m_Root[is].y();
                pts[iv++] = 0.0f;
                if (is==m_iSelectedRoot || is==m_iHoveredRoot)
                    pts[iv++] = 1.0f;
                else
                    pts[iv++] = -1.0f; // invalid state, use uniform
            }

            if(m_vboRoots.isCreated()) m_vboRoots.destroy();
            m_vboRoots.create();
            m_vboRoots.bind();
            m_vboRoots.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
            m_vboRoots.release();

            m_bResetRoots = false;
        }

        QMatrix4x4 m_matModel;
        QMatrix4x4 m_matView;
        QMatrix4x4 m_matProj;
        float s = 1.0;
        int width  = geometry().width();
        int height = geometry().height();
        m_matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-1.0e3*s,1.0e3*s);

        m_matView.scale(m_Scale, m_Scale, m_Scale);
        m_matView.translate(-off.x(), -off.y(), 0.0f);

        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);

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


void gl2dNewton::onNRoots()
{
    if     (m_prb3roots->isChecked()) m_nRoots = 3;
    else if(m_prb5roots->isChecked()) m_nRoots = 5;

    for(int i=0; i<m_nRoots; i++)
    {
        m_amp0[i] = 1.0f;
        m_phi0[i] = 2.0f*PIf/float(m_nRoots) * float(i);
        m_Root[i].setX(m_amp0[i]*cos(m_phi0[i]));
        m_Root[i].setY(m_amp0[i]*sin(m_phi0[i]));
    }

    m_Time = 0;
    m_bResetRoots = true;
    update();
}


void gl2dNewton::onAnimate(bool bAnimate)
{
    if(bAnimate)
    {
        if(!m_Timer.isActive())
        {
            m_Timer.start(17);
        }
    }
    else
        m_Timer.stop();
}


void gl2dNewton::mousePressEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);

    for(int i=0; i<m_nRoots; i++)
    {
        if(pt.distanceToPoint(m_Root[i])<0.025/m_Scale)
        {
            m_Timer.stop();
            m_iSelectedRoot  = i;
            return;
        }
    }

    gl2dView::mousePressEvent(pEvent);
}


void gl2dNewton::mouseMoveEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);

    if(m_iSelectedRoot>=0 && m_iSelectedRoot<m_nRoots)
    {
        m_Root[m_iSelectedRoot] = pt;
        for(int i=0; i<m_nRoots; i++)
        {
            m_amp0[i] = sqrt(m_Root[i].x()*m_Root[i].x()+m_Root[i].y()*m_Root[i].y());
            m_phi0[i] = atan2f(m_Root[i].y(), m_Root[i].x());
        }
        m_Time = 0;
        m_bResetRoots = true;
        update();
        return;
    }
    else
    {
        for(int i=0; i<m_nRoots; i++)
        {
            if(pt.distanceToPoint(m_Root[i])<0.025/m_Scale)
            {
                m_iHoveredRoot = i;
                m_bResetRoots = true;
                update();
                return;
            }
        }
        m_iHoveredRoot = -1;
        m_bResetRoots = true;
        if(!m_pchAnimateRoots->isChecked()) update();
    }
    gl2dView::mouseMoveEvent(pEvent);
}


void gl2dNewton::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(m_iSelectedRoot>=0 || m_iHoveredRoot>=0)
    {
        m_iSelectedRoot = m_iHoveredRoot = -1;
        m_bResetRoots = true;
        m_Time = 0;
        update();
        QApplication::restoreOverrideCursor();

        if(m_pchAnimateRoots->isChecked()) m_Timer.start(17);
        return;
    }
    gl2dView::mouseReleaseEvent(pEvent);
}


void gl2dNewton::onMoveRoots()
{
    float t = float(m_Time)/1000.0f;

    for(int i=0; i<m_nRoots; i++)
    {
        float amp = m_amp0[i] - (1.0f - cosf(m_omega[2*i]*t*6.0*PIf))/2.0f;
        float phi = m_phi0[i] + 2.0f*PIf*sinf(m_omega[2*i+1]*t);

        m_Root[i].setX(amp*cos(phi));
        m_Root[i].setY(amp*sin(phi));
    }

    m_Time++;

    m_bResetRoots = true;
    update();
}








