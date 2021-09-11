/****************************************************************************

    XFLR5App  Class
    Copyright (C) 2008-2017 André Deperrois 

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

#include <QFileOpenEvent>
#include <QSplashScreen>
#include <QDateTime>
#include <QMessageBox>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QSurfaceFormat>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <ctime>
#include <iostream>

#include "xflr5app.h"
#include <globals/mainframe.h>
#include <xflcore/trace.h>



XFLR5App::XFLR5App(int &argc, char** argv) : QApplication(argc, argv)
{
    setApplicationDisplayName(VERSIONNAME);
    setApplicationName(VERSIONNAME);
//    setDesktopFileName(VERSIONNAME);
    setOrganizationName("Cere-Aero");
    setOrganizationDomain("cere-aero.tech");

    m_bDone = false;

    QString StyleName;
    QString LanguagePath ="";

    QString str;

    int a=150;
    int b=50;
    int c=800;
    int d=700;

#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5v649");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

//    qsrand(uint(time(nullptr)));

    bool bMaximized = true;
    bool bOK= false;
    bool bSheet = false;
    int k=0;
    settings.beginGroup("MainFrame");
    {
        int SettingsFormat = settings.value("SettingsFormat").toInt();
        if(SettingsFormat == SETTINGSFORMAT)
        {
            k = settings.value("FrameGeometryx").toInt(&bOK);
            if(bOK) a = k;
            k = settings.value("FrameGeometryy").toInt(&bOK);
            if(bOK) b = k;
            k = settings.value("SizeWidth").toInt(&bOK);
            if(bOK) c = k;
            k = settings.value("SizeHeight").toInt(&bOK);
            if(bOK) d = k;

            bMaximized = settings.value("SizeMaximized").toBool();

            str = settings.value("LanguageFilePath").toString();
            if(str.length()) LanguagePath = str;

            str = settings.value("StyleName").toString();
            if(str.length()) StyleName = str;


            bSheet = settings.value("bStyleSheet", false).toBool();
        }
    }
    settings.endGroup();

    QTranslator xflr5Translator;
    if(LanguagePath.length())
    {
        if(xflr5Translator.load(LanguagePath)) installTranslator(&xflr5Translator);
    }


    if(StyleName.length())	qApp->setStyle(StyleName);
    if(bSheet)
    {
        QFile stylefile;
        QString qssPathName =  qApp->applicationDirPath() + QDir::separator() +"/xflr5_dark.qss";

        QFileInfo fi(qssPathName);
        if(fi.exists())
            stylefile.setFileName(qssPathName);
        else
            stylefile.setFileName(QStringLiteral(":/qss/xflr5_dark.qss"));

        if (stylefile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString qsStylesheet = QString::fromLatin1(stylefile.readAll());
            qApp->setStyleSheet(qsStylesheet);
            stylefile.close();
        }
    }

    int OGLversion = -1;
    bool bScript=false, bShowProgress=false;
    QString ScriptPathName;

    parseCmdLine(*this, ScriptPathName, bScript, bShowProgress, OGLversion);

    QPixmap pixmap;
    pixmap.load(":/images/splash.png");
    QSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::SplashScreen);
    if(!bScript)
    {
        splash.show();
    //else            - JX mod: fix of r1230
    //{
#ifdef Q_OS_WIN
#ifndef QT_DEBUG
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
#endif
    }

    QPoint pt(a,b);
    QSize sz(c,d);

    if(StyleName.length())    qApp->setStyle(StyleName);
    MainFrame *pMainFrame = MainFrame::self();
    MainFrame::self()->resize(sz);
    MainFrame::self()->move(pt);

    if(bScript)
    {
        pMainFrame->executeScript(ScriptPathName, bShowProgress);
        m_bDone = true;
        return;
    }


#ifndef Q_OS_MAC
    bool bProjectFile = false;
    if(argc>1)
    {
        QString PathName, Extension;
        PathName=argv[1];
        PathName.replace("'","");
        QFileInfo fi(PathName);
        Extension = fi.suffix();

        if (Extension.compare("xfl",Qt::CaseInsensitive)==0 || Extension.compare("wpa",Qt::CaseInsensitive)==0 ||
            Extension.compare("plr",Qt::CaseInsensitive)==0 || Extension.compare("dat",Qt::CaseInsensitive)==0)
        {
            bProjectFile = true;
            int iApp = pMainFrame->loadXFLR5File(PathName);

            if      (iApp == xfl::MIAREX)        pMainFrame->onMiarex();
            else if (iApp == xfl::XFOILANALYSIS) pMainFrame->onXDirect();
        }
    }

    if(!bProjectFile)
    {
        if(pMainFrame->bAutoLoadLast())
        {
            pMainFrame->loadLastProject();
        }
    }
#else
    if(pMainFrame->bAutoLoadLast() && !MainFrame::projectName().length())
    {
        // if nothing has been loaded, load the last project file
        pMainFrame->onLoadLastProject();
    }
#endif

    addStandardBtnStrings();


    if(bMaximized)  pMainFrame->showMaximized();
    else            pMainFrame->show();
    splash.finish(pMainFrame);

}


void XFLR5App::addStandardBtnStrings()
{
    QT_TRANSLATE_NOOP("QPlatformTheme", "OK");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Save");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Save All");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Open");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Yes to &All");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&No");
    QT_TRANSLATE_NOOP("QPlatformTheme", "N&o to All");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Abort");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Retry");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Ignore");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Close");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Discard");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Help");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Apply");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Reset");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Restore Defaults");
}


bool XFLR5App::event(QEvent *pEvent)
{
    int iApp = xfl::NOAPP;
    switch (pEvent->type())
    {
        case QEvent::FileOpen:
        {
            iApp = MainFrame::self()->loadXFLR5File(static_cast<QFileOpenEvent *>(pEvent)->file());
            if (iApp == xfl::MIAREX)             MainFrame::self()->onMiarex();
            else if (iApp == xfl::XFOILANALYSIS) MainFrame::self()->onXDirect();

            return true;
        }

        default:
            break;
    }
    return QApplication::event(pEvent);
}



void XFLR5App::parseCmdLine(XFLR5App &xflapp,
                            QString &scriptfilename, bool &bScript, bool &bShowProgress,
                            int &OGLVersion)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Analysis tool for planes and sails operating at low Reynolds numbers");
    parser.addHelpOption();
    parser.addVersionOption();

    // An integer option with a single name (-o)
    QCommandLineOption OGLOption(QStringList() << "o" << "opengl");
    OGLOption.setValueName("OpenGL_version");
    OGLOption.setDefaultValue("44");
    OGLOption.setDescription("Launches the application with the specified OpenGL version. "
                             "The default is the legacy format 2.1. Test and set higher "
                             "versions using the in-app OpenGL test window in the Options menu. "
                             "Usage: xflr5 -o 41 to request a 4.1 context.");
    parser.addOption(OGLOption);

    QCommandLineOption ShowProgressOption(QStringList() << "p" << "progress");
    ShowProgressOption.setDescription(QCoreApplication::translate("main", "Show progress during script execution."));
    parser.addOption(ShowProgressOption);

    QCommandLineOption ScriptOption(QStringList() << "s" << "script");
    ScriptOption.setValueName("file");
    ScriptOption.setDescription("Runs the script file");
    parser.addOption(ScriptOption);

    QCommandLineOption TraceOption(QStringList() << "t" << "trace");
    TraceOption.setDescription("Runs the program in trace mode. The trace file is "+QDir::tempPath() + "/Trace.log");
    parser.addOption(TraceOption);

    // Process the actual command line arguments provided by the user
    parser.process(xflapp);

    bShowProgress = parser.isSet(ShowProgressOption);
    bScript = parser.isSet(ScriptOption);
    scriptfilename = parser.value(ScriptOption);


    bScript = parser.isSet(ScriptOption);
    if(bScript)
    {
        Trace("Processing option -s", true);
    }

    if(parser.isSet(TraceOption))
    {
        Trace("Processing option -t", true);
        g_bTrace=true;
    }


    if(parser.isSet(OGLOption))
    {
        bool bOK=false;
        int version = parser.value(OGLOption).toInt(&bOK);
        if(bOK) OGLVersion = version; else OGLVersion = -1;
        Trace("Processing option -o", OGLVersion);
    }
    else
    {
        OGLVersion = -1;
    }
}

