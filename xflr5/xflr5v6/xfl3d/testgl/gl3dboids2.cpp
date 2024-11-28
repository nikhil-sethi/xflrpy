/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QTime>
#include <QGridLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QRandomGenerator>
#include <QFutureSynchronizer>
#include <QtConcurrent/qtconcurrentrun.h>

#define INFLUENCEDIST 3.0
#define MAXSPEED 3.0
#define MAXFORCE 0.03

#include "gl3dboids2.h"
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xfl3d/globals/gl_globals.h>
#include <xfl3d/globals/w3dprefs.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/wt_globals.h>

int gl3dBoids2::s_NGroups = 32;
float gl3dBoids2::s_Cohesion   = 1.0f;
float gl3dBoids2::s_Separation = 1.0f;
float gl3dBoids2::s_Alignment  = 1.0f;
float gl3dBoids2::s_Predator   = 1.0f;
float gl3dBoids2::s_Ratio      = 0.15f;


gl3dBoids2::gl3dBoids2(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Boids (GPU)");

    m_bResetBox = m_bResetInstances = true;

    m_stackInterval.resize(50);
    m_stackInterval.fill(0);

    m_bAxes = false;

    m_BoxWidth = 200.0f;

    QSurfaceFormat Surfformat;
    Surfformat.setProfile(QSurfaceFormat::CoreProfile);
    Surfformat.setMajorVersion(4);
    Surfformat.setMinorVersion(3);

    makeCurrent();
    setFormat(Surfformat);
    create();

    QPalette palette;
    palette.setColor(QPalette::WindowText, s_TextColor);
    palette.setColor(QPalette::Text, s_TextColor);

    QColor clr = s_BackgroundColor;
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        pFrame->setMinimumWidth(350);
        QGridLayout*pMainLayout = new QGridLayout;
        {
            QLabel *plabCS = new QLabel("<b>Compute shader settings:</b>");

            QLabel *plabGroupSize = new QLabel(QString::asprintf("Group size = %d", GROUP_SIZE));
            m_plabNMaxGroups = new QLabel("Max. number of groups");
            QLabel *plabNGroups = new QLabel("Number of groups =");
            m_pieNGroups = new IntEdit(s_NGroups);
            m_pieNGroups->setToolTip("The number of groups should be less than the max. number of groups accepted by the GPU.<br>"
                                     "The GroupSize is hard-coded in the compute shader.<br>"
                                     "The number of particles is NGroups x GroupSize.<br>"
                                     "Note that in the present case the main limitations to the number of groups are:"
                                     "<ul>"
                                     "<li>the processing time in the compute shader</li>"
                                     "<li>the number of particles that can be rendered without loss of frame rate</li>"
                                     "</ul>");
            connect(m_pieNGroups, SIGNAL(valueChanged()), SLOT(onSwarmReset()));

            m_plabNParticles = new QLabel;


            QLabel *plabBoids = new QLabel("<b>Boid settings:</b>");

            QLabel *plabCohesion   = new QLabel("Cohesion:");
            QLabel *plabSeparation = new QLabel("Separation:");
            QLabel *plabAlignment  = new QLabel("Alignment:");

            m_pslCohesion   = new QSlider(Qt::Horizontal);
            m_pslCohesion->setToolTip("<b>Cohesion:</b> steer to move towards the average position of local flockmates");
            m_pslCohesion->setMinimum(0);
            m_pslCohesion->setMaximum(100);
            m_pslCohesion->setTickInterval(10);
            m_pslCohesion->setTickPosition(QSlider::TicksBelow);
            m_pslCohesion->setValue(int(s_Cohesion*30.0f));
            connect(m_pslCohesion, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabCohesion = new QLabel;
            m_plabCohesion->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            m_pslSeparation = new QSlider(Qt::Horizontal);
            m_pslSeparation->setToolTip("<b>Separation:</b> Steer to avoid crowding local flockmates.");
            m_pslSeparation->setMinimum(0);
            m_pslSeparation->setMaximum(100);
            m_pslSeparation->setTickInterval(10);
            m_pslSeparation->setTickPosition(QSlider::TicksBelow);
            m_pslSeparation->setValue(int(s_Separation*30.0f));
            connect(m_pslSeparation, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabSeparation = new QLabel;
            m_plabSeparation->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));


            m_pslAlignment  = new QSlider(Qt::Horizontal);
            m_pslAlignment->setToolTip("<b>Alignment:</b> Steer towards the average heading of local flockmates.");
            m_pslAlignment->setMinimum(0);
            m_pslAlignment->setMaximum(100);
            m_pslAlignment->setTickInterval(10);
            m_pslAlignment->setTickPosition(QSlider::TicksBelow);
            m_pslAlignment->setValue(int(s_Alignment*30.0f));
            connect(m_pslAlignment, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabAlignment = new QLabel;
            m_plabAlignment->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            m_pchPredator = new QCheckBox("Predator");
            m_pchPredator->setToolTip("The predator steers towards the average position of the closest boids.<br>"
                                      "It is 50% faster and can accelerate three times more than the boids can.");
            m_pslPredator  = new QSlider(Qt::Horizontal);
            m_pslPredator->setToolTip("Defines the boid's fear level of the predator");
            m_pslPredator->setMinimum(0);
            m_pslPredator->setMaximum(100);
            m_pslPredator->setTickInterval(10);
            m_pslPredator->setTickPosition(QSlider::TicksBelow);
            m_pslPredator->setValue(int(s_Predator*30.0f));
            connect(m_pslPredator, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabPredator = new QLabel;
            m_plabPredator->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            QLabel *plabDisplay = new QLabel("<b>Display settings:</b>");

            QLabel *plabRatio = new QLabel("Box height:");
            m_pslRatio = new QSlider(Qt::Horizontal);
            m_pslRatio->setMinimum(0);
            m_pslRatio->setMaximum(100);
            m_pslRatio->setTickInterval(10);
            m_pslRatio->setTickPosition(QSlider::TicksBelow);
            m_pslRatio->setValue(int(s_Ratio*100.0f));
            connect(m_pslRatio, SIGNAL(sliderMoved(int)), SLOT(onSlider()));


            QLabel *plabOpacity   = new QLabel("Box opacity:");
            m_pslBoxOpacity  = new QSlider(Qt::Horizontal);
            m_pslBoxOpacity->setToolTip("<b>Alignment:</b> Steer towards the average heading of local flockmates.");
            m_pslBoxOpacity->setMinimum(0);
            m_pslBoxOpacity->setMaximum(100);
            m_pslBoxOpacity->setTickInterval(10);
            m_pslBoxOpacity->setTickPosition(QSlider::TicksBelow);
            m_pslBoxOpacity->setValue(10);

            QCheckBox *pchAxes = new QCheckBox("Axes");
            pchAxes->setChecked(m_bAxes);
            connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

            m_plabFrameRate = new QLabel;
            m_plabFrameRate->setFont(DisplayOptions::tableFont());

            QLabel *pBoidsLink = new QLabel;
            pBoidsLink->setText("<a href=https://en.wikipedia.org/wiki/Boids>https://en.wikipedia.org/wiki/Boids</a>");
            pBoidsLink->setOpenExternalLinks(true);
            pBoidsLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

            pMainLayout->addWidget(plabCS,            1, 1, 1, 2);
            pMainLayout->addWidget(plabGroupSize,     2, 1, 1, 2);
            pMainLayout->addWidget(m_plabNMaxGroups,  3, 1, 1, 2);
            pMainLayout->addWidget(plabNGroups,       4, 1);
            pMainLayout->addWidget(m_pieNGroups,      4, 2);
            pMainLayout->addWidget(m_plabNParticles,  5, 1, 1, 2);

            pMainLayout->addWidget(plabBoids,         6, 1, 1, 2);

            pMainLayout->addWidget(plabCohesion,      7, 1);
            pMainLayout->addWidget(m_pslCohesion,     7, 2);
            pMainLayout->addWidget(m_plabCohesion,    7, 3);

            pMainLayout->addWidget(plabSeparation,    8, 1);
            pMainLayout->addWidget(m_pslSeparation,   8, 2);
            pMainLayout->addWidget(m_plabSeparation,  8, 3);

            pMainLayout->addWidget(plabAlignment,     9, 1);
            pMainLayout->addWidget(m_pslAlignment,    9, 2);
            pMainLayout->addWidget(m_plabAlignment,   9, 3);

            pMainLayout->addWidget(m_pchPredator,     10, 1);
            pMainLayout->addWidget(m_pslPredator,     10, 2);
            pMainLayout->addWidget(m_plabPredator,    10, 3);

            pMainLayout->addWidget(plabDisplay,       11, 1, 1, 2);
            pMainLayout->addWidget(plabRatio,         12, 1);

            pMainLayout->addWidget(m_pslRatio,        12, 2);

            pMainLayout->addWidget(plabOpacity,       13, 1);
            pMainLayout->addWidget(m_pslBoxOpacity,   13, 2);

            pMainLayout->addWidget(pchAxes,           14, 1, 1, 3);
            pMainLayout->addWidget(m_plabFrameRate,   15, 1, 1, 3);
            pMainLayout->addWidget(pBoidsLink,        17, 1, 1, 3);

            pMainLayout->setColumnStretch(2,1);
        }
        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    onSlider();

    setReferenceLength(1.5*m_BoxWidth);
//    setLightOn(false);

//    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
//    qDebug()<<"refreshrate="<<QGuiApplication::primaryScreen()->refreshRate()<<period;
    connect(&m_Timer, SIGNAL(timeout()), SLOT(update()));
    m_Timer.start(0); //  As a special case, a QTimer with a timeout of 0 will time out as soon as possible
}


void gl3dBoids2::onSlider()
{
    s_Cohesion   = float(m_pslCohesion->value())  /30.0f;
    s_Separation = float(m_pslSeparation->value())/30.0f;
    s_Alignment  = float(m_pslAlignment->value()) /30.0f;
    s_Predator   = float(m_pslPredator->value())  /30.0f;

    m_plabCohesion->setText(  QString::asprintf("%7.3f", s_Cohesion));
    m_plabSeparation->setText(QString::asprintf("%7.3f", s_Separation));
    m_plabAlignment->setText( QString::asprintf("%7.3f", s_Alignment));
    m_plabPredator->setText( QString::asprintf("%7.3f", s_Predator));

    s_Ratio = std::max(m_pslRatio->value()/100.0f,0.01f);
}


void gl3dBoids2::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dBoids2");
    {
        s_NGroups      = settings.value("NGroups",    s_NGroups).toInt();
        s_Cohesion     = settings.value("Cohesion",   s_Cohesion).toFloat();
        s_Separation   = settings.value("Separation", s_Separation).toFloat();
        s_Alignment    = settings.value("Alignment",  s_Alignment).toFloat();
        s_Predator     = settings.value("Predator",   s_Predator).toFloat();
        s_Ratio        = settings.value("Ratio",      s_Ratio).toFloat();
    }
    settings.endGroup();
}


void gl3dBoids2::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dBoids2");
    {
        settings.setValue("NGroups",    s_NGroups);
        settings.setValue("Cohesion",   s_Cohesion);
        settings.setValue("Separation", s_Separation);
        settings.setValue("Alignment",  s_Alignment);
        settings.setValue("Predator",   s_Predator);
        settings.setValue("Ratio",      s_Ratio);
    }
    settings.endGroup();
}


void gl3dBoids2::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_Timer.isActive()) m_Timer.stop();
            else                   m_Timer.start(0);
            break;
        case Qt::Key_Escape:
            showNormal();
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dBoids2::initializeGL()
{
    gl3dTestGLView::initializeGL();
#ifndef Q_OS_MAC
    QString csrc, strange;

    csrc = ":/shaders/boids2/boids2_CS.glsl";
    m_shadCompute.addShaderFromSourceFile(QOpenGLShader::Compute, csrc);
    if(m_shadCompute.log().length())
    {
        strange = QString::asprintf("%s", QString("Compute shader log:"+m_shadCompute.log()).toStdString().c_str());
        trace(strange);
    }

    if(!m_shadCompute.link())
    {
        trace("Compute shader is not linked");
    }
    m_shadCompute.bind();
    {
        m_locWidth       = m_shadCompute.uniformLocation("width");
        m_locHeight      = m_shadCompute.uniformLocation("height");
        m_locCohesion    = m_shadCompute.uniformLocation("cohesion");
        m_locSeparation  = m_shadCompute.uniformLocation("separation");
        m_locAlignment   = m_shadCompute.uniformLocation("alignment");
        m_locPredator    = m_shadCompute.uniformLocation("predatorf");
        m_locHasPredator = m_shadCompute.uniformLocation("haspredator");
    }
    m_shadCompute.release();

    int MaxInvocations = 0;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &MaxInvocations);

    int workGroupCounts[3] = { 0 };
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCounts[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCounts[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCounts[2]);

    int workGroupSizes[3] = { 0 };
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizes[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizes[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizes[2]);

/*    qDebug("Max invocations = %5d", MaxInvocations);
    qDebug("Work Group Count= %5d %5d %5d", workGroupCounts[0], workGroupCounts[1], workGroupCounts[2]);
    qDebug("Work Group Size = %5d %5d %5d", workGroupSizes[0], workGroupSizes[1], workGroupSizes[2]);*/

    int n = workGroupCounts[0];
    int pow = 1;
    do
    {
        pow++;
        n=n/2;
    }while(n>1);

    m_plabNMaxGroups->setText(QString("Max. number of groups = 2<sup>")+QString::asprintf("%d", pow)+QString("</sup>"));
#endif
}


void gl3dBoids2::glMake3dObjects()
{
    if(m_bResetBox)
    {
        m_bResetBox = false;
    }

    if(m_bResetInstances)
    {
        int nParticles = GROUP_SIZE * s_NGroups;
        m_plabNParticles->setText(QString::asprintf("Number of particles =%d", nParticles));

        double amp = m_BoxWidth/4.0;

        //need to use v4 vertices for velocity due to std140/430 padding constraints for vec3:
        int buffersize = nParticles *(4+4+4); //4 vertices + 4 velocity + 4 color components for each boid
        // double the size to create a virtual double buffer
        // the second half of the buffer is populated in the compute shader
        buffersize *=2;

        QVector<float>BufferArray(buffersize); // defaults to 0
        int iv=0;
        for(int i=0; i<nParticles; i++)
        {
            BufferArray[iv++] = -amp/2.0f + QRandomGenerator::global()->bounded(amp);
            BufferArray[iv++] = -amp/2.0f + QRandomGenerator::global()->bounded(amp);
            BufferArray[iv++] = -amp/2.0f + QRandomGenerator::global()->bounded(amp);
            BufferArray[iv++] = 1.0f;

            BufferArray[iv++] = -amp/6.0  + QRandomGenerator::global()->bounded(amp/3.0);
            BufferArray[iv++] = -amp/6.0  + QRandomGenerator::global()->bounded(amp/3.0);
            BufferArray[iv++] = (-amp/6.0  + QRandomGenerator::global()->bounded(amp/3.0))*s_Ratio;
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = 0.0f; // overwritten at 1st iteration in the CS
            BufferArray[iv++] = 0.0f;
            BufferArray[iv++] = 0.0f;
            BufferArray[iv++] = 0.0f;
        }
        // rest is zero
//        Q_ASSERT(iv==buffersize/2);

        if(m_vboBoids.isCreated()) m_vboBoids.destroy();
        m_vboBoids.create();
        m_vboBoids.bind();
        {
            m_vboBoids.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
        }
        m_vboBoids.release();

        m_bResetInstances = false;
    }
}


void gl3dBoids2::glRenderView()
{
#ifndef Q_OS_MAC
    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    // move the flock at each frame update
    int stride = 12;
    int buffersize = GROUP_SIZE * s_NGroups * stride;
    m_shadCompute.bind();
    {
        m_shadCompute.setUniformValue(m_locWidth,      m_BoxWidth);
        m_shadCompute.setUniformValue(m_locHeight,     m_BoxWidth*s_Ratio);
        m_shadCompute.setUniformValue(m_locCohesion,   s_Cohesion);
        m_shadCompute.setUniformValue(m_locSeparation, s_Separation);
        m_shadCompute.setUniformValue(m_locAlignment,  s_Alignment);
        m_shadCompute.setUniformValue(m_locPredator,   s_Predator);
        if(m_pchPredator->isChecked()) m_shadCompute.setUniformValue(m_locHasPredator,  1);
        else                           m_shadCompute.setUniformValue(m_locHasPredator,  0);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vboBoids.bufferId());

        glDispatchCompute(s_NGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
        getGLError();

        // virtual double buffering
        glBindBuffer(GL_COPY_READ_BUFFER, m_vboBoids.bufferId());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_READ_BUFFER, buffersize*sizeof(float), 0, buffersize*sizeof(float));
        getGLError();
    }
    m_shadCompute.release();

    m_vao.bind();
    {
        m_shadPoint2.bind();
        {
            QMatrix4x4 vmMat(m_matView*m_matModel);
            QMatrix4x4 pvmMat(m_matProj*vmMat);


            m_shadPoint2.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
            m_shadPoint2.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);

            m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, m_ClipPlanePos);

            m_vboBoids.bind();
            {
                int nPts = m_vboBoids.size()/stride/int(sizeof(float)) /2; // first half of the buffer only

                m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
                m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 8* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

                glPointSize(7.0f);

                if(!m_pchPredator->isChecked())
                    glDrawArrays(GL_POINTS, 0, nPts);
                else
                {
                    glDrawArrays(GL_POINTS, 0, nPts-1);
                    glPointSize(31.0f);
                    glDrawArrays(GL_POINTS, nPts-1, 1);
                }

                m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
                m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
            }
            m_shadPoint2.release();
        }
        m_shadPoint2.release();
    }
    m_vao.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 0);
    }
    m_shadSurf.release();
    paintBox(0, 0, 0, 2.0*m_BoxWidth, 2.0*m_BoxWidth, 2.0*m_BoxWidth*s_Ratio, QColor(91,91,91, double(m_pslBoxOpacity->value())/100.0*255.0), false);

    m_stackInterval.push_back(QTime::currentTime().msecsSinceStartOfDay());
    double average = 0.0;
    for(int i=0; i<m_stackInterval.size()-1; i++)
        average += m_stackInterval.at(i+1)-m_stackInterval.at(i);
    average /= double(m_stackInterval.size()-1);
    m_plabFrameRate->setText(QString::asprintf("FPS = %4.1f Hz", 1000.0/average));
    m_stackInterval.pop_front();

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
#endif
}


void gl3dBoids2::onSwarmReset()
{
    s_NGroups = m_pieNGroups->value();
    m_bResetInstances = true;
}





