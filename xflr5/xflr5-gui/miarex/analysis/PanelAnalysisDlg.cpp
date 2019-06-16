/****************************************************************************

	PanelAnalysisDlg Class
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
#include <math.h>

#include "PanelAnalysisDlg.h"
#include <analysis3d/plane_analysis/PanelAnalysis.h>
#include <analysis3d/plane_analysis/planeanalysistask.h>
#include <globals/globals.h>
#include <globals/gui_params.h>
#include <miarex/Miarex.h>
#include <miarex/objects3d.h>
#include <misc/options/units.h>
#include <misc/options/displayoptions.h>
#include <objects/objects3d/Plane.h>
#include <objects/objects3d/WPolar.h>
#include <objects/objects3d/vector3d.h>

QPoint PanelAnalysisDlg::s_Position = QPoint(200,100);
QSize  PanelAnalysisDlg::s_WindowSize = QSize(900,550);
bool PanelAnalysisDlg::s_bWindowMaximized=false;


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
	m_pctrlProgress->setValue(m_Progress);
	m_pctrlTextOutput->clear();
	m_pctrlLogFile->setChecked(Miarex::m_bLogFile);
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
//		QThreadPool::globalInstance()->waitForDone();
		done(1);
	}
}


void PanelAnalysisDlg::onLogFile()
{
	Miarex::m_bLogFile = m_pctrlLogFile->isChecked();
}


/**Updates the progress of the analysis in the slider widget */
void PanelAnalysisDlg::onProgress()
{
/*	QTime dt = QTime::currentTime();
	QString str = dt.toString("hh:mm:ss.zzz");
	qDebug() << str;*/

	m_pctrlProgress->setMaximum(m_pTheTask->m_pthePanelAnalysis->m_TotalTime);
	m_pctrlProgress->setValue(m_pTheTask->m_pthePanelAnalysis->m_Progress);
	if(m_strOut.length())
	{
		m_pctrlTextOutput->insertPlainText(m_strOut);
		m_pctrlTextOutput->textCursor().movePosition(QTextCursor::End);
		m_pctrlTextOutput->ensureCursorVisible();
		m_strOut.clear();
	}
}


void PanelAnalysisDlg::setupLayout()
{
	m_pctrlTextOutput = new QTextEdit(this);
	m_pctrlTextOutput->setReadOnly(true);
	m_pctrlTextOutput->setLineWrapMode(QTextEdit::NoWrap);
	m_pctrlTextOutput->setWordWrapMode(QTextOption::NoWrap);
	m_pctrlTextOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	m_pctrlProgress = new QProgressBar(this);
	m_pctrlProgress->setOrientation(Qt::Horizontal);
	m_pctrlProgress->setMinimum(0);
	m_pctrlProgress->setMaximum(100);
	m_pctrlProgress->setValue(0);


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
		pMainLayout->addWidget(m_pctrlProgress);
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

	qApp->processEvents();

	m_pctrlCancel->setText(tr("Cancel"));
	m_bIsFinished = false;

	m_pctrlProgress->setMaximum(100000);

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
	QFuture<void> future = QtConcurrent::run(m_pTheTask, &PlaneAnalysisTask::run);

	while(future.isRunning())
	{
		qApp->processEvents();
		QThread::msleep(200);
	}
	qApp->processEvents();
	cleanUp();
}



void PanelAnalysisDlg::cleanUp()
{
	QString strong;
	m_Timer.stop();

	//WPolar has been populated with results by the PlaneAnalysisTask
	//Store the POpps if requested
	if(PlaneOpp::s_bStoreOpps)
	{
		for(int iPOpp=0; iPOpp<m_pTheTask->m_pthePanelAnalysis->m_PlaneOppList.size(); iPOpp++)
		{
			//add the data to the polar object
			PlaneOpp *pPOpp = m_pTheTask->m_pthePanelAnalysis->m_PlaneOppList.at(iPOpp);
			if(PlaneOpp::s_bKeepOutOpps || !pPOpp->isOut())	Objects3d::insertPOpp(pPOpp);
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

		outstream << m_pctrlTextOutput->toPlainText();
		outstream << "\n";
		QDateTime dt = QDateTime::currentDateTime();
		QString str = dt.toString(Qt::DefaultLocaleLongDate);
		outstream << "Analysis ended "<<str<<"\n";
		outstream << "Elapsed: "<<(double)clock.elapsed()/1000.0<<"s";
		outstream << "\n";
		outstream.flush();
		pXFile->close();
	}
	delete pXFile;

	m_pTheTask = nullptr;

	m_pctrlCancel->setText(tr("Close"));
	m_pctrlCancel->setFocus();
}


/**
* Updates the text output in the dialog box and the log file.
*@param strong the text message to append to the output widget and to the log file.
*/
void PanelAnalysisDlg::updateOutput(QString &strong)
{
	m_pctrlTextOutput->insertPlainText(strong);
	m_pctrlTextOutput->textCursor().movePosition(QTextCursor::End);
	m_pctrlTextOutput->ensureCursorVisible();
}


void PanelAnalysisDlg::onMessage(QString msg)
{
/*	m_pctrlTextOutput->insertPlainText(msg);
	m_pctrlTextOutput->textCursor().movePosition(QTextCursor::End);
	m_pctrlTextOutput->ensureCursorVisible();*/
	m_pctrlTextOutput->insertPlainText(msg);
	m_pctrlTextOutput->textCursor().movePosition(QTextCursor::End);
	m_pctrlTextOutput->ensureCursorVisible();
}


void PanelAnalysisDlg::showEvent(QShowEvent *event)
{
    move(s_Position);
	resize(s_WindowSize);
	if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

	event->accept();
}



void PanelAnalysisDlg::hideEvent(QHideEvent *event)
{
	s_WindowSize = size();
	s_bWindowMaximized = isMaximized();
    s_Position = pos();

	event->accept();
}

void PanelAnalysisDlg::deleteTask()
{
	if(m_pTheTask) delete m_pTheTask;
	m_pTheTask = nullptr;
}
