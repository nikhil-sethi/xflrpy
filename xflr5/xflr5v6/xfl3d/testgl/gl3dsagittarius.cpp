/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QScreen>
#include <QtConcurrent/QtConcurrent>
#include <QAbstractItemView>
#include <QGridLayout>
#include <QPushButton>

#include "gl3dsagittarius.h"
#include <xflcore/xflcore.h>
#include <xflcore/displayoptions.h>
#include <xfl3d/gl_globals.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/graph.h>
#include <xflgraph/curve.h>
//#include <xflgraph/controls/graphoptions.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflwidgets/wt_globals.h>

bool gl3dSagittarius::s_bMultithread = false;
int gl3dSagittarius::s_nStepsPerDay = 100;
double gl3dSagittarius::s_dt = 10.0;

#define NCOMPONENTS 8
#define POINTWIDTH 5.0f
#define SCALEFACTOR 1.0e14

gl3dSagittarius::gl3dSagittarius(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Sagittarius A*");

    m_Started = QDate::currentDate(); // to start with a valid date
    m_Current = m_Started;

    m_bResetStars = true;

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMoveStars()));

    makeStars();

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, s_TextColor);
        palette.setColor(QPalette::Text, s_TextColor);
        QColor clr = s_BackgroundColor;
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QGridLayout*pParamsLayout = new QGridLayout;
            {
                QLabel *pLabTitle = new QLabel("In honor of Genzel, Ghez and Penrose");
                pLabTitle->setStyleSheet("font: bold");

                QLabel *pLabInc = new QLabel(QString::asprintf("Increment (days) @%.0f Hz:", QGuiApplication::primaryScreen()->refreshRate()));
                m_pdeDt = new DoubleEdit(s_dt);
                m_pdeDt->setPalette(palette);
                m_pdeDt->setToolTip("This defines the time step used to move the stars.<br>"
                                    "If the calculation is fast enough, the positions are updated in time for the next screen refresh.<br>"
                                    "Recommendation: 10 days for a smooth animation @60Hz.");


                QLabel *plabSteps = new QLabel("RK4 steps/day:");

                m_pieSteps = new IntEdit(s_nStepsPerDay);
                m_pieSteps->setPalette(palette);
                m_pieSteps->setToolTip("This defines the number of Runge-Kutta steps per day.<br>"
                                       "Increase the number to reduce the error of the RK4 method and to reduce the energy loss.<br>"
                                       "Too high a number and the calculation will be slower than the screen refresh rate.<br>"
                                       "Recommendation: 50 or more.");

                m_pchMultiThread = new QCheckBox("Multi-threaded");
                m_pchMultiThread->setPalette(palette);
                m_pchMultiThread->setToolTip("If activated, the movement of each star is calculated in a separate thread.<br>"
                                             "This may not be faster than single-threading due to the overhead cost of thread creation.<br>"
                                             "May become beneficial when the number of RK4 steps/day increases above a few hundred.");
                m_pchMultiThread->setChecked(s_bMultithread);

                pParamsLayout->addWidget(pLabTitle,          1, 1, 1, 2);
                pParamsLayout->addWidget(pLabInc,            2, 1);
                pParamsLayout->addWidget(m_pdeDt,            2, 2);
                pParamsLayout->addWidget(plabSteps,          3, 1);
                pParamsLayout->addWidget(m_pieSteps,         3, 2);
                pParamsLayout->addWidget(m_pchMultiThread,   4, 1, 1, 2);
            }

            m_pcbStar = new QComboBox;
            for(int i=0; i<m_Star.size(); i++)
                m_pcbStar->addItem(m_Star.at(i).m_Name);
            m_pcbStar->setCurrentIndex(1); //S2
            connect(m_pcbStar, SIGNAL(activated(int)), SLOT(onStarSelection()));
            m_plabInfo = new QLabel("\n\n\n");
            m_plabInfo->setPalette(palette);
            m_plabInfo->setFont(DisplayOptions::tableFont());

            m_pGraphDistWt = new GraphWt;
            m_pGraphDistWt->setGraph(&m_GraphDist);
            m_pGraphDistWt->setDefaultSize(QSize(300,175));
/*            m_GraphDist.setDefaults(true);
            m_GraphDist.setCurveModel(new CurveModel);
            m_GraphDist.setScaleType(GRAPH::EXPANDING);
            m_GraphDist.setAuto(true);*/
            m_GraphDist.setXTitle("Years");
            m_GraphDist.setYTitle("Distance (a.u.)");
            m_GraphDist.setMargin(43);

            m_pGraphVelWt = new GraphWt;
            m_pGraphVelWt->setGraph(&m_GraphVel);
            m_pGraphVelWt->setDefaultSize(QSize(300,175));
/*            m_GraphVel.setDefaults(true);
            m_GraphVel.setCurveModel(new CurveModel);
            m_GraphVel.setScaleType(GRAPH::EXPANDING);
            m_GraphVel.setAuto(true);*/
            m_GraphVel.setXTitle("Years");
            m_GraphVel.setYTitle("Velocity (%c)");
            m_GraphVel.setMargin(43);

            QCheckBox *pchAxes = new QCheckBox("Axes");
            pchAxes->setChecked(m_bAxes);
            connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

            QPushButton *ppbRestart = new QPushButton("Restart");
            connect(ppbRestart, SIGNAL(clicked()), SLOT(onRestart()));

            QLabel *pWikiLink = new QLabel;
            pWikiLink->setText("<a href=https://en.wikipedia.org/wiki/Sagittarius_A*>https://en.wikipedia.org/wiki/Sagittarius_A*</a>");
            pWikiLink->setOpenExternalLinks(true);
            pWikiLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

            pMainLayout->addLayout(pParamsLayout);

            pMainLayout->addWidget(m_pcbStar);
            pMainLayout->addWidget(m_plabInfo);
            pMainLayout->addWidget(m_pGraphDistWt);
            pMainLayout->addWidget(m_pGraphVelWt);
            pMainLayout->addWidget(pchAxes);
            pMainLayout->addWidget(ppbRestart);
            pMainLayout->addWidget(pWikiLink);
        }

        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    setReferenceLength(20.0);

    onRestart();

    reset3dScale();
}


gl3dSagittarius::~gl3dSagittarius()
{

}


Planet const &gl3dSagittarius::selectedStar() const
{
    int index = m_pcbStar->currentIndex();
    return m_Star.at(index);
}


void gl3dSagittarius::onStarSelection()
{
    if(m_GraphDist.curve(0))
    {
        m_GraphDist.curve(0)->reset();
        m_GraphDist.curve(0)->setColor(selectedStar().m_Color);
    }
    if(m_GraphVel.curve(0))
    {
        m_GraphVel.curve(0)->reset();
        m_GraphVel.curve(0)->setColor(selectedStar().m_Color);
    }

    m_bResetStars = true;
    m_Started = m_Current; // to make sure the date is valid
}


void gl3dSagittarius::glRenderView()
{
/*    m_Shader_Line.bind();
    {
        m_Shader_Line.setUniformValue(m_LineLoc.m_vmMatrix, m_matView*m_matModel);
        m_Shader_Line.setUniformValue(m_LineLoc.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_Shader_Line.release();

    m_Shader_Surf.bind();
    {
        m_Shader_Surf.setUniformValue(m_SurfLoc.m_vmMatrix, m_matView);
        m_Shader_Surf.setUniformValue(m_SurfLoc.m_pvmMatrix, m_pvmMatrix);
    }
    m_Shader_Surf.release(); */
    Vector3d pos;
    QMatrix4x4 vmMat, pvmMat;

    for(int i=0; i<m_Star.size(); i++)
    {
        Planet const &star = m_Star.at(i);

        m_matModel.setToIdentity();
        m_matModel.rotate(star.m_i, cos(star.m_Omega*PI/180.0), sin(star.m_Omega*PI/180.0), 0.0f);
        m_matModel.rotate(star.m_omega, 0.0,0.0,1.0f);
//        m_pvmMatrix = m_matProj * m_matView * m_matModel;

        vmMat = m_matView*m_matModel;
        pvmMat = m_matProj*vmMat;

        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
        }
        m_shadLine.release();

        pos = star.position()/SCALEFACTOR;
        paintSphere(pos, 0.01/m_glScalef, star.m_Color, true);
        paintLineStrip(m_vboEllipse[i], star.m_Color, 1, Line::SOLID);
        glRenderText(pos.x+0.013/m_glScalef, pos.y+0.013/m_glScalef, pos.z+0.013/m_glScalef, star.m_Name, star.m_Color);
    }
    m_matModel.setToIdentity();

    // paint the selected disk
    Planet const &star = selectedStar();
    QMatrix4x4 hrotate;
    m_matModel.setToIdentity();
    m_matModel.rotate(star.m_i, cos(star.m_Omega*PI/180.0), sin(star.m_Omega*PI/180.0), 0.0f);
    m_matModel.rotate(star.m_omega, 0.0,0.0,1.0f);
//    m_pvmMatrix = m_matProj * m_matView * hrotate;
    vmMat = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
    QColor clr = star.m_Color;
    clr.setAlpha(125);
    paintTriangleFan(m_vboEllipseFan, clr, false, false);

    m_matModel.setToIdentity();
    vmMat = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;
//    m_pvmMatrix = m_matProj * m_matView;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();


    // paint the black hole
    paintSphere(Vector3d(), 0.01/m_glScalef, Qt::black, true);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dSagittarius::glMake3dObjects()
{
    if(m_bResetStars)
    {
        for(int i=0; i<m_vboEllipse.size(); i++)
        {
            if(m_vboEllipse[i].isCreated()) m_vboEllipse[i].destroy();
        }
        m_vboEllipse.resize(m_Star.size());

        for(int i=0; i<m_Star.size(); i++)
        {
            Planet const &planet = m_Star.at(i);
            glMakeEllipseLineStrip(planet.m_a/SCALEFACTOR, planet.m_e, Vector3d(), m_vboEllipse[i]);
            if(i==m_pcbStar->currentIndex())
            {
                glMakeEllipseFan(planet.m_a/SCALEFACTOR, planet.m_e, Vector3d(), m_vboEllipseFan);
            }
        }

/*        m_vboTrail.create();
        m_vboTrail.bind();
        m_vboTrail.allocate(m_Particle.constData(), m_Particle.size() * sizeof(GLfloat));
        m_vboTrail.release();
*/
       m_bResetStars = false;
    }
}


void gl3dSagittarius::onRestart()
{
    m_Started = m_Current; // to make sure the date is valid
    s_dt = m_pdeDt->value();

    makeStars();

    Curve *pCurveDist = m_GraphDist.curve(0);
    if(!pCurveDist) pCurveDist = m_GraphDist.addCurve("Distance");
    pCurveDist->setColor(selectedStar().m_Color);
    pCurveDist->reset();
    Curve *pCurveVel = m_GraphVel.curve(0);
    if(!pCurveVel) pCurveVel = m_GraphVel.addCurve("Velocity (%c)");
    pCurveVel->setColor(selectedStar().m_Color);
    pCurveVel->reset();

    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
    m_Timer.start(period);
}


void gl3dSagittarius::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSagittarius");
    {
        s_dt = settings.value("deltat", s_dt).toDouble();
        s_nStepsPerDay = settings.value("NSteps", s_nStepsPerDay).toInt();
        s_bMultithread = settings.value("MultiThreaded", s_bMultithread).toBool();
    }
    settings.endGroup();
}


void gl3dSagittarius::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSagittarius");
    {
         settings.setValue("deltat",        s_dt);
         settings.setValue("NSteps",        s_nStepsPerDay);
         settings.setValue("MultiThreaded", s_bMultithread);
    }
    settings.endGroup();
}


void gl3dSagittarius::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
        {
            if(m_Timer.isActive())
                m_Timer.stop();
            else
            {
                int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
                m_Timer.start(period);
            }
            break;
        }
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dSagittarius::makeStars()
{
    Planet::s_CentralMass = 4.154e6 * 2e30;// kg
//    Planet::s_CentralMass = 1.0e6;

    m_Star.resize(9);

    m_Star[0].m_Name     = "S1";
    m_Star[1].m_Name     = "S2";
    m_Star[2].m_Name     = "S8";
    m_Star[3].m_Name     = "S12";
    m_Star[4].m_Name     = "S13";
    m_Star[5].m_Name     = "S14";
    m_Star[6].m_Name     = "S55";
    m_Star[7].m_Name     = "S62";
    m_Star[8].m_Name     = "S4714";

    m_Star[0].setOrbit(0.5950, 0.5560, 119.14, 342.04, 122.30);
    m_Star[1].setOrbit(0.1251, 0.8843, 133.91, 228.07,  66.25);
    m_Star[2].setOrbit(0.4047, 0.8031, 74.37,  315.43, 346.70);
    m_Star[3].setOrbit(0.2987, 0.8883, 33.56,  230.10, 317.90);
    m_Star[4].setOrbit(0.2641, 0.4250, 24.70,   74.50, 245.20);
    m_Star[5].setOrbit(0.2863, 0.9761, 100.59, 226.38, 334.59);
    m_Star[6].setOrbit(0.1078, 0.721,  150.0,   325.0,  332.0); //https://en.wikipedia.org/wiki/S55_(star)
    m_Star[7].setOrbit(0.0905, 0.9760, 72.76,  122.61,  42.62);
    m_Star[8].setOrbit(0.102,  0.985,  127.7,  129.28, 357.25);

    double vel = 0;
    for(int i=0; i<m_Star.size(); i++)
    {
        Planet &star = m_Star[i];
        star.m_Color = xfl::getColor(i);
        // set a default position
        star.m_var[0] = star.m_a + star.m_a*star.m_e;
        star.m_var[1] = 0.0;
        vel = sqrt(GRAVITY*Planet::s_CentralMass*(2.0/star.distance()-1.0/star.m_a)); // vis-viva equation
        star.setVelocity(0,vel);
        star.setRefEnergy();
    }
}


void gl3dSagittarius::onMoveStars()
{
//    auto t0 = std::chrono::high_resolution_clock::now();

   s_dt = m_pdeDt->value(); // days
   m_Current = m_Current.addDays(s_dt);

    s_nStepsPerDay = m_pieSteps->value();
    s_bMultithread = m_pchMultiThread->isChecked();

    double dt = s_dt*24*3600;//seconds
    dt = dt/double(s_nStepsPerDay);

    if(s_bMultithread)
    {
        QFutureSynchronizer<void> futureSync;
        for(int irow=0; irow<m_Star.size(); irow++)
        {
//            futureSync.addFuture(QtConcurrent::run(this, &gl3dSagittarius::rk4_step, &m_Star[irow]));
            futureSync.addFuture(QtConcurrent::run(&m_Star[irow], &Planet::rk4_step, dt, s_nStepsPerDay));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int p=0; p<m_Star.size(); p++)
        {
            m_Star[p].rk4_step(dt, s_nStepsPerDay);
        }
    }
/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("gl3dSagittarius::onMoveStars: %gms", double(duration)/1000.0);*/

    QString strange;
    Planet const &star = selectedStar();
    star.list(strange);
    m_plabInfo->clear();
    m_plabInfo->setText(strange);
//qDebug()<<star.distance()/AU<<star.velocity()/LIGHTSPEED*100.0;
    m_GraphDist.curve(0)->appendPoint(double(m_Started.daysTo(m_Current))/365.0, star.distance()/AU);
    m_GraphVel.curve(0)->appendPoint( double(m_Started.daysTo(m_Current))/365.0, star.velocity()/LIGHTSPEED*100.0);
    m_GraphDist.resetLimits();
//    m_GraphDist.invalidate();
    m_pGraphDistWt->update();
    m_GraphVel.resetLimits();
//    m_GraphVel.invalidate();
    m_pGraphVelWt->update();

    update();
}





