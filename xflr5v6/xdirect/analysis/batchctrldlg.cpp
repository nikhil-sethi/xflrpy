/****************************************************************************

    BatchCtrlDlg Class
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

#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QDir>
#include <QFutureSynchronizer>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

#include "batchctrldlg.h"

#include <xdirect/analysis/xfoiltask.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/actiondelegate.h>
#include <xflwidgets/customwts/actionitemmodel.h>

double BatchCtrlDlg::s_XHinge     = 0.7;
double BatchCtrlDlg::s_YHinge     = 0.5;
double BatchCtrlDlg::s_AngleMin   = 0.0;
double BatchCtrlDlg::s_AngleMax   = 0.0;
double BatchCtrlDlg::s_AngleDelta = 0.0;


BatchCtrlDlg::BatchCtrlDlg(QWidget *pParent) : BatchAbstractDlg(pParent)
{
    setWindowTitle(tr("Batch flap analysis"));

    s_bUpdatePolarView = false;

    setupLayout();
    connectBaseSignals();

    m_pchUpdatePolarView->setChecked(false);
    m_pchUpdatePolarView->hide();

    m_nTasks = m_TaskCounter = 0;
}


BatchCtrlDlg::~BatchCtrlDlg()
{
}


/**
 * Sets up the GUI
 */
void BatchCtrlDlg::setupLayout()
{
    QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
    {
        QGroupBox *pFlapBox = new QGroupBox("Flap settings");
        {
            QGridLayout *pFlapLayout = new QGridLayout;
            {
                QLabel *plabHingPos = new QLabel(tr("Hinge position"));
                QLabel *pLabXh = new QLabel("X");
                QLabel *pLabYh = new QLabel("Y");
                m_pdeXHinge = new DoubleEdit;
                m_pdeYHinge = new DoubleEdit;
                QLabel *pLab4 = new QLabel(tr("% Chord"));

                QLabel *pLabMin = new QLabel("Min.");
                QLabel *pLabMax = new QLabel("Max.");
                QLabel *pLab1   = new QLabel(tr("Flap Angle"));
                QLabel *pLabInc = new QLabel(QString(QChar(0x0394)));
                QLabel *pLabDeg = new QLabel(QChar(0260));
                m_pdeAngleMin   = new DoubleEdit;
                m_pdeAngleMax   = new DoubleEdit;
                m_pdeAngleDelta = new DoubleEdit;

                pFlapLayout->addWidget(pLabXh,          1, 2, Qt::AlignCenter);
                pFlapLayout->addWidget(pLabYh,          1, 3, Qt::AlignCenter);
                pFlapLayout->addWidget(plabHingPos,     2, 1);
                pFlapLayout->addWidget(m_pdeXHinge,     2, 2);
                pFlapLayout->addWidget(m_pdeYHinge,     2, 3);
                pFlapLayout->addWidget(pLab4,           2, 4);

                pFlapLayout->addWidget(pLabMin,         3, 2, Qt::AlignCenter);
                pFlapLayout->addWidget(pLabMax,         3, 3, Qt::AlignCenter);
                pFlapLayout->addWidget(pLabInc,         3, 4, Qt::AlignCenter);
                pFlapLayout->addWidget(pLab1,           4, 1);
                pFlapLayout->addWidget(m_pdeAngleMin,   4, 2);
                pFlapLayout->addWidget(m_pdeAngleMax,   4, 3);
                pFlapLayout->addWidget(m_pdeAngleDelta, 4, 4);
                pFlapLayout->addWidget(pLabDeg,         4, 5);

            }
            pFlapBox->setLayout(pFlapLayout);
        }

        pLeftSideLayout->addWidget(m_pVSplitter);
        pLeftSideLayout->addWidget(pFlapBox);
        pLeftSideLayout->addWidget(m_pTransVarsGroupBox);
        pLeftSideLayout->addWidget(m_pRangeVarsGroupBox);
        pLeftSideLayout->addWidget(m_pButtonBox);
    }

    QVBoxLayout *pRightSideLayout = new QVBoxLayout;
    {
        pRightSideLayout->addWidget(m_pchInitBL);
        pRightSideLayout->addWidget(m_pOptionsFrame);
        pRightSideLayout->addWidget(m_pteTextOutput);
    }

    QHBoxLayout *pBoxesLayout = new QHBoxLayout;
    {
        pBoxesLayout->addLayout(pLeftSideLayout);
        pBoxesLayout->addLayout(pRightSideLayout);
        pBoxesLayout->setStretchFactor(pRightSideLayout, 1);
    }

    setLayout(pBoxesLayout);
}


void BatchCtrlDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BatchCtrlDlg");
    {
        s_XHinge      = settings.value("TEXHinge",     s_XHinge).toDouble();
        s_YHinge      = settings.value("TEYHinge",     s_YHinge).toDouble();
        s_AngleMin    = settings.value("TEAngleMin",   s_AngleMin).toDouble();
        s_AngleMax    = settings.value("TEAngleMax",   s_AngleMax).toDouble();
        s_AngleDelta  = settings.value("TEAngleDelta", s_AngleDelta).toDouble();
    }
    settings.endGroup();
}


void BatchCtrlDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BatchCtrlDlg");
    {
        settings.setValue("TEXHinge",     s_XHinge);
        settings.setValue("TEYHinge",     s_YHinge);
        settings.setValue("TEAngleMin",   s_AngleMin);
        settings.setValue("TEAngleMax",   s_AngleMax);
        settings.setValue("TEAngleDelta", s_AngleDelta);
    }
    settings.endGroup();
}


void BatchCtrlDlg::initDialog()
{
    BatchAbstractDlg::initDialog();

    m_pdeAngleMin->setValue(s_AngleMin);
    m_pdeAngleMax->setValue(s_AngleMax);
    m_pdeAngleDelta->setValue(s_AngleDelta);
    m_pdeXHinge->setValue(s_XHinge*100.0);
    m_pdeYHinge->setValue(s_YHinge*100.0);
}


void BatchCtrlDlg::readParams()
{
    BatchAbstractDlg::readParams();
    s_AngleMin   = m_pdeAngleMin->value();
    s_AngleMax   = m_pdeAngleMax->value();
    s_AngleDelta = m_pdeAngleDelta->value();
    s_XHinge     = m_pdeXHinge->value()/100.0;
    s_YHinge     = m_pdeYHinge->value()/100.0;

    QThreadPool::globalInstance()->setMaxThreadCount(s_nThreads);
}


/**
 * If an analysis is running, cancels the XFoilTask and returns.
 * It not, launches the analysis.
 */
void BatchCtrlDlg::onAnalyze()
{
    if(m_bIsRunning)
    {
        m_pteTextOutput->appendPlainText("Cancelling analyses\n");
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

    startAnalyses();
}


void BatchCtrlDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type()==XFOIL_END_TASK_EVENT)
    {
        XFoilTaskEvent *pTaskEvent = dynamic_cast<XFoilTaskEvent*>(pEvent);
        m_TaskCounter++;
        QString strange = QString::asprintf("%d/%d: ", m_TaskCounter, m_nTasks) + "Finished task "+ pTaskEvent->foil()->name() + " / " + pTaskEvent->polar()->name();
        m_pteTextOutput->appendPlainText(strange);
        if(m_TaskCounter>=m_nTasks)
        {
            m_pteTextOutput->appendPlainText("All tasks completed");
            cleanUp();
        }
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
        delete pOppEvent->theOpPoint();
    }
}


/**
 * Starts the multithreaded analysis.
 * First, creates a pool list of all (Foil, pairs) to analyze.
 * Then, starts the threads which will pick the pairs from the pool and remove them once the analayis is finished.
 */
void BatchCtrlDlg::startAnalyses()
{
    QString strange;
    int nRe=0;


    QVector<Foil*> foils;
    readFoils(foils);

    if(foils.isEmpty())
    {
        strange ="No foil defined for analysis\n\n";
        m_pteTextOutput->insertPlainText(strange);
        cleanUp();
        return;
    }

    qApp->setOverrideCursor(Qt::BusyCursor);

    m_ppbAnalyze->setText(tr("Cancel"));

     nRe = s_ReList.count();

    m_nTasks = foils.size()*nRe; // to monitor the progress
    m_TaskCounter = 0;

    XFoilTask::s_bCancel = false;

    m_pteTextOutput->appendPlainText("Starting analyses");

    for(int i=0; i<foils.count(); i++)
    {
        Foil *pFoil = foils.at(i);
        if(!pFoil) continue;

        m_pteTextOutput->appendPlainText("   starting XFoil tasks for "+pFoil->name());

//        QFutureSynchronizer<void> futureSync;
        for (int iRe=0; iRe<nRe; iRe++)
        {
            Polar *pPolar = nullptr;
            pPolar = Objects2d::createPolar(pFoil, xfl::FIXEDSPEEDPOLAR, s_ReList.at(iRe),
                                            s_MachList[iRe], s_NCritList[iRe], s_XTop, s_XBot);

            //initiate the task
            XFoilTask *pXFoilTask = new XFoilTask(this);

            if(s_bAlpha) pXFoilTask->setSequence(true,  s_AlphaMin, s_AlphaMax, s_AlphaInc);
            else         pXFoilTask->setSequence(false, s_ClMin, s_ClMax, s_ClInc);
            pXFoilTask->initializeXFoilTask(pFoil, pPolar, true, s_bInitBL, s_bFromZero);

//            QFuture<Polar*> future = QtConcurrent::run(pXFoilTask, &XFoilTask::runFuture);
//            futureSync.addFuture(future);

            QThreadPool::globalInstance()->start(pXFoilTask);
        }
//        futureSync.waitForFinished(); // maybe unnecessary: "The destructor of QFutureSynchronizer calls waitForFinished()"
        m_pteTextOutput->appendPlainText("   finished launching XFoil tasks for " + pFoil->name());

        s_pXDirect->resetCurves();
        s_pXDirect->updateView();
    }
    m_pteTextOutput->appendPlainText("All tasks launched... waiting\n");

    m_bIsRunning = true;
}



