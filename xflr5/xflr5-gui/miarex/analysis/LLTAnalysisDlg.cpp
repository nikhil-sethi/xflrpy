/****************************************************************************

    LLTAnalysisDlg Class
    Copyright (C) 2009-2016 Andre Deperrois

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

#include <math.h>
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
#include <QtDebug>

#include "LLTAnalysisDlg.h"
#include "graphwidget.h"
#include <analysis3d/plane_analysis/LLTAnalysis.h>
#include <analysis3d/plane_analysis/planeanalysistask.h>
#include <analysis3d/plane_analysis/planetaskevent.h>
#include <globals/gui_params.h>
#include <graph/curve.h>
#include <graph/graph.h>
#include <miarex/Miarex.h>
#include <miarex/objects3d.h>
#include <misc/options/displayoptions.h>
#include <objects/objects3d/WPolar.h>
#include <objects/objects3d/Wing.h>

QPoint LLTAnalysisDlg::s_Position = QPoint(200,100);
QSize  LLTAnalysisDlg::s_WindowSize = QSize(900,550);
bool LLTAnalysisDlg::s_bWindowMaximized=false;


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

    //	m_pIterGraph->SetXTitle(tr("Iterations"));
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
    m_pIterGraph->setXMax((double)LLTAnalysis::s_IterLim);
    m_pIterGraph->setX0(0.0);
    m_pIterGraph->setXUnit((int)(LLTAnalysis::s_IterLim/10.0));

    m_pIterGraph->setY0(0.0);
    m_pIterGraph->setYMin(0.0);
    m_pIterGraph->setYMax(1.0);

    m_pctrlLogFile->setChecked(Miarex::m_bLogFile);
}


void LLTAnalysisDlg::onLogFile()
{
    Miarex::m_bLogFile = m_pctrlLogFile->isChecked();
}


/** Overrides and handles the keyPressEvent sent by Qt */
void LLTAnalysisDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            onCancelAnalysis();
            event->accept();
            return;
        }
        default:
            event->ignore();
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
    QDesktopWidget desktop;
    QRect r = desktop.geometry();
    //	setMinimumHeight(r.height()*2/3);
    //	setMinimumWidth((int)(r.width()*2.0/5.0));

    m_pctrlTextOutput = new QTextEdit;
    m_pctrlTextOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_pctrlTextOutput->setReadOnly(true);
    m_pctrlTextOutput->setLineWrapMode(QTextEdit::NoWrap);
    m_pctrlTextOutput->setWordWrapMode(QTextOption::NoWrap);
    m_pctrlTextOutput->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_pGraphWidget = new GraphWidget;
    m_pGraphWidget->setMinimumHeight(r.height()/3);
    m_pGraphWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_pGraphWidget->showLegend(true);


    QHBoxLayout *pctrlLayout = new QHBoxLayout;
    {
        m_pctrlCancel = new QPushButton(tr("Cancel"));
        connect(m_pctrlCancel, SIGNAL(clicked()), this, SLOT(onCancelAnalysis()));

        m_pctrlLogFile = new QCheckBox(tr("Keep this window opened on errors"));
        connect(m_pctrlLogFile, SIGNAL(toggled(bool)), this, SLOT(onLogFile()));
        pctrlLayout->addWidget(m_pctrlLogFile);
        pctrlLayout->addStretch();
        pctrlLayout->addWidget(m_pctrlCancel);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pctrlTextOutput);
        pMainLayout->addWidget(m_pGraphWidget,2);
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

    m_pctrlCancel->setText(tr("Cancel"));
    m_bCancel     = false;
    m_bFinished   = false;

    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(onProgress()));
    m_Timer.setInterval(200);
    m_Timer.start();

    m_pctrlTextOutput->clear();

    QString strange;
    strange = pWing->wingName()+"\n";
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
    m_pIterGraph->setXMax((double)LLTAnalysis::s_IterLim);
    m_pIterGraph->setYMinGrid(false, true, QColor(100,100,100), 2, 1, 4);

    Curve *pCurve = m_pIterGraph->addCurve();
    m_pTheTask->m_ptheLLTAnalysis->setCurvePointers(&pCurve->x, &pCurve->y);

    //run the instance asynchronously
    QFuture<void> future = QtConcurrent::run(m_pTheTask, &PlaneAnalysisTask::run);

    while(future.isRunning())
    {
        qApp->processEvents();
        QThread::msleep(200);
        /*		QDateTime dt = QDateTime::currentDateTime();
        QString str = dt.toString("dd.MM.yyyy  hh:mm:ss.zzz");
        qDebug() << str<<m_pTheTask->isFinished();*/
    }

    qApp->processEvents();
    cleanUp();
}


void LLTAnalysisDlg::cleanUp()
{
    QString strange;
    m_Timer.stop();

    if(PlaneOpp::s_bStoreOpps)
    {
        for(int iPOpp=0; iPOpp<m_pTheTask->m_ptheLLTAnalysis->m_PlaneOppList.size(); iPOpp++)
        {
            Objects3d::insertPOpp(m_pTheTask->m_ptheLLTAnalysis->m_PlaneOppList.at(iPOpp));
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

        outstream << m_pctrlTextOutput->toPlainText();
        outstream.flush();
        pXFile->close();
    }
    delete pXFile;

    m_pTheTask = nullptr;
    m_pctrlCancel->setText(tr("Close"));
    m_pctrlCancel->setFocus();
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
    /*	if(m_strOut.length())
    {
        m_pctrlTextOutput->insertPlainText(m_strOut);
        m_pctrlTextOutput->textCursor().movePosition(QTextCursor::End);
        m_pctrlTextOutput->ensureCursorVisible();
    }
    m_strOut.clear();*/
    m_pGraphWidget->update();
}


/**
* Updates the text output in the dialog box and the log file.
*@param strong the text message to append to the output widget and to the log file.
*/
void LLTAnalysisDlg::updateOutput(QString &strong)
{
    m_pctrlTextOutput->insertPlainText(strong);
    m_pctrlTextOutput->ensureCursorVisible();
}



void LLTAnalysisDlg::showEvent(QShowEvent *event)
{
    move(s_Position);
    resize(s_WindowSize);
    if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

    event->accept();
}

void LLTAnalysisDlg::hideEvent(QHideEvent *event)
{
    s_WindowSize = size();
    s_bWindowMaximized = isMaximized();
    s_Position = pos();

    event->accept();
}


void LLTAnalysisDlg::customEvent(QEvent * event)
{
    // When we get here, we've crossed the thread boundary and are now
    // executing in this widget's thread

    if(event->type() == PLANE_END_POPP_EVENT)
    {
        m_pIterGraph->resetYLimits();
    }
}


void LLTAnalysisDlg::onMessage(QString msg)
{
    m_pctrlTextOutput->insertPlainText(msg);
    m_pctrlTextOutput->textCursor().movePosition(QTextCursor::End);
    m_pctrlTextOutput->ensureCursorVisible();
}


void LLTAnalysisDlg::deleteTask()
{
    if(m_pTheTask) delete m_pTheTask;
    m_pTheTask = nullptr;
}
