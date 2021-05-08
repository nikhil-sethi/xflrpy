/****************************************************************************

    MainFrame  Class
    Copyright (C) 2008-2019 Andre Deperrois

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

#include <QGLFormat>
#include <QMessageBox>
#include <QtCore>
#include <QToolBar>
#include <QDockWidget>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QOpenGLContext>
#include <QSysInfo>


#include <design/afoil.h>
#include <glcontextinfo/view3dtestdlg.h>
#include <globals/globals.h>
#include <globals/mainframe.h>
#include <graph/graphdlg.h>
#include <gui_objects/splinefoil.h>
#include <miarex/miarex.h>
#include <miarex/analysis/panelanalysisdlg.h>
#include <miarex/analysis/stabpolardlg.h>
#include <miarex/analysis/wpolardlg.h>
#include <miarex/design/bodytransdlg.h>
#include <miarex/design/editobjectdelegate.h>
#include <miarex/design/gl3dbodydlg.h>
#include <miarex/design/gl3dwingdlg.h>
#include <miarex/design/inertiadlg.h>
#include <miarex/design/planedlg.h>
#include <miarex/mgt/planetabledelegate.h>
#include <miarex/objects3d.h>
#include <miarex/view/gl3dscales.h>
#include <miarex/view/stabviewdlg.h>
#include <miarex/view/w3drefsdlg.h>
#include <misc/aboutq5.h>
#include <misc/editplrdlg.h>
#include <misc/objectpropsdlg.h>
#include <misc/renamedlg.h>
#include <misc/line/linepickerdlg.h>
#include <misc/options/settings.h>
#include <misc/options/languagewt.h>
#include <misc/options/preferencesdlg.h>
#include <misc/options/saveoptions.h>
#include <misc/options/units.h>
#include <misc/popup.h>
#include <misc/updater.h>
#include <objects/objects2d/foil.h>
#include <objects/objects2d/polar.h>
#include <objects/objects3d/wing.h>
#include <script/xflscriptexec.h>
#include <script/xflscriptreader.h>
#include <viewwidgets/foildesignwt.h>
#include <viewwidgets/glWidgets/gl3dmiarexview.h>
#include <viewwidgets/graphwidget.h>
#include <viewwidgets/inverseviewwidget.h>
#include <viewwidgets/legendwidget.h>
#include <viewwidgets/miarextilewidget.h>
#include <viewwidgets/wingwidget.h>
#include <viewwidgets/xdirecttilewidget.h>
#include <xdirect/mgt/managefoilsdlg.h>
#include <xdirect/xdirect.h>
#include <xdirect/analysis/batchdlg.h>
#include <xdirect/analysis/batchthreaddlg.h>
#include <xdirect/analysis/foilpolardlg.h>
#include <xdirect/analysis/xfoilanalysisdlg.h>
#include <xdirect/geometry/cadddlg.h>
#include <xdirect/geometry/foilcoorddlg.h>
#include <xdirect/geometry/foilgeomdlg.h>
#include <xdirect/geometry/interpolatefoilsdlg.h>
#include <xdirect/geometry/ledlg.h>
#include <xdirect/geometry/nacafoildlg.h>
#include <xdirect/geometry/tegapdlg.h>
#include <xdirect/geometry/twodpaneldlg.h>
#include <xdirect/objects2d.h>
#include <xinverse/xinverse.h>

#include "PythonQt.h"
#include "PythonQt_QtAll.h"
#include <gui/PythonQtScriptingConsole.h>

// #include "pyscriptexec.h"

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif


QPointer<MainFrame> MainFrame::_self = nullptr;

QString MainFrame::s_ProjectName = "";
QString MainFrame::s_LanguageFilePath = "";
QDir MainFrame::s_StylesheetDir;
QDir MainFrame::s_TranslationDir;

bool MainFrame::s_bShowMousePos = true;
bool MainFrame::s_bSaved = true;
bool MainFrame::s_bOpenGL = true;

#ifdef QT_DEBUG
bool MainFrame::s_bTrace = true;
#else
bool MainFrame::s_bTrace = false;
#endif

QFile *MainFrame::s_pTraceFile = nullptr;

QLabel *MainFrame::m_pctrlProjectName = nullptr;
QVector <QColor> MainFrame::s_ColorList;



MainFrame::MainFrame(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(VERSIONNAME);
    setWindowIcon(QIcon(":/images/xflr5_64.png"));

//    if(s_bTrace)
    testConfiguration();

    m_bManualUpdateCheck = false;


    //    Settings sets(this);//to initialize the static variables
    //"Qt does not support style hints on X11 since this information is not provided by the window system."
    /*    Settings::s_TextFont.setStyleHint(QFont::TypeWriter, QFont::OpenGLCompatible);
    Settings::s_TextFont.setStyleStrategy(QFont::OpenGLCompatible);
    Settings::s_TextFont.setFamily(Settings::s_TextFont.defaultFamily());
    Settings::s_TextFont.setPointSize(9);

    Settings::s_TableFont.setStyleHint(QFont::TypeWriter);
    Settings::s_TableFont.setStyleStrategy(QFont::PreferDevice);
    Settings::s_TableFont.setFamily(Settings::s_TableFont.defaultFamily());
    Settings::s_TableFont.setPointSize(8);*/

    Settings::setDefaultFonts();

    Settings::s_RefGraph.setTitleFont(Settings::s_TextFont);
    Settings::s_RefGraph.setLabelFont(Settings::s_TextFont);

    //    Settings::s_StyleSheetName = "xflr5_style";
    Settings::s_StyleSheetName = "";

    m_iApp = XFLR5::NOAPP;

    PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);
    PythonQt_QtAll::init();
    mainModule = PythonQt::self()->getMainModule();
    // add path to libraries
    PythonQt::self()->addSysPath("~/.local/lib/python3.8/site-packages");
    PythonQt::self()->addSysPath(QDir::currentPath());
    createDockWindows();

    m_ImageFormat = XFLR5::PNG;
    Settings::s_ExportFileType = XFLR5::TXT;

    m_bAutoLoadLast = false;
    m_bAutoSave     = false;
    m_bSaveOpps     = false;
    m_bSaveWOpps    = true;
    m_bSaveSettings = true;

    m_SaveInterval = 10;
    m_GraphExportFilter = "Comma Separated Values (*.csv)";

    m_pSaveTimer = nullptr;

#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

    QString str;
    int kx=117, ky=57;
    bool bOK;
    settings.beginGroup("MainFrame");
    {
        str = settings.value("LanguageFilePath").toString();
        if(str.length()) s_LanguageFilePath = str;
        kx = settings.value("FrameGeometryx", 117).toInt(&bOK);
        ky = settings.value("FrameGeometryy",  57).toInt(&bOK);
    }
    settings.endGroup();

    XFoilAnalysisDlg::s_Position = QPoint(kx+31, ky+31);
    BatchThreadDlg::s_Position   = QPoint(kx+31, ky+31);
    BatchDlg::s_Position         = QPoint(kx+31, ky+31);
    LLTAnalysisDlg::setPosition(QPoint(kx+31, ky+31));
    PanelAnalysisDlg::s_Position = QPoint(kx+31, ky+31);
    EditPlrDlg::s_Position       = QPoint(kx+31, ky+31);



    if(s_LanguageFilePath.length())
    {
        qApp->removeTranslator(&m_Translator);
        if(m_Translator.load(s_LanguageFilePath))
        {
            qApp->installTranslator(&m_Translator);
        }
    }

    if(loadSettings())
    {
        Settings::loadSettings(settings);

        m_pAFoil->loadSettings(&settings);
        m_pXDirect->loadSettings(settings);
        m_pMiarex->loadSettings(settings);
        m_pXInverse->loadSettings(settings);

        GL3DScales::loadSettings(&settings);
        W3dPrefsDlg::loadSettings(&settings);
    }

    if(m_pSaveTimer)
    {
        m_pSaveTimer->stop();
        delete m_pSaveTimer;
    }

    if(m_bAutoSave)
    {
        m_pSaveTimer = new QTimer(this);
        m_pSaveTimer->setInterval(m_SaveInterval*60*1000);
        m_pSaveTimer->start();
        connect(m_pSaveTimer, SIGNAL(timeout()), this, SLOT(onSaveTimer()));
    }

    setupDataDir();

    m_pXDirect->setAnalysisParams();
    createActions();
    createMenus();
    createToolbars();
    createStatusBar();

    m_pXDirectTileWidget->connectSignals();
    m_pMiarexTileWidget->connectSignals();

    s_ColorList.append(QColor(255,0,0));
    s_ColorList.append(QColor(0,255,0));
    s_ColorList.append(QColor(0,0,255));
    s_ColorList.append(QColor(255,255,0));
    s_ColorList.append(QColor(255,0,255));
    s_ColorList.append(QColor(0,255,255));

    s_bSaved     = true;

    m_iApp = XFLR5::NOAPP;
    m_pctrlAFoilToolBar->hide();
    m_pctrlXDirectToolBar->hide();
    m_pctrlXInverseToolBar->hide();
    m_pctrlMiarexToolBar->hide();
    m_pctrlStabViewWidget->hide();

    setMenus();

    QString styleSheet;
    if(Settings::s_bStyleSheets)
    {
        readStyleSheet(Settings::s_StyleSheetName, styleSheet);
    }

    /*    if(m_bAutoLoadLast)
    {
        onLoadLastProject();
    }*/

    m_pUpdater = nullptr;
    if(Updater::bAutoCheck())
        onAutoCheckForUpdates();

    
    QObject *pxflrpyPackage = new xflrpyPackage(this);
    // // PythonQtObjectPtr *xflrpyModule = p_xflrpyPackage;
    // // m_pMiarex->show();
    // // // MiarexPyWrapper example(m_pMiarex);
    // // // example->setPanel();
    // mainModule.addObject("xflrpyPackage", pxflrpyPackage);
    // // PythonQtObjectPtr xflrpy = mainModule.evalScript("xflrpyPackage\n", Py_eval_input);
    // PyObject *temp=mainModule;
    // PythonQtObjectPtr xflrpy = PythonQt::self()->lookupObject(mainModule.object(),"xflrpyPackage");
    

    QObject *pMiarexWrapper = new MiarexWrapper(m_pMiarex);
    mainModule.addObject("MiarexWrapper", pMiarexWrapper);
    QObject *pPlaneWrapper = new PlaneWrapper(m_pMiarex->m_pCurPlane, m_pMiarex);
    mainModule.addObject("PlaneWrapper", pPlaneWrapper);
    QObject *pAnalysisWrapper = new AnalysisWrapper(m_pMiarex->m_pCurWPolar,m_pMiarex->m_pCurPlane,m_pMiarex);
    mainModule.addObject("AnalysisWrapper", pAnalysisWrapper);
    QObject *pPolarWrapper = new PolarWrapper(m_pMiarex->m_pCurWPolar);
    mainModule.addObject("PolarWrapper", pPolarWrapper);
    // PythonQt::self()->lookupObject(mainModule.object(),"xflrpyPackage");
    // // mainModule.addObject("p", plane);
    // PythonQt::self()->registerCPPClass("p","","example", PythonQtCreateObject<CustomObjectWrapper>);
    // mainModule.addObject("example", m_pMiare);
}


MainFrame::~MainFrame()
{
    if(s_pTraceFile) s_pTraceFile->close();

    Objects2d::deleteAllFoils();

    if(m_pSaveTimer)
    {
        m_pSaveTimer->stop();
        delete m_pSaveTimer;
    }

    if(m_pUpdater)
    {
        delete m_pUpdater;
        m_pUpdater = nullptr;
    }
}


void MainFrame::testConfiguration()
{
    QString FileName = QDir::tempPath() + "/Trace.log";
    Trace(FileName);
    s_pTraceFile = new QFile(FileName);

    if (!s_pTraceFile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) s_bTrace = false;
    s_pTraceFile->reset();

#if QT_VERSION >= 0x050500
    QString strange;
    QOperatingSystemVersion const &sys = QOperatingSystemVersion::current();
    strange = sys.name();
    Trace(strange);

    QSysInfo sysInfo;

    Trace("build ABI: "       + sysInfo.buildAbi());
    Trace("build CPU: "       + sysInfo.buildCpuArchitecture());
    Trace("current CPU: "     + sysInfo.currentCpuArchitecture());
    Trace("kernel type: "     + sysInfo.kernelType());
    Trace("kernel version: "  + sysInfo.kernelVersion());
    Trace("product name: "    + sysInfo.prettyProductName());
    Trace("product type: "    + sysInfo.productType());
    Trace("product version: " + sysInfo.productVersion());
#endif
    Trace("OpenGL support:");
    Trace("    Desktop OpenGL ", qApp->testAttribute(Qt::AA_UseDesktopOpenGL));
    Trace("    OpenGL ES      ", qApp->testAttribute(Qt::AA_UseOpenGLES));
    Trace("    Software OpenGL", qApp->testAttribute(Qt::AA_UseSoftwareOpenGL));


    strange.sprintf("   Default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
    Trace(strange);


    if(!QGLFormat::hasOpenGL())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Your system does not provide support for OpenGL.\nXFLR5 will not operate correctly."));
        s_bOpenGL = false;
    }
    else if(QSurfaceFormat::defaultFormat().majorVersion()<2)
    {
        QString strong = "XFLR5 requires OpenGL 2.1 or greater.\n";
        QString strange;
        strange.sprintf("Your system provides by default OpenGL %d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
        QMessageBox::warning(this, tr("Warning"), strong+strange);
        s_bOpenGL = false;
    }
}


void MainFrame::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt(
            #ifdef Q_OS_MAC
                this
            #else
                //            activeWindow()
                this
            #endif // Q_OS_MAC
                );
#endif // QT_NO_MESSAGEBOX
}



void MainFrame::aboutXFLR5()
{
    AboutQ5 dlg(this);
    dlg.exec();
}


void MainFrame::addRecentFile(const QString &PathName)
{
    m_RecentFiles.removeAll(PathName);
    m_RecentFiles.prepend(PathName);
    while (m_RecentFiles.size() > MAXRECENTFILES)
        m_RecentFiles.removeLast();

    updateRecentFileActions();
}


void MainFrame::closeEvent (QCloseEvent * pEvent)
{
    if(!s_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Exit"), tr("Save the project before exit ?"),
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                         QMessageBox::Yes);
        if(resp == QMessageBox::Yes)
        {
            if(!saveProject(m_FileName))
            {
                pEvent->ignore();
                return;
            }
            addRecentFile(m_FileName);
        }
        else if (resp==QMessageBox::Cancel)
        {
            pEvent->ignore();
            return;
        }
    }
    //close all dockwindows
    m_p2dWidget->close();
    m_pDirect2dWidget->close();
    m_pgl3dMiarexView->close();
    m_pMiarexTileWidget->close();
    m_pXDirectTileWidget->close();
    m_VoidWidget.close();


    m_pctrlXDirectWidget->close();
    m_pctrlMiarexWidget->close();
    m_pctrlAFoilWidget->close();
    m_pctrlXInverseWidget->close();
    m_pctrl3DScalesWidget->close();
    m_pctrlStabViewWidget->close();
    m_pGL3DScales->close();
    m_pctrlCentralWidget->close();
    m_glLightDlg.close();
    deleteProject(true);

    saveSettings();
    pEvent->accept();//continue closing
}


void MainFrame::createActions()
{
    m_pNewProjectAct = new QAction(QIcon(":/images/new.png"), tr("New Project") + "\tCtrl+N", this);
//    m_pNewProjectAct->setShortcut(QKeySequence::New); // bug in Qt libs: shortcut is defined twice in translation files
//    m_pNewProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    m_pNewProjectAct->setStatusTip(tr("Save and close the current project, create a new project"));
    connect(m_pNewProjectAct, SIGNAL(triggered()), this, SLOT(onNewProject()));

    m_pOpenAct = new QAction(QIcon(":/images/open.png"), tr("Open")  + "\tCtrl+O", this);
//    m_pOpenAct->setShortcut(QKeySequence::Open); // bug in Qt libs: shortcut is defined twice in translation files
    m_pOpenAct->setStatusTip(tr("Open an existing file"));
    connect(m_pOpenAct, SIGNAL(triggered()), this, SLOT(onLoadFile()));

    m_pSaveAct = new QAction(QIcon(":/images/save.png"), tr("Save") + "\tCtrl+S", this);
//    m_pSaveAct->setShortcut(QKeySequence::Save); // bug in Qt libs: shortcut is defined twice in translation files
    m_pSaveAct->setStatusTip(tr("Save the project to disk"));
    connect(m_pSaveAct, SIGNAL(triggered()), this, SLOT(onSaveProject()));

    m_pSaveProjectAsAct = new QAction(tr("Save Project As")  + "\tCtrl+Shift+S", this);
//    m_pSaveProjectAsAct->setShortcut(QKeySequence::SaveAs); // bug in Qt libs: shortcut is defined twice in translation files
    m_pSaveProjectAsAct->setStatusTip(tr("Save the current project under a new name"));
    connect(m_pSaveProjectAsAct, SIGNAL(triggered()), this, SLOT(onSaveProjectAs()));

    m_pCloseProjectAct = new QAction(QIcon(":/images/new.png"), tr("Close the Project"), this);
    m_pCloseProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
    m_pCloseProjectAct->setStatusTip(tr("Save and close the current project"));
    connect(m_pCloseProjectAct, SIGNAL(triggered()), this, SLOT(onNewProject()));

    m_pInsertAct = new QAction(tr("Insert Project"), this);
    m_pInsertAct->setStatusTip(tr("Insert an existing project in the current project"));
    m_pInsertAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    connect(m_pInsertAct, SIGNAL(triggered()), this, SLOT(onInsertProject()));

    m_pOnAFoilAct = new QAction(tr("Direct Foil Design"), this);
    m_pOnAFoilAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
    m_pOnAFoilAct->setStatusTip(tr("Open Foil Design application"));
    connect(m_pOnAFoilAct, SIGNAL(triggered()), this, SLOT(onAFoil()));

    m_pOnXInverseAct = new QAction(tr("XFoil Inverse Design"), this);
    m_pOnXInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
    m_pOnXInverseAct->setStatusTip(tr("Open XFoil inverse analysis application"));
    connect(m_pOnXInverseAct, SIGNAL(triggered()), this, SLOT(onXInverse()));

    m_pOnMixedInverseAct = new QAction(tr("XFoil Mixed Inverse Design"), this);
    m_pOnMixedInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
    m_pOnMixedInverseAct->setStatusTip(tr("Open XFoil Mixed Inverse analysis application"));
    connect(m_pOnMixedInverseAct, SIGNAL(triggered()), this, SLOT(onXInverseMixed()));

    m_pOnXDirectAct = new QAction(tr("XFoil Direct Analysis"), this);
    m_pOnXDirectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));
    m_pOnXDirectAct->setStatusTip(tr("Open XFoil direct analysis application"));
    connect(m_pOnXDirectAct, SIGNAL(triggered()), this, SLOT(onXDirect()));

    m_pOnMiarexAct = new QAction(tr("Wing and Plane Design"), this);
    m_pOnMiarexAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_6));
    m_pOnMiarexAct->setStatusTip(tr("Open Wing/plane design and analysis application"));
    connect(m_pOnMiarexAct, SIGNAL(triggered()), this, SLOT(onMiarex()));


    m_pLoadLastProjectAction = new QAction(tr("Load Last Project"), this);
    m_pLoadLastProjectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
    m_pLoadLastProjectAction->setStatusTip(tr("Loads the last saved project"));
    connect(m_pLoadLastProjectAction, SIGNAL(triggered()), this, SLOT(onLoadLastProject()));


    m_pPreferencesAct = new QAction(tr("Preferences"), this);
    connect(m_pPreferencesAct, SIGNAL(triggered()), this, SLOT(onPreferences()));

    m_pRestoreToolbarsAct     = new QAction(tr("Restore toolbars"), this);
    m_pRestoreToolbarsAct->setStatusTip(tr("Restores the toolbars to their original state"));
    connect(m_pRestoreToolbarsAct, SIGNAL(triggered()), this, SLOT(onRestoreToolbars()));

    m_pSaveViewToImageFileAct = new QAction(tr("Save View to Image File"), this);
    m_pSaveViewToImageFileAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    m_pSaveViewToImageFileAct->setStatusTip(tr("Saves the current view to a file on disk"));
    connect(m_pSaveViewToImageFileAct, SIGNAL(triggered()), this, SLOT(onSaveViewToImageFile()));

    m_pResetSettingsAct = new QAction(tr("Reset Default Settings"), this);
    m_pResetSettingsAct->setStatusTip(tr("will revert to default settings at the next session"));
    connect(m_pResetSettingsAct, SIGNAL(triggered()), this, SLOT(onResetSettings()));


    for (int i=0; i<MAXRECENTFILES; ++i)
    {
        m_pRecentFileActs[i] = new QAction(this);
        m_pRecentFileActs[i]->setVisible(false);
        connect(m_pRecentFileActs[i], SIGNAL(triggered()), this, SLOT(onOpenRecentFile()));
    }

    m_pExportCurGraphAct = new QAction(tr("Export Graph"), this);
    m_pExportCurGraphAct->setStatusTip(tr("Export the current graph data to a text file"));
    connect(m_pExportCurGraphAct, SIGNAL(triggered()), this, SLOT(onExportCurGraph()));

    m_pResetCurGraphScales = new QAction(QIcon(":/images/OnResetGraphScale.png"), tr("Reset Graph Scales")+"\tR", this);
    //    m_pResetCurGraphScales->setShortcut(Qt::Key_R);
    m_pResetCurGraphScales->setStatusTip(tr("Restores the graph's x and y scales"));
    connect(m_pResetCurGraphScales, SIGNAL(triggered()), this, SLOT(onResetCurGraphScales()));

    m_pCurGraphDlgAct = new QAction(tr("Define Graph Settings"), this);
    m_pCurGraphDlgAct->setShortcut(Qt::Key_G);
    connect(m_pCurGraphDlgAct, SIGNAL(triggered()), this, SLOT(onCurGraphSettings()));

    m_pExitAct = new QAction(tr("E&xit"), this);
    m_pExitAct->setShortcut(QKeySequence::Quit);
    m_pExitAct->setStatusTip(tr("Exit the application"));
    connect(m_pExitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_pOpenGLAct = new QAction(tr("OpenGL info"), this);
    connect(m_pOpenGLAct, SIGNAL(triggered()), this, SLOT(onOpenGLInfo()));

    m_pAboutAct = new QAction(tr("About"), this);
    m_pAboutAct->setStatusTip(tr("More information about XFLR5"));
    connect(m_pAboutAct, SIGNAL(triggered()), this, SLOT(aboutXFLR5()));

    m_pAboutQtAct = new QAction(tr("About Qt"), this);
    connect(m_pAboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));

    m_pCheckForUpdates = new QAction(tr("Check for updates"), this);
    connect(m_pCheckForUpdates, SIGNAL(triggered(bool)), this, SLOT(onCheckForUpdates()));

    m_pExecPyScript = new QAction(tr("Execute python script"), this);
    connect(m_pExecPyScript, SIGNAL(triggered(bool)), this, SLOT(onExecutePythonScript()));

    createGraphActions();
    createAFoilActions();
    createXDirectActions();
    createXInverseActions();
    createMiarexActions();
}


void MainFrame::createAFoilActions()
{
    m_pStoreSplineAct= new QAction(QIcon(":/images/OnStoreFoil.png"), tr("Store Splines as Foil"), this);
    m_pStoreSplineAct->setStatusTip(tr("Store the current splines in the foil database"));
    connect(m_pStoreSplineAct, SIGNAL(triggered()), m_pAFoil, SLOT(onStoreSplines()));

    m_pSplineControlsAct= new QAction(tr("Splines Params"), this);
    m_pSplineControlsAct->setStatusTip(tr("Define parameters for the splines : degree, number of out points"));
    connect(m_pSplineControlsAct, SIGNAL(triggered()), m_pAFoil, SLOT(onSplineControls()));

    m_pExportSplinesToFileAct= new QAction(tr("Export Splines To File"), this);
    m_pExportSplinesToFileAct->setStatusTip(tr("Define parameters for the splines : degree, number of out points"));
    connect(m_pExportSplinesToFileAct, SIGNAL(triggered()), m_pAFoil, SLOT(onExportSplinesToFile()));

    m_pNewSplinesAct= new QAction(tr("New Splines"), this);
    m_pNewSplinesAct->setStatusTip(tr("Reset the splines"));
    connect(m_pNewSplinesAct, SIGNAL(triggered()), m_pAFoil, SLOT(onNewSplines()));

    m_pUndoAFoilAct= new QAction(QIcon(":/images/OnUndo.png"), tr("Undo") + "\tCtrl+Z", this);
//    m_pUndoAFoilAct->setShortcut(QKeySequence::Undo); // bug in Qt libs: shortcut is defined twice in translation files
    m_pUndoAFoilAct->setStatusTip(tr("Cancels the last modification"));
    connect(m_pUndoAFoilAct, SIGNAL(triggered()), m_pAFoil, SLOT(onUndo()));

    m_pRedoAFoilAct= new QAction(QIcon(":/images/OnRedo.png"), tr("Redo") + "\tCtrl+Shift+Z", this);
//    m_pRedoAFoilAct->setShortcut(QKeySequence::Redo); // bug in Qt libs: shortcut is defined twice in translation files
    m_pRedoAFoilAct->setStatusTip(tr("Restores the last cancelled modification"));
    connect(m_pRedoAFoilAct, SIGNAL(triggered()), m_pAFoil, SLOT(onRedo()));

    m_pShowAllFoils= new QAction(tr("Show All Foils"), this);
    connect(m_pShowAllFoils, SIGNAL(triggered()), m_pAFoil, SLOT(onShowAllFoils()));
    m_pHideAllFoils= new QAction(tr("Hide All Foils"), this);
    connect(m_pHideAllFoils, SIGNAL(triggered()), m_pAFoil, SLOT(onHideAllFoils()));


    m_pAFoilDelete = new QAction(tr("Delete"), this);
    connect(m_pAFoilDelete, SIGNAL(triggered()), m_pAFoil, SLOT(onDeleteCurFoil()));

    m_pAFoilRename = new QAction(tr("Rename"), this);
    m_pAFoilRename->setShortcut(Qt::Key_F2);
    connect(m_pAFoilRename, SIGNAL(triggered()), m_pAFoil, SLOT(onRenameFoil()));

    m_pAFoilExport = new QAction(tr("Export"), this);
    connect(m_pAFoilExport, SIGNAL(triggered()), m_pAFoil, SLOT(onExportCurFoil()));

    m_pAFoilDuplicateFoil = new QAction(tr("Duplicate"), this);
    connect(m_pAFoilDuplicateFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onDuplicate()));

    m_pShowCurrentFoil= new QAction(tr("Show Current Foil"), this);
    connect(m_pShowCurrentFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onShowCurrentFoil()));

    m_pHideCurrentFoil= new QAction(tr("Hide Current Foil"), this);
    connect(m_pHideCurrentFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onHideCurrentFoil()));

    m_pAFoilDerotateFoil = new QAction(tr("De-rotate the Foil"), this);
    m_pAFoilDerotateFoil->setToolTip(tr("Set chord line level"));
    connect(m_pAFoilDerotateFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilDerotateFoil()));

    m_pAFoilNormalizeFoil = new QAction(tr("Normalize the Foil"), this);
    connect(m_pAFoilNormalizeFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilNormalizeFoil()));

    m_pAFoilRefineGlobalFoil = new QAction(tr("Refine Globally")/*+"\t(F3)"*/, this);
    m_pAFoilRefineGlobalFoil->setShortcut(Qt::Key_F3);
    connect(m_pAFoilRefineGlobalFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilPanels()));

    m_pAFoilRefineLocalFoil = new QAction(tr("Refine Locally")/*+"\t(Shift+F3)"*/, this);
    m_pAFoilRefineLocalFoil->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F3));
    connect(m_pAFoilRefineLocalFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilCadd()));

    m_pAFoilEditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
    connect(m_pAFoilEditCoordsFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilFoilCoordinates()));

    m_pAFoilScaleFoil = new QAction(tr("Scale camber and thickness")+"\tF9", this);
    connect(m_pAFoilScaleFoil, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilFoilGeom()));

    m_pAFoilSetTEGap = new QAction(tr("Set T.E. Gap"), this);
    connect(m_pAFoilSetTEGap, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilSetTEGap()));

    m_pAFoilSetLERadius = new QAction(tr("Set L.E. Radius"), this);
    connect(m_pAFoilSetLERadius, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilSetLERadius()));

    m_pAFoilLECircle = new QAction(tr("Show LE Circle"), this);
    connect(m_pAFoilLECircle, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilLECircle()));

    m_pShowLegend = new QAction(tr("Show Legend"), this);
    m_pShowLegend->setCheckable(true);
    connect(m_pShowLegend, SIGNAL(triggered()), m_pAFoil, SLOT(onShowLegend()));

    m_pAFoilSetFlap = new QAction(tr("Set Flap")+"\tF10", this);
    connect(m_pAFoilSetFlap, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilSetFlap()));

    m_pAFoilInterpolateFoils = new QAction(tr("Interpolate Foils")+"\tF11", this);
    connect(m_pAFoilInterpolateFoils, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilInterpolateFoils()));

    m_pAFoilNacaFoils = new QAction(tr("Naca Foils"), this);
    connect(m_pAFoilNacaFoils, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilNacaFoils()));

    m_pAFoilTableColumns = new QAction(tr("Set Table Columns"), this);
    connect(m_pAFoilTableColumns, SIGNAL(triggered()), m_pAFoil, SLOT(onAFoilTableColumns()));

    m_pAFoilTableColumnWidths = new QAction(tr("Reset column widths"), this);
    connect(m_pAFoilTableColumnWidths, SIGNAL(triggered()), m_pAFoil, SLOT(onResetColumnWidths()));



    m_pAFoilGridAct= new QAction(tr("Grid Settings"), this);
    m_pAFoilGridAct->setStatusTip(tr("Define the grid settings for the view"));
    connect(m_pAFoilGridAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onGridSettings()));

    m_pInsertSplinePt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
    connect(m_pInsertSplinePt, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onInsertPt()));

    m_pRemoveSplinePt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
    connect(m_pRemoveSplinePt, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onRemovePt()));

    m_pResetXScaleAct= new QAction(QIcon(":/images/OnResetXScale.png"), tr("Reset X Scale"), this);
    m_pResetXScaleAct->setStatusTip(tr("Resets the scale to fit the current screen width"));
    connect(m_pResetXScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetXScale()));

    m_pResetXYScaleAct= new QAction(QIcon(":/images/OnResetFoilScale.png"), tr("Reset Scales")+"\tR", this);
    m_pResetXYScaleAct->setStatusTip(tr("Resets the x and y scales to screen size"));
    connect(m_pResetXYScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetScales()));


    m_pAFoilLoadImage = new QAction(tr("Load background image")   +"\tCtrl+Shift+I", this);
    connect(m_pAFoilLoadImage, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onLoadBackImage()));
    m_pAFoilClearImage = new QAction(tr("Clear background image") +"\tCtrl+Shift+I", this);

    connect(m_pAFoilClearImage, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onClearBackImage()));


    m_pResetYScaleAct= new QAction(tr("Reset Y Scale"), this);
    connect(m_pResetYScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetYScale()));

    m_pZoomInAct= new QAction(QIcon(":/images/OnZoomIn.png"), tr("Zoom in"), this);
    m_pZoomInAct->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
    connect(m_pZoomInAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomIn()));

    m_pZoomLessAct= new QAction(QIcon(":/images/OnZoomLess.png"), tr("Zoom Less"), this);
    m_pZoomLessAct->setStatusTip(tr("Zoom Less"));
    connect(m_pZoomLessAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomLess()));

    m_pZoomYAct= new QAction(QIcon(":/images/OnZoomYScale.png"), tr("Zoom Y Scale Only"), this);
    m_pZoomYAct->setStatusTip(tr("Zoom Y scale Only"));
    connect(m_pZoomYAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomYOnly()));
}


void MainFrame::createAFoilMenus()
{
    m_pAFoilViewMenu = menuBar()->addMenu(tr("View"));
    {
        m_pAFoilViewMenu->addAction(m_pShowCurrentFoil);
        m_pAFoilViewMenu->addAction(m_pHideCurrentFoil);
        m_pAFoilViewMenu->addAction(m_pShowAllFoils);
        m_pAFoilViewMenu->addAction(m_pHideAllFoils);
        m_pAFoilViewMenu->addSeparator();
        m_pAFoilViewMenu->addAction(m_pZoomInAct);
        m_pAFoilViewMenu->addAction(m_pZoomLessAct);
        m_pAFoilViewMenu->addAction(m_pResetXScaleAct);
        m_pAFoilViewMenu->addAction(m_pResetYScaleAct);
        m_pAFoilViewMenu->addAction(m_pResetXYScaleAct);
        m_pAFoilViewMenu->addSeparator();
        m_pAFoilViewMenu->addAction(m_pShowLegend);
        m_pAFoilViewMenu->addAction(m_pAFoilLECircle);
        m_pAFoilViewMenu->addAction(m_pAFoilGridAct);
        m_pAFoilViewMenu->addSeparator();
        m_pAFoilViewMenu->addAction(m_pAFoilLoadImage);
        m_pAFoilViewMenu->addAction(m_pAFoilClearImage);
        m_pAFoilViewMenu->addSeparator();
        m_pAFoilViewMenu->addAction(m_pSaveViewToImageFileAct);
    }

    m_pAFoilDesignMenu = menuBar()->addMenu(tr("F&oil"));
    {
        m_pAFoilDesignMenu->addAction(m_pAFoilRename);
        m_pAFoilDesignMenu->addAction(m_pAFoilDelete);
        m_pAFoilDesignMenu->addAction(m_pAFoilExport);
        m_pAFoilDesignMenu->addAction(m_pAFoilDuplicateFoil);
        m_pAFoilDesignMenu->addSeparator();
        m_pAFoilDesignMenu->addAction(m_pHideAllFoils);
        m_pAFoilDesignMenu->addAction(m_pShowAllFoils);
        m_pAFoilDesignMenu->addSeparator();
        m_pAFoilDesignMenu->addAction(m_pAFoilNormalizeFoil);
        m_pAFoilDesignMenu->addAction(m_pAFoilDerotateFoil);
        m_pAFoilDesignMenu->addAction(m_pAFoilRefineLocalFoil);
        m_pAFoilDesignMenu->addAction(m_pAFoilRefineGlobalFoil);
        m_pAFoilDesignMenu->addAction(m_pAFoilEditCoordsFoil);
        m_pAFoilDesignMenu->addAction(m_pAFoilScaleFoil);
        m_pAFoilDesignMenu->addAction(m_pAFoilSetTEGap);
        m_pAFoilDesignMenu->addAction(m_pAFoilSetLERadius);
        m_pAFoilDesignMenu->addAction(m_pAFoilSetFlap);
        m_pAFoilDesignMenu->addSeparator();
        m_pAFoilDesignMenu->addAction(m_pAFoilInterpolateFoils);
        m_pAFoilDesignMenu->addAction(m_pAFoilNacaFoils);
        m_pAFoilDesignMenu->addAction(m_pManageFoilsAct);
    }

    m_pAFoilSplineMenu = menuBar()->addMenu(tr("Splines"));
    {
        /*        m_pAFoilSplineMenu->addAction(m_pInsertSplinePt);
        m_pAFoilSplineMenu->addAction(m_pRemoveSplinePt);
        m_pAFoilSplineMenu->addSeparator();*/
        m_pAFoilSplineMenu->addAction(m_pUndoAFoilAct);
        m_pAFoilSplineMenu->addAction(m_pRedoAFoilAct);
        m_pAFoilSplineMenu->addSeparator();
        m_pAFoilSplineMenu->addAction(m_pNewSplinesAct);
        m_pAFoilSplineMenu->addAction(m_pSplineControlsAct);
        m_pAFoilSplineMenu->addAction(m_pStoreSplineAct);
        m_pAFoilSplineMenu->addAction(m_pExportSplinesToFileAct);
    }

    //AFoil Context Menu
    m_pAFoilCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pAFoilDesignMenu_AFoilCtxMenu = m_pAFoilCtxMenu->addMenu(tr("F&oil"));
        {
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilRename);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilDelete);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilExport);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilDuplicateFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pHideAllFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pShowAllFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilNormalizeFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilDerotateFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilRefineLocalFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilRefineGlobalFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilEditCoordsFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilScaleFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilSetTEGap);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilSetLERadius);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilSetFlap);
            m_pAFoilDesignMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilInterpolateFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilNacaFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pManageFoilsAct);
        }
        //m_pAFoilCtxMenu->addMenu(m_pAFoilDesignMenu);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilSplineMenu_AFoilCtxMenu = m_pAFoilCtxMenu->addMenu(tr("Splines"));
        {
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pInsertSplinePt);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pRemoveSplinePt);
            m_pAFoilSplineMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pUndoAFoilAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pRedoAFoilAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pNewSplinesAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pSplineControlsAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pStoreSplineAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pExportSplinesToFileAct);
        }
        //m_pAFoilCtxMenu->addMenu(m_pAFoilSplineMenu);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilCtxMenu->addAction(m_pShowAllFoils);
        m_pAFoilCtxMenu->addAction(m_pHideAllFoils);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilCtxMenu->addAction(m_pResetXScaleAct);
        m_pAFoilCtxMenu->addAction(m_pResetYScaleAct);
        m_pAFoilCtxMenu->addAction(m_pResetXYScaleAct);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilCtxMenu->addAction(m_pShowLegend);
        m_pAFoilCtxMenu->addAction(m_pAFoilLECircle);
        m_pAFoilCtxMenu->addAction(m_pAFoilGridAct);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilCtxMenu->addAction(m_pAFoilLoadImage);
        m_pAFoilCtxMenu->addAction(m_pAFoilClearImage);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilCtxMenu->addAction(m_pSaveViewToImageFileAct);
        m_pAFoilCtxMenu->addSeparator();
        m_pAFoilCtxMenu->addAction(m_pAFoilTableColumns);
        m_pAFoilCtxMenu->addAction(m_pAFoilTableColumnWidths);
    }
    m_pDirect2dWidget->setContextMenu(m_pAFoilCtxMenu);

    //Context menu to be displayed when user right clicks on a foil in the table
    m_pAFoilTableCtxMenu = new QMenu(tr("Foil Actions"),this);
    {
        m_pAFoilTableCtxMenu->addAction(m_pAFoilRename);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilDelete);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilExport);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilDuplicateFoil);
        m_pAFoilTableCtxMenu->addSeparator();
        m_pAFoilTableCtxMenu->addAction(m_pAFoilNormalizeFoil);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilDerotateFoil);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilRefineLocalFoil);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilRefineGlobalFoil);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilEditCoordsFoil);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilScaleFoil);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilSetTEGap);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilSetLERadius);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilSetFlap);
        m_pAFoilTableCtxMenu->addSeparator();
        m_pAFoilTableCtxMenu->addAction(m_pAFoilTableColumns);
        m_pAFoilTableCtxMenu->addAction(m_pAFoilTableColumnWidths);
    }
}


void MainFrame::createAFoilToolbar()
{
    m_pctrlAFoilToolBar = addToolBar(tr("Foil 1"));
    m_pctrlAFoilToolBar->addAction(m_pNewProjectAct);
    m_pctrlAFoilToolBar->addAction(m_pOpenAct);
    m_pctrlAFoilToolBar->addAction(m_pSaveAct);
    m_pctrlAFoilToolBar->addSeparator();
    m_pctrlAFoilToolBar->addAction(m_pZoomInAct);
    m_pctrlAFoilToolBar->addAction(m_pZoomLessAct);
    m_pctrlAFoilToolBar->addAction(m_pResetXYScaleAct);
    m_pctrlAFoilToolBar->addAction(m_pResetXScaleAct);
    m_pctrlAFoilToolBar->addAction(m_pZoomYAct);
    m_pctrlAFoilToolBar->addSeparator();
    m_pctrlAFoilToolBar->addAction(m_pUndoAFoilAct);
    m_pctrlAFoilToolBar->addAction(m_pRedoAFoilAct);

    m_pctrlAFoilToolBar->addSeparator();
    m_pctrlAFoilToolBar->addAction(m_pStoreSplineAct);
}



void MainFrame::createDockWindows()
{
    AFoil::s_pMainFrame           = this;
    XDirect::s_pMainFrame         = this;
    XInverse::s_pMainFrame        = this;
    Miarex::s_pMainFrame          = this;
    gl3dView::s_pMainFrame         = this;
    GraphWidget::s_pMainFrame      = this;
    OpPointWidget::s_pMainFrame    = this;


    m_pctrlXDirectWidget = new QDockWidget(tr("Direct foil analysis"), this);
    m_pctrlXDirectWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_pctrlXDirectWidget);

    m_pctrlXInverseWidget = new QDockWidget(tr("Inverse foil design"), this);
    m_pctrlXInverseWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_pctrlXInverseWidget);

    m_pctrlMiarexWidget = new QDockWidget(tr("Plane analysis"), this);
    m_pctrlMiarexWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_pctrlMiarexWidget);

    m_pctrlAFoilWidget = new QDockWidget(tr("Foil direct design"), this);
    m_pctrlAFoilWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_pctrlAFoilWidget);


    m_pctrlPyConsoleWidget = new QDockWidget(tr("Python console"), this);
    m_pctrlPyConsoleWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_pctrlPyConsoleWidget);
    
    m_pConsole = new PythonQtScriptingConsole(NULL, mainModule);
    m_pctrlPyConsoleWidget->setWidget(m_pConsole);
    m_pctrlPyConsoleWidget->setFloating(false);
    m_pctrlPyConsoleWidget->setVisible(true);


    m_p2dWidget = new InverseViewWidget(this);
    m_pgl3dMiarexView = new gl3dMiarexView(this);

    m_pDirect2dWidget = new FoilDesignWt(this);
    m_pXDirectTileWidget = new XDirectTileWidget(this);
    m_pMiarexTileWidget  = new MiarexTileWidget(this);

    m_pXDirect = new XDirect(this);
    m_pXDirect->setObjectName("XDirect ???");
    m_pctrlXDirectWidget->setWidget(m_pXDirect);
    m_pctrlXDirectWidget->setVisible(false);
    m_pctrlXDirectWidget->setFloating(true);
    m_pctrlXDirectWidget->move(960,60);

    m_pXInverse = new XInverse(this);
    m_pctrlXInverseWidget->setWidget(m_pXInverse);
    m_pctrlXInverseWidget->setVisible(false);
    m_pctrlXInverseWidget->setFloating(true);
    m_pctrlXInverseWidget->move(960,60);

    m_pMiarex = new Miarex(this);
    m_pctrlMiarexWidget->setWidget(m_pMiarex);
    
    m_pctrlMiarexWidget->setVisible(false);
    m_pctrlMiarexWidget->setFloating(true);
    m_pctrlMiarexWidget->move(960,60);


    m_pGL3DScales = new GL3DScales(this);
    GL3DScales::s_pMiarex      = m_pMiarex;
    m_pctrl3DScalesWidget = new QDockWidget(tr("3D Scales"), this);
    m_pctrl3DScalesWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_pctrl3DScalesWidget);
    m_pctrl3DScalesWidget->setWidget(m_pGL3DScales);
    m_pGL3DScales->setParent(m_pctrl3DScalesWidget);
    m_pctrl3DScalesWidget->setVisible(false);
    m_pctrl3DScalesWidget->setFloating(true);
    m_pctrl3DScalesWidget->move(60,60);

    StabViewDlg::s_pMiarex = m_pMiarex;
    m_pStabView = new StabViewDlg(this);
    StabViewDlg * pStabView = m_pStabView;
    m_pctrlStabViewWidget = new QDockWidget(tr("Stability"), this);
    m_pctrlStabViewWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_pctrlStabViewWidget);
    m_pctrlStabViewWidget->setWidget(pStabView);
    m_pctrlStabViewWidget->setVisible(false);
    m_pctrlStabViewWidget->setFloating(true);
    m_pctrlStabViewWidget->move(60,60);

    m_pctrlCentralWidget = new QStackedWidget;
    m_pctrlCentralWidget->addWidget(&m_VoidWidget);
    m_pctrlCentralWidget->addWidget(m_p2dWidget);
    m_pctrlCentralWidget->addWidget(m_pgl3dMiarexView);
    m_pctrlCentralWidget->addWidget(m_pDirect2dWidget);
    m_pctrlCentralWidget->addWidget(m_pXDirectTileWidget);
    m_pctrlCentralWidget->addWidget(m_pMiarexTileWidget);

    setCentralWidget(m_pctrlCentralWidget);
    setMainFrameCentralWidget();

    m_pAFoil  = new AFoil(this);
    m_pAFoil->m_p2DWidget = m_pDirect2dWidget;
    connect(m_pDirect2dWidget, SIGNAL(objectModified()), m_pAFoil, SLOT(onSplinesModified()));

    m_pctrlAFoilWidget->setWidget(m_pAFoil);
    m_pctrlAFoilWidget->setVisible(false);

    m_p2dWidget->m_pXInverse  = m_pXInverse;
    m_p2dWidget->m_pMainFrame = this;

    m_pMiarex->m_pgl3dMiarexView = m_pgl3dMiarexView;

    m_pXDirect->m_poaFoil  = Objects2d::pOAFoil();
    m_pXDirect->m_poaPolar = Objects2d::pOAPolar();
    m_pXDirect->m_poaOpp   = Objects2d::pOAOpp();
    m_pXDirect->m_pOpPointWidget = m_pXDirectTileWidget->opPointWidget();

    m_pAFoil->initDialog(m_pDirect2dWidget, Objects2d::pOAFoil(), &m_pXDirect->m_XFoil);

    XInverse::s_p2dWidget        = m_p2dWidget;
    m_pXInverse->s_pMainFrame       = this;
    m_pXInverse->m_pXFoil           = &m_pXDirect->m_XFoil;

    EditObjectDelegate::s_poaFoil = Objects2d::pOAFoil();

    gl3dMiarexView::s_pMiarex = m_pMiarex;

    WingWidget::s_pMiarex         = m_pMiarex;
    XFoilAnalysisDlg::s_pXDirect  = m_pXDirect;
    NacaFoilDlg::s_pXFoil         = &m_pXDirect->m_XFoil;
    InterpolateFoilsDlg::s_pXFoil = &m_pXDirect->m_XFoil;
    CAddDlg::s_pXFoil             = &m_pXDirect->m_XFoil;
    TwoDPanelDlg::s_pXFoil        = &m_pXDirect->m_XFoil;
    FoilGeomDlg::s_pXFoil         = &m_pXDirect->m_XFoil;
    TEGapDlg::s_pXFoil            = &m_pXDirect->m_XFoil;
    LEDlg::s_pXFoil               = &m_pXDirect->m_XFoil;
    BatchDlg::s_pXDirect          = m_pXDirect;
    BatchThreadDlg::s_pXDirect    = m_pXDirect;

    GraphTileWidget::s_pMainFrame = this;
    GraphTileWidget::s_pMiarex    = m_pMiarex;
    GraphTileWidget::s_pXDirect   = m_pXDirect;

    LegendWidget::s_pMainFrame = this;
    LegendWidget::s_pMiarex    = m_pMiarex;
    LegendWidget::s_pXDirect   = m_pXDirect;


    m_pMiarex->connectSignals();
}



void MainFrame::createMenus()
{
    m_pFileMenu = menuBar()->addMenu(tr("File"));
    {
        m_pFileMenu->addAction(m_pNewProjectAct);
        m_pFileMenu->addAction(m_pOpenAct);
        m_pFileMenu->addAction(m_pLoadLastProjectAction);
        m_pFileMenu->addAction(m_pInsertAct);
        m_pFileMenu->addAction(m_pCloseProjectAct);
        m_pFileMenu->addSeparator();
        m_pFileMenu->addAction(m_pSaveAct);
        m_pFileMenu->addAction(m_pSaveProjectAsAct);
        m_pFileMenu->addSeparator();

        m_pFileMenu->addAction(m_pExecPyScript);
        m_pFileMenu->addSeparator();

        m_pFileMenu->addAction(m_pOnAFoilAct);
        m_pFileMenu->addAction(m_pOnXInverseAct);
        m_pFileMenu->addAction(m_pOnXDirectAct);
        m_pFileMenu->addAction(m_pOnMiarexAct);
        m_pSeparatorAct = m_pFileMenu->addSeparator();
        for (int i = 0; i < MAXRECENTFILES; ++i)
            m_pFileMenu->addAction(m_pRecentFileActs[i]);
        m_pFileMenu->addSeparator();
        m_pFileMenu->addAction(m_pExitAct);
        updateRecentFileActions();
    }

    m_pOptionsMenu = menuBar()->addMenu(tr("O&ptions"));
    {
        m_pOptionsMenu->addSeparator();
        m_pOptionsMenu->addAction(m_pPreferencesAct);
        m_pOptionsMenu->addSeparator();
        m_pOptionsMenu->addAction(m_pRestoreToolbarsAct);
        m_pOptionsMenu->addSeparator();
        m_pOptionsMenu->addAction(m_pResetSettingsAct);
    }

    m_pGraphMenu = menuBar()->addMenu(tr("Graphs"));
    {
        for(int ig=0; ig<MAXGRAPHS; ig++)
            m_pGraphMenu->addAction(m_pSingleGraph[ig]);
        m_pGraphMenu->addSeparator();
        m_pGraphMenu->addAction(m_pTwoGraphs);
        m_pGraphMenu->addAction(m_pFourGraphs);
        m_pGraphMenu->addAction(m_pAllGraphs);
        m_pGraphMenu->addSeparator();
        m_pGraphMenu->addAction(m_pAllGraphsSettings);
        m_pGraphMenu->addAction(m_pAllGraphsScalesAct);
        m_pGraphMenu->addSeparator();
        m_pGraphMenu->addAction(m_pHighlightOppAct);
        m_pGraphMenu->addSeparator();
        m_pGraphMenu->addAction(m_pShowMousePosAct);

    }

    m_pHelpMenu = menuBar()->addMenu(tr("?"));
    {
        m_pHelpMenu->addAction(m_pOpenGLAct);
        m_pHelpMenu->addAction(m_pAboutQtAct);
        m_pHelpMenu->addAction(m_pAboutAct);
        m_pHelpMenu->addAction(m_pCheckForUpdates);
    }

    //Create Application-Specific Menus
    createXDirectMenus();
    createXInverseMenus();
    createMiarexMenus();
    createAFoilMenus();
}



void MainFrame::createGraphActions()
{
    for (short ig=0; ig<MAXGRAPHS; ++ig)
    {
        m_pSingleGraph[ig] = new QAction(tr("Graph")+QString(" %1").arg(ig+1), this);
        m_pSingleGraph[ig]->setShortcut(Qt::Key_1+ig);
        m_pSingleGraph[ig]->setData(ig);
        m_pSingleGraph[ig]->setCheckable(true);
        connect(m_pSingleGraph[ig], SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onSingleGraph()));
        connect(m_pSingleGraph[ig], SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onSingleGraph()));
    }
    //    m_pTwoGraphs = new QAction(tr("Two Graphs")+"\t(T)", this);
    m_pTwoGraphs = new QAction(tr("Two Graphs"), this);
    m_pTwoGraphs->setShortcut(Qt::Key_T);
    m_pTwoGraphs->setStatusTip(tr("Display the first two graphs"));
    m_pTwoGraphs->setCheckable(true);
    connect(m_pTwoGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onTwoGraphs()));
    connect(m_pTwoGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onTwoGraphs()));

    //    m_pFourGraphs = new QAction(tr("Four Graphs")+"\t(F)", this);
    m_pFourGraphs = new QAction(tr("Four Graphs"), this);
    m_pFourGraphs->setShortcut(Qt::Key_F);
    m_pFourGraphs->setStatusTip(tr("Display four graphs"));
    m_pFourGraphs->setCheckable(true);
    connect(m_pFourGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onFourGraphs()));
    connect(m_pFourGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onFourGraphs()));

    m_pAllGraphs = new QAction(tr("All Graphs"), this);
    m_pAllGraphs->setShortcut(Qt::Key_A);
    m_pAllGraphs->setStatusTip(tr("Display four graphs"));
    m_pAllGraphs->setCheckable(true);
    connect(m_pAllGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphs()));
    connect(m_pAllGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphs()));

    m_pGraphDlgAct = new QAction(tr("Define Graph Settings"), this);
    m_pGraphDlgAct->setShortcut(Qt::Key_G);
    m_pGraphDlgAct->setStatusTip(tr("Define the settings for the selected graph"));
    connect(m_pGraphDlgAct, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onCurGraphSettings()));
    connect(m_pGraphDlgAct, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onCurGraphSettings()));

    m_pAllGraphsScalesAct = new QAction(tr("Reset All Graph Scales"), this);
    m_pAllGraphsScalesAct->setStatusTip(tr("Reset the scales of all four polar graphs"));
    connect(m_pAllGraphsScalesAct, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphScales()));
    connect(m_pAllGraphsScalesAct, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphScales()));

    m_pAllGraphsSettings = new QAction(tr("All Graph Settings"), this);
    m_pAllGraphsSettings->setStatusTip(tr("Define the settings of all graphs"));
    connect(m_pAllGraphsSettings, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphSettings()));
    connect(m_pAllGraphsSettings, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphSettings()));

    m_pShowMousePosAct     = new QAction(tr("Display mouse coordinates"), this);
    m_pShowMousePosAct->setCheckable(true);
    m_pShowMousePosAct->setChecked(s_bShowMousePos);
    m_pShowMousePosAct->setStatusTip(tr("Display the coordinates of the mouse on the top right corner of the graph"));
    connect(m_pShowMousePosAct, SIGNAL(triggered()), this, SLOT(onShowMousePos()));

}


void MainFrame::createMiarexActions()
{
    //groups view actions so their exclusive state is properly toggled in menu and toolbar
    m_pMiarexViewActGroup = new QActionGroup(this);
    {
        m_pWOppAct = new QAction(QIcon(":/images/OnWOppView.png"), tr("OpPoint View")+"\tF5", this);
        m_pWOppAct->setCheckable(true);
        m_pWOppAct->setStatusTip(tr("Switch to the Operating point view"));
        m_pWOppAct->setActionGroup(m_pMiarexViewActGroup);
        //    WOppAct->setShortcut(Qt::Key_F5);
        connect(m_pWOppAct, SIGNAL(triggered()), m_pMiarex, SLOT(onWOppView()));

        m_pWPolarAct = new QAction(QIcon(":/images/OnPolarView.png"), tr("Polar View")+"\tF8", this);
        m_pWPolarAct->setCheckable(true);
        m_pWPolarAct->setStatusTip(tr("Switch to the Polar view"));
        m_pWPolarAct->setActionGroup(m_pMiarexViewActGroup);
        //    WPolarAct->setShortcut(Qt::Key_F8);
        connect(m_pWPolarAct, SIGNAL(triggered()), m_pMiarex, SLOT(onWPolarView()));

        m_pStabTimeAct = new QAction(QIcon(":/images/OnStabView.png"),tr("Time Response View")+"\tShift+F8", this);
        m_pStabTimeAct->setCheckable(true);
        m_pStabTimeAct->setStatusTip(tr("Switch to stability analysis post-processing"));
        m_pStabTimeAct->setActionGroup(m_pMiarexViewActGroup);
        //    StabTimeAct->setShortcut(tr("Shift+F8"));
        connect(m_pStabTimeAct, SIGNAL(triggered()), m_pMiarex, SLOT(onStabTimeView()));

        m_pRootLocusAct = new QAction(QIcon(":/images/OnRootLocus.png"),tr("Root Locus View")+"\tCtrl+F8", this);
        m_pRootLocusAct->setCheckable(true);
        m_pRootLocusAct->setStatusTip(tr("Switch to root locus view"));
        m_pRootLocusAct->setActionGroup(m_pMiarexViewActGroup);
        connect(m_pRootLocusAct, SIGNAL(triggered()), m_pMiarex, SLOT(onRootLocusView()));

        m_pW3DAct = new QAction(QIcon(":/images/On3DView.png"), tr("3D View")+"\tF4", this);
        m_pW3DAct->setCheckable(true);
        m_pW3DAct->setStatusTip(tr("Switch to the 3D view"));
        m_pW3DAct->setActionGroup(m_pMiarexViewActGroup);
        connect(m_pW3DAct, SIGNAL(triggered()), m_pMiarex, SLOT(on3DView()));
        if(!hasOpenGL()) m_pW3DAct->setEnabled(false);
        //end action group

        m_pCpViewAct = new QAction(QIcon(":/images/OnCpView.png"), tr("Cp View")+"\tF9", this);
        m_pCpViewAct->setCheckable(true);
        m_pCpViewAct->setStatusTip(tr("Switch to the Cp view"));
        m_pCpViewAct->setActionGroup(m_pMiarexViewActGroup);
        connect(m_pCpViewAct, SIGNAL(triggered()), m_pMiarex, SLOT(onCpView()));
    }

    m_pW3DPrefsAct = new QAction(tr("3D View Preferences"), this);
    m_pW3DPrefsAct->setStatusTip(tr("Define the preferences for the 3D view"));
    connect(m_pW3DPrefsAct, SIGNAL(triggered()), m_pMiarex, SLOT(on3DPrefs()));

    m_pMiarexPolarFilter = new QAction(tr("Polar Filter"), this);
    m_pMiarexPolarFilter->setStatusTip(tr("Define which type of polars should be shown or hidden"));
    connect(m_pMiarexPolarFilter, SIGNAL(triggered()), m_pMiarex, SLOT(onPolarFilter()));

    m_pReset3DScale = new QAction(tr("Reset scale")+"\tR", this);
    m_pReset3DScale->setStatusTip(tr("Resets the display scale so that the plane fits in the window"));
    connect(m_pReset3DScale, SIGNAL(triggered()), m_pMiarex, SLOT(on3DResetScale()));

    m_pShowFlapMoments = new QAction(tr("Show flap moments"), this);
    m_pShowFlapMoments->setCheckable(true);
    m_pShowFlapMoments->setStatusTip(tr("Display the flap moment values together with the other operating point results"));
    connect(m_pShowFlapMoments, SIGNAL(triggered()), m_pMiarex, SLOT(onShowFlapMoments()));

    m_pW3DScalesAct = new QAction(tr("3D Scales"), this);
    m_pW3DScalesAct->setStatusTip(tr("Define the scales for the 3D display of lift, moment, drag, and downwash"));
    m_pW3DScalesAct->setCheckable(true);
    connect(m_pW3DScalesAct, SIGNAL(triggered()), m_pMiarex, SLOT(onGL3DScale()));

    m_pW3DLightAct = new QAction(tr("3D Light Options"), this);
    m_pW3DLightAct->setStatusTip(tr("Define the light options in 3D view"));
    connect(m_pW3DLightAct, SIGNAL(triggered()), m_pMiarex, SLOT(onSetupLight()));

    m_pDefinePlaneAct = new QAction(tr("Define a New Plane")/*+"\tF3"*/, this);
    m_pDefinePlaneAct->setShortcut(Qt::Key_F3);
    m_pDefinePlaneAct->setStatusTip(tr("Shows a dialogbox to create a new plane definition"));
    connect(m_pDefinePlaneAct, SIGNAL(triggered()), m_pMiarex, SLOT(onNewPlane()));

    m_pDefinePlaneObjectAct = new QAction(tr("Define (Advanced users)")/*+"\tF3"*/, this);
    m_pDefinePlaneObjectAct->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F3));
    m_pDefinePlaneObjectAct->setStatusTip(tr("Shows a dialogbox to create a new plane definition"));
    connect(m_pDefinePlaneObjectAct, SIGNAL(triggered()), m_pMiarex, SLOT(onNewPlaneObject()));

    m_pEditPlaneAct = new QAction(tr("Edit"), this);
    m_pEditPlaneAct->setStatusTip(tr("Shows a form to edit the currently selected plane"));
    m_pEditPlaneAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connect(m_pEditPlaneAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurPlane()));

    m_pEditObjectAct = new QAction(tr("Edit (advanced users)"), this);
    m_pEditObjectAct->setStatusTip(tr("Shows a form to edit the currently selected plane"));
    m_pEditObjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT+Qt::Key_P));
    connect(m_pEditObjectAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurObject()));

    m_pEditWingAct = new QAction(tr("Edit wing"), this);
    m_pEditWingAct->setStatusTip(tr("Shows a form to edit the wing of the currently selected plane"));
    m_pEditWingAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
    m_pEditWingAct->setData(0);
    connect(m_pEditWingAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurWing()));

    m_pEditStabAct = new QAction(tr("Edit elevator"), this);
    m_pEditStabAct->setData(2);
    m_pEditStabAct->setShortcut(Qt::CTRL + Qt::Key_E);
    connect(m_pEditStabAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurWing()));

    m_pEditFinAct = new QAction(tr("Edit fin"), this);
    m_pEditFinAct->setData(3);
    m_pEditFinAct->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(m_pEditFinAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurWing()));

    m_pEditBodyAct = new QAction(tr("Edit body"), this);
    m_pEditBodyAct->setStatusTip(tr("Shows a form to edit the body of the currently selected plane"));
    m_pEditBodyAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    connect(m_pEditBodyAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurBody()));

    m_pEditBodyObjectAct= new QAction(tr("Edit body (advanced users)"), this);
    m_pEditBodyObjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_B));
    connect(m_pEditBodyObjectAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurBodyObject()));

    m_pRenameCurPlaneAct = new QAction(tr("Rename")+"\tF2", this);
    m_pRenameCurPlaneAct->setStatusTip(tr("Rename the currently selected object"));
    connect(m_pRenameCurPlaneAct, SIGNAL(triggered()), m_pMiarex, SLOT(onRenameCurPlane()));

    m_pExporttoAVL = new QAction(tr("Export to AVL"), this);
    m_pExporttoAVL->setStatusTip(tr("Export the current plane or wing to a text file in the format required by AVL"));
    connect(m_pExporttoAVL, SIGNAL(triggered()), m_pMiarex, SLOT(onExporttoAVL()));

    m_pExporttoSTL = new QAction(tr("Export to STL"), this);
    m_pExporttoSTL->setStatusTip(tr("Export the current wing to a file in the STL format"));
    connect(m_pExporttoSTL, SIGNAL(triggered()), m_pMiarex, SLOT(onExporttoSTL()));

    m_pExportCurWOpp = new QAction(tr("Export"), this);
    m_pExportCurWOpp->setStatusTip(tr("Export the current operating point to a text or csv file"));
    connect(m_pExportCurWOpp, SIGNAL(triggered()), m_pMiarex, SLOT(onExportCurPOpp()));

    m_pScaleWingAct = new QAction(tr("Scale Wing"), this);
    m_pScaleWingAct->setStatusTip(tr("Scale the dimensions of the currently selected wing"));
    connect(m_pScaleWingAct, SIGNAL(triggered()), m_pMiarex, SLOT(onScaleWing()));

    m_pManagePlanesAct = new QAction(tr("Manage objects"), this);
    m_pManagePlanesAct->setStatusTip(tr("Rename or delete the planes and wings stored in the database"));
    m_pManagePlanesAct->setShortcut(Qt::Key_F7);
    connect(m_pManagePlanesAct, SIGNAL(triggered()), m_pMiarex, SLOT(onManagePlanes()));

    m_pImportWPolars = new QAction(tr("Import Polar(s)"), this);
    m_pImportWPolars->setStatusTip(tr("Import polar(s) from text file(s)"));
    connect(m_pImportWPolars, SIGNAL(triggered()), m_pMiarex, SLOT(onImportWPolars()));

    m_pExportWPolars = new QAction(tr("Export all polars"), this);
    m_pExportWPolars->setStatusTip(tr("Export polar(s) to text file(s)"));
    connect(m_pExportWPolars, SIGNAL(triggered()), m_pMiarex, SLOT(onExportWPolars()));

    m_pPlaneInertia = new QAction(tr("Define Inertia")+"\tF12", this);
    m_pPlaneInertia->setStatusTip(tr("Define the inertia for the current plane or wing"));
    connect(m_pPlaneInertia, SIGNAL(triggered()), m_pMiarex, SLOT(onPlaneInertia()));

    m_pShowCurWOppOnly = new QAction(tr("Show Current OpPoint Only"), this);
    m_pShowCurWOppOnly->setStatusTip(tr("Hide all the curves except for the one corresponding to the currently selected operating point"));
    m_pShowCurWOppOnly->setCheckable(true);
    connect(m_pShowCurWOppOnly, SIGNAL(triggered()), m_pMiarex, SLOT(onCurWOppOnly()));

    m_pShowAllWOpps = new QAction(tr("Show All OpPoints"), this);
    m_pShowAllWOpps->setStatusTip(tr("Show the graph curves of all operating points"));
    connect(m_pShowAllWOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onShowAllWOpps()));

    m_pHideAllWOpps = new QAction(tr("Hide All OpPoints"), this);
    m_pHideAllWOpps->setStatusTip(tr("Hide the graph curves of all operating points"));
    connect(m_pHideAllWOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onHideAllWOpps()));

    m_pDeleteAllWOpps = new QAction(tr("Delete All OpPoints"), this);
    m_pDeleteAllWOpps->setStatusTip(tr("Delete all the operating points of all planes and polars"));
    connect(m_pDeleteAllWOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onDeleteAllWOpps()));

    m_pShowAllWPlrOpps = new QAction(tr("Show Associated OpPoints"), this);
    m_pShowAllWPlrOpps->setStatusTip(tr("Show the curves of all the operating points of the currently selected polar"));
    connect(m_pShowAllWPlrOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onShowAllWPlrOpps()));

    m_pHideAllWPlrOpps = new QAction(tr("Hide Associated OpPoints"), this);
    m_pHideAllWPlrOpps->setStatusTip(tr("Hide the curves of all the operating points of the currently selected polar"));
    connect(m_pHideAllWPlrOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onHideAllWPlrOpps()));

    m_pShowWPlrOppsOnly = new QAction(tr("Show Only Associated OpPoints"), this);
    connect(m_pShowWPlrOppsOnly, SIGNAL(triggered()), m_pMiarex, SLOT(onShowWPolarOppsOnly()));

    m_pDeleteAllWPlrOpps = new QAction(tr("Delete Associated OpPoints"), this);
    m_pDeleteAllWPlrOpps->setStatusTip(tr("Delete all the operating points of the currently selected polar"));
    connect(m_pDeleteAllWPlrOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onDeleteAllWPlrOpps()));

    m_pShowTargetCurve = new QAction(tr("Show Target Curve"), this);
    m_pShowTargetCurve->setCheckable(false);
    connect(m_pShowTargetCurve, SIGNAL(triggered()), m_pMiarex, SLOT(onShowTargetCurve()));

    m_pShowXCmRefLocation = new QAction(tr("Show XCG location"), this);
    m_pShowXCmRefLocation->setStatusTip(tr("Show the position of the center of gravity defined in the analysis"));
    m_pShowXCmRefLocation->setCheckable(true);
    connect(m_pShowXCmRefLocation, SIGNAL(triggered()), m_pMiarex, SLOT(onShowXCmRef()));

    m_pShowStabCurve = new QAction(tr("Show Elevator Curve"), this);
    m_pShowStabCurve->setStatusTip(tr("Show the graph curves for the elevator"));
    m_pShowStabCurve->setCheckable(true);
    connect(m_pShowStabCurve, SIGNAL(triggered()), m_pMiarex, SLOT(onStabCurve()));

    m_pShowFinCurve = new QAction(tr("Show Fin Curve"), this);
    m_pShowFinCurve->setStatusTip(tr("Show the graph curves for the fin"));
    m_pShowFinCurve->setCheckable(true);
    connect(m_pShowFinCurve, SIGNAL(triggered()), m_pMiarex, SLOT(onFinCurve()));

    m_pShowWing2Curve = new QAction(tr("Show Second Wing Curve"), this);
    m_pShowWing2Curve->setStatusTip(tr("Show the graph curves for the second wing"));
    m_pShowWing2Curve->setCheckable(true);
    connect(m_pShowWing2Curve, SIGNAL(triggered()), m_pMiarex, SLOT(onWing2Curve()));

    m_pDefineWPolar = new QAction(tr("Define an Analysis")+" \tF6", this);
    m_pDefineWPolar->setStatusTip(tr("Define an analysis for the current wing or plane"));
    connect(m_pDefineWPolar, SIGNAL(triggered()), m_pMiarex, SLOT(onDefineWPolar()));

    m_pDefineWPolarObjectAct = new QAction(tr("Define an Analysis (advanced users)")+" \tCtrl+F6", this);
    m_pDefineWPolarObjectAct->setStatusTip(tr("Shows a form to edit a new polar object"));
    connect(m_pDefineWPolarObjectAct, SIGNAL(triggered()), m_pMiarex, SLOT(onDefineWPolarObject()));

    m_pEditWPolarAct = new QAction(tr("Edit"), this);
    m_pEditWPolarAct->setStatusTip(tr("Modify the analysis parameters of this polar"));
    connect(m_pEditWPolarAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurWPolar()));

    m_pEditWPolarObjectAct = new QAction(tr("Edit object (advanced users)"), this);
    m_pEditWPolarObjectAct->setStatusTip(tr("Shows a form to edit the currently selected polar"));
    connect(m_pEditWPolarObjectAct, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurWPolarObject()));

    m_pEditWPolarPts = new QAction(tr("Edit data points"), this);
    m_pEditWPolarPts->setStatusTip(tr("Modify the data points of this polar"));
    connect(m_pEditWPolarPts, SIGNAL(triggered()), m_pMiarex, SLOT(onEditCurWPolarPts()));

    m_pDefineStabPolar = new QAction(tr("Define a Stability Analysis")+"\tShift+F6", this);
    m_pDefineStabPolar->setStatusTip(tr("Define a stability analysis for the current wing or plane"));
    connect(m_pDefineStabPolar, SIGNAL(triggered()), m_pMiarex, SLOT(onDefineStabPolar()));

    m_pHidePlaneWPlrs = new QAction(tr("Hide Associated Polars"), this);
    m_pHidePlaneWPlrs->setStatusTip(tr("Hide all the polar curves associated to the currently selected wing or plane"));
    connect(m_pHidePlaneWPlrs, SIGNAL(triggered()), m_pMiarex, SLOT(onHidePlaneWPolars()));

    m_pShowPlaneWPlrsOnly = new QAction(tr("Show only associated Polars"), this);
    m_pShowPlaneWPlrsOnly->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    connect(m_pShowPlaneWPlrsOnly, SIGNAL(triggered()), m_pMiarex, SLOT(onShowPlaneWPolarsOnly()));

    m_pShowPlaneWPlrs = new QAction(tr("Show Associated Polars"), this);
    m_pShowPlaneWPlrs->setStatusTip(tr("Show all the polar curves associated to the currently selected wing or plane"));
    connect(m_pShowPlaneWPlrs, SIGNAL(triggered()), m_pMiarex, SLOT(onShowPlaneWPolars()));

    m_pDeletePlaneWPlrs = new QAction(tr("Delete Associated Polars"), this);
    m_pDeletePlaneWPlrs->setStatusTip(tr("Delete all the polars associated to the currently selected wing or plane"));
    connect(m_pDeletePlaneWPlrs, SIGNAL(triggered()), m_pMiarex, SLOT(onDeletePlaneWPolars()));

    m_pHideAllWPlrs = new QAction(tr("Hide All Polars"), this);
    m_pHideAllWPlrs->setStatusTip(tr("Hide all the polar curves of all wings and planes"));
    connect(m_pHideAllWPlrs, SIGNAL(triggered()), m_pMiarex, SLOT(onHideAllWPolars()));

    m_pShowAllWPlrs = new QAction(tr("Show All Polars"), this);
    m_pShowAllWPlrs->setStatusTip(tr("Show all the polar curves of all wings and planes"));
    connect(m_pShowAllWPlrs, SIGNAL(triggered()), m_pMiarex, SLOT(onShowAllWPolars()));

    m_pHidePlaneWOpps = new QAction(tr("Hide Associated OpPoints"), this);
    m_pHidePlaneWOpps->setStatusTip(tr("Hide all the operating point curves of the currently selected wing or plane"));
    connect(m_pHidePlaneWOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onHidePlaneOpps()));

    m_pShowPlaneWOpps = new QAction(tr("Show Associated OpPoints"), this);
    m_pShowPlaneWOpps->setStatusTip(tr("Show all the operating point curves of the currently selected wing or plane"));
    connect(m_pShowPlaneWOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onShowPlaneOpps()));

    m_pDeletePlaneWOpps = new QAction(tr("Delete Associated OpPoints"), this);
    m_pDeletePlaneWOpps->setStatusTip(tr("Delete all the operating points of the currently selected wing or plane"));
    connect(m_pDeletePlaneWOpps, SIGNAL(triggered()), m_pMiarex, SLOT(onDeletePlanePOpps()));

    m_pDeleteCurPlane = new QAction(tr("Delete"), this);
    m_pDeleteCurPlane->setStatusTip(tr("Delete the currently selected wing or plane"));
    connect(m_pDeleteCurPlane, SIGNAL(triggered()), m_pMiarex, SLOT(onDeleteCurPlane()));

    m_pDuplicateCurPlane = new QAction(tr("Duplicate"), this);
    m_pDuplicateCurPlane->setStatusTip(tr("Duplicate the currently selected wing or plane"));
    connect(m_pDuplicateCurPlane, SIGNAL(triggered()), m_pMiarex, SLOT(onDuplicateCurPlane()));

    m_pSavePlaneAsProjectAct = new QAction(tr("Save as Project"), this);
    m_pSavePlaneAsProjectAct->setStatusTip(tr("Save the currently selected wing or plane as a new separate project"));
    connect(m_pSavePlaneAsProjectAct, SIGNAL(triggered()), this, SLOT(onSavePlaneAsProject()));

    m_pRenameCurWPolar = new QAction(tr("Rename")+"\tShift+F2", this);
    m_pRenameCurWPolar->setStatusTip(tr("Rename the currently selected polar"));
    connect(m_pRenameCurWPolar, SIGNAL(triggered()), m_pMiarex, SLOT(onRenameCurWPolar()));

    m_pExportCurWPolar = new QAction(tr("Export results"), this);
    m_pExportCurWPolar->setStatusTip(tr("Export the currently selected polar to a text or csv file"));
    connect(m_pExportCurWPolar, SIGNAL(triggered()), m_pMiarex, SLOT(onExportCurWPolar()));

    m_pResetCurWPolar = new QAction(tr("Reset"), this);
    m_pResetCurWPolar->setStatusTip(tr("Delete all the points of the currently selected polar, but keep the analysis settings"));
    connect(m_pResetCurWPolar, SIGNAL(triggered()), m_pMiarex, SLOT(onResetCurWPolar()));

    m_pDeleteCurWPolar = new QAction(tr("Delete"), this);
    m_pDeleteCurWPolar->setStatusTip(tr("Delete the currently selected polar"));
    connect(m_pDeleteCurWPolar, SIGNAL(triggered()), m_pMiarex, SLOT(onDeleteCurWPolar()));

    m_pDeleteCurWOpp = new QAction(tr("Delete"), this);
    m_pDeleteCurWOpp->setStatusTip(tr("Delete the currently selected operating point"));
    connect(m_pDeleteCurWOpp, SIGNAL(triggered()), m_pMiarex, SLOT(onDeleteCurWOpp()));

    m_pAadvancedSettings = new QAction(tr("Advanced Settings"), this);
    m_pAadvancedSettings->setStatusTip(tr("Define the settings for LLT, VLM and Panel analysis"));
    connect(m_pAadvancedSettings, SIGNAL(triggered()), m_pMiarex, SLOT(onAdvancedSettings()));

    m_pShowPolarProps = new QAction(tr("Properties"), this);
    m_pShowPolarProps->setStatusTip(tr("Show the properties of the currently selected polar"));
    m_pShowPolarProps->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
    connect(m_pShowPolarProps, SIGNAL(triggered()), m_pMiarex, SLOT(onWPolarProperties()));

    m_pShowWOppProps = new QAction(tr("Properties"), this);
    m_pShowWOppProps->setStatusTip(tr("Show the properties of the currently selected operating point"));
    m_pShowWOppProps->setShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Return));
    connect(m_pShowWOppProps, SIGNAL(triggered()), m_pMiarex, SLOT(onPlaneOppProperties()));

    m_pExportPlaneToXML = new QAction(tr("Export to xml file"), this);
    connect(m_pExportPlaneToXML, SIGNAL(triggered()), m_pMiarex, SLOT(onExportPlanetoXML()));

    m_pImportPlaneFromXml= new QAction(tr("Import plane(s) from xml file(s)"), this);
    connect(m_pImportPlaneFromXml, SIGNAL(triggered()), m_pMiarex, SLOT(onImportPlanesfromXML()));

    m_pExportAnalysisToXML = new QAction(tr("Export analysis to xml file"), this);
    connect(m_pExportAnalysisToXML, SIGNAL(triggered()), m_pMiarex, SLOT(onExportAnalysisToXML()));

    m_pImportAnalysisFromXml= new QAction(tr("Import analysis from xml file"), this);
    m_pImportAnalysisFromXml->setStatusTip(tr("Import analysis definition(s) from XML file(s)"));
    connect(m_pImportAnalysisFromXml, SIGNAL(triggered()), m_pMiarex, SLOT(onImportAnalysisFromXML()));
}


void MainFrame::createMiarexMenus()
{
    //MainMenu for Miarex Application
    m_pMiarexViewMenu = menuBar()->addMenu(tr("View"));
    {
        // all Miarex view actions are in the group...
        m_pMiarexViewMenu->addActions(m_pMiarexViewActGroup->actions());

        m_pMiarexViewMenu->addSeparator();
        m_pMiarexViewMenu->addAction(m_pW3DPrefsAct);
        m_pMiarexViewMenu->addAction(m_pW3DLightAct);
        m_pMiarexViewMenu->addAction(m_pW3DScalesAct);
        m_pMiarexViewMenu->addSeparator();
        m_pMiarexViewMenu->addAction(m_pSaveViewToImageFileAct);
    }


    m_pPlaneMenu = menuBar()->addMenu(tr("Plane"));
    {
        m_pPlaneMenu->addAction(m_pDefinePlaneAct);
        m_pPlaneMenu->addAction(m_pDefinePlaneObjectAct);
        m_pPlaneMenu->addAction(m_pManagePlanesAct);
        m_pCurrentPlaneMenu = m_pPlaneMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu->addSeparator();
            m_pCurrentPlaneMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu->addAction(m_pDeletePlaneWOpps);
        }
        m_pPlaneMenu->addAction(m_pImportPlaneFromXml);
    }

    m_pMiarexWPlrMenu = menuBar()->addMenu(tr("Polars"));
    {
        m_pCurWPlrMenu = m_pMiarexWPlrMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu->addSeparator();
            m_pCurWPlrMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu->addSeparator();
            m_pCurWPlrMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu->addAction(m_pExportAnalysisToXML);
        }

        m_pMiarexWPlrMenu->addSeparator();
        m_pMiarexWPlrMenu->addAction(m_pImportWPolars);
        m_pMiarexWPlrMenu->addAction(m_pExportWPolars);
        m_pMiarexWPlrMenu->addSeparator();
        m_pMiarexWPlrMenu->addAction(m_pMiarexPolarFilter);
        m_pMiarexWPlrMenu->addSeparator();
        m_pMiarexWPlrMenu->addAction(m_pHideAllWPlrs);
        m_pMiarexWPlrMenu->addAction(m_pShowAllWPlrs);
        m_pMiarexWPlrMenu->addSeparator();
    }

    m_pMiarexWOppMenu = menuBar()->addMenu(tr("OpPoint"));
    {
        m_pCurWOppMenu = m_pMiarexWOppMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu->addAction(m_pDeleteCurWOpp);
        }
        m_pMiarexWOppMenu->addSeparator();
        m_pMiarexWOppMenu->addAction(m_pShowCurWOppOnly);
        m_pMiarexWOppMenu->addAction(m_pShowAllWOpps);
        m_pMiarexWOppMenu->addAction(m_pHideAllWOpps);
        m_pMiarexWOppMenu->addAction(m_pDeleteAllWOpps);
        m_pMiarexWOppMenu->addSeparator();
        m_pMiarexWOppMenu->addAction(m_pShowTargetCurve);
        m_pMiarexWOppMenu->addAction(m_pShowXCmRefLocation);
        m_pMiarexWOppMenu->addAction(m_pShowWing2Curve);
        m_pMiarexWOppMenu->addAction(m_pShowStabCurve);
        m_pMiarexWOppMenu->addAction(m_pShowFinCurve);
        m_pMiarexWOppMenu->addAction(m_pShowFlapMoments);
    }

    //Miarex Analysis Menu
    m_pMiarexAnalysisMenu  = menuBar()->addMenu(tr("Analysis"));
    {
        m_pMiarexAnalysisMenu->addAction(m_pDefineWPolar);
        m_pMiarexAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
        m_pMiarexAnalysisMenu->addAction(m_pDefineStabPolar);
        m_pMiarexAnalysisMenu->addAction(m_pImportAnalysisFromXml);
        m_pMiarexAnalysisMenu->addSeparator();
        m_pMiarexAnalysisMenu->addAction(m_pViewLogFile);
        m_pMiarexAnalysisMenu->addAction(m_pAadvancedSettings);
    }


    //WOpp View Context Menu
    m_pWOppCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentPlaneMenu_WOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WOppCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDeletePlaneWOpps);
        }
        //m_pWOppCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWOppCtxMenu->addSeparator();
        m_pCurWPlrMenu_WOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_WOppCtxMenu->addSeparator();
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu_WOppCtxMenu->addSeparator();
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pExportAnalysisToXML);
        }

        //m_pWOppCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWOppCtxMenu->addSeparator();
        m_pCurWOppMenu_WOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_WOppCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_WOppCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_WOppCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pWOppCtxMenu->addMenu(m_pCurWOppMenu);
        m_pWOppCtxMenu->addSeparator();
        m_pWOppCtxMenu->addAction(m_pShowCurWOppOnly);
        m_pWOppCtxMenu->addAction(m_pShowAllWOpps);
        m_pWOppCtxMenu->addAction(m_pHideAllWOpps);
        m_pWOppCtxMenu->addAction(m_pDeleteAllWOpps);
        m_pWOppCtxMenu->addSeparator();
        m_pWOppCtxMenu->addAction(m_pShowTargetCurve);
        m_pWOppCtxMenu->addAction(m_pShowXCmRefLocation);
        m_pWOppCtxMenu->addAction(m_pShowWing2Curve);
        m_pWOppCtxMenu->addAction(m_pShowStabCurve);
        m_pWOppCtxMenu->addAction(m_pShowFinCurve);
        m_pWOppCtxMenu->addAction(m_pShowFlapMoments);
        m_pWOppCtxMenu->addSeparator();
        QMenu *pCurGraphCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current Graph"));
        {
            pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
        }
        m_pWOppCtxMenu->addAction(m_pShowMousePosAct);
        m_pWOppCtxMenu->addSeparator();
        m_pWOppCtxMenu->addAction(m_pViewLogFile);
        m_pWOppCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }

    //WOpp View Context Menu
    m_pWCpCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentPlaneMenu_WCpCtxMenu = m_pWCpCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WCpCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDeletePlaneWOpps);
        }
        //m_pWCpCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWCpCtxMenu->addSeparator();
        m_pCurWPlrMenu_WCpCtxMenu = m_pWCpCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_WCpCtxMenu->addSeparator();
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu_WCpCtxMenu->addSeparator();
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pExportAnalysisToXML);
        }

        //m_pWCpCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWCpCtxMenu->addSeparator();
        m_pCurWOppMenu_WCpCtxMenu = m_pWCpCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_WCpCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_WCpCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_WCpCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pWCpCtxMenu->addMenu(m_pCurWOppMenu);
        m_pWCpCtxMenu->addSeparator();
        m_pWCpCtxMenu->addAction(m_pShowWing2Curve);
        m_pWCpCtxMenu->addAction(m_pShowStabCurve);
        m_pWCpCtxMenu->addAction(m_pShowFinCurve);
        m_pWCpCtxMenu->addSeparator();
        m_pWCpCtxMenu->addAction(m_pViewLogFile);
        m_pWCpCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }

    //WTime View Context Menu
    m_pWTimeCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentPlaneMenu_WTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WTimeCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDeletePlaneWOpps);
        }

        //m_pWTimeCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWTimeCtxMenu->addSeparator();
        m_pCurWPlrMenu_WTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_WTimeCtxMenu->addSeparator();
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu_WTimeCtxMenu->addSeparator();
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pExportAnalysisToXML);
        }
        //m_pWTimeCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWTimeCtxMenu->addSeparator();
        m_pCurWOppMenu_WTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_WTimeCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_WTimeCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_WTimeCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pWTimeCtxMenu->addMenu(m_pCurWOppMenu);
        m_pWTimeCtxMenu->addSeparator();
        QMenu *pCurGraphTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current Graph"));
        {
            pCurGraphTimeCtxMenu->addAction(m_pResetCurGraphScales);
            pCurGraphTimeCtxMenu->addAction(m_pCurGraphDlgAct);
            pCurGraphTimeCtxMenu->addAction(m_pExportCurGraphAct);
        }
        m_pWTimeCtxMenu->addSeparator();
        /*        m_pWTimeCtxMenu->addAction(m_pShowCurWOppOnly);
        m_pWTimeCtxMenu->addAction(m_pShowAllWOpps);
        m_pWTimeCtxMenu->addAction(m_pHideAllWOpps);
        m_pWTimeCtxMenu->addAction(m_pDeleteAllWOpps);
        m_pWTimeCtxMenu->addSeparator();*/
        m_pWTimeCtxMenu->addAction(m_pViewLogFile);
        m_pWTimeCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }

    //Polar View Context Menu
    m_pWPlrCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentPlaneMenu_WPlrCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WPlrCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDeletePlaneWOpps);
        }

        //m_pWPlrCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pWPlrCtxMenu->addSeparator();
        m_pCurWPlrMenu_WPlrCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addSeparator();
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu_WPlrCtxMenu->addSeparator();
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pExportAnalysisToXML);
        }
        //m_pWPlrCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pWPlrCtxMenu->addSeparator();
        m_pWPlrCtxMenu->addAction(m_pHideAllWPlrs);
        m_pWPlrCtxMenu->addAction(m_pShowAllWPlrs);
        m_pWPlrCtxMenu->addSeparator();
        QMenu *pCurGraphCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Current Graph"));
        {
            pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
            pCurGraphCtxMenu->addAction(m_pHighlightOppAct);
        }
        m_pWPlrCtxMenu->addAction(m_pShowMousePosAct);
        m_pWPlrCtxMenu->addSeparator();
        m_pWPlrCtxMenu->addAction(m_pViewLogFile);
        m_pWPlrCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }

    //W3D View Context Menu
    m_pW3DCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentPlaneMenu_W3DCtxMenu = m_pW3DCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_W3DCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDeletePlaneWOpps);
        }

        //m_pW3DCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pW3DCtxMenu->addSeparator();
        m_pCurWPlrMenu_W3DCtxMenu = m_pW3DCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addSeparator();
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu_W3DCtxMenu->addSeparator();
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pExportAnalysisToXML);
        }
        //m_pW3DCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pW3DCtxMenu->addSeparator();
        m_pCurWOppMenu_W3DCtxMenu = m_pW3DCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_W3DCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_W3DCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_W3DCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pW3DCtxMenu->addMenu(m_pCurWOppMenu);
        m_pW3DCtxMenu->addSeparator();
        m_pW3DCtxMenu->addAction(m_pDeleteAllWOpps);
        m_pW3DCtxMenu->addSeparator();
        m_pW3DCtxMenu->addAction(m_pReset3DScale);
        m_pW3DCtxMenu->addAction(m_pW3DScalesAct);
        m_pW3DCtxMenu->addAction(m_pW3DLightAct);
        m_pW3DCtxMenu->addAction(m_pShowFlapMoments);
        m_pW3DCtxMenu->addSeparator();
        m_pW3DCtxMenu->addAction(m_pViewLogFile);
        m_pW3DCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }

    //W3D Stab View Context Menu
    m_pW3DStabCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentPlaneMenu_W3DStabCtxMenu = m_pW3DStabCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_W3DStabCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditObjectAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditWingAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditStabAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditFinAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pExporttoSTL);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDeletePlaneWOpps);
        }
        //m_pW3DStabCtxMenu->addMenu(m_pCurrentPlaneMenu);
        m_pW3DStabCtxMenu->addSeparator();
        m_pCurWPlrMenu_W3DStabCtxMenu = m_pW3DStabCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pRenameCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addSeparator();
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pDeleteAllWPlrOpps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pShowWPlrOppsOnly);
            m_pCurWPlrMenu_W3DStabCtxMenu->addSeparator();
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pExportAnalysisToXML);
        }

        //m_pW3DStabCtxMenu->addMenu(m_pCurWPlrMenu);
        m_pW3DStabCtxMenu->addSeparator();
        m_pCurWOppMenu_W3DStabCtxMenu = m_pW3DStabCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_W3DStabCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_W3DStabCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_W3DStabCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pW3DStabCtxMenu->addMenu(m_pCurWOppMenu);
        m_pW3DStabCtxMenu->addSeparator();
        m_pW3DStabCtxMenu->addAction(m_pReset3DScale);
        m_pW3DStabCtxMenu->addAction(m_pW3DScalesAct);
        m_pW3DStabCtxMenu->addAction(m_pW3DLightAct);
        m_pW3DStabCtxMenu->addAction(m_pShowFlapMoments);
        m_pW3DStabCtxMenu->addSeparator();
        m_pW3DStabCtxMenu->addAction(m_pViewLogFile);
        m_pW3DStabCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }
}


void MainFrame::createMiarexToolbar()
{
    m_pctrlPlane  = new QComboBox();
    QStyledItemDelegate *pPlaneDelegate = new QStyledItemDelegate(this);
    m_pctrlPlane->setItemDelegate(pPlaneDelegate); //necessary to support stylesheets
    m_pctrlPlanePolar = new QComboBox;
    QStyledItemDelegate *pPPolarDelegate = new QStyledItemDelegate(this);
    m_pctrlPlanePolar->setItemDelegate(pPPolarDelegate); //necessary to support stylesheets
    m_pctrlPlaneOpp   = new QComboBox;
    QStyledItemDelegate *pPOppDelegate = new QStyledItemDelegate(this);
    m_pctrlPlaneOpp->setItemDelegate(pPOppDelegate); //necessary to support stylesheets

    m_pctrlPlane->setFocusPolicy(Qt::NoFocus);// to override keypressevent and key_F4
    m_pctrlPlanePolar->setFocusPolicy(Qt::NoFocus);
    m_pctrlPlaneOpp->setFocusPolicy(Qt::NoFocus);


    m_pctrlPlane->setMinimumWidth(150);
    m_pctrlPlanePolar->setMinimumWidth(150);
    m_pctrlPlaneOpp->setMinimumWidth(80);
    m_pctrlPlane->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlPlanePolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlPlaneOpp->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlPlane->setMaxVisibleItems(23);
    m_pctrlPlanePolar->setMaxVisibleItems(23);
    m_pctrlPlaneOpp->setMaxVisibleItems(23);

    m_pctrlMiarexToolBar = addToolBar(tr("Plane"));
    m_pctrlMiarexToolBar->addAction(m_pNewProjectAct);
    m_pctrlMiarexToolBar->addAction(m_pOpenAct);
    m_pctrlMiarexToolBar->addAction(m_pSaveAct);
    m_pctrlMiarexToolBar->addSeparator();

    // all miarex view actions are in action group
    m_pctrlMiarexToolBar->addActions(m_pMiarexViewActGroup->actions());

    m_pctrlMiarexToolBar->addSeparator();
    m_pctrlMiarexToolBar->addWidget(m_pctrlPlane);
    m_pctrlMiarexToolBar->addWidget(m_pctrlPlanePolar);
    m_pctrlMiarexToolBar->addWidget(m_pctrlPlaneOpp);

    connect(m_pctrlPlane,      SIGNAL(activated(int)), this, SLOT(onSelChangePlane(int)));
    connect(m_pctrlPlanePolar, SIGNAL(activated(int)), this, SLOT(onSelChangeWPolar(int)));
    connect(m_pctrlPlaneOpp,   SIGNAL(activated(int)), this, SLOT(onSelChangePlaneOpp(int)));
}


void MainFrame::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
    m_pctrlProjectName = new QLabel(" ");
    m_pctrlProjectName->setMinimumWidth(200);
    statusBar()->addPermanentWidget(m_pctrlProjectName);
}


void MainFrame::createToolbars()
{
    createXDirectToolbar();
    createXInverseToolbar();
    createMiarexToolbar();
    createAFoilToolbar();
}


void MainFrame::createXDirectToolbar()
{
    m_pctrlFoil    = new QComboBox;
    m_pctrlPolar   = new QComboBox;
    m_pctrlOpPoint = new QComboBox;

    QStyledItemDelegate *pFoilDelegate = new QStyledItemDelegate(this);
    m_pctrlFoil->setItemDelegate(pFoilDelegate); //necessary to support stylesheets

    QStyledItemDelegate *pPolarDelegate = new QStyledItemDelegate(this);
    m_pctrlPolar->setItemDelegate(pPolarDelegate); //necessary to support stylesheets

    QStyledItemDelegate *pOppDelegate = new QStyledItemDelegate(this);
    m_pctrlOpPoint->setItemDelegate(pOppDelegate); //necessary to support stylesheets

    m_pctrlFoil->setMaxVisibleItems(23);
    m_pctrlPolar->setMaxVisibleItems(23);
    m_pctrlOpPoint->setMaxVisibleItems(23);
    m_pctrlFoil->setMinimumWidth(150);
    m_pctrlPolar->setMinimumWidth(150);
    m_pctrlOpPoint->setMinimumWidth(80);
    m_pctrlFoil->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlPolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlOpPoint->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    m_pctrlXDirectToolBar = addToolBar(tr("Foil 2"));
    m_pctrlXDirectToolBar->addAction(m_pNewProjectAct);
    m_pctrlXDirectToolBar->addAction(m_pOpenAct);
    m_pctrlXDirectToolBar->addAction(m_pSaveAct);
    m_pctrlXDirectToolBar->addSeparator();
    // all XDirect view actions are in action group
    m_pctrlXDirectToolBar->addActions(m_pXDirectViewActGroup->actions());

    m_pctrlXDirectToolBar->addSeparator();
    m_pctrlXDirectToolBar->addWidget(m_pctrlFoil);
    m_pctrlXDirectToolBar->addWidget(m_pctrlPolar);
    m_pctrlXDirectToolBar->addWidget(m_pctrlOpPoint);

    connect(m_pctrlFoil,    SIGNAL(activated(int)), this, SLOT(onSelChangeFoil(int)));
    connect(m_pctrlPolar,   SIGNAL(activated(int)), this, SLOT(onSelChangePolar(int)));
    connect(m_pctrlOpPoint, SIGNAL(activated(int)), this, SLOT(onSelChangeOpp(int)));
}


void MainFrame::createXDirectActions()
{
    // groups the XDirect view so toolbar and menu actions are toggled properly
    m_pXDirectViewActGroup = new QActionGroup(this);
    {
        m_pOpPointsAct = new QAction(QIcon(":/images/OnCpView.png"), tr("OpPoint view")+"\tF5", this);
        m_pOpPointsAct->setCheckable(true);
        m_pOpPointsAct->setStatusTip(tr("Show Operating point view"));
        m_pOpPointsAct->setActionGroup(m_pXDirectViewActGroup);
        connect(m_pOpPointsAct, SIGNAL(triggered()), m_pXDirect, SLOT(onOpPointView()));

        m_pPolarsAct = new QAction(QIcon(":/images/OnPolarView.png"), tr("Polar view")+"\tF8", this);
        m_pPolarsAct->setCheckable(true);
        m_pPolarsAct->setStatusTip(tr("Show Polar view"));
        m_pPolarsAct->setActionGroup(m_pXDirectViewActGroup);
        connect(m_pPolarsAct, SIGNAL(triggered()), m_pXDirect, SLOT(onPolarView()));
        // end action group
    }

    m_pXDirectPolarFilter = new QAction(tr("Polar Filter"), this);
    connect(m_pXDirectPolarFilter, SIGNAL(triggered()), m_pXDirect, SLOT(onPolarFilter()));

    m_pHighlightOppAct     = new QAction(tr("Highlight Current OpPoint")+"\tCtrl+H", this);
    m_pHighlightOppAct->setCheckable(true);
    m_pHighlightOppAct->setStatusTip(tr("Highlights on the polar curve the currently selected operating point"));
    connect(m_pHighlightOppAct, SIGNAL(triggered()), this, SLOT(onHighlightOperatingPoint()));

    m_pDeleteCurFoil = new QAction(tr("Delete"), this);
    connect(m_pDeleteCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteCurFoil()));

    m_pRenameCurFoil = new QAction(tr("Rename")/*+"\tF2"*/, this);
    m_pRenameCurFoil->setShortcut(Qt::Key_F2);
    connect(m_pRenameCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onRenameCurFoil()));

    m_pExportCurFoil = new QAction(tr("Export"), this);
    connect(m_pExportCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurFoil()));

    m_pDirectDuplicateCurFoil = new QAction(tr("Duplicate"), this);
    connect(m_pDirectDuplicateCurFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onDuplicateFoil()));

    m_pSetCurFoilStyle = new QAction(tr("Set Style"), this);
    connect(m_pSetCurFoilStyle, SIGNAL(triggered()), this, SLOT(onCurFoilStyle()));

    m_pDeleteFoilPolars = new QAction(tr("Delete associated polars"), this);
    m_pDeleteFoilPolars->setStatusTip(tr("Delete all the polars associated to this foil"));
    connect(m_pDeleteFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteFoilPolars()));

    m_pShowFoilPolarsOnly = new QAction(tr("Show only associated polars"), this);
    m_pShowFoilPolarsOnly->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    connect(m_pShowFoilPolarsOnly, SIGNAL(triggered()), m_pXDirect, SLOT(onShowFoilPolarsOnly()));

    m_pShowFoilPolars = new QAction(tr("Show associated polars"), this);
    connect(m_pShowFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onShowFoilPolars()));

    m_pHideFoilPolars = new QAction(tr("Hide associated polars"), this);
    connect(m_pHideFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onHideFoilPolars()));

    m_pSaveFoilPolars = new QAction(tr("Export polars to .plr file"), this);
    connect(m_pSaveFoilPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onSaveFoilPolars()));

    m_pHidePolarOpps = new QAction(tr("Hide associated OpPoints"), this);
    connect(m_pHidePolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onHidePolarOpps()));

    m_pShowPolarOpps = new QAction(tr("Show associated OpPoints"), this);
    connect(m_pShowPolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onShowPolarOpps()));

    m_pDeletePolarOpps = new QAction(tr("Delete associated OpPoints"), this);
    connect(m_pDeletePolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onDeletePolarOpps()));

    m_pExportPolarOpps = new QAction(tr("Export associated OpPoints"), this);
    connect(m_pExportPolarOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onExportPolarOpps()));

    m_pHideFoilOpps = new QAction(tr("Hide associated OpPoints"), this);
    connect(m_pHideFoilOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onHideFoilOpps()));

    m_pShowFoilOpps = new QAction(tr("Show associated OpPoints"), this);
    connect(m_pShowFoilOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onShowFoilOpps()));

    m_pDeleteFoilOpps = new QAction(tr("Delete associated OpPoints"), this);
    connect(m_pDeleteFoilOpps, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteFoilOpps()));

    m_pDefinePolarAct = new QAction(tr("Define an Analysis")/*+"\tF6"*/, this);
    m_pDefinePolarAct->setShortcut(Qt::Key_F6);
    m_pDefinePolarAct->setStatusTip(tr("Defines a single analysis/polar"));
    connect(m_pDefinePolarAct, SIGNAL(triggered()), m_pXDirect, SLOT(onDefinePolar()));

    m_pBatchAnalysisAct = new QAction(tr("Batch Analysis")/*+"\tShift+F6"*/, this);
    m_pBatchAnalysisAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F6));
    m_pBatchAnalysisAct->setStatusTip(tr("Launches a batch of analysis calculation for a specified range or list of Reynolds numbers"));
    connect(m_pBatchAnalysisAct, SIGNAL(triggered()), m_pXDirect, SLOT(onBatchAnalysis()));

    m_pMultiThreadedBatchAct = new QAction(tr("Multi-threaded Batch Analysis")/*+"\tCtrl+F6"*/, this);
    m_pMultiThreadedBatchAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F6));
    m_pMultiThreadedBatchAct->setStatusTip(tr("Launches a batch of analysis calculation using all available computer CPU cores"));
    connect(m_pMultiThreadedBatchAct, SIGNAL(triggered()), m_pXDirect, SLOT(onMultiThreadedBatchAnalysis()));

    m_pDeletePolar = new QAction(tr("Delete"), this);
    m_pDeletePolar->setStatusTip(tr("Deletes the currently selected polar"));
    connect(m_pDeletePolar, SIGNAL(triggered()), m_pXDirect, SLOT(onDeleteCurPolar()));

    m_pResetCurPolar = new QAction(tr("Reset"), this);
    m_pResetCurPolar->setStatusTip(tr("Deletes the contents of the currently selected polar"));
    connect(m_pResetCurPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onResetCurPolar()));

    m_pEditCurPolar = new QAction(tr("Edit data points"), this);
    m_pEditCurPolar->setStatusTip(tr("Remove the unconverged or erroneaous points of the currently selected polar"));
    connect(m_pEditCurPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onEditCurPolar()));

    m_pExportCurPolar = new QAction(tr("Export"), this);
    connect(m_pExportCurPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurPolar()));

    m_pExportPolarsTxt = new QAction(tr("to text format"), this);
    connect(m_pExportPolarsTxt, SIGNAL(triggered()), m_pXDirect, SLOT(onExportAllPolarsTxt()));

    m_pExportFoilPlrs = new QAction(tr("to .plr format"), this);
    connect(m_pExportFoilPlrs, SIGNAL(triggered()), m_pXDirect, SLOT(onExportAllFoilPolars()));

    m_pXDirectStyleAct = new QAction(tr("Define Styles"), this);
    m_pXDirectStyleAct->setStatusTip(tr("Define the style for the boundary layer and the pressure arrows"));

    m_pShowNeutralLine = new QAction(tr("Neutral Line"), this);
    m_pShowNeutralLine->setCheckable(true);

    /*    m_pShowPanels = new QAction(tr("Show Panels"), this);
    m_pShowPanels->setCheckable(true);
    m_pShowPanels->setStatusTip(tr("Show the foil's panels"));*/

    m_pResetFoilScale = new QAction(tr("Reset Foil Scale"), this);
    m_pResetFoilScale->setStatusTip(tr("Resets the foil's scale to original size"));

    m_pManageFoilsAct = new QAction(tr("Manage Foils"), this);
    m_pManageFoilsAct->setShortcut(Qt::Key_F7);
    connect(m_pManageFoilsAct, SIGNAL(triggered()), this, SLOT(onManageFoils()));

    m_pRenamePolarAct = new QAction(tr("Rename")/*+"\t(Shift+F2)"*/, this);
    m_pRenamePolarAct->setShortcut(QKeySequence(Qt::SHIFT +Qt::Key_F2));
    connect(m_pRenamePolarAct, SIGNAL(triggered()), m_pXDirect, SLOT(onRenameCurPolar()));


    m_pShowInviscidCurve = new QAction(tr("Show Inviscid Curve"), this);
    m_pShowInviscidCurve->setCheckable(true);
    m_pShowInviscidCurve->setStatusTip(tr("Display the Opp's inviscid curve"));
    connect(m_pShowInviscidCurve, SIGNAL(triggered()), m_pXDirect, SLOT(onCpi()));


    m_pShowAllPolars = new QAction(tr("Show All Polars"), this);
    connect(m_pShowAllPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onShowAllPolars()));

    m_pHideAllPolars = new QAction(tr("Hide All Polars"), this);
    connect(m_pHideAllPolars, SIGNAL(triggered()), m_pXDirect, SLOT(onHideAllPolars()));

    m_pShowCurOppOnly = new QAction(tr("Show Current Opp Only"), this);
    m_pShowCurOppOnly->setCheckable(true);
    connect(m_pShowCurOppOnly, SIGNAL(triggered()), m_pXDirect, SLOT(onCurOppOnly()));

    m_pShowAllOpPoints = new QAction(tr("Show All Opps"), this);
    connect(m_pShowAllOpPoints, SIGNAL(triggered()), m_pXDirect, SLOT(onShowAllOpps()));

    m_pHideAllOpPoints = new QAction(tr("Hide All Opps"), this);
    connect(m_pHideAllOpPoints, SIGNAL(triggered()), m_pXDirect, SLOT(onHideAllOpps()));

    m_pExportCurOpp = new QAction(tr("Export"), this);
    connect(m_pExportCurOpp, SIGNAL(triggered()), m_pXDirect, SLOT(onExportCurOpp()));

    m_pDeleteCurOpp = new QAction(tr("Delete"), this);
    connect(m_pDeleteCurOpp, SIGNAL(triggered()), m_pXDirect, SLOT(onDelCurOpp()));

    m_pGetOppProps = new QAction(tr("Properties"), this);
    connect(m_pGetOppProps, SIGNAL(triggered()), m_pXDirect, SLOT(onOpPointProps()));

    m_pGetPolarProps = new QAction(tr("Properties"), this);
    connect(m_pGetPolarProps, SIGNAL(triggered()), m_pXDirect, SLOT(onPolarProps()));

    m_pViewXFoilAdvanced = new QAction(tr("XFoil Advanced Settings"), this);
    m_pBatchAnalysisAct->setStatusTip(tr("Tip : you don't want to use that option..."));
    connect(m_pViewXFoilAdvanced, SIGNAL(triggered()), m_pXDirect, SLOT(onXFoilAdvanced()));

    m_pViewLogFile = new QAction(tr("View Log File")+"\tL", this);
    connect(m_pViewLogFile, SIGNAL(triggered()), this, SLOT(onLogFile()));

    m_pDerotateFoil = new QAction(tr("De-rotate the Foil"), this);
    connect(m_pDerotateFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onDerotateFoil()));

    m_pNormalizeFoil = new QAction(tr("Normalize the Foil"), this);
    connect(m_pNormalizeFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onNormalizeFoil()));

    m_pRefineLocalFoil = new QAction(tr("Refine Locally")/*+"\t(Shift+F3)"*/, this);
    m_pRefineLocalFoil->setShortcut(QKeySequence(Qt::SHIFT +Qt::Key_F3));
    connect(m_pRefineLocalFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onCadd()));

    m_pRefineGlobalFoil = new QAction(tr("Refine Globally")/*+"\t(F3)"*/, this);
    m_pRefineGlobalFoil->setShortcut(Qt::Key_F3);
    connect(m_pRefineGlobalFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onRefinePanelsGlobally()));

    m_pEditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
    connect(m_pEditCoordsFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilCoordinates()));

    m_pScaleFoil = new QAction(tr("Scale camber and thickness")/*+"\t(F9)"*/, this);
    m_pScaleFoil->setShortcut(Qt::Key_F9);
    connect(m_pScaleFoil, SIGNAL(triggered()), m_pXDirect, SLOT(onFoilGeom()));

    m_pSetTEGap = new QAction(tr("Set T.E. Gap"), this);
    connect(m_pSetTEGap, SIGNAL(triggered()), m_pXDirect, SLOT(onSetTEGap()));

    m_pSetLERadius = new QAction(tr("Set L.E. Radius"), this);
    connect(m_pSetLERadius, SIGNAL(triggered()), m_pXDirect, SLOT(onSetLERadius()));

    m_pSetFlap = new QAction(tr("Set Flap")/*+"\t(F10)"*/, this);
    m_pSetFlap->setShortcut(Qt::Key_F10);
    connect(m_pSetFlap, SIGNAL(triggered()), m_pXDirect, SLOT(onSetFlap()));

    m_pInterpolateFoils = new QAction(tr("Interpolate Foils")/*+"\t(F11)"*/, this);
    m_pInterpolateFoils->setShortcut(Qt::Key_F11);
    connect(m_pInterpolateFoils, SIGNAL(triggered()), m_pXDirect, SLOT(onInterpolateFoils()));

    m_pNacaFoils = new QAction(tr("Naca Foils"), this);
    connect(m_pNacaFoils, SIGNAL(triggered()), m_pXDirect, SLOT(onNacaFoils()));

    m_psetCpVarGraph = new QAction(tr("Cp Variable"), this);
    m_psetCpVarGraph->setCheckable(true);
    m_psetCpVarGraph->setStatusTip(tr("Sets Cp vs. chord graph"));
    connect(m_psetCpVarGraph, SIGNAL(triggered()), m_pXDirect, SLOT(onCpGraph()));

    m_psetQVarGraph = new QAction(tr("Q Variable"), this);
    m_psetQVarGraph->setCheckable(true);
    m_psetQVarGraph->setStatusTip(tr("Sets Speed vs. chord graph"));
    connect(m_psetQVarGraph, SIGNAL(triggered()), m_pXDirect, SLOT(onQGraph()));

    m_pExportBLData = new QAction(tr("Export BL Data"), this);
    m_pExportBLData->setStatusTip(tr("Sets Speed vs. chord graph"));
    connect(m_pExportBLData, SIGNAL(triggered()), m_pXDirect, SLOT(onExportBLData()));

    //    m_pImportJavaFoilPolar = new QAction(tr("Import JavaFoil Polar"), this);
    //    connect(m_pImportJavaFoilPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onImportJavaFoilPolar()));

    m_pImportXFoilPolar = new QAction(tr("Import XFoil Polar(s)"), this);
    connect(m_pImportXFoilPolar, SIGNAL(triggered()), m_pXDirect, SLOT(onImportXFoilPolars()));

    m_pImportXMLFoilAnalysis = new QAction(tr("Import Analysis from xml file"), this);
    connect(m_pImportXMLFoilAnalysis, SIGNAL(triggered()), m_pXDirect, SLOT(onImportXMLAnalysis()));

    m_pExportXMLFoilAnalysis = new QAction(tr("Export analysis to xml file"), this);
    connect(m_pExportXMLFoilAnalysis, SIGNAL(triggered()), m_pXDirect, SLOT(onExportXMLAnalysis()));
}


void MainFrame::createXDirectMenus()
{
    //MainMenu for XDirect Application
    m_pXDirectViewMenu = menuBar()->addMenu(tr("View"));
    {
        // all XDirect view actions are in the group...
        m_pXDirectViewMenu->addActions(m_pXDirectViewActGroup->actions());

        m_pXDirectViewMenu->addSeparator();
        m_pXDirectViewMenu->addAction(m_pSaveViewToImageFileAct);
    }

    m_pXDirectFoilMenu = menuBar()->addMenu(tr("Foil"));
    {
        m_pXDirectFoilMenu->addAction(m_pManageFoilsAct);
        m_pXDirectFoilMenu->addSeparator();
        m_pCurrentFoilMenu = m_pXDirectFoilMenu->addMenu(tr("Current Foil"));
        {
            m_pCurrentFoilMenu->addAction(m_pSetCurFoilStyle);
            m_pCurrentFoilMenu->addSeparator();
            m_pCurrentFoilMenu->addAction(m_pExportCurFoil);
            m_pCurrentFoilMenu->addAction(m_pRenameCurFoil);
            m_pCurrentFoilMenu->addAction(m_pDeleteCurFoil);
            m_pCurrentFoilMenu->addAction(m_pDirectDuplicateCurFoil);
            m_pCurrentFoilMenu->addSeparator();
            m_pCurrentFoilMenu->addAction(m_pShowFoilPolarsOnly);
            m_pCurrentFoilMenu->addAction(m_pShowFoilPolars);
            m_pCurrentFoilMenu->addAction(m_pHideFoilPolars);
            m_pCurrentFoilMenu->addAction(m_pDeleteFoilPolars);
            m_pCurrentFoilMenu->addSeparator();
            m_pCurrentFoilMenu->addAction(m_pSaveFoilPolars);
            m_pCurrentFoilMenu->addSeparator();
            m_pCurrentFoilMenu->addAction(m_pShowFoilOpps);
            m_pCurrentFoilMenu->addAction(m_pHideFoilOpps);
            m_pCurrentFoilMenu->addAction(m_pDeleteFoilOpps);
        }
        m_pXDirectFoilMenu->addSeparator();
        m_pXDirectFoilMenu->addAction(m_pResetFoilScale);
        //        m_pXDirectFoilMenu->addAction(m_pShowPanels);
        m_pXDirectFoilMenu->addAction(m_pShowNeutralLine);
        m_pXDirectFoilMenu->addAction(m_pXDirectStyleAct);
    }

    m_pDesignMenu = menuBar()->addMenu(tr("Design"));
    {
        m_pDesignMenu->addAction(m_pNormalizeFoil);
        m_pDesignMenu->addAction(m_pDerotateFoil);
        m_pDesignMenu->addAction(m_pRefineGlobalFoil);
        m_pDesignMenu->addAction(m_pRefineLocalFoil);
        m_pDesignMenu->addAction(m_pEditCoordsFoil);
        m_pDesignMenu->addAction(m_pScaleFoil);
        m_pDesignMenu->addAction(m_pSetTEGap);
        m_pDesignMenu->addAction(m_pSetLERadius);
        m_pDesignMenu->addAction(m_pSetFlap);
        m_pDesignMenu->addSeparator();
        m_pDesignMenu->addAction(m_pInterpolateFoils);
        m_pDesignMenu->addAction(m_pNacaFoils);
    }

    m_pXFoilAnalysisMenu = menuBar()->addMenu(tr("Analysis"));
    {
        m_pXFoilAnalysisMenu->addAction(m_pDefinePolarAct);
        m_pXFoilAnalysisMenu->addAction(m_pBatchAnalysisAct);
        m_pXFoilAnalysisMenu->addAction(m_pMultiThreadedBatchAct);
        m_pXFoilAnalysisMenu->addSeparator();
        m_pXFoilAnalysisMenu->addAction(m_pImportXMLFoilAnalysis);
        m_pXFoilAnalysisMenu->addSeparator();
        m_pXFoilAnalysisMenu->addAction(m_pViewXFoilAdvanced);
        m_pXFoilAnalysisMenu->addAction(m_pViewLogFile);
    }

    m_pPolarMenu = menuBar()->addMenu(tr("Polars"));
    {
        m_pCurrentPolarMenu = m_pPolarMenu->addMenu(tr("Current Polar"));
        {
            m_pCurrentPolarMenu->addAction(m_pGetPolarProps);
            m_pCurrentPolarMenu->addAction(m_pEditCurPolar);
            m_pCurrentPolarMenu->addAction(m_pResetCurPolar);
            m_pCurrentPolarMenu->addAction(m_pDeletePolar);
            m_pCurrentPolarMenu->addAction(m_pRenamePolarAct);
            m_pCurrentPolarMenu->addAction(m_pExportCurPolar);
            m_pCurrentPolarMenu->addSeparator();
            m_pCurrentPolarMenu->addAction(m_pExportXMLFoilAnalysis);
            m_pCurrentPolarMenu->addSeparator();
            m_pCurrentPolarMenu->addAction(m_pShowPolarOpps);
            m_pCurrentPolarMenu->addAction(m_pHidePolarOpps);
            m_pCurrentPolarMenu->addAction(m_pDeletePolarOpps);
            m_pCurrentPolarMenu->addAction(m_pExportPolarOpps);
        }
        m_pPolarMenu->addSeparator();
        m_pPolarMenu->addAction(m_pImportXFoilPolar);
        m_pPolarMenu->addSeparator();
        QMenu *pExportMenu = m_pPolarMenu->addMenu(tr("Export all"));
        {
            pExportMenu->addAction(m_pExportFoilPlrs);
            pExportMenu->addAction(m_pExportPolarsTxt);
        }
        m_pPolarMenu->addSeparator();
        m_pPolarMenu->addAction(m_pXDirectPolarFilter);
        m_pPolarMenu->addSeparator();
        m_pPolarMenu->addAction(m_pShowAllPolars);
        m_pPolarMenu->addAction(m_pHideAllPolars);
        m_pPolarMenu->addSeparator();
    }

    m_pOpPointMenu = menuBar()->addMenu(tr("Operating Points"));
    {
        m_pCurrentOppMenu = m_pOpPointMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurrentOppMenu->addAction(m_pExportCurOpp);
            m_pCurrentOppMenu->addAction(m_pExportBLData);
            m_pCurrentOppMenu->addAction(m_pDeleteCurOpp);
            m_pCurrentOppMenu->addAction(m_pGetOppProps);
        }
        m_pOpPointMenu->addSeparator();
        m_pXDirectCpGraphMenu = m_pOpPointMenu->addMenu(tr("Cp Graph"));
        {
            m_pXDirectCpGraphMenu->addAction(m_psetCpVarGraph);
            m_pXDirectCpGraphMenu->addAction(m_psetQVarGraph);
            m_pXDirectCpGraphMenu->addSeparator();
            m_pXDirectCpGraphMenu->addAction(m_pShowInviscidCurve);
            m_pXDirectCpGraphMenu->addSeparator();
            m_pXDirectCpGraphMenu->addAction(m_pResetCurGraphScales);
            m_pXDirectCpGraphMenu->addAction(m_pExportBLData);
            m_pXDirectCpGraphMenu->addAction(m_pExportCurGraphAct);
        }
        m_pOpPointMenu->addAction(m_pShowMousePosAct);
        m_pOpPointMenu->addSeparator();
        m_pOpPointMenu->addAction(m_pShowCurOppOnly);
        m_pOpPointMenu->addAction(m_pHideAllOpPoints);
        m_pOpPointMenu->addAction(m_pShowAllOpPoints);
    }

    //XDirect foil Context Menu
    m_pOperFoilCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentFoilMenu_OperFoilCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current Foil"));
        {
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pSetCurFoilStyle);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pExportCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pRenameCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDeleteCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDirectDuplicateCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pShowFoilPolarsOnly);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pShowFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pHideFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDeleteFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pSaveFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pShowFoilOpps);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pHideFoilOpps);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDeleteFoilOpps);
        }
        //m_pOperFoilCtxMenu->addMenu(m_pCurrentFoilMenu);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pCurrentPolarMenu_OperFoilCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pGetPolarProps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pEditCurPolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pResetCurPolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pDeletePolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pRenamePolarAct);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pExportCurPolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pShowPolarOpps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pHidePolarOpps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pDeletePolarOpps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pExportPolarOpps);
        }
        //m_pOperFoilCtxMenu->addMenu(m_pCurrentPolarMenu);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pDesignMenu_OperPolarCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Design"));
        {
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pNormalizeFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pDerotateFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pRefineGlobalFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pRefineLocalFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pEditCoordsFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pScaleFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pSetTEGap);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pSetLERadius);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pSetFlap);
            m_pDesignMenu_OperPolarCtxMenu->addSeparator();
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pInterpolateFoils);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pNacaFoils);
        }

        //m_pOperFoilCtxMenu->addMenu(m_pDesignMenu);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pCurOppCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurOppCtxMenu->addAction(m_pExportCurOpp);
            m_pCurOppCtxMenu->addAction(m_pExportBLData);
            m_pCurOppCtxMenu->addAction(m_pDeleteCurOpp);
            m_pCurOppCtxMenu->addAction(m_pGetOppProps);
        }
        m_pOperFoilCtxMenu->addAction(m_pShowMousePosAct);

        m_pOperFoilCtxMenu->addSeparator();//_______________
        //    CurGraphCtxMenu = OperFoilCtxMenu->addMenu(tr("Cp graph"));
        m_pXDirectCpGraphMenu_OperPolarCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Cp Graph"));
        {
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_psetCpVarGraph);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_psetQVarGraph);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addSeparator();
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_pShowInviscidCurve);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addSeparator();
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_pResetCurGraphScales);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_pExportCurGraphAct);
        }
        //m_pOperFoilCtxMenu->addMenu(m_pXDirectCpGraphMenu);

        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addAction(m_pDefinePolarAct);
        m_pOperFoilCtxMenu->addAction(m_pBatchAnalysisAct);
        m_pOperFoilCtxMenu->addAction(m_pMultiThreadedBatchAct);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addAction(m_pShowAllPolars);
        m_pOperFoilCtxMenu->addAction(m_pHideAllPolars);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addAction(m_pShowCurOppOnly);
        m_pOperFoilCtxMenu->addAction(m_pShowAllOpPoints);
        m_pOperFoilCtxMenu->addAction(m_pHideAllOpPoints);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addAction(m_pResetFoilScale);
        //        m_pOperFoilCtxMenu->addAction(m_pShowPanels);
        m_pOperFoilCtxMenu->addAction(m_pShowNeutralLine);
        m_pOperFoilCtxMenu->addAction(m_pXDirectStyleAct);
        m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pOperFoilCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }
    //End XDirect foil Context Menu


    //XDirect polar Context Menu
    m_pOperPolarCtxMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pCurrentFoilMenu_OperPolarCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Foil"));
        {
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pSetCurFoilStyle);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pExportCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pRenameCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDeleteCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDirectDuplicateCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pShowFoilPolarsOnly);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pShowFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pHideFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDeleteFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pSaveFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pShowFoilOpps);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pHideFoilOpps);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDeleteFoilOpps);
        }

        //m_pOperPolarCtxMenu->addMenu(m_pCurrentFoilMenu);
        m_pCurrentPolarMenu_OperPolarCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pGetPolarProps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pEditCurPolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pResetCurPolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pDeletePolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pRenamePolarAct);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pExportCurPolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pShowPolarOpps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pHidePolarOpps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pDeletePolarOpps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pExportPolarOpps);
        }
        //m_pOperPolarCtxMenu->addMenu(m_pCurrentPolarMenu);
        m_pOperPolarCtxMenu->addSeparator();//_______________
        QMenu *pCurGraphCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Graph"));
        {
            pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
            pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
            pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
        }
        m_pOperPolarCtxMenu->addAction(m_pAllGraphsSettings);
        m_pOperPolarCtxMenu->addAction(m_pAllGraphsScalesAct);
        m_pOperPolarCtxMenu->addAction(m_pShowMousePosAct);
        m_pOperPolarCtxMenu->addSeparator();//_______________
        m_pOperPolarCtxMenu->addAction(m_pDefinePolarAct);
        m_pOperPolarCtxMenu->addAction(m_pBatchAnalysisAct);
        m_pOperPolarCtxMenu->addAction(m_pMultiThreadedBatchAct);
        m_pOperPolarCtxMenu->addSeparator();//_______________
        m_pOperPolarCtxMenu->addAction(m_pShowAllPolars);
        m_pOperPolarCtxMenu->addAction(m_pHideAllPolars);
        m_pOperPolarCtxMenu->addAction(m_pShowAllOpPoints);
        m_pOperPolarCtxMenu->addAction(m_pHideAllOpPoints);
        m_pOperPolarCtxMenu->addSeparator();//_______________
        m_pOperPolarCtxMenu->addAction(m_pSaveViewToImageFileAct);
    }

    //End XDirect polar Context Menu
}



void MainFrame::createXInverseActions()
{
    m_pStoreFoil = new QAction(QIcon(":/images/OnStoreFoil.png"), tr("Store Foil"), this);
    m_pStoreFoil->setStatusTip(tr("Store Foil in database"));
    connect(m_pStoreFoil, SIGNAL(triggered()), m_pXInverse, SLOT(onStoreFoil()));

    m_pExtractFoil = new QAction(QIcon(":/images/OnExtractFoil.png"),tr("Extract Foil"), this);
    m_pExtractFoil->setStatusTip(tr("Extract a Foil from the database for modification"));
    connect(m_pExtractFoil, SIGNAL(triggered()), m_pXInverse, SLOT(onExtractFoil()));

    m_pXInverseStyles = new QAction(tr("Define Styles"), this);
    m_pXInverseStyles->setStatusTip(tr("Define the styles for this view"));
    connect(m_pXInverseStyles, SIGNAL(triggered()), m_pXInverse, SLOT(onInverseStyles()));

    m_pOverlayFoil = new QAction(tr("Overlay foil"), this);
    m_pOverlayFoil->setStatusTip(tr("Overlay an additional foil for guidance"));
    connect(m_pOverlayFoil, SIGNAL(triggered()), m_pXInverse, SLOT(onOverlayFoil()));

    m_pClearOverlayFoil = new QAction(tr("Clear overlay foil"), this);
    connect(m_pClearOverlayFoil, SIGNAL(triggered()), m_pXInverse, SLOT(onClearOverlayFoil()));

    m_pXInverseResetFoilScale = new QAction(QIcon(":/images/OnResetFoilScale.png"), tr("Reset foil scale")+"\tR", this);
    m_pXInverseResetFoilScale->setStatusTip(tr("Resets the scale to fit the screen size"));
    connect(m_pXInverseResetFoilScale, SIGNAL(triggered()), m_pXInverse, SLOT(onResetFoilScale()));

    m_pInverseInsertCtrlPt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
    connect(m_pInverseInsertCtrlPt, SIGNAL(triggered()), m_pXInverse, SLOT(onInsertCtrlPt()));

    m_pInverseRemoveCtrlPt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
    connect(m_pInverseRemoveCtrlPt, SIGNAL(triggered()), m_pXInverse, SLOT(onRemoveCtrlPt()));

    m_pInvQInitial = new QAction(tr("Show Q-Initial"), this);
    m_pInvQInitial->setCheckable(true);
    connect(m_pInvQInitial, SIGNAL(triggered()), m_pXInverse, SLOT(onQInitial()));

    m_pInvQSpec = new QAction(tr("Show Q-Spec"), this);
    m_pInvQSpec->setCheckable(true);
    connect(m_pInvQSpec, SIGNAL(triggered()), m_pXInverse, SLOT(onQSpec()));

    m_pInvQViscous = new QAction(tr("Show Q-Viscous"), this);
    m_pInvQViscous->setCheckable(true);
    connect(m_pInvQViscous, SIGNAL(triggered()), m_pXInverse, SLOT(onQViscous()));

    m_pInvQPoints = new QAction(tr("Show Points"), this);
    m_pInvQPoints->setCheckable(true);
    connect(m_pInvQPoints, SIGNAL(triggered()), m_pXInverse, SLOT(onQPoints()));

    m_pInvQReflected = new QAction(tr("Show Reflected"), this);
    m_pInvQReflected->setCheckable(true);
    connect(m_pInvQReflected, SIGNAL(triggered()), m_pXInverse, SLOT(onQReflected()));

    m_pInverseZoomIn = new QAction(QIcon(":/images/OnZoomIn.png"), tr("Zoom in"), this);
    m_pInverseZoomIn->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
    connect(m_pInverseZoomIn, SIGNAL(triggered()), m_pXInverse, SLOT(onZoomIn()));
}


void MainFrame::createXInverseMenus()
{
    //MainMenu for XInverse Application
    m_pXInverseViewMenu = menuBar()->addMenu(tr("View"));
    {
        m_pXInverseViewMenu->addAction(m_pXInverseStyles);
        m_pXInverseViewMenu->addSeparator();
        m_pXInverseViewMenu->addAction(m_pOverlayFoil);
        m_pXInverseViewMenu->addAction(m_pClearOverlayFoil);
        m_pXInverseViewMenu->addSeparator();
        m_pXInverseViewMenu->addAction(m_pSaveViewToImageFileAct);
    }

    m_pXInverseGraphMenu = menuBar()->addMenu(tr("Graph"));
    {
        m_pXInverseGraphMenu->addAction(m_pCurGraphDlgAct);
        m_pXInverseGraphMenu->addAction(m_pResetCurGraphScales);
        m_pXInverseGraphMenu->addAction(m_pExportCurGraphAct);
    }

    m_pXInverseFoilMenu = menuBar()->addMenu(tr("Foil"));
    {
        m_pXInverseFoilMenu->addAction(m_pStoreFoil);
        m_pXInverseFoilMenu->addAction(m_pExtractFoil);
        m_pXInverseFoilMenu->addAction(m_pXInverseResetFoilScale);
        m_pXInverseFoilMenu->addSeparator();
        m_pXInverseFoilMenu->addAction(m_pInvQInitial);
        m_pXInverseFoilMenu->addAction(m_pInvQSpec);
        m_pXInverseFoilMenu->addAction(m_pInvQViscous);
        m_pXInverseFoilMenu->addAction(m_pInvQPoints);
        m_pXInverseFoilMenu->addAction(m_pInvQReflected);
    }

    //Context Menu for XInverse Application
    m_pInverseContextMenu = new QMenu(tr("Context Menu"),this);
    {
        m_pInverseContextMenu->addAction(m_pXInverseStyles);
        m_pInverseContextMenu->addAction(m_pXInverseResetFoilScale);
        m_pInverseContextMenu->addSeparator();
        m_pInverseContextMenu->addAction(m_pOverlayFoil);
        m_pInverseContextMenu->addAction(m_pClearOverlayFoil);
        m_pInverseContextMenu->addSeparator();
        m_pInverseContextMenu->addAction(m_pCurGraphDlgAct);
        m_pInverseContextMenu->addAction(m_pResetCurGraphScales);
        m_pInverseContextMenu->addAction(m_pShowMousePosAct);
        m_pInverseContextMenu->addSeparator();
        m_pInverseContextMenu->addAction(m_pInverseInsertCtrlPt);
        m_pInverseContextMenu->addAction(m_pInverseRemoveCtrlPt);
        m_pInverseContextMenu->addSeparator();
        m_pInverseContextMenu->addAction(m_pInvQInitial);
        m_pInverseContextMenu->addAction(m_pInvQSpec);
        m_pInverseContextMenu->addAction(m_pInvQViscous);
        m_pInverseContextMenu->addAction(m_pInvQPoints);
        m_pInverseContextMenu->addAction(m_pInvQReflected);
        m_pInverseContextMenu->addSeparator();
        m_pInverseContextMenu->addAction(m_pStoreFoil);
        m_pInverseContextMenu->addAction(m_pExtractFoil);
        m_pInverseContextMenu->addSeparator();

        m_pInverseContextMenu->addSeparator();
        m_pInverseContextMenu->addAction(m_pXInverseResetFoilScale);
    }
}



void MainFrame::createXInverseToolbar()
{
    m_pctrlFullInverse  = new QRadioButton(tr("Full Inverse"));
    m_pctrlMixedInverse = new QRadioButton(tr("Mixed Inverse"));
    connect(m_pctrlFullInverse,  SIGNAL(clicked()), m_pXInverse, SLOT(onInverseApp()));
    connect(m_pctrlMixedInverse, SIGNAL(clicked()), m_pXInverse, SLOT(onInverseApp()));

    m_pctrlXInverseToolBar = addToolBar(tr("XInverse"));
    m_pctrlXInverseToolBar->addAction(m_pNewProjectAct);
    m_pctrlXInverseToolBar->addAction(m_pOpenAct);
    m_pctrlXInverseToolBar->addAction(m_pSaveAct);
    m_pctrlXInverseToolBar->addSeparator();
    m_pctrlXInverseToolBar->addWidget(m_pctrlFullInverse);
    m_pctrlXInverseToolBar->addWidget(m_pctrlMixedInverse);
    m_pctrlXInverseToolBar->addSeparator();
    m_pctrlXInverseToolBar->addAction(m_pExtractFoil);
    m_pctrlXInverseToolBar->addAction(m_pStoreFoil);
    m_pctrlXInverseToolBar->addSeparator();
    m_pctrlXInverseToolBar->addAction(m_pInverseZoomIn);
    m_pctrlXInverseToolBar->addAction(m_pResetCurGraphScales);
    m_pctrlXInverseToolBar->addAction(m_pXInverseResetFoilScale);
}



void MainFrame::deleteProject(bool bClosing)
{
    // clear everything
    Objects3d::deleteObjects();

    Objects2d::deleteAllFoils();

    m_pMiarex->m_pCurPlane  = nullptr;
    m_pMiarex->m_pCurPOpp   = nullptr;
    m_pMiarex->m_pCurWPolar = nullptr;
    m_pMiarex->m_pgl3dMiarexView->m_bStream = false;

    if(!bClosing)
    {
        m_pctrlPlane->clear();
        m_pctrlPlanePolar->clear();
        m_pctrlPlaneOpp->clear();
        m_pctrlFoil->clear();
        m_pctrlPolar->clear();
        m_pctrlOpPoint->clear();

        m_pctrlPlane->setEnabled(false);
        m_pctrlPlanePolar->setEnabled(false);
        m_pctrlPlaneOpp->setEnabled(false);
        m_pctrlFoil->setEnabled(false);
        m_pctrlPolar->setEnabled(false);
        m_pctrlOpPoint->setEnabled(false);


        m_pMiarex->setPlane();

        if(m_iApp==XFLR5::MIAREX) m_pMiarex->setControls();

        m_pXDirect->setCurFoil(nullptr);
        m_pXDirect->setCurPolar(nullptr);
        m_pXDirect->setCurOpp(nullptr);
        m_pXDirect->setFoil();

        updateFoilListBox();

        if(m_pXDirect->m_bPolarView) m_pXDirect->createPolarCurves();
        else                         m_pXDirect->createOppCurves();

        m_pAFoil->fillFoilTable();
        m_pAFoil->selectFoil();

        m_pXInverse->clear();

        setProjectName("");

        setSaveState(true);
    }
}



QColor MainFrame::getColor(int type)
{
    //type
    // 0=Foil
    // 1=Polar
    // 2=Opp
    // 3=Wing (unused)
    // 4=WPolar
    // 5=WOpp
    // 6=POpp
    int i=0,j=0;
    bool bFound = false;
    switch (type)
    {
        case 0:
        {
            Foil *pFoil;
            for (j=0; j<s_ColorList.size(); j++)
            {
                for (i=0; i<Objects2d::foilCount(); i++)
                {
                    pFoil = Objects2d::foilAt(i);
                    bFound = false;
                    if(colour(pFoil) == s_ColorList.at(j))
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound) return s_ColorList.at(j);
            }
            break;
        }
        case 1:
        {
            Polar *pPolar;
            for (j=0; j<s_ColorList.size(); j++)
            {
                for (i=0; i<Objects2d::polarCount(); i++)
                {
                    pPolar = Objects2d::polarAt(i);
                    bFound = false;
                    if(colour(pPolar) == s_ColorList.at(j))
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound)
                    return s_ColorList.at(j);
            }
            break;
        }
        case 2:
        {
            OpPoint *pOpPoint;
            for (j=0; j<s_ColorList.size(); j++){
                for (i=0; i<Objects2d::oppCount(); i++)
                {
                    pOpPoint = Objects2d::oppAt(i);
                    bFound = false;
                    int r=0,g=0,b=0,a=0;
                    pOpPoint->getColor(r,g,b,a);
                    QColor clr(r,g,b,a);
                    if(clr == s_ColorList.at(j))
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound) return s_ColorList.at(j);
            }
            break;
        }

        default:
        {
        }
    }
    return randomColor(!Settings::isLightTheme());
}



void MainFrame::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    bool bShift = (pEvent->modifiers() & Qt::ShiftModifier);

    switch (pEvent->key())
    {
        case Qt::Key_N:
        {
            if(bCtrl)
            {
                onNewProject();
                return;
            }
            break;
        }
        case Qt::Key_S:
        {
            if(bCtrl)
            {
                if(bShift)
                    onSaveProjectAs();
                else
                    onSaveProject();
                return;
            }
            break;
        }
        case Qt::Key_O:
        {
            if(bCtrl)
            {
                onLoadFile();
                return;
            }
            break;
        }
    }

    if(m_iApp == XFLR5::XFOILANALYSIS && m_pXDirect)
    {
        return m_pXDirect->keyPressEvent(pEvent);
    }
    else if(m_iApp == XFLR5::MIAREX && m_pMiarex)
    {
        return m_pMiarex->keyPressEvent(pEvent);
    }
    else if(m_iApp == XFLR5::DIRECTDESIGN && m_pAFoil)
    {
        return m_pAFoil->keyPressEvent(pEvent);

    }
    else if(m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
    {
        return m_pXInverse->keyPressEvent(pEvent);
    }
    else
    {
        switch (pEvent->key())
        {
            case Qt::Key_1:
            {
                if(bCtrl) onAFoil();
                break;
            }
            case Qt::Key_2:
            {
                if(bCtrl) onAFoil();
                break;
            }
            case Qt::Key_3:
            {
                if(bCtrl) onXInverse();
                break;
            }
            case Qt::Key_4:
            {
                if(bCtrl) onXInverseMixed();
                break;
            }
            case Qt::Key_5:
            {
                if(bCtrl) onXDirect();
                break;
            }
            case Qt::Key_6:
            {
                if(bCtrl) onMiarex();
                break;
            }
                /*            case Qt::Key_7:
            {
                if(bCtrl)
                {
                    onloadLastProject();
                }
                break;
            }*/
            case Qt::Key_8:
            {
                break;
            }
            case Qt::Key_L:
            {
                onLogFile();
                break;
            }
            default:
                pEvent->ignore();
        }
    }
    pEvent->accept();
}


void MainFrame::keyReleaseEvent(QKeyEvent *pEvent)
{
    if(m_iApp == XFLR5::XFOILANALYSIS && m_pXDirect)
    {
        m_pXDirect->keyReleaseEvent(pEvent);
    }
    else if(m_iApp == XFLR5::MIAREX && m_pMiarex)
    {
        if (pEvent->key()==Qt::Key_Control)
        {
            updateView();
        }
        else m_pMiarex->keyReleaseEvent(pEvent);
    }
    else if(m_iApp == XFLR5::DIRECTDESIGN && m_pAFoil)
    {
        m_pAFoil->keyReleaseEvent(pEvent);
    }
    else if(m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
    {
        m_pXInverse->keyReleaseEvent(pEvent);
    }
    pEvent->accept();
}


bool MainFrame::loadPolarFileV3(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    Foil *pFoil = nullptr;
    Polar *pPolar = nullptr;
    Polar *pOldPlr = nullptr;
    OpPoint *pOpp = nullptr, *pOldOpp = nullptr;

    //first read all available foils
    int i,l,n;
    ar >> n;
    //    if(n<100003) return false;

    ar>>n;

    for (i=0;i<n; i++)
    {
        pFoil = new Foil();
        if (!serializeFoil(pFoil, ar, false))
        {
            delete pFoil;
            return false;
        }

        Objects2d::insertThisFoil(pFoil);
    }


    //next read all available polars
    ar>>n;
    for (i=0; i<n; i++)
    {
        pPolar = new Polar();
        QColor clr = getColor(1);
        pPolar->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
        if (!serializePolar(pPolar, ar, false))
        {
            delete pPolar;
            return false;
        }
        for (l=0; l<Objects2d::polarCount(); l++)
        {
            pOldPlr = Objects2d::polarAt(l);
            if (pOldPlr->foilName() == pPolar->foilName() &&
                    pOldPlr->polarName()  == pPolar->polarName())
            {
                //just overwrite...
                Objects2d::deletePolarAt(l);
                //... and continue to add
            }
        }
        Objects2d::addPolar(pPolar);
    }

    //Last read all available operating points
    ar>>n;
    for (i=0; i<n; i++)
    {
        pOpp = new OpPoint();
        if(!pOpp)
        {
            delete pOpp;
            return false;
        }

        QColor clr = s_ColorList[Objects2d::oppCount()%24];
        pOpp->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
        if(ArchiveFormat>=100002)
        {
            if (!pOpp->serializeOppWPA(ar, bIsStoring, 100002))
            {
                delete pOpp;
                return false;
            }
            else
            {
                pFoil = Objects2d::foil(pOpp->foilName());
                if(pFoil)
                {
                    //                    memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
                    //                    memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
                }
                else
                {
                    delete pOpp;
                }
            }
        }
        else
        {
            if (!pOpp->serializeOppWPA(ar, bIsStoring))
            {
                delete pOpp;
                return false;
            }
            else
            {
                pFoil = Objects2d::foil(pOpp->foilName());
                if(pFoil)
                {
                    //                    memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
                    //                    memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
                }
                else
                {
                    delete pOpp;
                }
            }
        }
        if(pOpp)
        {
            for (int l=Objects2d::oppCount()-1;l>=0; l--)
            {
                pOldOpp = Objects2d::oppAt(l);
                if (pOldOpp->foilName() == pOpp->foilName() &&
                        pOldOpp->polarName()  == pOpp->polarName() &&
                        qAbs(pOldOpp->aoa()-pOpp->aoa())<0.001)
                {
                    //just overwrite...
                    Objects2d::deleteOppAt(l);
                    //... and continue to add
                }
            }
        }
        Objects2d::insertOpPoint(pOpp);
    }

    return true;
}


bool MainFrame::loadSettings()
{
    QPoint pt;
    bool bFloat=false;
    int SettingsFormat=0;
    QSize size;


#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

    settings.beginGroup("MainFrame");
    {
        SettingsFormat = settings.value("SettingsFormat").toInt();
        if(SettingsFormat != SETTINGSFORMAT) return false;

        Settings::s_StyleName = settings.value("StyleName","").toString();
        int k = settings.value("ExportFileType", 0).toInt();
        if (k==0) Settings::s_ExportFileType = XFLR5::TXT;
        else      Settings::s_ExportFileType = XFLR5::CSV;

        s_LanguageFilePath = settings.value("LanguageFilePath").toString();
        m_GraphExportFilter = settings.value("GraphExportFilter",".csv").toString();

        bFloat  = settings.value("Miarex_Float").toBool();
        pt.rx() = settings.value("Miarex_x").toInt();
        pt.ry() = settings.value("Miarex_y").toInt();
        size    = settings.value("MiarexSize").toSize();
        m_pctrlMiarexWidget->setFloating(bFloat);
        if(bFloat) m_pctrlMiarexWidget->move(pt);
        m_pctrlMiarexWidget->resize(size);

        bFloat  = settings.value("XDirect_Float").toBool();
        pt.rx() = settings.value("XDirect_x").toInt();
        pt.ry() = settings.value("XDirect_y").toInt();
        size    = settings.value("XDirectSize").toSize();
        m_pctrlXDirectWidget->setFloating(bFloat);
        if(bFloat) m_pctrlXDirectWidget->move(pt);
        m_pctrlXDirectWidget->resize(size);

        bFloat  = settings.value("AFoil_Float").toBool();
        pt.rx() = settings.value("AFoil_x").toInt();
        pt.ry() = settings.value("AFoil_y").toInt();
        size    = settings.value("AFoilSize").toSize();
        m_pctrlAFoilWidget->setFloating(bFloat);
        if(bFloat) m_pctrlAFoilWidget->move(pt);
        m_pctrlAFoilWidget->resize(size);

        bFloat  = settings.value("XInverse_Float").toBool();
        pt.rx() = settings.value("XInverse_x").toInt();
        pt.ry() = settings.value("XInverse_y").toInt();
        size    = settings.value("XInverseSize").toSize();
        m_pctrlXInverseWidget->setFloating(bFloat);
        if(bFloat) m_pctrlXInverseWidget->move(pt);
        m_pctrlXInverseWidget->resize(size);

        bFloat  = settings.value("StabView_Float").toBool();
        pt.rx() = settings.value("StabView_x").toInt();
        pt.ry() = settings.value("StabView_y").toInt();
        size    = settings.value("StabSize").toSize();
        m_pctrlStabViewWidget->setFloating(bFloat);
        if(bFloat) m_pctrlStabViewWidget->move(pt);
        m_pctrlStabViewWidget->resize(size);

        m_ImageDirName = settings.value("ImageDirName").toString();
        m_ExportLastDirName = settings.value("ExportLastDirName").toString();

        Units::setLengthUnitIndex(settings.value("LengthUnit").toInt());
        Units::setAreaUnitIndex(settings.value("AreaUnit").toInt());
        Units::setWeightUnitIndex(settings.value("WeightUnit").toInt());
        Units::setSpeedUnitIndex(settings.value("SpeedUnit").toInt());
        Units::setForceUnitIndex(settings.value("ForceUnit").toInt());
        Units::setMomentUnitIndex(settings.value("MomentUnit").toInt());
        Units::setPressureUnitIndex(settings.value("PressureUnit").toInt());
        Units::setInertiaUnitIndex(settings.value("InertiaUnit").toInt());
        Units::setUnitConversionFactors();

        Graph::setOppHighlighting(settings.value("HighlightOpp").toBool());

        switch(settings.value("ImageFormat").toInt())
        {
            case 0:
                m_ImageFormat = XFLR5::PNG;
                break;
            case 1:
                m_ImageFormat = XFLR5::JPEG;
                break;
            case 2:
                m_ImageFormat = XFLR5::BMP;
                break;
            default:
                m_ImageFormat = XFLR5::PNG;
                break;
        }

        m_bAutoLoadLast = settings.value("AutoLoadLastProject").toBool();
        m_bSaveOpps   = settings.value("SaveOpps").toBool();
        m_bSaveWOpps  = settings.value("SaveWOpps").toBool();

        m_bAutoSave = settings.value("AutoSaveProject", false).toBool();

        m_SaveInterval = settings.value("AutoSaveInterval", 10).toInt();

        //        a = settings.value("RecentFileSize").toInt();
        QString RecentF,strange;
        m_RecentFiles.clear();
        int n=0;
        do
        {
            RecentF = QString("RecentFile_%1").arg(n);
            strange = settings.value(RecentF).toString();
            if(strange.length())
            {
                m_RecentFiles.append(strange);
                n++;
            }
            else break;
        }while(n<MAXRECENTFILES);


        Settings::s_bStyleSheets   = settings.value("ShowStyleSheets", false).toBool();
        Settings::s_StyleSheetName = settings.value("StyleSheetName", "xflr5_style").toString();
        s_bShowMousePos = settings.value("ShowMousePosition", true).toBool();

        Updater::loadSettings(settings);
    }

    return true;
}


MainFrame* MainFrame::self() {
    if (!_self) {
        _self = new MainFrame;
    }
    return _self;
}


XFLR5::enumApp MainFrame::loadXFLR5File(QString pathname)
{
    QFile XFile(pathname);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not open the file\n")+pathname;
        QMessageBox::information(window(), tr("Info"), strange);
        return XFLR5::NOAPP;
    }

    QString end = pathname.right(4).toLower();

    pathname.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

    int pos = pathname.lastIndexOf("/");
    if(pos>0) Settings::setLastDirName(pathname.left(pos));

    if(end==".plr")
    {
        QDataStream ar(&XFile);
        ar.setVersion(QDataStream::Qt_4_5);
        ar.setByteOrder(QDataStream::LittleEndian);

        readPolarFile(ar);

        m_pXDirect->m_bPolarView = true;
        m_pXDirect->setCurPolar(nullptr);
        m_pXDirect->setCurOpp(nullptr);

        m_pXDirect->setFoil();

        m_pXDirect->setPolar();

        XFile.close();

        addRecentFile(pathname);
        setSaveState(false);
        m_pXDirect->setControls();
        return XFLR5::XFOILANALYSIS;
    }
    else if(end==".dat")
    {
        QString fileName = pathname;
        fileName.replace(".dat","");
        int pos1 = fileName.lastIndexOf("hn");
        fileName = fileName.right(fileName.length()-pos1);

        Foil *pFoil = readFoilFile(XFile);
        XFile.close();

        if(pFoil)
        {
            Objects2d::insertThisFoil(pFoil);

            m_pXDirect->setCurFoil(pFoil);
            m_pXDirect->setCurPolar(nullptr);
            m_pXDirect->setCurOpp(nullptr);

            XFile.close();

            setSaveState(false);
            addRecentFile(pathname);

            if(m_iApp==XFLR5::XFOILANALYSIS)
            {
                m_pXDirect->setControls();
                m_pXDirect->setFoil(pFoil);

                return XFLR5::XFOILANALYSIS;
            }
            else if(m_iApp==XFLR5::DIRECTDESIGN)  m_pAFoil->selectFoil(pFoil);


            return XFLR5::DIRECTDESIGN;
        }
    }
    else if(end==".wpa")
    {
        if(!s_bSaved)
        {
            QString strong = tr("Save the current project ?");
            int resp =  QMessageBox::question(this ,tr("Save"), strong,  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
            if(resp==QMessageBox::Cancel)
            {
                XFile.close();
                return XFLR5::NOAPP;
            }
            else if (resp==QMessageBox::Yes)
            {
                if(!saveProject(m_FileName))
                {
                    XFile.close();
                    return XFLR5::NOAPP;
                }
            }
        }

        deleteProject();

        QDataStream ar(&XFile);
        ar.setVersion(QDataStream::Qt_4_5);
        ar.setByteOrder(QDataStream::LittleEndian);
        if(!serializeProjectWPA(ar, false))
        {
            QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
        }

        addRecentFile(pathname);
        setSaveState(true);
        pathname.replace(".wpa", ".xfl", Qt::CaseInsensitive);
        setProjectName(pathname);

        XFile.close();

        if(Objects3d::s_oaPlane.size()) return XFLR5::MIAREX;
        else                            return XFLR5::XFOILANALYSIS;
    }
    else if(end==".xfl")
    {
        if(!s_bSaved)
        {
            QString strong = tr("Save the current project ?");
            int resp =  QMessageBox::question(this ,tr("Save"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
            if(resp==QMessageBox::Cancel)
            {
                XFile.close();
                return XFLR5::NOAPP;
            }
            else if (resp==QMessageBox::Yes)
            {
                if(!saveProject(m_FileName))
                {
                    XFile.close();
                    return XFLR5::NOAPP;
                }
            }
        }

        deleteProject();

        QDataStream ar(&XFile);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if(!serializeProjectXFL(ar, false))
        {
            QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
        }
        QApplication::restoreOverrideCursor();

        addRecentFile(pathname);
        setSaveState(true);
        setProjectName(pathname);

        XFile.close();

        if(Objects3d::s_oaPlane.size()) return XFLR5::MIAREX;
        else                            return XFLR5::XFOILANALYSIS;
    }


    XFile.close();

    return XFLR5::NOAPP;
}


void MainFrame::onAFoil()
{
    m_pMiarex->stopAnimate();

    m_pXDirect->stopAnimate();

    m_iApp = XFLR5::DIRECTDESIGN;
    m_pctrlMiarexToolBar->hide();
    m_pctrlStabViewWidget->hide();
    m_pctrlXDirectToolBar->hide();
    m_pctrlXInverseToolBar->hide();
    m_pctrlAFoilToolBar->show();

    m_pctrlMiarexWidget->hide();
    m_pctrlXDirectWidget->hide();
    m_pctrlXInverseWidget->hide();
    m_pctrlAFoilWidget->show();

    setMainFrameCentralWidget();
    setMenus();
    m_pAFoil->setAFoilParams();
    updateView();
}


void MainFrame::onCurFoilStyle()
{
    if(!XDirect::curFoil()) return;

    LinePickerDlg dlg(this);
    dlg.initDialog(XDirect::curFoil()->foilPointStyle(), XDirect::curFoil()->foilLineStyle(), XDirect::curFoil()->foilLineWidth(),
                   colour(XDirect::curFoil()), true);

    if(QDialog::Accepted==dlg.exec())
    {
        XDirect::curFoil()->setColor(dlg.lineColor().red(), dlg.lineColor().green(), dlg.lineColor().blue(), dlg.lineColor().alpha());
        XDirect::curFoil()->foilLineStyle() = dlg.lineStyle();
        XDirect::curFoil()->foilLineWidth() = dlg.lineWidth();
        XDirect::curFoil()->foilPointStyle() = dlg.pointStyle();

        if(Settings::isAlignedChildrenStyle())
            Objects2d::setFoilChildrenStyle(XDirect::curFoil());

        m_pXDirect->setControls();
        m_pXDirect->m_bResetCurves = true;
        setSaveState(false);
    }

    updateView();
}


void MainFrame::onInsertProject()
{
    QString PathName;

    PathName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                            Settings::lastDirName(),
                                            "Project file (*.wpa *.xfl)");
    if(!PathName.length()) return;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) Settings::setLastDirName(PathName.left(pos));

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,tr("Warning"), tr("Could not read the file\n")+ PathName);
        return;
    }

    QString end = PathName.right(4).toLower();

    if(end==".wpa")
    {
        QDataStream ar(&XFile);
        ar.setVersion(QDataStream::Qt_4_5);
        ar.setByteOrder(QDataStream::LittleEndian);
        if(!serializeProjectWPA(ar, false))
        {
            QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+PathName+"\n");
        }

    }
    else if(end==".xfl")
    {
        QDataStream ar(&XFile);
        if(!serializeProjectXFL(ar, false))
        {
            QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+PathName+"\n");
        }
    }

    XFile.close();
    setSaveState(false);

    if(m_iApp == XFLR5::MIAREX)
    {
        updatePlaneListBox();
        m_pMiarex->setPlane();
        Miarex::s_bResetCurves = true;
    }
    else if(m_iApp == XFLR5::XFOILANALYSIS)
    {
        if(m_pXDirect->m_bPolarView) m_pXDirect->createPolarCurves();
        else                         m_pXDirect->createOppCurves();
        updateFoilListBox();
    }
    else if(m_iApp == XFLR5::DIRECTDESIGN)
    {
        m_pAFoil->fillFoilTable();
        m_pAFoil->selectFoil();
    }
    updateView();
}


void MainFrame::onHighlightOperatingPoint()
{
    Graph::setOppHighlighting(!Graph::isHighLighting());
    m_pHighlightOppAct->setChecked(Graph::isHighLighting());

    if(m_iApp == XFLR5::MIAREX)
    {
        Miarex::s_bResetCurves = true;
        m_pMiarex->updateView();
    }
    else if(m_iApp == XFLR5::XFOILANALYSIS)
    {
        m_pXDirect->m_bResetCurves = true;
        m_pXDirect->updateView();
    }
}



void MainFrame::onLoadFile()
{
    QStringList PathNames;
    QString PathName;
    XFLR5::enumApp App  = XFLR5::NOAPP;
    bool warn_non_airfoil_multiload = false;

    PathNames = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                              Settings::lastDirName(),
                                              "XFLR5 file (*.dat *.plr *.wpa *.xfl)");
    if(!PathNames.size()) return;
    if(PathNames.size() > 1)
    {
        for (int i=0; i<PathNames.size(); i++)
        {
            PathName = PathNames.at(i);
            if (PathName.endsWith(".dat"))
            {
                App = loadXFLR5File(PathName);
            } else {
                warn_non_airfoil_multiload = true;
            }
        }
        if (warn_non_airfoil_multiload) {
            QMessageBox::warning(this, QObject::tr("Warning"), QObject::tr("Multiple file loading only available for airfoil files.\nNon *.dat files will be ignored."));
        }
    }
    else
    {
        PathName = PathNames.at(0);
        if(!PathName.length()) return;

        PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

        int pos = PathName.lastIndexOf("/");
        if(pos>0) Settings::setLastDirName(PathName.left(pos));

        App = loadXFLR5File(PathName);
    }

    if(m_iApp==XFLR5::NOAPP)
    {
        m_iApp = App;

        if(m_iApp==XFLR5::MIAREX) onMiarex();
        else                      onXDirect();
    }

    if(App==0)
    {
    }
    else if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        if(Objects2d::polarCount())
        {
            if(m_pXDirect->m_bPolarView) m_pXDirect->createPolarCurves();
            else                         m_pXDirect->createOppCurves();
        }
        updateFoilListBox();
        updateView();
    }
    else if(m_iApp==XFLR5::MIAREX)
    {
        updatePlaneListBox();
        m_pMiarex->setPlane();
        m_pMiarex->setScale();
        m_pMiarex->m_bIs2DScaleSet = false;
        m_pMiarex->setControls();
        updateView();
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        m_pAFoil->setAFoilParams();
        m_pAFoil->selectFoil(XDirect::curFoil());
        updateView();
    }
    else if(m_iApp==XFLR5::INVERSEDESIGN)
    {
        onXInverse();
        updateView();
    }
}


void MainFrame::onLogFile()
{
    QString FileName = QDir::tempPath() + "/XFLR5.log";
    // 20090605 Francesco Meschia
    QDesktopServices::openUrl(QUrl::fromLocalFile(FileName));
}




void MainFrame::onNewProject()
{
    //m_pctrlPyConsoleWidget->
    if(!s_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Question"), tr("Save the current project ?"),
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if (QMessageBox::Cancel == resp)
        {
            return;
        }
        else if (QMessageBox::Yes == resp)
        {
            if(saveProject(m_FileName))
            {
                deleteProject();
                statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
            }
            else return; //save failed, don't close
        }
        else if (QMessageBox::No == resp)
        {
            deleteProject();
        }
    }
    else
    {
        deleteProject();
    }

    m_pMiarex->    m_PixText.fill(Qt::transparent);
    m_pgl3dMiarexView->m_bArcball = false;

    updateView();
}


void MainFrame::onOpenGLInfo()
{
    View3dTestDlg w(this);
    w.resize(900, 800);
    w.exec();
}


void MainFrame::onResetSettings()
{
    int resp = QMessageBox::question(this, tr("Default Settings"), tr("Are you sure you want to reset the default settings ?"),
                                     QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    if(resp == QMessageBox::Yes)
    {
        QMessageBox::warning(this,tr("Default Settings"), tr("The settings will be reset at the next session"));

#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

        settings.clear();
        Settings::setLastDirName(QDir::homePath());
        Settings::setXmlDirName(QDir::homePath());
        Settings::setPlrDirName(QDir::homePath());
        // do not save on exit
        m_bSaveSettings = false;
    }
}


void MainFrame::onRestoreToolbars()
{
    if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        m_pctrlXInverseToolBar->hide();
        m_pctrlAFoilToolBar->hide();
        m_pctrlMiarexToolBar->hide();
        m_pctrlStabViewWidget->hide();
        m_pctrl3DScalesWidget->hide();

        m_pctrlAFoilWidget->hide();
        m_pctrlXInverseWidget->hide();
        m_pctrlMiarexWidget->hide();

        m_pctrlXDirectToolBar->show();
        m_pctrlXDirectWidget->show();
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        m_pctrlXInverseToolBar->hide();
        m_pctrlMiarexToolBar->hide();
        m_pctrlXDirectToolBar->hide();
        m_pctrl3DScalesWidget->hide();

        m_pctrlXDirectWidget->hide();
        m_pctrlXInverseWidget->hide();
        m_pctrlMiarexWidget->hide();
        m_pctrlStabViewWidget->hide();

        m_pctrlAFoilToolBar->show();
        m_pctrlAFoilWidget->show();
    }
    else if(m_iApp==XFLR5::INVERSEDESIGN)
    {
        m_pctrlAFoilToolBar->hide();
        m_pctrlMiarexToolBar->hide();
        m_pctrlXDirectToolBar->hide();
        m_pctrl3DScalesWidget->hide();
        m_pctrlStabViewWidget->hide();

        m_pctrlAFoilWidget->hide();
        m_pctrlXDirectWidget->hide();
        m_pctrlMiarexWidget->hide();

        m_pctrlXInverseToolBar->show();
        m_pctrlXInverseWidget->show();
    }
    else if(m_iApp==XFLR5::MIAREX)
    {
        m_pctrlXInverseToolBar->hide();
        m_pctrlAFoilToolBar->hide();
        m_pctrlXDirectToolBar->hide();
        m_pctrl3DScalesWidget->hide();

        m_pctrlAFoilWidget->hide();
        m_pctrlXDirectWidget->hide();
        m_pctrlXInverseWidget->hide();
        m_pctrlMiarexWidget->show();
        m_pctrlMiarexToolBar->show();
        //        if(pMiarex->m_iView==WSTABVIEW) m_pctrlStabViewWidget->show();
        //        else                            m_pctrlStabViewWidget->hide();
    }
}



void MainFrame::onSaveTimer()
{
    if (!s_ProjectName.length()) return;
    if(saveProject(m_FileName))
    {
        statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
    }
}


void MainFrame::onSaveProject()
{
    /*    QString Filter = "XFLR5 v6 Project File (*.xfl)";

    QString saveFileName = "/home/techwinder/download/save.bin";
    QFile fp(saveFileName);
    fp.open(QIODevice::WriteOnly);
    QDataStream ar(&fp);

    ar<<1025;
    ar << QColor(159,158,157,156);

    ar << QColor(7,6,5,4);
    fp.close();
    return;*/

    QString FileName = s_ProjectName;

    if (!s_ProjectName.length())
    {
        onSaveProjectAs();
        return;
    }
    if(saveProject(m_FileName))
    {
        addRecentFile(m_FileName);
        statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
    }
    m_pMiarex->updateView();
}



bool MainFrame::onSaveProjectAs()
{
    if(saveProject())
    {
        setProjectName(m_FileName);
        addRecentFile(m_FileName);
        statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
    }

    return true;
}




void MainFrame::onSaveViewToImageFile()
{
    QString FileName, Filter;

    switch(m_ImageFormat)
    {
        case XFLR5::PNG :
        {
            Filter = "Portable Network Graphics (*.png)";
            break;
        }
        case XFLR5::JPEG :
        {
            Filter = "JPEG (*.jpg)";
            break;
        }
        case XFLR5::BMP :
        {
            Filter = "Windows Bitmap (*.bmp)";
            break;
        }
    }

    FileName = QFileDialog::getSaveFileName(this, tr("Save Image"),
                                            m_ImageDirName,
                                            "Portable Network Graphics (*.png);;JPEG (*.jpg);;Windows Bitmap (*.bmp)",
                                            &Filter);

    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) m_ImageDirName = FileName.left(pos);

    if(Filter == "Portable Network Graphics (*.png)")
    {
        if(FileName.right(4)!=".png") FileName+= ".png";
        m_ImageFormat = XFLR5::PNG;
    }
    else if(Filter == "JPEG (*.jpg)")
    {
        if(FileName.right(4)!=".jpg") FileName+= ".jpg";
        m_ImageFormat = XFLR5::JPEG;
    }
    else if(Filter == "Windows Bitmap (*.bmp)")
    {
        if(FileName.right(4)!=".bmp") FileName+= ".bmp";
        m_ImageFormat = XFLR5::BMP;
    }

    switch(m_iApp)
    {
        case XFLR5::XFOILANALYSIS:
        {
            QPixmap pix = m_pXDirectTileWidget->grab();
            pix.save(FileName, "PNG");
            return;
        }
        case XFLR5::DIRECTDESIGN:
        {
            QPixmap pix = m_pDirect2dWidget->grab();
            pix.save(FileName, "PNG");
            return;
        }
        case XFLR5::INVERSEDESIGN:
        {
            QPixmap pix = m_p2dWidget->grab();
            pix.save(FileName, "PNG");
            return;
        }
        case XFLR5::MIAREX:
        {
            if(m_pMiarex->m_iView==XFLR5::W3DVIEW)
            {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "3D save option", tr("Set a transparent background ?"), QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes)
                {
                    QPixmap outPix = m_pgl3dMiarexView->grab();
                    QPainter painter(&outPix);
                    if(!m_pMiarex->m_PixText.isNull())                  painter.drawPixmap(0,0, m_pMiarex->m_PixText);
                    if(!m_pgl3dMiarexView->m_PixTextOverlay.isNull()) painter.drawPixmap(0,0, m_pgl3dMiarexView->m_PixTextOverlay);

                    outPix.save(FileName);
                }
                else
                {
                    QImage outImg = m_pgl3dMiarexView->grabFramebuffer();
                    QPainter painter(&outImg);
                    if(!m_pMiarex->m_PixText.isNull())                  painter.drawPixmap(0,0, m_pMiarex->m_PixText);
                    if(!m_pgl3dMiarexView->m_PixTextOverlay.isNull()) painter.drawPixmap(0,0, m_pgl3dMiarexView->m_PixTextOverlay);

                    outImg.save(FileName);
                }

                return;
            }
            else
            {
                QPixmap pix = m_pMiarexTileWidget->grab();
                pix.save(FileName, "PNG");
                return;
            }
        }
        default:
            break;
    }
}


void MainFrame::onSelChangePlane(int sel)
{
    // Gets the new selected wing name and notifies Miarex
    // then updates WPolar combobox
    // and selects either the current WPolar
    // or the first one in the list, if any
    m_pMiarex->stopAnimate();
    QString strong;
    //    int sel = m_pctrlPlane->currentIndex();
    if (sel >=0) strong = m_pctrlPlane->itemText(sel);
    m_pMiarex->setPlane(strong);
    m_pMiarex->updateView();
}


void MainFrame::onSelChangeWPolar(int sel)
{
    m_pMiarex->stopAnimate();

    QString strong;
    //    int sel = m_pctrlPlanePolar->currentIndex();
    if (sel>=0) strong = m_pctrlPlanePolar->itemText(sel);
    m_iApp = XFLR5::MIAREX;
    m_pMiarex->setWPolar(false, strong);
    m_pMiarex->setControls();
    m_pMiarex->updateView();
}


void MainFrame::onSelChangePlaneOpp(int sel)
{
    QString strong;
    m_pMiarex->stopAnimate();

    // Gets the new selected WOpp name and notifies Miarex
    if(!m_pctrlPlaneOpp->count())
    {
        Miarex::s_bResetCurves = true;
        m_pMiarex->updateView();
        return;
    }

    if (sel>=0) strong = m_pctrlPlaneOpp->itemText(sel);
    else
    {
        m_pctrlPlaneOpp->setCurrentIndex(0);
        strong = m_pctrlPlaneOpp->itemText(0).trimmed();
    }
    if(strong.length())
    {
        bool bOK;
        double x = locale().toDouble(strong, &bOK);
        if(bOK)
        {
            m_iApp = XFLR5::MIAREX;
            m_pMiarex->setPlaneOpp(false, x);
            m_pMiarex->updateView();
        }
        else
        {
            QMessageBox::warning(window(), tr("Warning"), tr("Unidentified Operating Point"));
            m_pMiarex->setPlaneOpp(true);
        }
    }
}


void MainFrame::onSelChangeFoil(int sel)
{
    // Gets the new selected foil name and notifies XDirect
    // sets a polar, if any
    // then updates the polar combobox

    m_pXDirect->m_bAnimate = false;

    QString strong;
    //    int sel = m_pctrlFoil->currentIndex();
    if (sel >=0) strong = m_pctrlFoil->itemText(sel);

    m_pXDirect->setCurFoil(Objects2d::foil(strong));
    m_pXDirect->setFoil(XDirect::curFoil());
    m_pXDirect->setPolar();
    m_iApp = XFLR5::XFOILANALYSIS;
    updatePolarListBox();
    m_pXDirect->setControls();
    updateView();
}


void MainFrame::onSelChangePolar(int sel)
{
    m_pXDirect->m_bAnimate = false;

    // Gets the new selected polar name and notifies XDirect
    QString strong;
    //    int selNew = m_pctrlPolar->currentIndex();
    if (sel>=0) strong = m_pctrlPolar->itemText(sel);
    m_iApp = XFLR5::XFOILANALYSIS;


    m_pXDirect->setPolar(Objects2d::getPolar(XDirect::curFoil(), strong));
    updateOppListBox();
    m_pXDirect->setControls();
    updateView();
}


void MainFrame::onSelChangeOpp(int sel)
{
    m_pXDirect->m_bAnimate = false;

    // Gets the new selected Opp name and notifies XDirect
    QString strong;

    if (sel>=0) strong = m_pctrlOpPoint->itemText(sel);
    m_iApp = XFLR5::XFOILANALYSIS;

    double Alpha=0;
    bool bOK=false;
    Alpha = locale().toDouble(strong, &bOK);

    if(bOK)
    {
        m_pXDirect->setOpp(Alpha);
    }
    else
    {
        QMessageBox::warning(window(), tr("Warning"), tr("Unidentified Operating Point"));
        m_pXDirect->setCurOpp(nullptr);
        m_pXDirect->setOpp();
    }
    updateView();
}


void MainFrame::onShowMousePos()
{
    s_bShowMousePos = !s_bShowMousePos;
    m_pShowMousePosAct->setChecked(s_bShowMousePos);
}




void MainFrame::onXDirect()
{
    m_pMiarex->stopAnimate();

    m_iApp = XFLR5::XFOILANALYSIS;

    m_pctrlMiarexToolBar->hide();
    m_pctrlStabViewWidget->hide();
    m_pctrlAFoilToolBar->hide();
    m_pctrlXInverseToolBar->hide();
    m_pctrlXDirectToolBar->show();

    m_pctrlAFoilWidget->hide();
    m_pctrlMiarexWidget->hide();
    m_pctrlXInverseWidget->hide();
    m_pctrlXDirectWidget->show();

    m_pXDirect->setFoil();
    updateFoilListBox();
    setMainFrameCentralWidget();
    setMenus();
    checkGraphActions();

    m_pXDirect->setControls();
    m_pXDirect->setFoilScale();
    m_pXDirect->updateView();
}




void MainFrame::onMiarex()
{
    m_pXDirect->stopAnimate();
    m_iApp = XFLR5::MIAREX;

    m_pctrlXDirectToolBar->hide();
    m_pctrlXInverseToolBar->hide();
    m_pctrlAFoilToolBar->hide();
    m_pctrlMiarexToolBar->show();

    m_pctrlXDirectWidget->hide();
    m_pctrlAFoilWidget->hide();
    m_pctrlXInverseWidget->hide();
    m_pctrlMiarexWidget->show();

    updatePlaneListBox();
    m_pMiarex->setPlane();

    setMenus();
    setMainFrameCentralWidget();
    checkGraphActions();
    m_pMiarex->setControls();

    m_pMiarex->setCurveParams();
    updateView();
}



void MainFrame::onXInverse()
{
    m_pXDirect->stopAnimate();
    m_pMiarex->stopAnimate();

    //    pXInverse->SetScale();
    m_iApp = XFLR5::INVERSEDESIGN;

    m_pctrlMiarexToolBar->hide();
    m_pctrlStabViewWidget->hide();
    m_pctrlAFoilToolBar->hide();
    m_pctrlXDirectToolBar->hide();
    m_pctrlXInverseToolBar->show();

    m_pctrlAFoilWidget->hide();
    m_pctrlMiarexWidget->hide();
    m_pctrlXDirectWidget->hide();
    m_pctrlXInverseWidget->show();

    setMainFrameCentralWidget();
    setMenus();
    checkGraphActions();
    m_pXInverse->setParams();
    m_pXInverse->updateView();
}


void MainFrame::onXInverseMixed()
{
    m_pXDirect->stopAnimate();
    m_pMiarex->stopAnimate();

    //    pXInverse->SetScale();
    m_iApp = XFLR5::INVERSEDESIGN;
    m_pctrlMiarexToolBar->hide();
    m_pctrlStabViewWidget->hide();
    m_pctrlAFoilToolBar->hide();
    m_pctrlXDirectToolBar->hide();
    m_pctrlXInverseToolBar->show();

    m_pctrlAFoilWidget->hide();
    m_pctrlMiarexWidget->hide();
    m_pctrlXDirectWidget->hide();
    m_pctrlXInverseWidget->show();
    m_pXInverse->m_bFullInverse = false;
    setMainFrameCentralWidget();
    setMenus();
    checkGraphActions();
    m_pXInverse->setParams();
    m_pXInverse->updateView();
}


void MainFrame::onOpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    XFLR5::enumApp App = loadXFLR5File(action->data().toString());
    if(m_iApp==XFLR5::NOAPP) m_iApp = App;

    if(App==XFLR5::NOAPP)
    {
        m_iApp = App;
        QString FileName = action->data().toString();
        m_RecentFiles.removeAll(FileName);
        updateRecentFileActions();
    }

    else if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        if(Objects2d::polarCount())
        {
            if(m_pXDirect->bPolarView()) m_pXDirect->createPolarCurves();
            else                         m_pXDirect->createOppCurves();
        }
        updateFoilListBox();
        onXDirect();
    }
    else if(m_iApp==XFLR5::MIAREX)
    {
        onMiarex();
        m_pMiarex->setScale();
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        m_pAFoil->setAFoilParams();
        onAFoil();
        updateView();
    }
    else if(m_iApp==XFLR5::INVERSEDESIGN)
    {
        onXInverse();
        updateView();
    }
}


/**
 *Reads a Foil and its related Polar objects from a binary stream associated to a .plr file.
 * @param ar the binary stream
 * @return the pointer to the Foil object which has been created, or NULL if failure.
 */
void MainFrame::readPolarFile(QDataStream &ar)
{
    Foil* pFoil = nullptr;
    Polar *pPolar = nullptr;
    Polar * pOldPolar = nullptr;
    int i=0, n=0, l=0;

    ar >> n;

    if(n<100000)
    {
        //old format
        QMessageBox::warning(window(), tr("Warning"), tr("Obsolete format, cannot read"));
        return;
    }
    else if (n >=100000)
    {
        //new format XFLR5 v1.99+
        //first read all available foils
        ar>>n;
        for (i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!serializeFoil(pFoil, ar, false))
            {
                delete pFoil;
                return;
            }
            Objects2d::insertThisFoil(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (i=0;i<n; i++)
        {
            pPolar = new Polar();

            if (!serializePolar(pPolar, ar, false))
            {
                delete pPolar;
                return;
            }
            for (l=0; l<Objects2d::polarCount(); l++)
            {
                pOldPolar = Objects2d::polarAt(l);
                if (pOldPolar->foilName() == pPolar->foilName() &&
                        pOldPolar->polarName()  == pPolar->polarName())
                {
                    //just overwrite...
                    Objects2d::deletePolarAt(l);
                    //... and continue to add
                }
            }
            Objects2d::addPolar(pPolar);
        }
    }
}


bool MainFrame::saveProject(QString PathName)
{
    QString Filter = "XFLR5 v6 Project File (*.xfl)";
    QString FileName = s_ProjectName;

    if(!PathName.length())
    {
        PathName = QFileDialog::getSaveFileName(this, tr("Save the Project File"),
                                                Settings::lastDirName()+"/"+FileName,
                                                "XFLR5 v6 Project File (*.xfl)",
                                                &Filter);

        if(!PathName.length()) return false;//nothing more to do

        int pos = PathName.indexOf(".xfl", Qt::CaseInsensitive);
        if(pos<0) PathName += ".xfl";

        PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

        pos = PathName.lastIndexOf("/");
        if(pos>0) Settings::setLastDirName(PathName.left(pos));
    }


    QString backupFileName = QDir::tempPath() + QDir::separator() + s_ProjectName + ".bak";

    QFile::copy(PathName, backupFileName);

    QFile fp(PathName);
    if(fp.exists())
    {
        // move the old file to the temp directory instead of overwriting it
        fp.rename(backupFileName);
    }

    if (!fp.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
        return false;
    }

    QDataStream ar(&fp);
    if(!serializeProjectXFL(ar,true))
    {
        QString strong = tr("Error saving the project file");
        strong +="\n";
        QFile::copy(backupFileName, PathName);
        strong +="The changes have not been saved";
        QMessageBox::critical(window(), tr("Error"), strong);
    }
    else QFile::remove(backupFileName);


    m_FileName = PathName;
    fp.close();

    saveSettings();

    setSaveState(true);

    return true;
}



void MainFrame::onSavePlaneAsProject()
{
    QString strong;
    if(m_pMiarex->m_pCurPlane) strong = m_pMiarex->m_pCurPlane->planeName();
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("Nothing to save"));
        return ;
    }

    QString PathName;
    QString Filter = "XFLR5 v6 Project File (*.xfl)";
    QString FileName = strong;

    PathName = QFileDialog::getSaveFileName(this, tr("Save the Project File"),
                                            Settings::lastDirName()+"/"+FileName,
                                            "XFLR5 v6 Project File (*.xfl)",
                                            &Filter);

    if(!PathName.length()) return;//nothing more to do
    int pos = PathName.indexOf(".xfl", Qt::CaseInsensitive);
    if(pos<0) PathName += ".xfl";
    PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator
    pos = PathName.lastIndexOf("/");
    if(pos>0) Settings::setLastDirName(PathName.left(pos));


    QFile fp(PathName);

    if (!fp.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
        return;
    }

    QDataStream ar(&fp);

    serializePlaneProject(ar);
    fp.close();
}



bool MainFrame::serializePlaneProject(QDataStream &ar)
{
    if(!m_pMiarex->m_pCurPlane)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Nothing to save"));
        return false;
    }

    WPolar *pWPolar = nullptr;
    Polar *pPolar   = nullptr;

    QString PlaneName = m_pMiarex->m_pCurPlane->planeName();

    bool bIsStoring = true;
    int i=0, iSize=0;

    int ArchiveFormat = 200001;
    ar << ArchiveFormat;
    // 200001 : First instance of new ".xfl" format

    //Save unit data
    ar << Units::lengthUnitIndex();
    ar << Units::areaUnitIndex();
    ar << Units::weightUnitIndex();
    ar << Units::speedUnitIndex();
    ar << Units::forceUnitIndex();
    ar << Units::momentUnitIndex();

    //Save default Polar data. Not in the Settings, since this is Project dependant
    if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
    else if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
    else if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDAOAPOLAR)   ar<<4;
    else if(WPolarDlg::s_WPolar.polarType()==XFLR5::BETAPOLAR)       ar<<5;
    else if(WPolarDlg::s_WPolar.polarType()==XFLR5::STABILITYPOLAR)  ar<<7;
    else ar << 0;

    if(WPolarDlg::s_WPolar.isLLTMethod())             ar << 1;
    else if(WPolarDlg::s_WPolar.isVLMMethod())        ar << 2;
    else if(WPolarDlg::s_WPolar.isPanel4Method())     ar << 3;
    else if(WPolarDlg::s_WPolar.isTriCstMethod())     ar << 4;
    else if(WPolarDlg::s_WPolar.isTriLinearMethod())  ar << 5;
    else ar << 0;


    ar << WPolarDlg::s_WPolar.mass();
    ar << WPolarDlg::s_WPolar.m_QInfSpec;
    ar << WPolarDlg::s_WPolar.CoG().x;
    ar << WPolarDlg::s_WPolar.CoG().y;
    ar << WPolarDlg::s_WPolar.CoG().z;

    ar << WPolarDlg::s_WPolar.density();
    ar << WPolarDlg::s_WPolar.viscosity();
    ar << WPolarDlg::s_WPolar.m_AlphaSpec;
    ar << WPolarDlg::s_WPolar.m_BetaSpec;

    ar << WPolarDlg::s_WPolar.bTilted();
    ar << WPolarDlg::s_WPolar.bWakeRollUp();

    // save the plane
    ar << 1;
    m_pMiarex->m_pCurPlane->serializePlaneXFL(ar, bIsStoring);

    // save the WPolars associated to this plane
    //count the polars
    iSize = 0;
    for (i=0; i<Objects3d::s_oaWPolar.size();i++)
    {
        pWPolar = Objects3d::s_oaWPolar.at(i);
        if(pWPolar->planeName()==PlaneName) iSize++;
    }
    ar << iSize;
    for (i=0; i<Objects3d::s_oaWPolar.size();i++)
    {
        pWPolar = Objects3d::s_oaWPolar.at(i);
        if(pWPolar->planeName()==PlaneName) pWPolar->serializeWPlrXFL(ar, bIsStoring);
    }

    ar << 0; //no need to save the operating points

    // then the foils
    // list the foils associated to this Plane's wings
    QVector<Foil*> foilList;
    for(i=0; i<Objects2d::foilCount(); i++)
    {
        Foil *pFoil = Objects2d::foilAt(i);
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pMiarex->m_pCurPlane->m_Wing[iw].isWingFoil(pFoil))
            {
                foilList.append(pFoil);
                break;
            }
        }
    }

    ar << foilList.count();
    for(int iFoil=0; iFoil<foilList.size(); iFoil++)
    {
        serializeFoilXFL(foilList.at(iFoil), ar, bIsStoring);
    }
    // the foil polars
    // list the foil polars associated to this Plane's wings
    QVector<Polar*> polarList;
    for(i=0; i<Objects2d::polarCount(); i++)
    {
        pPolar = Objects2d::polarAt(i);
        for(int iFoil=0; iFoil<foilList.count(); iFoil++)
        {
            if(pPolar->foilName() == foilList.at(iFoil)->foilName())
            {
                bool bListed = false;
                for(int ip=0; ip<polarList.count(); ip++)
                {
                    if(pPolar==polarList.at(ip))
                    {
                        bListed = true;
                        break; // polar list
                    }
                }
                if(!bListed)
                {
                    polarList.append(pPolar);
                    break;  //foil list
                }
            }
        }
    }


    ar << polarList.size();
    for (int ip=0; ip<polarList.size();ip++)
    {
        serializePolarXFL(polarList.at(ip), ar, true);
    }
    ar << 0; //no need to save the operating points

    // and the spline foil whilst we're at it
    m_pAFoil->m_pSF->serialize(ar, bIsStoring);

    return true;
}


void MainFrame::saveSettings()
{
    if(!m_bSaveSettings) return;

#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

    settings.beginGroup("MainFrame");
    {
        settings.setValue("SettingsFormat", SETTINGSFORMAT);
        settings.setValue("FrameGeometryx", frameGeometry().x());
        settings.setValue("FrameGeometryy", frameGeometry().y());
        settings.setValue("SizeWidth", size().width());
        settings.setValue("SizeHeight", size().height());
        settings.setValue("SizeMaximized", isMaximized());
        settings.setValue("StyleName", Settings::s_StyleName);

        if (Settings::s_ExportFileType==XFLR5::TXT) settings.setValue("ExportFileType", 0);
        else                                        settings.setValue("ExportFileType", 1);

        settings.setValue("GraphExportFilter", m_GraphExportFilter);
        settings.setValue("Miarex_Float", m_pctrlMiarexWidget->isFloating());
        settings.setValue("XDirect_Float", m_pctrlXDirectWidget->isFloating());
        settings.setValue("AFoil_Float", m_pctrlAFoilWidget->isFloating());
        settings.setValue("XInverse_Float", m_pctrlXInverseWidget->isFloating());
        settings.setValue("StabView_Float", m_pctrlStabViewWidget->isFloating());
        settings.setValue("Miarex_x", m_pctrlMiarexWidget->frameGeometry().x());
        settings.setValue("Miarex_y", m_pctrlMiarexWidget->frameGeometry().y());
        settings.setValue("XDirect_x", m_pctrlXDirectWidget->frameGeometry().x());
        settings.setValue("XDirect_y", m_pctrlXDirectWidget->frameGeometry().y());
        settings.setValue("AFoil_x", m_pctrlAFoilWidget->frameGeometry().x());
        settings.setValue("AFoil_y", m_pctrlAFoilWidget->frameGeometry().y());
        settings.setValue("XInverse_x", m_pctrlXInverseWidget->frameGeometry().x());
        settings.setValue("XInverse_y", m_pctrlXInverseWidget->frameGeometry().y());
        settings.setValue("StabView_x", m_pctrlStabViewWidget->frameGeometry().x());
        settings.setValue("StabView_y", m_pctrlStabViewWidget->frameGeometry().y());

        settings.setValue("XDirectSize", m_pctrlXDirectWidget->size());
        settings.setValue("AFoilSize", m_pctrlAFoilWidget->size());
        settings.setValue("XInverseSize", m_pctrlXInverseWidget->size());
        settings.setValue("MiarexSize", m_pctrlMiarexWidget->size());
        settings.setValue("StabSize", m_pctrlStabViewWidget->size());

        settings.setValue("ImageDirName", m_ImageDirName);
        settings.setValue("ExportLastDirName", m_ExportLastDirName);

        settings.setValue("LengthUnit",   Units::lengthUnitIndex());
        settings.setValue("AreaUnit",     Units::areaUnitIndex());
        settings.setValue("WeightUnit",   Units::weightUnitIndex());
        settings.setValue("SpeedUnit",    Units::speedUnitIndex());
        settings.setValue("ForceUnit",    Units::forceUnitIndex());
        settings.setValue("MomentUnit",   Units::momentUnitIndex());
        settings.setValue("PressureUnit", Units::pressureUnitIndex());
        settings.setValue("InertiaUnit",  Units::inertiaUnitIndex());

        settings.setValue("LanguageFilePath", s_LanguageFilePath);
        settings.setValue("ImageFormat", m_ImageFormat);
        settings.setValue("AutoSaveProject", m_bAutoSave);
        settings.setValue("AutoSaveInterval", m_SaveInterval);
        settings.setValue("AutoLoadLastProject",m_bAutoLoadLast);
        settings.setValue("SaveOpps", m_bSaveOpps);
        settings.setValue("SaveWOpps", m_bSaveWOpps);
        settings.setValue("RecentFileSize", m_RecentFiles.size());
        settings.setValue("ShowStyleSheets", Settings::s_bStyleSheets);
        settings.setValue("StyleSheetName", Settings::s_StyleSheetName);

        settings.setValue("ShowMousePosition", s_bShowMousePos);

        QString RecentF;
        for(int i=0; i<m_RecentFiles.size() && i<MAXRECENTFILES; i++)
        {
            RecentF = QString("RecentFile_%1").arg(i);
            if(m_RecentFiles[i].length()) settings.setValue(RecentF, m_RecentFiles.at(i));
            else                          settings.setValue(RecentF, "");
        }
        for(int i=m_RecentFiles.size(); i<MAXRECENTFILES; i++)
        {
            RecentF = QString("RecentFile_%1").arg(i);
            settings.setValue(RecentF, "");
        }

        Updater::saveSettings(settings);
    }
    settings.endGroup();

    Settings::saveSettings(settings);
    m_pAFoil->saveSettings(&settings);
    m_pXDirect->saveSettings(settings);
    m_pMiarex->saveSettings(settings);
    m_pXInverse->saveSettings(settings);
    GL3DScales::saveSettings(&settings);
    W3dPrefsDlg::saveSettings(&settings);
}


void MainFrame::setMainFrameCentralWidget()
{
    if(m_iApp==XFLR5::NOAPP)
    {
        m_pctrlCentralWidget->setCurrentWidget(&m_VoidWidget);
    }
    else if(m_iApp==XFLR5::MIAREX)
    {
        if (m_pMiarex->m_iView==XFLR5::WOPPVIEW || m_pMiarex->m_iView==XFLR5::WPOLARVIEW || m_pMiarex->m_iView==XFLR5::WCPVIEW ||
                m_pMiarex->m_iView==XFLR5::STABPOLARVIEW  || m_pMiarex->m_iView==XFLR5::STABTIMEVIEW)
        {
            m_pctrlCentralWidget->setCurrentWidget(m_pMiarexTileWidget);
            m_pMiarex->setGraphTiles();
            m_pMiarexTileWidget->setFocus();
        }
        else if(m_pMiarex->m_iView==XFLR5::W3DVIEW)
        {
            m_pctrlCentralWidget->setCurrentWidget(m_pgl3dMiarexView);
            m_pgl3dMiarexView->setFocus();
        }
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        m_pctrlCentralWidget->setCurrentWidget(m_pDirect2dWidget);
        m_pDirect2dWidget->setFocus();
    }
    else if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        m_pctrlCentralWidget->setCurrentWidget(m_pXDirectTileWidget);
        m_pXDirect->setGraphTiles();
        m_pXDirectTileWidget->setFocus();
    }
    else if(m_iApp==XFLR5::INVERSEDESIGN)
    {
        m_pctrlCentralWidget->setCurrentWidget(m_p2dWidget);
        m_p2dWidget->setFocus();
    }
}


void MainFrame::selectFoil(Foil *pFoil)
{
    if(!m_pctrlFoil->count()) return;

    m_pctrlFoil->blockSignals(true);

    if (pFoil)
    {
        int pos = m_pctrlFoil->findText(pFoil->foilName());

        if (pos>=0) m_pctrlFoil->setCurrentIndex(pos);
        else        m_pctrlFoil->setCurrentIndex(0);
    }
    else
    {
        m_pctrlFoil->setCurrentIndex(0);
    }
    m_pctrlFoil->blockSignals(false);
}




void MainFrame::selectPolar(Polar *pPolar)
{
    if(!m_pctrlPolar->count()) return;

    m_pctrlPolar->blockSignals(true);

    if (pPolar)
    {
        int pos = m_pctrlPolar->findText(pPolar->polarName());

        if (pos>=0) m_pctrlPolar->setCurrentIndex(pos);
        else        m_pctrlPolar->setCurrentIndex(0);
    }
    else
    {
        m_pctrlPolar->setCurrentIndex(0);
    }

    m_pctrlPolar->blockSignals(false);
}



/**
 *Selects the operating point in the combobox and returns true
 *On error, selects the first and returns false
 */
void MainFrame::selectOpPoint(OpPoint *pOpp)
{
    Polar *pCurPlr = m_pXDirect->curPolar();

    if(!pOpp || !pCurPlr) return;
    if(!m_pctrlOpPoint->count()) return;

    QString strange;
    bool bOK;

    m_pctrlOpPoint->blockSignals(true);

    for(int i=0; i<m_pctrlOpPoint->count(); i++)
    {
        strange = m_pctrlOpPoint->itemText(i).trimmed();

        double val = locale().toDouble(strange, &bOK);

        if(pCurPlr->polarType() == XFLR5::FIXEDAOAPOLAR)
        {
            if(bOK && qAbs(val-pOpp->Reynolds())<1.0)
            {
                m_pctrlOpPoint->setCurrentIndex(i);
                break;
            }
        }
        else
        {
            if(bOK && qAbs(val-pOpp->aoa())<0.001)
            {
                m_pctrlOpPoint->setCurrentIndex(i);
                break;
            }
        }
    }
    m_pctrlOpPoint->blockSignals(false);
}


void MainFrame::selectPlane(Plane *pPlane)
{
    if(!m_pctrlPlane->count()) return;
    m_pctrlPlane->blockSignals(true);

    if(pPlane)
    {
        int pos = m_pctrlPlane->findText(pPlane->planeName());
        if(pos>=0) m_pctrlPlane->setCurrentIndex(pos);
        else if(m_pctrlPlane->count())
            m_pctrlPlane->setCurrentIndex(0);
    }
    else
    {
        m_pctrlPlane->setCurrentIndex(0);
    }

    m_pctrlPlane->blockSignals(false);
}


void MainFrame::selectWPolar(WPolar *pWPolar)
{
    if(!m_pctrlPlanePolar->count()) return;

    m_pctrlPlanePolar->blockSignals(true);

    if(pWPolar)
    {
        int pos = m_pctrlPlanePolar->findText(pWPolar->polarName());
        if (pos>=0) m_pctrlPlanePolar->setCurrentIndex(pos);
        else if(m_pctrlPlanePolar->count())
            m_pctrlPlanePolar->setCurrentIndex(0);
    }
    else
    {
        m_pctrlPlanePolar->setCurrentIndex(0);
    }
    m_pctrlPlanePolar->blockSignals(false);
}


void MainFrame::selectPlaneOpp(PlaneOpp *pPlaneOpp)
{
    double x = 0.0;

    if(pPlaneOpp)
    {
        if(pPlaneOpp->polarType()<XFLR5::FIXEDAOAPOLAR)        x = pPlaneOpp->m_Alpha;
        else if(pPlaneOpp->polarType()==XFLR5::FIXEDAOAPOLAR)  x = pPlaneOpp->m_QInf;
        else if(pPlaneOpp->polarType()==XFLR5::BETAPOLAR)      x = pPlaneOpp->m_Beta;
        else if(pPlaneOpp->polarType()==XFLR5::STABILITYPOLAR) x = pPlaneOpp->m_Ctrl;
    }
    //Selects a pOpp in the combobox
    WPolar *pCurWPlr = m_pMiarex->m_pCurWPolar;
    if(!pCurWPlr)
    {
        m_pctrlPlaneOpp->clear();
        m_pctrlPlaneOpp->setEnabled(false);
        return;
    }
    double val=0;
    bool bOK=false;
    QString strange;

    m_pctrlPlaneOpp->blockSignals(true);
    //    x = (double)qRound(x*100.0)/100.0;

    for(int i=0; i<m_pctrlPlaneOpp->count(); i++)
    {
        strange = m_pctrlPlaneOpp->itemText(i);
        val = locale().toDouble(strange, &bOK);

        if(pCurWPlr->polarType()<XFLR5::FIXEDAOAPOLAR)
        {
            if(bOK && qAbs(val-x)<0.001)
            {
                m_pctrlPlaneOpp->setCurrentIndex(i);
                break;
            }
        }
        else if(pCurWPlr->polarType()==XFLR5::FIXEDAOAPOLAR)
        {
            if(bOK && qAbs(val-x)<1.0)
            {
                m_pctrlPlaneOpp->setCurrentIndex(i);
                break;
            }
        }
        else if(pCurWPlr->polarType()==XFLR5::BETAPOLAR)
        {
            if(bOK && qAbs(val-x)<0.001)
            {
                m_pctrlPlaneOpp->setCurrentIndex(i);
                break;
            }
        }
        else if(pCurWPlr->polarType()==XFLR5::STABILITYPOLAR)
        {
            if(bOK && qAbs(val-x)<.001)
            {
                m_pctrlPlaneOpp->setCurrentIndex(i);
                break;
            }
        }
    }
    m_pctrlPlaneOpp->blockSignals(false);
}


bool MainFrame::serializeProjectXFL(QDataStream &ar, bool bIsStoring)
{
    WPolar *pWPolar = nullptr;
    PlaneOpp *pPOpp = nullptr;
    Plane *pPlane   = nullptr;
    Polar *pPolar   = nullptr;
    OpPoint *pOpp    = nullptr;

    int i=0, n=0;
    float f=0;
    double dble=0;

    if (bIsStoring)
    {
        // storing code
        int ArchiveFormat = 200002;
        ar << ArchiveFormat;
        // 200001 : First instance of new ".xfl" format

        //Save unit data
        ar << Units::lengthUnitIndex();
        ar << Units::areaUnitIndex();
        ar << Units::weightUnitIndex();
        ar << Units::speedUnitIndex();
        ar << Units::forceUnitIndex();
        ar << Units::momentUnitIndex();


        //Save default Polar data. Not in the Settings, since this is Project dependant
        /*        if(WPolarDlg::s_WPolar.isFixedSpeedPolar())       ar<<1;
        else if(WPolarDlg::s_WPolar.isFixedLiftPolar())   ar<<2;
        else if(WPolarDlg::s_WPolar.isFixedaoaPolar())    ar<<4;
        else if(WPolarDlg::s_WPolar.isBetaPolar())        ar<<5;
        else if(WPolarDlg::s_WPolar.isStabilityPolar())   ar<<7;
        else ar << 0;

        if(WPolarDlg::s_WPolar.isLLTMethod())             ar << 1;
        else if(WPolarDlg::s_WPolar.isVLMMethod())        ar << 2;
        else if(WPolarDlg::s_WPolar.isPanel4Method())     ar << 3;
        else if(WPolarDlg::s_WPolar.isTriCstMethod())     ar << 4;
        else if(WPolarDlg::s_WPolar.isTriLinearMethod())  ar << 5;
        else ar << 0;

        ar << WPolarDlg::s_WPolar.mass();
        ar << WPolarDlg::s_WPolar.m_QInfSpec;
        ar << WPolarDlg::s_WPolar.CoG().x;
        ar << WPolarDlg::s_WPolar.CoG().y;
        ar << WPolarDlg::s_WPolar.CoG().z;

        ar << WPolarDlg::s_WPolar.density();
        ar << WPolarDlg::s_WPolar.viscosity();
        ar << WPolarDlg::s_WPolar.m_AlphaSpec;
        ar << WPolarDlg::s_WPolar.m_BetaSpec;

        ar << WPolarDlg::s_WPolar.bTilted();
        ar << WPolarDlg::s_WPolar.bWakeRollUp();*/

        // format 200002
        // saving WPolar full data including extra drag
        WPolarDlg::s_WPolar.serializeWPlrXFL(ar, true);

        // save the planes...
        ar << Objects3d::s_oaPlane.size();
        for (i=0; i<Objects3d::s_oaPlane.size();i++)
        {
            pPlane = Objects3d::s_oaPlane.at(i);
            pPlane->serializePlaneXFL(ar, bIsStoring);
        }

        // save the WPolars
        ar << Objects3d::s_oaWPolar.size();
        for (i=0; i<Objects3d::s_oaWPolar.size();i++)
        {
            pWPolar = Objects3d::s_oaWPolar.at(i);
            pWPolar->serializeWPlrXFL(ar, bIsStoring);
        }

        if(m_bSaveWOpps)
        {
            // not forgetting their POpps
            ar << Objects3d::s_oaPOpp.size();
            for (i=0; i<Objects3d::s_oaPOpp.size();i++)
            {
                pPOpp = Objects3d::s_oaPOpp.at(i);
                pPOpp->serializePOppXFL(ar, bIsStoring);
            }
        }
        else ar << 0;

        // then the foils
        ar << Objects2d::foilCount();
        for(int i=0; i<Objects2d::foilCount(); i++)
        {
            Foil *pFoil = Objects2d::foilAt(i);
            serializeFoilXFL(pFoil, ar, bIsStoring);
        }

        //the foil polars
        ar << Objects2d::polarCount();
        for (int i=0; i<Objects2d::polarCount();i++)
        {
            pPolar = Objects2d::polarAt(i);
            serializePolarXFL(pPolar, ar, bIsStoring);
        }

        //the oppoints
        if(m_bSaveOpps)
        {
            ar << Objects2d::oppCount();
            for (int i=0; i<Objects2d::oppCount();i++)
            {
                pOpp = Objects2d::oppAt(i);
                pOpp->serializeOppXFL(ar, bIsStoring);
            }
        }
        else ar << 0;

        // and the spline foil whilst we're at it
        m_pAFoil->m_pSF->serializeXFL(ar, bIsStoring);

        ar << Units::pressureUnitIndex();
        ar << Units::inertiaUnitIndex();
        //add provisions
        // space allocation for the future storage of more data, without need to change the format
        for (int i=2; i<20; i++) ar << 0;
        dble=0;
        for (int i=0; i<50; i++) ar << dble;
    }
    else
    {
        // LOADING CODE
        int ArchiveFormat;
        ar >> ArchiveFormat;
        if(ArchiveFormat<200001 || ArchiveFormat>200002) return false;

        //Load unit data
        ar >> n; Units::setLengthUnitIndex(n);
        ar >> n; Units::setAreaUnitIndex(n);
        ar >> n; Units::setWeightUnitIndex(n);
        ar >> n; Units::setSpeedUnitIndex(n);
        ar >> n; Units::setForceUnitIndex(n);
        ar >> n; Units::setMomentUnitIndex(n);

        //pressure and inertia units are added later on in the provisions.

        Units::setUnitConversionFactors();

        if(ArchiveFormat==200001)
        {
            //Load the default Polar data. Not in the Settings, since this is Project dependant
            ar >> n;
            if     (n==1) WPolarDlg::s_WPolar.setPolarType(XFLR5::FIXEDSPEEDPOLAR);
            else if(n==2) WPolarDlg::s_WPolar.setPolarType(XFLR5::FIXEDLIFTPOLAR);
            else if(n==4) WPolarDlg::s_WPolar.setPolarType(XFLR5::FIXEDAOAPOLAR);
            else if(n==5) WPolarDlg::s_WPolar.setPolarType(XFLR5::BETAPOLAR);
            else if(n==7) WPolarDlg::s_WPolar.setPolarType(XFLR5::STABILITYPOLAR);

            ar >> n;
            if(n==1)      WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::LLTMETHOD);
            else if(n==2) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::VLMMETHOD);
            else if(n==3) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::PANEL4METHOD);
            else if(n==4) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::TRICSTMETHOD);
            else if(n==5) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::TRILINMETHOD);

            ar >> dble;    WPolarDlg::s_WPolar.setMass(dble);
            ar >> WPolarDlg::s_WPolar.m_QInfSpec;
            double x=0, y=0, z=0;
            ar >> x >> y >> z;
            WPolarDlg::s_WPolar.setCoG({x,y,z});

            ar >> f; WPolarDlg::s_WPolar.setDensity(double(f));
            ar >> f; WPolarDlg::s_WPolar.setViscosity(double(f));
            ar >> WPolarDlg::s_WPolar.m_AlphaSpec;
            ar >> WPolarDlg::s_WPolar.m_BetaSpec;

            ar >> WPolarDlg::s_WPolar.bTilted();
            ar >> WPolarDlg::s_WPolar.bWakeRollUp();
        }
        else if(ArchiveFormat==200002) WPolarDlg::s_WPolar.serializeWPlrXFL(ar, false);

        // load the planes...
        // assumes all object have been deleted and the array cleared.
        ar >> n;
        for(i=0; i<n; i++)
        {
            pPlane = new Plane();
            if(pPlane->serializePlaneXFL(ar, bIsStoring)) Objects3d::s_oaPlane.append(pPlane);
            else
            {
                delete pPlane;
                QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
                return false;
            }
        }

        // load the WPolars
        ar >> n;
        for(i=0; i<n; i++)
        {
            pWPolar = new WPolar();
            if(pWPolar->serializeWPlrXFL(ar, bIsStoring))
            {
                // clean up : the project may be carrying useless WPolars due to past programming errors
                pPlane = Objects3d::getPlane(pWPolar->planeName());
                if(pPlane)
                {
                    Objects3d::s_oaWPolar.append(pWPolar);
                    if(pWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)
                    {
                        pWPolar->setReferenceSpanLength(pPlane->planformSpan());
                        double area  = pPlane->planformArea();
                        if(pPlane->biPlane()) area += pPlane->wing2()->m_PlanformArea;
                        pWPolar->setReferenceArea(area);
                    }
                    else if(pWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM)
                    {
                        pWPolar->setReferenceSpanLength(pPlane->projectedSpan());
                        double area = pPlane->projectedArea();
                        if(pPlane->biPlane()) area += pPlane->wing2()->m_ProjectedArea;
                        pWPolar->setReferenceArea(area);
                    }
                    pWPolar->setReferenceChordLength(pPlane->mac());
                }
                else
                {
                }
            }
            else
            {
                delete pWPolar;
                QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
                return false;
            }
        }

        // the PlaneOpps
        ar >> n;
        for(i=0; i<n; i++)
        {
            pPOpp = new PlaneOpp();
            if(pPOpp->serializePOppXFL(ar, bIsStoring))
            {
                //just append, since POpps have been sorted when first inserted
                pPlane = Objects3d::getPlane(pPOpp->planeName());
                pWPolar = Objects3d::getWPolar(pPlane, pPOpp->polarName());

                // clean up : the project may be carrying useless PlaneOpps due to past programming errors
                if(pPlane && pWPolar) Objects3d::s_oaPOpp.append(pPOpp);
                else
                {
                }
            }
            else
            {
                delete pPOpp;
                QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
                return false;
            }
        }

        // load the Foils
        ar >> n;
        for(i=0; i<n; i++)
        {
            Foil *pFoil = new Foil();
            if(serializeFoilXFL(pFoil, ar, bIsStoring))
            {
                // delete any former foil with that name - necessary in the case of project insertion to avoid duplication
                // there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
                Foil *pOldFoil = Objects2d::foil(pFoil->foilName());
                if(pOldFoil) Objects2d::deleteFoil(pOldFoil);
                Objects2d::appendFoil(pFoil);
            }
            else
            {
                delete pFoil;
                QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
                return false;
            }
        }

        // load the Polars
        ar >> n;

        for(i=0; i<n; i++)
        {
            pPolar = new Polar();
            if(serializePolarXFL(pPolar, ar, bIsStoring)) Objects2d::appendPolar(pPolar);
            else
            {
                delete pPolar;
                QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
                return false;
            }
        }

        // OpPoints
        ar >> n;
        for(i=0; i<n; i++)
        {
            pOpp = new OpPoint();
            if(pOpp->serializeOppXFL(ar, bIsStoring))  Objects2d::appendOpp(pOpp);
            else
            {
                delete pOpp;
                QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
                return false;
            }
        }

        // and the spline foil whilst we're at it
        m_pAFoil->m_pSF->serializeXFL(ar, bIsStoring);

        ar >> n; Units::setPressureUnitIndex(n);
        ar >> n; Units::setInertiaUnitIndex(n);

        // space allocation
        int k=0;
        double dble=0;
        for (int i=2; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}


bool MainFrame::serializeProjectWPA(QDataStream &ar, bool bIsStoring)
{
    Wing *pWing     = nullptr;
    WPolar *pWPolar = nullptr;
    WingOpp *pWOpp  = nullptr;
    PlaneOpp *pPOpp = nullptr;
    Plane *pPlane   = nullptr;
    Body *pBody     = nullptr;
    Polar *pPolar   = nullptr;
    Foil *pFoil     = nullptr;

    QString str;
    int i=0, n=0, j=0, k=0;
    float f=0, g=0, h=0;

    if (bIsStoring)
    {
        //not storing to .wpa format anymore
        return true;
    }
    else
    {
        // LOADING CODE

        int ArchiveFormat;
        ar >> n;

        if(n<100000)
        {
            // then n is the number of wings to load
            // up to v1.99beta15
            ArchiveFormat = 100000;
        }
        else
        {
            // then n is the ArchiveFormat number
            ArchiveFormat = n;
            ar >> n; Units::setLengthUnitIndex(n);
            ar >> n; Units::setAreaUnitIndex(n);
            ar >> n; Units::setWeightUnitIndex(n);
            ar >> n; Units::setSpeedUnitIndex(n);
            ar >> n; Units::setForceUnitIndex(n);

            if(ArchiveFormat>=100005)
            {
                ar >> n; Units::setMomentUnitIndex(n);
            }

            Units::setUnitConversionFactors();

            if(ArchiveFormat>=100004)
            {
                ar >>k;
                if     (k==1) WPolarDlg::s_WPolar.setPolarType(XFLR5::FIXEDSPEEDPOLAR);
                else if(k==2) WPolarDlg::s_WPolar.setPolarType(XFLR5::FIXEDLIFTPOLAR);
                else if(k==4) WPolarDlg::s_WPolar.setPolarType(XFLR5::FIXEDAOAPOLAR);
                else if(k==5) WPolarDlg::s_WPolar.setPolarType(XFLR5::BETAPOLAR);
                else if(k==7) WPolarDlg::s_WPolar.setPolarType(XFLR5::STABILITYPOLAR);

                ar >> f; WPolarDlg::s_WPolar.setMass(double(f));
                ar >> f; WPolarDlg::s_WPolar.m_QInfSpec=double(f);
                if(ArchiveFormat>=100013)
                {
                    ar >> f >> g >> h;
                    WPolarDlg::s_WPolar.setCoG({double(f), double(g), double(h)});
                }
                else
                {
                    ar >> f; WPolarDlg::s_WPolar.setCoGx(double(f));
                    WPolarDlg::s_WPolar.setCoGy(0);
                    WPolarDlg::s_WPolar.setCoGz(0);
                }
                if(ArchiveFormat<100010) WPolarDlg::s_WPolar.setCoGx(double(f)/1000.0);
                ar >> f; WPolarDlg::s_WPolar.setDensity(double(f));
                ar >> f; WPolarDlg::s_WPolar.setViscosity(double(f));
                ar >> f; WPolarDlg::s_WPolar.m_AlphaSpec     = double(f);
                if(ArchiveFormat>=100012)
                {
                    ar >>f; WPolarDlg::s_WPolar.m_BetaSpec=double(f);
                }

                ar >> k;
                if     (k==1) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::LLTMETHOD);
                else if(k==2) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::VLMMETHOD);
                else if(k==3) WPolarDlg::s_WPolar.setAnalysisMethod(XFLR5::PANEL4METHOD);
            }
            if(ArchiveFormat>=100006)
            {
                ar >> k;
                if (k) WPolarDlg::s_WPolar.bVLM1() = true;
                else   WPolarDlg::s_WPolar.bVLM1() = false;

                ar >> k;
            }

            if(ArchiveFormat>=100008)
            {
                ar >> k;
                if (k) WPolarDlg::s_WPolar.bTilted() = true;
                else   WPolarDlg::s_WPolar.bTilted() = false;

                ar >> k;
                if (k) WPolarDlg::s_WPolar.bWakeRollUp() = true;
                else   WPolarDlg::s_WPolar.bWakeRollUp() = false;
            }
            // and read n again
            ar >> n;
        }

        // WINGS FIRST
        for (i=0;i<n; i++)
        {
            pWing = new Wing;

            if (!pWing->serializeWingWPA(ar, bIsStoring))
            {
                if(pWing) delete pWing;
                return false;
            }
            if(pWing)
            {
                //create a plane with this wing
                pPlane = new Plane();
                pPlane->setPlaneName(pWing->wingName());
                pPlane->m_Wing[0].duplicate(pWing);
                pPlane->setBody(nullptr);
                pPlane->setWings(false, false, false);
                Objects3d::addPlane(pPlane);
                delete pWing;
            }
        }

        //THEN WPOLARS

        ar >> n;// number of WPolars to load
        bool bWPolarOK;

        for (i=0; i<n; i++)
        {
            pWPolar = new WPolar;
            bWPolarOK = pWPolar->serializeWPlrWPA(ar, bIsStoring);
            //force compatibilty
            if(pWPolar->analysisMethod()==XFLR5::PANEL4METHOD && pWPolar->polarType()==XFLR5::STABILITYPOLAR) pWPolar->bThinSurfaces() = true;

            if (!bWPolarOK)
            {
                if(pWPolar) delete pWPolar;
                return false;
            }
            if(pWPolar->analysisMethod()!=XFLR5::LLTMETHOD && ArchiveFormat <100003)    pWPolar->clearData();//former VLM version was flawed
            //            if(pWPolar->polarType()==STABILITYPOLAR)    pWPolar->bThinSurfaces() = true;

            if(pWPolar->polarFormat()!=1020 || pWPolar->polarType()!=XFLR5::STABILITYPOLAR)
                Objects3d::addWPolar(pWPolar);
        }

        //THEN WOPPS
        ar >> n;// number of WOpps to load
        bool bWOppOK;
        for (i=0;i<n; i++)
        {
            pWOpp = new WingOpp();
            if (ArchiveFormat<=100001)
            {
                if(pWOpp) delete pWOpp;
                return false;
            }
            else
            {
                bWOppOK = pWOpp->serializeWingOppWPA(ar, bIsStoring);
                if (!bWOppOK)
                {
                    if(pWOpp) delete pWOpp;
                    return false;
                }
            }

            delete pWOpp;
        }

        //        ar >> n;
        //=100000 ... unused
        //THEN FOILS, POLARS and OPPS
        if(ArchiveFormat>=100009)
        {
            if(!loadPolarFileV3(ar, bIsStoring,100002))
            {
                return false;
            }
        }
        else
        {
            if(ArchiveFormat>=100006)
            {
                if(!loadPolarFileV3(ar, bIsStoring))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        if(n==100000)
        {
            for (j=0; j<Objects2d::polarCount(); j++)
            {
                pPolar = Objects2d::polarAt(j);
                for (k=0; k<Objects2d::foilCount(); k++)
                {
                    pFoil = Objects2d::foilAt(k);
                    if(pFoil->foilName()==pPolar->foilName())
                    {
                        pPolar->setNCrit(9.0);
                        pPolar->setXtrTop(1.0);
                        pPolar->setXtrBot(1.0);
                        str = QString("_N%1").arg(9.0,4,'f',1);
                        QString name = pPolar->polarName();
                        pPolar->setPolarName(name + str);
                        break;
                    }
                }
            }
        }

        if(ArchiveFormat>=100011)
        {
            ar >> n;// number of Bodies to load
            for (i=0;i<n; i++)
            {
                pBody = new Body();

                if (pBody->serializeBodyWPA(ar, bIsStoring))
                {
                    Objects3d::s_oaBody.append(pBody);

                }
                else
                {
                    if(pBody) delete pBody;
                    return false;
                }
            }
        }
        if(ArchiveFormat>=100006)
        { //read the planes
            ar >> n;
            for (i=0; i<n;i++)
            {
                pPlane = new Plane();
                if(pPlane)
                {
                    if(pPlane->serializePlaneWPA(ar, bIsStoring))
                    {
                        Objects3d::addPlane(pPlane);
                        if(pPlane->bodyName().length()) pPlane->body()->duplicate(Objects3d::getBody(pPlane->bodyName()));
                    }
                    else
                    {
                        if(pPlane) delete pPlane;
                        return false;
                    }
                }
            }

            // attach the body pointers to the Plane objects
            for (int ib=0; ib<Objects3d::s_oaBody.size(); ib++)
            {
                Body *pBody = Objects3d::s_oaBody.at(ib);
                Objects3d::addBody(pBody);
            }


            //and their pPolars
            if(ArchiveFormat <100007)
            {
                ar >> n;// number of WPolars to load
                for (i=0;i<n; i++)
                {
                    pWPolar = new WPolar();

                    if (!pWPolar->serializeWPlrWPA(ar, bIsStoring))
                    {
                        if(pWPolar) delete pWPolar;
                        return false;
                    }
                    if(pWPolar->analysisMethod()!=XFLR5::LLTMETHOD && ArchiveFormat <100003)
                        pWPolar->clearData();
                    Objects3d::addWPolar(pWPolar);
                }
            }

            ar >> n;// number of PlaneOpps to load
            for (i=0;i<n; i++)
            {
                pPOpp = new PlaneOpp();

                if (!pPOpp->serializePOppWPA(ar, bIsStoring))
                {
                    if(pPOpp) delete pPOpp;
                    return false;
                }
                Objects3d::insertPOpp(pPOpp);
                //                Objects3D::s_oaPOpp.append(pPOpp);
            }
        }
        m_pMiarex->m_pCurPOpp = nullptr;

        m_pAFoil->m_pSF->serialize(ar, bIsStoring);

        if(m_iApp==XFLR5::MIAREX)
        {
            m_pMiarex->setPlane();
            m_pMiarex->setScale();
        }

        m_pMiarex->updateUnits();

        return true;
    }
}


void MainFrame::setMenus()
{
    if(m_iApp==XFLR5::NOAPP)
    {
        menuBar()->clear();
        menuBar()->addMenu(m_pFileMenu);
        menuBar()->addMenu(m_pOptionsMenu);
        menuBar()->addMenu(m_pHelpMenu);
    }
    else if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        menuBar()->clear();
        menuBar()->addMenu(m_pFileMenu);
        menuBar()->addMenu(m_pXDirectViewMenu);
        menuBar()->addMenu(m_pXDirectFoilMenu);
        menuBar()->addMenu(m_pDesignMenu);
        menuBar()->addMenu(m_pXFoilAnalysisMenu);
        menuBar()->addMenu(m_pPolarMenu);
        menuBar()->addMenu(m_pOpPointMenu);
        menuBar()->addMenu(m_pGraphMenu);
        menuBar()->addMenu(m_pOptionsMenu);
        menuBar()->addMenu(m_pHelpMenu);
    }
    else if(m_iApp==XFLR5::INVERSEDESIGN)
    {
        menuBar()->clear();
        menuBar()->addMenu(m_pFileMenu);
        menuBar()->addMenu(m_pXInverseViewMenu);
        menuBar()->addMenu(m_pXInverseGraphMenu);
        menuBar()->addMenu(m_pXInverseFoilMenu);
        menuBar()->addMenu(m_pOptionsMenu);
        menuBar()->addMenu(m_pHelpMenu);
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        menuBar()->clear();
        menuBar()->addMenu(m_pFileMenu);
        menuBar()->addMenu(m_pAFoilViewMenu);
        menuBar()->addMenu(m_pAFoilDesignMenu);
        menuBar()->addMenu(m_pAFoilSplineMenu);
        menuBar()->addMenu(m_pOptionsMenu);
        menuBar()->addMenu(m_pHelpMenu);
    }
    else if(m_iApp== XFLR5::MIAREX)
    {
        menuBar()->clear();
        menuBar()->addMenu(m_pFileMenu);
        menuBar()->addMenu(m_pMiarexViewMenu);
        menuBar()->addMenu(m_pPlaneMenu);
        menuBar()->addMenu(m_pMiarexWPlrMenu);
        menuBar()->addMenu(m_pMiarexWOppMenu);
        menuBar()->addMenu(m_pMiarexAnalysisMenu);
        menuBar()->addMenu(m_pGraphMenu);
        menuBar()->addMenu(m_pOptionsMenu);
        menuBar()->addMenu(m_pHelpMenu);
    }
}


void MainFrame::setProjectName(QString PathName)
{
    m_FileName = PathName;
    int pos = PathName.lastIndexOf("/");
    if (pos>0) s_ProjectName = PathName.right(PathName.length()-pos-1);
    else       s_ProjectName = PathName;

    if(s_ProjectName.length()>4)
    {
        s_ProjectName = s_ProjectName.left(s_ProjectName.length()-4);
        m_pctrlProjectName->setText(s_ProjectName);
    }
}


void MainFrame::setSaveState(bool bSave)
{
    s_bSaved = bSave;
    if(bSave)
        m_pctrlProjectName->setText(s_ProjectName);
    else
        m_pctrlProjectName->setText(s_ProjectName+ "*");
}



void MainFrame::setGraphSettings(Graph *pGraph)
{
    m_pXDirect->m_CpGraph.copySettings(pGraph, false);
    for(int ig=0; ig<qMax(MAXPOLARGRAPHS, m_pXDirect->m_PlrGraph.count()); ig++) m_pXDirect->m_PlrGraph[ig]->copySettings(pGraph, false);

    m_pXInverse->m_QGraph.copySettings(pGraph, false);

    m_pMiarex->m_CpGraph.copySettings(pGraph, false);

    for(int ig=0; ig<m_pMiarex->m_WingGraph.count(); ig++) m_pMiarex->m_WingGraph[ig]->copySettings(pGraph, false);
    for(int ig=0; ig<m_pMiarex->m_WPlrGraph.count(); ig++) m_pMiarex->m_WPlrGraph[ig]->copySettings(pGraph, false);
    for(int ig=0; ig<m_pMiarex->m_TimeGraph.count(); ig++) m_pMiarex->m_TimeGraph[ig]->copySettings(pGraph, false);
    for(int ig=0; ig<m_pMiarex->m_StabPlrGraph.count(); ig++) m_pMiarex->m_StabPlrGraph[ig]->copySettings(pGraph, false);
}



QString MainFrame::shortenFileName(QString &PathName)
{
    QString strong, strange;
    if(PathName.length()>60)
    {
        int pos = PathName.lastIndexOf('/');
        if (pos>0)
        {
            strong = '/'+PathName.right(PathName.length()-pos-1);
            strange = PathName.left(60-strong.length()-6);
            pos = strange.lastIndexOf('/');
            if(pos>0) strange = strange.left(pos)+"/...  ";
            strong = strange+strong;
        }
        else
        {
            strong = PathName.left(40);
        }
    }
    else strong = PathName;
    return strong;
}

/**
* Fills the combobox with  plane names.
* then selects the current wing or plane, if any;
* else selects the first, if any;
* else disables the combobox.
*/
void MainFrame::updatePlaneListBox()
{
    int i=0;
    Plane *pPlane=nullptr, *pCurPlane=nullptr;

    m_pctrlPlane->blockSignals(true);
    m_pctrlPlane->clear();

    pCurPlane = m_pMiarex->m_pCurPlane;
    QStringList PlaneNames;
    for (i=0; i<Objects3d::s_oaPlane.size(); i++)
    {
        pPlane = Objects3d::s_oaPlane[i];
        PlaneNames.append(pPlane->planeName());
    }

#if QT_VERSION >= 0x050000
    PlaneNames.sort(Qt::CaseInsensitive);
#else
    PlaneNames.sort();
#endif

    m_pctrlPlane->addItems(PlaneNames);

    //select the current Plane, if any...
    if(pCurPlane) selectPlane(pCurPlane);

    m_pctrlPlane->setEnabled(m_pctrlPlane->count());
    m_pctrlPlane->blockSignals(false);

    updateWPolarListBox();
}



void MainFrame::updateWPolarListBox()
{
    //    fills the combobox with WPolar names associated to Miarex's current wing
    //    then selects Miarex current WPolar if any, else selects the first, if any
    //    else disables the combobox
    WPolar *pWPolar=nullptr;
    QString PlaneName;
    int i=0;

    m_pctrlPlanePolar->blockSignals(true);
    m_pctrlPlanePolar->clear();

    Plane  *pCurPlane = m_pMiarex->m_pCurPlane;
    WPolar *pCurWPlr  = m_pMiarex->m_pCurWPolar;

    if(pCurPlane)      PlaneName = pCurPlane->planeName();
    else               PlaneName = "";

    if(!PlaneName.length())
    {
        m_pMiarex->m_pCurWPolar = nullptr;
        m_pctrlPlanePolar->setEnabled(false);
        m_pctrlPlaneOpp->setEnabled(false);
        return;
    }

    for (i=0; i<Objects3d::s_oaWPolar.size(); i++)
    {
        pWPolar = Objects3d::s_oaWPolar[i];
        if(pWPolar->planeName() == PlaneName)
        {
            m_pctrlPlanePolar->addItem(pWPolar->polarName());
        }
    }

    m_pctrlPlanePolar->setEnabled(m_pctrlPlanePolar->count());

    if(pCurWPlr) selectWPolar(pCurWPlr);

    m_pctrlPlanePolar->blockSignals(false);
    updatePOppListBox();
}


/**
 * Fills the combobox with the WOpp parameters associated to Miarex' current WPLr,
 * then selects the current WingOpp or PlaneOpp if any, else selects the first, if any,
 * else disables the combobox.
 */
void MainFrame::updatePOppListBox()
{
    m_pctrlPlaneOpp->blockSignals(true);

    Plane  *pCurPlane   = m_pMiarex->m_pCurPlane;
    WPolar *pCurWPlr    = m_pMiarex->m_pCurWPolar;

    QString str;
    m_pctrlPlaneOpp->clear();


    if (!pCurPlane || !pCurPlane->planeName().length() || !pCurWPlr || !pCurWPlr->polarName().length())
    {
        m_pctrlPlaneOpp->setEnabled(false);
        return;
    }

    for (int iPOpp=0; iPOpp<Objects3d::s_oaPOpp.size(); iPOpp++)
    {
        PlaneOpp *pPOpp = Objects3d::s_oaPOpp.at(iPOpp);
        if (pPOpp->planeName()==pCurPlane->planeName() && pPOpp->polarName()==pCurWPlr->polarName())
        {
            if(pCurWPlr->polarType()<XFLR5::FIXEDAOAPOLAR)        str = QString("%L1").arg(pPOpp->m_Alpha,8,'f',3);
            else if(pCurWPlr->polarType()==XFLR5::FIXEDAOAPOLAR)  str = QString("%L1").arg(pPOpp->m_QInf,8,'f',3);
            else if(pCurWPlr->polarType()==XFLR5::BETAPOLAR)      str = QString("%L1").arg(pPOpp->m_Beta,8,'f',3);
            else if(pCurWPlr->polarType()==XFLR5::STABILITYPOLAR) str = QString("%L1").arg(pPOpp->m_Ctrl,8,'f',3);
            m_pctrlPlaneOpp->addItem(str);
        }
    }

    if(m_pMiarex->m_pCurPOpp) selectPlaneOpp(m_pMiarex->m_pCurPOpp);
    else                    m_pctrlPlaneOpp->setCurrentIndex(0);

    // otherwise disable control
    m_pctrlPlaneOpp->setEnabled(m_pctrlPlaneOpp->count());

    m_pctrlPlaneOpp->blockSignals(false);
}


/**
 * Fills the combobox with the Foil names,
 * then selects the current Foil if any, else selects the first, if any,
 * else disables the combobox
 */
void MainFrame::updateFoilListBox()
{
    m_pctrlFoil->blockSignals(true);
    m_pctrlFoil->clear();

    QStringList foilList;
    for (int iFoil=0; iFoil<Objects2d::foilCount(); iFoil++)
    {
        Foil *pFoil = Objects2d::foilAt(iFoil);
        foilList.append(pFoil->foilName());
    }

#if QT_VERSION >= 0x050000
    foilList.sort(Qt::CaseInsensitive);
#else
    foilList.sort();
#endif

    m_pctrlFoil->addItems(foilList);
    m_pctrlFoil->setEnabled(m_pctrlFoil->count());
    selectFoil(XDirect::curFoil());

    m_pctrlFoil->blockSignals(false);

    updatePolarListBox();
}


/**
 * Fills the combobox with polar names associated to the current foil,
 * then selects XDirect current polar if any, else selects the first, if any,
 * else disables the combobox,
 */
void MainFrame::updatePolarListBox()
{
    m_pctrlPolar->blockSignals(true);
    m_pctrlPolar->clear();

    if(!XDirect::curFoil())
    {
    }
    else
    {
        QStringList polarList;
        for (int i=0; i<Objects2d::polarCount(); i++)
        {
            Polar *pPolar = Objects2d::polarAt(i);
            if(pPolar->foilName() == XDirect::curFoil()->foilName())
            {
                polarList.append(pPolar->polarName());
            }
        }

#if QT_VERSION >= 0x050000
        polarList.sort(Qt::CaseInsensitive);
#else
        polarList.sort();
#endif

        m_pctrlPolar->addItems(polarList);
    }

    m_pctrlPolar->setEnabled(m_pctrlPolar->count());
    selectPolar(m_pXDirect->curPolar());

    m_pctrlPolar->blockSignals(false);

    updateOppListBox();
}


/**
 * Fills the combobox with the OpPoint values associated to the current foil,
 * then selects the current OpPoint if any, else selects the first, if any,
 * else disables the combobox.
 */
void MainFrame::updateOppListBox()
{
    m_pctrlOpPoint->blockSignals(true);
    m_pctrlOpPoint->clear();

    Polar *pCurPlr    = m_pXDirect->curPolar();

    if (!XDirect::curFoil()  || !pCurPlr)
    {
        m_pctrlOpPoint->clear();
        m_pctrlOpPoint->setEnabled(false);
        m_pctrlOpPoint->blockSignals(false);
    }
    else
    {
        QStringList oppList;
        QString str;
        for (int iOpp=0; iOpp<Objects2d::oppCount(); iOpp++)
        {
            OpPoint *pOpp = Objects2d::oppAt(iOpp);
            if (pOpp->foilName()==XDirect::curFoil()->foilName() && pOpp->polarName()==pCurPlr->polarName())
            {
                if (pCurPlr->polarType() != XFLR5::FIXEDAOAPOLAR)
                    str = QString("%L1").arg(pOpp->aoa(),8,'f',3);
                else
                    str = QString("%L1").arg(pOpp->Reynolds(),8,'f',0);
                oppList.append(str);
            }
        }
        /*
#if QT_VERSION >= 0x050000
        oppList.sort(Qt::CaseInsensitive);
#else
        oppList.sort();
#endif
*/
        m_pctrlOpPoint->addItems(oppList);
    }

    m_pctrlOpPoint->setEnabled(m_pctrlOpPoint->count());
    selectOpPoint(XDirect::curOpp());

    m_pctrlOpPoint->blockSignals(false);
}


void MainFrame::updateRecentFileActions()
{
    int numRecentFiles = qMin(m_RecentFiles.size(), MAXRECENTFILES);

    QString text;
    for (int i=0; i<numRecentFiles; ++i)
    {
        text = tr("&%1 %2").arg(i + 1).arg(shortenFileName(m_RecentFiles[i]));
        if(i==0) text +="\tCtrl+7";

        m_pRecentFileActs[i]->setText(text);
        m_pRecentFileActs[i]->setData(m_RecentFiles[i]);
        m_pRecentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)
        m_pRecentFileActs[j]->setVisible(false);

    m_pSeparatorAct->setVisible(numRecentFiles > 0);
}


void MainFrame::updateView()
{
    switch(m_iApp)
    {
        case XFLR5::XFOILANALYSIS:
        {
            m_pXDirect->updateView();
            break;
        }
        case XFLR5::DIRECTDESIGN:
        {
            m_pDirect2dWidget->update();
            break;
        }
        case XFLR5::MIAREX:
        {
            m_pMiarex->updateView();
            break;
        }
        case XFLR5::INVERSEDESIGN:
        {
            m_pXInverse->updateView();
            break;
        }
        default:
            break;
    }
}


void MainFrame::saveFoilPolars(QDataStream &ar, QVector<Foil*> const &FoilList)
{
    QVector<Foil*> ExportList;
    if(FoilList.isEmpty())
    {
        // Save them all, the user will sort them out
        for(int iFoil=0; iFoil<Objects2d::foilCount(); iFoil++)
        {
            ExportList.append(Objects2d::foilAt(iFoil));
        }
    }
    else
    {
        for(int iFoil=0; iFoil<FoilList.size(); iFoil++)
        {
            ExportList.append(FoilList.at(iFoil));
        }
    }

    // count the number of polars to export
    int nPolars = 0;
    for (int i=0; i<ExportList.size(); i++)
    {
        Foil const *pFoil = ExportList.at(i);
        for (int j=0; j<Objects2d::polarCount(); j++)
        {
            Polar *pPolar = Objects2d::polarAt(j);
            if(pPolar->foilName().compare(pFoil->foilName())==0) nPolars++;
        }
    }

    int ArchiveFormat = 100003;
    ar << ArchiveFormat;
    //100003 : added foil comment
    //100002 : means we are serializings opps in the new numbered format
    //100001 : transferred NCrit, XTopTr, XBotTr to polar file
    //first write foils
    ar << ExportList.size();

    for (int i=0; i<ExportList.size(); i++)
    {
        Foil *pFoil = ExportList.at(i);
        serializeFoil(pFoil, ar, true);
    }

    //then write the polars associated to the selected foils
    ar << nPolars;

    for (int i=0; i<ExportList.size(); i++)
    {
        Foil const *pFoil = ExportList.at(i);
        for (int j=0; j<Objects2d::polarCount(); j++)
        {
            Polar *pPolar = Objects2d::polarAt(j);
            if(pPolar->foilName().compare(pFoil->foilName())==0)
                serializePolar(pPolar, ar, true);
        }
    }

    ar << 0; // don't save opps
}



void MainFrame::setupDataDir()
{
#ifdef Q_OS_MAC
    s_TranslationDir.setPath(qApp->applicationDirPath()+"/translations/");
    s_StylesheetDir.setPath(qApp->applicationDirPath()+"/qss/");
#endif
#ifdef Q_OS_WIN
    s_TranslationDir.setPath(qApp->applicationDirPath()+"/translations/xfl");
    s_StylesheetDir.setPath(qApp->applicationDirPath()+"/qss");
#endif
#ifdef Q_OS_LINUX
    s_TranslationDir.setPath("/usr/local/share/xflr5/translations");
    s_StylesheetDir.setPath("/usr/local/share/xflr5/qss");
#endif

/*    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    qDebug()<<QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);*/
}


void MainFrame::readStyleSheet(QString styleSheetName, QString &styleSheet)
{
    QFile file(s_StylesheetDir.canonicalPath()+"/"+styleSheetName+".qss");
    if(file.open(QFile::ReadOnly))
    {
        styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    }
}



void MainFrame::onProjectModified()
{
    setSaveState(false);
}



/**
 * The user has requested the launch of the interface to manage Foil objects.
 */
void MainFrame::onManageFoils()
{
    ManageFoilsDlg mfDlg(this);

    QString FoilName = "";
    if(XDirect::curFoil()) FoilName = XDirect::curFoil()->foilName();
    mfDlg.initDialog(FoilName);
    mfDlg.exec();

    // set null ptrs in case the current objects have been deleted
    XDirect::setCurFoil(nullptr);
    XDirect::setCurPolar(nullptr);
    XDirect::setCurOpp(nullptr);

    if(mfDlg.m_bChanged) setSaveState(false);

    if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        m_pXDirect->setFoil(mfDlg.m_pFoil);
        updateFoilListBox();
        m_pXDirect->setControls();
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        m_pAFoil->fillFoilTable();
        m_pAFoil->selectFoil();
    }

    updateView();
}



void MainFrame::checkGraphActions()
{
    for(int ig=0; ig<MAXGRAPHS; ig++)    m_pSingleGraph[ig]->setChecked(false);
    m_pTwoGraphs->setChecked(false);
    m_pFourGraphs->setChecked(false);
    m_pAllGraphs->setChecked(false);
    m_pSingleGraph[0]->setEnabled(true);

    if(m_iApp==XFLR5::MIAREX)
    {
        switch(m_pMiarex->m_iView)
        {
            case XFLR5::WOPPVIEW:
            {
                if(m_pMiarex->m_iWingView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
                else if(m_pMiarex->m_iWingView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
                else if(m_pMiarex->m_iWingView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
                else if(m_pMiarex->m_iWingView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
                m_pSingleGraph[1]->setEnabled(true);
                m_pSingleGraph[2]->setEnabled(true);
                m_pSingleGraph[3]->setEnabled(true);
                m_pSingleGraph[4]->setEnabled(true);
                m_pSingleGraph[5]->setEnabled(true);
                m_pTwoGraphs->setEnabled(true);
                m_pFourGraphs->setEnabled(true);
                m_pAllGraphs->setEnabled(true);
                break;
            }
            case XFLR5::WPOLARVIEW:
            {
                if(m_pMiarex->m_iWPlrView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
                else if(m_pMiarex->m_iWPlrView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
                else if(m_pMiarex->m_iWPlrView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
                else if(m_pMiarex->m_iWPlrView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
                m_pSingleGraph[1]->setEnabled(true);
                m_pSingleGraph[2]->setEnabled(true);
                m_pSingleGraph[3]->setEnabled(true);
                m_pSingleGraph[4]->setEnabled(true);
                m_pSingleGraph[5]->setEnabled(true);
                m_pTwoGraphs->setEnabled(true);
                m_pFourGraphs->setEnabled(true);
                m_pAllGraphs->setEnabled(true);
                break;
            }
            case XFLR5::STABPOLARVIEW:
            {
                if(m_pMiarex->m_bLongitudinal== XFLR5::ONEGRAPH)      m_pSingleGraph[0]->setChecked(true);
                else if(m_pMiarex->m_bLongitudinal== XFLR5::ONEGRAPH) m_pSingleGraph[1]->setChecked(true);
                m_pSingleGraph[2]->setEnabled(false);
                m_pSingleGraph[3]->setEnabled(false);
                m_pSingleGraph[4]->setEnabled(false);
                m_pSingleGraph[5]->setEnabled(false);
                m_pTwoGraphs->setEnabled(true);
                m_pFourGraphs->setEnabled(false);
                m_pAllGraphs->setEnabled(false);
                break;
            }
            case XFLR5::STABTIMEVIEW:
            {
                if(m_pMiarex->m_iStabTimeView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
                else if(m_pMiarex->m_iStabTimeView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
                else if(m_pMiarex->m_iStabTimeView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
                else if(m_pMiarex->m_iStabTimeView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
                m_pSingleGraph[1]->setEnabled(true);
                m_pSingleGraph[2]->setEnabled(true);
                m_pSingleGraph[3]->setEnabled(true);
                m_pSingleGraph[4]->setEnabled(false);
                m_pSingleGraph[5]->setEnabled(false);
                m_pTwoGraphs->setEnabled(true);
                m_pFourGraphs->setEnabled(true);
                m_pAllGraphs->setEnabled(false);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else if (m_iApp==XFLR5::XFOILANALYSIS)
    {
        if(!m_pXDirect->m_bPolarView)
        {
            m_pSingleGraph[0]->setChecked(true);

            m_pSingleGraph[1]->setEnabled(false);
            m_pSingleGraph[2]->setEnabled(false);
            m_pSingleGraph[3]->setEnabled(false);
            m_pSingleGraph[4]->setEnabled(false);
            m_pSingleGraph[5]->setEnabled(false);
            m_pTwoGraphs->setEnabled(false);
            m_pFourGraphs->setEnabled(false);
            m_pAllGraphs->setEnabled(false);
        }
        else
        {
            if(m_pXDirect->m_iPlrView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pXDirectTileWidget->activeGraphIndex()]->setChecked(true);
            else if(m_pXDirect->m_iPlrView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
            else if(m_pXDirect->m_iPlrView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
            else if(m_pXDirect->m_iPlrView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);

            m_pSingleGraph[1]->setEnabled(true);
            m_pSingleGraph[2]->setEnabled(true);
            m_pSingleGraph[3]->setEnabled(true);
            m_pSingleGraph[4]->setEnabled(true);
            m_pSingleGraph[5]->setEnabled(false);
            m_pTwoGraphs->setEnabled(true);
            m_pFourGraphs->setEnabled(true);
            m_pAllGraphs->setEnabled(true);
        }
    }
    else if (m_iApp==XFLR5::INVERSEDESIGN)
    {
        m_pSingleGraph[0]->setChecked(true);
    }
}


void MainFrame::onResetCurGraphScales()
{
    switch(m_iApp)
    {
        case XFLR5::MIAREX:
        {
            m_pMiarexTileWidget->onResetCurGraphScales();
            break;
        }
        case XFLR5::XFOILANALYSIS:
        {
            m_pXDirectTileWidget->onResetCurGraphScales();
            break;
        }
        case XFLR5::INVERSEDESIGN:
        {
            m_pXInverse->m_QGraph.setAuto(true);
            m_pXInverse->releaseZoom();
            m_pXInverse->updateView();
            return;
        }
        default:
            return;
    }

}


void MainFrame::onExportCurGraph()
{
    switch(m_iApp)
    {
        case XFLR5::MIAREX:
        {
            m_pMiarexTileWidget->onExportCurGraph();
            break;
        }
        case XFLR5::XFOILANALYSIS:
        {
            if(m_pXDirect->bPolarView()) m_pXDirectTileWidget->onExportCurGraph();
            else exportGraph(&m_pXDirect->m_CpGraph);
            break;
        }
        case XFLR5::INVERSEDESIGN:
        {
            exportGraph(&m_pXInverse->m_QGraph);
            break;
        }
        default:
            break;
    }
}





void MainFrame::onCurGraphSettings()
{
    switch(m_iApp)
    {
        case XFLR5::MIAREX:
        {
            m_pMiarexTileWidget->onCurGraphSettings();
            break;
        }
        case XFLR5::XFOILANALYSIS:
        {
            m_pXDirectTileWidget->onCurGraphSettings();
            break;
        }
        case XFLR5::INVERSEDESIGN:
        {
            m_pXInverse->onQGraphSettings();
            break;
        }
        default:
            break;
    }
}



void MainFrame::onLoadLastProject()
{
    if(!m_RecentFiles.size()) return;

    XFLR5::enumApp iApp = loadXFLR5File(m_RecentFiles.at(0));
    if(m_iApp==XFLR5::NOAPP) m_iApp = iApp;
    if(m_iApp==XFLR5::XFOILANALYSIS)
    {
        onXDirect();
    }
    else if(m_iApp==XFLR5::MIAREX)
    {
        onMiarex();
        m_pMiarex->setScale();
    }
    else if(m_iApp==XFLR5::DIRECTDESIGN)
    {
        onAFoil();
    }
    else if(m_iApp==XFLR5::INVERSEDESIGN)
    {
        if(m_pXInverse->m_bFullInverse) onXInverse();
        else                            onXInverseMixed();
    }
}


void MainFrame::onExecuteScript()
{
    XflScriptExec scriptExecutor;
    scriptExecutor.readScript();
    scriptExecutor.makeFoils();
    scriptExecutor.loadFoilPolarFiles();
    scriptExecutor.makePlanes();
    scriptExecutor.makePlaneAnalysisList();

    //    scriptExecutor.runScript();
}

void MainFrame::onExecutePythonScript()
{
    // PythonScriptExecutor scriptExecutor;
    // scriptExecutor.loadScript();
    // scriptExecutor.makeFoils();
    // scriptExecutor.loadFoilPolarFiles();
    // scriptExecutor.makePlanes();
    // scriptExecutor.makePlaneAnalysisList();

    //    scriptExecutor.runScript();
    QString PathName;

    PathName = QFileDialog::getOpenFileName(this, tr("Open Python File"),
                                              Settings::lastDirName(),
                                              "Python file (*.py *.txt )");
    if(!PathName.length()) return;
    if(PathName.endsWith(".py")) {
        std::cout<<"check";
        mainModule.evalFile(PathName);
    }
    m_pConsole->appendCommandPrompt();    

}

void MainFrame::showEvent(QShowEvent *pEvent)
{
    // make sure the graph and foil scales have been initialized when we first display
    // the foil operating point view
    m_pXDirect->m_CpGraph.initializeGraph(m_pctrlCentralWidget->width(), m_pctrlCentralWidget->height());
    pEvent->ignore();
}



/**
 * Loads or Saves the data of this foil to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool MainFrame::serializeFoilXFL(Foil *pFoil, QDataStream &ar, bool bIsStoring)
{
    qint8 b = 0x00;
    QString strange;
    int ArchiveFormat = 100006;
    // 100006 : first version of new xfl format

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << pFoil->m_FoilName;
        ar << pFoil->m_FoilDescription;
        ar << pFoil->m_FoilStyle << pFoil->m_FoilWidth;
        writeColor(ar, pFoil->m_red, pFoil->m_green, pFoil->m_blue, pFoil->m_alphaChannel);

        ar << pFoil->m_bIsFoilVisible;
        //        ar << m_bShowFoilPoints;

        ar << qint8(pFoil->m_PointStyle);
        ar << pFoil->m_bCenterLine << pFoil->m_bLEFlap << pFoil->m_bTEFlap;
        ar << pFoil->m_LEFlapAngle << pFoil->m_LEXHinge << pFoil->m_LEYHinge;
        ar << pFoil->m_TEFlapAngle << pFoil->m_TEXHinge << pFoil->m_TEYHinge;
        ar << pFoil->nb;
        for (int j=0; j<pFoil->nb; j++)
        {
            ar << pFoil->xb[j] << pFoil->yb[j];
        }
        return true;
    }
    else
    {
        ar >> ArchiveFormat;

        ar >> strange;
        pFoil->setFoilName(strange);

        ar >> strange;
        pFoil->setFoilDescription(strange);

        ar >> pFoil->m_FoilStyle >> pFoil->m_FoilWidth;

        readColor(ar, pFoil->m_red, pFoil->m_green, pFoil->m_blue, pFoil->m_alphaChannel);

        ar >> pFoil->m_bIsFoilVisible;
        //        ar >> m_bShowFoilPoints;
        ar >> b; pFoil->m_PointStyle = int(b);
        ar >> pFoil->m_bCenterLine >> pFoil->m_bLEFlap >> pFoil->m_bTEFlap;
        ar >> pFoil->m_LEFlapAngle >> pFoil->m_LEXHinge >> pFoil->m_LEYHinge;
        ar >> pFoil->m_TEFlapAngle >> pFoil->m_TEXHinge >> pFoil->m_TEYHinge;
        ar >> pFoil->nb;

        for (int j=0; j<pFoil->nb; j++)
        {
            ar >> pFoil->xb[j] >> pFoil->yb[j];
        }

        memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
        memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
        pFoil->n = pFoil->nb;

        pFoil->initFoil();
        pFoil->setFlap();

        return true;
    }
}

/**
 * Loads or saves the data of this polar to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool MainFrame::serializePolarXFL(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    double dble=0;
    bool boolean=false;
    int i=0, k=0, n=0;
    int ArchiveFormat=0;// identifies the format of the file

    if(bIsStoring)
    {
        ar << 100004; // first format for XFL file

        ar << pPolar->m_FoilName;
        ar << pPolar->m_PlrName;

        ar << pPolar->m_Style << pPolar->m_Width;
        writeColor(ar, pPolar->m_red, pPolar->m_green, pPolar->m_blue, pPolar->m_alphaChannel);
        ar << pPolar->m_bIsVisible << false;


        if      (pPolar->m_PolarType==XFLR5::FIXEDSPEEDPOLAR) ar<<1;
        else if(pPolar->m_PolarType==XFLR5::FIXEDLIFTPOLAR)   ar<<2;
        else if(pPolar->m_PolarType==XFLR5::RUBBERCHORDPOLAR) ar<<3;
        else if(pPolar->m_PolarType==XFLR5::FIXEDAOAPOLAR)    ar<<4;
        else                                                  ar<<1;

        ar << pPolar->m_MaType << pPolar->m_ReType;
        ar << pPolar->m_Reynolds << pPolar->m_Mach;
        ar << pPolar->m_ASpec;
        ar << pPolar->m_XTop << pPolar->m_XBot;
        ar << pPolar->m_ACrit;

        ar << pPolar->m_Alpha.size();
        for (i=0; i< pPolar->m_Alpha.size(); i++)
        {
            ar << float(pPolar->m_Alpha[i]) << float(pPolar->m_Cd[i]) ;
            ar << float(pPolar->m_Cdp[i])   << float(pPolar->m_Cl[i]) << float(pPolar->m_Cm[i]);
            ar << float(pPolar->m_XTr1[i])  << float(pPolar->m_XTr2[i]);
            ar << float(pPolar->m_HMom[i])  << float(pPolar->m_Cpmn[i]);
            ar << float(pPolar->m_Re[i]);
            ar << float(pPolar->m_XCp[i]);
        }

        ar << pPolar->m_PointStyle;
        // space allocation for the future storage of more data, without need to change the format
        for (int i=0; i<19; i++) ar << 0;
        for (int i=0; i<50; i++) ar << 0.0;

        return true;
    }
    else
    {
        //read variables
        float Alpha=0, Cd=0, Cdp=0, Cl=0, Cm=0, XTr1=0, XTr2=0, HMom=0, Cpmn=0, Re=0, XCp=0;

        ar >> ArchiveFormat;
        if (ArchiveFormat <100000 || ArchiveFormat>110000) return false;

        ar >> pPolar->m_FoilName;
        ar >> pPolar->m_PlrName;

        ar >> n; pPolar->setPolarStyle(n);
        ar >> n; pPolar->setPolarWidth(n);
        readColor(ar, pPolar->m_red, pPolar->m_green, pPolar->m_blue, pPolar->m_alphaChannel);
        ar >> pPolar->m_bIsVisible >> boolean;

        ar >> n;
        if(n==1)      pPolar->m_PolarType=XFLR5::FIXEDSPEEDPOLAR;
        else if(n==2) pPolar->m_PolarType=XFLR5::FIXEDLIFTPOLAR;
        else if(n==3) pPolar->m_PolarType=XFLR5::RUBBERCHORDPOLAR;
        else if(n==4) pPolar->m_PolarType=XFLR5::FIXEDAOAPOLAR;

        ar >> pPolar->m_MaType >> pPolar->m_ReType;
        ar >> pPolar->m_Reynolds >> pPolar->m_Mach;
        ar >> pPolar->m_ASpec;
        ar >> pPolar->m_XTop >> pPolar->m_XBot;
        ar >> pPolar->m_ACrit;

        ar >> n;

        for (i=0; i< n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm >> XTr1 >> XTr2 >> HMom >> Cpmn >> Re >> XCp;
            pPolar->addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm),
                             double(XTr1), double(XTr2), double(HMom), double(Cpmn), double(Re), double(XCp));
        }

        ar >> pPolar->m_PointStyle;

        // space allocation
        for (int i=0; i<19; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}


/**
 * Loads or saves the data of this OpPoint to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool MainFrame::serializeOppXFL(OpPoint *pOpp, QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    bool boolean=false;
    int k=0;
    float f0=0.f,f1=0.f;
    double dble=0;

    if(bIsStoring)
    {
        ar << 200003;
        //200003 : first xfl format
        //write variables
        ar << pOpp->foilName();
        ar << pOpp->polarName();

        ar << pOpp->m_Style << pOpp->m_Width;
        writeColor(ar, pOpp->m_red, pOpp->m_green, pOpp->m_blue, pOpp->m_alphaChannel);
        ar << pOpp->m_bIsVisible << false;

        ar << pOpp->m_Reynolds << pOpp->m_Mach << pOpp->m_Alpha;
        ar << pOpp->n << pOpp->blx.nd1 << pOpp->blx.nd2 << pOpp->blx.nd3;

        ar << pOpp->m_bViscResults;
        ar << pOpp->m_bBL;

        ar << pOpp->Cl << pOpp->Cm << pOpp->Cd << pOpp->Cdp;
        ar << pOpp->Xtr1 << pOpp->Xtr2 << pOpp->m_XCP;
        ar << pOpp->ACrit << pOpp->m_TEHMom << pOpp->Cpmn;

        for (k=0; k<pOpp->n; k++)     ar << float(pOpp->Cpv[k]) << float(pOpp->Cpi[k]);
        for (k=0; k<pOpp->n; k++)     ar << float(pOpp->Qv[k] ) << float(pOpp->Qi[k]);
        for (k=0; k<=pOpp->blx.nd1; k++)  ar << float(pOpp->blx.xd1[k]) << float(pOpp->blx.yd1[k]);
        for (k=0; k<pOpp->blx.nd2; k++)   ar << float(pOpp->blx.xd2[k]) << float(pOpp->blx.yd2[k]);
        for (k=0; k<pOpp->blx.nd3; k++)      ar << float(pOpp->blx.xd3[k]) << float(pOpp->blx.yd3[k]);

        // space allocation for the future storage of more data, without need to change the format
        for (int i=0; i<20; i++) ar << 0;
        dble = 0;
        for (int i=0; i<50; i++) ar << dble;
    }
    else
    {

        ar >> ArchiveFormat;
        //write variables
        ar >> pOpp->m_FoilName;
        ar >> pOpp->m_PlrName;

        ar >> pOpp->m_Style >> pOpp->m_Width;
        readColor(ar, pOpp->m_red, pOpp->m_green, pOpp->m_blue, pOpp->m_alphaChannel);
        ar >> pOpp->m_bIsVisible >> boolean;

        ar >> pOpp->m_Reynolds >> pOpp->m_Mach >> pOpp->m_Alpha;
        ar >> pOpp->n >> pOpp->blx.nd1 >> pOpp->blx.nd2 >> pOpp->blx.nd3;

        ar >> pOpp->m_bViscResults;
        ar >> pOpp->m_bBL;

        ar >> pOpp->Cl >> pOpp->Cm >> pOpp->Cd >> pOpp->Cdp;
        ar >> pOpp->Xtr1 >> pOpp->Xtr2 >> pOpp->m_XCP;
        ar >> pOpp->ACrit >> pOpp->m_TEHMom >> pOpp->Cpmn;

        for (k=0; k<pOpp->n; k++)
        {
            ar >> f0 >> f1;
            pOpp->Cpv[k] = double(f0);
            pOpp->Cpi[k] = double(f1);
        }
        for (k=0; k<pOpp->n; k++)
        {
            ar >> f0 >> f1;
            pOpp->Qv[k] = double(f0);
            pOpp->Qi[k] = double(f1);
        }
        for (k=0; k<=pOpp->blx.nd1; k++)
        {
            ar >> f0 >> f1;
            pOpp->blx.xd1[k] = double(f0);
            pOpp->blx.yd1[k] = double(f1);
        }
        for (k=0; k<pOpp->blx.nd2; k++)
        {
            ar >> f0 >> f1;
            pOpp->blx.xd2[k] = double(f0);
            pOpp->blx.yd2[k] = double(f1);
        }
        for (k=0; k<pOpp->blx.nd3; k++)
        {
            ar >> f0 >> f1;
            pOpp->blx.xd3[k] = double(f0);
            pOpp->blx.yd3[k] = double(f1);
        }

        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}



/**
 * The user has requested of the graph data to a text file
 */
void MainFrame::exportGraph(Graph *pGraph)
{
    QString FileName = pGraph->graphName().trimmed();
    FileName.replace(" ", "_");
    FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Graph"), exportLastDirName()+"/"+pGraph->graphName(),
                                            QObject::tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &exportGraphFilter());
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) exportLastDirName() = FileName.left(pos);

    bool bCSV = false;
    if(exportGraphFilter().indexOf("*.txt")>0)
    {
        Settings::s_ExportFileType = XFLR5::TXT;
        if(FileName.indexOf(".txt")<0) FileName +=".txt";
    }
    else if(exportGraphFilter().indexOf("*.csv")>0)
    {
        Settings::s_ExportFileType = XFLR5::CSV;
        if(FileName.indexOf(".csv")<0) FileName +=".csv";
        bCSV = true;
    }

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    pGraph->exportToFile(XFile, bCSV);
}


void MainFrame::onPreferences()
{
    PreferencesDlg dlg(this);
    dlg.m_pSaveOptionsWt->initWidget(m_bAutoLoadLast, m_bSaveOpps, m_bSaveWOpps, m_bAutoSave, m_SaveInterval);
    dlg.m_pUnitsWt->initWidget();
    dlg.m_pDisplayOptionsWt->initWidget();
    dlg.m_pLanguageWt->initWidget();

    if(dlg.exec()==QDialog::Accepted)
    {
        m_bAutoLoadLast = dlg.m_pSaveOptionsWt->m_bAutoLoadLast;
        m_bAutoSave     = dlg.m_pSaveOptionsWt->m_bAutoSave;
        m_SaveInterval  = dlg.m_pSaveOptionsWt->m_SaveInterval;
        m_bSaveOpps     = dlg.m_pSaveOptionsWt->m_bOpps;
        m_bSaveWOpps    = dlg.m_pSaveOptionsWt->m_bWOpps;

        if(m_bAutoSave)
        {
            if(m_pSaveTimer)
            {
                m_pSaveTimer->stop();
                delete m_pSaveTimer;
            }
            m_pSaveTimer = new QTimer(this);
            m_pSaveTimer->setInterval(m_SaveInterval*60*1000);
            m_pSaveTimer->start();
            connect(m_pSaveTimer, SIGNAL(timeout()), SLOT(onSaveTimer()));
        }

        saveSettings();
    }

    if(dlg.m_pDisplayOptionsWt->m_bIsGraphModified)
    {
        setGraphSettings(&Settings::s_RefGraph);
    }
    m_pAFoil->setTableFont();
    if(Settings::s_Theme==SETTINGS::DARKTHEME)
    {
        m_pXDirectTileWidget->opPointWidget()->setNeutralLineColor(QColor(190,190,190));
        m_pAFoil->m_p2DWidget->setNeutralLineColor(QColor(190,190,190));
    }
    else
    {
        m_pXDirectTileWidget->opPointWidget()->setNeutralLineColor(QColor(60,60,60));
        m_pAFoil->m_p2DWidget->setNeutralLineColor(QColor(60,60,60));
    }
    gl3dMiarexView::s_bResetglGeom = true;
    gl3dMiarexView::s_bResetglBody = true;
    gl3dMiarexView::s_bResetglLegend = true;

    m_pMiarex->m_CpGraph.setInverted(true);
    m_pMiarex->m_bResetTextLegend = true;
    m_pMiarex->setControls();

    m_pXDirect->CpGraph()->setInverted(true);

    m_VoidWidget.update();

    setMainFrameCentralWidget();

    saveSettings();
    //    setSaveState(false);

    updateView();
}


/** auto check at startup */
void MainFrame::onAutoCheckForUpdates()
{
    //check if a month has passed since the last check
    QDate today = QDate::currentDate();
    Trace("Updater::lastCheckDate().isNull():  ", Updater::lastCheckDate().isNull());
    Trace("Updater::lastCheckDate().isValid(): ", Updater::lastCheckDate().isValid());
    Trace("Updater::lastCheckDate(): " + Updater::lastCheckDate().toString("yyyy.MM.dd"));
    if(Updater::lastCheckDate().isNull() || !Updater::lastCheckDate().isValid() || Updater::lastCheckDate().daysTo(today)>31)
    {
        m_bManualUpdateCheck = false;
        checkForUpdates();
    }
//    else checkForUpdates();
}


/** manual check triggered by menu action */
void MainFrame::onCheckForUpdates()
{
    m_bManualUpdateCheck = true;
    checkForUpdates();
}


void MainFrame::checkForUpdates()
{
    if(!m_pUpdater)
    {
        m_pUpdater = new Updater(this);
        connect(m_pUpdater, SIGNAL(finishedUpdate()), this, SLOT(onFinishedUpdater()));
    }
    if(m_pUpdater)
        QTimer::singleShot(0, m_pUpdater, SLOT(checkForUpdates()));

 /*
    QThread *pThread = new QThread;
    m_pUpdater->moveToThread(pThread);
    connect(pThread,    SIGNAL(started()),        m_pUpdater, SLOT(checkForUpdates()));
    connect(pThread,    SIGNAL(finished()),       pThread,    SLOT(deleteLater()));
    pThread->start();
    pThread->setPriority(QThread::LowPriority);*/
}


void MainFrame::onFinishedUpdater()
{
    if(!m_pUpdater) return;
    Popup *pPopup = new Popup;

    Trace("Finished update check; available version: ", double(Updater::majorVersion())+double(Updater::minorVersion())/100.0);

    if(m_pUpdater->hasUpdate())  pPopup->setRed();
    else                         pPopup->setGreen();

    if(m_pUpdater->hasUpdate() || m_bManualUpdateCheck)
    {
        QString strange;
        strange.sprintf("Latest version %d.%d", Updater::majorVersion(), Updater::minorVersion());
        pPopup->setTextMessage(strange);
        pPopup->appendTextMessage("Release date: " + m_pUpdater->releaseDate());
        strange.sprintf("%d.%d", MAJOR_VERSION, MINOR_VERSION);
        pPopup->appendTextMessage("Your version: " + strange);
        //        aPopup->appendTextMessage(m_pUpdater->releaseDescription()+"\n");

        QSize sz = pPopup->size();
        QRect rr = centralWidget()->frameGeometry();
        QPoint tr = rr.topRight();

        pPopup->setWindowPos(QPoint(pos().x()+centralWidget()->pos().x() + tr.x()-sz.width(),
                                    pos().y()+centralWidget()->pos().y()));
        pPopup->show();

    }
}
