/****************************************************************************

    Optim2d Class
    Copyright (C) 2021 André Deperrois

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

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>

#include "optim2d.h"
#include <misc/options/settingswt.h>
#include <twodwidgets/foilwt.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/optim2d/gatask.h>
#include <xdirect/optim2d/mopsotask2d.h>
#include <xdirect/optim2d/optstructures.h>
#include <xflcore/constants.h>
#include <xflcore/xflcore.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/curve.h>
#include <xflobjects/editors/renamedlg.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects2d/polar.h>
#include <xflwidgets/customwts/actiondelegate.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xfoil.h>

QByteArray Optim2d::s_LeftSplitterSizes;
QByteArray Optim2d::s_HSplitterSizes;
QByteArray Optim2d::s_VSplitterSizes;
QByteArray Optim2d::s_Geometry;

double Optim2d::s_Alpha           = 0.0;
double Optim2d::s_Re              = 1.0e6;
double Optim2d::s_Mach            = 0.0;
double Optim2d::s_NCrit           = 9.0;
double Optim2d::s_XtrTop          = 1.0;
double Optim2d::s_XtrBot          = 1.0;

bool Optim2d::s_bTEFlap = false;
double Optim2d::s_FlapAngleMin=0;
double Optim2d::s_FlapAngleMax=10;
double Optim2d::s_XHinge=0.7;
double Optim2d::s_YHinge=0.5;



bool Optim2d::s_bCl = true;
int Optim2d::s_ClObjType = 1;
double Optim2d::s_Cl              = 0.0;
double Optim2d::s_ClMaxError      = 1.e-3;

bool Optim2d::s_bCd = true;
int Optim2d::s_CdObjType = -1;
double Optim2d::s_Cd              = 0.01;
double Optim2d::s_CdMaxError      = 1.e-4;

bool Optim2d::s_bClCd = false;
int Optim2d::s_ClCdObjType = 1;
double Optim2d::s_ClCd            = 31;
double Optim2d::s_ClCdMaxError    = 1;

bool Optim2d::s_bCpmin = false;
int Optim2d::s_CpMinObjType = 1;
double Optim2d::s_Cpmin           = -0.5;
double Optim2d::s_CpMaxError      = 1.e-3;

bool Optim2d::s_bCm = false;
int Optim2d::s_CmObjType = 0;
double Optim2d::s_Cm              = 0;
double Optim2d::s_CmMaxError      = 1.e-3;

bool Optim2d::s_bCm0 = false;
int Optim2d::s_Cm0ObjType = 0;
double Optim2d::s_Cm0             = 0;
double Optim2d::s_Cm0MaxError     = 1.e-3;

int    Optim2d::s_HHn             = 6;
double Optim2d::s_HHt1            = 1.7;
double Optim2d::s_HHt2            = 1.0;
double Optim2d::s_HHmax           = 0.01;

Optim2d::Optim2d(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("2d Optimization");

    m_pFoil=nullptr;

    m_pBestFoil = new Foil;
    m_pPolar = new Polar;

    m_bIsSwarmValid  = false;
    m_FlapAngle = 0;
    m_iLE = -1;

    m_bModified = false;
    m_bSaved = true;

    m_pPSOTask = nullptr;

    m_Objective.resize(NOBJECTIVES);
    m_Objective[0] = {"Cl",      s_bCl,    s_Cl,    s_ClMaxError,   xfl::MAXIMIZE};
    m_Objective[1] = {"Cd",      s_bCd,    s_Cd,    s_CdMaxError,   xfl::MINIMIZE};
    m_Objective[2] = {"Cl/Cd",   s_bClCd,  s_ClCd,  s_ClCdMaxError, xfl::MAXIMIZE};
    m_Objective[3] = {"Cp_min",  s_bCpmin, s_Cpmin, s_CpMaxError,   xfl::MAXIMIZE};
    m_Objective[4] = {"Cm",      s_bCm,    s_Cm,    s_CmMaxError,   xfl::EQUALIZE};
    m_Objective[5] = {"Cm0",     s_bCm0,   s_Cm0,   s_Cm0MaxError,  xfl::EQUALIZE};

    switch(s_ClObjType)
    {
        default:
        case  0:  m_Objective[0].m_Type=xfl::EQUALIZE;  break;
        case -1:  m_Objective[0].m_Type=xfl::MINIMIZE;  break;
        case  1:  m_Objective[0].m_Type=xfl::MAXIMIZE;  break;
    }
    switch(s_CdObjType)
    {
        default:
        case  0:  m_Objective[1].m_Type=xfl::EQUALIZE;  break;
        case -1:  m_Objective[1].m_Type=xfl::MINIMIZE;  break;
        case  1:  m_Objective[1].m_Type=xfl::MAXIMIZE;  break;
    }
    switch(s_ClCdObjType)
    {
        default:
        case  0:  m_Objective[2].m_Type=xfl::EQUALIZE;  break;
        case -1:  m_Objective[2].m_Type=xfl::MINIMIZE;  break;
        case  1:  m_Objective[2].m_Type=xfl::MAXIMIZE;  break;
    }
    switch(s_CpMinObjType)
    {
        default:
        case  0:  m_Objective[3].m_Type=xfl::EQUALIZE;  break;
        case -1:  m_Objective[3].m_Type=xfl::MINIMIZE;  break;
        case  1:  m_Objective[3].m_Type=xfl::MAXIMIZE;  break;
    }
    switch(s_CmObjType)
    {
        default:
        case  0:  m_Objective[4].m_Type=xfl::EQUALIZE;  break;
        case -1:  m_Objective[4].m_Type=xfl::MINIMIZE;  break;
        case  1:  m_Objective[4].m_Type=xfl::MAXIMIZE;  break;
    }
    switch(s_Cm0ObjType)
    {
        default:
        case  0:  m_Objective[5].m_Type=xfl::EQUALIZE;  break;
        case -1:  m_Objective[5].m_Type=xfl::MINIMIZE;  break;
        case  1:  m_Objective[5].m_Type=xfl::MAXIMIZE;  break;
    }
    setupLayout();

    fillObjectives();

    for(int ig=0; ig<NOBJECTIVES; ig++)
    {
        OptObjective const &obj = m_Objective.at(ig);
        m_ObjGraph[ig].setYTitle(obj.m_Name);
        Curve *pCurve = m_ObjGraph[ig].addCurve();
        pCurve->setName(obj.m_Name);
        pCurve->setWidth(2);
    }

    connectSignals();
    onPlotHH();
}


Optim2d::~Optim2d()
{
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];

    delete m_pBestFoil;
    delete m_pPolar;

    if(m_pPSOTask) delete m_pPSOTask;
}


void Optim2d::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHSplitter = new QSplitter(::Qt::Horizontal);
        {
            m_pHSplitter->setChildrenCollapsible(false);
            QFrame *pLeftFrame = new QFrame();
            {
                QVBoxLayout *pLeftLayout = new QVBoxLayout;
                {
                    m_pLeftSplitter = new QSplitter(Qt::Vertical);
                    {
                        QTabWidget *ptwMain = new QTabWidget;
                        {
                            QFrame *pXFoilPage = new  QFrame;
                            {
                                QGridLayout *pXFoilLayout = new QGridLayout;
                                {
                                    QLabel *plabAlpha = new QLabel("Alpha:");
                                    m_pdeAlpha = new DoubleEdit(s_Alpha);
                                    QLabel *plabDeg = new QLabel(QChar(0260));

                                    QLabel *plabRe = new QLabel("Reynolds:");
                                    m_pdeRe   = new DoubleEdit(s_Re);

                                    QLabel *plabMa = new QLabel("Mach:");
                                    m_pdeMach = new DoubleEdit(s_Mach);

                                    QLabel *plabNCrit = new QLabel("NCrit:");
                                    m_pdeNCrit = new DoubleEdit(s_NCrit);

                                    QLabel *plabXtrTop = new QLabel("Top transition:");
                                    m_pdeXtrTop = new DoubleEdit(s_XtrTop);

                                    QLabel *plabXtrBot = new QLabel("Bot. transition:");
                                    m_pdeXtrBot = new DoubleEdit(s_XtrBot);

                                    m_ppbXFoilRun = new  QPushButton("Run XFoil");

                                    pXFoilLayout->addWidget(plabAlpha,     1, 1);
                                    pXFoilLayout->addWidget(m_pdeAlpha,    1, 2);
                                    pXFoilLayout->addWidget(plabDeg,       1, 3);
                                    pXFoilLayout->addWidget(plabRe,        2, 1);
                                    pXFoilLayout->addWidget(m_pdeRe,       2, 2);
                                    pXFoilLayout->addWidget(plabMa,        3, 1);
                                    pXFoilLayout->addWidget(m_pdeMach,     3, 2);
                                    pXFoilLayout->addWidget(plabNCrit,     4, 1);
                                    pXFoilLayout->addWidget(m_pdeNCrit,    4, 2);
                                    pXFoilLayout->addWidget(plabXtrTop,    5, 1);
                                    pXFoilLayout->addWidget(m_pdeXtrTop,   5, 2);
                                    pXFoilLayout->addWidget(plabXtrBot,    6, 1);
                                    pXFoilLayout->addWidget(m_pdeXtrBot,   6, 2);
                                    pXFoilLayout->addWidget(m_ppbXFoilRun, 8, 1,1,3);

                                    pXFoilLayout->setRowStretch(7,1);
                                    pXFoilLayout->setColumnStretch(1,1);
                                    pXFoilLayout->setColumnStretch(2,1);
                                }
                                pXFoilPage->setLayout(pXFoilLayout);
                            }

                            QFrame *pHHPage = new QFrame;
                            {
                                QVBoxLayout *pHHPageLayout = new QVBoxLayout;
                                {
                                    QGridLayout *pHHLayout = new QGridLayout;
                                    {

                                        QLabel *plabEq = new QLabel;
                                        if(DisplayOptions::isLightTheme())
                                            plabEq->setPixmap(QPixmap(QString::fromUtf8(":/images/HH.png")));
                                        else
                                            plabEq->setPixmap(QPixmap(QString::fromUtf8(":/images/HH_inv.png")));

                                        QLabel *plabNHH = new QLabel("Nb. of functions/side:");
                                        m_pieNHH = new IntEdit(s_HHn/2); // per side
                                        m_pieNHH->setToolTip("The number of design variables is twice the number of bump functions.\nRecommendation: n=3");
                                        QLabel *plabt1 = new QLabel("t1:");
                                        m_pdeHHt1 = new DoubleEdit(s_HHt1);
                                        m_pdeHHt1->setToolTip("This parameter controls the location of the maximum point of the bump functions.\n"
                                                              "Recommendation: 1 <= t1<= 2");
                                        QLabel *plabt2 = new QLabel("t2:");
                                        m_pdeHHt2 = new DoubleEdit(s_HHt2);
                                        m_pdeHHt2->setToolTip("This parameter controls the width of the bump functions.\n"
                                                              "Recommendation: 0.5 <= t2 <= 2");
                                        QLabel *plabLax = new QLabel("Max. HH amplitude:");
                                        m_pdeHHmax = new DoubleEdit(s_HHmax*100);
                                        m_pdeHHmax->setToolTip("This parameter controls the amplitude of the bump functions.\n"
                                                               "The greater the amplitude the larger is  the design space, "
                                                               "however large amplitudes will lead to wobbly surfaces and may hinder XFoil's convergence.\n"
                                                               "Recommendation; 1.5% or less.");
                                        QLabel *plabPercent = new QLabel("% Ch.");

                                        pHHLayout->addWidget(plabNHH,     1, 1);
                                        pHHLayout->addWidget(m_pieNHH,    1, 2);
                                        pHHLayout->addWidget(plabLax,     2, 1);
                                        pHHLayout->addWidget(m_pdeHHmax,  2, 2);
                                        pHHLayout->addWidget(plabPercent, 2, 3);
                                        pHHLayout->addWidget(plabt1,      3, 1);
                                        pHHLayout->addWidget(m_pdeHHt1,   3, 2);
                                        pHHLayout->addWidget(plabt2,      4, 1);
                                        pHHLayout->addWidget(m_pdeHHt2,   4, 2);

                                        pHHLayout->addWidget(plabEq,5,1,1,3,  Qt::AlignCenter);

//                                        pHHLayout->setRowStretch(6,1);
                                        pHHLayout->setColumnStretch(1, 1);
                                        pHHLayout->setColumnStretch(2, 1);
                                    }


                                    m_pHHGraphWt = new GraphWt;
                                    {
//                                        m_pHHGraphWt->setSizePolicy(QSizePolicy::MinimumExpanding,  QSizePolicy::MinimumExpanding);
                                        m_pHHGraphWt->setGraph(&m_HHGraph);
                                        m_HHGraph.setXTitle("x");
                                        m_HHGraph.setYTitle("Amplitude %Chord");
                                        m_HHGraph.setAuto(true);
                                        m_HHGraph.copySettings(&Settings::s_RefGraph);
                                    }

                                    pHHPageLayout->addLayout(pHHLayout);
                                    pHHPageLayout->addWidget(m_pHHGraphWt);
                                    pHHPageLayout->setStretchFactor(pHHLayout,1);
                                    pHHPageLayout->setStretchFactor(m_pHHGraphWt,5);
                                }

                                pHHPage->setLayout(pHHPageLayout);
                            }

                            QFrame *pFlapPage = new QFrame;
                            {
                                QGridLayout *pFlapDataLayout = new QGridLayout;
                                {
                                    m_pchTEFlap = new QCheckBox(tr("T.E. Flap"));
                                    m_pchTEFlap->setToolTip("Activate to set the T.E. flap angle as a design variable");
                                    m_pchTEFlap->setChecked(s_bTEFlap);
                                    QLabel *pLabMin = new QLabel("Min.");
                                    QLabel *pLabMax = new QLabel("Max.");
                                    m_pdeFlapAngleMin   = new DoubleEdit(s_FlapAngleMin);
                                    m_pdeFlapAngleMax   = new DoubleEdit(s_FlapAngleMax);
                                    m_pdeTEXHinge    = new DoubleEdit(s_XHinge*100.0);
                                    m_pdeTEYHinge    = new DoubleEdit(s_YHinge*100.0);

                                    m_pdeFlapAngleMin->setEnabled(s_bTEFlap);
                                    m_pdeFlapAngleMax->setEnabled(s_bTEFlap);

                                    m_pdeTEXHinge->setEnabled(s_bTEFlap);
                                    m_pdeTEYHinge->setEnabled(s_bTEFlap);

                                    QLabel *pLabAngle = new QLabel(tr("Flap Angle"));
                                    QLabel *pLab2 = new QLabel(QString::fromUtf8("° (")+tr("+ is down") +")");
                                    QLabel *pLabXHinge = new QLabel(tr("Hinge X Position"));
                                    QLabel *pLab4 = new QLabel(tr("% Chord"));
                                    QLabel *pLabYHinge = new QLabel(tr("Hinge Y Position"));
                                    QLabel *pLab6 = new QLabel(tr("% Thickness"));

                                    pFlapDataLayout->addWidget(m_pchTEFlap, 1, 1);

                                    pFlapDataLayout->addWidget(pLabMin,           3, 2, Qt::AlignCenter);
                                    pFlapDataLayout->addWidget(pLabMax,           3, 3, Qt::AlignCenter);
                                    pFlapDataLayout->addWidget(pLabAngle,         4, 1);
                                    pFlapDataLayout->addWidget(m_pdeFlapAngleMin, 4, 2);
                                    pFlapDataLayout->addWidget(m_pdeFlapAngleMax, 4, 3);
                                    pFlapDataLayout->addWidget(pLab2,             4, 4);

                                    pFlapDataLayout->addWidget(pLabXHinge,     6, 1);
                                    pFlapDataLayout->addWidget(m_pdeTEXHinge,  6, 2);
                                    pFlapDataLayout->addWidget(pLab4,          6, 3);

                                    pFlapDataLayout->addWidget(pLabYHinge,     7, 1);
                                    pFlapDataLayout->addWidget(m_pdeTEYHinge,  7, 2);
                                    pFlapDataLayout->addWidget(pLab6,          7, 3);

                                    pFlapDataLayout->setRowStretch(5,1);
                                    pFlapDataLayout->setRowStretch(8,7);
                                }
                                pFlapPage->setLayout(pFlapDataLayout);
                            }

                            QFrame *pAlgoPage= new QFrame;
                            {
                                QGridLayout *pMOSPSOLayout = new QGridLayout;
                                {
                                    QLabel *plabPopSize = new QLabel("Swarm size:");
                                    m_piePopSize = new IntEdit(OptimTask::s_PopSize);
                                    m_piePopSize->setToolTip(tr("Recommendation: 7 to 15"));

                                    QLabel *plabMaxIter = new QLabel("Max. iterations:");
                                    m_pieMaxIter = new IntEdit(OptimTask::s_MaxIter);

                                    QLabel *plabInertia = new QLabel("Inertia weight:");
                                    m_pdeInertiaWeight = new DoubleEdit(MOPSOTask::s_InertiaWeight);
                                    m_pdeInertiaWeight->setToolTip("The inertia weight determines the influence of the particle's\n"
                                                                   "current velocity on its updated velocity.\n"
                                                                   "Recommendation: 0.3");

                                    QLabel *plabCognitive = new QLabel("Cognitive weight:");
                                    m_pdeCognitiveWeight = new DoubleEdit(MOPSOTask::s_CognitiveWeight);
                                    m_pdeCognitiveWeight->setToolTip("The cognitive weights determines the influence of the particle's best position.\n"
                                                                     "Recommendation: 0.7");

                                    QLabel *plabSocial = new QLabel("Social weight:");
                                    m_pdeSocialWeight = new DoubleEdit(MOPSOTask::s_SocialWeight);
                                    m_pdeSocialWeight->setToolTip("The social weight determines the influence of the global best-known position.\n"
                                                                  "Recommendation: 0.7");

                                    QLabel *plabProbRegen = new QLabel("Regeneration probability:");
                                    m_pdeProbaRegen = new DoubleEdit(MOPSOTask::s_ProbRegenerate*100.0);
                                    m_pdeProbaRegen->setToolTip("The probability that a particle will be re-created at a random position at each iteration.\n"
                                                                "Increases the likelyhood that the swarm will not get stuck on a local minimum.\n"
                                                                "Recommendation: 5% to 25%");
                                    QLabel *pLabPercent = new QLabel("%");

                                    m_pchMultithread = new QCheckBox("Multithreaded");
                                    m_pchMultithread->setChecked(OptimTask::s_bMultiThreaded);

                                    pMOSPSOLayout->addWidget(plabPopSize,          1, 1);
                                    pMOSPSOLayout->addWidget(m_piePopSize,         1, 2);

                                    pMOSPSOLayout->addWidget(plabMaxIter,          2, 1);
                                    pMOSPSOLayout->addWidget(m_pieMaxIter,         2, 2);

                                    pMOSPSOLayout->addWidget(plabInertia,          5, 1);
                                    pMOSPSOLayout->addWidget(m_pdeInertiaWeight,   5, 2);

                                    pMOSPSOLayout->addWidget(plabCognitive,        6, 1);
                                    pMOSPSOLayout->addWidget(m_pdeCognitiveWeight, 6, 2);

                                    pMOSPSOLayout->addWidget(plabSocial,           7, 1);
                                    pMOSPSOLayout->addWidget(m_pdeSocialWeight,    7, 2);

                                    pMOSPSOLayout->addWidget(plabProbRegen,        8, 1);
                                    pMOSPSOLayout->addWidget(m_pdeProbaRegen,      8, 2);
                                    pMOSPSOLayout->addWidget(pLabPercent,          8, 3);

                                    pMOSPSOLayout->addWidget(m_pchMultithread,     9, 1);

                                    pMOSPSOLayout->setRowStretch(10,1);
                                }

                                pAlgoPage->setLayout(pMOSPSOLayout);
                            }

                            QFrame *pTargetPage = new QFrame;
                            {
                                QVBoxLayout *pObjectiveLayout  = new QVBoxLayout;
                                {
                                    m_pcptObjective = new CPTableView;
                                    {
                                        m_pcptObjective->setEditable(true);
                                        m_pObjModel = new QStandardItemModel(this);
                                        m_pObjModel->setRowCount(1);//temporary
                                        m_pObjModel->setColumnCount(5);
                                        m_pObjModel->setHeaderData(0, Qt::Horizontal, "Active");
                                        m_pObjModel->setHeaderData(1, Qt::Horizontal, "Objective");
                                        m_pObjModel->setHeaderData(2, Qt::Horizontal, "Type");
                                        m_pObjModel->setHeaderData(3, Qt::Horizontal, "Target");
                                        m_pObjModel->setHeaderData(4, Qt::Horizontal, "Max. error");
                                        for(int icol=0; icol<m_pObjModel->columnCount(); icol++)
                                            m_pObjModel->setHeaderData(icol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
                                        m_pcptObjective->setModel(m_pObjModel);

                                        m_pObjDelegate = new ActionDelegate(this);
                                        m_pObjDelegate->setCheckColumn(0);
                                        m_pObjDelegate->setActionColumn(-1);
                                        m_pObjDelegate->setDigits({-1,0,0,5,5});
                                        m_pObjDelegate->setName("Objecives");
                                        m_pcptObjective->setItemDelegate(m_pObjDelegate);
                                    }

                                    m_ppbMakeSwarm = new QPushButton("Make random population");
                                    m_ppbSwarm = new QPushButton(tr("Swarm"));
                                    m_ppbStoreBestFoil = new QPushButton("Store best foil");
                                    m_ppbStoreBestFoil->setToolTip("Adds the current best foil to the database");
                                    m_ppbContinueBest = new QPushButton("Continue from current best");
                                    m_ppbContinueBest->setToolTip("Uses the current best foil as the basis for further optimization");

                                    pObjectiveLayout->addWidget(m_pcptObjective);

                                    pObjectiveLayout->addWidget(m_ppbMakeSwarm);
                                    pObjectiveLayout->addWidget(m_ppbSwarm);
                                    pObjectiveLayout->addWidget(m_ppbStoreBestFoil);
                                    pObjectiveLayout->addWidget(m_ppbContinueBest);
                                }
                                pTargetPage->setLayout(pObjectiveLayout);
                            }

                            ptwMain->addTab(pXFoilPage,  "XFoil");
                            ptwMain->addTab(pHHPage,     "Hicks-Henne");
                            ptwMain->addTab(pFlapPage,   "T.E. flap");
                            ptwMain->addTab(pAlgoPage,   "MOPSO");
                            ptwMain->addTab(pTargetPage, "Optimizer");

                            connect(ptwMain,  SIGNAL(currentChanged(int)), SLOT(onResizeColumns()));
                        }

                        m_ppto = new PlainTextOutput;
                        QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                        m_ppto->setFont(fixedfnt);

                        m_pLeftSplitter->addWidget(ptwMain);
                        m_pLeftSplitter->addWidget(m_ppto);

                        m_pLeftSplitter->setStretchFactor(1,1);
                        m_pLeftSplitter->setStretchFactor(2,5);

                        m_pLeftSplitter->setChildrenCollapsible(false);
                    }

                    QHBoxLayout  *pBottomLayout = new QHBoxLayout;
                    {
                        QLabel *pFlow5Link = new QLabel;
                        pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Tutorials/Optim2d.html>https://flow5.tech/docs/.../Optim2d.html</a>");
                        pFlow5Link->setOpenExternalLinks(true);
                        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                        pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

                        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
                        {
                            QPushButton *ppbClear = new QPushButton("Clear");
                            connect(ppbClear, SIGNAL(clicked()), m_ppto, SLOT(clear()));
                            m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);

                            connect(m_pButtonBox,       SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                        }
                        pBottomLayout->addWidget(pFlow5Link);
                        pBottomLayout->addStretch();
                        pBottomLayout->addWidget(m_pButtonBox);
                    }
                    pLeftLayout->addWidget(m_pLeftSplitter);
                    pLeftLayout->addLayout(pBottomLayout);
                }
                pLeftFrame->setLayout(pLeftLayout);
            }

            m_pVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pVSplitter->setChildrenCollapsible(false);
                QFrame *pTopFrame = new QFrame;
                {
                    QHBoxLayout *pTopLayout = new QHBoxLayout;
                    {
                        QGridLayout *pObjGraphLayout = new QGridLayout;
                        {
                            for(int ig=0; ig<NOBJECTIVES; ig++)
                            {
                                m_pObjGraphWt[ig] = new GraphWt;
                                m_pObjGraphWt[ig]->setGraph(&m_ObjGraph[ig]);
                                m_ObjGraph[ig].setXTitle("Iter");
                                m_ObjGraph[ig].setYTitle("Objective");//temp
                                m_ObjGraph[ig].copySettings(&Settings::s_RefGraph);
                            }
                        }
                        pObjGraphLayout->addWidget(m_pObjGraphWt[0],1,1);
                        pObjGraphLayout->addWidget(m_pObjGraphWt[1],1,2);
                        pObjGraphLayout->addWidget(m_pObjGraphWt[2],2,1);
                        pObjGraphLayout->addWidget(m_pObjGraphWt[3],2,2);
                        pObjGraphLayout->addWidget(m_pObjGraphWt[4],3,1);
                        pObjGraphLayout->addWidget(m_pObjGraphWt[5],3,2);

                        pTopLayout->addLayout(pObjGraphLayout);
//                        pTopLayout->addWidget(m_pParetoGraphWt);
                    }
                    pTopFrame->setLayout(pTopLayout);
                }

                m_pFoilWt = new FoilWt;

                m_pVSplitter->addWidget(pTopFrame);
                m_pVSplitter->addWidget(m_pFoilWt);
                m_pVSplitter->setStretchFactor(0,1);
            }

            m_pHSplitter->addWidget(pLeftFrame);
            m_pHSplitter->addWidget(m_pVSplitter);
            m_pHSplitter->setStretchFactor(1,1);
        }

        pMainLayout->addWidget(m_pHSplitter);

    }
    setLayout(pMainLayout);
}


void Optim2d::connectSignals()
{
    connect(m_pcptObjective,    SIGNAL(pressed(QModelIndex)),    SLOT(onObjTableClicked(QModelIndex)));
    connect(m_pHSplitter,       SIGNAL(splitterMoved(int,int)),  SLOT(onResizeColumns()));

    connect(m_ppbMakeSwarm,     SIGNAL(clicked()),               SLOT(onMakeSwarm()));
    connect(m_ppbSwarm,         SIGNAL(clicked()),               SLOT(onRunOptimizer()));
    connect(m_ppbStoreBestFoil, SIGNAL(clicked()),               SLOT(onStoreBestFoil()));
    connect(m_ppbContinueBest,  SIGNAL(clicked()),               SLOT(onContinueBest()));
    connect(m_pchTEFlap,        SIGNAL(toggled(bool)),           SLOT(onTEFlapCheck()));

    connect(m_ppbXFoilRun,      SIGNAL(clicked()),               SLOT(onRunXFoil()));

    connect(m_pieNHH,        SIGNAL(valueChanged()), SLOT(onPlotHH()));
    connect(m_pdeHHt1,       SIGNAL(valueChanged()), SLOT(onPlotHH()));
    connect(m_pdeHHt2,       SIGNAL(valueChanged()), SLOT(onPlotHH()));
    connect(m_pdeHHmax,      SIGNAL(valueChanged()), SLOT(onPlotHH()));
}


void Optim2d::onTEFlapCheck()
{
    s_bTEFlap = m_pchTEFlap->isChecked();
    s_FlapAngleMin = m_pdeFlapAngleMin->value();
    s_FlapAngleMax = m_pdeFlapAngleMax->value();
    s_XHinge    = m_pdeTEXHinge->value()/100.0;
    s_YHinge    = m_pdeTEYHinge->value()/100.0;
    m_pdeFlapAngleMin->setEnabled(s_bTEFlap);
    m_pdeFlapAngleMax->setEnabled(s_bTEFlap);
    m_pdeTEXHinge->setEnabled(s_bTEFlap);
    m_pdeTEYHinge->setEnabled(s_bTEFlap);
}


void Optim2d::onResizeColumns()
{
    int nCols=m_pObjModel->columnCount()-1;
    QHeaderView *pHHeader = m_pcptObjective->horizontalHeader();
    pHHeader->setSectionResizeMode(nCols, QHeaderView::Stretch);
    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically
    double wt = double(m_pcptObjective->width());
    int wch = int(wt/10.0);
    int wType = int(wt/9.0);
    int w = int((wt-wch)/double(3)*0.95);
    m_pcptObjective->setColumnWidth(0, wch);
    m_pcptObjective->setColumnWidth(1, w);
    m_pcptObjective->setColumnWidth(2, wType);
    m_pcptObjective->setColumnWidth(3, w);
    m_pcptObjective->setColumnWidth(4, w);
}



void Optim2d::onObjTableClicked(QModelIndex index)
{
    if(index.row()>=m_pObjModel->rowCount()) return;
    if(index.column()!=0) return;
    bool bActive = m_pObjModel->data(index, Qt::UserRole).toBool(); // use a QVariant with the EditRole rather thant the QtCheckStateRole - not interested in Qt::PartiallyChecked
    bActive = !bActive; // toggle
    // update the range
    m_Objective[index.row()].m_bActive = bActive;
    QModelIndex chindex = m_pObjModel->index(index.row(), 0);
    m_pObjModel->setData(chindex, bActive, Qt::UserRole);
    m_pcptObjective->selectRow(chindex.row()); // only way found to force the repaint
    m_pcptObjective->setCurrentIndex(chindex);

//    m_ppto->onAppendThisPlainText("Invalidating swarm\n");
//    invalidateSwarm();
    update();
}


void Optim2d::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) onClose();
}


void Optim2d::reject()
{
    onClose();
}


void Optim2d::onClose()
{
    cancelTask();

    if(!m_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Question"), tr("Discard changes?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(resp != QMessageBox::Yes) return;
    }
    accept();
}


void Optim2d::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
    if(s_LeftSplitterSizes.length()>0) m_pLeftSplitter->restoreState(s_LeftSplitterSizes);
    if(s_HSplitterSizes.length()>0)    m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0)    m_pVSplitter->restoreState(s_VSplitterSizes);
    onResizeColumns();

    for(int ig=0; ig<NOBJECTIVES; ig++)  m_pObjGraphWt[ig]->update();
}


void Optim2d::resizeEvent(QResizeEvent *pEvent)
{
    onResizeColumns();
    QDialog::resizeEvent(pEvent);
}


void Optim2d::hideEvent(QHideEvent *)
{
    readData();

    s_Geometry = saveGeometry();
    s_LeftSplitterSizes = m_pLeftSplitter->saveState();
    s_HSplitterSizes    = m_pHSplitter->saveState();
    s_VSplitterSizes    = m_pVSplitter->saveState();
}


void Optim2d::loadSettings(QSettings &settings)
{
    settings.beginGroup("Optim2d");
    {
        s_Alpha           = settings.value("Alpha",           s_Alpha).toDouble();
        s_Re              = settings.value("Re",              s_Re).toDouble();
        s_Mach            = settings.value("Mach",            s_Mach).toDouble();
        s_NCrit           = settings.value("NCrit",           s_NCrit).toDouble();
        s_XtrTop          = settings.value("XtrTop",          s_XtrTop).toDouble();
        s_XtrBot          = settings.value("XtrBot",          s_XtrBot).toDouble();

        s_bTEFlap         = settings.value("TEFLap",          s_bTEFlap).toBool();
        s_FlapAngleMin    = settings.value("FlapAngleMin",    s_FlapAngleMin).toDouble();
        s_FlapAngleMax    = settings.value("FlapAngleMax",    s_FlapAngleMax).toDouble();
        s_XHinge          = settings.value("XHinge",          s_XHinge).toDouble();
        s_YHinge          = settings.value("YHinge",          s_YHinge).toDouble();

        s_bCl             = settings.value("ClActive",        s_bCl).toBool();
        s_ClObjType       = settings.value("ClObjType",       s_ClObjType).toInt();
        s_Cl              = settings.value("Cl",              s_Cl).toDouble();
        s_ClMaxError      = settings.value("ClMaxError",      s_ClMaxError).toDouble();

        s_bCd             = settings.value("CdActive",        s_bCl).toBool();
        s_CdObjType       = settings.value("CdObjType",       s_CdObjType).toInt();
        s_Cd              = settings.value("Cd",              s_Cd).toDouble();
        s_CdMaxError      = settings.value("CdMaxError",      s_CdMaxError).toDouble();

        s_bClCd           = settings.value("ClCdActive",      s_bClCd).toBool();
        s_ClCdObjType     = settings.value("ClCdObjType",     s_ClCdObjType).toInt();
        s_ClCd            = settings.value("ClCd",            s_ClCd).toDouble();
        s_ClCdMaxError    = settings.value("ClCdMaxError",    s_ClCdMaxError).toDouble();

        s_bCpmin          = settings.value("CpActive",        s_bCpmin).toBool();
        s_CpMinObjType    = settings.value("CpMinObjType",    s_CpMinObjType).toInt();
        s_Cpmin           = settings.value("Cp",              s_Cpmin).toDouble();
        s_CpMaxError      = settings.value("CpMaxError",      s_CpMaxError).toDouble();

        s_bCm             = settings.value("CmActive",        s_bCm).toBool();
        s_CmObjType       = settings.value("CmObjType",       s_CmObjType).toInt();
        s_Cm              = settings.value("Cm",              s_Cm).toDouble();
        s_CmMaxError      = settings.value("CmMaxError",      s_CmMaxError).toDouble();

        s_bCm0            = settings.value("Cm0Active",       s_bCm0).toBool();
        s_Cm0ObjType      = settings.value("Cm0ObjType",      s_Cm0ObjType).toInt();
        s_Cm0             = settings.value("Cm0",             s_Cm0).toDouble();
        s_Cm0MaxError     = settings.value("Cm0MaxError",     s_Cm0MaxError).toDouble();

        s_HHn             = settings.value("HHn",             s_HHn).toInt();
        s_HHt1            = settings.value("HHt1",            s_HHt1).toDouble();
        s_HHt2            = settings.value("HHt2",            s_HHt2).toDouble();
        s_HHmax           = settings.value("HHmax",           s_HHmax).toDouble();

        OptimTask::s_bMultiThreaded  = settings.value("bMultithread",    OptimTask::s_bMultiThreaded).toBool();
        OptimTask::s_PopSize         = settings.value("PopSize",         OptimTask::s_PopSize).toInt();
        OptimTask::s_MaxIter         = settings.value("MaxIter",         OptimTask::s_MaxIter).toInt();

        MOPSOTask::s_InertiaWeight   = settings.value("InertiaWeight",   MOPSOTask::s_InertiaWeight).toDouble();
        MOPSOTask::s_CognitiveWeight = settings.value("CognitiveWeight", MOPSOTask::s_CognitiveWeight).toDouble();
        MOPSOTask::s_SocialWeight    = settings.value("SocialWeight",    MOPSOTask::s_SocialWeight).toDouble();
        MOPSOTask::s_ProbRegenerate  = settings.value("ProbRegen",       MOPSOTask::s_ProbRegenerate).toDouble();

        s_Geometry           = settings.value("WindowGeom",        QByteArray()).toByteArray();
        s_LeftSplitterSizes  = settings.value("LeftSplitterSizes", QByteArray()).toByteArray();
        s_HSplitterSizes     = settings.value("HSplitterSizes",    QByteArray()).toByteArray();
        s_VSplitterSizes     = settings.value("VSplitterSizes",    QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void Optim2d::saveSettings(QSettings &settings)
{
    settings.beginGroup("Optim2d");
    {
        settings.setValue("Alpha",             s_Alpha);
        settings.setValue("Re",                s_Re);
        settings.setValue("Mach",              s_Mach);
        settings.setValue("NCrit",             s_NCrit);
        settings.setValue("XtrTop",            s_XtrTop);
        settings.setValue("XtrBot",            s_XtrBot);


        settings.setValue("TEFLap",            s_bTEFlap);
        settings.setValue("FlapAngleMin",      s_FlapAngleMin);
        settings.setValue("FlapAngleMax",      s_FlapAngleMax);
        settings.setValue("XHinge",            s_XHinge);
        settings.setValue("YHinge",            s_YHinge);


        settings.setValue("ClActive",          s_bCl);
        settings.setValue("ClObjType",         s_ClObjType);
        settings.setValue("Cl",                s_Cl);
        settings.setValue("ClMaxError",        s_ClMaxError);

        settings.setValue("CdActive",          s_bCd);
        settings.setValue("CdObjType",         s_CdObjType);
        settings.setValue("Cd",                s_Cd);
        settings.setValue("CdMaxError",        s_CdMaxError);

        settings.setValue("ClCdActive",        s_bClCd);
        settings.setValue("ClCdObjType",       s_ClCdObjType);
        settings.setValue("ClCd",              s_ClCd);
        settings.setValue("ClCdMaxError",      s_ClCdMaxError);

        settings.setValue("CpActive",          s_bCpmin);
        settings.setValue("CpMinObjType",      s_CpMinObjType);
        settings.setValue("Cp",                s_Cpmin);
        settings.setValue("CpMaxError",        s_CpMaxError);

        settings.setValue("CmActive",          s_bCm);
        settings.setValue("CmObjType",         s_CmObjType);
        settings.setValue("Cm",                s_Cm);
        settings.setValue("CmMaxError",        s_CmMaxError);

        settings.setValue("Cm0Active",         s_bCm0);
        settings.setValue("Cm0ObjType",        s_Cm0ObjType);
        settings.setValue("Cm0",               s_Cm0);
        settings.setValue("Cm0MaxError",       s_Cm0MaxError);

        settings.setValue("HHn",               s_HHn);
        settings.setValue("HHt1",              s_HHt1);
        settings.setValue("HHt2",              s_HHt2);
        settings.setValue("HHmax",             s_HHmax);

        settings.setValue("PopSize",           OptimTask::s_PopSize);
        settings.setValue("bMultithread",      OptimTask::s_bMultiThreaded);
        settings.setValue("MaxIter",           OptimTask::s_MaxIter);

        settings.setValue("InertiaWeight",     MOPSOTask::s_InertiaWeight);
        settings.setValue("CognitiveWeight",   MOPSOTask::s_CognitiveWeight);
        settings.setValue("SocialWeight",      MOPSOTask::s_SocialWeight);
        settings.setValue("ProbRegen",         MOPSOTask::s_ProbRegenerate);

        settings.setValue("WindowGeom",        s_Geometry);
        settings.setValue("LeftSplitterSizes", s_LeftSplitterSizes);
        settings.setValue("HSplitterSizes",    s_HSplitterSizes);
        settings.setValue("VSplitterSizes",    s_VSplitterSizes);
    }
    settings.endGroup();
}


void Optim2d::enableControls(bool bEnable)
{
    m_ppbXFoilRun->setEnabled(bEnable);
    m_ppbMakeSwarm->setEnabled(bEnable);
    m_ppbStoreBestFoil->setEnabled(bEnable);
    m_ppbContinueBest->setEnabled(bEnable);
}


Foil *Optim2d::onStoreBestFoil()
{
    Foil *pNewFoil = new Foil(*m_pBestFoil);

    pNewFoil->initFoil();
    pNewFoil->setFlap();

    QString basename = "Optimized";

    int iter = 1;
    QString name = basename + QString::asprintf("_%d", iter);
    while(Objects2d::foilExists(name))
    {
        name = basename + QString::asprintf("_%d", iter);
        iter++;
    }
    pNewFoil->setName(name);


    QString strange = QString::asprintf("Optimized at aoa=%g", s_Alpha) + QChar(0260);
    if(s_bTEFlap) strange += QString::asprintf("  Flap angle=%5.3g", m_FlapAngle) + QChar(0260);
    strange +=" for: ";
    for(int iobj=0; iobj<m_pPSOTask->nObjectives(); iobj++)
    {
        OptObjective const &obj = m_pPSOTask->objective(iobj);
        strange += " " + obj.m_Name;
        if     (obj.m_Type==xfl::EQUALIZE)   strange += QString::asprintf("=%.3g ", obj.m_Target);
        else if(obj.m_Type==xfl::MAXIMIZE)   strange += QString::asprintf(">=%.3g ", obj.m_Target);
        else if(obj.m_Type==xfl::MINIMIZE)   strange += QString::asprintf("<=%.3g ", obj.m_Target);
    }
    pNewFoil->setDescription(strange);

    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(this);
    renDlg.initDialog(&NameList, pNewFoil->name(), tr("Enter the foil's new name"));
    if(renDlg.exec() == QDialog::Rejected)
    {
        delete pNewFoil;
        return nullptr;
    }

    pNewFoil->setName(renDlg.newName());
    Objects2d::insertThisFoil(pNewFoil);

    pNewFoil->setColor(xfl::randomColor(DisplayOptions::isLightTheme()));
    outputText("Saved the current best foil with name: "+pNewFoil->name()+"\n");
    m_bSaved = true;

    m_bModified = true;

    return pNewFoil;
}


void Optim2d::onContinueBest()
{
    Foil *pNewBest = onStoreBestFoil();

    if(!pNewBest) return;

    setFoil(pNewBest); // continue from here
    m_bIsSwarmValid = false;
    outputText("Continuing with foil: "+pNewBest->name()+"\n");
    outputText("Invalidating current swarm\n");
    update();
}


void Optim2d::readData()
{
    s_Alpha        = m_pdeAlpha->value();
    s_Re           = m_pdeRe->value();
    s_Mach         = m_pdeMach->value();
    s_NCrit        = m_pdeNCrit->value();
    s_XtrTop       = m_pdeXtrTop->value();
    s_XtrBot       = m_pdeXtrBot->value();
    s_HHn          = m_pieNHH->value()*2;
    s_HHt1         = m_pdeHHt1->value();
    s_HHt2         = m_pdeHHt2->value();
    s_HHmax        = m_pdeHHmax->value()/100.0;

    s_bTEFlap      = m_pchTEFlap->isChecked();
    s_FlapAngleMin = m_pdeFlapAngleMin->value();
    s_FlapAngleMax = m_pdeFlapAngleMax->value();
    s_XHinge       = m_pdeTEXHinge->value()/100.0;
    s_YHinge       = m_pdeTEYHinge->value()/100.0;

    readObjectives();

    // temporary duplication
    OptimTask::s_MaxIter         = m_pieMaxIter->value();
    OptimTask::s_PopSize         = m_piePopSize->value();
    OptimTask::s_bMultiThreaded  = m_pchMultithread->isChecked();

    MOPSOTask::s_InertiaWeight   = m_pdeInertiaWeight->value();
    MOPSOTask::s_CognitiveWeight = m_pdeCognitiveWeight->value();
    MOPSOTask::s_SocialWeight    = m_pdeSocialWeight->value();
    MOPSOTask::s_ProbRegenerate  = m_pdeProbaRegen->value()/100.0;
}


void Optim2d::onMakeSwarm(bool bShow)
{
    if(m_pPSOTask && !m_pPSOTask->isFinished())
    {
        outputText("\n***Cannot make a swarm when a task is running***\n\n");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    outputText("\nMaking swarm...\n");
    m_ppto->update();

    for(int ig=0; ig<NOBJECTIVES; ig++)
    {
        Curve *pCurve = m_ObjGraph[ig].curve(0);
        if(pCurve) pCurve->clear();
    }

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];
    m_TempFoils.clear();
    update();

    readData();
    updatePolar();


    bool bConverged(false);
    double Cl(0), Cd(0), ClCd(0),  Cpmin(0), Cm(0), Cm0(0);
    runXFoil(m_pFoil, Cl, Cd,  ClCd, Cpmin, Cm, Cm0, bConverged); // required to determine the leading edge index for the specified AOA

/*    QString strange;
    strange = QString::asprintf("   Cl     = %7.3g\n"
                                "   Cd     = %7.3g\n"
                                "   Cl/Cd  = %7.3g\n"
                                "   Cp_min = %7.3g\n"
                                "   Cm     = %7.3g\n"
                                "   Cm0    = %7.3g\n\n",
                                Cl, Cd, ClCd, Cpmin, Cm, Cm0);
    outputText(strange);
    m_ppto->update();*/

    makePSOSwarm();

    if(bShow)
    {
        QColor clr = Qt::cyan;

        for (int isw=0; isw<m_pPSOTask->swarmSize(); isw++)
        {
            Foil *pFoil = new Foil;
            pFoil->setName(QString::asprintf("Particle_%d", isw));
            pFoil->setColor(clr);
            clr = clr.darker(113);

            Particle const &particle = m_pPSOTask->particle(isw);
            m_pPSOTask->makeFoil(particle, pFoil);

            m_pFoilWt->addFoil(pFoil);
            m_TempFoils.append(pFoil);
        }

    }

    m_bIsSwarmValid = true;

    update();

    QApplication::restoreOverrideCursor();
}


void Optim2d::makePSOSwarm()
{
    //initialize task
    if(m_pPSOTask) delete m_pPSOTask;
    m_pPSOTask = nullptr; // to avoid GUI conflicts

    MOPSOTask2d *pPSOTask2d = new MOPSOTask2d;

    pPSOTask2d->setParent(this);

    if(s_bTEFlap)
    {
        // adjust only the flap location, angle is a variable
        m_pFoil->setTEFlapData(s_bTEFlap, s_XHinge*100.0, s_YHinge*100.0, m_pFoil->TEFlapAngle());
    }
    // else leave unchanged

    pPSOTask2d->setFoil(m_pFoil, m_iLE);
    pPSOTask2d->setPolar(m_pPolar);
    pPSOTask2d->setAlpha(s_Alpha);
    pPSOTask2d->setHHParams(s_HHt1, s_HHt2, s_HHmax);

    updateVariables(pPSOTask2d);

    setPSOObjectives(pPSOTask2d);

    //make the swarm
    pPSOTask2d->makeSwarm();
    pPSOTask2d->clearPareto();   // current Pareto may be obsolete if target values have changed since swarm creation
    pPSOTask2d->makeParetoFront();

    m_pPSOTask = pPSOTask2d;
    m_pPSOTask->setAnalysisStatus(xfl::FINISHED);
    //using SIGNAL/SLOT communication instead of Events to ensure messages arrive in order
    connect(m_pPSOTask, SIGNAL(iterEvent(OptimEvent*)), this, SLOT(onIterEvent(OptimEvent*)), Qt::BlockingQueuedConnection);
}


void Optim2d::setPSOObjectives(MOPSOTask2d *pSOTask)
{
    /** @todo copy as array and let the task sort the active objectives */
    int nObj=0;
    for(int i=0; i<m_Objective.size();  i++)
    {
        if(m_Objective.at(i).m_bActive)
        {
            nObj++;
        }
    }
    pSOTask->setNObjectives(nObj);

     nObj=0;
     for(int i=0; i<m_Objective.size();  i++)
    {
        if(m_Objective.at(i).m_bActive)
        {
            pSOTask->setObjective(nObj, m_Objective.at(i));
            nObj++;
        }
    }
}


void Optim2d::updateVariables(MOPSOTask2d *pPSOTask2d)
{
    int dimension = s_HHn;
    if(s_bTEFlap) dimension++;
    pPSOTask2d->setDimension(dimension);  // two sides
    for(int i=0; i<s_HHn; i++)
        pPSOTask2d->setVariable(i, {QString::asprintf("HH%d", i), -s_HHmax, s_HHmax});
    if(s_bTEFlap)     pPSOTask2d->setVariable(s_HHn, {"FlapAngle", s_FlapAngleMin, s_FlapAngleMax});
}


void Optim2d::updateTaskParameters()
{
    updatePolar();

    if(m_pPSOTask)
    {
        m_pPSOTask->setAlpha(s_Alpha);
        m_pPSOTask->setHHParams(s_HHt1, s_HHt2, s_HHmax);

        updateVariables(m_pPSOTask);

        for(int i=0; i<s_HHn; i++)
            m_pPSOTask->setVariable(i, {QString::asprintf("HH%d", i), -s_HHmax, s_HHmax});

        setPSOObjectives(m_pPSOTask);

        m_pPSOTask->updateFitnesses();
        m_pPSOTask->updateErrors();
        m_pPSOTask->clearPareto();   // current Pareto may be obsolete if target values have changed since swarm creation
        m_pPSOTask->makeParetoFront();
    }
}


void Optim2d::outputText(QString const &msg)
{
    m_ppto->onAppendThisPlainText(msg);
}


void Optim2d::cancelTask()
{
//    if(m_Timer.isActive())

    if(m_pPSOTask && m_pPSOTask->isRunning())
    {
        outputText("\nInterrupting optimization task\n\n");
        m_ppto->update();
        m_pPSOTask->cancelAnalyis();
        m_ppbSwarm->setText("Swarm");
        QApplication::restoreOverrideCursor(); // you never know
    }
}


void Optim2d::onRunOptimizer()
{
    if(m_pPSOTask && m_pPSOTask->isRunning())
    {
        cancelTask();
        return;
    }

    readData();


    int nObj = 0;
    for(int i=0; i<m_Objective.size(); i++)
        if(m_Objective.at(i).m_bActive) nObj++;

    if(nObj==0)
    {
        outputText("\nAt least one objective needs be active - aborting\n\n");
        enableControls(true);
        return;
    }

    enableControls(false);

    m_bSaved = false;
    QApplication::setOverrideCursor(Qt::BusyCursor);

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);

    for(int ig=0; ig<NOBJECTIVES; ig++)
    {
        Curve *pCurve = m_ObjGraph[ig].curve(0);
        if(pCurve) pCurve->reset();
    }

    for(int ig=0; ig<m_Objective.size(); ig++)
    {
        OptObjective const &obj = m_Objective.at(ig);
        if(obj.m_Type==xfl::EQUALIZE && obj.m_MaxError<PRECISION)
        {
            QString strange = "Warning: " + obj.m_Name+QString::asprintf(" error should be >0 for EQUALIZE objective\n");
            outputText(strange);
        }
    }


    bool bIsSwarmValid = m_bIsSwarmValid;
    if(!m_pPSOTask) bIsSwarmValid = false;
    else
    {
        if(MOPSOTask::s_PopSize != m_pPSOTask->m_Swarm.size())
        {
            outputText("Swarm size has changed: invalidating current swarm");
            bIsSwarmValid = false;
        }

        if(nObj!=m_pPSOTask->m_Objective.size())
        {
            outputText("Objectives have changed: invalidating current swarm");
            bIsSwarmValid = false;
        }
        int dimension = s_HHn;
        if(s_bTEFlap) dimension++;
        if(dimension!=m_pPSOTask->m_Variable.size())
        {
            outputText("Variables have changed: invalidating current swarm");
            bIsSwarmValid = false;
        }
    }

    if(!bIsSwarmValid)
    {
        onMakeSwarm(false);
    }
    else updateTaskParameters();

    QString strange("\n\nOptimizing for:\n");
    for(int iobj=0; iobj<m_pPSOTask->nObjectives(); iobj++)
    {
        OptObjective const &obj = m_pPSOTask->objective(iobj);
        strange += "    " + obj.m_Name;
        if     (obj.m_Type==xfl::EQUALIZE)   strange += QString::asprintf("=%.3g ", obj.m_Target);
        else if(obj.m_Type==xfl::MAXIMIZE)   strange += QString::asprintf(">=%.3g ", obj.m_Target);
        else if(obj.m_Type==xfl::MINIMIZE)   strange += QString::asprintf("<=%.3g ", obj.m_Target);
    }
    strange += "\n";
    outputText(strange+"\n");

    m_pPSOTask->setAnalysisStatus(xfl::RUNNING);
    m_pPSOTask->restartIterations();
    m_pPSOTask->clearPareto();  // current Pareto may be obsolete if target values have changed since swarm creation
    m_pPSOTask->makeParetoFront();

    QThread *pThread = new QThread;
    m_pPSOTask->setAnalysisStatus(xfl::RUNNING);
    m_pPSOTask->moveToThread(pThread); // don't touch it until the PSO end task event is received

    outputText("Launching optimization task asynchronously\n");
    connect(pThread, SIGNAL(started()),  m_pPSOTask, SLOT(onIterate()));
    connect(pThread, SIGNAL(finished()), pThread,    SLOT(deleteLater())); // deletes the thread but not the object
//    connect(pThread, SIGNAL(finished()), this,       SLOT(onThreadFinished()), Qt::DirectConnection); // deletes the thread but not the object


    pThread->start();
//    pThread->setPriority(QThread::NormalPriority);

    m_ppbSwarm->setText("Interrupt task");
    update();
}


void Optim2d::setFoil(Foil *pFoil)
{
    m_pFoil = pFoil;
    m_pFoil->setVisible(true);
    m_pBestFoil->copyFoil(m_pFoil, false);
    m_pBestFoil->setColor(QColor(151, 107, 73));
    m_pBestFoil->setName("Best foil");

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
}


void  Optim2d::onRunXFoil()
{
    readData();
    updatePolar();
    outputText("\nRunning XFoil:\n");

    double Cl(0),  Cd(0),  ClCd(0),  Cpmin(0),  Cm(0),  Cm0(0);
    double Clb(0), Cdb(0), ClCdb(0), Cpminb(0), Cmb(0), Cm0b(0);
    bool bConverged(false);
    runXFoil(m_pFoil,     Cl, Cd, ClCd, Cpmin, Cm, Cm0, bConverged); // required to determine the leading edge index for the specified AOA
    if(!bConverged) outputText("Seed foil is unconverged\n");

    runXFoil(m_pBestFoil, Clb, Cdb, ClCdb, Cpminb, Cmb, Cm0b, bConverged); // required to determine the leading edge index for the specified AOA
    if(!bConverged) outputText("Best foil is unconverged\n");

    QString strange;
    strange = QString::asprintf("               Seed foil     Current best\n"
                                "      Cl     = %7.3g       %7.3g\n"
                                "      Cd     = %7.3g       %7.3g\n"
                                "      Cl/Cd  = %7.3g       %7.3g\n"
                                "      Cp_min = %7.3g       %7.3g\n"
                                "      Cm     = %7.3g       %7.3g\n"
                                "      Cm0    = %7.3g       %7.3g\n\n",
                                Cl, Clb, Cd, Cdb, ClCd, ClCdb, Cpmin, Cpminb, Cm, Cmb, Cm0, Cm0b);

    outputText(strange);
}


void Optim2d::runXFoil(Foil const*pFoil,
                       double &Cl, double &Cd, double &ClCd, double &Cpmin, double &Cm, double &Cm0,
                       bool &bConverged)
{
    Foil tempfoil(*pFoil);
    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask; // watch the stack
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, s_Alpha, s_Alpha, 0.0);
    task->initializeXFoilTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
    task->run();

    m_iLE = -1;
    for(int i=0; i<xfoil.n-1; i++)
    {
        if(xfoil.x[i+1]<xfoil.x[i+2])
        {
            m_iLE = i;
            break;
        }
    }

    Cl    = xfoil.cl;
    Cd    = xfoil.cd;
    ClCd  = xfoil.cl/xfoil.cd;
    Cpmin = xfoil.cpmn;
    Cm    = xfoil.cm;
    bConverged = xfoil.lvconv;

    task->setSequence(false, 0, 0, 0);
    task->initializeXFoilTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
    task->run();
    Cm0 = xfoil.cm;
    bConverged = bConverged && xfoil.lvconv;

    update();
    delete task;
}


void Optim2d::onIterEvent(OptimEvent *result)
{
    Particle const &particle = result->particle();

    if(m_pPSOTask)
    {
        m_pPSOTask->makeFoil(particle, m_pBestFoil);

        bool bConverged(false);
        double Cl(0), Cd(0), ClCd(0), Cpmin(0), Cm(0), Cm0(0);
        runXFoil(m_pBestFoil, Cl, Cd, ClCd, Cpmin, Cm, Cm0, bConverged);

        // update graph
        Curve *pCurve(nullptr);
        pCurve = m_ObjGraph[0].curve(0);
        if(pCurve)  pCurve->appendPoint(result->iter(), Cl);
        pCurve = m_ObjGraph[1].curve(0);
        if(pCurve)  pCurve->appendPoint(result->iter(), Cd);
        pCurve = m_ObjGraph[2].curve(0);
        if(pCurve)  pCurve->appendPoint(result->iter(), ClCd);
        pCurve = m_ObjGraph[3].curve(0);
        if(pCurve)  pCurve->appendPoint(result->iter(), Cpmin);
        pCurve = m_ObjGraph[4].curve(0);
        if(pCurve)  pCurve->appendPoint(result->iter(), Cm);
        pCurve = m_ObjGraph[5].curve(0);
        if(pCurve)  pCurve->appendPoint(result->iter(), Cm0);

        for(int ig=0; ig<NOBJECTIVES; ig++)
        {
            m_ObjGraph[ig].resetLimits();
            m_pObjGraphWt[ig]->update();
        }

        QString strange = QString::asprintf("It.%2d:", result->iter()) ;
        if(s_bTEFlap) strange += QString::asprintf("  Flap angle=%5.2g", particle.position().last()) + QChar(0260)+"  ";
        for(int iobj=0; iobj<m_pPSOTask->nObjectives(); iobj++)
        {
            OptObjective const &obj = m_pPSOTask->objective(iobj);
            strange += " " + obj.m_Name;
            if      (obj.m_Name=="Cl")    strange += QString::asprintf("=%7g ", Cl);
            else if (obj.m_Name=="Cd")    strange += QString::asprintf("=%7g ", Cd);
            else if (obj.m_Name=="ClCd")  strange += QString::asprintf("=%7g ", ClCd);
            else if (obj.m_Name=="Cpmin") strange += QString::asprintf("=%7g ", Cpmin);
            else if (obj.m_Name=="Cm")    strange += QString::asprintf("=%7g ", Cm);
            else if (obj.m_Name=="Cm0")   strange += QString::asprintf("=%7g ", Cm0);
        }
        outputText(strange+"\n");
    }

//    delete result;


    update();
}


void Optim2d::customEvent(QEvent *pEvent)
{
    if(pEvent->type()==OPTIM_END_EVENT)
    {
        OptimEvent *pOptEvent = dynamic_cast<OptimEvent*>(pEvent);
        Particle const &particle = pOptEvent->particle();

        // display best foil - method may not be the same for each case
        if (m_pPSOTask)
        {
            m_pPSOTask->makeFoil(particle, m_pBestFoil);
            if(m_pPSOTask->m_Iter>=MOPSOTask::s_MaxIter) outputText("The maximum number of iterations has been reached\n");
        }

        if(particle.dimension()%2==1)  m_FlapAngle = particle.position().last();
        else                           m_FlapAngle = 0.0;
        enableControls(true);
        m_ppbSwarm->setText("Swarm");
        QApplication::restoreOverrideCursor();
    }
    else if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        outputText(pMsgEvent->msg());
    }
    else
        QDialog::customEvent(pEvent);
}


void Optim2d::fillObjectives()
{
    m_pObjModel->setRowCount(m_Objective.size());

    QModelIndex ind;
    m_pObjModel->blockSignals(true); // do notemit the dataChanged signal

    for(int row=0; row<m_Objective.size(); row++)
    {
        OptObjective const &obj = m_Objective.at(row);
        ind = m_pObjModel->index(row, 0, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("Activate or deactivate the objective");
        m_pObjModel->setData(ind, obj.m_bActive, Qt::UserRole);

        ind = m_pObjModel->index(row, 1, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The name of the objective");
        m_pObjModel->setData(ind, obj.m_Name);

        ind = m_pObjModel->index(row, 2, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("-1 to MINIMIZE the objective\n"
                                                    " 0 to EQUALIZE\n"
                                                    " 1 to MAXIMIZE");
        switch(obj.m_Type)
        {
            case xfl::MINIMIZE:  m_pObjModel->setData(ind, -1); break;
            case xfl::MAXIMIZE:  m_pObjModel->setData(ind, 1); break;
            case xfl::EQUALIZE:  m_pObjModel->setData(ind, 0); break;
        }

        ind = m_pObjModel->index(row, 3, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The target value");
        m_pObjModel->setData(ind, obj.m_Target);

        ind = m_pObjModel->index(row, 4, QModelIndex());
        m_pObjModel->itemFromIndex(ind)->setToolTip("The tolerance on the target value.\n"
                                                    "Only of use if the objective is to EQUALIZE");
        m_pObjModel->setData(ind, obj.m_MaxError);
    }
    m_pObjModel->blockSignals(false);
}


void Optim2d::readObjectives()
{
    bool bOk(false);

    QString strange;

    QModelIndex ind;

    for(int row=0; row<m_pObjModel->rowCount(); row++)
    {
        ind = m_pObjModel->index(row, 0);
        m_Objective[row].m_bActive = m_pObjModel->data(ind, Qt::UserRole).toBool();

        ind = m_pObjModel->index(row, 1);
        m_Objective[row].m_Name = m_pObjModel->data(ind).toString();

        ind = m_pObjModel->index(row, 2);
        int val = m_pObjModel->data(ind).toInt();
        if     (val<0) m_Objective[row].m_Type = xfl::MINIMIZE;
        else if(val>0) m_Objective[row].m_Type = xfl::MAXIMIZE;
        else           m_Objective[row].m_Type = xfl::EQUALIZE;

        ind = m_pObjModel->index(row, 3);
        strange = m_pObjModel->data(ind).toString();
        strange.replace(" ","");
        m_Objective[row].m_Target = strange.toDouble(&bOk);

        ind = m_pObjModel->index(row, 4);
        strange = m_pObjModel->data(ind).toString();
        strange.replace(" ","");
        m_Objective[row].m_MaxError = strange.toDouble(&bOk);
    }

    s_bCl          = m_Objective.at(0).m_bActive;
    s_ClObjType    = m_Objective.at(0).m_Type-1;
    s_Cl           = m_Objective.at(0).m_Target;
    s_ClMaxError   = m_Objective.at(0).m_MaxError;

    s_bCd          = m_Objective.at(1).m_bActive;
    s_CdObjType    = m_Objective.at(1).m_Type-1;
    s_Cd           = m_Objective.at(1).m_Target;
    s_CdMaxError   = m_Objective.at(1).m_MaxError;

    s_bClCd        = m_Objective.at(2).m_bActive;
    s_ClCdObjType  = m_Objective.at(2).m_Type-1;
    s_ClCd         = m_Objective.at(2).m_Target;
    s_ClCdMaxError = m_Objective.at(2).m_MaxError;

    s_bCpmin       = m_Objective.at(3).m_bActive;
    s_CpMinObjType = m_Objective.at(3).m_Type-1;
    s_Cpmin        = m_Objective.at(3).m_Target;
    s_CpMaxError   = m_Objective.at(3).m_MaxError;

    s_bCm          = m_Objective.at(4).m_bActive;
    s_CmObjType    = m_Objective.at(4).m_Type-1;
    s_Cm           = m_Objective.at(4).m_Target;
    s_CmMaxError   = m_Objective.at(4).m_MaxError;

    s_bCm0         = m_Objective.at(5).m_bActive;
    s_Cm0ObjType   = m_Objective.at(5).m_Type-1;
    s_Cm0          = m_Objective.at(5).m_Target;
    s_Cm0MaxError  = m_Objective.at(5).m_MaxError;
}


void Optim2d::updatePolar()
{
    // update the polar with the latest user data
    m_pPolar->setPolarType(xfl::FIXEDSPEEDPOLAR);
    m_pPolar->setReynolds(s_Re);
    m_pPolar->setMach(s_Mach);
    m_pPolar->setNCrit(s_NCrit);
    m_pPolar->setXtrTop(s_XtrTop);
    m_pPolar->setXtrBot(s_XtrBot);
}


void Optim2d::onPlotHH()
{
    readData();

    m_HHGraph.deleteCurves();

    int npts = 300;
    double hh = 0.0, x=0.0;
    for(int j=0; j<s_HHn/2; j++)
    {
        Curve *pCurve = m_HHGraph.addCurve(QString::asprintf("f%d", j+1));
        pCurve->setWidth(2);
        pCurve->resizePoints(npts);
        double t1 = s_HHt1*double(j+1)/double(s_HHn+1); // HH undefined for t1=0
        for(int k=0; k<npts; k++)
        {
            x = double(k)/double(npts-1);
            hh = HH(x, t1, s_HHt2)*s_HHmax*100.0;
            pCurve->setPoint(k, x, hh);
        }
    }

    m_HHGraph.resetLimits();
    m_pHHGraphWt->update();
}


