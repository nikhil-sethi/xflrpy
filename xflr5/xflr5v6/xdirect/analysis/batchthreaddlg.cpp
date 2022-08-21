/****************************************************************************

    BatchThreadDlg Class
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


#include <QVBoxLayout>
#include <QGroupBox>
#include <QCoreApplication>
#include <QThreadPool>
#include <QTimer>
#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <QDebug>

#include "batchthreaddlg.h"

#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/xdirect.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/plaintextoutput.h>

/**
 * The public contructor
 */
BatchThreadDlg::BatchThreadDlg(QWidget *pParent) : BatchAbstractDlg(pParent)
{
    setWindowTitle(tr("Multi-threaded batch analysis"));

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onTimerEvent()));

    m_nTaskDone    = 0;
    m_nTaskStarted = 0;
    m_nAnalysis    = 0;

    setupLayout();
    connectBaseSignals();
}


BatchThreadDlg::~BatchThreadDlg()
{
    //clean up the rest of the analysis in case of cancellation
    for(int ia=m_AnalysisPair.count()-1; ia>=0; ia--)
    {
        FoilAnalysis *pAnalysis = m_AnalysisPair.last();
        m_AnalysisPair.removeLast();
        delete pAnalysis;
    }
}


/**
 * Sets up the GUI
 */
void BatchThreadDlg::setupLayout()
{
    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        connect(m_pHSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onResizeColumns()));
        QFrame *pLeftFrame = new QFrame;
        {
            QVBoxLayout *pLeftSide = new QVBoxLayout;
            {
                pLeftSide->addWidget(m_pVSplitter);
                pLeftSide->addWidget(m_pgbPolarType);
                pLeftSide->addWidget(m_pgbTransVars);
                pLeftSide->addWidget(m_pgbRangeVars);
                pLeftSide->addWidget(m_pButtonBox);
            }
            pLeftFrame->setLayout(pLeftSide);
        }

        QFrame *pRightFrame = new QFrame;
        {
            QVBoxLayout *pRightSideLayout = new QVBoxLayout;
            {
                pRightSideLayout->addWidget(m_pfrOptions);
                pRightSideLayout->addWidget(m_pteTextOutput);
            }
            pRightFrame->setLayout(pRightSideLayout);
        }
        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(pRightFrame);
    }

    QHBoxLayout *pBoxesLayout = new QHBoxLayout;
    {
        pBoxesLayout->addWidget(m_pHSplitter);
    }

    setLayout(pBoxesLayout);
}


/**
 * If an analysis is running, cancels the XFoilTask and returns.
 * It not, launches the analysis.
 */
void BatchThreadDlg::onAnalyze()
{
    if(m_bIsRunning)
    {
        m_bCancel = true;
        XFoilTask::s_bCancel = true;
        XFoil::setCancel(true);
        return;
    }

    m_bCancel    = false;
    m_bIsRunning = true;

    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);

    QString FileName = QDir::tempPath() + "/XFLR5.log";
    m_pXFile = new QFile(FileName);
    if (!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text)) m_pXFile = nullptr;

    readParams();

    setFileHeader();
    s_bInitBL = m_pchInitBL->isChecked();

    m_ppbAnalyze->setFocus();

    QString strong;

    QVector<Foil*> foils;
    readFoils(foils);

    if(foils.isEmpty())
    {
        strong ="No foil defined for analysis\n\n";
        m_pteTextOutput->insertPlainText(strong);
        cleanUp();
        return;
    }

    m_ppbAnalyze->setText(tr("Cancel"));

//    int nRe = s_ReList.count();
/*    int nRe = 0;
    for(int i=0; i<s_ActiveList.size(); i++) nRe += s_ActiveList.at(i) ? 1 : 0;*/

    //    QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms

    //build an array of all analysis pairs to run
    m_nAnalysis = 0;
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    for(int ifoil=0; ifoil<foils.count(); ifoil++)
    {
        Foil *pFoil = foils.at(ifoil);
        if(pFoil)
        {
            for (int iRe=0; iRe<s_ActiveList.size(); iRe++)
            {
                if(s_ActiveList.at(iRe))
                {
                    FoilAnalysis *pAnalysis = new FoilAnalysis;
                    m_AnalysisPair.append(pAnalysis);
                    pAnalysis->pFoil = pFoil;

                    pAnalysis->pPolar = Objects2d::createPolar(pFoil, s_PolarType,
                                                               s_ReList.at(iRe), s_MachList.at(iRe), s_NCritList.at(iRe),
                                                               s_XTop, s_XBot);
                    m_nAnalysis++;
                }
            }
        }
    }
    strong = QString(tr("Found %1 foil/polar pairs to analyze\n")).arg(m_nAnalysis);
    m_pteTextOutput->insertPlainText(strong);

    // Start as many threads as the user has requested
    // This is a complicated way of doing things; QFuture and QRunnable are simpler

    XFoilTask::s_bCancel = false;

    strong = QString(tr("Starting with %1 threads\n\n")).arg(s_nThreads);
    m_pteTextOutput->insertPlainText(strong);
    m_pteTextOutput->insertPlainText(tr("\nStarted/Done/Total\n"));

    if(m_Timer.isActive())  m_Timer.stop();
    m_Timer.start(30);
}


/**
 * A timer event used to check at regular intervals if any threads are inactive
 * in which case it launches a task if any are left.
 * Also checks if the user has pressed the Cancel button
*/
void BatchThreadDlg::onTimerEvent()
{
    QString strong;
    //time to launch another analysis, if any are left

    if(m_AnalysisPair.size()<=0 || m_bCancel)
    {
        //nothing left to launch...
        if(m_nTaskDone>=m_nAnalysis || m_bCancel)
        {
            QThreadPool::globalInstance()->waitForDone();

            if(m_bCancel) strong = tr("\n_____Analysis cancelled_____\n");
            else          strong = tr("\n_____Analysis completed_____\n");
            m_pteTextOutput->insertPlainText(strong);
            m_pteTextOutput->ensureCursorVisible();

            m_Timer.stop();
            cleanUp();

            if(s_pXDirect->m_bPolarView && s_bUpdatePolarView)
            {
                s_pXDirect->createPolarCurves();
                s_pXDirect->updateView();
            }
        }
    }
    else if(m_bIsRunning)
    {
        QString strong;
        if(QThreadPool::globalInstance()->activeThreadCount()<s_nThreads && m_AnalysisPair.count())
        {
            XFoilTask *pXFoilTask = new XFoilTask(this);
            //take the last analysis in the array
            FoilAnalysis *pAnalysis = m_AnalysisPair.at(m_AnalysisPair.size()-1);

            pAnalysis->pPolar->setVisible(true);

            //initiate the task
            if(s_bAlpha) pXFoilTask->setSequence(true,  s_AlphaMin, s_AlphaMax, s_AlphaInc);
            else         pXFoilTask->setSequence(false, s_ClMin, s_ClMax, s_ClInc);

            pXFoilTask->initializeXFoilTask(pAnalysis->pFoil, pAnalysis->pPolar, true, s_bInitBL, s_bFromZero);

            //launch it
            m_nTaskStarted++;
            strong = tr("Starting ")+pAnalysis->pFoil->name()+" / "+pAnalysis->pPolar->polarName() + "\n";
            updateOutput(strong);

//            QThreadPool::globalInstance()->start(pXFoilTask);

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
            QFuture<void> future = QtConcurrent::run(&XFoilTask::run, pXFoilTask);
#else
            QtConcurrent::run(pXFoilTask, &XFoilTask::run);
#endif
            //remove it from the array of pairs to analyze
            pAnalysis = m_AnalysisPair.last();
            m_AnalysisPair.removeLast();
            delete pAnalysis;
        }
    }
}



void BatchThreadDlg::updateOutput(QString const&str)
{
    QString strong;
    strong = QString::asprintf("%3d/%3d/%3d  ", m_nTaskStarted, m_nTaskDone, m_nAnalysis);
    m_pteTextOutput->insertPlainText(strong + str);
    m_pteTextOutput->ensureCursorVisible();
}


/**
 * Clean-up is performed when all the threads have finished
 */
void BatchThreadDlg::cleanUp()
{
    BatchAbstractDlg::cleanUp();

    //in case we cancelled, delete all Analyses that are left
    for(int ia=m_AnalysisPair.count()-1; ia>=0; ia--)
    {
        FoilAnalysis *pAnalysis = m_AnalysisPair.at(ia);
        delete pAnalysis;
    }
    m_AnalysisPair.clear();
}


void BatchThreadDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type() == XFOIL_END_TASK_EVENT)
    {
        XFoilTaskEvent*pXFEvent = static_cast<XFoilTaskEvent *>(pEvent);
        m_nTaskDone++; //one down, more to go
        QString str = tr("   ...Finished ")+ (pXFEvent->foil())->name()+" / "
                      +(pXFEvent->polar())->polarName()+"\n";
        updateOutput(str);

        if(s_bUpdatePolarView)
        {
            s_pXDirect->createPolarCurves();
            s_pXDirect->updateView();
        }
        delete pXFEvent->task();
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
        if(OpPoint::bStoreOpp())  Objects2d::insertOpPoint(pOppEvent->theOpPoint());
        else                      delete pOppEvent->theOpPoint();
    }
}


