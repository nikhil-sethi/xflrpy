/****************************************************************************

    StabViewDlg Class
    Copyright (C) André Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/


#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTimer>
#include <complex>



#include "stabviewdlg.h"

#include <miarex/miarex.h>
#include <miarex/view/gl3dmiarexview.h>

#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflcore/mathelem.h>
#include <xflgraph/curve.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/customdlg/newnamedlg.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/floateditdelegate.h>

Miarex *StabViewDlg::s_pMiarex(nullptr);


StabViewDlg::StabViewDlg(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Stability View Params"));
    setWindowFlags(Qt::Tool);

    m_iCurrentMode = 0;
    m_ModeAmplitude = 1.0;
    m_ModeInterval = 200;
    m_pCurve = nullptr;
    for(int i=0; i<20; i++)
    {
        m_Time[i] = double(i);
        m_Amplitude[i] = 0.0;
    }
    setupLayout();
    connectSignals();
}

StabViewDlg::~StabViewDlg()
{
}


void StabViewDlg::connectSignals()
{
    connect(m_prbLongDynamics,       SIGNAL(clicked()), s_pMiarex, SLOT(onStabilityDirection()));
    connect(m_prbLatDynamics,        SIGNAL(clicked()), s_pMiarex, SLOT(onStabilityDirection()));

    connect(m_ppbPlotStabGraph,      SIGNAL(clicked()),            SLOT(onPlotStabilityGraph()));

    connect(m_prbRLMode1,            SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbRLMode2,            SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbRLMode3,            SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbRLMode4,            SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbTimeMode1,          SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbTimeMode2,          SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbTimeMode3,          SIGNAL(clicked()),            SLOT(onModeSelection()));
    connect(m_prbTimeMode4,          SIGNAL(clicked()),            SLOT(onModeSelection()));
    
    connect(m_ppbAnimate,            SIGNAL(clicked()),            SLOT(onAnimate()));
    connect(m_pdAnimationSpeed ,     SIGNAL(valueChanged(int)),    SLOT(onAnimationSpeed(int)));
    connect(m_pdAnimationAmplitude,  SIGNAL(valueChanged(int)),    SLOT(onAnimationAmplitude(int)));
    connect(m_ppbAnimateRestart,     SIGNAL(clicked()),            SLOT(onAnimateRestart()));
    connect(m_pdeDeltat,             SIGNAL(editingFinished()),    SLOT(onReadData()));
    connect(m_pdeModeStep,           SIGNAL(editingFinished()),    SLOT(onReadData()));

    connect(m_prbInitCondResponse,   SIGNAL(clicked()),            SLOT(onResponseType()));
    connect(m_prbForcedResponse,     SIGNAL(clicked()),            SLOT(onResponseType()));
    connect(m_prbModalResponse,      SIGNAL(clicked()),            SLOT(onResponseType()));
    
    connect(m_ppbAddCurve,           SIGNAL(clicked()),            SLOT(onAddCurve()));
    connect(m_ppbDeleteCurve,        SIGNAL(clicked()),            SLOT(onDeleteCurve()));
    connect(m_ppbRenameCurve,        SIGNAL(clicked()),            SLOT(onRenameCurve()));
    connect(m_pcbCurveList,          SIGNAL(activated(int)),       SLOT(onSelChangeCurve(int)));
    
    m_pControlModel = new QStandardItemModel(this);
    m_pControlModel->setRowCount(20);//temporary
    m_pControlModel->setColumnCount(2);
    m_pControlModel->setHeaderData(0, Qt::Horizontal, tr("Time (s)"));
    m_pControlModel->setHeaderData(1, Qt::Horizontal, tr("Angle ")+QString::fromUtf8("(°)"));

    m_ptvControl->setModel(m_pControlModel);
    m_ptvControl->setWindowTitle(tr("Controls"));
    m_ptvControl->setColumnWidth(0,50);
    m_ptvControl->setColumnWidth(1,40);
    QHeaderView *HorizontalHeader = m_ptvControl->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);
}


void StabViewDlg::updateControlModelData()
{
    QModelIndex ind;
    for(int i=0; i<m_pControlModel->rowCount(); i++)
    {
        ind = m_pControlModel->index(i, 0, QModelIndex());
        m_pControlModel->setData(ind, m_Time[i]);
        ind = m_pControlModel->index(i, 1, QModelIndex());
        m_pControlModel->setData(ind, m_Amplitude[i]);
    }
}


void StabViewDlg::readControlModelData()
{
    for(int i=0; i<m_pControlModel->rowCount(); i++)
    {
        m_Time[i]      = m_pControlModel->index(i, 0, QModelIndex()).data().toDouble();
        m_Amplitude[i] = m_pControlModel->index(i, 1, QModelIndex()).data().toDouble();
    }
}


void StabViewDlg::fillEigenThings()
{
    complex<double> eigenvalue;
    double OmegaN, Omega1, Zeta;
    QString strange;
    double u0, mac, span;
    complex<double> angle;

    OmegaN = Omega1 = Zeta = u0 = mac = span = 0;

    QString ModeDescription = tr("<small>Mode Properties:")+"<br/>";

    if(s_pMiarex->m_pCurPlane && s_pMiarex->m_pCurPOpp && s_pMiarex->m_pCurWPolar->polarType()==xfl::STABILITYPOLAR)
    {
        //We normalize the mode before display and only for display purposes
        u0   = s_pMiarex->m_pCurPOpp->m_QInf;
        mac  = s_pMiarex->m_pCurPlane->m_Wing[0].m_MAChord;
        span = s_pMiarex->m_pCurPlane->m_Wing[0].m_PlanformSpan;

        eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenValue[m_iCurrentMode];
        if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%9.4f+%9.4fi", eigenvalue.real(), eigenvalue.imag());
        else                       strange = QString::asprintf("%9.4f-%9.4fi", eigenvalue.real(), eigenvalue.imag());
        m_pleEigenValue->setText(strange);
        ModeDescription.append("Lambda="+strange+"<br/>");

        modeProperties(eigenvalue, OmegaN, Omega1, Zeta);
//        Omega1 = qAbs(eigenvalue.imag());
//        OmegaN = sqrt(eigenvalue.real()*eigenvalue.real()+Omega1*Omega1);
//        Dsi = -eigenvalue.real()/Omega1;

        if(Omega1>PRECISION)
        {
            m_pdeFreq1->setValue(Omega1/2.0/PI);
            strange = QString::asprintf("Fd=%6.3f Hz", Omega1/2.0/PI);
            ModeDescription.append(strange+"<br/>");
        }
        else
        {
            m_pdeFreq1->clear();
        }

        if(Omega1 > PRECISION)
        {
            m_pdeFreqN->setValue(OmegaN/2.0/PI);
            m_pdeZeta->setValue(Zeta);
            strange = QString::asprintf("FN=%6.3f Hz",OmegaN/2.0/PI);
            ModeDescription.append(strange+"<br/>");
            strange = QString::asprintf("Zeta=%6.3f",Zeta);
            ModeDescription.append(strange+"<br/>");
        }
        else
        {
            m_pdeFreqN->clear();
            m_pdeZeta->clear();
        }

        if(fabs(eigenvalue.real())>PRECISION && fabs(eigenvalue.imag())<PRECISION)
        {
            strange = QString::asprintf("T2=%6.3f s", log(2)/fabs(eigenvalue.real()));
            ModeDescription.append(strange+"<br/>");
            m_pdeT2->setValue(log(2)/fabs(eigenvalue.real()));
            if(eigenvalue.real()<0.0)
            {
                m_pdeTau->setValue(-1.0/eigenvalue.real());
                strange = QString::asprintf("tau=%6.3f", -1.0/eigenvalue.real());
                ModeDescription.append(strange+"<br/>");
            }
            else
                m_pdeTau->clear();
        }
        else
        {
            m_pdeT2->clear();
            m_pdeTau->clear();
        }


        if(s_pMiarex->m_bLongitudinal && s_pMiarex->m_pCurPOpp)
        {
            angle = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][3];
            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][0]/u0;
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector1->setText(strange);
            ModeDescription.append("v1="+strange+"<br/>");

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][1]/u0;
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector2->setText(strange);
            ModeDescription.append("v2="+strange+"<br/>");

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][2]/(2.0*u0/mac);
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector3->setText(strange);
            ModeDescription.append("v3="+strange+"<br/>");

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][3]/angle;
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector4->setText(strange);
            ModeDescription.append("v4="+strange);
        }
        else if(!s_pMiarex->m_bLongitudinal && s_pMiarex->m_pCurPOpp)
        {
            angle = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][3];

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][0]/u0;
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector1->setText(strange);
            ModeDescription.append("v1="+strange+"<br/>");

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][1]/(2.0*u0/span);
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector2->setText(strange);
            ModeDescription.append("v2="+strange+"<br/>");

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][2]/(2.0*u0/span);
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector3->setText(strange);
            ModeDescription.append("v3="+strange+"<br/>");

            eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenVector[m_iCurrentMode][3]/angle;
            if(eigenvalue.imag()>=0.0) strange = QString::asprintf("%10.5f+%10.5fi",eigenvalue.real(),eigenvalue.imag());
            else                       strange = QString::asprintf("%10.5f-%10.5fi",eigenvalue.real(),fabs(eigenvalue.imag()));
            m_pleEigenVector4->setText(strange);
            ModeDescription.append("v4="+strange);

        }
        ModeDescription.append("</small>");
        m_plabModeProperties->setText(ModeDescription);
    }
    else
    {
        m_pleEigenValue->clear();
        m_pleEigenVector1->clear();
        m_pleEigenVector2->clear();
        m_pleEigenVector3->clear();
        m_pleEigenVector4->clear();
        m_pdeFreqN->clear();
        m_pdeFreq1->clear();
        m_pdeZeta->clear();
        m_pdeT2->clear();
        m_pdeTau->clear();
        m_plabModeProperties->clear();
    }
}


void StabViewDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_ppbPlotStabGraph->hasFocus()) m_ppbPlotStabGraph->setFocus();
            else onPlotStabilityGraph();
             
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_ppbAnimate->isChecked()) m_ppbAnimate->setChecked(false);
            onAnimate();
            break;
        }
        default:
        {
            s_pMiarex->keyPressEvent(pEvent);
        }
    }
}


void StabViewDlg::onAnimate()
{
    if(m_ppbAnimate->isChecked())
    {
//        pMiarex->m_iView = WSTABVIEW;
        s_pMiarex->m_iView=xfl::W3DVIEW;
        s_pMiarex->setControls();
        
        s_pMiarex->m_Modedt = m_pdeModeStep->value();
        s_pMiarex->m_bAnimateMode  = true;
        int speed = m_pdAnimationSpeed->value();
        s_pMiarex->m_pTimerMode->setInterval(400-speed);
        s_pMiarex->m_pTimerMode->start();
    }
    else
    {
        s_pMiarex->stopAnimate();
    }
}


void StabViewDlg::onAnimationAmplitude(int val)
{
    m_ModeAmplitude = double(val)/500.0;
    s_pMiarex->onAnimateModeSingle(false);
}


void StabViewDlg::onAnimationSpeed(int val)
{
    m_ModeInterval = val;
    s_pMiarex->m_pTimerMode->setInterval(400-val);
}


void StabViewDlg::onAnimateRestart()
{
    double sigma, s2, omega, o2;
    double norm1, norm2, theta_sum, psi_sum, ModeState[6];

    for(int im=0; im<6; im++) ModeState[im] = 0;

    PlaneOpp *pPOpp = s_pMiarex->m_pCurPOpp;
    Plane *pPlane = s_pMiarex->m_pCurPlane;

    s_pMiarex->m_ModeTime = 0.0;

    if(!pPOpp || !pPlane)
    {
        s_pMiarex->m_ModeState[0] = 0.0;
        s_pMiarex->m_ModeState[1] = 0.0;
        s_pMiarex->m_ModeState[2] = 0.0;
        s_pMiarex->m_ModeState[3] = 0.0;
        s_pMiarex->m_ModeState[4] = 0.0;
        s_pMiarex->m_ModeState[5] = 0.0;
        s_pMiarex->updateView();
        return;
    }

    sigma = pPOpp->m_EigenValue[m_iCurrentMode].real();
    omega = pPOpp->m_EigenValue[m_iCurrentMode].imag();
    s2 = sigma*sigma;
    o2 = omega*omega;
//    maxso  = qMax(qAbs(sigma), qAbs(omega));

    //calculate state at t=0 for normalization
    if(s2+o2>PRECISION)
    {
        if(s_pMiarex->m_bLongitudinal)
        {
            //x, z, theta are evaluated by direct inegration of u, w, q
            ModeState[1] = 0.0;
            ModeState[3] = 0.0;
            ModeState[5] = 0.0;
            ModeState[0] = m_vabs[0]/(s2+o2) * (sigma*cos(m_phi[0])+omega*sin(m_phi[0]));
            ModeState[2] = m_vabs[1]/(s2+o2) * (sigma*cos(m_phi[1])+omega*sin(m_phi[1]));
            ModeState[4] = m_vabs[2]/(s2+o2) * (sigma*cos(m_phi[2])+omega*sin(m_phi[2]));
    //        ModeState[4] = m_vabs[3]*cos(m_phi[3]);

            //add u0 x theta_sum to z component
            theta_sum      = m_vabs[3]/(s2+o2) * (sigma*cos(m_phi[3])+omega*sin(m_phi[3]));

            ModeState[2] -= theta_sum *pPOpp->m_QInf;
        }
        else
        {
            //y, phi, psi evaluation
            ModeState[0] = 0.0;
            ModeState[2] = 0.0;
            ModeState[4] = 0.0;

            // integrate (v+u0.psi.cos(theta0)) to get y
            ModeState[1] = m_vabs[0]/(s2+o2) * (sigma*cos(m_phi[0])+omega*sin(m_phi[0]));

            //integrate psi = integrate twice r (thanks Matlab !)
            psi_sum =   sigma * ( sigma * cos(m_phi[2]) + omega * sin(m_phi[2]))
                      + omega * (-omega * cos(m_phi[2]) + sigma * sin(m_phi[2]));
            psi_sum *= m_vabs[2]/ (s2+o2)/(s2+o2);

            ModeState[1] += pPOpp->m_QInf * psi_sum;

            // get directly phi from fourth eigenvector component (alternatively integrate p+r.tan(theta0));
            ModeState[3] = m_vabs[3]*cos(m_phi[3]);
    //        m_ModeState[3] = m_ModeNorm*m_vabs[1]/(s2+o2) * (sigma*cos(m_phi[1])+omega*sin(m_phi[1]));

            // integrate once 'p+r.sin(theta0)' to get heading angle
            ModeState[5] = m_vabs[2]/(s2+o2) * (sigma*cos(m_phi[2])+omega*sin(m_phi[2]));
        }
    }

    //max 10% span
    norm1 = qMax(qAbs(ModeState[0]), qAbs(ModeState[1]));
    norm1 = qMax(norm1, qAbs(ModeState[2]));
    if(norm1>PRECISION)  norm1 = pPlane->m_Wing[0].m_PlanformSpan *.10 / norm1;
    else                 norm1 = 1.0;

    //max 10degrees
    norm2 = qMax(qAbs(ModeState[3]), qAbs(ModeState[4]));
    norm2 = qMax(norm2, qAbs(ModeState[5]));
    if(norm2>PRECISION)  norm2 = PI*(10.0/180.0)/ norm2;
    else                 norm2 = 1.0;

    s_pMiarex->m_ModeNorm = qMin(norm1, norm2);

    //set initial mode positions, i.e. t=0
    s_pMiarex->onAnimateModeSingle(false);
}


void StabViewDlg::onCellChanged(QWidget *)
{
    
}


void StabViewDlg::onPlotStabilityGraph()
{
    if(!s_pMiarex->m_TimeGraph[0]->curveCount())
    {
        //we don't have a curve yet
        // so return
        return;
    }
    
    s_pMiarex->createStabilityCurves();
    s_pMiarex->updateView();
    s_pMiarex->setFocus();
}


void StabViewDlg::onModeSelection()
{
    if(s_pMiarex->m_iView==xfl::STABTIMEVIEW)
    {
        if(m_prbTimeMode1->isChecked())      m_iCurrentMode = 0;
        else if(m_prbTimeMode2->isChecked()) m_iCurrentMode = 1;
        else if(m_prbTimeMode3->isChecked()) m_iCurrentMode = 2;
        else if(m_prbTimeMode4->isChecked()) m_iCurrentMode = 3;
    }
    else if(s_pMiarex->m_iView==xfl::STABPOLARVIEW || s_pMiarex->m_iView==xfl::W3DVIEW)
    {
        if(m_prbRLMode1->isChecked())      m_iCurrentMode = 0;
        else if(m_prbRLMode2->isChecked()) m_iCurrentMode = 1;
        else if(m_prbRLMode3->isChecked()) m_iCurrentMode = 2;
        else if(m_prbRLMode4->isChecked()) m_iCurrentMode = 3;
    }
    if(!s_pMiarex->m_bLongitudinal) m_iCurrentMode +=4;
    setMode(m_iCurrentMode);

    if(s_pMiarex->m_iView==xfl::STABPOLARVIEW && Graph::isHighLighting())
    {
        s_pMiarex->createStabRLCurves();
        s_pMiarex->updateView();
    }
}


void StabViewDlg::onReadData()
{
    s_pMiarex->m_Modedt = m_pdeModeStep->value();
    s_pMiarex->m_Deltat = m_pdeDeltat->value();
}



void StabViewDlg::onResponseType()
{
    int type=0;
    
    if(m_prbInitCondResponse->isChecked())    type=0;
    else if(m_prbForcedResponse->isChecked()) type=1;
    else if(m_prbModalResponse->isChecked())  type=2;
    
    if(type==s_pMiarex->m_StabilityResponseType) return;
    
    s_pMiarex->m_StabilityResponseType=type;
    setControls();
//    pMiarex->CreateStabilityCurves();
    s_pMiarex->updateView();
    
}


void StabViewDlg::setMode(int iMode)
{
    if(iMode>=0)
    {
        m_iCurrentMode = iMode%4;
        if(!s_pMiarex->m_bLongitudinal) m_iCurrentMode += 4;
    }
    else if(m_iCurrentMode<0) m_iCurrentMode=0;

    m_prbRLMode1->setChecked(m_iCurrentMode%4==0);
    m_prbRLMode2->setChecked(m_iCurrentMode%4==1);
    m_prbRLMode3->setChecked(m_iCurrentMode%4==2);
    m_prbRLMode4->setChecked(m_iCurrentMode%4==3);
    fillEigenThings();
    PlaneOpp *pPOpp = s_pMiarex->m_pCurPOpp;

    if(pPOpp)
    {
        m_vabs[0] = abs(pPOpp->m_EigenVector[m_iCurrentMode][0]);
        m_vabs[1] = abs(pPOpp->m_EigenVector[m_iCurrentMode][1]);
        m_vabs[2] = abs(pPOpp->m_EigenVector[m_iCurrentMode][2]);
        m_vabs[3] = abs(pPOpp->m_EigenVector[m_iCurrentMode][3]);
        m_phi[0]  = arg(pPOpp->m_EigenVector[m_iCurrentMode][0]);
        m_phi[1]  = arg(pPOpp->m_EigenVector[m_iCurrentMode][1]);
        m_phi[2]  = arg(pPOpp->m_EigenVector[m_iCurrentMode][2]);
        m_phi[3]  = arg(pPOpp->m_EigenVector[m_iCurrentMode][3]);
    }
    else
    {
        m_vabs[0] = m_vabs[1] = m_vabs[2] = m_vabs[3] = 0.0;
        m_phi[0] = m_phi[1] = m_phi[2] = m_phi[3] = 0.0;
    }

    gl3dMiarexView::s_bResetglLegend = true;

//    if(pMiarex->m_pCurRLStabGraph && pMiarex->m_pCurWPolar) pMiarex->m_pCurRLStabGraph->DeselectPoint();

    onAnimateRestart();
}


void StabViewDlg::setupLayout()
{
    QFont SymbolFont("Symbol");

    //____________Stability direction__________
    QGroupBox *pStabilityDirBox = new QGroupBox(tr("Stability direction"));
    {
        m_prbLongDynamics = new QRadioButton(tr("Longitudinal"));
        m_prbLatDynamics = new QRadioButton(tr("Lateral"));
        QHBoxLayout *pStabilityDirLayout = new QHBoxLayout;
        {
            pStabilityDirLayout->addStretch(1);
            pStabilityDirLayout->addWidget(m_prbLongDynamics);
            pStabilityDirLayout->addStretch(1);
            pStabilityDirLayout->addWidget(m_prbLatDynamics);
            pStabilityDirLayout->addStretch(1);
        }
        pStabilityDirBox->setLayout(pStabilityDirLayout);
    }

    //_______________________Time view Parameters
    QGroupBox *TimeBox = new QGroupBox(tr("Time Graph Params"));
    {
        QVBoxLayout *pResponseTypeLayout = new QVBoxLayout;
        {
            m_prbModalResponse = new QRadioButton(tr("Modal Response"));
            m_prbModalResponse->setToolTip("Display the time response on a specific mode with normalized amplitude and random initial phase");
            m_prbInitCondResponse = new QRadioButton(tr("Initial Conditions Response"));
            m_prbInitCondResponse->setToolTip("Display the time response for specific initial conditions");
            m_prbForcedResponse = new QRadioButton(tr("Forced Response"));
            m_prbForcedResponse->setToolTip("Display the time response for a given control actuation in the form of a user-specified function of time");
            pResponseTypeLayout->addWidget(m_prbInitCondResponse);
            pResponseTypeLayout->addWidget(m_prbForcedResponse);
            pResponseTypeLayout->addWidget(m_prbModalResponse);
        }


        QGroupBox *pInitCondResponse = new QGroupBox(tr("Initial conditions"));
        {
            m_plabStab1 = new QLabel("u0__");
            m_plabStab2 = new QLabel("w0__");
            m_plabStab3 = new QLabel("q0__");
            m_plabUnit1 = new QLabel("m/s");
            m_plabUnit2 = new QLabel("m/s");
            m_plabUnit3 = new QLabel("rad/s");
            m_plabStab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_plabStab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_plabStab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_plabUnit1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_plabUnit2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_plabUnit3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_pdeStabVar1 = new DoubleEdit(0.00,3);
            m_pdeStabVar2 = new DoubleEdit(0.00,3);
            m_pdeStabVar3 = new DoubleEdit(1.00,3);
            QGridLayout *VarParams = new QGridLayout;
            VarParams->addWidget(m_plabStab1,1,1);
            VarParams->addWidget(m_plabStab2,2,1);
            VarParams->addWidget(m_plabStab3,3,1);
            VarParams->addWidget(m_pdeStabVar1,1,2);
            VarParams->addWidget(m_pdeStabVar2,2,2);
            VarParams->addWidget(m_pdeStabVar3,3,2);
            VarParams->addWidget(m_plabUnit1,1,3);
            VarParams->addWidget(m_plabUnit2,2,3);
            VarParams->addWidget(m_plabUnit3,3,3);
            QVBoxLayout *InitCondResponseLayout = new QVBoxLayout;
            InitCondResponseLayout ->addLayout(VarParams);
            InitCondResponseLayout->addStretch(1);
            pInitCondResponse->setLayout(InitCondResponseLayout);
        }

        QGroupBox *pForcedResponseBox = new QGroupBox(tr("Forced Response"));
        {
            QVBoxLayout *pForcedResponseLayout = new QVBoxLayout;
            QLabel *ForcedText = new QLabel(tr("Control function"));
            m_ptvControl = new QTableView(this);
            m_ptvControl->setFont(DisplayOptions::tableFont());

            m_ptvControl->setToolTip(tr("Enter the function of the control vs. time"));
            m_ptvControl->setMinimumHeight(150);
            m_ptvControl->setSelectionMode(QAbstractItemView::SingleSelection);
            m_ptvControl->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_pControlDelegate = new FloatEditDelegate(this);

            QVector<int> m_precision(3, 3);
            m_pControlDelegate->setPrecision(m_precision);
            m_ptvControl->setItemDelegate(m_pControlDelegate);

            pForcedResponseLayout->addWidget(ForcedText);
            pForcedResponseLayout->addWidget(m_ptvControl);
            pForcedResponseLayout->addStretch(1);
            pForcedResponseBox->setLayout(pForcedResponseLayout);
        }

        QGroupBox *pModalTimeBox = new QGroupBox(tr("Modal response"));
        {
            QVBoxLayout *pModalTimeLayout = new QVBoxLayout;
            m_prbTimeMode1 = new QRadioButton("Mode 1");
            m_prbTimeMode2 = new QRadioButton("Mode 2");
            m_prbTimeMode3 = new QRadioButton("Mode 3");
            m_prbTimeMode4 = new QRadioButton("Mode 4");
            m_plabModeProperties = new QLabel("Mode Properties");
            pModalTimeLayout->addWidget(m_prbTimeMode1);
            pModalTimeLayout->addWidget(m_prbTimeMode2);
            pModalTimeLayout->addWidget(m_prbTimeMode3);
            pModalTimeLayout->addWidget(m_prbTimeMode4);
            pModalTimeLayout->addStretch(1);
            pModalTimeLayout->addWidget(m_plabModeProperties);
            pModalTimeBox->setLayout(pModalTimeLayout);
        }

        m_pswInitialConditions = new QStackedWidget;
        m_pswInitialConditions->addWidget(pInitCondResponse);
        m_pswInitialConditions->addWidget(pForcedResponseBox);
        m_pswInitialConditions->addWidget(pModalTimeBox);
        m_pswInitialConditions->setCurrentIndex(0);

        m_pdeTotalTime = new DoubleEdit(5,3);
        m_pdeTotalTime->setToolTip(tr("Define the total time range for the graphs"));
        m_pdeDeltat    = new DoubleEdit(.01,3);
        m_pdeDeltat->setToolTip(tr("Define the time step for the resolution of the differential equations"));

        QGridLayout *pDtLayout  = new QGridLayout;
        {
            QLabel *DtLabel        = new QLabel("dt=");
            QLabel *TotalTimeLabel = new QLabel(tr("Total Time")+"=");
            QLabel *TimeLab1       = new QLabel("s");
            QLabel *TimeLab2       = new QLabel("s");
            pDtLayout->addWidget(DtLabel,1,1);
            pDtLayout->addWidget(m_pdeDeltat,1,2);
            pDtLayout->addWidget(TimeLab1,1,3);
            pDtLayout->addWidget(TotalTimeLabel,2,1);
            pDtLayout->addWidget(m_pdeTotalTime,2,2);
            pDtLayout->addWidget(TimeLab2,2,3);
        }

        QGridLayout *pCurveLayout = new QGridLayout;
        {
            m_ppbPlotStabGraph = new QPushButton(tr("Recalc."));
            m_ppbPlotStabGraph->setToolTip(tr("Re-calculate the currently selected curve with the user-specified input data"));
            m_ppbAddCurve  = new QPushButton(tr("Add"));
            m_ppbAddCurve->setToolTip(tr("Add a new curve to the graphs, using the current user-specified input"));
            m_ppbRenameCurve  = new QPushButton(tr("Rename"));
            m_ppbRenameCurve->setToolTip(tr("Rename the currently selected curve"));
            m_ppbDeleteCurve  = new QPushButton(tr("Delete"));
            m_ppbDeleteCurve->setToolTip(tr("Delete the currently selected curve"));
            m_pcbCurveList = new QComboBox();
            pCurveLayout->addWidget(m_ppbAddCurve,1,1);
            pCurveLayout->addWidget(m_ppbPlotStabGraph,1,2);
            pCurveLayout->addWidget(m_ppbRenameCurve,2,1);
            pCurveLayout->addWidget(m_ppbDeleteCurve,2,2);
        }

        QGroupBox *pCurveSettingsBox = new QGroupBox(tr("Curve Settings"));
        {
            QVBoxLayout *pTimeLayout = new QVBoxLayout;
            {
                pTimeLayout->addLayout(pDtLayout);
                pTimeLayout->addWidget(m_pcbCurveList);
                pTimeLayout->addLayout(pCurveLayout);
            }
            pCurveSettingsBox->setLayout(pTimeLayout);
        }


        QVBoxLayout *pTimeParamsLayout = new QVBoxLayout;
        {
        //    TimeParamsLayout->addLayout(InitialConditionsLayout);
            pTimeParamsLayout->addLayout(pResponseTypeLayout);
            pTimeParamsLayout->addWidget(m_pswInitialConditions);
            pTimeParamsLayout->addWidget(pCurveSettingsBox);
            pTimeParamsLayout->addStretch(5);
        }
        TimeBox->setLayout(pTimeParamsLayout);
    }

    //_______________________Root Locus View and 3D animation Parameters
    QGroupBox *pModeBox = new QGroupBox(tr("Operating point modes"));
    {
        QGroupBox *pRLModeBox = new QGroupBox(tr("Mode Selection"));
        {
            QHBoxLayout *pRLModeLayout = new QHBoxLayout;
            {
                m_prbRLMode1 = new QRadioButton("1");
                m_prbRLMode2 = new QRadioButton("2");
                m_prbRLMode3 = new QRadioButton("3");
                m_prbRLMode4 = new QRadioButton("4");
                m_prbRLMode1->setToolTip(tr("Press Ctrl+H to highlight the mode on the root locus plot"));
                m_prbRLMode2->setToolTip(tr("Press Ctrl+H to highlight the mode on the root locus plot"));
                m_prbRLMode3->setToolTip(tr("Press Ctrl+H to highlight the mode on the root locus plot"));
                m_prbRLMode4->setToolTip(tr("Press Ctrl+H to highlight the mode on the root locus plot"));
                pRLModeLayout->addWidget(m_prbRLMode1);
                pRLModeLayout->addWidget(m_prbRLMode2);
                pRLModeLayout->addWidget(m_prbRLMode3);
                pRLModeLayout->addWidget(m_prbRLMode4);
            }
            pRLModeBox->setLayout(pRLModeLayout);
        }

        //_____________Mode properties _________
        QGroupBox *pFreakBox = new QGroupBox(tr("Mode properties"));
        {
            QLabel *FreqNLab = new QLabel("F =");
            QLabel *Freq1Lab = new QLabel("F1 =");
            QLabel *DsiLab   = new QLabel("z =");
            QLabel *T2lab    = new QLabel("t2 =");
            QLabel *tauLab   = new QLabel("t =");
            FreqNLab->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            Freq1Lab->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            DsiLab->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            T2lab->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            tauLab->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            DsiLab->setFont(SymbolFont);
            tauLab->setFont(SymbolFont);

            m_pdeFreqN  = new DoubleEdit(0.0, 3);
            m_pdeFreq1  = new DoubleEdit(0.0, 3);
            m_pdeZeta   = new DoubleEdit(0.0, 3);
            m_pdeT2     = new DoubleEdit(0.0, 3);
            m_pdeTau    = new DoubleEdit(0.0, 3);

            QString strong;
            strong = tr("Natural frequency");
            FreqNLab->setToolTip(strong);
            m_pdeFreqN->setToolTip(strong);

            strong = tr("Damped natural frequency");
            Freq1Lab->setToolTip(strong);
            m_pdeFreq1->setToolTip(strong);

            strong = tr("Damping ratio");
            DsiLab->setToolTip(strong);
            m_pdeZeta->setToolTip(strong);

            strong = tr("Time to double");
            T2lab->setToolTip(strong);
            m_pdeT2->setToolTip(strong);

            strong = tr("Time constant") + "= (1-1/e)/|real(lambda)|";
            tauLab->setToolTip(strong);
            m_pdeTau->setToolTip(strong);

            m_pdeFreqN->setEnabled(false);
            m_pdeFreq1->setEnabled(false);
            m_pdeZeta->setEnabled(false);
            m_pdeT2->setEnabled(false);
            m_pdeTau->setEnabled(false);
            QLabel *FreqUnit1 = new QLabel("Hz");
            QLabel *FreqUnit2 = new QLabel("Hz");
            QLabel *T2Unit    = new QLabel("s");
            QGridLayout *pFreakLayout = new QGridLayout;
            {
                pFreakLayout->addWidget(FreqNLab,1,1);
                pFreakLayout->addWidget(Freq1Lab,2,1);
                pFreakLayout->addWidget(DsiLab,3,1);
                pFreakLayout->addWidget(T2lab, 4, 1);
                pFreakLayout->addWidget(tauLab, 5, 1);
                pFreakLayout->addWidget(m_pdeFreqN,1,2);
                pFreakLayout->addWidget(m_pdeFreq1,2,2);
                pFreakLayout->addWidget(m_pdeZeta,3,2);
                pFreakLayout->addWidget(m_pdeT2,4,2);
                pFreakLayout->addWidget(m_pdeTau,5,2);
                pFreakLayout->addWidget(FreqUnit1,1,3);
                pFreakLayout->addWidget(FreqUnit2,2,3);
                pFreakLayout->addWidget(T2Unit,4,3);

                pFreakBox->setLayout(pFreakLayout);
            }
        }

        //_____________Eigenvalue data box________________________
        QGroupBox *pEigenBox = new QGroupBox(tr("Eigenvalues"));
        {
            QGridLayout *pEigenLayout = new QGridLayout;
            {
                QLabel *LabValue = new QLabel("l=");
                QFont SymbolFont("Symbol");
                LabValue->setFont(SymbolFont);
                QLabel *LabVect1 = new QLabel("v1=");
                QLabel *LabVect2 = new QLabel("v2=");
                QLabel *LabVect3 = new QLabel("v3=");
                QLabel *LabVect4 = new QLabel("v4=");
                pEigenLayout->addWidget(LabValue,1,1);
                pEigenLayout->addWidget(LabVect1,2,1);
                pEigenLayout->addWidget(LabVect2,3,1);
                pEigenLayout->addWidget(LabVect3,4,1);
                pEigenLayout->addWidget(LabVect4,5,1);
                m_pleEigenValue = new QLineEdit("2+4i");
                m_pleEigenVector1 = new QLineEdit("3-7i");
                m_pleEigenVector2 = new QLineEdit("4-6i");
                m_pleEigenVector3 = new QLineEdit("2.76-1.8782i");
                m_pleEigenVector4 = new QLineEdit("3.4567+9.2746i");
                m_pleEigenValue->setAlignment(Qt::AlignRight);
                m_pleEigenVector1->setAlignment(Qt::AlignRight);
                m_pleEigenVector2->setAlignment(Qt::AlignRight);
                m_pleEigenVector3->setAlignment(Qt::AlignRight);
                m_pleEigenVector4->setAlignment(Qt::AlignRight);
                m_pleEigenValue->setEnabled(false);
                m_pleEigenVector1->setEnabled(false);
                m_pleEigenVector2->setEnabled(false);
                m_pleEigenVector3->setEnabled(false);
                m_pleEigenVector4->setEnabled(false);
                pEigenLayout->addWidget(m_pleEigenValue,  1,2);
                pEigenLayout->addWidget(m_pleEigenVector1,2,2);
                pEigenLayout->addWidget(m_pleEigenVector2,3,2);
                pEigenLayout->addWidget(m_pleEigenVector3,4,2);
                pEigenLayout->addWidget(m_pleEigenVector4,5,2);
                pEigenBox->setLayout(pEigenLayout);
            }
        }

        //    ___________3D Animation box_________
        QGroupBox *pAnimationBox = new QGroupBox(tr("Animation"));
        {
            QGridLayout *pAnimSpeedLayout = new QGridLayout;
            {
                QLabel *LabSpeed = new QLabel(tr("Speed"));
                LabSpeed->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                m_pdAnimationSpeed  = new QDial();
                m_pdAnimationSpeed->setMinimum(0);
                m_pdAnimationSpeed->setMaximum(400);
                m_pdAnimationSpeed->setSliderPosition(m_ModeInterval);
                m_pdAnimationSpeed->setNotchesVisible(true);
                m_pdAnimationSpeed->setSingleStep(20);
                m_pdAnimationSpeed->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


                pAnimSpeedLayout->addWidget(m_pdAnimationSpeed,1,1);
                pAnimSpeedLayout->addWidget(LabSpeed,2,1);

                QLabel *LabAmplitude = new QLabel(tr("Amplitude"));
                LabAmplitude->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                m_pdAnimationAmplitude  = new QDial();
                m_pdAnimationAmplitude->setMinimum(0);
                m_pdAnimationAmplitude->setMaximum(1000);
                m_pdAnimationAmplitude->setSliderPosition(int(m_ModeAmplitude*500));
                m_pdAnimationAmplitude->setNotchesVisible(true);
                m_pdAnimationAmplitude->setSingleStep(20);
                m_pdAnimationAmplitude->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                pAnimSpeedLayout->addWidget(m_pdAnimationAmplitude,1,2);
                pAnimSpeedLayout->addWidget(LabAmplitude,2,2);
            }

            QVBoxLayout *pAnimationCommandsLayout = new QVBoxLayout;
            {
                m_ppbAnimate = new QPushButton(tr("Animate"));
                m_ppbAnimate->setCheckable(true);
                m_ppbAnimateRestart = new QPushButton(tr("Restart"));
                pAnimationCommandsLayout->addWidget(m_ppbAnimateRestart);
                pAnimationCommandsLayout->addWidget(m_ppbAnimate);
            }

            QHBoxLayout *pStepLayout = new  QHBoxLayout;
            {
                m_pdeModeStep = new DoubleEdit(0.01,3);
                QLabel *StepLabel = new QLabel(tr("Time Step ="));
                QLabel *StepUnit  = new QLabel(tr("s"));
                pStepLayout->addWidget(StepLabel);
                pStepLayout->addWidget(m_pdeModeStep);
                pStepLayout->addWidget(StepUnit);
            }
            QVBoxLayout *pAnimationLayout = new QVBoxLayout;
            {
                pAnimationLayout->addLayout(pStepLayout);
                pAnimationLayout->addLayout(pAnimSpeedLayout);
                pAnimationLayout->addLayout(pAnimationCommandsLayout);
            }
            pAnimationBox->setLayout(pAnimationLayout);
        }

        m_pswModeViewType= new QStackedWidget;
        m_pswModeViewType->addWidget(pEigenBox);
        m_pswModeViewType->addWidget(pAnimationBox);
        m_pswModeViewType->setCurrentIndex(0);

        QVBoxLayout *pRLLayout = new QVBoxLayout;
        {
            pRLLayout->addWidget(pRLModeBox);
            pRLLayout->addWidget(pFreakBox);
            pRLLayout->addWidget(m_pswModeViewType);
            pRLLayout->addStretch();
            pModeBox->setLayout(pRLLayout);
        }
    }

    //___________________Main Layout____________
    m_pswStack = new QStackedWidget;
    m_pswStack->addWidget(TimeBox);
    m_pswStack->addWidget(pModeBox);
    m_pswStack->setCurrentIndex(0);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(pStabilityDirBox);
    pMainLayout->addWidget(m_pswStack);
    setLayout(pMainLayout);
}



void StabViewDlg::setControls()
{
    QString str, strong;
    Units::getSpeedUnitLabel(str);

    blockSignals(true);

    m_prbLongDynamics->setChecked(s_pMiarex->m_bLongitudinal);
    m_prbLatDynamics->setChecked(!s_pMiarex->m_bLongitudinal);

    if(s_pMiarex->m_pCurWPolar && s_pMiarex->m_pCurWPolar->polarType()!=xfl::STABILITYPOLAR)
    {
//        m_pControlModel->setRowCount(0);
    }

    if(s_pMiarex->m_iView==xfl::STABTIMEVIEW)
    {
        m_pswStack->setCurrentIndex(0);
        m_pswInitialConditions->setCurrentIndex(s_pMiarex->m_StabilityResponseType);
        
        m_prbInitCondResponse->setChecked(s_pMiarex->m_StabilityResponseType==0);
        m_prbForcedResponse->setChecked(s_pMiarex->m_StabilityResponseType==1);
        m_prbModalResponse->setChecked(s_pMiarex->m_StabilityResponseType==2);
    }
    else if(s_pMiarex->m_iView==xfl::STABPOLARVIEW)
    {
        m_pswStack->setCurrentIndex(1);
        m_pswModeViewType->setCurrentIndex(0);
    }
    else if(s_pMiarex->m_iView==xfl::W3DVIEW)
    {
        m_pswStack->setCurrentIndex(1);
        m_pswModeViewType->setCurrentIndex(1);
    }
    setMode(m_iCurrentMode);

    strong = QString::fromUtf8("°/s");
    if(s_pMiarex->m_bLongitudinal)
    {
        m_plabStab1->setText(tr("u0="));
        m_plabStab2->setText(tr("w0="));
        m_plabStab3->setText(tr("q0="));
        m_plabUnit1->setText(str);
        m_plabUnit2->setText(str);
        m_plabUnit3->setText(strong);
    }
    else
    {
        m_plabStab1->setText(tr("v0="));
        m_plabStab2->setText(tr("p0="));
        m_plabStab3->setText(tr("r0="));
        m_plabUnit1->setText(str);
        m_plabUnit2->setText(strong);
        m_plabUnit3->setText(strong);
    }

    m_pdeStabVar1->setValue(s_pMiarex->m_TimeInput[0]);
    m_pdeStabVar2->setValue(s_pMiarex->m_TimeInput[1]);
    m_pdeStabVar3->setValue(s_pMiarex->m_TimeInput[2]);
    m_pdeTotalTime->setValue(s_pMiarex->m_TotalTime);
    m_pdeDeltat->setValue(s_pMiarex->m_Deltat);

    m_prbTimeMode1->setChecked(m_iCurrentMode%4==0);
    m_prbTimeMode2->setChecked(m_iCurrentMode%4==1);
    m_prbTimeMode3->setChecked(m_iCurrentMode%4==2);
    m_prbTimeMode4->setChecked(m_iCurrentMode%4==3);
    m_prbRLMode1->setChecked(m_iCurrentMode%4==0);
    m_prbRLMode2->setChecked(m_iCurrentMode%4==1);
    m_prbRLMode3->setChecked(m_iCurrentMode%4==2);
    m_prbRLMode4->setChecked(m_iCurrentMode%4==3);


    bool bStabPOpp = s_pMiarex->m_pCurWPolar && s_pMiarex->m_pCurWPolar->isStabilityPolar() && s_pMiarex->m_pCurPOpp && s_pMiarex->m_iView>=xfl::W3DVIEW;
    m_prbRLMode1->setEnabled(bStabPOpp);
    m_prbRLMode2->setEnabled(bStabPOpp);
    m_prbRLMode3->setEnabled(bStabPOpp);
    m_prbRLMode4->setEnabled(bStabPOpp);

    bool bEnableTimeCtrl = s_pMiarex->m_pCurPOpp && s_pMiarex->m_pCurPOpp->polarType()==xfl::STABILITYPOLAR && s_pMiarex->m_iView==xfl::STABTIMEVIEW;
    m_ppbAddCurve->setEnabled(bEnableTimeCtrl);
    m_ppbRenameCurve->setEnabled(m_pcbCurveList->count());
    m_ppbPlotStabGraph->setEnabled(m_pcbCurveList->count());
    m_ppbDeleteCurve->setEnabled(m_pcbCurveList->count());
    m_pcbCurveList->setEnabled(m_pcbCurveList->count());


    m_prbTimeMode1->setEnabled(bEnableTimeCtrl);
    m_prbTimeMode2->setEnabled(bEnableTimeCtrl);
    m_prbTimeMode3->setEnabled(bEnableTimeCtrl);
    m_prbTimeMode4->setEnabled(bEnableTimeCtrl);

    m_pdeStabVar1->setEnabled(bEnableTimeCtrl);
    m_pdeStabVar2->setEnabled(bEnableTimeCtrl);
    m_pdeStabVar3->setEnabled(bEnableTimeCtrl);
    m_pdeDeltat->setEnabled(bEnableTimeCtrl);
    m_pdeTotalTime->setEnabled(bEnableTimeCtrl);

    // Enable the 3D mode animation controls only if
    //   - the polar's type is 7
    //   - we have an active wopp
    //   - the StabilityView is 3
    bool bEnable3DAnimation = s_pMiarex->m_iView==xfl::W3DVIEW && s_pMiarex->m_pCurPOpp && s_pMiarex->m_pCurPOpp->polarType()==xfl::STABILITYPOLAR;
    m_ppbAnimate->setEnabled(bEnable3DAnimation);
    m_ppbAnimateRestart->setEnabled(bEnable3DAnimation);
    m_pdAnimationAmplitude->setEnabled(bEnable3DAnimation);
    m_pdAnimationSpeed->setEnabled(bEnable3DAnimation);

    m_pdeModeStep->setValue(s_pMiarex->m_Modedt);

    fillEigenThings();

    blockSignals(false);
}



void StabViewDlg::setTimeCurveStyle(QColor const &Color, int const&Style, int const &Width, bool const& bCurve, int const& PointStyle)
{
    if(!m_pCurve) return;
    for (int i=0; i<s_pMiarex->m_TimeGraph[0]->curveCount(); i++)
    {
        Curve *pCurve = s_pMiarex->m_TimeGraph[0]->curve(i);
        if(pCurve == m_pCurve)
        {
            for(int ig=0; ig<4; ig++)
            {
                pCurve = s_pMiarex->m_TimeGraph[ig]->curve(i);
                pCurve->setColor(Color);
                pCurve->setStipple(Style);
                pCurve->setWidth(Width);
                pCurve->setVisible(bCurve);
                pCurve->setPointStyle(PointStyle);
            }
                        
            return;
        }
    }
}


void StabViewDlg::onRenameCurve()
{
    if(!m_pCurve) return;

    QString NewName = "Test Name";
    NewNameDlg dlg(NewName, this);

    if(dlg.exec() != QDialog::Accepted) return;
    NewName = dlg.newName();

    for (int i=0; i<s_pMiarex->m_TimeGraph[0]->curveCount(); i++)
    {
        Curve *pCurve = s_pMiarex->m_TimeGraph[0]->curve(i);
        if(pCurve && (pCurve == m_pCurve))
        {
            for(int ig=0; ig<4; ig++)
            {
                pCurve = s_pMiarex->m_TimeGraph[ig]->curve(i);
                pCurve->setName(NewName);
            }

            fillCurveList();
            onPlotStabilityGraph();
            return;
        }
    }
}


void StabViewDlg::onSelChangeCurve(int sel)
{
    QString strong = m_pcbCurveList->itemText(sel);
    m_pCurve = s_pMiarex->m_TimeGraph[0]->curve(strong);
    m_pCurve->curveName(strong);
}


void StabViewDlg::onAddCurve()
{
    addCurve();
    if(m_pCurve)
    {
        int pos =m_pcbCurveList->findText(m_pCurve->curveName());
        m_pcbCurveList->setCurrentIndex(pos);
    }
    onPlotStabilityGraph();
}


void StabViewDlg::onDeleteCurve()
{
    if(!m_pCurve) return;
    QString CurveTitle = m_pCurve->curveName();
    for(int ig=0; ig<MAXTIMEGRAPHS; ig++)    s_pMiarex->m_TimeGraph[ig]->deleteCurve(CurveTitle);
    m_pCurve = nullptr;

    fillCurveList();
    m_pcbCurveList->setCurrentIndex(0);
    m_ppbPlotStabGraph->setEnabled(s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
    m_ppbRenameCurve->setEnabled(  s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
    m_ppbDeleteCurve->setEnabled(  s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
    m_pcbCurveList->setEnabled(    s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());

    if(m_pcbCurveList->count()) m_pCurve = s_pMiarex->m_TimeGraph[0]->curve(m_pcbCurveList->itemText(0));
    else                          m_pCurve = nullptr;

    s_pMiarex->createStabilityCurves();
    s_pMiarex->updateView();
    s_pMiarex->setFocus();
}


void StabViewDlg::addCurve()
{
    int nCurves = s_pMiarex->m_TimeGraph.at(0)->curveCount();
    QString strong = tr("New curve") + QString(" %1").arg(nCurves);

    Curve *pCurve = nullptr;
    for(int ig=0; ig<s_pMiarex->m_TimeGraph.size(); ig++)
    {
        pCurve = s_pMiarex->m_TimeGraph[ig]->addCurve();
        pCurve->setName(strong);
        if(ig==0) m_pCurve = pCurve;
    }

    if(pCurve) m_pcbCurveList->addItem(pCurve->curveName());

    m_ppbPlotStabGraph->setEnabled(s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
    m_ppbRenameCurve->setEnabled(  s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
    m_ppbDeleteCurve->setEnabled(  s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
    m_pcbCurveList->setEnabled(    s_pMiarex->m_pCurPOpp && m_pcbCurveList->count());
}


void StabViewDlg::fillCurveList()
{
    m_pcbCurveList->clear();
    QString strong;
    for(int i=0; i<s_pMiarex->m_TimeGraph[0]->curveCount(); i++)
    {
        s_pMiarex->m_TimeGraph[0]->curve(i)->curveName(strong);
        m_pcbCurveList->addItem(strong);
    }
    if(m_pCurve)
    {
        int sel = m_pcbCurveList->findText(m_pCurve->curveName());
        m_pcbCurveList->setCurrentIndex(sel);
    }
}


double StabViewDlg::getControlInput(const double &time)
{
    double t1, t2, in1, in2;
    t1 = t2 = 0.0;
    t1 = m_pControlModel->index(0, 0, QModelIndex()).data().toDouble();
    for(int i=1; i<m_pControlModel->rowCount()-1; i++)
    {
        t2 = m_pControlModel->index(i, 0, QModelIndex()).data().toDouble();
        if(t1<=time && time<t2)
        {
            in1 = m_pControlModel->index(i-1, 1, QModelIndex()).data().toDouble();
            in2 = m_pControlModel->index(i,   1, QModelIndex()).data().toDouble();
            return (in1 + (time-t1) * (in2-in1)/(t2-t1))*PI/180.0;
        }
        t1 = t2;
    }
    return 0.0;
}











