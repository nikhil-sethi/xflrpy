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

#include "batchthreaddlg.h"

#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/xdirect.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflwidgets/customwts/cptableview.h>

/**
 * The public contructor
 */
BatchThreadDlg::BatchThreadDlg(QWidget *pParent) : BatchAbstractDlg(pParent)
{
    setWindowTitle(tr("Multi-threaded batch analysis"));

    m_pTimer = nullptr;

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
    QVBoxLayout *pLeftSide = new QVBoxLayout;
    {
        pLeftSide->addWidget(m_pVSplitter);
        pLeftSide->addWidget(m_pTransVarsGroupBox);
        pLeftSide->addWidget(m_pRangeVarsGroupBox);
        pLeftSide->addWidget(m_pButtonBox);
    }

    QVBoxLayout *pRightSideLayout = new QVBoxLayout;
    {
        pRightSideLayout->addWidget(m_pchInitBL);
        pRightSideLayout->addWidget(m_pOptionsFrame);
        pRightSideLayout->addWidget(m_pteTextOutput);
    }

    QHBoxLayout *pBoxesLayout = new QHBoxLayout;
    {
        pBoxesLayout->addLayout(pLeftSide);
        pBoxesLayout->addLayout(pRightSideLayout);
        pBoxesLayout->setStretchFactor(pRightSideLayout, 1);
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

    QThreadPool::globalInstance()->setMaxThreadCount(2);

    m_ppbAnalyze->setFocus();
    startAnalysis();
}


/**
 * Starts the multithreaded analysis.
 * First, creates a pool list of all (Foil, pairs) to analyze.
 * Then, starts the threads which will pick the pairs from the pool and remove them once the analayis is finished.
 */
void BatchThreadDlg::startAnalysis()
{
    QString strong;
    int nRe=0;

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

    nRe = s_ReList.count();

    //    QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms

    //build an array of all analysis pairs to run
    m_nAnalysis = 0;
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    FoilAnalysis *pAnalysis=nullptr;
    for(int i=0; i<foils.count(); i++)
    {
        Foil *pFoil = foils.at(i);
        if(pFoil)
        {
            for (int iRe=0; iRe<nRe; iRe++)
            {
                pAnalysis = new FoilAnalysis;
                m_AnalysisPair.append(pAnalysis);
                pAnalysis->pFoil = pFoil;

                pAnalysis->pPolar = Objects2d::createPolar(pFoil, xfl::FIXEDSPEEDPOLAR,
                                                           s_ReList.at(iRe), s_MachList.at(iRe), s_NCritList.at(iRe),
                                                           s_XTop, s_XBot);

                m_nAnalysis++;
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

    if(m_pTimer)
    {
        m_pTimer->stop();
        delete m_pTimer;
    }
    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), SLOT(onTimerEvent()));
    m_pTimer->start(100);
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

            m_pTimer->stop();
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
        //need to check if we are still running in case a timer event arrives after a cancellation for instance.
        startThread(); // analyze a new pair
    }
}

/**
 * Starts an individual thread
 */
void BatchThreadDlg::startThread()
{
    QString strong;
    //  browse through the array until we find an available thread

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
        strong = tr("Starting ")+pAnalysis->pFoil->name()+" / "+pAnalysis->pPolar->polarName()+"\n";
        updateOutput(strong);
        QThreadPool::globalInstance()->start(pXFoilTask);

        //remove it from the array of pairs to analyze
        pAnalysis = m_AnalysisPair.last();
        m_AnalysisPair.removeLast();
        delete pAnalysis;
    }
}


/**
 * Adds a text message to the ouput widget
 * @param str the message to output
 */
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
        handleXFoilTaskEvent(static_cast<XFoilTaskEvent *>(pEvent));
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
        delete pOppEvent->theOpPoint();
    }
}


void BatchThreadDlg::handleXFoilTaskEvent(const XFoilTaskEvent *pEvent)
{
    m_nTaskDone++; //one down, more to go
    QString str = tr("   ...Finished ")+ (pEvent->foil())->name()+" / "
                  +(pEvent->polar())->polarName()+"\n";
    updateOutput(str);

    if(s_bUpdatePolarView)
    {
        s_pXDirect->createPolarCurves();
        s_pXDirect->updateView();
    }
}


