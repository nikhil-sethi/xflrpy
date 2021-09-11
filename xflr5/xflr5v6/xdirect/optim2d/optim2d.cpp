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
#include <QDebug>

#include "optim2d.h"

#include <xflcore/xflcore.h>
#include <xflgraph/curve.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflgraph/containers/graphwt.h>
#include <twodwidgets/foilwt.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/optim2d/gatask.h>
#include <xdirect/optim2d/mopsotask2d.h>
//#include <xdirect/optim2d/optimevent.h>
#include <xflcore/constants.h>
#include <xflobjects/objects2d/polar.h>
#include <xfoil.h>

QByteArray Optim2d::s_HSplitterSizes;
QByteArray Optim2d::s_VSplitterSizes;
QByteArray Optim2d::s_Geometry;

double Optim2d::s_Alpha           = 0.0;
double Optim2d::s_Re              = 1.0e6;
double Optim2d::s_NCrit           = 9.0;
double Optim2d::s_XtrTop          = 1.0;
double Optim2d::s_XtrBot          = 1.0;
double Optim2d::s_Cl              = 0.0;
double Optim2d::s_ClMaxError      = 1.e-3;
double Optim2d::s_Cd              = 0.02;
double Optim2d::s_CdMaxError      = 1.e-4;

bool   Optim2d::s_bPSO            = true;
int    Optim2d::s_Dt              = 16;

int    Optim2d::s_HHn             = 6;   //GA seems to converge a little better if even number so that there is no bump function centered on the LE
double Optim2d::s_HHt2            = 1.0;
double Optim2d::s_HHmax           = 0.025;

Optim2d::Optim2d(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("2d Optimization");

    m_pFoil=nullptr;
    m_pBestFoil = new Foil;

    m_pPolar = new Polar;

    m_iLE = -1;

    m_bModified = false;
    m_bSaved = true;

    //PSO
    m_pPSOTask = nullptr;

    //GA
    m_pGATask = nullptr;

    setupLayout();
    connectSignals();
}


Optim2d::~Optim2d()
{
    delete m_pBestFoil;
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];

    delete m_pPolar;

    if(m_pPSOTask) delete m_pPSOTask;
    if(m_pGATask)  delete m_pGATask;
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
                    QTabWidget *ptwMain = new QTabWidget;
                    {
                        QFrame *pTargetPage = new QFrame;
                        {
                            QGridLayout *pOptimLayout = new QGridLayout;
                            {
                                QLabel *plabTarget = new QLabel("Target ");
                                QLabel *plabMaxError = new QLabel("Max. error");

                                QLabel *plabCl = new QLabel("Cl:");
                                m_pdeCl = new DoubleEdit(s_Cl);
                                m_pdeClMaxError = new DoubleEdit(s_ClMaxError);

                                QLabel *plabCd = new QLabel("Cd:");
                                m_pdeCd = new DoubleEdit(s_Cd);
                                m_pdeCdMaxError = new DoubleEdit(s_CdMaxError);

                                QLabel *plabNote = new QLabel("Note: GA is single objective only");

                                m_pchMultithread = new QCheckBox("Multithreaded");
                                m_pchMultithread->setChecked(OptimTask::s_bMultiThreaded);
                                pOptimLayout->addWidget(plabTarget,         1, 2, Qt::AlignCenter);
                                pOptimLayout->addWidget(plabMaxError,       1, 3, Qt::AlignCenter);
                                pOptimLayout->addWidget(plabCl,             2, 1);
                                pOptimLayout->addWidget(m_pdeCl,            2, 2);
                                pOptimLayout->addWidget(m_pdeClMaxError,    2, 3);
                                pOptimLayout->addWidget(plabCd,             3, 1);
                                pOptimLayout->addWidget(m_pdeCd,            3, 2);
                                pOptimLayout->addWidget(m_pdeCdMaxError,    3, 3);
                                pOptimLayout->addWidget(plabNote,           4, 1, 1, 3);
                                pOptimLayout->setRowStretch(5,1);
                                pOptimLayout->addWidget(m_pchMultithread, 6,1,1,3);
                            }
                            pTargetPage->setLayout(pOptimLayout);
                        }

                        QFrame *pAlgoPage= new QFrame;
                        {
                            QVBoxLayout *pSwarmLayout = new QVBoxLayout;
                            {
                                QFrame *pAlgoFrame = new QFrame;
                                {
                                    QHBoxLayout *pAlgoLayout = new QHBoxLayout;
                                    {
                                        QLabel *pLabAlgo = new QLabel("Algorithm:");
                                        m_prbPSO = new QRadioButton("PSO");
                                        m_prbPSO->setToolTip("Particle Swarm Optimization");
                                        m_prbGA  = new QRadioButton("GA");
                                        m_prbGA->setToolTip("Genetic Algorithm");
                                        m_prbPSO->setChecked(s_bPSO);
                                        m_prbGA->setChecked(!s_bPSO);

                                        pAlgoLayout->addStretch();
                                        pAlgoLayout->addWidget(pLabAlgo);
                                        pAlgoLayout->addWidget(m_prbPSO);
                                        pAlgoLayout->addWidget(m_prbGA);
                                        pAlgoLayout->addStretch();
                                    }
                                    pAlgoFrame->setLayout(pAlgoLayout);
                                }

                                QGroupBox *pCommonBox = new QGroupBox("Common");
                                {
                                    QGridLayout *pCommonLayout = new QGridLayout;
                                    {
                                        QLabel *plabPopSize = new QLabel("Swarm size:");
                                        m_piePopSize = new IntEdit(OptimTask::s_PopSize);

                                        QLabel *plabMaxIter = new QLabel("Max. iterations:");
                                        m_pieMaxIter = new IntEdit(OptimTask::s_MaxIter);

                                        QLabel *pLabUpdate = new QLabel("Update interval:");
                                        m_pieUpdateDt = new IntEdit(s_Dt);
                                        QLabel *pLabMilliSecs = new QLabel("ms");

                                        pCommonLayout->addWidget(plabPopSize,          1, 1);
                                        pCommonLayout->addWidget(m_piePopSize,         1, 2);

                                        pCommonLayout->addWidget(plabMaxIter,          5, 1);
                                        pCommonLayout->addWidget(m_pieMaxIter,         5, 2);

                                        pCommonLayout->addWidget(pLabUpdate,           6, 1);
                                        pCommonLayout->addWidget(m_pieUpdateDt,        6, 2);
                                        pCommonLayout->addWidget(pLabMilliSecs,        6, 3);
                                    }
                                    pCommonBox->setLayout(pCommonLayout);
                                }
                                m_pswAlgo = new QStackedWidget;
                                {
                                    QGroupBox *pPSOBox = new QGroupBox("PSO");
                                    {
                                        QGridLayout *pPSOLayout = new QGridLayout;
                                        {
                                            QLabel *plabInertia = new QLabel("Inertia weight:");
                                            m_pdeInertiaWeight = new DoubleEdit(MOPSOTask::s_InertiaWeight);
                                            m_pdeInertiaWeight->setToolTip("The inertia weight determines the influence of the particle's\n"
                                                                           "current velocity on its updated velocity.");

                                            QLabel *plabCognitive = new QLabel("Cognitive weight:");
                                            m_pdeCognitiveWeight = new DoubleEdit(MOPSOTask::s_CognitiveWeight);
                                            m_pdeCognitiveWeight->setToolTip("The cognitive weights determines the influence of the particle's best position");

                                            QLabel *plabSocial = new QLabel("Social weight:");
                                            m_pdeSocialWeight = new DoubleEdit(MOPSOTask::s_SocialWeight);
                                            m_pdeSocialWeight->setToolTip("The social weight determines the influence of the global best-known position");


                                            pPSOLayout->addWidget(plabInertia,          1, 1);
                                            pPSOLayout->addWidget(m_pdeInertiaWeight,   1, 2);

                                            pPSOLayout->addWidget(plabCognitive,        2, 1);
                                            pPSOLayout->addWidget(m_pdeCognitiveWeight, 2, 2);

                                            pPSOLayout->addWidget(plabSocial,           3, 1);
                                            pPSOLayout->addWidget(m_pdeSocialWeight,    3, 2);

                                            pPSOLayout->setColumnStretch(3,1);
                                        }
                                        pPSOBox->setLayout(pPSOLayout);
                                    }

                                    QGroupBox *pGABox = new QGroupBox("GA");
                                    {
                                        QGridLayout *pGALayout = new QGridLayout;
                                        {
                                            QLabel *pLabXOver = new QLabel("Cross-over probability:");
                                            m_pdeProbXOver = new DoubleEdit(GATask::s_ProbXOver);
    //                                        m_pdeProbXOver->setRange(0.0, 1.0);

                                            QLabel *pLabProbMute = new QLabel("Mutation probability:");
                                            m_pdeProbMutation = new DoubleEdit(GATask::s_ProbMutation);
    //                                        m_pdeProbMutation->setRange(0.0, 1.0);

                                            QLabel *pLabSigMute = new QLabel("Mutation standard deviation:");
                                            m_pdeSigmaMutation = new DoubleEdit(GATask::s_SigmaMutation);
    //                                        m_pdeSigmaMutation->setRange(0.0, 1.0); // absolute value

                                            pGALayout->addWidget(pLabXOver,            1, 1);
                                            pGALayout->addWidget(m_pdeProbXOver,       1, 2);

                                            pGALayout->addWidget(pLabProbMute,         2, 1);
                                            pGALayout->addWidget(m_pdeProbMutation,    2, 2);

                                            pGALayout->addWidget(pLabSigMute,          3, 1);
                                            pGALayout->addWidget(m_pdeSigmaMutation,   3, 2);

                                            pGALayout->setColumnStretch(3,1);
                                        }
                                        pGABox->setLayout(pGALayout);
                                    }

                                    m_pswAlgo->addWidget(pPSOBox);
                                    m_pswAlgo->addWidget(pGABox);
                                    m_pswAlgo->setCurrentIndex(s_bPSO ? 0 : 1);
                                }
                                pSwarmLayout->addWidget(pAlgoFrame);
                                pSwarmLayout->addWidget(pCommonBox);
                                pSwarmLayout->addWidget(m_pswAlgo);

                                pSwarmLayout->addStretch();
                            }
                            pAlgoPage->setLayout(pSwarmLayout);
                        }

                        QFrame *pXFoilPage = new QFrame;
                        {
                            QGridLayout *pXFoilLayout = new QGridLayout;
                            {
                                QLabel *plabAlpha = new QLabel("Alpha:");
                                m_pdeAlpha = new DoubleEdit(s_Alpha);
                                QLabel *plabDeg = new QLabel(QChar(0260));

                                QLabel *plabRe = new QLabel("Reynolds:");
                                m_pdeRe   = new DoubleEdit(s_Re);

                                QLabel *plabNCrit = new QLabel("NCrit:");
                                m_pdeNCrit = new DoubleEdit(s_NCrit);

                                QLabel *plabXtrTop = new QLabel("Top transition:");
                                m_pdeXtrTop = new DoubleEdit(s_XtrTop);

                                QLabel *plabXtrBot = new QLabel("Bot. transition:");
                                m_pdeXtrBot = new DoubleEdit(s_XtrBot);

                                m_ppbAnalyze = new QPushButton("Analyze");

                                pXFoilLayout->addWidget(plabAlpha,    1, 1);
                                pXFoilLayout->addWidget(m_pdeAlpha,   1, 2);
                                pXFoilLayout->addWidget(plabDeg,      1, 3);
                                pXFoilLayout->addWidget(plabRe,       2, 1);
                                pXFoilLayout->addWidget(m_pdeRe,      2, 2);
                                pXFoilLayout->addWidget(plabNCrit,    3, 1);
                                pXFoilLayout->addWidget(m_pdeNCrit,   3, 2);
                                pXFoilLayout->addWidget(plabXtrTop,   4, 1);
                                pXFoilLayout->addWidget(m_pdeXtrTop,  4, 2);
                                pXFoilLayout->addWidget(plabXtrBot,   5, 1);
                                pXFoilLayout->addWidget(m_pdeXtrBot,  5, 2);
                                pXFoilLayout->addWidget(m_ppbAnalyze, 7,1,1,3);

                                pXFoilLayout->setRowStretch(6,1);
                                pXFoilLayout->setColumnStretch(1,1);
                                pXFoilLayout->setColumnStretch(2,1);
                            }
                            pXFoilPage->setLayout(pXFoilLayout);
                        }

                        QFrame *pHHPage = new QFrame;
                        {
                            QGridLayout *pHHLayout = new QGridLayout;
                            {
                                QLabel *plabNHH = new QLabel("Nb. of functions:");
                                m_pieNHH = new IntEdit(s_HHn);
                                QLabel *plabt2 = new QLabel("t2:");
                                m_pdeHHt2 = new DoubleEdit(s_HHt2);
                                QLabel *plabLax = new QLabel("Max. HH amplitude:");
                                m_pdeHHmax = new DoubleEdit(s_HHmax*100);
                                QLabel *plabPercent = new QLabel("% Ch.");

                                pHHLayout->addWidget(plabNHH,     1, 1);
                                pHHLayout->addWidget(m_pieNHH,    1, 2);
                                pHHLayout->addWidget(plabLax,     2, 1);
                                pHHLayout->addWidget(m_pdeHHmax,  2, 2);
                                pHHLayout->addWidget(plabPercent, 2, 3);
                                pHHLayout->addWidget(plabt2,      3, 1);
                                pHHLayout->addWidget(m_pdeHHt2,   3, 2);

                                pHHLayout->setRowStretch(4,1);
                                pHHLayout->setColumnStretch(1, 1);
                                pHHLayout->setColumnStretch(2, 1);
                            }

                            pHHPage->setLayout(pHHLayout);
                        }

                        ptwMain->addTab(pTargetPage, "Target");
                        ptwMain->addTab(pAlgoPage,   "Algorithms");
                        ptwMain->addTab(pXFoilPage,  "XFoil");
                        ptwMain->addTab(pHHPage,     "Hicks-Henne");
                    }

                    m_ppbMakeSwarm = new QPushButton("Make random population");
                    m_ppbSwarm = new QPushButton("Start optimization");

                    m_ppt = new QPlainTextEdit;
                    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                    m_ppt->setFont(fixedfnt);

                    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
                    {
                        m_ppbStoreBestFoil = new QPushButton("Store best foil");
                        m_pButtonBox->addButton(m_ppbStoreBestFoil, QDialogButtonBox::ActionRole);

                        QPushButton *ppbClear = new QPushButton("Clear output");
                        connect(ppbClear, SIGNAL(clicked()), m_ppt, SLOT(clear()));
                        m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);

                        connect(m_ppbStoreBestFoil, SIGNAL(clicked()), SLOT(onStoreBestFoil()));
                        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                    }

                    pLeftLayout->addWidget(ptwMain);
                    pLeftLayout->addWidget(m_ppbMakeSwarm);
                    pLeftLayout->addWidget(m_ppbSwarm);
                    pLeftLayout->addWidget(m_ppt);
                    pLeftLayout->addWidget(m_pButtonBox);
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
                        m_pErrorGraphWt = new GraphWt;
                        m_pErrorGraphWt->setGraph(&m_ErrorGraph);
                        m_ErrorGraph.setMargin(71);
                        m_ErrorGraph.setXTitle("Iter");
                        m_ErrorGraph.setYTitle("Error");
                        m_pErrorGraphWt->showLegend(true);

                        m_pParetoGraphWt = new GraphWt;
                        m_pParetoGraphWt->setGraph(&m_ParetoGraph);
                        m_ParetoGraph.setMargin(71);
                        m_ParetoGraph.setXTitle("Iter");
                        m_ParetoGraph.setYTitle("Pareto");
                        m_ParetoGraph.setAuto(false);
                        m_pParetoGraphWt->showLegend(true);

                        pTopLayout->addWidget(m_pErrorGraphWt);
                        pTopLayout->addWidget(m_pParetoGraphWt);
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
    connect(m_prbPSO,       SIGNAL(clicked()), SLOT(onAlgorithm()));
    connect(m_prbGA,        SIGNAL(clicked()), SLOT(onAlgorithm()));
    connect(m_ppbMakeSwarm, SIGNAL(clicked()), SLOT(onMakeSwarm()));
    connect(m_ppbSwarm,     SIGNAL(clicked()), SLOT(onOptimize()));
    connect(m_ppbAnalyze,   SIGNAL(clicked()), SLOT(onAnalyze()));
}


void Optim2d::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) onClose();
}


void Optim2d::onAlgorithm()
{
    s_bPSO = m_prbPSO->isChecked();
    m_pdeCd->setEnabled(s_bPSO);
    m_pdeCdMaxError->setEnabled(s_bPSO);

    m_pswAlgo->setCurrentIndex(s_bPSO ? 0 : 1);
    update();
}


void Optim2d::reject()
{
    onClose();
}


void Optim2d::onClose()
{
    QApplication::restoreOverrideCursor();// you never know
    m_Timer.stop();

    if(!m_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Question"), tr("Exit without saving?"),  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(resp != QMessageBox::Yes) return;
    }
    accept();
}


void Optim2d::outputText(QString const &msg) const
{
    m_ppt->moveCursor(QTextCursor::End);
    m_ppt->insertPlainText(msg);
    m_ppt->moveCursor(QTextCursor::End);
    m_ppt->ensureCursorVisible();
}


void Optim2d::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);

    onAlgorithm();

    m_pErrorGraphWt->update();
}


void Optim2d::hideEvent(QHideEvent *)
{
    readData();

    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();
}


void Optim2d::loadSettings(QSettings &settings)
{
    settings.beginGroup("Optim2d");
    {
        s_Alpha           = settings.value("Alpha",           s_Alpha).toDouble();
        s_Re              = settings.value("Re",              s_Re).toDouble();
        s_NCrit           = settings.value("NCrit",           s_NCrit).toDouble();
        s_XtrTop          = settings.value("XtrTop",          s_XtrTop).toDouble();
        s_XtrBot          = settings.value("XtrBot",          s_XtrBot).toDouble();
        s_Cl              = settings.value("Cl",              s_Cl).toDouble();
        s_ClMaxError      = settings.value("ClMaxError",      s_ClMaxError).toDouble();
        s_Cd              = settings.value("Cd",              s_Cd).toDouble();
        s_CdMaxError      = settings.value("CdMaxError",      s_CdMaxError).toDouble();

        s_bPSO            = settings.value("bPSO",            s_bPSO).toBool();
        s_Dt              = settings.value("Dt",              s_Dt).toInt();
        s_HHn             = settings.value("HHn",             s_HHn).toInt();
        s_HHt2            = settings.value("HHt2",            s_HHt2).toDouble();
        s_HHmax           = settings.value("HHmax",           s_HHmax).toDouble();

        OptimTask::s_bMultiThreaded  = settings.value("bMultithread",    OptimTask::s_bMultiThreaded).toBool();
        OptimTask::s_PopSize         = settings.value("PopSize",         OptimTask::s_PopSize).toInt();
        OptimTask::s_MaxIter         = settings.value("MaxIter",         OptimTask::s_MaxIter).toInt();

        MOPSOTask::s_InertiaWeight   = settings.value("InertiaWeight",   MOPSOTask::s_InertiaWeight).toDouble();
        MOPSOTask::s_CognitiveWeight = settings.value("CognitiveWeight", MOPSOTask::s_CognitiveWeight).toDouble();
        MOPSOTask::s_SocialWeight    = settings.value("SocialWeight",    MOPSOTask::s_SocialWeight).toDouble();

        GATask::s_ProbXOver          = settings.value("CrossOver",       GATask::s_ProbXOver).toDouble();
        GATask::s_ProbMutation       = settings.value("ProbMutation",    GATask::s_ProbMutation).toDouble();
        GATask::s_SigmaMutation      = settings.value("SigMutation",     GATask::s_SigmaMutation).toDouble();

        s_Geometry        = settings.value("WindowGeom",     QByteArray()).toByteArray();
        s_HSplitterSizes  = settings.value("HSplitterSizes", QByteArray()).toByteArray();
        s_VSplitterSizes  = settings.value("VSplitterSizes", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void Optim2d::saveSettings(QSettings &settings)
{
    settings.beginGroup("Optim2d");
    {
        settings.setValue("Alpha",             s_Alpha);
        settings.setValue("Re",                s_Re);
        settings.setValue("NCrit",             s_NCrit);
        settings.setValue("XtrTop",            s_XtrTop);
        settings.setValue("XtrBot",            s_XtrBot);
        settings.setValue("Cl",                s_Cl);
        settings.setValue("ClMaxError",        s_ClMaxError);
        settings.setValue("Cd",                s_Cd);
        settings.setValue("CdMaxError",        s_CdMaxError);

        settings.setValue("bPSO",              s_bPSO);
        settings.setValue("Dt",                s_Dt);
        settings.setValue("HHn",               s_HHn);
        settings.setValue("HHt2",              s_HHt2);
        settings.setValue("HHmax",             s_HHmax);

        settings.setValue("PopSize",           OptimTask::s_PopSize);
        settings.setValue("bMultithread",      OptimTask::s_bMultiThreaded);
        settings.setValue("MaxIter",           OptimTask::s_MaxIter);

        settings.setValue("InertiaWeight",     MOPSOTask::s_InertiaWeight);
        settings.setValue("CognitiveWeight",   MOPSOTask::s_CognitiveWeight);
        settings.setValue("SocialWeight",      MOPSOTask::s_SocialWeight);

        settings.setValue("CrossOver",         GATask::s_ProbXOver);
        settings.setValue("ProbMutation",      GATask::s_ProbMutation);
        settings.setValue("SigMutation",       GATask::s_SigmaMutation);

        settings.setValue("WindowGeom",     s_Geometry);
        settings.setValue("HSplitterSizes", s_HSplitterSizes);
        settings.setValue("VSplitterSizes", s_VSplitterSizes);
    }
    settings.endGroup();
}


void Optim2d::onStoreBestFoil()
{
    Foil *pNewFoil = new Foil(*m_pBestFoil);
    pNewFoil->initFoil();
    QString basename = "Optimized";

    int iter = 1;
    QString name = basename + QString::asprintf("_%d", iter);
    while(Objects2d::foilExists(name))
    {
        name = basename + QString::asprintf("_%d", iter);
        iter++;
    }
    pNewFoil->setName(name);
    pNewFoil->setColor(xfl::randomColor(DisplayOptions::isLightTheme()));
    Objects2d::insertThisFoil(pNewFoil); // overwrites existing
    outputText("Saved the foil with name: "+name+"\n");
    m_bSaved = true;

    m_bModified = true;
}


void Optim2d::readData()
{
    s_bPSO            = m_prbPSO->isChecked();
    s_Alpha           = m_pdeAlpha->value();
    s_Re              = m_pdeRe->value();
    s_NCrit           = m_pdeNCrit->value();
    s_XtrTop          = m_pdeXtrTop->value();
    s_XtrBot          = m_pdeXtrBot->value();
    s_Cl              = m_pdeCl->value();
    s_ClMaxError      = m_pdeClMaxError->value();
    s_Cd              = m_pdeCd->value();
    s_CdMaxError      = m_pdeCdMaxError->value();
    s_HHn             = m_pieNHH->value();
    s_HHt2            = m_pdeHHt2->value();
    s_HHmax           = m_pdeHHmax->value()/100.0;
    s_Dt              = m_pieUpdateDt->value();

    // temporary duplication
    OptimTask::s_MaxIter         = m_pieMaxIter->value();
    OptimTask::s_PopSize         = m_piePopSize->value();
    OptimTask::s_bMultiThreaded  = m_pchMultithread->isChecked();

    MOPSOTask::s_InertiaWeight   = m_pdeInertiaWeight->value();
    MOPSOTask::s_CognitiveWeight = m_pdeCognitiveWeight->value();
    MOPSOTask::s_SocialWeight    = m_pdeSocialWeight->value();

    GATask::s_ProbMutation    = m_pdeProbMutation->value();
    GATask::s_SigmaMutation   = m_pdeSigmaMutation->value();
    GATask::s_ProbXOver       = m_pdeProbXOver->value();
}


void Optim2d::onMakeSwarm(bool bShow)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Timer.stop();

    m_ErrorGraph.deleteCurves();

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];
    m_TempFoils.clear();
    update();

    readData();

    outputText("\nCreating particles...\n");

    // update the polar with the latest user data
    m_pPolar->setReType(1);
    m_pPolar->setMaType(1);
    m_pPolar->setReynolds(s_Re);
    m_pPolar->setNCrit(s_NCrit);
    m_pPolar->setXtrTop(s_XtrTop);
    m_pPolar->setXtrBot(s_XtrBot);

    onAnalyze(); // required to determine the leading edge index for the specified AOA

    if(s_bPSO) makePSOSwarm();
    else       makeGAGen();

    // debug helper
    if(bShow)
    {
        QColor clr = xfl::randomColor(DisplayOptions::isLightTheme());
        if(s_bPSO)
        {
            for (int isw=0; isw<m_pPSOTask->swarmSize(); isw++)
            {
                Foil *pFoil = new Foil;
                pFoil->setName(QString::asprintf("Particle_%d", isw));
                pFoil->setColor(clr);
                clr = clr.darker(117);

                Particle const &particle = m_pPSOTask->particle(isw);
                m_pPSOTask->makeFoil(particle, pFoil);

                m_pFoilWt->addFoil(pFoil);
                m_TempFoils.append(pFoil);
            }
        }
        else
        {
            for (int isw=0; isw<m_pGATask->swarmSize(); isw++)
            {
                Foil *pFoil = new Foil;
                pFoil->setName(QString::asprintf("Particle_%d", isw));
                pFoil->setColor(clr);
                clr = clr.darker(117);

                Particle const &particle = m_pGATask->particle(isw);
                m_pGATask->makeFoil(&particle, pFoil);
                m_pFoilWt->addFoil(pFoil);
                m_TempFoils.append(pFoil);
            }
        }
    }

    update();

    QApplication::restoreOverrideCursor();
}


void Optim2d::makePSOSwarm()
{
    //initialize task
    if(m_pPSOTask) delete m_pPSOTask;
    m_pPSOTask = nullptr; // to avoid GUI conflicts

    MOPSOTask2d *pPSOTask2d = new MOPSOTask2d;

    pPSOTask2d->setPolar(m_pPolar);
    pPSOTask2d->setParent(this);
    pPSOTask2d->setFoil(m_pFoil, m_iLE);
    pPSOTask2d->setAlpha(s_Alpha);
    pPSOTask2d->setHHParams(s_HHn, s_HHt2, s_HHmax);
    pPSOTask2d->setDimension(s_HHn); //
    for(int i=0; i<s_HHn; i++)
        pPSOTask2d->setVariable(i, {QString::asprintf("HH%d", i), -s_HHmax, s_HHmax});
    pPSOTask2d->setNObjectives(2);  // Cl, Cd
    pPSOTask2d->setObjective(0, {"Cl", true, s_Cl, s_ClMaxError});
    pPSOTask2d->setObjective(1, {"Cd", true, s_Cd, s_CdMaxError});

    //make the swarm
    pPSOTask2d->makeSwarm();
    pPSOTask2d->clearPareto();   // current Pareto may be obsolete if target values have changed since swarm creationli
    pPSOTask2d->makeParetoFront();
    pPSOTask2d->setAnalysisStatus(xfl::PENDING);// not started yet

    m_pPSOTask = pPSOTask2d;
    updateParetoGraph();
    m_ParetoGraph.resetLimits();
    m_pParetoGraphWt->update();
}


void Optim2d::makeGAGen()
{
    //initialize task
    if(m_pGATask) delete m_pGATask;
    m_pGATask = new GATask;
    m_pGATask->setPolar(m_pPolar);
    m_pGATask->setParent(this);
    m_pGATask->setFoil(m_pFoil, m_iLE);
    m_pGATask->setAlpha(s_Alpha);
    m_pGATask->setHHParams(s_HHn, s_HHt2, s_HHmax);
    m_pGATask->setDimension(s_HHn); //
    for(int i=0; i<s_HHn; i++)
        m_pGATask->setVariable(i, {QString::asprintf("HH%d", i), -s_HHmax, s_HHmax});
    m_pGATask->setObjective({"Cl", true, s_Cl, s_ClMaxError});

    //make the swarm
    m_pGATask->makeSwarm();
    m_pGATask->setAnalysisStatus(xfl::PENDING);// not started yet
}


void Optim2d::onOptimize()
{
    readData();

    if(m_Timer.isActive())
    {
        m_Timer.stop();
        m_ppbSwarm->setText("Swarm");
        outputText("\nOptimization interrupted\n\n");
        QApplication::restoreOverrideCursor();
        return;
    }
    else
        m_ppbSwarm->setText("Stop");

    m_bSaved = false;

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);

    QApplication::setOverrideCursor(Qt::BusyCursor);

    if(s_bPSO)
    {
        if(!m_pPSOTask) onMakeSwarm(false);

        if(m_pPSOTask->isRunning())
        {
            m_pPSOTask->setAnalysisStatus(xfl::CANCELLED);
            // m_ppbSwarm->setText("Swarm"); done when the finish event is received
            return;
        }
        m_pPSOTask->setAnalysisStatus(xfl::RUNNING);


        QString strange("Optimizing for: ");
        for(int iobj=0; iobj<m_pPSOTask->nObjectives(); iobj++)
        {
            OptObjective const &obj = m_pPSOTask->objective(iobj);
            strange += obj.m_Name;
            strange += QString::asprintf("=%7.3g ", obj.m_Target);
        }
        outputText(strange+"\n");

        for(int iobj=0; iobj<m_pPSOTask->nObjectives(); iobj++)
        {
            OptObjective const &obj = m_pPSOTask->objective(iobj);
            Curve *pCurve = m_ErrorGraph.curve(iobj);
            if(!pCurve) pCurve = m_ErrorGraph.addCurve();
            pCurve->setName(obj.m_Name+"_error");
            pCurve->clear();
        }

        m_pPSOTask->setAnalysisStatus(xfl::RUNNING);
        m_pPSOTask->restartIterations();
        m_pPSOTask->clearPareto();  // current Pareto may be obsolete if target values have changed since swarm creation
        m_pPSOTask->makeParetoFront();

        disconnect(&m_Timer, nullptr, nullptr, nullptr);
        connect(&m_Timer, SIGNAL(timeout()), m_pPSOTask, SLOT(onIteration()));
        m_Timer.start(s_Dt);
    }
    else
    {
        if(!m_pGATask) onMakeSwarm(false);

        QString strange("Optimizing for ");
        OptObjective const &obj = m_pGATask->objective();
        strange += obj.m_Name + QString::asprintf("=%.3g @ aoa=%.3g", obj.m_Target, m_pGATask->alpha())+QChar(0260);
        outputText(strange + "\n");
        Curve *pCurve = m_ErrorGraph.curve(0);
        if(!pCurve) pCurve = m_ErrorGraph.addCurve();
        pCurve->setName(obj.m_Name + "_error");
        pCurve->clear();

        if(m_pGATask->isRunning())
        {
            m_pGATask->setAnalysisStatus(xfl::CANCELLED);
            // m_ppbSwarm->setText("Swarm"); done when the finish event is received
            return;
        }
        m_pGATask->setAnalysisStatus(xfl::RUNNING);
        m_pGATask->restartIterations();

        disconnect(&m_Timer, nullptr, nullptr, nullptr);
        connect(&m_Timer, SIGNAL(timeout()), m_pGATask, SLOT(onIteration()));
        m_Timer.start(s_Dt);
    }

    update();
}


void Optim2d::setFoil(Foil const *pFoil)
{
    m_pFoil = pFoil;
    m_pBestFoil->copyFoil(m_pFoil, false);
    m_pBestFoil->setColor(QColor(151, 107, 73));
    m_pBestFoil->setName("Best foil");

    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
}


void Optim2d::onAnalyze()
{
    m_Timer.stop();

    s_Alpha = m_pdeAlpha->value();

    Polar polar;
    polar.setReType(1);
    polar.setMaType(1);
    polar.setReynolds(s_Re);
    polar.setNCrit(s_NCrit);
    polar.setXtrTop(s_XtrTop);
    polar.setXtrBot(s_XtrBot);
    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask; // watch the stack
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, s_Alpha, s_Alpha, 0.0);
    task->initializeXFoilTask(m_pFoil, &polar, bViscous, bInitBL, false);
    task->run();
    outputText(strange+"\n");

    m_iLE = -1;
    for(int i=0; i<xfoil.n-1; i++)
    {
        if(xfoil.x[i+1]<xfoil.x[i+2])
        {
            m_iLE = i;
            break;
        }
    }
    outputText(QString::asprintf("LE is at index %d\n", m_iLE));

    update();
    delete task;
}


void Optim2d::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == OPTIM_ITER_EVENT)
    {
        OptimEvent *pOptEvent = dynamic_cast<OptimEvent*>(pEvent);
        Particle const &particle = pOptEvent->particle();

        QString strange = QString::asprintf("It.%2d:", pOptEvent->iter());
        if(m_pPSOTask)
        {
            for(int iobj=0; iobj<particle.nObjectives(); iobj++)
            {
                OptObjective const &obj = m_pPSOTask->objective(iobj);
                strange += obj.m_Name + "_err";
                strange += QString::asprintf("=%7.3g ", particle.error(iobj));
            }
        }
        else if(m_pGATask)
        {
            OptObjective const &obj = m_pGATask->objective();
            strange += obj.m_Name + "_err";
            strange += QString::asprintf("=%7.3g ", particle.error(0));
        }
        outputText(strange+"\n");

        // display best foil - method may not be the same for each case
        if     (m_pPSOTask) m_pPSOTask->makeFoil(particle, m_pBestFoil);
        else if(m_pGATask)  m_pGATask->makeFoil(&particle, m_pBestFoil);

        // update graph
        for(int iobj=0; iobj<particle.nObjectives(); iobj++)
        {
            Curve *pCurve = m_ErrorGraph.curve(iobj);
            if(pCurve) pCurve->appendPoint(pOptEvent->iter(), particle.error(iobj));
        }
        m_ErrorGraph.resetLimits();

        if(m_pPSOTask) updateParetoGraph();

        update();
    }
    else if(pEvent->type()==OPTIM_END_EVENT)
    {
        OptimEvent *pOptEvent = dynamic_cast<OptimEvent*>(pEvent);
        Particle const &particle = pOptEvent->particle();
        m_Timer.stop();
        disconnect(&m_Timer, nullptr, nullptr, nullptr);

        // display best foil - method may not be the same for each case
        if (m_pPSOTask)
        {
            m_pPSOTask->makeFoil(particle, m_pBestFoil);
            updateParetoGraph();
        }
        else if(m_pGATask)
        {
            // Already built at the last iteration
/*            Particle particle = m_pGATask->bestParticle();
            m_pGATask->makeFoil(&particle, m_pBestFoil);*/
        }

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


void Optim2d::updateParetoGraph()
{
    if(m_pPSOTask)
    {
        if(m_pPSOTask->nObjectives()!=2)
        {
            outputText("\nTwo objectives are required to build the Pareto graph\n");
            return;
        }

        m_ParetoGraph.setXTitle(m_pPSOTask->objective(0).m_Name+"_err");
        m_ParetoGraph.setYTitle(m_pPSOTask->objective(1).m_Name+"_err");
        m_pParetoGraphWt->setOverlayedRect(true,  m_pPSOTask->objective(0).m_MaxError, 0, 0, m_pPSOTask->objective(1).m_MaxError);

        Curve *pCurveSw  = m_ParetoGraph.curve(0);
        if(!pCurveSw)
        {
            pCurveSw = m_ParetoGraph.addCurve("Swarm");
            pCurveSw->setStipple(0);
            pCurveSw->setWidth(0);
            pCurveSw->setColor(Qt::darkYellow);
            pCurveSw->setPointStyle(1);
        }
        pCurveSw->clear();
        for(int i=0; i<m_pPSOTask->m_Swarm.size(); i++)
        {
            Particle const &particle = m_pPSOTask->m_Swarm.at(i);
            double err0 = fabs(particle.fitness(0)-m_pPSOTask->objective(0).m_Target);
            double err1 = fabs(particle.fitness(1)-m_pPSOTask->objective(1).m_Target);
            pCurveSw->appendPoint(err0, err1);
        }

        Curve *pCurveDom = m_ParetoGraph.curve(1);
        if(!pCurveDom)
        {
            pCurveDom = m_ParetoGraph.addCurve("Frontier");
            pCurveDom->setColor(Qt::cyan);
            pCurveDom->setWidth(0);
            pCurveDom->setStipple(0);
            pCurveDom->setPointStyle(2);
        }
        pCurveDom->clear();
        for(int i=0; i<m_pPSOTask->m_Pareto.size(); i++)
        {
            Particle const &particle = m_pPSOTask->m_Pareto.at(i);
            double err0 = fabs(particle.fitness(0)-m_pPSOTask->objective(0).m_Target);
            double err1 = fabs(particle.fitness(1)-m_pPSOTask->objective(1).m_Target);
            pCurveDom->appendPoint(err0, err1);
        }

        m_pParetoGraphWt->update();
    }
}
