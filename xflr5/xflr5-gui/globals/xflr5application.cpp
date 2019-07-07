/****************************************************************************

    XFLR5Application  Class
    Copyright (C) 2008-2017 Andre Deperrois 

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
#include <QDebug>


#include "xflr5application.h"
#include <globals/mainframe.h>
#include <misc/options/settings.h>
#include <ctime>
#include <iostream>

XFLR5Application::XFLR5Application(int &argc, char** argv) : QApplication(argc, argv)
{
    setApplicationDisplayName(VERSIONNAME);
    setApplicationName(VERSIONNAME);
//    setDesktopFileName(VERSIONNAME);

    QPixmap pixmap;
    pixmap.load(":/images/splash.png");
    QSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::SplashScreen);
    splash.show();

    QString StyleName;
    QString LanguagePath ="";

    QString str;
    int a,b,c,d,k;
    a=150;
    b=50;
    c=800;
    d=700;


#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

    qsrand(uint(time(nullptr)));

    bool bMaximized = true;
    bool bOK;
    settings.beginGroup("MainFrame");
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
    }
    settings.endGroup();

    QTranslator xflr5Translator;
    if(LanguagePath.length())
    {
        if(xflr5Translator.load(LanguagePath)) installTranslator(&xflr5Translator);
    }

    QPoint pt(a,b);
    QSize sz(c,d);

    if(StyleName.length())    qApp->setStyle(StyleName);
    MainFrame *w = MainFrame::self();
    MainFrame::self()->resize(sz);
    MainFrame::self()->move(pt);
    if(bMaximized)    MainFrame::self()->showMaximized();
    else            MainFrame::self()->show();
    splash.finish(w);


#ifndef Q_OS_MAC
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
            int iApp = w->loadXFLR5File(PathName);

            if (iApp == XFLR5::MIAREX)             w->onMiarex();
            else if (iApp == XFLR5::XFOILANALYSIS) w->onXDirect();
        }
    }
    else
    {
        if(w->bAutoLoadLast())
        {
            w->onLoadLastProject();
        }
    }
#else
    if(w->bAutoLoadLast() && !MainFrame::projectName().length())
    {
        // if nothing has been loaded, load the last project file
        w->onLoadLastProject();
    }
#endif

    addStandardBtnStrings();
}


void XFLR5Application::addStandardBtnStrings()
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


bool XFLR5Application::event(QEvent *event)
{
    int iApp;
    switch (event->type())
    {
        case QEvent::FileOpen:
        {
            iApp = MainFrame::self()->loadXFLR5File(static_cast<QFileOpenEvent *>(event)->file());
            if (iApp == XFLR5::MIAREX)             MainFrame::self()->onMiarex();
            else if (iApp == XFLR5::XFOILANALYSIS) MainFrame::self()->onXDirect();

            return true;
        }

        default:
            break;
    }
    return QApplication::event(event);
}



