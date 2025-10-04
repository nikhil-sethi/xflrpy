/****************************************************************************

    XFLScriptReader Class
    Copyright (C) 2016-2016 André Deperrois 

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

#ifndef XFLSCRIPTREADER_H
#define XFLSCRIPTREADER_H

#include <QFile>
#include <QXmlStreamReader>
#include <QVector>
#include <QThread>

#include <xflcore/core_enums.h>

class XFLScriptReader : public QXmlStreamReader
{
    friend class XflScriptExec;

public:
    XFLScriptReader();
    bool readScript();

    bool readDirectoryData();
    bool readBatchAnalysisData();
    bool readFoilAnalysisFiles();
    bool readFoilAnalysisOptions();
    bool readFoilAnalysisOutput();
    bool readFoilAnalysisRange();
    bool readFoilBatchRange();
    bool readFoilData();
    bool readFoils();
    bool readMetaData();
    bool readThreadingOptions();

    bool bMakeFoilOpps() const {return m_bMakeOpps;}
    bool bRunAllAnalyses() const {return m_bRunAllFoilAnalyses;}

    QString const &datFoilDirPath()     const {return m_datFoilDirPath;}
    QString const &binPolarDirPath()    const {return m_PolarBinDirPath;}
    QString const &xfoilPolarDirPath()  const {return m_XFoilPolarsDir;}
    QString const &xmlAnalysisDirPath() const {return m_xmlAnalysisDirPath;}
    QString const &outputDirPath()      const {return m_OutputDirPath;}

private:
    QString const &projectFileName()   const {return m_ProjectFileName;}

    QStringList m_FoilList, m_PolarList;

    QVector<double> m_Reynolds, m_NCrit, m_Mach; /** Type 123 polars */
    QVector<double> m_Alpha; /** Type 4 polars */
    double m_XtrTop, m_XtrBot;

    double m_aoaMin, m_aoaMax, m_aoaInc;
    double m_ClMin, m_ClMax, m_ClInc;
    double m_ReMin, m_ReMax, m_ReInc;

    int m_MaxXFoilIterations;
    int m_NFoilPanels;
    bool m_bRepanelFoils;

    QString m_OutputDirectoryPath, m_ProjectFileName;
    QString m_datFoilDirPath, m_xmlAnalysisDirPath;
    QString m_PolarBinDirPath, m_XFoilPolarsDir;
    QString m_OutputDirPath;
    QStringList m_XmlFoilAnalysisList;         /**< The list of xml foil analysis files to load */

    bool m_bOutputPolarsBin;
    bool m_bOutputPolarsText;
    bool m_bMakeProjectFile;
    bool m_bRunAllFoilAnalyses;

    bool m_bMakeOpps, m_bAlphaSpec, m_bFromZero;

    bool m_bMakeXfl;
    bool m_bMakePOpps;
    bool m_bMultiThreading;
    int m_nMaxThreads;
    bool m_bRecursiveDirScan;
    bool m_bcsvPolarOutput;

    xfl::enumPolarType m_PolarType;

    QThread::Priority m_ThreadPriority;
};

#endif // XFLSCRIPTREADER_H
