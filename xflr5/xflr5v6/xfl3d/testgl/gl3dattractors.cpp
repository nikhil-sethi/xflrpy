/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>
#include <QLabel>
#include <QRandomGenerator>

#include "gl3dattractors.h"

#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/wt_globals.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linemenu.h>
#include <xfl3d/gl_globals.h>

#define NATTRACTORS 13

int gl3dAttractors::s_iAttractor(0);
int gl3dAttractors::s_NTrace(11);
int gl3dAttractors::s_TailSize  = 729;
LineStyle gl3dAttractors::s_ls = {true, Line::SOLID, 1, QColor(205,92,92), Line::NOSYMBOL, QString()};
bool gl3dAttractors::s_bDynColor(true);


gl3dAttractors::gl3dAttractors(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setWindowTitle("Strange attractors");
    m_bResetAttractor = true;
    m_iLead = 0;

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
            QGridLayout*pParamLayout = new QGridLayout;
            {
                m_prbAttractors.resize(NATTRACTORS);
                m_prbAttractors[0]  = new QRadioButton("Lorenz");
                m_prbAttractors[1]  = new QRadioButton("Newton-Leipnik");
                m_prbAttractors[2]  = new QRadioButton("Thomas");
                m_prbAttractors[3]  = new QRadioButton("Dadras");
                m_prbAttractors[4]  = new QRadioButton("Chen-Lee");
                m_prbAttractors[5]  = new QRadioButton("Aizawa");
                m_prbAttractors[6]  = new QRadioButton("Rössler");
                m_prbAttractors[7]  = new QRadioButton("Sprott");
                m_prbAttractors[8]  = new QRadioButton("Four wings");
                m_prbAttractors[9]  = new QRadioButton("Halvorsen");
                m_prbAttractors[10] = new QRadioButton("Rabinovich-Fabrikant");
                m_prbAttractors[11] = new QRadioButton("Nosé-Hoover");
                m_prbAttractors[12] = new QRadioButton("TSUCS 1");

                for(int i=0; i<m_prbAttractors.size(); i++)
                    connect(m_prbAttractors[i], SIGNAL(clicked()), SLOT(onAttractor()));
                if(s_iAttractor<m_prbAttractors.size())
                    m_prbAttractors[s_iAttractor]->setChecked(true);
                else
                {
                    s_iAttractor = 0;
                    m_prbAttractors.first()->setChecked(true);
                }

                QLabel *plabNTrace = new QLabel("Nbr. traces=");
                m_pieNTrace = new IntEdit(s_NTrace);
                connect(m_pieNTrace, SIGNAL(valueChanged()), SLOT(onRandomSeed()));

                QLabel *plabTailSize = new QLabel("Tail length=");
                m_pieTailSize = new IntEdit(s_TailSize);
                connect(m_pieTailSize, SIGNAL(valueChanged()), SLOT(onRandomSeed()));

                QLabel *plabSpeed = new QLabel("Increment:");
                m_pslSpeed = new QSlider(Qt::Horizontal);
                m_pslSpeed->setMinimum(00);
                m_pslSpeed->setMaximum(300);
                m_pslSpeed->setTickInterval(25);
                m_pslSpeed->setValue(100);

                m_plbStyle  = new LineBtn(s_ls);
                m_plbStyle->setPalette(palette);
                connect(m_plbStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onLineStyle(LineStyle)));

                m_pchLeadingSphere = new QCheckBox("Leading spheres");
                m_pchLeadingSphere->setChecked(true);

                m_pchDynColor = new QCheckBox("Dynamic colour");
                m_pchDynColor->setChecked(s_bDynColor);

                QCheckBox *pchAxes = new QCheckBox("Axes");
                pchAxes->setChecked(true);
                connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

                for(int i=0; i<m_prbAttractors.size(); i++)
                    pParamLayout->addWidget(m_prbAttractors[i], i+1, 1, 1 , 2);

                pParamLayout->addWidget(plabNTrace,         NATTRACTORS+1, 1);
                pParamLayout->addWidget(m_pieNTrace,        NATTRACTORS+1, 2);
                pParamLayout->addWidget(plabTailSize,       NATTRACTORS+2, 1);
                pParamLayout->addWidget(m_pieTailSize,      NATTRACTORS+2, 2);
                pParamLayout->addWidget(plabSpeed,          NATTRACTORS+3, 1);
                pParamLayout->addWidget(m_pslSpeed,         NATTRACTORS+3, 2);
                pParamLayout->addWidget(m_plbStyle,         NATTRACTORS+4, 1, 1, 2);
                pParamLayout->addWidget(m_pchDynColor,      NATTRACTORS+5, 1, 1, 2);
                pParamLayout->addWidget(m_pchLeadingSphere, NATTRACTORS+6, 1, 1, 2);
                pParamLayout->addWidget(pchAxes,            NATTRACTORS+7, 1, 1, 2);
                pParamLayout->setColumnStretch(1,1);
                pParamLayout->setColumnStretch(2,2);
            }

            pFrameLayout->addLayout(pParamLayout);
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    onRandomSeed();
    connect(&m_Timer, SIGNAL(timeout()), SLOT(moveThem()));
}


/**
 * The user has changed the color of the current curve
 */
void gl3dAttractors::onLineStyle(LineStyle)
{
    LineMenu *pLineMenu = new LineMenu(nullptr, false);
    pLineMenu->initMenu(s_ls);
    pLineMenu->exec(QCursor::pos());
    s_ls = pLineMenu->theStyle();
    m_plbStyle->setTheStyle(s_ls);
}


void gl3dAttractors::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAttractors");
    {
        s_iAttractor = settings.value("Attractor", s_iAttractor).toInt();
        s_NTrace     = settings.value("NTrace",    s_NTrace).toInt();
        s_TailSize   = settings.value("TailSize",  s_TailSize).toInt();
        s_bDynColor  = settings.value("DynColor",   s_bDynColor).toBool();
        s_ls.loadSettings(settings, "LineStyle");
    }
    settings.endGroup();
}


void gl3dAttractors::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAttractors");
    {
        settings.setValue("Attractor", s_iAttractor);
        settings.setValue("NTrace",    s_NTrace);
        settings.setValue("TailSize",  s_TailSize);
        settings.setValue("DynColor",  s_bDynColor);
        s_ls.saveSettings(settings, "LineStyle");
    }
    settings.endGroup();
}


void gl3dAttractors::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_Timer.isActive()) m_Timer.stop();
            else                   m_Timer.start(16);
            break;
        case Qt::Key_Escape:
            showNormal();
            break;

        case Qt::Key_F10:
            moveThem();
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dAttractors::glRenderView()
{
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView);
    }
    m_shadLine.release();

    paintColourSegments(m_vboTrace, s_ls);

    if(m_pchLeadingSphere->isChecked())
    {
        QColor clr;
        for(int i=0; i<m_Trace.size(); i++)
        {
            if(s_bDynColor)
            {
                QVector<double> const &velocity = m_Velocity.at(i);
                clr.setRedF(  xfl::GLGetRed(  velocity.at(m_iLead)/m_MaxVelocity));
                clr.setGreenF(xfl::GLGetGreen(velocity.at(m_iLead)/m_MaxVelocity));
                clr.setBlueF( xfl::GLGetBlue( velocity.at(m_iLead)/m_MaxVelocity));
            }
            else
                clr = s_ls.m_Color;

            paintSphere(m_Trace.at(i).at(m_iLead), 0.0061f/m_glScalef, clr, true);
        }
    }
    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dAttractors::glMake3dObjects()
{
    if(m_bResetAttractor)
    {
        s_bDynColor = m_pchDynColor->isChecked();

        int buffersize =  s_NTrace
                         *(s_TailSize-1)  // NSegments
                         *2*(3+4);     // 2 vertices * (3 coordinates+ 4 color components)
        QVector<float> buffer(buffersize);

        int ip0(0), ip1(0);

        int iv = 0;
        for(int i=0; i<m_Trace.size(); i++)
        {
            QVector<Vector3d> const &trace = m_Trace.at(i);
            QVector<double> &velocity = m_Velocity[i];
            for(int j=1; j<trace.size(); j++)
            {
                ip0 = (m_iLead+j-1)%s_TailSize;
                ip1 = (m_iLead+j  )%s_TailSize;
                buffer[iv++] = trace[ip0].xf();
                buffer[iv++] = trace[ip0].yf();
                buffer[iv++] = trace[ip0].zf();
                if(s_bDynColor)
                {
                    buffer[iv++] = xfl::GLGetRed(  velocity.at(ip0)/m_MaxVelocity);
                    buffer[iv++] = xfl::GLGetGreen(velocity.at(ip0)/m_MaxVelocity);
                    buffer[iv++] = xfl::GLGetBlue( velocity.at(ip0)/m_MaxVelocity);
                }
                else
                {
                    buffer[iv++] = s_ls.m_Color.redF();
                    buffer[iv++] = s_ls.m_Color.greenF();
                    buffer[iv++] = s_ls.m_Color.blueF();
                }

                buffer[iv++] = double(trace.size()-j+1)/double(trace.size()-1);

                buffer[iv++] = trace[ip1].xf();
                buffer[iv++] = trace[ip1].yf();
                buffer[iv++] = trace[ip1].zf();
                if(s_bDynColor)
                {
                    buffer[iv++] = xfl::GLGetRed(  velocity.at(ip1)/m_MaxVelocity);
                    buffer[iv++] = xfl::GLGetGreen(velocity.at(ip1)/m_MaxVelocity);
                    buffer[iv++] = xfl::GLGetBlue( velocity.at(ip1)/m_MaxVelocity);
                }
                else
                {
                    buffer[iv++] = s_ls.m_Color.redF();
                    buffer[iv++] = s_ls.m_Color.greenF();
                    buffer[iv++] = s_ls.m_Color.blueF();
                }


                buffer[iv++] = double(trace.size()-j)/double(trace.size()-1);
            }
        }

        Q_ASSERT(iv==buffersize);

        if(m_vboTrace.isCreated()) m_vboTrace.destroy();
        m_vboTrace.create();
        m_vboTrace.bind();
        m_vboTrace.allocate(buffer.data(), buffersize * int(sizeof(GLfloat)));

        m_vboTrace.release();

        m_bResetAttractor = false;
    }
}


void gl3dAttractors::onAttractor()
{
    s_iAttractor = 0;
    for(int i=0; i<m_prbAttractors.size(); i++)
        if(m_prbAttractors[i]->isChecked())
        {
            s_iAttractor = i;
            break;
        }

    m_pslSpeed->setValue(100);
    onRandomSeed();
    on3dReset();
}


void gl3dAttractors::onRandomSeed()
{
    if(m_Timer.isActive())
        m_Timer.stop();

    s_NTrace = m_pieNTrace->value();
    s_TailSize = m_pieTailSize->value();

    m_Trace.resize(s_NTrace);
    for(int jt=0; jt<m_Trace.size(); jt++) m_Trace[jt].resize(s_TailSize);
    m_Velocity.resize(s_NTrace);
    for(int jt=0; jt<m_Velocity.size(); jt++)
    {
        m_Velocity[jt].resize(s_TailSize);
        m_Velocity[jt].fill(0);
    }
    m_MaxVelocity = 0.0001;

    m_iLead = 0;

    double xmin(0), ymin(0), zmin(0), amp(1);
    switch(s_iAttractor)
    {
        default:
        case 0:  xmin=ymin=-5; zmin=0.0; amp=20; break;
        case 1:  xmin=ymin=-15; amp = 30; break;
        case 2:  xmin = ymin = zmin = 0; amp = 3; break;
        case 3:  xmin = ymin = zmin = -5; amp = 10; break;
        case 4:  xmin=ymin=-5; zmin=3.0; amp=10; break;
        case 5:  xmin=ymin=-0.5; zmin=0; amp=1.0; break;
        case 6:  xmin = ymin = -7.5; amp = 15; break;
        case 7:  xmin = 0; ymin = zmin = -0.5; amp = 1.0; break;
        case 8:  xmin = ymin = zmin = -0.5; amp = 1.0; break;
        case 9:  xmin = ymin = zmin = -2.5; amp = 5; break;
        case 10: xmin=ymin=-1; zmin=0; amp=2; break;
        case 11: xmin = ymin = -2; zmin = 0; amp = 5; break;
        case 12: xmin = ymin = -30; zmin = 0; amp = 60; break;
    }

    double rmax(0);
    Vector3d pos;
    for(int i=0; i<s_NTrace; i++)
    {
        pos.x = xmin + QRandomGenerator::global()->bounded(amp);
        pos.y = ymin + QRandomGenerator::global()->bounded(amp);
        pos.z = zmin + QRandomGenerator::global()->bounded(amp);
        if(s_iAttractor==6) pos.z = 0;

        rmax = std::max(rmax, pos.norm());
        QVector<Vector3d> &trace = m_Trace[i];
        for(int jt=0; jt<s_TailSize; jt++)
            trace[jt] = pos;
        m_Velocity[i].fill(0);
    }
    m_MaxVelocity = 0.0001;

    setReferenceLength(rmax*3.0);
//    on3dReset();
    m_bResetAttractor = true;
    setFocus();
    m_Timer.start(16);
}


double gl3dAttractors::f(double x, double y, double z)  const
{
    switch(s_iAttractor)
    {
        default:
        case 0:  return 10.0*(y-x);
        case 1:  return -0.4*x + y + 10.0*y*z;
        case 2:  return sin(y) -0.208186*x;
        case 3:  return y-3.0*x+2.7*y*z;
        case 4:  return 5.0*x-y*z;
        case 5:  return x*(z-0.7) - 3.5*y;
        case 6:  return -(y+z);
        case 7:  return y + 2.07*x*y + x*z;
        case 8:  return 0.2*x + y*z;
        case 9:  return -1.89*x - 4*y - 4*z - y*y;
        case 10: return y*(z-1.0+x*x) + 0.1*x;
        case 11: return y;
        case 12: return 40.0*(y-x)+0.5*x*z;
    }
}


double gl3dAttractors::g(double x, double y, double z) const
{
    switch(s_iAttractor)
    {
        default:
        case 0:  return x*(28.0-z)-y;
        case 1:  return -x - 0.4*y + 5.0*x*z;
        case 2:  return sin(z) -0.208186*y;
        case 3:  return 1.7*y -x*z+z;
        case 4:  return -10.0*y+x*z;
        case 5:  return 3.5*x + y*(z-0.7);
        case 6:  return x+0.2*y;
        case 7:  return 1.0 - 1.79*x*x + y*z;
        case 8:  return 0.01*x - 0.4*y -x*z;
        case 9:  return -1.89*y - 4*z - 4*x - z*z;
        case 10: return x*(3.0*z+1.0-x*x) + 0.1*y;
        case 11: return -x+y*z;
        case 12: return 20.0*y-x*z;
    }
}


double gl3dAttractors::h(double x, double y, double z) const
{
    switch(s_iAttractor)
    {
        default:
        case 0:  return x*y-8.0/3.0*z;
        case 1:  return 0.175*z - 5*x*y;
        case 2:  return sin(x) -0.208186*z;
        case 3:  return 2*x*y-9.0*z;
        case 4:  return -0.38*z+x*y/3.0;
        case 5:  return 0.6 + 0.95*z - z*z*z/3.0 -(x*x+y*y)*(1.0+0.25*z)+0.1*z*x*x*x;
        case 6:  return 0.2 + z*(x-5.7);
        case 7:  return x -x*x -y*y;
        case 8:  return -z - x*y;
        case 9:  return -1.89*z - 4*x - 4*y - x*x;
        case 10: return -2*z*(0.14+x*y);
        case 11: return (1.5-y*y);
        case 12: return 0.833*z+x*y-0.65*x*x;
    }
}


void gl3dAttractors::moveThem()
{
    double k1(0), l1(0), m1(0);
    double k2(0), l2(0), m2(0);
    double k3(0), l3(0), m3(0);
    double k4(0), l4(0), m4(0);

    double dt = 0.003;
    if     (s_iAttractor==0)  dt = 0.003;   // Lorenz
    else if(s_iAttractor==1)  dt = 2.e-4;   // Newton-Leipnik
    else if(s_iAttractor==2)  dt = 0.07;    // Thomas
    else if(s_iAttractor==3)  dt = 0.004;   // Dadras
    else if(s_iAttractor==4)  dt = 0.0025;  // Chen
    else if(s_iAttractor==5)  dt = 0.005;   // Aizawa
    else if(s_iAttractor==6)  dt = 0.01;    // Rossler
    else if(s_iAttractor==7)  dt = 0.01;    // Sprott
    else if(s_iAttractor==8)  dt = 0.05;    // Four-wing
    else if(s_iAttractor==9)  dt = 0.003;   // Halvorsen
    else if(s_iAttractor==10) dt = 0.015;   // Rabinovich-Fabrikant
    else if(s_iAttractor==11) dt = 0.01;    // Nosé-Hoover
    else if(s_iAttractor==12) dt = 0.0005;  // TSUCS 1

    double coef = double(m_pslSpeed->value())/100.0;
    coef = std::max(0.1, coef);
    dt *= coef;

    m_iLead--;
    if(m_iLead<0) m_iLead = s_TailSize-1;

    double rmax = 0.0;

    m_MaxVelocity *=0.995; // partial reset to prevent the colors from getting squashed

    for(int i=0; i<m_Trace.size(); i++)
    {
        QVector<Vector3d> &trace = m_Trace[i];
        Vector3d &pt = trace[m_iLead];
        pt = trace[(m_iLead+1)%s_TailSize];

        //predictor
        k1 = f(pt.x,           pt.y,             pt.z);
        l1 = g(pt.x,           pt.y,             pt.z);
        m1 = h(pt.x,           pt.y,             pt.z);

        k2 = f(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
        l2 = g(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
        m2 = h(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));

        k3 = f(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
        l3 = g(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
        m3 = h(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));

        k4 = f(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
        l4 = g(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
        m4 = h(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);

        //corrector
        pt.x += dt*(k1 +2*k2 +2*k3 +k4)/6;
        pt.y += dt*(l1 +2*l2 +2*l3 +l4)/6;
        pt.z += dt*(m1 +2*m2 +2*m3 +m4)/6;

        double dx = f(pt.x, pt.y, pt.z);
        double dy = g(pt.x, pt.y, pt.z);
        double dz = h(pt.x, pt.y, pt.z);
        QVector<double> &velocity = m_Velocity[i];
        velocity[m_iLead] = sqrt(dx*dx+dy*dy+dz*dz)/5.0;
        m_MaxVelocity = std::max(m_MaxVelocity, velocity.at(m_iLead));
        rmax = std::max(rmax, pt.norm());
    }
    m_bResetAttractor = true;
    setReferenceLength(rmax*3.0);
    update();
}




