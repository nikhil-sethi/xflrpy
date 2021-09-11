/****************************************************************************

    XFoilAnalysisDlg Class
    Copyright (C) 2008-2019 André Deperrois

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
#include <QDateTime>
#include <QTimer>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QThread>
#include <QDebug>


#include "xfoilanalysisdlg.h"
#include <misc/options/settingswt.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflcore/gui_params.h>
#include <xflgraph/containers/graphwt.h>
#include <xflgraph/curve.h>
#include <xflgraph/graph.h>
#include <xfoil.h>


XDirect *XFoilAnalysisDlg::s_pXDirect;
QByteArray XFoilAnalysisDlg::s_Geometry;

bool XFoilAnalysisDlg::s_bSequence(false);
double XFoilAnalysisDlg::s_Alpha      = 0.0;
double XFoilAnalysisDlg::s_AlphaMax   = 1.0;
double XFoilAnalysisDlg::s_AlphaDelta = 0.5;
double XFoilAnalysisDlg::s_Cl         = 0.0;
double XFoilAnalysisDlg::s_ClMax      = 1.0;
double XFoilAnalysisDlg::s_ClDelta    = 0.1;

XFoilAnalysisDlg::XFoilAnalysisDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("XFoil Analysis"));
    setupLayout();

    m_pXFoilTask = new XFoilTask;
    m_pXFoilTask->m_pParent = this;

    m_pXFile       = nullptr;

    m_pRmsGraph = new Graph;
    m_pGraphWt->setGraph(m_pRmsGraph);

    m_pRmsGraph->setXTitle(tr("Iter"));
    m_pRmsGraph->setYTitle("");//Change from BL newton system solution

    m_pRmsGraph->setXMajGrid(true, QColor(120,120,120),2,1);
    m_pRmsGraph->setYMajGrid(true, QColor(120,120,120),2,1);

    m_pRmsGraph->setXMin(0.0);
    m_pRmsGraph->setXMax(50);
    m_pRmsGraph->setYMin(0.0);
    m_pRmsGraph->setYMax(1.0);
    m_pRmsGraph->setScaleType(1);

    m_pRmsGraph->setXTitle("abs");
    m_pRmsGraph->setYTitle("rms");

    m_pRmsGraph->setAuto(true);
    m_pRmsGraph->setMargin(40);
    m_pRmsGraph->copySettings(&Settings::s_RefGraph, false);

    m_bErrors     = false;
    m_bAlpha      = true;

    s_Alpha   = 0.0;
    s_AlphaMax   = 1.0;
    s_AlphaDelta = 0.5;
    s_Cl      = 0.0;
    s_ClMax      = 1.0;
    s_ClDelta    = 0.1;
    m_ReMin      =  10000.0;
    m_ReMax      = 100000.0;
    m_ReDelta    =  10000.0;
}

XFoilAnalysisDlg::~XFoilAnalysisDlg()
{
    //    Trace("Destroying XFoilAnalysisDlg");
    if(m_pXFoilTask) delete m_pXFoilTask;
    if(m_pXFile) delete m_pXFile;
    if(m_pRmsGraph) delete m_pRmsGraph;
}

void XFoilAnalysisDlg::setupLayout()
{
    m_pteTextOutput = new QTextEdit;
    m_pteTextOutput->setReadOnly(true);
    m_pteTextOutput->setLineWrapMode(QTextEdit::NoWrap);
    m_pteTextOutput->setWordWrapMode(QTextOption::NoWrap);
    m_pteTextOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_pGraphWt = new GraphWt;
    m_pGraphWt->setMinimumHeight(350);
    m_pGraphWt->setMinimumWidth(600);
    m_pGraphWt->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    {
        m_pchLogFile = new QCheckBox(tr("Keep this window opened on errors"));
        m_ppbSkip   = new QPushButton(tr("Skip"));
        m_pButtonBox->addButton(m_pchLogFile, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbSkip, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pteTextOutput);
        pMainLayout->addWidget(m_pGraphWt);
        pMainLayout->addWidget(m_pButtonBox);
        setLayout(pMainLayout);
    }
}


void XFoilAnalysisDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)
        onCancelAnalysis();
    else if (m_ppbSkip == pButton)
        onSkipPoint();
}


void XFoilAnalysisDlg::initDialog()
{
    m_pchLogFile->setChecked(XDirect::s_bKeepOpenErrors);

    QString FileName = QDir::tempPath() + "/XFLR5.log";
    m_pXFile = new QFile(FileName);
    if (!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        delete m_pXFile;
        m_pXFile = nullptr;
    }

    m_pXFoilTask->m_OutStream.setDevice(m_pXFile);

    if(m_bAlpha)
    {
        if(s_bSequence) m_pXFoilTask->setSequence(true,  s_Alpha, s_AlphaMax, s_AlphaDelta);
        else            m_pXFoilTask->setSequence(true,  s_Alpha, s_Alpha, s_AlphaDelta);
    }
    else
    {
        if(s_bSequence) m_pXFoilTask->setSequence(false, s_Cl, s_ClMax, s_ClDelta);
        else            m_pXFoilTask->setSequence(false, s_Cl, s_Cl, s_ClDelta);
    }

    m_pXFoilTask->setReRange(m_ReMin, m_ReMax, m_ReDelta);
    m_pXFoilTask->initializeXFoilTask(XDirect::curFoil(), XDirect::curPolar(),
                                      XDirect::s_bViscous, XDirect::s_bInitBL, false);

    setFileHeader();

    QString str;
    m_pRmsGraph->deleteCurves();
    Curve *pCurve0 = m_pRmsGraph->addCurve();
    Curve *pCurve1 = m_pRmsGraph->addCurve();

    str = "rms";
    pCurve0->setName(str);
    str = "max";
    pCurve1->setName(str);
    pCurve1->setStipple(0);

    m_pRmsGraph->setXMin(0.0);
    m_pRmsGraph->setXMax(double(XFoilTask::s_IterLim));
    m_pRmsGraph->setX0(0.0);
    m_pRmsGraph->setXUnit(int(XFoilTask::s_IterLim/5.0));

    m_pRmsGraph->setY0(0.0);
    m_pRmsGraph->setYMin(0.0);
    m_pRmsGraph->setYMax(1.0);

//    m_pXFoilTask->setGraphPointers(&pCurve0->m_x, &pCurve0->m_y, &pCurve1->m_x, &pCurve1->m_y);

    m_pteTextOutput->clear();
}


void XFoilAnalysisDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void XFoilAnalysisDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void XFoilAnalysisDlg::onCancelAnalysis()
{
    XFoil::setCancel(true);
    XFoilTask::s_bCancel = true;

    if(m_pXFoilTask->isFinished()) reject();
}



void XFoilAnalysisDlg::reject()
{
    if(!m_pXFoilTask->isFinished())
    {
        XFoil::setCancel(true);
        XFoilTask::s_bCancel = true;
        return;
    }

    XFoilTask::s_bCancel = true;
    XFoil::setCancel(true);
    if(m_pXFile)
    {
        m_pXFoilTask->m_OutStream.flush();
        m_pXFile->close();
    }

    QDialog::reject();
}


void XFoilAnalysisDlg::accept()
{
    XFoilTask::s_bCancel = true;
    XFoil::setCancel(true);
    if(m_pXFile)
    {
        m_pXFoilTask->m_OutStream.flush();
        m_pXFile->close();
    }

    QDialog::accept();
}


void XFoilAnalysisDlg::onLogFile()
{
    XDirect::s_bKeepOpenErrors = m_pchLogFile->isChecked();
}


void XFoilAnalysisDlg::onSkipPoint()
{
    XFoilTask::s_bSkipOpp= true;
}


void XFoilAnalysisDlg::resetCurves()
{
    Curve*pCurve;
    pCurve = m_pRmsGraph->curve(0);
    if(pCurve) pCurve->clear();
    pCurve = m_pRmsGraph->curve(1);
    if(pCurve) pCurve->clear();
}


void XFoilAnalysisDlg::setAlpha(double AlphaMin, double AlphaMax, double AlphaDelta)
{
    s_Alpha = AlphaMin;
    s_AlphaMax = AlphaMax;
    s_AlphaDelta = AlphaDelta;
}


void XFoilAnalysisDlg::setCl(double ClMin, double ClMax, double DeltaCl)
{
    s_Cl = ClMin;
    s_ClMax = ClMax;
    s_ClDelta = DeltaCl;
}


void XFoilAnalysisDlg::setRe(double ReMin, double ReMax, double DeltaRe)
{
    m_ReMin = ReMin;
    m_ReMax = ReMax;
    m_ReDelta = DeltaRe;
}


void XFoilAnalysisDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << VERSIONNAME;
    out << "\n";
    out << XDirect::curFoil()->name();
    out << "\n";
    if(XDirect::curPolar())
    {
        out << XDirect::curPolar()->polarName();
        out << "\n";
    }

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


void XFoilAnalysisDlg::analyze()
{
    m_pButtonBox->button(QDialogButtonBox::Close)->setText(tr("Cancel"));
    m_ppbSkip->setEnabled(true);

    //all set to launch the analysis

    //create a timer to update the output at regular intervals
    QTimer *pTimer = new QTimer;
    connect(pTimer, SIGNAL(timeout()), this, SLOT(onProgress()));
    pTimer->setInterval(XDirect::s_TimeUpdateInterval);
    pTimer->start();

    //Launch the task
    m_pXFoilTask->run();

    pTimer->stop();
    delete pTimer;

    onProgress();
    m_pXFoilTask->m_OutStream.flush();

    m_bErrors = m_pXFoilTask->m_bErrors;
    if(m_bErrors)
    {
        m_pteTextOutput->insertPlainText(tr(" ...some points are unconverged"));
        m_pteTextOutput->ensureCursorVisible();
    }

    m_pButtonBox->button(QDialogButtonBox::Close)->setText(tr("Close"));
    m_ppbSkip->setEnabled(false);
    update();

    qApp->processEvents(); // make sure that all custom events are received before exiting the dialog
}


void XFoilAnalysisDlg::onProgress()
{
    if(m_pXFoilTask)
    {
        if(m_pXFoilTask->m_OutMessage.length())
        {
            m_pteTextOutput->insertPlainText(m_pXFoilTask->m_OutMessage);
            m_pteTextOutput->ensureCursorVisible();
        }
        m_pXFoilTask->m_OutMessage.clear();

        repaint(); // not recommended

        s_pXDirect->createPolarCurves();
        s_pXDirect->updateView();
    }
}


void XFoilAnalysisDlg::customEvent(QEvent * pEvent)
{
    if(pEvent->type() == XFOIL_END_TASK_EVENT)
    {
    }
    else if(pEvent->type() == XFOIL_ITER_EVENT)
    {
        XFoilIterEvent *pIterEvent = dynamic_cast<XFoilIterEvent*>(pEvent);
        Curve *pCurve0 = m_pRmsGraph->curve(0);
        Curve *pCurve1 = m_pRmsGraph->curve(1);
        if(pCurve0) pCurve0->appendPoint(pIterEvent->m_x0, pIterEvent->m_y0);
        if(pCurve1) pCurve1->appendPoint(pIterEvent->m_x1, pIterEvent->m_y1);
        m_pGraphWt->update();
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
        if(OpPoint::bStoreOpp())  Objects2d::insertOpPoint(pOppEvent->theOpPoint());
        else                      delete pOppEvent->theOpPoint();

        m_pRmsGraph->resetYLimits();
        Curve *pCurve0 = m_pRmsGraph->curve(0);
        Curve *pCurve1 = m_pRmsGraph->curve(1);
        if(pCurve0) pCurve0->clear();
        if(pCurve1) pCurve1->clear();
        update();
    }
}


void XFoilAnalysisDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilAnalysisDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        s_bSequence  = settings.value("Sequence", false).toBool();
        s_Alpha      = settings.value("AlphaMin",   s_Alpha).toDouble();
        s_AlphaMax   = settings.value("AlphaMax",   s_AlphaMax).toDouble();
        s_AlphaDelta = settings.value("AlphaDelta", s_AlphaDelta).toDouble();
        s_Cl         = settings.value("ClMin",      s_Cl).toDouble();
        s_ClMax      = settings.value("ClMax",      s_ClMax).toDouble();
        s_ClDelta    = settings.value("ClDelta",    s_ClDelta).toDouble();
    }
    settings.endGroup();
}



void XFoilAnalysisDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilAnalysisDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);

        settings.setValue("Sequence",   s_bSequence);
        settings.setValue("AlphaMin",   s_Alpha);
        settings.setValue("AlphaMax",   s_AlphaMax);
        settings.setValue("AlphaDelta", s_AlphaDelta);
        settings.setValue("ClMin",      s_Cl);
        settings.setValue("ClMax",      s_ClMax);
        settings.setValue("ClDelta",    s_ClDelta);
    }
    settings.endGroup();
}



