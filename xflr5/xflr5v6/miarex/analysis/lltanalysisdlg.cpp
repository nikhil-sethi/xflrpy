/****************************************************************************

    LLTAnalysisDlg Class
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


#include <QApplication>
#include <QDir>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentRun>
#include <QFontDatabase>
#include <QDateTime>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

#include "lltanalysisdlg.h"
#include <miarex/miarex.h>
#include <xflobjects/objects3d/objects3d.h>
#include <misc/options/settingswt.h>
#include <xflanalysis/plane_analysis/lltanalysis.h>
#include <xflanalysis/plane_analysis/planetask.h>
#include <xflanalysis/plane_analysis/planetaskevent.h>
#include <xflcore/gui_params.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/curve.h>
#include <xflgraph/graph.h>
#include <xflobjects/objects3d/wing.h>
#include <xflobjects/objects3d/wpolar.h>

QByteArray LLTAnalysisDlg::s_Geometry;

/**
*The public constructor
*/
LLTAnalysisDlg::LLTAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("LLT Analysis"));

    setupLayout();

    m_pTheTask = nullptr;

    m_pIterGraph = new Graph();

    m_pGraphWidget->setGraph(m_pIterGraph);
    m_pIterGraph->copySettings(&Settings::s_RefGraph, false);

    //    m_pIterGraph->SetXTitle(tr("Iterations"));
    m_pIterGraph->setYTitle("");
    QFont fnt("Symbol");
    m_pIterGraph->setTitleFont(fnt);

    m_pIterGraph->setAuto(true);

    m_pIterGraph->setMargin(40);

    m_pIterGraph->setXMajGrid(true, QColor(120,120,120),2,1);
    m_pIterGraph->setYMajGrid(true, QColor(120,120,120),2,1);

    m_pIterGraph->setXMin(0.0);
    m_pIterGraph->setXMax(50);
    m_pIterGraph->setYMin(0.0);
    m_pIterGraph->setYMax(1.0);
    m_pIterGraph->setScaleType(1);

    m_pIterGraph->setYTitle("|Da|");


    m_bCancel     = false;
    m_bFinished   = false;

    m_LegendPlace.rx() = 0;
    m_LegendPlace.ry() = 0;
}

/**
 * The class destructor.
 */
LLTAnalysisDlg::~LLTAnalysisDlg()
{
    if(m_pTheTask)   delete m_pTheTask;
    if(m_pIterGraph) delete m_pIterGraph;
}


/**
*Initializes the dialog and its associated data.
*/
void LLTAnalysisDlg::initDialog()
{
    m_pIterGraph->deleteCurves();

    m_pIterGraph->setXMin(0.0);
    m_pIterGraph->setXMax(double(LLTAnalysis::s_IterLim));
    m_pIterGraph->setX0(0.0);
    m_pIterGraph->setXUnit(int(LLTAnalysis::s_IterLim/10.0));

    m_pIterGraph->setY0(0.0);
    m_pIterGraph->setYMin(0.0);
    m_pIterGraph->setYMax(1.0);

    m_pchLogFile->setChecked(Miarex::s_bLogFile);
}


void LLTAnalysisDlg::onLogFile()
{
    Miarex::s_bLogFile = m_pchLogFile->isChecked();
}


/** Overrides and handles the keyPressEvent sent by Qt */
void LLTAnalysisDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            onCancelAnalysis();
            pEvent->accept();
            return;
        }
        default:
            pEvent->ignore();
    }
}



/** The user has requested the cancellation of the analysis*/
void LLTAnalysisDlg::onCancelAnalysis()
{
    m_bCancel = true;
    if(m_bFinished) accept();
}



/**
* Initializes the interface of the dialog box
*/
void LLTAnalysisDlg::setupLayout()
{
    m_pteTextOutput = new QTextEdit;
    m_pteTextOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_pteTextOutput->setReadOnly(true);
    m_pteTextOutput->setLineWrapMode(QTextEdit::NoWrap);
    m_pteTextOutput->setWordWrapMode(QTextOption::NoWrap);

    m_pGraphWidget = new GraphWt;

    m_pGraphWidget->showLegend(true);


    QHBoxLayout *pctrlLayout = new QHBoxLayout;
    {
        m_ppbCancel = new QPushButton(tr("Cancel"));
        connect(m_ppbCancel, SIGNAL(clicked()), this, SLOT(onCancelAnalysis()));

        m_pchLogFile = new QCheckBox(tr("Keep this window opened on errors"));
        connect(m_pchLogFile, SIGNAL(toggled(bool)), this, SLOT(onLogFile()));
        pctrlLayout->addWidget(m_pchLogFile);
        pctrlLayout->addStretch();
        pctrlLayout->addWidget(m_ppbCancel);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pteTextOutput,1);
        pMainLayout->addWidget(m_pGraphWidget,1);
        pMainLayout->addLayout(pctrlLayout);
    }
    setLayout(pMainLayout);
}


/**
* Launches the analysis.
* At this stage, the dialogbox has been setup and initialized.
* The LLTAnalysis object has been created and the input data has been loaded.
* Depending on the type of polar, the method launches either a loop over aoa or velocity values.
*/
void LLTAnalysisDlg::analyze()
{
    if(!m_pTheTask->m_pPlane || !m_pTheTask->m_pWPolar) return;
    //all set to launch the analysis

    Wing *pWing = m_pTheTask->m_pPlane->wing();

    m_ppbCancel->setText(tr("Cancel"));
    m_bCancel     = false;
    m_bFinished   = false;

    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(onProgress()));
    m_Timer.setInterval(200);
    m_Timer.start();

    m_pteTextOutput->clear();

    QString strange;
    strange = pWing->name()+"\n";
    updateOutput(strange);
    strange = m_pTheTask->m_pWPolar->polarName()+"\n";
    updateOutput(strange);

    strange = tr("Launching analysis....")+"\n\n";
    updateOutput(strange);
    strange = QString(tr("Max iterations     = %1")+"\n").arg(LLTAnalysis::s_IterLim);
    updateOutput(strange);
    strange = QString(tr("Alpha precision    = %1 deg")+"\n").arg(LLTAnalysis::s_CvPrec,0,'f',6);
    updateOutput(strange);
    strange = QString(tr("Number of stations = %1")+"\n").arg(LLTAnalysis::s_NLLTStations);
    updateOutput(strange);
    strange = QString(tr("Relaxation factor  = %1")+"\n\n").arg(LLTAnalysis::s_RelaxMax,0,'f',1);
    updateOutput(strange);

    m_pIterGraph->deleteCurves();
    m_pIterGraph->resetLimits();
    m_pIterGraph->setXMax(double(LLTAnalysis::s_IterLim));

    Curve *pCurve = m_pIterGraph->addCurve();
    m_pTheTask->m_ptheLLTAnalysis->setCurvePointers(&pCurve->m_x, &pCurve->m_y);

    //run the instance asynchronously
    disconnect(m_pTheTask, nullptr, nullptr, nullptr);
    connect(m_pTheTask,  SIGNAL(taskFinished()),   this,          SLOT(onTaskFinished()));
    QFuture<void> future = QtConcurrent::run(m_pTheTask, &PlaneTask::run);
}


void LLTAnalysisDlg::onTaskFinished()
{
    QString strange;
    m_Timer.stop();

    if(PlaneOpp::s_bStoreOpps)
    {
        for(int iPOpp=0; iPOpp<m_pTheTask->m_ptheLLTAnalysis->m_PlaneOppList.size(); iPOpp++)
        {
            PlaneOpp *pPOpp = m_pTheTask->m_ptheLLTAnalysis->m_PlaneOppList.at(iPOpp);
            if(DisplayOptions::isAlignedChildrenStyle())
            {
                pPOpp->setTheStyle(m_pTheTask->m_pWPolar->theStyle());
            }

            pPOpp->setVisible(true);

            Objects3d::insertPOpp(pPOpp);
        }
    }
    else
    {
        m_pTheTask->m_ptheLLTAnalysis->clearPOppList();
    }

    m_bFinished = true;
    strange = "\n_________\n"+tr("Analysis completed");
    if(m_pTheTask->m_ptheLLTAnalysis->m_bWarning)     strange += tr(" ...some points are outside the flight envelope");
    else if(m_pTheTask->m_ptheLLTAnalysis->m_bError)  strange += tr(" ...some points are unconverged");

    strange+= "\n";

    m_pTheTask->m_ptheLLTAnalysis->traceLog(strange);
    onProgress();

    QString FileName = QDir::tempPath() + "/XFLR5.log";
    QFile *pXFile = new QFile(FileName);
    if(pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outstream(pXFile);
        outstream << "\n";
        outstream << VERSIONNAME;
        outstream << "\n";
        QDateTime dt = QDateTime::currentDateTime();
        QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
        outstream << str<<"\n";

        outstream << m_pteTextOutput->toPlainText();
        outstream.flush();
        pXFile->close();
    }
    delete pXFile;

    m_pTheTask = nullptr;
    m_ppbCancel->setText(tr("Close"));
    m_ppbCancel->setFocus();

    emit lltAnalysisFinished();
}


/**
* Updates the graph widget. Called after each iteration of the LLTAnalysis. 
* Time consuming, but it's necessary to provide the user with visual feedback on the progress of the analysis
*/
void LLTAnalysisDlg::updateView()
{
    m_pGraphWidget->update();
    repaint();
}


/**Updates the progress of the analysis in the slider widget */
void LLTAnalysisDlg::onProgress()
{
    m_pGraphWidget->update();
}


/**
* Updates the text output in the dialog box and the log file.
*@param strong the text message to append to the output widget and to the log file.
*/
void LLTAnalysisDlg::updateOutput(QString &strong)
{
    m_pteTextOutput->insertPlainText(strong);
    m_pteTextOutput->ensureCursorVisible();
}


void LLTAnalysisDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void LLTAnalysisDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void LLTAnalysisDlg::customEvent(QEvent * pEvent)
{
    // When we get here, we've crossed the thread boundary and are now
    // executing in this widget's thread

    if(pEvent->type() == PLANE_END_POPP_EVENT)
    {
        m_pIterGraph->resetYLimits();
    }
}


void LLTAnalysisDlg::onMessage(QString msg)
{
    m_pteTextOutput->insertPlainText(msg);
    m_pteTextOutput->textCursor().movePosition(QTextCursor::End);
    m_pteTextOutput->ensureCursorVisible();
}


void LLTAnalysisDlg::deleteTask()
{
    if(m_pTheTask) delete m_pTheTask;
    m_pTheTask = nullptr;
}
