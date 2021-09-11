/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QGuiApplication>
#include <QScreen>
#include <QFormLayout>
#include <QCheckBox>

#include "gl3dsolarsys.h"
#include <xfl3d/gl_globals.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/wt_globals.h>

double gl3dSolarSys::s_dt = 1.0; //day
double gl3dSolarSys::s_PlanetSize = 1000.0;

#define SCALEFACTOR 1.0e9


gl3dSolarSys::gl3dSolarSys(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Solar system");

    m_bResetPlanet = true;
    m_bResetStars  = true;
    m_bHalley = false;
    m_Elapsed = QDate::currentDate();

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMovePlanets()));

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

        pFrame->setPalette(palette);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QGridLayout*pParamsLayout = new QGridLayout;
            {
                QLabel *pLabInc = new QLabel(QString::asprintf("Increment (days) @%.0f Hz:", QGuiApplication::primaryScreen()->refreshRate()));
                pLabInc->setPalette(palette);
                m_pdeDt = new DoubleEdit(s_dt);
                m_pdeDt->setPalette(palette);
                QLabel *pLabSize = new QLabel("Planet size factor:");
                pLabSize->setPalette(palette);
                m_pdePlanetSize = new DoubleEdit;
                m_pdePlanetSize->setPalette(palette);
                m_pdePlanetSize->setValue(s_PlanetSize);

                pParamsLayout->addWidget(pLabInc,         1, 1);
                pParamsLayout->addWidget(m_pdeDt,         1, 2);
                pParamsLayout->addWidget(pLabSize,        2, 1);
                pParamsLayout->addWidget(m_pdePlanetSize, 2, 2);
            }

            QFont fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            QFontMetrics fm(fnt);

            m_plabDate = new QLabel(" ");
            m_plabDate->setPalette(palette);
            m_plabDate->setFont(fnt);

            m_plabHalley = new QLabel("\n\n\n\n");
            m_plabHalley->setPalette(palette);
            m_plabHalley->setFont(fnt);

            m_plabHalley->setMinimumWidth(fm.averageCharWidth()*30);
            m_plabHalley->setMinimumHeight(fm.height()*6);

            QCheckBox *pchAxes = new QCheckBox("Axes");
            QCheckBox *pchHalleyPlane = new QCheckBox("Halley's comet");
            pchAxes->setChecked(m_bAxes);
            pchHalleyPlane->setChecked(m_bHalley);
            connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));
            connect(pchHalleyPlane, SIGNAL(clicked(bool)), SLOT(onHalley(bool)));

            pMainLayout->addLayout(pParamsLayout);
            pMainLayout->addWidget(m_plabDate);
            pMainLayout->addWidget(m_plabHalley);
            pMainLayout->addWidget(pchHalleyPlane);
            pMainLayout->addWidget(pchAxes);
        }

        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    makePlanets();


    // save the current light
    m_RefLight = s_Light;
    // position the light at the Sun's center
    setLightPos(0,0,0);
    setSpecular(0);

    setReferenceLength(m_Planet.last().distance()/SCALEFACTOR);
    reset3dScale();

    onRestart();
}


gl3dSolarSys::~gl3dSolarSys()
{
    //restore the light's position
    setLight(m_RefLight);
}


void gl3dSolarSys::hideEvent(QHideEvent *pEvent)
{
    //restore the light's position
    setLight(m_RefLight);

    gl3dTestGLView::hideEvent(pEvent);
}


void gl3dSolarSys::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_Timer.isActive()) m_Timer.stop();
            else                   m_Timer.start(16);
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dSolarSys::initializeGL()
{
    gl3dTestGLView::initializeGL();
    glMakeIcoSphere(3);
}


void gl3dSolarSys::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    Vector3d pos;

    for(int i=0; i<m_Planet.size(); i++)
    {
        Planet const &planet = m_Planet.at(i);
        pos = planet.position()/SCALEFACTOR; // million km

        m_matModel.setToIdentity();
        m_matModel.rotate(planet.m_omega, 0.0f,0.0f,1.0f);
        m_matModel.rotate(planet.m_i,     0.0f,1.0f,0.0f);

        vmMat = m_matView*m_matModel;
        pvmMat = m_matProj*vmMat;

        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
        }
        m_shadLine.release();

        paintSegments(m_vboCircle[i], planet.m_Color, 1, Line::SOLID, false);
        paintSphere(pos, planet.m_Radius/SCALEFACTOR*s_PlanetSize, planet.m_Color, true);
        glRenderText(pos.x, pos.y, pos.z+planet.m_Radius/SCALEFACTOR*s_PlanetSize*1.1,
                     planet.m_Name, planet.m_Color);

        if(i==5)
        {
            //paint Saturn's disk
            Planet const &Saturn = m_Planet.at(5);
//            QMatrix4x4 scale, rotate, translate;
            m_matModel.translate(Saturn.m_var[0]/SCALEFACTOR, Saturn.m_var[1]/SCALEFACTOR, Saturn.m_var[2]/SCALEFACTOR);
            m_matModel.rotate(-27.0f, sqrtf(2)/2.0f, sqrtf(2)/2.0f,0.0f);
            m_matModel.scale(s_PlanetSize);
//            m_pvmMatrix = m_pvmMatrix*translate*rotate*scale;

            vmMat = m_matView*m_matModel;
            pvmMat = m_matProj*vmMat;

            m_shadSurf.bind();
            {
                m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
                m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
            }
            m_shadSurf.release();

            QColor clr = Saturn.m_Color;
            clr.setAlpha(175);
            paintTriangleFan(m_vboSaturnDisk, clr, true, false);

            m_matModel.setToIdentity();
            vmMat = m_matView*m_matModel;
            pvmMat = m_matProj*vmMat;
            m_shadSurf.bind();
            {
                m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
                m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
            }
            m_shadSurf.release();
        }
    }

    if(m_bHalley)
    {
        //paint Halley's comet

        m_matModel.setToIdentity();
        m_matModel.rotate(m_Halley.m_omega, 0.0f,0.0f,1.0f);
        m_matModel.rotate(m_Halley.m_i,     0.0f,1.0f,0.0f);

        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);

        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();

        QColor clr = m_Halley.m_Color;
        clr.setAlpha(75);
        paintTriangleFan(m_vboHalleyEllipse, clr, false, false);

        pos = m_Halley.position()/SCALEFACTOR; // million km
        paintSphere(pos, 0.005/m_glScalef, m_Halley.m_Color, true);
        glRenderText(pos.x, pos.y, pos.z*s_PlanetSize*1.1, m_Halley.m_Name, s_TextColor);
    }

    // paint the sun
    m_matModel.setToIdentity();
    vmMat = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;

    float radius = float(1.3927e9/SCALEFACTOR);
    if(m_bUse120StyleShaders)
        paintSphere(Vector3d(), radius, Qt::yellow, false);
    else
    {
        m_shadPoint.bind();
        {
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix, vmMat);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
        }
        m_shadPoint.release();
        paintPoints(m_vboLightSource, radius*m_glScalef*500.0, 0, false, Qt::yellow, 4);
    }

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dSolarSys::glMake3dObjects()
{
    if(m_bResetPlanet)
    {
        for(int i=0; i<m_vboCircle.size(); i++)
        {
            if(m_vboCircle[i].isCreated()) m_vboCircle[i].destroy();
        }
        m_vboCircle.resize(m_Planet.size());
        for(int i=0; i<m_Planet.size(); i++)
        {
            Planet const &planet = m_Planet.at(i);
            glMakeCircle(planet.distance()/SCALEFACTOR, Vector3d(), m_vboCircle[i]);
        }
        glMakeDisk(270.0e6/2.0/SCALEFACTOR, Vector3d(), m_vboSaturnDisk); // 270 000 km diameter

        // make Halley's ellipse
        double a = 17.834 * 1.495978707e11; // semi-major axis length
        double e = 0.96714; //excentricity
        glMakeEllipseFan(a/SCALEFACTOR, e, Vector3d(), m_vboHalleyEllipse);

       m_bResetPlanet = false;
    }

    if(m_bResetStars)
    {
        int buffersize = m_Stars.size()*4;
        QVector<float> pts(buffersize);
        int iv =0;
        for(int i=0; i<m_Stars.size(); i++)
        {
            pts[iv++] = m_Stars.at(i).m_Position.xf()/SCALEFACTOR;
            pts[iv++] = m_Stars.at(i).m_Position.yf()/SCALEFACTOR;
            pts[iv++] = m_Stars.at(i).m_Position.zf()/SCALEFACTOR;
            pts[iv++] = 1.0f;
        }

        if(m_vboStars.isCreated()) m_vboStars.destroy();
        m_vboStars.create();
        m_vboStars.bind();
        m_vboStars.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
        m_vboStars.release();
    }
}


void gl3dSolarSys::onRestart()
{
    s_dt = m_pdeDt->value();
    s_PlanetSize = m_pdePlanetSize->value();

    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
    m_Timer.start(period);
}


void gl3dSolarSys::onPlanetSize(int size)
{
    size = std::max(size, 1);
    s_PlanetSize = double(size);
    update();
}


void gl3dSolarSys::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSolarSystem");
    {
        s_dt = settings.value("deltat", s_dt).toDouble();
        s_PlanetSize = settings.value("PlanetSizeCoef", s_PlanetSize).toDouble();
    }
    settings.endGroup();
}


void gl3dSolarSys::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSolarSystem");
    {
         settings.setValue("deltat", s_dt);
         settings.setValue("PlanetSizeCoef", s_PlanetSize);
    }
    settings.endGroup();
}


void gl3dSolarSys::onHalley(bool bShow)
{
    m_bHalley = bShow;
    update();
}


void gl3dSolarSys::makePlanets()
{
    Planet::setCentralMass(1.98847e30); //kg

    double Distance(0), angle(0), v(0);

    m_Planet.resize(8);

    int p=0;
    m_Planet[p].m_Name     = "Mercury";
    m_Planet[p].m_Color    = QColor(125, 125, 125);
    m_Planet[p].m_Radius   = 2.4397 * 1.e6; // meters
    m_Planet[p].m_i = 7.005; // degrees
    m_Planet[p].m_a = 57.909e9; // meters
    m_Planet[p].m_e = 0.205630;
    m_Planet[p].m_omega= 29.124; // degrees
    Distance = 57.91e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Venus";
    m_Planet[p].m_Color    = QColor(255, 150, 50);
    m_Planet[p].m_Radius   = 6.0518 * 1.e6;
    m_Planet[p].m_i = 3.39458; // degrees
    m_Planet[p].m_a = 108.208e9; // meters
    m_Planet[p].m_e = 0.006772;
    m_Planet[p].m_omega= 54.884;
    Distance = 108.2e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Earth";
    m_Planet[p].m_Color    = QColor(100, 100, 255);
    m_Planet[p].m_Radius   = 6.371 * 1.e6;
    m_Planet[p].m_i = 3.39458; // degrees
    m_Planet[p].m_a = 149.256e9; // meters
    m_Planet[p].m_e = 0.0167086;
    m_Planet[p].m_omega= 114.20783;
    Distance = 149.6e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Mars";
    m_Planet[p].m_Color    = QColor(205, 100, 100);
    m_Planet[p].m_Radius   = 3.3895 * 1.e6;
    m_Planet[p].m_i = 1.850; // degrees
    m_Planet[p].m_a = 227.9392e9; // meters
    m_Planet[p].m_e = 0.0934;
    m_Planet[p].m_omega= 286.502; // degrees
    Distance = 227.9e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Jupiter";
    m_Planet[p].m_Color    = QColor(150, 95, 75);
    m_Planet[p].m_Radius   = 69.911 * 1.e6;
    m_Planet[p].m_i = 1.303; // degrees
    m_Planet[p].m_a = 778.57e9; // meters
    m_Planet[p].m_e = 0.0489;
    m_Planet[p].m_omega= 273.867; // degrees
    Distance = 778.5e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Saturn";
    m_Planet[p].m_Color    = QColor(150, 95, 150);
    m_Planet[p].m_Radius   = 58.232 * 1.e6;
    m_Planet[p].m_i = 2.485; // degrees
    m_Planet[p].m_a = 1433.53e9; // meters
    m_Planet[p].m_e = 0.0565;
    m_Planet[p].m_omega= 339.392; // degrees
    Distance = 1434e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Uranus";
    m_Planet[p].m_Color    = QColor(100, 35, 55);
    m_Planet[p].m_Radius   = 25.362 * 1.e6;
    m_Planet[p].m_i = 0.773; // degrees
    m_Planet[p].m_a = 2875.04e9; // meters
    m_Planet[p].m_e = 0.046381;
    m_Planet[p].m_omega= 96.998857; // degrees
    Distance = 2871e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    p++;
    m_Planet[p].m_Name     = "Neptune";
    m_Planet[p].m_Color    = QColor(50, 50, 175);
    m_Planet[p].m_Radius   = 24.622 * 1.e6;
    m_Planet[p].m_i = 1.767975; // degrees
    m_Planet[p].m_a = 4495e9; // meters
    m_Planet[p].m_e = 0.008678;
    m_Planet[p].m_omega= 276.336; // degrees
    Distance = 4495e9;
    angle =  (double(rand())/double(RAND_MAX) )*2.0*PI;
    m_Planet[p].setPosition(Distance*cos(angle), Distance*sin(angle));
    m_Planet[p].setCircularVelocity();

    m_Halley.m_Name = "Halley";
    m_Halley.m_Color = Qt::gray;

    m_Halley.m_Radius = 1.0; // whatever
    m_Halley.m_a = 17.834 * 1.495978707e11; // semi-major axis length
    m_Halley.m_e = 0.96714; //excentricity
    m_Halley.m_i = -180.0f+162.26f;
    m_Halley.m_omega = 111.33;
    Distance = m_Halley.m_a*(m_Halley.m_e+1.0); // apogee distance to Sun
    m_Halley.setPosition(Distance, 0.0);
    v = sqrt(GRAVITY*Planet::s_CentralMass*(2.0/m_Halley.distance()-1.0/m_Halley.m_a)); // vis-viva equation
    // Halley's orbit is retrograde; it orbits the Sun in the opposite direction to the planets,
    // or clockwise from above the Sun's north pole
    m_Halley.setVelocity(0,-v);
    m_Halley.setRefEnergy();
}


void gl3dSolarSys::onMovePlanets()
{
//    if(m_DynTimer.isActive()) return;
    s_PlanetSize = m_pdePlanetSize->value();

    s_dt = m_pdeDt->value(); // days
    m_Elapsed = m_Elapsed.addDays(s_dt);
    double dt = s_dt*24*3600;//seconds
    int nSteps = 20;
    dt /= nSteps;

    for(int p=0; p<m_Planet.size(); p++)
    {
        m_Planet[p].rk4_step(dt, nSteps);
    }
//    m_Moon.rk4_step(dt, nSteps);
    m_Halley.rk4_step(dt, nSteps);


    m_plabDate->setText(QString::asprintf("%2d years %2d months %3d days", m_Elapsed.year()-QDate::currentDate().year(), m_Elapsed.month(), m_Elapsed.day()));

    QString strange;
    m_Halley.list(strange);
    m_plabHalley->setText(strange);

    update();
}
