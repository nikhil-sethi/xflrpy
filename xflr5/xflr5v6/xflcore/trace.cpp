/****************************************************************************

    Trace functions

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

#include <QFile>
#include <QTextStream>
#include <QSurfaceFormat>
#include <QDir>
#include <QOperatingSystemVersion>
#include <QThread>
#include <QApplication>

#include <xflcore/trace.h>

bool g_bTrace = true;
QFile *g_pTraceFile = nullptr;

/**
* Outputs in a debug file the current time and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log
* Used for debugging.
*@param n the integer to output
*/
void Trace(int n)
{
    if(!g_bTrace) return;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << "Int value=" << n << "\n";
    }
}


void Trace(QString const &msg, bool b)
{
    if(!g_bTrace) return;
    QString str;
    if(b) str += msg + "= true";
    else  str += msg + "= false";

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << str << "\n";
    }
}


/**
* Outputs in a debug file the current time and a string message passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log.
* Used for debugging.
*@param msg the message to output
*/
void Trace(QString const &msg)
{
    if(!g_bTrace) return;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts<<msg<<"\n";
        ts.flush();
    }
}


/**
* Outputs in a debug file the current time, a string message and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log.
* Used for debugging.
* @param msg the message to output
* @param n the integer to output
*/
void Trace(QString const &msg, int n)
{
    if(!g_bTrace) return;

    QString strong;
    strong = QString("  %1").arg(n);
    strong = msg + strong;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << strong << "\n";
    }
}


/**
* Outputs in a debug file the current time, a string message and the value of the floating number passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log.
* Used for debugging.
* @param msg the message to output
* @param f the float number to output
*/
void Trace(QString const &msg, double f)
{
    if(!g_bTrace) return;

    QString strong;
    strong = QString("  %1").arg(f);
    strong = msg + strong;


    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << strong << "\n";
    }
}


void startTrace(bool bTrace)
{
    g_bTrace = bTrace;

    QString FileName = QDir::tempPath() + "/Trace.log";

    g_pTraceFile = new QFile(FileName);

    if (!g_pTraceFile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) g_bTrace = false;
    g_pTraceFile->reset();

    QString strange;
    QOperatingSystemVersion const &sys = QOperatingSystemVersion::current();
    strange = sys.name();
    Trace(strange+"\n");

    QSysInfo sysInfo;

    strange  = "SysInfo:\n";
    strange += "   build ABI:       "  + sysInfo.buildAbi() +"\n";
    strange += "   build CPU:       "  + sysInfo.buildCpuArchitecture() +"\n";
    strange += "   current CPU:     "  + sysInfo.currentCpuArchitecture() +"\n";
    strange += "   kernel type:     "  + sysInfo.kernelType() +"\n";
    strange += "   kernel version:  "  + sysInfo.kernelVersion() +"\n";
    strange += "   product name:    "  + sysInfo.prettyProductName() +"\n";
    strange += "   product type:    "  + sysInfo.productType() +"\n";
    strange += "   product version: "  + sysInfo.productVersion() +"\n\n";
    Trace(strange);

    const char *qt_version = qVersion();
    strange = QString::asprintf("Qt version: %s\n\n", qt_version);
    Trace(strange);

    strange = QString::asprintf("Ideal thread count: %d\n\n", QThread::idealThreadCount());
    Trace(strange);


    strange = QString::asprintf("Default OpengGl format:%d.%d\n",
                    QSurfaceFormat::defaultFormat().majorVersion(),
                    QSurfaceFormat::defaultFormat().minorVersion());
    Trace(strange);
    Trace("OpenGL support:\n");
    Trace("    Desktop OpenGL ", qApp->testAttribute(Qt::AA_UseDesktopOpenGL));
    Trace("    OpenGL ES      ", qApp->testAttribute(Qt::AA_UseOpenGLES));
    Trace("    Software OpenGL", qApp->testAttribute(Qt::AA_UseSoftwareOpenGL));
    Trace("\n");

}
