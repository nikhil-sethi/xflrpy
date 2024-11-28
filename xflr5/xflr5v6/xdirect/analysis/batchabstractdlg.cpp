/****************************************************************************

    BatchAbstractDlg Class
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
#include <QHeaderView>
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
#include <QMenu>
#include <QModelIndex>

#include "batchabstractdlg.h"

#include <xdirect/analysis/xfoiladvanceddlg.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/xdirect.h>
#include <xflcore/displayoptions.h>
#include <xflcore/gui_params.h>
#include <xflcore/xflcore.h>
#include <xflcore/xflevents.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects2d/polar.h>
#include <xflwidgets/customwts/actiondelegate.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>

bool BatchAbstractDlg::s_bAlpha    = true;
bool BatchAbstractDlg::s_bFromZero = true;

double BatchAbstractDlg::s_AlphaMin  = 0.0;
double BatchAbstractDlg::s_AlphaMax  = 1.0;
double BatchAbstractDlg::s_AlphaInc  = 0.5;
double BatchAbstractDlg::s_ClMin     = 0.0;
double BatchAbstractDlg::s_ClMax     = 1.0;
double BatchAbstractDlg::s_ClInc     = 0.1;

bool BatchAbstractDlg::s_bInitBL   = false;

xfl::enumPolarType BatchAbstractDlg::s_PolarType = xfl::FIXEDSPEEDPOLAR;

double BatchAbstractDlg::s_XTop   = 1.0;
double BatchAbstractDlg::s_XBot   = 1.0;

bool BatchAbstractDlg::s_bUpdatePolarView = false;
XDirect * BatchAbstractDlg::s_pXDirect;

int BatchAbstractDlg::s_nThreads = 1;


QVector<bool> BatchAbstractDlg::s_ActiveList;
QVector<double> BatchAbstractDlg::s_ReList;
QVector<double> BatchAbstractDlg::s_MachList;
QVector<double> BatchAbstractDlg::s_NCritList;

QByteArray BatchAbstractDlg::s_Geometry;
QByteArray BatchAbstractDlg::s_HSplitterSizes;
QByteArray BatchAbstractDlg::s_VSplitterSizes;

/**
 * The public contructor
 */
BatchAbstractDlg::BatchAbstractDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    m_pXFile = nullptr;
    m_pFoil = nullptr;

    m_bCancel         = false;
    m_bIsRunning      = false;

    XFoil::setCancel(false);
    XFoilTask::s_bSkipOpp = false;
    XFoilTask::s_bSkipPolar = false;

    makeCommonWidgets();
}


/**
 * This course of action will lead us to destruction.
 */
BatchAbstractDlg::~BatchAbstractDlg()
{
    if(m_pXFile)  delete m_pXFile;
    m_pXFile = nullptr;
}


/**
 * Sets up the GUI
 */
void BatchAbstractDlg::makeCommonWidgets()
{
    m_pHSplitter = nullptr; // defined in the subclasses
    m_pVSplitter = new QSplitter(Qt::Vertical);
    {
        m_plwNameList = new QListWidget;
        m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);

        m_pcptReTable = new CPTableView(this);
        m_pcptReTable->setEditable(true);
        m_pcptReTable->setEditTriggers(QAbstractItemView::CurrentChanged |
                                       QAbstractItemView::DoubleClicked |
                                       QAbstractItemView::SelectedClicked |
                                       QAbstractItemView::EditKeyPressed |
                                       QAbstractItemView::AnyKeyPressed);
        m_pReModel = new ActionItemModel(this);
        m_pReModel->setRowCount(5);//temporary
        m_pReModel->setColumnCount(5);
        m_pReModel->setActionColumn(4);
        m_pReModel->setHeaderData(0, Qt::Horizontal, QString());
        m_pReModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Re"));
        m_pReModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Mach"));
        m_pReModel->setHeaderData(3, Qt::Horizontal, QObject::tr("NCrit"));
        m_pReModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Actions"));

        m_pcptReTable->setModel(m_pReModel);

        int n = m_pReModel->actionColumn();
        QHeaderView *pHHeader = m_pcptReTable->horizontalHeader();
        pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
        pHHeader->resizeSection(n, 1);

        m_pFloatDelegate = new ActionDelegate(this);
        m_pFloatDelegate->setCheckColumn(0);
        m_pFloatDelegate->setActionColumn(4);
        QVector<int>m_Precision = {0,2,2,2,2};
        m_pFloatDelegate->setDigits(m_Precision);
        m_pcptReTable->setItemDelegate(m_pFloatDelegate);

        m_pInsertBeforeAct	= new QAction(tr("Insert before"), this);
        m_pInsertAfterAct	= new QAction(tr("Insert after"), this);
        m_pDeleteAct	    = new QAction(tr("Delete"), this);

        m_pVSplitter->addWidget(m_plwNameList);
        m_pVSplitter->addWidget(m_pcptReTable);
    }

    m_pgbPolarType = new QGroupBox(tr("Polar type"));
    {
        QHBoxLayout *pPolarTypeLayout =new QHBoxLayout;
        {
            m_prbT1 = new QRadioButton("T1");
            m_prbT1->setToolTip("Fixed speed polar");
            m_prbT2 = new QRadioButton("T2");
            m_prbT2->setToolTip("Fixed lift polar");
            m_prbT3 = new QRadioButton("T3");
            m_prbT3->setToolTip("Rubber chord polar");
            pPolarTypeLayout->addStretch();
            pPolarTypeLayout->addWidget(m_prbT1);
            pPolarTypeLayout->addStretch();
            pPolarTypeLayout->addWidget(m_prbT2);
            pPolarTypeLayout->addStretch();
            pPolarTypeLayout->addWidget(m_prbT3);
            pPolarTypeLayout->addStretch();
        }
        m_pgbPolarType->setLayout(pPolarTypeLayout);
    }

    m_pgbTransVars = new QGroupBox(tr("Forced Transitions"));
    {
        QGridLayout *pTransVars = new QGridLayout;
        {
            pTransVars->setColumnStretch(0,4);
            pTransVars->setColumnStretch(1,1);
            QLabel *TopTransLabel = new QLabel(tr("Top transition location (x/c)"));
            QLabel *BotTransLabel = new QLabel(tr("Bottom transition location (x/c)"));
            TopTransLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            BotTransLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            m_pdeXTopTr = new DoubleEdit(1.00);
            m_pdeXBotTr = new DoubleEdit(1.00);

            pTransVars->addWidget(TopTransLabel, 2, 1);
            pTransVars->addWidget(m_pdeXTopTr, 2, 2);
            pTransVars->addWidget(BotTransLabel, 3, 1);
            pTransVars->addWidget(m_pdeXBotTr, 3, 2);
        }
        m_pgbTransVars->setLayout(pTransVars);
    }


    m_pgbRangeVars = new QGroupBox(tr("Analysis Range"));
    {
        QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
        {
            QLabel *Spec = new QLabel(tr("Specify:"));
            m_prbAlpha = new QRadioButton(QChar(0x03B1));
            m_prbCl = new QRadioButton(tr("Cl"));
            m_pchFromZero   = new QCheckBox(tr("From Zero"));
            pRangeSpecLayout->addWidget(Spec);
            pRangeSpecLayout->addWidget(m_prbAlpha);
            pRangeSpecLayout->addWidget(m_prbCl);
            pRangeSpecLayout->addStretch(1);
            pRangeSpecLayout->addWidget(m_pchFromZero);
        }

        QGridLayout *pRangeVarsLayout = new QGridLayout;
        {
            QLabel *SpecMin   = new QLabel(tr("Min"));
            QLabel *SpecMax   = new QLabel(tr("Max"));
            QLabel *SpecDelta = new QLabel(tr("Increment"));
            SpecMin->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            SpecMax->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            SpecDelta->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            m_plabSpecVar    = new QLabel(tr("Spec ="));
            m_pdeSpecMin    = new DoubleEdit(0.00,3);
            m_pdeSpecMax    = new DoubleEdit(1.00,3);
            m_pdeSpecDelta  = new DoubleEdit(0.50,3);
            pRangeVarsLayout->addWidget(SpecMin, 1, 2);
            pRangeVarsLayout->addWidget(SpecMax, 1, 3);
            pRangeVarsLayout->addWidget(SpecDelta, 1, 4);
            pRangeVarsLayout->addWidget(m_plabSpecVar, 2, 1);
            pRangeVarsLayout->addWidget(m_pdeSpecMin, 2, 2);
            pRangeVarsLayout->addWidget(m_pdeSpecMax, 2, 3);
            pRangeVarsLayout->addWidget(m_pdeSpecDelta, 2, 4);
        }

        QVBoxLayout *pRangeVarsGroupLayout = new QVBoxLayout;
        {
            pRangeVarsGroupLayout->addLayout(pRangeSpecLayout);
            pRangeVarsGroupLayout->addLayout(pRangeVarsLayout);
            m_pgbRangeVars->setLayout(pRangeVarsGroupLayout);
        }
    }

    m_ppto = new PlainTextOutput;
    QFontMetrics fm(DisplayOptions::tableFont());
    m_ppto->setMinimumWidth(67*fm.averageCharWidth());

    m_pfrOptions = new QFrame;
    {
        QGridLayout *pOptionsLayout = new QGridLayout;
        {
            m_pchInitBL          = new QCheckBox(tr("Initialize BLs between polars"));
            m_pchStoreOpp        = new QCheckBox(tr("Store operating points"));
            QLabel *pLab1 = new QLabel(tr("Max. Threads to use for the analysis:"));
            int maxThreads = QThread::idealThreadCount();
            m_pieMaxThreads = new IntEdit(std::min(s_nThreads, maxThreads));
            QLabel *pLab2 = new QLabel(QString("/%1").arg(maxThreads));

            m_pchUpdatePolarView = new QCheckBox(tr("Update polar view"));
            m_pchUpdatePolarView->setToolTip(tr("Update the polar graphs after the completion of each foil/polar pair.\nUncheck for increased analysis speed."));

            pOptionsLayout->addWidget(m_pchInitBL,          1, 1);
            pOptionsLayout->addWidget(m_pchStoreOpp,        2, 1);
            pOptionsLayout->addWidget(m_pchUpdatePolarView, 3, 1);
            pOptionsLayout->addWidget(pLab1,                4, 1);
            pOptionsLayout->addWidget(m_pieMaxThreads,      4, 2);
            pOptionsLayout->addWidget(pLab2,                4, 3);
            pOptionsLayout->setColumnStretch(4,1);
        }
        m_pfrOptions->setLayout(pOptionsLayout);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    {
        m_ppbAdvancedSettings =  new QPushButton(tr("Advanced Settings"));
        m_pButtonBox->addButton(m_ppbAdvancedSettings, QDialogButtonBox::ActionRole);

        QPushButton *ppbClearBtn = new QPushButton(tr("Clear Output"));
        connect(ppbClearBtn, SIGNAL(clicked()), m_ppto, SLOT(clear()));
        m_pButtonBox->addButton(ppbClearBtn, QDialogButtonBox::ActionRole);

        m_ppbAnalyze   = new QPushButton(tr("Analyze"));
        m_pButtonBox->addButton(m_ppbAnalyze, QDialogButtonBox::ActionRole);

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void BatchAbstractDlg::connectBaseSignals()
{
    connect(m_prbAlpha,           SIGNAL(toggled(bool)),     SLOT(onAcl()));
    connect(m_prbCl,              SIGNAL(toggled(bool)),     SLOT(onAcl()));
    connect(m_pchUpdatePolarView, SIGNAL(clicked(bool)),     SLOT(onUpdatePolarView()));

    connect(m_pcptReTable,        SIGNAL(clicked(QModelIndex)),                 SLOT(onReTableClicked(QModelIndex)));
    connect(m_pReModel,           SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onCellChanged(QModelIndex,QModelIndex)));
//    connect(m_pFloatDelegate,     SIGNAL(closeEditor(QWidget*)),                SLOT(onCellChanged()));
    connect(m_pDeleteAct,         SIGNAL(triggered(bool)),                      SLOT(onDelete()));
    connect(m_pInsertBeforeAct,   SIGNAL(triggered(bool)),                      SLOT(onInsertBefore()));
    connect(m_pInsertAfterAct,    SIGNAL(triggered(bool)),                      SLOT(onInsertAfter()));
}


void BatchAbstractDlg::onButton(QAbstractButton *pButton)
{
    if      (pButton == m_pButtonBox->button(QDialogButtonBox::Close)) onClose();
    else if (pButton == m_ppbAnalyze)                                  onAnalyze();
    else if (pButton == m_ppbAdvancedSettings)                         onAdvancedSettings();
}


/**
 * Clean-up is performed when all the threads have finished
 */
void BatchAbstractDlg::cleanUp()
{
    if(m_pXFile->isOpen())
    {
        QTextStream out(m_pXFile);
        out<<m_ppto->toPlainText();
        m_pXFile->close();
    }
    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
    m_ppbAnalyze->setText(tr("Analyze"));
    m_bIsRunning = false;
    m_bCancel    = false;
    XFoil::setCancel(false);
    m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();
    qApp->restoreOverrideCursor();
}


/**
 * Overrides the base class keyPressEvent method
 * @param event the keyPressEvent.
 */
void BatchAbstractDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(m_pButtonBox->button(QDialogButtonBox::Close)->hasFocus())   done(1);
            else if(m_ppbAnalyze->hasFocus())  onAnalyze();
            else                               m_ppbAnalyze->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_bIsRunning)
            {
                m_bCancel = true;
                XFoilTask::s_bCancel = true;
                XFoil::setCancel(true);
            }
            else
            {
                onClose(); // will close the dialog box
            }
            break;
        }
        default:
            pEvent->ignore();
    }
    pEvent->accept();
}


/**
 * Initializes the dialog and the GUI interface
 */
void BatchAbstractDlg::initDialog()
{
    if(!XDirect::curFoil()) return;

    blockSignals(true);

    if(s_ActiveList.size()==0 || s_ReList.size()==0 || s_MachList.size()==0 || s_NCritList.size()==0)
        initReList();


    for(int i=0; i<Objects2d::foilCount(); i++)
    {
        Foil *pFoil = Objects2d::foilAt(i);
        if(pFoil)
        {
            m_plwNameList->addItem(pFoil->name());
            if(m_pFoil==pFoil)
            {
                QListWidgetItem *pItem =  m_plwNameList->item(i);
                pItem->setSelected(true);
            }
        }
    }

    m_ppto->clear();
    m_ppto->setFont(DisplayOptions::tableFont());

//    s_PolarType = xfl::FIXEDSPEEDPOLAR; //no choice...
    switch (s_PolarType)
    {
        default:
        case xfl::FIXEDSPEEDPOLAR:    m_prbT1->setChecked(true);   break;
        case xfl::FIXEDLIFTPOLAR:     m_prbT2->setChecked(true);   break;
        case xfl::RUBBERCHORDPOLAR:   m_prbT3->setChecked(true);   break;
    }

/*    m_pdeSpecMin->setDigits(3);
    m_pdeSpecMax->setDigits(3);
    m_pdeSpecDelta->setDigits(3); */

    m_pdeSpecMin->setValue(s_AlphaMin);
    m_pdeSpecMax->setValue(s_AlphaMax);
    m_pdeSpecDelta->setValue(s_AlphaInc);

    m_pdeXTopTr->setValue(s_XTop);
    m_pdeXBotTr->setValue(s_XBot);

    if(s_bAlpha) m_prbAlpha->setChecked(true);
    else         m_prbCl->setChecked(true);
    onAcl();

    m_pchFromZero->setChecked(s_bFromZero);

    m_pchInitBL->setChecked(true);
    m_pchStoreOpp->setChecked(OpPoint::bStoreOpp());
    m_pchUpdatePolarView->setChecked(s_bUpdatePolarView);

    fillReModel();

    blockSignals(false);
}


/**
 * The user has switched between aoa and lift coeficient.
 * Initializes the interface with the corresponding values.
 */
void BatchAbstractDlg::onAcl()
{
    if(s_PolarType==xfl::FIXEDAOAPOLAR) return;
    s_bAlpha = m_prbAlpha->isChecked();
    if(s_bAlpha)
    {
        m_plabSpecVar->setText(tr("Alpha"));
        m_pdeSpecMin->setValue(s_AlphaMin);
        m_pdeSpecMax->setValue(s_AlphaMax);
        m_pdeSpecDelta->setValue(s_AlphaInc);
        m_pchFromZero->setEnabled(true);
    }
    else
    {
        m_plabSpecVar->setText(tr("CL"));
        m_pdeSpecMin->setValue(s_ClMin);
        m_pdeSpecMax->setValue(s_ClMax);
        m_pdeSpecDelta->setValue(s_ClInc);
        s_bFromZero = false;
        m_pchFromZero->setChecked(false);
        m_pchFromZero->setEnabled(false);
    }
}

/**
 * The user has changed the range of Re values to analyze
 */
void BatchAbstractDlg::onSpecChanged()
{
    readParams();
}


/**
 * The user has requested to quit the dialog box
 */
void BatchAbstractDlg::onClose()
{
    if(m_bIsRunning) return;

    m_bCancel = true;
    XFoilTask::s_bCancel = true;
    QThreadPool::globalInstance()->waitForDone();

    // leave things as they were
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    readParams();

    accept();
}


/**
 * Overrides the base class reject() method, to prevent window closure when an analysis is running.
 * If the analysis is running, cancels it and returns.
 * If not, closes the window.
 */
void BatchAbstractDlg::reject()
{
    if(m_bIsRunning)
    {
        m_bCancel    = true;
        XFoil::setCancel(true);
    }
    else
    {
        QDialog::reject();
        //close the dialog box
    }
}


/**
 * The user has clicked the checkbox which specifies the initialization of the boundary layer
 **/
void BatchAbstractDlg::onInitBL(int)
{
    s_bInitBL = m_pchInitBL->isChecked();
}


/**
 * Reads the value of the input parameters from the widgets and maps the data
 */
void BatchAbstractDlg::readParams()
{
    s_bAlpha = m_prbAlpha->isChecked();

    if     (m_prbT2->isChecked()) s_PolarType = xfl::FIXEDLIFTPOLAR;
    else if(m_prbT3->isChecked()) s_PolarType = xfl::RUBBERCHORDPOLAR;
    else                          s_PolarType = xfl::FIXEDSPEEDPOLAR;


    if(s_bAlpha)
    {
        s_AlphaInc = qAbs(m_pdeSpecDelta->value());
        s_AlphaMax = m_pdeSpecMax->value();
        s_AlphaMin = m_pdeSpecMin->value();
    }
    else
    {
        s_ClInc = qAbs(m_pdeSpecDelta->value());
        s_ClMin = m_pdeSpecMin->value();
        s_ClMax = m_pdeSpecMax->value();
    }


    s_XTop   = m_pdeXTopTr->value();
    s_XBot   = m_pdeXBotTr->value();

    s_nThreads = m_pieMaxThreads->value();
    s_nThreads = std::min(s_nThreads, QThread::idealThreadCount());
    m_pieMaxThreads->setValue(s_nThreads);

    s_bInitBL = m_pchInitBL->isChecked();
    s_bFromZero = m_pchFromZero->isChecked();
    OpPoint::setStoreOpp(m_pchStoreOpp->isChecked());
}


/**
 * Initializes the header of the log file
 */
void BatchAbstractDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << xfl::versionName();
    out << "\n";

    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


/**
 * Adds a text message to the log file
 * @param str the message to output
 */
void BatchAbstractDlg::writeString(QString &strong)
{
    if(!m_pXFile || !m_pXFile->isOpen()) return;
    QTextStream ds(m_pXFile);
    ds << strong;
}


/**
 * Outputs the list of the Re values selected for analysis to the output text window.
 */
void BatchAbstractDlg::outputReList()
{
    m_ppto->appendPlainText(tr("Reynolds numbers to analyze:")+"\n");

    for(int i=0; i<s_ReList.count(); i++)
    {
        if(s_ActiveList.at(i))
        {
            QString strong = QString("   Re = %L1  /  Mach = %L2  /  NCrit = %L3")
                    .arg(s_ReList.at(i), 10,'f',0)
                    .arg(s_MachList.at(i), 5,'f',3)
                    .arg(s_NCritList.at(i), 5, 'f', 2);
            m_ppto->appendPlainText(strong+"\n");
        }
    }

    m_ppto->appendPlainText("\n");
}


void BatchAbstractDlg::onResizeColumns()
{
    double w = double(m_pcptReTable->width())*.93;
    int wCols  = int(w/10);
    m_pcptReTable->setColumnWidth(0, wCols);
    m_pcptReTable->setColumnWidth(1, 2*wCols);
    m_pcptReTable->setColumnWidth(2, 2*wCols);
    m_pcptReTable->setColumnWidth(3, 2*wCols);
    m_pcptReTable->setColumnWidth(4, 2*wCols);
    update();
}


void BatchAbstractDlg::resizeEvent(QResizeEvent*)
{
    onResizeColumns();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void BatchAbstractDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);

    if(m_pHSplitter) m_pHSplitter->restoreState(s_HSplitterSizes);
    m_pVSplitter->restoreState(s_VSplitterSizes);

    onResizeColumns();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void BatchAbstractDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
    if(m_pHSplitter) s_HSplitterSizes = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();
}


void BatchAbstractDlg::onAdvancedSettings()
{
    XFoilAdvancedDlg xfaDlg(this);
    xfaDlg.m_IterLimit   = XFoilTask::s_IterLim;
    xfaDlg.m_bAutoInitBL = XFoilTask::s_bAutoInitBL;
    xfaDlg.m_VAccel      = XFoil::VAccel();
    xfaDlg.m_bFullReport = XFoil::fullReport();
    xfaDlg.initDialog();

    if (QDialog::Accepted == xfaDlg.exec())
    {
        XFoil::setVAccel(xfaDlg.m_VAccel);
        XFoil::setFullReport(xfaDlg.m_bFullReport);
        XFoilTask::s_bAutoInitBL  = xfaDlg.m_bAutoInitBL;
        XFoilTask::s_IterLim      = xfaDlg.m_IterLimit;
    }
}


void BatchAbstractDlg::onUpdatePolarView()
{
    s_bUpdatePolarView = m_pchUpdatePolarView->isChecked();
    s_pXDirect->updateView();
}


void BatchAbstractDlg::initReList()
{
    s_ActiveList.resize(12);
    s_ReList.resize(12);
    s_MachList.resize(12);
    s_NCritList.resize(12);

    s_ActiveList.fill(true);

    s_ReList[0]  =   30000.0;
    s_ReList[1]  =   40000.0;
    s_ReList[2]  =   60000.0;
    s_ReList[3]  =   80000.0;
    s_ReList[4]  =  100000.0;
    s_ReList[5]  =  130000.0;
    s_ReList[6]  =  160000.0;
    s_ReList[7]  =  200000.0;
    s_ReList[8]  =  300000.0;
    s_ReList[9]  =  500000.0;
    s_ReList[10] = 1000000.0;
    s_ReList[11] = 3000000.0;

    s_MachList.fill(0);
    s_NCritList.fill(9);
}


void BatchAbstractDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("BatchAbstractDlg");
    {
        int iType   = settings.value("PolarType",    1).toInt();
        if     (iType==2)  s_PolarType = xfl::FIXEDLIFTPOLAR;
        else if(iType==3)  s_PolarType = xfl::RUBBERCHORDPOLAR;
        else               s_PolarType = xfl::FIXEDSPEEDPOLAR;

        s_bInitBL   = settings.value("bInitBL",      s_bInitBL).toBool();

        s_bAlpha    = settings.value("bAlpha",       s_bAlpha).toBool();
        s_bFromZero = settings.value("bFromZero",    s_bFromZero).toBool();

        s_XTop      = settings.value("XTrTop",       s_XTop).toDouble();
        s_XBot      = settings.value("XTrBot",       s_XBot).toDouble();

        s_AlphaMin  = settings.value("AlphaMin",     s_AlphaMin).toDouble();
        s_AlphaMax  = settings.value("AlphaMax",     s_AlphaMax).toDouble();
        s_AlphaInc  = settings.value("AlphaDelta",   s_AlphaInc).toDouble();
        s_ClMin     = settings.value("ClMin",        s_ClMin).toDouble();
        s_ClMax     = settings.value("ClMax",        s_ClMax).toDouble();
        s_ClInc     = settings.value("ClDelta",      s_ClInc).toDouble();

        if(settings.contains("NReynolds"))
        {
            int NRe = settings.value("NReynolds").toInt();
            s_ActiveList.clear();
            s_ReList.clear();
            s_MachList.clear();
            s_NCritList.clear();
            for (int i=0; i<NRe; i++)
            {
                QString str0 = QString("ActiveList%1").arg(i);
                QString str1 = QString("ReList%1").arg(i);
                QString str2 = QString("MaList%1").arg(i);
                QString str3 = QString("NcList%1").arg(i);
                if(settings.contains(str0)) s_ActiveList.append(settings.value(str0).toBool());
                if(settings.contains(str1)) s_ReList.append(settings.value(str1).toDouble());
                if(settings.contains(str2)) s_MachList.append(settings.value(str2).toDouble());
                if(settings.contains(str3)) s_NCritList.append(settings.value(str3).toDouble());
            }
        }

        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_VSplitterSizes = settings.value("VSplitterSizes").toByteArray();
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void BatchAbstractDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("BatchAbstractDlg");
    {
        settings.setValue("bInitBL",      s_bInitBL);

        switch (s_PolarType)
        {
            default:
            case xfl::FIXEDSPEEDPOLAR:    settings.setValue("PolarType", 1);   break;
            case xfl::FIXEDLIFTPOLAR:     settings.setValue("PolarType", 2);   break;
            case xfl::RUBBERCHORDPOLAR:   settings.setValue("PolarType", 3);   break;
        }

        settings.setValue("bAlpha",       s_bAlpha);
        settings.setValue("bFromZero",    s_bFromZero);

        settings.setValue("XTrTop",       s_XTop);
        settings.setValue("XTrBot",       s_XBot);

        settings.setValue("AlphaMin",     s_AlphaMin);
        settings.setValue("AlphaMax",     s_AlphaMax);
        settings.setValue("AlphaDelta",   s_AlphaInc);
        settings.setValue("ClMin",        s_ClMin);
        settings.setValue("ClMax",        s_ClMax);
        settings.setValue("ClDelta",      s_ClInc);

        settings.setValue("NReynolds", s_ReList.count());
        for (int i=0; i<s_ReList.count(); i++)
        {
            QString str0 = QString("ActiveList%1").arg(i);
            QString str1 = QString("ReList%1").arg(i);
            QString str2 = QString("MaList%1").arg(i);
            QString str3 = QString("NcList%1").arg(i);
            settings.setValue(str0, s_ActiveList.at(i));
            settings.setValue(str1, s_ReList.at(i));
            settings.setValue(str2, s_MachList.at(i));
            settings.setValue(str3, s_NCritList.at(i));
        }

        settings.setValue("VSplitterSizes",  s_HSplitterSizes);
        settings.setValue("VSplitterSizes",  s_VSplitterSizes);
        settings.setValue("WindowGeom",      s_Geometry);
    }
    settings.endGroup();
}


void BatchAbstractDlg::fillReModel()
{
    m_pReModel->setRowCount(s_ReList.count());
    m_pReModel->blockSignals(true);

    for (int i=0; i<s_ReList.count(); i++)
    {
        QModelIndex chindex = m_pReModel->index(i, 0, QModelIndex());
        m_pReModel->setData(chindex, s_ActiveList.at(i), Qt::UserRole);

        QModelIndex Xindex = m_pReModel->index(i, 1, QModelIndex());
        m_pReModel->setData(Xindex, s_ReList.at(i));

        QModelIndex Yindex =m_pReModel->index(i, 2, QModelIndex());
        m_pReModel->setData(Yindex, s_MachList.at(i));

        QModelIndex Zindex =m_pReModel->index(i, 3, QModelIndex());
        m_pReModel->setData(Zindex, s_NCritList.at(i));

        QModelIndex actionindex = m_pReModel->index(i, 4, QModelIndex());
        m_pReModel->setData(actionindex, QString("..."));
    }
    m_pReModel->blockSignals(false);
    m_pcptReTable->resizeRowsToContents();
}


void BatchAbstractDlg::setRowEnabled(int row, bool bEnabled)
{
    for(int col=0; col<m_pReModel->columnCount(); col++)
    {
        QModelIndex ind = m_pReModel->index(row, col, QModelIndex());
        m_pReModel->setData(ind, bEnabled, Qt::UserRole); // used to display the row as enabled or disabled
    }
}


void BatchAbstractDlg::onDelete()
{
    if(m_pReModel->rowCount()<=1) return;

    QModelIndex index = m_pcptReTable->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=s_ReList.count()) return;

    s_ActiveList.removeAt(sel);
    s_ReList.removeAt(sel);
    s_MachList.removeAt(sel);
    s_NCritList.removeAt(sel);

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());
}


void BatchAbstractDlg::onInsertBefore()
{
    int sel = m_pcptReTable->currentIndex().row();

    s_ActiveList.insert(sel, true);
    s_ReList.insert(sel, 0.0);
    s_MachList.insert(sel, 0.0);
    s_NCritList.insert(sel, 0.0);

    if     (sel>0)   s_ReList[sel] = (s_ReList.at(sel-1)+s_ReList.at(sel+1)) /2.0;
    else if(sel==0)  s_ReList[sel] =  s_ReList.at(sel+1)                     /2.0;
    else             s_ReList[0]   = 100000.0;

    if(sel>=0)
    {
        s_MachList[sel]  = s_MachList.at(sel+1);
        s_NCritList[sel] = s_NCritList.at(sel+1);
    }
    else
    {
        sel = 0;
        s_MachList[sel]  = 0.0;
        s_NCritList[sel] = 0.0;
    }

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());


    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void BatchAbstractDlg::onInsertAfter()
{
    int sel = m_pcptReTable->currentIndex().row()+1;

    s_ActiveList.insert(sel, true);
    s_ReList.insert(sel, 0.0);
    s_MachList.insert(sel, 0.0);
    s_NCritList.insert(sel, 0.0);

    if(sel==s_ReList.size()-1) s_ReList[sel] = s_ReList[sel-1]*2.0;
    else if(sel>0)             s_ReList[sel] = (s_ReList[sel-1]+s_ReList[sel+1]) /2.0;
    else if(sel==0)            s_ReList[sel] = s_ReList[sel+1]                   /2.0;

    if(sel>0)
    {
        s_MachList[sel]  = s_MachList[sel-1];
        s_NCritList[sel] = s_NCritList[sel-1];
    }
    else
    {
        sel = 0;
        s_MachList[sel]  = 0.0;
        s_NCritList[sel] = 0.0;
    }

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void BatchAbstractDlg::onCellChanged(QModelIndex topLeft, QModelIndex )
{
    s_ActiveList.clear();
    s_ReList.clear();
    s_MachList.clear();
    s_NCritList.clear();

    for (int i=0; i<m_pReModel->rowCount(); i++)
    {
        s_ActiveList.append(m_pReModel->index(i, 0, QModelIndex()).data(Qt::UserRole).toBool());
        s_ReList.append(   m_pReModel->index(i, 1, QModelIndex()).data().toDouble());
        s_MachList.append( m_pReModel->index(i, 2, QModelIndex()).data().toDouble());
        s_NCritList.append(m_pReModel->index(i, 3, QModelIndex()).data().toDouble());
    }

    if(topLeft.column()==0)
    {
        sortRe();

        //and fill back the model
        fillReModel();
    }
}


/**
* Bubble sort algorithm for the arrays of Reynolds, Mach and NCrit numbers.
* The arrays are sorted by crescending Re numbers.
*/
void BatchAbstractDlg::sortRe()
{
    int indx(0), indx2(0);
    bool Chtemp(true), Chtemp2(true);
    double Retemp(0), Retemp2(0);
    double Matemp(0), Matemp2(0);
    double NCtemp(0), NCtemp2(0);
    int flipped(0);

    if (s_ReList.size()<=1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for (indx2 = s_ReList.size() - 1; indx2 >= indx; --indx2)
        {
            Chtemp  = s_ActiveList.at(indx2);
            Chtemp2 = s_ActiveList.at(indx2 - 1);
            Retemp  = s_ReList.at(indx2);
            Retemp2 = s_ReList.at(indx2 - 1);
            Matemp  = s_MachList.at(indx2);
            Matemp2 = s_MachList.at(indx2 - 1);
            NCtemp  = s_NCritList.at(indx2);
            NCtemp2 = s_NCritList.at(indx2 - 1);
            if (Retemp2> Retemp)
            {
                s_ActiveList[indx2 - 1] = Chtemp;
                s_ActiveList[indx2]     = Chtemp2;
                s_ReList[indx2 - 1]     = Retemp;
                s_ReList[indx2]         = Retemp2;
                s_MachList[indx2 - 1]   = Matemp;
                s_MachList[indx2]       = Matemp2;
                s_NCritList[indx2 - 1]  = NCtemp;
                s_NCritList[indx2]      = NCtemp2;
                flipped = 1;
            }
        }
    } while ((++indx < s_ReList.size()) && flipped);
}


void BatchAbstractDlg::onReTableClicked(QModelIndex index)
{
    if(!index.isValid())  return;

    int row = index.row();
    if(row<0 || row>=m_pReModel->rowCount()) return;

    m_pcptReTable->selectRow(row);

    switch(index.column())
    {
        case 0:
        {
            bool bActive = m_pReModel->data(index, Qt::UserRole).toBool();
            if(row<s_ActiveList.size())
            {
                s_ActiveList[row] = !bActive; // toggle
                m_pReModel->setData(index, s_ActiveList.at(row), Qt::UserRole);
            }
            break;
        }
        case 4:
        {
            QRect itemrect = m_pcptReTable->visualRect(index);
            QPoint menupos = m_pcptReTable->mapToGlobal(itemrect.topLeft());
            QMenu *pReTableRowMenu = new QMenu(tr("Section"),this);
            pReTableRowMenu->addAction(m_pInsertBeforeAct);
            pReTableRowMenu->addAction(m_pInsertAfterAct);
            pReTableRowMenu->addAction(m_pDeleteAct);
            pReTableRowMenu->exec(menupos, m_pInsertBeforeAct);

            break;
        }
        default:  break;

    }
    update();
}


void BatchAbstractDlg::readFoils(QVector<Foil*> &foils)
{
    foils.clear();
    for(int i=0; i<m_plwNameList->count();i++)
    {
        QListWidgetItem *pItem = m_plwNameList->item(i);
        if(pItem && pItem->isSelected())
        {
            Foil *pFoil = Objects2d::foil(pItem->text());
            if(pFoil)
                foils.append(pFoil);
        }
    }
}


