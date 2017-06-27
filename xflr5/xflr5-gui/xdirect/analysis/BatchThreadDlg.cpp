/****************************************************************************

	BatchThreadDlg Class
	   Copyright (C) 2003-2016 Andre Deperrois adeperrois@xflr5.com

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

#include "BatchThreadDlg.h"
#include "XFoilAdvancedDlg.h"
#include "ReListDlg.h"
#include <gui_params.h>
#include "globals.h"
#include <xdirect/XDirect.h>
#include <xdirect/objects2d.h>
#include <misc/Settings.h>
#include <xinverse/FoilSelectionDlg.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QFontDatabase>
#include <QtDebug>

bool BatchThreadDlg::s_bCurrentFoil=true;
bool BatchThreadDlg::s_bUpdatePolarView = false;
void * BatchThreadDlg::s_pXDirect;
QPoint BatchThreadDlg::s_Position;
int BatchThreadDlg::s_nThreads = 1;

/**
 * The public contructor
 */
BatchThreadDlg::BatchThreadDlg(QWidget *pParent) : QDialog(pParent)
{
	QString str = tr("Multi-threaded batch analysis");
	setWindowTitle(str);

	m_pXFile = NULL;

	m_PolarType = XFOIL::FIXEDSPEEDPOLAR;

	m_FoilList.clear();

	m_Mach  = 0.0;
	m_ReMin = 100000.0;
	m_ReMax = 300000.0;
	m_ReInc =  50000.0;

	m_ClMin = 0.0;
	m_ClMax = 1.0;
	m_ClInc = 0.1;

	m_ACrit  = 9.0;
	m_XTop = 1.0;
	m_XBot = 1.0;

	m_nTaskDone = 0;
	m_nTaskStarted = 0;
	m_nAnalysis = 0;

	m_bAlpha          = true;
	m_bFromList       = false;
	m_bFromZero       = false;
	m_bInitBL         = false;
	m_bCancel         = false;

	m_bIsRunning      = false;

	XFoil::s_bCancel = false;
	XFoilTask::s_bSkipOpp = false;
	XFoilTask::s_bSkipPolar = false;

	setupLayout();
	connect(m_pctrlFoil1,           SIGNAL(clicked()),         this, SLOT(onFoilSelectionType()));
	connect(m_pctrlFoil2,           SIGNAL(clicked()),         this, SLOT(onFoilSelectionType()));
	connect(m_pctrlFoilList,        SIGNAL(clicked()),         this, SLOT(onFoilList()));
	connect(m_pctrlClose,           SIGNAL(clicked()),         this, SLOT(onClose()));
	connect(m_pctrlAnalyze,         SIGNAL(clicked()),         this, SLOT(onAnalyze()));
	connect(m_pctrlAlpha,           SIGNAL(toggled(bool)),     this, SLOT(onAcl()));
	connect(m_pctrlCl,              SIGNAL(toggled(bool)),     this, SLOT(onAcl()));
	connect(m_rbRange1,             SIGNAL(toggled(bool)),     this, SLOT(onRange()));
	connect(m_pctrlEditList,        SIGNAL(clicked()),         this, SLOT(onEditReList()));
	connect(m_pctrlFromZero,        SIGNAL(stateChanged(int)), this, SLOT(onFromZero(int)));
	connect(m_pctrlSpecMin,         SIGNAL(editingFinished()), this, SLOT(onSpecChanged()));
	connect(m_pctrlSpecMax,         SIGNAL(editingFinished()), this, SLOT(onSpecChanged()));
	connect(m_pctrlSpecDelta,       SIGNAL(editingFinished()), this, SLOT(onSpecChanged()));
	connect(m_pctrlUpdatePolarView, SIGNAL(clicked(bool)),     this, SLOT(onUpdatePolarView()));
}



/**
 * This course of action will lead us to destruction.
 */
BatchThreadDlg::~BatchThreadDlg()
{
	if(m_pXFile)     delete m_pXFile;

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
	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

	QSizePolicy szPolicyMinimum;
	szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
	szPolicyMinimum.setVerticalPolicy(QSizePolicy::MinimumExpanding);

	QSizePolicy szPolicyMaximum;
	szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
	szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);


	QVBoxLayout *pLeftSide = new QVBoxLayout;
	{
		QGroupBox *pFoilBox = new QGroupBox(tr("Foil Selection"));
		{
			QHBoxLayout *pFoilLayout = new QHBoxLayout;
			m_pctrlFoil1 = new QRadioButton(tr("Current foil only"), this);
			m_pctrlFoil2 = new QRadioButton(tr("Foil list"), this);
			m_pctrlFoilList = new QPushButton(tr("Foil list"), this);
			pFoilLayout->addWidget(m_pctrlFoil1);
			pFoilLayout->addWidget(m_pctrlFoil2);
			pFoilLayout->addStretch(1);
			pFoilLayout->addWidget(m_pctrlFoilList);
			pFoilBox->setLayout(pFoilLayout);
		}

		QGroupBox *pBatchVarsGroupBox = new QGroupBox(tr("Batch Variables"));
		{
			QGridLayout *pBatchVarsLayout = new QGridLayout;
			{
				m_rbRange1 = new QRadioButton(tr("Range"), this);
				m_rbRange2 = new QRadioButton(tr("Re List"), this);
				m_pctrlEditList = new QPushButton(tr("Edit List"));
				QLabel *MinVal   = new QLabel(tr("Min"));
				QLabel *MaxVal   = new QLabel(tr("Max"));
				QLabel *DeltaVal = new QLabel(tr("Increment"));
				MinVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
				MaxVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
				DeltaVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

				m_pctrlReType  = new QLabel("Reynolds=");
				m_pctrlMaType  = new QLabel("Mach=");
				QLabel *NCritLabel = new QLabel(tr("NCrit="));
				m_pctrlReType->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
				NCritLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
				m_pctrlMaType->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
				m_pctrlACrit   = new DoubleEdit(9.00,2, this);

				m_pctrlReMin   = new DoubleEdit(100000,0,this);
				m_pctrlReMax   = new DoubleEdit(150000,0,this);
				m_pctrlReDelta = new DoubleEdit(50000,0, this);
				m_pctrlMach    = new DoubleEdit(0.00, 3, this);

				pBatchVarsLayout->addWidget(MinVal, 2, 2);
				pBatchVarsLayout->addWidget(MaxVal, 2, 3);
				pBatchVarsLayout->addWidget(DeltaVal, 2, 4);
				pBatchVarsLayout->addWidget(m_pctrlReType, 3, 1);
				pBatchVarsLayout->addWidget(m_pctrlReMin, 3, 2);
				pBatchVarsLayout->addWidget(m_pctrlReMax, 3, 3);
				pBatchVarsLayout->addWidget(m_pctrlReDelta, 3, 4);
				pBatchVarsLayout->addWidget(m_pctrlMaType, 4, 1);
				pBatchVarsLayout->addWidget(m_pctrlMach, 4, 2);
				pBatchVarsLayout->addWidget(NCritLabel, 5,1);
				pBatchVarsLayout->addWidget(m_pctrlACrit, 5, 2);
			}

			QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
			{
				pRangeSpecLayout->addWidget(m_rbRange1);
				pRangeSpecLayout->addWidget(m_rbRange2);
				pRangeSpecLayout->addStretch(1);
				pRangeSpecLayout->addWidget(m_pctrlEditList);
			}

			QVBoxLayout *pBatchVarsGroupLayout = new QVBoxLayout;
			{
				pBatchVarsGroupLayout->addLayout(pRangeSpecLayout);
				pBatchVarsGroupLayout->addLayout(pBatchVarsLayout);
				pBatchVarsGroupBox->setLayout(pBatchVarsGroupLayout);
			}
		}

		QGroupBox *pRangeVarsGroupBox = new QGroupBox(tr("Analysis Range"));
		{
			QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
			{
				QLabel *Spec = new QLabel(tr("Specify:"));
				m_pctrlAlpha = new QRadioButton(tr("Alpha"));
				m_pctrlCl = new QRadioButton(tr("Cl"));
				m_pctrlFromZero   = new QCheckBox(tr("From Zero"));
				pRangeSpecLayout->addWidget(Spec);
				pRangeSpecLayout->addWidget(m_pctrlAlpha);
				pRangeSpecLayout->addWidget(m_pctrlCl);
				pRangeSpecLayout->addStretch(1);
				pRangeSpecLayout->addWidget(m_pctrlFromZero);
			}

			QGridLayout *pRangeVarsLayout = new QGridLayout;
			{
				QLabel *SpecMin   = new QLabel(tr("Min"));
				QLabel *SpecMax   = new QLabel(tr("Max"));
				QLabel *SpecDelta = new QLabel(tr("Increment"));
				SpecMin->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
				SpecMax->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
				SpecDelta->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
				m_pctrlSpecVar    = new QLabel(tr("Spec ="));
				m_pctrlSpecMin    = new DoubleEdit(0.00,3);
				m_pctrlSpecMax    = new DoubleEdit(1.00,3);
				m_pctrlSpecDelta  = new DoubleEdit(0.50,3);
				pRangeVarsLayout->addWidget(SpecMin, 1, 2);
				pRangeVarsLayout->addWidget(SpecMax, 1, 3);
				pRangeVarsLayout->addWidget(SpecDelta, 1, 4);
				pRangeVarsLayout->addWidget(m_pctrlSpecVar, 2, 1);
				pRangeVarsLayout->addWidget(m_pctrlSpecMin, 2, 2);
				pRangeVarsLayout->addWidget(m_pctrlSpecMax, 2, 3);
				pRangeVarsLayout->addWidget(m_pctrlSpecDelta, 2, 4);
			}

			QVBoxLayout *pRangeVarsGroupLayout = new QVBoxLayout;
			{
				pRangeVarsGroupLayout->addLayout(pRangeSpecLayout);
				pRangeVarsGroupLayout->addLayout(pRangeVarsLayout);
				pRangeVarsGroupBox->setLayout(pRangeVarsGroupLayout);
			}
		}

		QGroupBox *pTransVarsGroupBox = new QGroupBox(tr("Forced Transitions"));
		{
			QGridLayout *pTransVars = new QGridLayout;
			{
				pTransVars->setColumnStretch(0,4);
				pTransVars->setColumnStretch(1,1);
				QLabel *TopTransLabel = new QLabel(tr("Top transition location (x/c)"));
				QLabel *BotTransLabel = new QLabel(tr("Bottom transition location (x/c)"));
				TopTransLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
				BotTransLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
				m_pctrlXTopTr = new DoubleEdit(1.00);
				m_pctrlXBotTr = new DoubleEdit(1.00);

				pTransVars->addWidget(TopTransLabel, 2, 1);
				pTransVars->addWidget(m_pctrlXTopTr, 2, 2);
				pTransVars->addWidget(BotTransLabel, 3, 1);
				pTransVars->addWidget(m_pctrlXBotTr, 3, 2);
			}
			pTransVarsGroupBox->setLayout(pTransVars);
		}
		QHBoxLayout *pCommandButtons = new QHBoxLayout;
		{
			QPushButton *pAdvancedSettings =  new QPushButton(tr("Advanced Settings"));
			connect(pAdvancedSettings, SIGNAL(clicked()), this, SLOT(onAdvancedSettings()));

			m_pctrlClose     = new QPushButton(tr("Close"));
			m_pctrlAnalyze   = new QPushButton(tr("Analyze"))	;
			m_pctrlAnalyze->setAutoDefault(true);

			pCommandButtons->addStretch(1);
			pCommandButtons->addWidget(pAdvancedSettings);
			pCommandButtons->addStretch(1);
			pCommandButtons->addWidget(m_pctrlAnalyze);
			pCommandButtons->addStretch(1);
			pCommandButtons->addWidget(m_pctrlClose);
			pCommandButtons->addStretch(1);
		}
		pLeftSide->addWidget(pFoilBox);
		pLeftSide->addWidget(pBatchVarsGroupBox);
		pLeftSide->addWidget(pTransVarsGroupBox);
		pLeftSide->addWidget(pRangeVarsGroupBox);
		pLeftSide->addStretch(1);
		pLeftSide->addSpacing(20);
		pLeftSide->addLayout(pCommandButtons);
	}

	QVBoxLayout *pRightSide = new QVBoxLayout;
	{
		m_pctrlTextOutput = new QTextEdit;
		m_pctrlTextOutput->setReadOnly(true);
		m_pctrlTextOutput->setLineWrapMode(QTextEdit::NoWrap);
		m_pctrlTextOutput->setWordWrapMode(QTextOption::NoWrap);
		m_pctrlTextOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		m_pctrlTextOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

		QFontMetrics fm(Settings::s_TableFont);
		m_pctrlTextOutput->setMinimumWidth(67*fm.averageCharWidth());
		m_pctrlInitBL          = new QCheckBox(tr("Initialize BLs between polars"));

		QHBoxLayout *pOptionsLayout = new QHBoxLayout;
		{
			m_pctrlUpdatePolarView = new QCheckBox(tr("Update polar view"));
			m_pctrlUpdatePolarView->setToolTip(tr("Update the polar graphs after the completion of each foil/polar pair.\nUncheck for increased analysis speed."));
			QPushButton *pClearBtn = new QPushButton(tr("Clear Output"));
			connect(pClearBtn, SIGNAL(clicked()), m_pctrlTextOutput, SLOT(clear()));
			pOptionsLayout->addWidget(m_pctrlUpdatePolarView);
			pOptionsLayout->addStretch(1);
			pOptionsLayout->addWidget(pClearBtn);
		}
		QHBoxLayout *pnThreadLayout = new QHBoxLayout;
		{
			QLabel *label1 = new QLabel(tr("Max. Threads to use for the analysis:"));
			int maxThreads = QThread::idealThreadCount();
			m_pctrlMaxThreads = new IntEdit(std::min(s_nThreads, maxThreads));
			QLabel *label2= new QLabel(QString("/%1").arg(maxThreads));
			pnThreadLayout->addWidget(label1);
			pnThreadLayout->addWidget(m_pctrlMaxThreads);
			pnThreadLayout->addWidget(label2);
			pnThreadLayout->addStretch();
		}

		pRightSide->addWidget(m_pctrlInitBL);
		pRightSide->addLayout(pOptionsLayout);
		pRightSide->addLayout(pnThreadLayout);
		pRightSide->addWidget(m_pctrlTextOutput,1);
	}

	QHBoxLayout *pBoxesLayout = new QHBoxLayout;
	{
		pBoxesLayout->addLayout(pLeftSide);
		pBoxesLayout->addLayout(pRightSide);
	}

	setLayout(pBoxesLayout);
}


/**
 * Clean-up is performed when all the threads are terminated
 */
void BatchThreadDlg::cleanUp()
{
	if(m_pXFile->isOpen())
	{
		QTextStream out(m_pXFile);
		out<<m_pctrlTextOutput->toPlainText();
		m_pXFile->close();
	}
	m_pctrlClose->setEnabled(true);
	m_pctrlAnalyze->setText(tr("Analyze"));
	m_bIsRunning = false;
	m_bCancel    = false;
	XFoil::s_bCancel = false;
	m_pctrlClose->setFocus();

	//in case we cancelled, delete all Analysis that are left
	for(int ia=m_AnalysisPair.count()-1; ia>=0; ia--)
	{
		FoilAnalysis *pAnalysis = m_AnalysisPair.at(ia);
		delete pAnalysis;
		m_AnalysisPair.removeAt(ia);
	}
}


/**
 * Creates a polar object for a given set of specified input data
 * @param pFoil a pointer to the Foil object to which the Polar will be attached
 * @param Re  the value of the Reynolds number
 * @param Mach  the value of the Mach number
 * @param NCrit the value of the transition criterion
 * @return a pointer to the Polar object which has been created
 */
Polar * BatchThreadDlg::createPolar(Foil *pFoil, double Re, double Mach, double NCrit)
{
	if(!pFoil) return NULL;

	Polar *pNewPolar = new Polar;
	QColor clr = randomColor(!Settings::isLightTheme());
	pNewPolar->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
	pNewPolar->foilName()   = pFoil->foilName();
	pNewPolar->isVisible() = true;
	pNewPolar->polarType() = m_PolarType;

	switch (pNewPolar->polarType())
	{
	case XFOIL::FIXEDSPEEDPOLAR:
		pNewPolar->MaType() = 1;
		pNewPolar->ReType() = 1;
		break;
	case XFOIL::FIXEDLIFTPOLAR:
		pNewPolar->MaType() = 2;
		pNewPolar->ReType() = 2;
		break;
	case XFOIL::RUBBERCHORDPOLAR:
		pNewPolar->MaType() = 1;
		pNewPolar->ReType() = 3;
		break;
	case XFOIL::FIXEDAOAPOLAR:
		pNewPolar->MaType() = 1;
		pNewPolar->ReType() = 1;
		break;
	default:
		pNewPolar->ReType() = 1;
		pNewPolar->MaType() = 1;
		break;
	}
	if(m_PolarType!=XFOIL::FIXEDAOAPOLAR)  pNewPolar->Reynolds() = Re;
	else                                   pNewPolar->aoa()    = 0.0;

	pNewPolar->Mach()    = Mach;
	pNewPolar->NCrit()   = NCrit;
	pNewPolar->XtrTop()  = m_XTop;
	pNewPolar->XtrBot()  = m_XBot;


	setPlrName(pNewPolar);
	Polar *pOldPolar = Objects2D::getPolar(pFoil, pNewPolar->polarName());

	if(pOldPolar)
	{
		delete pNewPolar;
		pNewPolar = pOldPolar;
	}
	else Objects2D::addPolar(pNewPolar);
	return pNewPolar;
}


/**
 * Overrides the base class keyPressEvent method
 * @param event the keyPressEvent.
 */
void BatchThreadDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(m_pctrlClose->hasFocus())	     done(1);
			else if(m_pctrlAnalyze->hasFocus())  onAnalyze();
			else                                 m_pctrlAnalyze->setFocus();
			break;
		}
		case Qt::Key_Escape:
		{
			if(m_bIsRunning)
			{
				m_bCancel = true;
				XFoilTask::s_bCancel = true;
				XFoil::s_bCancel = true;
			}
			else
			{
				onClose(); // will close the dialog box
			}
			break;
		}
		default:
			event->ignore();
	}
	event->accept();
}


/**
 * Initializes the dialog and the GUI interface
 */
void BatchThreadDlg::initDialog()
{
	if(!QXDirect::curFoil()) return;
	blockSignals(true);

	m_pctrlTextOutput->clear();
	m_pctrlTextOutput->setFont(Settings::s_TableFont);

	m_ACrit     = QXDirect::s_refPolar.NCrit();
	m_XBot      = QXDirect::s_refPolar.XtrBot();
	m_XTop      = QXDirect::s_refPolar.XtrTop();
	m_Mach      = QXDirect::s_refPolar.Mach();

	m_PolarType = XFOIL::FIXEDSPEEDPOLAR; //no choice...

	m_pctrlFoil1->setChecked(s_bCurrentFoil);
	m_pctrlFoil2->setChecked(!s_bCurrentFoil);
	onFoilSelectionType();
	
	m_pctrlReMin->setPrecision(0);
	m_pctrlReMax->setPrecision(0);
	m_pctrlReDelta->setPrecision(0);

	m_pctrlSpecMin->setPrecision(3);
	m_pctrlSpecMax->setPrecision(3);
	m_pctrlSpecDelta->setPrecision(3);

	if(m_ReMin<=0.0) m_ReMin = qAbs(m_ReInc);
	m_pctrlReMin->setValue(m_ReMin);
	m_pctrlReMax->setValue(m_ReMax);
	m_pctrlReDelta->setValue(m_ReInc);
	m_pctrlSpecMin->setValue(m_AlphaMin);
	m_pctrlSpecMax->setValue(m_AlphaMax);
	m_pctrlSpecDelta->setValue(m_AlphaInc);

	m_pctrlMach->setValue(m_Mach);
	m_pctrlACrit->setValue(m_ACrit);
	m_pctrlXTopTr->setValue(m_XTop);
	m_pctrlXBotTr->setValue(m_XBot);

	if(m_bAlpha) m_pctrlAlpha->setChecked(true);
	else         m_pctrlCl->setChecked(m_bAlpha);
	onAcl();


	if(!m_bFromList)  m_rbRange1->setChecked(true);
	else              m_rbRange2->setChecked(true);

	m_pctrlEditList->setEnabled(m_bFromList);
	m_pctrlReMin->setEnabled(!m_bFromList);
	m_pctrlReMax->setEnabled(!m_bFromList);
	m_pctrlReDelta->setEnabled(!m_bFromList);
	m_pctrlMach->setEnabled(!m_bFromList);
	m_pctrlACrit->setEnabled(!m_bFromList);

	if(m_bFromZero)  m_pctrlFromZero->setChecked(true);
	else             m_pctrlFromZero->setChecked(false);

	m_pctrlInitBL->setChecked(true);
	m_pctrlUpdatePolarView->setChecked(s_bUpdatePolarView);
	blockSignals(false);
}


/**
 * The user has switched between aoa and lift coeficient.
 * Initializes the interface with the corresponding values.
 */
void BatchThreadDlg::onAcl()
{
	if(m_PolarType==XFOIL::FIXEDAOAPOLAR) return;
	m_bAlpha = m_pctrlAlpha->isChecked();
	if(m_bAlpha)
	{
		m_pctrlSpecVar->setText(tr("Alpha"));
		m_pctrlSpecMin->setValue(m_AlphaMin);
		m_pctrlSpecMax->setValue(m_AlphaMax);
		m_pctrlSpecDelta->setValue(m_AlphaInc);
		m_pctrlFromZero->setEnabled(true);
	}
	else
	{
		m_pctrlSpecVar->setText(tr("CL"));
		m_pctrlSpecMin->setValue(m_ClMin);
		m_pctrlSpecMax->setValue(m_ClMax);
		m_pctrlSpecDelta->setValue(m_ClInc);
		m_bFromZero = false;
		m_pctrlFromZero->setChecked(false);
		m_pctrlFromZero->setEnabled(false);
	}
}

/**
 * The user has changed the range of Re values to analyze
 */
void BatchThreadDlg::onSpecChanged()
{
	readParams();
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
		XFoil::s_bCancel = true;
		return;
	}

	m_bCancel    = false;
	m_bIsRunning = true;

	m_pctrlClose->setEnabled(false);

	QString FileName = QDir::tempPath() + "/XFLR5.log";
	m_pXFile = new QFile(FileName);
	if (!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text)) m_pXFile = NULL;

	readParams();

	setFileHeader();
	m_bInitBL = m_pctrlInitBL->isChecked();

	m_pctrlAnalyze->setFocus();
	startAnalysis();
}


/**
 * The user has requested to quit the dialog box
 */
void BatchThreadDlg::onClose()
{
	if(m_bIsRunning) return;

	m_bCancel = true;
	XFoilTask::s_bCancel = true;
	QThreadPool::globalInstance()->waitForDone();
	readParams();

	QXDirect::s_refPolar.NCrit()    = m_ACrit;
	QXDirect::s_refPolar.XtrBot()   = m_XBot;
	QXDirect::s_refPolar.XtrTop()   = m_XTop;
	QXDirect::s_refPolar.Mach()     = m_Mach;

	accept();
}



/**
 * Overrides the base class reject() method, to prevent window closure when an analysis is running.
 * If the analysis is running, cancels it and returns.
 * If not, closes the window.
 */
void BatchThreadDlg::reject()
{
	if(m_bIsRunning)
	{
		m_bCancel    = true;
		XFoil::s_bCancel = true;
	}
	else
	{
		QDialog::reject();
		//close the dialog box
	}
}



/**
 * The user has requested an edition of the Re list
*/
void BatchThreadDlg::onEditReList()
{
	ReListDlg dlg(this);
	dlg.initDialog(QXDirect::s_ReList,QXDirect::s_MachList, QXDirect::s_NCritList);

	if(QDialog::Accepted == dlg.exec())
	{
		QXDirect::s_ReList.clear();
		QXDirect::s_MachList.clear();
		QXDirect::s_NCritList.clear();

		QXDirect::s_ReList.append(dlg.m_ReList);
		QXDirect::s_MachList.append(dlg.m_MachList);
		QXDirect::s_NCritList.append(dlg.m_NCritList);
	}
}


/**
 * The user has requested an edition of the list of Foil objects to analyze
 */
void BatchThreadDlg::onFoilList()
{
	FoilSelectionDlg dlg(this);
//	dlg.SetSelectionMode(true);
	dlg.m_poaFoil = &Objects2D::s_oaFoil;

	dlg.m_FoilList.clear();
	dlg.m_FoilList.append(m_FoilList);
	dlg.initDialog();

	m_FoilList.clear();

	if(QDialog::Accepted == dlg.exec())
	{
		m_FoilList.append(dlg.m_FoilList);
	}
	outputFoilList();
}


/**
 *The user has changed between single Foil and list of Foil objects to analyze
 */
void BatchThreadDlg::onFoilSelectionType()
{
	s_bCurrentFoil = m_pctrlFoil1->isChecked();
	m_pctrlFoilList->setEnabled(!s_bCurrentFoil);

	if(s_bCurrentFoil)
	{
		m_FoilList.clear();
		m_FoilList.append(QXDirect::curFoil()->foilName());
	}

	outputFoilList();
}

/**
 * The user has clicked the "start from zero" checkbox
 * @param state the new state of the checkbox
 */
void BatchThreadDlg::onFromZero(int )
{
	if(m_pctrlFromZero->isChecked()) m_bFromZero = true;
	else                             m_bFromZero = false;
}


/**
 * The user has clicked the checkbox which specifies the initialization of the boundary layer
 **/
void BatchThreadDlg::onInitBL(int)
{
	if (m_pctrlInitBL->isChecked()) m_bInitBL = true;
	else                            m_bInitBL = false;
}

/**
 * The user has clicked the checkbox specifying which range of Re should be analyzed
 */
void BatchThreadDlg::onRange()
{
	if(m_rbRange1->isChecked())
		m_bFromList = false;
	else
		m_bFromList = true;

	m_pctrlEditList->setEnabled(m_bFromList);
	m_pctrlReMin->setEnabled(!m_bFromList);
	m_pctrlReMax->setEnabled(!m_bFromList);
	m_pctrlReDelta->setEnabled(!m_bFromList);
	m_pctrlMach->setEnabled(!m_bFromList);
	m_pctrlACrit->setEnabled(!m_bFromList);
}


/**
 * Reads the value of the input parameters from the widgets and maps the data
 */
void BatchThreadDlg::readParams()
{
	m_bAlpha = m_pctrlAlpha->isChecked();

	if(m_PolarType!=XFOIL::FIXEDAOAPOLAR)
	{
		m_ReInc = m_pctrlReDelta->value();
		m_ReMax = m_pctrlReMax->value();
		m_ReMin = m_pctrlReMin->value();

		if(m_bAlpha)
		{
			m_AlphaInc = qAbs(m_pctrlSpecDelta->value());
			m_AlphaMax = m_pctrlSpecMax->value();
			m_AlphaMin = m_pctrlSpecMin->value();
		}
		else
		{
			m_ClInc = qAbs(m_pctrlSpecDelta->value());
			m_ClMin = m_pctrlSpecMin->value();
			m_ClMax = m_pctrlSpecMax->value();
		}
	}

	if(m_ReMin<=0.0) m_ReMin = qAbs(m_ReInc);
	if(m_ReMax<=0.0) m_ReMax = qAbs(m_ReMax);

	m_Mach     = m_pctrlMach->value();
	if(m_Mach<=0.0) m_Mach = 0.0;
	m_ACrit    = m_pctrlACrit->value();
	m_XTop   = m_pctrlXTopTr->value();
	m_XBot   = m_pctrlXBotTr->value();

	s_nThreads = m_pctrlMaxThreads->value();
	s_nThreads = std::min(s_nThreads, QThread::idealThreadCount());
	m_pctrlMaxThreads->setValue(s_nThreads);
}


/**
 * Initializes the header of the log file
 */
void BatchThreadDlg::setFileHeader()
{
	if(!m_pXFile) return;
	QTextStream out(m_pXFile);

	out << "\n";
	out << VERSIONNAME;
	out << "\n";
	if(s_bCurrentFoil)
	{
		out << QXDirect::curFoil()->foilName();
		out << "\n";
	}


	QDateTime dt = QDateTime::currentDateTime();
	QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

	out << str;
	out << "\n___________________________________\n\n";

}


/**
 * Creates the polar name for the input Polar
 * @param pNewPolar a pointer to the Polar object to name
 */
void BatchThreadDlg::setPlrName(Polar *pNewPolar)
{
	if(!pNewPolar) return;
	pNewPolar->setAutoPolarName();
}


/**
 * Adds a text message to the ouput widget
 * @param str the message to output
 */
void BatchThreadDlg::updateOutput(QString &str)
{
	QString strong;
	strong.sprintf("%3d/%3d/%3d  ", m_nTaskStarted, m_nTaskDone, m_nAnalysis);
	m_pctrlTextOutput->insertPlainText(strong + str);
	m_pctrlTextOutput->ensureCursorVisible();
}



/**
 * Adds a text message to the log file
 * @param str the message to output
 */
void BatchThreadDlg::writeString(QString &strong)
{
	if(!m_pXFile || !m_pXFile->isOpen()) return;
	QTextStream ds(m_pXFile);
	ds << strong;
}


/**
 * Starts the multithreaded analysis.
 * First, creates a pool list of all (Foil, pairs) to analyze.
 * Then, starts the threads which will pick the pairs from the pool and remove them once the analayis is finished.
 */
void BatchThreadDlg::startAnalysis()
{
	Foil *pFoil;
	Polar *pPolar;
	QString strong;
	int iRe, nRe;

	if(s_bCurrentFoil)
	{
		m_FoilList.clear();
		m_FoilList.append(QXDirect::curFoil()->foilName());
	}

	if(!m_FoilList.count())
	{
		strong ="No foil defined for analysis\n\n";
		m_pctrlTextOutput->insertPlainText(strong);
		cleanUp();
		return;
	}

	m_pctrlAnalyze->setText(tr("Cancel"));


	if(!m_bFromList) nRe = (int)qAbs((m_ReMax-m_ReMin)/m_ReInc)+1;
	else             nRe = QXDirect::s_ReList.count();

//	QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms

	//build an array of all analysis pairs to run
	m_nAnalysis = 0;
	m_nTaskDone = 0;
	m_nTaskStarted = 0;

	FoilAnalysis *pAnalysis=NULL;
	for(int i=0; i<m_FoilList.count(); i++)
	{
		pFoil = Objects2D::foil(m_FoilList.at(i));
		if(pFoil)
		{
			for (iRe=0; iRe<nRe; iRe++)
			{
				pAnalysis = new FoilAnalysis;
				m_AnalysisPair.append(pAnalysis);
				pAnalysis->pFoil = pFoil;

				if(!m_bFromList) pPolar = createPolar(pFoil, m_ReMin + iRe *m_ReInc, m_Mach, m_ACrit);
				else             pPolar = createPolar(pFoil, QXDirect::s_ReList[iRe], QXDirect::s_MachList[iRe], QXDirect::s_NCritList[iRe]);
				pAnalysis->pPolar=pPolar;

				m_nAnalysis++;
			}
		}
	}
	strong = QString("Found %1 foil/polar pairs to analyze\n").arg(m_nAnalysis);
	m_pctrlTextOutput->insertPlainText(strong);

	//Start as many threads as the user has requested
//	m_nThreads = QThread::idealThreadCount();

	XFoilTask::s_bCancel = false;

	strong = QString("Starting with %1 threads\n\n").arg(s_nThreads);
	m_pctrlTextOutput->insertPlainText(strong);
	m_pctrlTextOutput->insertPlainText("\nStarted/Done/Total\n");

	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimerEvent()));
	m_pTimer->start(100);
}


/**
 * A timer event used to check at regluar intervals if any threads are inactive
 * in which case it launches a task if any are left
*/
void BatchThreadDlg::onTimerEvent()
{
	qApp->processEvents();

	QString strong;
	//time to launch another analysis, if any are left

	if(m_AnalysisPair.size()<=0 || m_bCancel)
	{
		//nothing left to launch... just wait and enjoy the show
		if(m_nTaskDone>=m_nAnalysis || m_bCancel)
		{
			QThreadPool::globalInstance()->waitForDone();

			if(m_bCancel) strong = "\n_____Analysis cancelled_____\n";
			else          strong = "\n_____Analysis completed_____\n";
			m_pctrlTextOutput->insertPlainText(strong);
			m_pctrlTextOutput->ensureCursorVisible();

			m_pTimer->stop();
			cleanUp();

			QXDirect *pXDirect = (QXDirect*)s_pXDirect;
			if(pXDirect->m_bPolarView && s_bUpdatePolarView)
			{
				pXDirect->createPolarCurves();
				pXDirect->updateView();
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
	FoilAnalysis *pAnalysis;
	QString strong;
	//  browse through the array until we find an available thread

	if(QThreadPool::globalInstance()->activeThreadCount()<s_nThreads && m_AnalysisPair.count())
	{
		XFoilTask *pXFoilTask = new XFoilTask(this);

		//take the last analysis in the array
		pAnalysis = (FoilAnalysis*)m_AnalysisPair.at(m_AnalysisPair.size()-1);

		pAnalysis->pPolar->isVisible() = true;

		//initiate the task
		if(m_bAlpha) pXFoilTask->setSequence(true,  m_AlphaMin, m_AlphaMax, m_AlphaInc);
		else         pXFoilTask->setSequence(false, m_ClMin, m_ClMax, m_ClInc);


		pXFoilTask->initializeTask(pAnalysis->pFoil, pAnalysis->pPolar, false, true, m_bInitBL, m_bFromZero);

		//launch it
		m_nTaskStarted++;
		strong = "Starting "+pAnalysis->pFoil->foilName()+" / "+pAnalysis->pPolar->polarName()+"\n";
		updateOutput(strong);
		QThreadPool::globalInstance()->start(pXFoilTask);

		//remove it from the array of pairs to analyze
		pAnalysis = m_AnalysisPair.last();
		m_AnalysisPair.removeLast();
		delete pAnalysis;
	}
}

void BatchThreadDlg::customEvent(QEvent * event)
{
	// When we get here, we've crossed the thread boundary and are now
	// executing in this widget's thread

	if(event->type() == XFOIL_END_TASK_EVENT)
	{
		handleXFoilTaskEvent(static_cast<XFoilTaskEvent *>(event));
	}
	else if(event->type() == XFOIL_END_OPP_EVENT)
	{
		XFoilOppEvent *pOppEvent = (XFoilOppEvent*)event;
		Objects2D::addOpPoint(pOppEvent->foilPtr(), pOppEvent->polarPtr(), pOppEvent->XFoilPtr(), QXDirect::s_bStoreOpp);
	}
}



void BatchThreadDlg::handleXFoilTaskEvent(const XFoilTaskEvent *event)
{
	// Now we can safely do something with our Qt objects.
	m_nTaskDone++; //one down, more to go
	QString str = "   ...Finished "+ ((Foil*)event->foilPtr())->foilName()+" / "
								   +((Polar*)event->polarPtr())->polarName()+"\n";
	updateOutput(str);


	QXDirect *pXDirect = (QXDirect*)s_pXDirect;
	if(s_bUpdatePolarView)
	{
		pXDirect->createPolarCurves();
		pXDirect->updateView();
	}
}




/**
 * Outputs the list of the Foil names selected for analysis to the output text window.
 */
void BatchThreadDlg::outputFoilList()
{
	m_pctrlTextOutput->append("Foils to analyze:");
	for(int i=0; i<m_FoilList.count();i++)
	{
		m_pctrlTextOutput->append("   "+m_FoilList.at(i));
	}
	m_pctrlTextOutput->append("\n");
}


/**
 * Outputs the list of the Re values selected for analysis to the output text window.
 */
void BatchThreadDlg::outputReList()
{
	m_pctrlTextOutput->append("Reynolds numbers to analyze:");
	if(m_bFromList)
	{
		for(int i=0; i<QXDirect::s_ReList.count(); i++)
		{
			QString strong = QString("   Re = %L1  /  Mach = %L2  /  NCrit = %L3")
								   .arg(QXDirect::s_ReList.at(i), 10,'f',0)
								   .arg(QXDirect::s_MachList.at(i), 5,'f',3)
								   .arg(QXDirect::s_NCritList.at(i), 5, 'f', 2);
			m_pctrlTextOutput->append(strong);
		}
	}
	else
	{
		for(double Re=m_ReMin; Re<m_ReMax; Re+=m_ReInc)
		{
			QString strong = QString("   Re = %L1  /  Mach = %L2  /  NCrit = %L3")
								   .arg(Re, 10,'f',0)
								   .arg(m_Mach, 5,'f',3)
								   .arg(m_ACrit, 5, 'f', 2);
			m_pctrlTextOutput->append(strong);
		}
	}
	m_pctrlTextOutput->append("");
}

/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void BatchThreadDlg::showEvent(QShowEvent *event)
{
    move(s_Position);
	event->accept();
}

/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void BatchThreadDlg::hideEvent(QHideEvent *event)
{
    s_Position = pos();
	event->accept();
}


void BatchThreadDlg::onAdvancedSettings()
{
	XFoilAdvancedDlg xfaDlg(this);
	xfaDlg.m_IterLimit   = XFoilTask::s_IterLim;
	xfaDlg.m_bAutoInitBL     = XFoilTask::s_bAutoInitBL;
	xfaDlg.m_VAccel      = XFoil::vaccel;
	xfaDlg.m_bFullReport = XFoil::s_bFullReport;
	xfaDlg.initDialog();

	if (QDialog::Accepted == xfaDlg.exec())
	{
		XFoil::vaccel             = xfaDlg.m_VAccel;
		XFoil::s_bFullReport      = xfaDlg.m_bFullReport;
		XFoilTask::s_bAutoInitBL  = xfaDlg.m_bAutoInitBL;
		XFoilTask::s_IterLim      = xfaDlg.m_IterLimit;
	}
}

void BatchThreadDlg::onUpdatePolarView()
{
	s_bUpdatePolarView = m_pctrlUpdatePolarView->isChecked();
	QXDirect *pXDirect = (QXDirect*)s_pXDirect;
	pXDirect->updateView();
}





