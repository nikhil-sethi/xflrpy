/****************************************************************************

    PanelAnalysisDlg Class
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

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QDateTime>
#include <QDesktopWidget>
#include <QTimer>
#include <QDir>
#include <QKeyEvent>
#include <QFontDatabase>


#include "panelanalysisdlg.h"
#include <miarex/miarex.h>
#include <xflobjects/objects3d/objects3d.h>

#include <xflanalysis/plane_analysis/panelanalysis.h>
#include <xflanalysis/plane_analysis/planetask.h>
#include <xflcore/displayoptions.h>
#include <xflcore/gui_params.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>

QByteArray PanelAnalysisDlg::s_Geometry;

/**
* The public constructor
*/
PanelAnalysisDlg::PanelAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("3D Panel Analysis"));
    setupLayout();

    m_pTheTask = nullptr;
}


/**
 * The public destructor.
 *
 */
PanelAnalysisDlg::~PanelAnalysisDlg()
{
    deleteTask();
}


/**
* Initializes the dialog and the analysis
*/
void PanelAnalysisDlg::initDialog()
{
    m_Progress = 0.0;
    m_ppbProgress->setValue(int(m_Progress));
    m_pteOutput->clear();
    m_pchLogFile->setChecked(Miarex::s_bLogFile);
}


/** Overrides the keyPressEvent sent by Qt */
void PanelAnalysisDlg::keyPressEvent(QKeyEvent *event)
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


/** The user has requested to cancel the on-going analysis*/
void PanelAnalysisDlg::onCancelAnalysis()
{
    PanelAnalysis::s_bCancel = true;
    if(m_bIsFinished)
    {
        PanelAnalysis::s_bCancel = false;
        //        QThreadPool::globalInstance()->waitForDone();
        done(1);
    }
}


void PanelAnalysisDlg::onLogFile()
{
    Miarex::s_bLogFile = m_pchLogFile->isChecked();
}


/**Updates the progress of the analysis in the slider widget */
void PanelAnalysisDlg::onProgress()
{
    /*    QTime dt = QTime::currentTime();
    QString str = dt.toString("hh:mm:ss.zzz");
    qDebug() << str;*/

    m_ppbProgress->setMaximum(int(m_pTheTask->m_pthePanelAnalysis->m_TotalTime));
    m_ppbProgress->setValue(int(m_pTheTask->m_pthePanelAnalysis->m_Progress));
    if(m_strOut.length())
    {
        m_pteOutput->insertPlainText(m_strOut);
        m_pteOutput->textCursor().movePosition(QTextCursor::End);
        m_pteOutput->ensureCursorVisible();
        m_strOut.clear();
    }
}


void PanelAnalysisDlg::setupLayout()
{
    m_pteOutput = new QTextEdit(this);
    m_pteOutput->setReadOnly(true);
    m_pteOutput->setLineWrapMode(QTextEdit::NoWrap);
    m_pteOutput->setWordWrapMode(QTextOption::NoWrap);
    m_pteOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_ppbProgress = new QProgressBar(this);
    m_ppbProgress->setOrientation(Qt::Horizontal);
    m_ppbProgress->setMinimum(0);
    m_ppbProgress->setMaximum(100);
    m_ppbProgress->setValue(0);


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
        pMainLayout->addWidget(m_pteOutput);
        pMainLayout->addWidget(m_ppbProgress);
        pMainLayout->addLayout(pctrlLayout);
    }
    setLayout(pMainLayout);
}





/**
* Starts the panel or VLM analysis
*
* Method applied from v6.00 onwards:
* 
* First case :
*    If the analysis is for a wing and not a plane, the full 3D panel method is applied
*    and the wing is modelled as a thick surface
*    The method is strictly the one described in NASA TN 4023
*    The boundary condition is of the Dirichlet type, which has proved more convincing than the Neumann BC for full 3D panel methods
*
* Second case :
*    If the analysis is for a plane, the full 3D method is not applicable since the 
*    junctions between wing and body, or between fin and elevator, cannot be adequately 
*    represented as closed surfaces. This would require a 3D CAD programe. 
*    Therefore, in this case, the wings are modelled as thin surfaces.
*    Trial tests using the method of NASA TN4023 have not been conclusive. With a uniform doublet
*    distribution and a boundary condition applied at the panel's centroid, the results 
*    are less convincing than with VLM.
*    Therefore in this case, the VLM1 method is applied to the thin surfaces, and the 3D-panel method
*    is applied to the body.
*    Last consideration : since the potential of a straight vortex line requires a lot of computations, 
*    the Neumann type BC is applied to the body panels, rather than the Dirichlet type BC
*/
void PanelAnalysisDlg::analyze()
{
    if(!m_pTheTask) return;

    m_ppbCancel->setText(tr("Cancel"));
    m_bIsFinished = false;

    m_ppbProgress->setMaximum(100000);

    clock.start(); // put some pressure

    QString strange = "\n" + QString(VERSIONNAME) +"\n";
    updateOutput(strange);
    QDateTime dt = QDateTime::currentDateTime();
    strange = dt.toString("dd.MM.yyyy  hh:mm:ss\n\n");
    updateOutput(strange);
    strange = "Launching Analysis\n\n";
    updateOutput(strange);


    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(onProgress()));
    m_Timer.setInterval(250);
    m_Timer.start();

    //run the instance asynchronously
    disconnect(m_pTheTask, nullptr, nullptr, nullptr);
    connect(m_pTheTask,  SIGNAL(taskFinished()),   this,  SLOT(onTaskFinished()));
    QFuture<void> future = QtConcurrent::run(m_pTheTask, &PlaneTask::run);
}


void PanelAnalysisDlg::onTaskFinished()
{
    QString strong;
    m_Timer.stop();

    //WPolar has been populated with results by the PlaneTask
    //Store the POpps if requested
    if(PlaneOpp::s_bStoreOpps)
    {
        for(int iPOpp=0; iPOpp<m_pTheTask->m_pthePanelAnalysis->m_PlaneOppList.size(); iPOpp++)
        {
            //add the data to the polar object
            PlaneOpp *pPOpp = m_pTheTask->m_pthePanelAnalysis->m_PlaneOppList.at(iPOpp);

            if(DisplayOptions::isAlignedChildrenStyle())
            {
                pPOpp->setTheStyle(m_pTheTask->m_pWPolar->theStyle());
            }

            pPOpp->setVisible(true);

            if(PlaneOpp::s_bKeepOutOpps || !pPOpp->isOut())    Objects3d::insertPOpp(pPOpp);
            else
            {
                delete pPOpp;
                pPOpp = nullptr;
            }
        }
    }
    else
    {
        m_pTheTask->m_pthePanelAnalysis->clearPOppList();
    }

    m_bIsFinished = true;

    if (!PanelAnalysis::s_bCancel && !PanelAnalysis::s_bWarning)
        strong = "\n"+tr("Panel Analysis completed successfully")+"\n";
    else if (PanelAnalysis::s_bWarning)
        strong = "\n"+tr("Panel Analysis completed ... Errors encountered")+"\n";

    updateOutput(strong);
    onProgress();

    QString FileName = QDir::tempPath() + "/XFLR5.log";
    QFile *pXFile = new QFile(FileName);
    if(pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outstream(pXFile);

        outstream << m_pteOutput->toPlainText();
        outstream << "\n";
        QDateTime dt = QDateTime::currentDateTime();
        QString str = dt.toString(Qt::DefaultLocaleLongDate);
        outstream << "Analysis ended "<<str<<"\n";
        outstream << "Elapsed: "<<double(clock.elapsed())/1000.0<<" s";
        outstream << "\n";
        outstream.flush();
        pXFile->close();
    }
    delete pXFile;

    m_pTheTask = nullptr;

    m_ppbCancel->setText(tr("Close"));
    m_ppbCancel->setFocus();

    emit analysisFinished();
}


/**
* Updates the text output in the dialog box and the log file.
*@param strong the text message to append to the output widget and to the log file.
*/
void PanelAnalysisDlg::updateOutput(QString const&strong)
{
    m_pteOutput->insertPlainText(strong);
    m_pteOutput->textCursor().movePosition(QTextCursor::End);
    m_pteOutput->ensureCursorVisible();
}


void PanelAnalysisDlg::onMessage(QString const &msg)
{
    m_pteOutput->insertPlainText(msg);
    m_pteOutput->textCursor().movePosition(QTextCursor::End);
    m_pteOutput->ensureCursorVisible();
}


void PanelAnalysisDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void PanelAnalysisDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void PanelAnalysisDlg::deleteTask()
{
    if(m_pTheTask) delete m_pTheTask;
    m_pTheTask = nullptr;
}
