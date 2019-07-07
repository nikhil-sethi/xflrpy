/****************************************************************************

    xflScriptExec Class
    Copyright (C) 2016-2016 Andre Deperrois

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

#ifndef XFLSCRIPTEXEC_H
#define XFLSCRIPTEXEC_H

#include <QTextStream>

#include <script/xflscriptreader.h>



class Foil;
class Plane;
class PlaneAnalysisTask;
class PlaneOpp;
class Polar;
class WPolar;
class XfoilTask;


struct FoilAnalysis;
struct PlaneAnalysis;

class XflScriptExec
{
public:
    XflScriptExec();
    ~XflScriptExec();
    void readScript();
    void runScript();
    void makeFoils();
    void makePlanes();
    void loadFoilPolarFiles();
    void makeFoilAnalysisList();
    void makePlaneAnalysisList();

private:
    void setLogFile();
    void traceLog(QString strMsg);
    Polar *makePolar(QString pathName);
    WPolar *makeWPolar(QString pathName);

private:
    XFLScriptReader m_scriptReader;
    QFile *m_pXFile;
    QTextStream m_outLogStream;

    QVector<FoilAnalysis*> m_FoilExecList;
    QVector<PlaneAnalysis*> m_PlaneExecList;

    QVector <Plane *> m_oaPlane;   /**< The array of void pointers to the Plane objects. */
    QVector <WPolar *> m_oaWPolar;  /**< The array of void pointers to the WPolar objects. */
    QVector <PlaneOpp *> m_oaPOpp;    /**< The array of void pointers to the PlaneOpp objects. */
    QVector <Foil*>  m_oaFoil;
    QVector <Polar*> m_oaPolar;
};

#endif // XFLSCRIPTEXEC_H
