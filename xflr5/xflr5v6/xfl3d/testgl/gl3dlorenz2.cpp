/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/


#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QRandomGenerator>
#include <QOpenGLVertexArrayObject>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTime>

#include "gl3dlorenz2.h"
#include <xfl3d/globals/gl_globals.h>
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/wt_globals.h>

#define GROUP_SIZE 64 //max. value hardcoded in the CS


int gl3dLorenz2::s_NGroups(1024);


double gl3dLorenz2::s_X0(7.0);
double gl3dLorenz2::s_Y0(13.0);
double gl3dLorenz2::s_Z0(17.0);
double gl3dLorenz2::s_Scatter(0.25);
double gl3dLorenz2::s_dt(0.002);


float gl3dLorenz2::s_Size = 5.0f;

gl3dLorenz2::gl3dLorenz2(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Lorenz");

    m_bResetParticles = true;

    m_locRadius = -1;
    m_locPosition = m_locFillColor = -1;

    m_stackInterval.resize(50);
    m_stackInterval.fill(0);


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
        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *plabTitle = new QLabel("Using OpenGL's compute shader<br>to compute the Lorenz attractor");
#ifdef Q_OS_MAC
            plabTitle->setText("Compute shaders require OpenGL 4.3<br>whereas macOS only supports OpenGL 4.1");
            plabTitle->setStyleSheet("color: blue; font: bold");
#endif


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
                                         "Note that in the present case the main limitation to the number of groups is "
                                         "the number of particles that can be rendered without loss of frame rate.");

                m_plabNParticles = new QLabel;

                QLabel *pLabAttractor = new QLabel("<b>Attractor settings:</b>");
                QLabel *plabX = new QLabel("x<sub>0</sub> =");
                QLabel *plabY = new QLabel("y<sub>0</sub> =");
                QLabel *plabZ = new QLabel("z<sub>0</sub> =");
                m_pdeX = new DoubleEdit(s_X0);
                m_pdeY = new DoubleEdit(s_Y0);
                m_pdeZ = new DoubleEdit(s_Z0);
                m_pdeX->setToolTip("This defines the starting average position of the particles");
                m_pdeY->setToolTip("This defines the starting average position of the particles");
                m_pdeZ->setToolTip("This defines the starting average position of the particles");

                QLabel *plabScatt = new QLabel("Initial scatter=");
                m_pdeScatter = new DoubleEdit(s_Scatter);
                m_pdeScatter->setToolTip("This defines the max. deviation of the particles around the average starting position");


                QLabel *plabDt = new QLabel("dt=");
                m_pdeDt = new DoubleEdit(s_dt);
                m_pdeDt->setToolTip("This defines the time step of the RK4 method");

                QPushButton *ppbRestart = new QPushButton("Restart attractor");
                connect(ppbRestart, SIGNAL(clicked(bool)), SLOT(onRestart()));

                m_plabFrameRate = new QLabel;
                m_plabFrameRate->setFont(DisplayOptions::tableFont());

                QLabel *plabSize = new QLabel("Particle size =");
                m_pdeParticleSize = new DoubleEdit(s_Size);
                connect(m_pdeParticleSize, SIGNAL(valueChanged()), SLOT(onParticleSize()));

                QCheckBox *pchAxes = new QCheckBox("Axes");
                pchAxes->setChecked(true);
                connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

                pMainLayout->addWidget(plabCS,            1,  1, 1, 2);
                pMainLayout->addWidget(plabGroupSize,     2,  1, 1, 2);
                pMainLayout->addWidget(m_plabNMaxGroups,  3,  1, 1, 2);
                pMainLayout->addWidget(plabNGroups,       4,  1);
                pMainLayout->addWidget(m_pieNGroups,      4,  2);
                pMainLayout->addWidget(m_plabNParticles,  5,  1, 1, 2);

                pMainLayout->addWidget(pLabAttractor,     7,  1, 1, 2);
                pMainLayout->addWidget(plabX,             8,  1);
                pMainLayout->addWidget(m_pdeX,            8,  2);
                pMainLayout->addWidget(plabY,             9,  1);
                pMainLayout->addWidget(m_pdeY,            9,  2);
                pMainLayout->addWidget(plabZ,             10,  1);
                pMainLayout->addWidget(m_pdeZ,            10,  2);

                pMainLayout->addWidget(plabScatt,         11,  1);
                pMainLayout->addWidget(m_pdeScatter,      11,  2);

                pMainLayout->addWidget(plabDt,            12,  1);
                pMainLayout->addWidget(m_pdeDt,           12,  2);

                pMainLayout->addWidget(ppbRestart,        13, 1, 1, 2);

                pMainLayout->addWidget(plabSize,          14, 1);
                pMainLayout->addWidget(m_pdeParticleSize, 14, 2);

                pMainLayout->addWidget(pchAxes,           15, 1, 1, 2);
                pMainLayout->addWidget(m_plabFrameRate,   16, 1, 1, 2);

                pMainLayout->setColumnStretch(2,1);
            }
            pFrameLayout->addWidget(plabTitle);
            pFrameLayout->addLayout(pMainLayout);
            pFrameLayout->addStretch();
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }


    setReferenceLength(100.0);

//    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
    m_Timer.start(0);
    connect(&m_Timer, SIGNAL(timeout()), SLOT(update()));
}


void gl3dLorenz2::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dLorenz2");
    {
        s_NGroups = settings.value("NGroups",      s_NGroups).toInt();
        s_Size    = settings.value("ParticleSize", s_Size   ).toDouble();
        s_X0      = settings.value("X0",           s_X0     ).toDouble();
        s_Y0      = settings.value("Y0",           s_Y0     ).toDouble();
        s_Z0      = settings.value("Z0",           s_Z0     ).toDouble();
        s_Scatter = settings.value("Scatter",      s_Scatter).toDouble();
        s_dt      = settings.value("dt",           s_dt     ).toDouble();
    }
    settings.endGroup();
}


void gl3dLorenz2::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dLorenz2");
    {
        settings.setValue("NGroups",      s_NGroups);
        settings.setValue("ParticleSize", s_Size);
        settings.setValue("X0",           s_X0     );
        settings.setValue("Y0",           s_Y0     );
        settings.setValue("Z0",           s_Z0     );
        settings.setValue("Scatter",      s_Scatter);
        settings.setValue("dt",           s_dt     );
    }
    settings.endGroup();
}


void gl3dLorenz2::initializeGL()
{
    gl3dTestGLView::initializeGL();
#ifdef Q_OS_MAC
    return; // Compute shaders require OpenGL 4.3 whereas macOS only supports OpenGL 4.1
#else
    QString csrc, strange;
    csrc = ":/shaders/lorenz2/lorenz2_CS.glsl";
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
        m_locRadius = m_shadCompute.uniformLocation("radius");
        m_locDt = m_shadCompute.uniformLocation("dt");
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


void gl3dLorenz2::onRestart()
{
    s_X0 = m_pdeX->value();
    s_Y0 = m_pdeY->value();
    s_Z0 = m_pdeZ->value();
    s_NGroups = m_pieNGroups->value();
    s_Scatter = m_pdeScatter->value();
    s_dt = m_pdeDt->value();
    s_Size = m_pdeParticleSize->value();

    m_bResetParticles = true;
}


void gl3dLorenz2::onParticleSize()
{
    s_Size = m_pdeParticleSize->value();
}


void gl3dLorenz2::glMake3dObjects()
{
    if(m_bResetParticles)
    {
        const int stride = 8;

        int nParticles = GROUP_SIZE * s_NGroups;
        m_plabNParticles->setText(QString::asprintf("Number of particles =%d", nParticles));

        int buffersize = nParticles*stride;
        QVector<float> data(buffersize);

        int iv = 0;
        float state(0.0f);

        double X = m_pdeX->value();
        double Y = m_pdeY->value();
        double Z = m_pdeZ->value();
        Vector3d start(X,Y,Z);


        for(int i=0; i<nParticles; i++)
        {
            data[iv++] = start.x + QRandomGenerator::global()->generateDouble()*s_Scatter;
            data[iv++] = start.y + QRandomGenerator::global()->generateDouble()*s_Scatter;
            data[iv++] = start.z + QRandomGenerator::global()->generateDouble()*s_Scatter;
            data[iv++] = state;

            data[iv++] = glGetRed(  state); // for the surface shader
            data[iv++] = glGetGreen(state);
            data[iv++] = glGetBlue( state);
            data[iv++] = 1.0f; //alpha
        }

        if(!m_ssbParticle.create())
        {
            qDebug("Failed to create the VBO");
            return;
        }
        m_ssbParticle.bind();
        {
//            m_ssbParticle.setUsagePattern(QOpenGLBuffer::DynamicCopy);
            m_ssbParticle.allocate(data.data(), buffersize*sizeof(GLfloat));
        }
        m_ssbParticle.release();

//        printBuffer(m_ssbParticle, stride);

        m_bResetParticles = false;
    }
}


void gl3dLorenz2::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
#ifdef Q_OS_MAC
    return; // Compute shaders require OpenGL 4.3 whereas macOS only supports OpenGL 4.1
#else
    m_shadCompute.bind();
    {
        s_dt = m_pdeDt->value();
        m_shadCompute.setUniformValue(m_locRadius, 10.0f);
        m_shadCompute.setUniformValue(m_locDt, float(s_dt));

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbParticle.bufferId());
        glDispatchCompute(s_NGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
        getGLError();
    }
    m_shadCompute.release();
#endif
    m_shadPoint2.bind();
    {
        // Bind the VBO
//        glBindBuffer(GL_ARRAY_BUFFER, m_ssbParticle.bufferId());
        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);


        m_shadPoint2.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
        m_shadPoint2.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);
//        m_shadPoint2.setUniformValue(m_locPt2.m_Shape, 3.0f);
        m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, m_ClipPlanePos);

        m_ssbParticle.bind();
        {
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, 8 * sizeof(GLfloat));
            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, 8 * sizeof(GLfloat));

    //        glEnable(GL_POINT_SPRITE);
    //        getGLError();
            glPointSize(s_Size);
            getGLError();
    //        glEnable(GL_PROGRAM_POINT_SIZE);
    //        getGLError();

            glDrawArrays(GL_POINTS, 0, s_NGroups * GROUP_SIZE);

            getGLError();

            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
        }
        m_ssbParticle.release();
    }
    m_shadPoint2.release();

    m_stackInterval.push_back(QTime::currentTime().msecsSinceStartOfDay());
    double average = 0.0;
    for(int i=0; i<m_stackInterval.size()-1; i++)
        average += m_stackInterval.at(i+1)-m_stackInterval.at(i);
    average /= double(m_stackInterval.size()-1);
    m_plabFrameRate->setText(QString::asprintf("Frame rate = %4.1f Hz", 1000.0/average));
    m_stackInterval.pop_front();

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}




