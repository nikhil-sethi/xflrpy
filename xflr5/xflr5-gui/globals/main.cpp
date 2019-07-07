/****************************************************************************

    XFLR5 Application

    Copyright (C) 2008-2019 Andre Deperrois techwinder@users.sourceforge.net

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

#include "xflr5application.h"
#include <globals/mainframe.h>



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
    strange.sprintf("Default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
    qDebug()<<strange;*/
#endif

/*    QSurfaceFormat defaultFormat = QSurfaceFormat::defaultFormat();
    defaultFormat.setVersion(3, 3);
//    defaultFormat.setProfile(QSurfaceFormat::CompatibilityProfile); //only relevant for 3.2+
    QSurfaceFormat::setDefaultFormat(defaultFormat);*/

/*
#ifdef QT_DEBUG
    QString strange;
    strange.sprintf("App default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
    qDebug()<<strange;
    switch (QSurfaceFormat::defaultFormat().profile()) {
        case QSurfaceFormat::NoProfile:
                qDebug()<<"   No Profile";
            break;
        case QSurfaceFormat::CoreProfile:
                qDebug()<<"   Core Profile";
            break;
        case QSurfaceFormat::CompatibilityProfile:
                qDebug()<<"   Compatibility Profile";
            break;
        default:
            break;
    }
#endif*/

    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    XFLR5Application app(argc, argv);

    return app.exec();
}




