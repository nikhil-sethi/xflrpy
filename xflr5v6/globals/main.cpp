/****************************************************************************

    XFLR5 Application

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

#include <QSurfaceFormat>
#include <QDebug>

#include "xflr5app.h"
#include <globals/mainframe.h>
#include <xflcore/trace.h>
#include <xfl3d/views/gl3dview.h>

void customLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    (void)context;
    QByteArray localMsg = msg.toLocal8Bit();
//    const char *file = context.file ? context.file : "";
//    const char *function = context.function ? context.function : "";
    switch (type)
    {
        case QtDebugMsg:
//            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "%s\n", localMsg.constData());
            break;
        case QtInfoMsg:
//            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Info: %s\n", localMsg.constData());
            break;
        case QtWarningMsg:
//            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Warning: %s\n", localMsg.constData());
            break;
        case QtCriticalMsg:
//            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Critical: %s\n", localMsg.constData());
            break;
        case QtFatalMsg:
//            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Fatal: %s\n", localMsg.constData());
            break;
    }

    if(g_bTrace) Trace(msg);
}



/** OpenGL Default format must be set prior to app construction if Qt::AA_ShareOpenGLContexts is set */
void setOGLDefaultFormat(int version)
{
#if defined Q_OS_MAC && defined MAC_NATIVE_PREFS
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5v649");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
    // Load preferred OpenGL version
    // and set the default format before any 3d view is created
    int OGLMajor = 3;
    int OGLMinor = 3;
    if(QFile(settings.fileName()).exists())
    {
        gl3dView::loadSettings(settings);
        OGLMajor = gl3dView::oglMajor();
        OGLMinor = gl3dView::oglMinor();
    }

    if(version>0)
    {
        OGLMajor = int(version/10);
        OGLMinor = version - 10*OGLMajor;
        qDebug()<<"setting context"<<OGLMajor<<OGLMinor;
    }

    // choose between the version passed as option if valid and the saved setting

    if(OGLMajor<=2 || (OGLMajor==3 && OGLMinor<3))
    {
        // Systems (may? commonly?) respond with the latest 4.x context,
        // so force deprecated functions and compatibility profile
        // Will also force v120 style shaders in GL initialization
        gl3dView::setProfile(QSurfaceFormat::NoProfile);
        gl3dView::setDeprecatedFuncs(true);
    }
    else
    {
        gl3dView::setProfile(QSurfaceFormat::CoreProfile);
        gl3dView::setDeprecatedFuncs(false);
    }

    gl3dView::setOGLVersion(OGLMajor, OGLMinor);

    if(gl3dView::defaultXflSurfaceFormat().samples()<0) gl3dView::setDefaultSamples(4);


    QSurfaceFormat::setDefaultFormat(gl3dView::defaultXflSurfaceFormat()); // for all QOpenGLWidgets
//qDebug()<<gl3dView::defaultXflSurfaceFormat();
}

/**
*The app's point of entry !
*/
int main(int argc, char *argv[])
{
//    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

#ifdef Q_OS_MACX
    /*
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
    */
#endif

/* Note: Calling QSurfaceFormat::setDefaultFormat() before constructing the QApplication instance
 * is mandatory on some platforms (for example, OS X) when an OpenGL core profile context is requested.
 * This is to ensure that resource sharing between contexts stays functional as all internal contexts
 *  are created using the correct version and profile.
 *
 * */
/*
 *  https://www.opengl.org/wiki/Core_And_Compatibility_in_Contexts
 *  Platform Issue (MacOSX): When MacOSX 10.7 introduced support for OpenGL beyond 2.1, they also
 *  introduced the core/compatibility dichotomy. However, they did not introduce support for the
 *  compatibility profile itself. Instead, MacOSX gives you a choice: core profile for versions 3.2 or
 *  higher, or just version 2.1.
 *  There is no way to get access to features after 2.1 and still access the Fixed Function Pipeline.
 */


#ifdef QT_DEBUG
/*    QString strange;
    strange = QString::asprintf("Default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
    qDebug()<<strange;*/
#endif


    qInstallMessageHandler(&customLogHandler);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);


    /*To set up sharing between QOpenGLWidget instances belonging to different windows,
     * set the Qt::AA_ShareOpenGLContexts application attribute before instantiating QApplication.
     * This will trigger sharing between all QOpenGLWidget instances without any further steps.*/
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
//    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
//    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
//    qDebug()<<QCoreApplication::testAttribute(Qt::AA_ShareOpenGLContexts);

    int version = -1;
    for(int i=0; i<argc; i++)
    {
        QString strange = argv[i];
        if(strange.compare("-o", Qt::CaseSensitive)==0 && i<argc-1)
        {
            version = QString(argv[i+1]).toInt();
            qDebug()<<"OGL version" << version << "requested";
        }
    }
    setOGLDefaultFormat(version);
    qInstallMessageHandler(&customLogHandler);
    XFLR5App app(argc, argv);

    if(app.done())	return 0;
    else            return app.exec();
}




