/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

// Lorenz attractor

#include <QFormLayout>
#include <QPushButton>

#include "gl3dattractor.h"

#include <xflcore/trace.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/wt_globals.h>

Vector3d gl3dAttractor::s_P = Vector3d(13, 15, 30);
double gl3dAttractor::s_Sigma = 10.0;
double gl3dAttractor::s_Beta = 8.0/3.0;
double gl3dAttractor::s_Rho = 28.0;

int gl3dAttractor::s_RefreshInterval = 16; //ms = 1/60Hz = usual monitor refresh rate

int gl3dAttractor::s_MaxPts = 5000;
double gl3dAttractor::s_dt = 0.003;
LineStyle gl3dAttractor::s_ls = {true, Line::SOLID, 2, QColor(205,92,92), Line::NOSYMBOL, QString()};


gl3dAttractor::gl3dAttractor(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setWindowTitle("Lorenz attractor");

    m_pTimer = nullptr;
    m_Counter = 0;
    m_bResetAttractor = true;
    m_iLead = 0;

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, s_TextColor);
        palette.setColor(QPalette::Text,       s_TextColor);
        QColor clr = s_BackgroundColor;
        clr.setAlpha(0);
        palette.setColor(QPalette::Window,     clr);
        palette.setColor(QPalette::Base,       clr);

        pFrame->setCursor(Qt::ArrowCursor);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *pSystemLab = new QLabel;
            pSystemLab->setObjectName("Lorenz system");

            if(s_BackgroundColor.value()>125)
                pSystemLab->setPixmap(QPixmap(QString::fromUtf8(":/images/Lorenz.png")));
            else
                pSystemLab->setPixmap(QPixmap(QString::fromUtf8(":/images/Lorenz_inv.png")));
            pSystemLab->setAlignment(Qt::AlignCenter);

            QFrame *pParamsForm = new QFrame;
            {
                pParamsForm->setPalette(palette);
                QGridLayout*pParamsLayout = new QGridLayout;
                {
                    m_pdeSigma = new DoubleEdit(s_Sigma);
                    m_pdeRho   = new DoubleEdit(s_Rho);
                    m_pdeBeta  = new DoubleEdit(s_Beta);
                    m_pdeX     = new DoubleEdit(s_P.x);
                    m_pdeY     = new DoubleEdit(s_P.y);
                    m_pdeZ     = new DoubleEdit(s_P.z);
                    m_pdeDt    = new DoubleEdit(s_dt);
                    m_pieIntervalms = new IntEdit(s_RefreshInterval);
                    m_pieMaxPts     = new IntEdit(s_MaxPts);

                    m_pdeSigma->setPalette(palette);
                    m_pdeRho->setPalette(palette);
                    m_pdeBeta->setPalette(palette);
                    m_pdeX->setPalette(palette);
                    m_pdeY->setPalette(palette);
                    m_pdeZ->setPalette(palette);
                    m_pdeDt->setPalette(palette);
                    m_pieIntervalms->setPalette(palette);
                    m_pieMaxPts->setPalette(palette);

                    m_plbStyle  = new LineBtn(s_ls);
                    m_plbStyle->setPalette(palette);
                    connect(m_plbStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onLineStyle(LineStyle)));

                    QLabel *pLabSigma     = new QLabel(QString(QChar(963))+"=");    pLabSigma->setPalette(palette);
                    QLabel *pLabRho       = new QLabel(QString(QChar(961))+"=");    pLabRho->setPalette(palette);
                    QLabel *pLabBeta      = new QLabel(QString(QChar(946))+"=");    pLabBeta->setPalette(palette);
                    QLabel *pLabX0        = new QLabel("X<sub>0</sub>=");           pLabX0->setPalette(palette);
                    QLabel *pLabY0        = new QLabel("Y<sub>0</sub>=");           pLabY0->setPalette(palette);
                    QLabel *pLabZ0        = new QLabel("Z<sub>0</sub>=");           pLabZ0->setPalette(palette);
                    QLabel *pLabdt        = new QLabel("dt=");                      pLabdt->setPalette(palette);
                    QLabel *pLabMaxPts    = new QLabel("Max. points=");             pLabMaxPts->setPalette(palette);
                    QLabel *pLabLineStyle = new QLabel("Line style:");              pLabLineStyle->setPalette(palette);
                    QLabel *pLabRefresh   = new QLabel("Refresh interval (ms)=");   pLabRefresh->setPalette(palette);

                    pParamsLayout->addWidget(pLabSigma,      1, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabRho,        2, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabBeta,       3, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabX0,         4, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabY0,         5, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabZ0,         6, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabdt,         7, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabMaxPts,     8, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabLineStyle,  9, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabRefresh,   10, 1, Qt::AlignRight | Qt::AlignVCenter);

                    pParamsLayout->addWidget(m_pdeSigma,       1, 2);
                    pParamsLayout->addWidget(m_pdeRho,         2, 2);
                    pParamsLayout->addWidget(m_pdeBeta,        3, 2);
                    pParamsLayout->addWidget(m_pdeX,           4, 2);
                    pParamsLayout->addWidget(m_pdeY,           5, 2);
                    pParamsLayout->addWidget(m_pdeZ,           6, 2);
                    pParamsLayout->addWidget(m_pdeDt,          7, 2);
                    pParamsLayout->addWidget(m_pieMaxPts,      8, 2);
                    pParamsLayout->addWidget(m_plbStyle,       9, 2);
                    pParamsLayout->addWidget(m_pieIntervalms, 10, 2);
                }
                pParamsForm->setLayout(pParamsLayout);
            }

            QPushButton *pRestartBtn = new QPushButton("Restart attractor");
            pRestartBtn->setPalette(palette);
            connect(pRestartBtn, SIGNAL(clicked()), SLOT(onRestart()));

            QPushButton *pResetDefaults = new QPushButton("Reset defaults");
            connect(pResetDefaults, SIGNAL(clicked()), SLOT(onResetDefaults()));

            pFrameLayout->addWidget(pSystemLab);
            pFrameLayout->addWidget(pParamsForm);
            pFrameLayout->addWidget(pRestartBtn);
            pFrameLayout->addWidget(pResetDefaults);
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    onRestart();

    setReferenceLength(150);
    reset3dScale();
}


gl3dAttractor::~gl3dAttractor()
{
    if(m_pTimer)
    {
        m_pTimer->stop();
        delete m_pTimer;
        m_pTimer = nullptr;
    }
}


void gl3dAttractor::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_pTimer)
            {
                if(m_pTimer->isActive()) m_pTimer->stop();
                else                     m_pTimer->start(17);
            }
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dAttractor::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAttractor");
    {
        s_P.x             = settings.value("X0", s_P.x).toDouble();
        s_P.y             = settings.value("Y0", s_P.y).toDouble();
        s_P.z             = settings.value("Z0", s_P.z).toDouble();
        s_dt              = settings.value("dt", s_dt).toDouble();
        s_MaxPts          = settings.value("MaxPoints",       s_MaxPts).toInt();
        s_RefreshInterval = settings.value("RefreshInterval", s_RefreshInterval).toInt();  // ms = 1/60Hz
        s_ls.loadSettings(settings, "LineStyle");
    }
    settings.endGroup();
}


void gl3dAttractor::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAttractor");
    {
        settings.setValue("X0", s_P.x);
        settings.setValue("Y0", s_P.y);
        settings.setValue("Z0", s_P.z);
        settings.setValue("dt", s_dt);
        settings.setValue("MaxPoints",       s_MaxPts);
        settings.setValue("RefreshInterval", s_RefreshInterval);
        s_ls.saveSettings(settings, "LineStyle");
    }
    settings.endGroup();
}


/**
 * The user has changed the color of the current curve
 */
void gl3dAttractor::onLineStyle(LineStyle)
{
    LineMenu *pLineMenu = new LineMenu(nullptr, false);
    pLineMenu->initMenu(s_ls);
    pLineMenu->exec(QCursor::pos());
    s_ls = pLineMenu->theStyle();
    m_plbStyle->setTheStyle(s_ls);
}


void gl3dAttractor::onResetDefaults()
{
    s_P = Vector3d(13, 15, 30);
    m_pdeX->setValue(s_P.x);
    m_pdeY->setValue(s_P.y);
    m_pdeZ->setValue(s_P.z);
    s_Sigma = 10.0;
    s_Beta  = 8.0/3.0;
    s_Rho   = 28.0;
    m_pdeSigma->setValue(s_Sigma);
    m_pdeRho->setValue(s_Rho);
    m_pdeBeta->setValue(s_Beta);

    s_RefreshInterval = 16; //ms --> 60Hz
    m_pieIntervalms->setValue(s_RefreshInterval);

    s_MaxPts = 5000;
    m_pieMaxPts->setValue(s_MaxPts);

    s_dt = 0.003;
    m_pdeDt->setValue(s_dt);
    onRestart();
}


void gl3dAttractor::glRenderView()
{
    paintSphere(s_P, 0.5f, s_ls.m_Color, true);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();

    paintColourSegments(m_vboTrace, s_ls);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dAttractor::glMake3dObjects()
{
    if(m_bResetAttractor)
    {
        int buffersize = (s_MaxPts-1)  // NSegments
                         *2*(3+4);     // 2 vertices * (3 coordinates+ 4 color components)
        QVector<float> buffer(buffersize);

        int ip0(0), ip1(0);

        int iv = 0;

        for(int j=1; j<m_Trace.size(); j++)
        {
            ip0 = (m_iLead+j-1)%s_MaxPts;
            ip1 = (m_iLead+j  )%s_MaxPts;
            buffer[iv++] = m_Trace[ip0].xf();
            buffer[iv++] = m_Trace[ip0].yf();
            buffer[iv++] = m_Trace[ip0].zf();

            buffer[iv++] = s_ls.m_Color.redF();
            buffer[iv++] = s_ls.m_Color.greenF();
            buffer[iv++] = s_ls.m_Color.blueF();
            buffer[iv++] = double(m_Trace.size()-j+1)/double(m_Trace.size()-1);

            buffer[iv++] = m_Trace[ip1].xf();
            buffer[iv++] = m_Trace[ip1].yf();
            buffer[iv++] = m_Trace[ip1].zf();

            buffer[iv++] = s_ls.m_Color.redF();
            buffer[iv++] = s_ls.m_Color.greenF();
            buffer[iv++] = s_ls.m_Color.blueF();
            buffer[iv++] = double(m_Trace.size()-j)/double(m_Trace.size()-1);
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

double gl3dAttractor::f(double x, double y, double )  const {return s_Sigma*(y-x);}
double gl3dAttractor::g(double x, double y, double z) const {return x*(s_Rho-z)-y;}
double gl3dAttractor::h(double x, double y, double z) const {return x*y-s_Beta*z;}


void gl3dAttractor::moveIt()
{
    s_dt = m_pdeDt->value();
    s_RefreshInterval = m_pieIntervalms->value();


    if(m_pTimer)
        m_pTimer->setInterval(s_RefreshInterval);

    m_iLead--;
    if(m_iLead<0) m_iLead = s_MaxPts-1;

    Vector3d &pt = m_Trace[m_iLead];
    pt = m_Trace[(m_iLead+1)%s_MaxPts];
    // RK4
    double dt = s_dt;

    //predictor
    double k1 = f(pt.x,           pt.y,             pt.z);
    double l1 = g(pt.x,           pt.y,             pt.z);
    double m1 = h(pt.x,           pt.y,             pt.z);

    double k2 = f(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
    double l2 = g(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
    double m2 = h(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));

    double k3 = f(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
    double l3 = g(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
    double m3 = h(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));

    double k4 = f(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
    double l4 = g(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
    double m4 = h(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);

    //corrector
    pt.x += dt*(k1 +2*k2 +2*k3 +k4)/6;
    pt.y += dt*(l1 +2*l2 +2*l3 +l4)/6;
    pt.z += dt*(m1 +2*m2 +2*m3 +m4)/6;

    m_bResetAttractor = true;

    s_P = pt; // save it


    update();
}


void gl3dAttractor::onRestart()
{
    s_P = Vector3d(m_pdeX->value(), m_pdeY->value(), m_pdeZ->value());

    s_Sigma = m_pdeSigma->value();
    s_Beta  = m_pdeBeta->value();
    s_Rho   = m_pdeRho->value();

    // initialize the array
    s_MaxPts = m_pieMaxPts->value();
    s_dt = m_pdeDt->value();
    s_RefreshInterval = m_pieIntervalms->value();

    m_Trace.resize(s_MaxPts);
    for(int p=0; p<s_MaxPts; p++)
    {
        m_Trace[p] = s_P;
    }
    m_iLead = 0;

    moveIt(); // initialize the vbo

    if(m_pTimer)
    {
        m_pTimer->stop();
        delete m_pTimer;
    }

    m_LastTime.start();
    m_Counter = 0;

    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), SLOT(moveIt()));
    m_pTimer->start(s_RefreshInterval);
}
