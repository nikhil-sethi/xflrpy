/****************************************************************************

    XflScriptExec Class
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

#pragma once

#include <QTextStream>

#include <xflscript/xflscriptreader.h>



class Foil;
class PlaneOpp;
class Polar;
class XFoilTask;
class MainFrame;

struct FoilAnalysis;

class XflScriptExec : public QObject
{
        Q_OBJECT
    public:
        XflScriptExec(MainFrame *pMainFrame);
        ~XflScriptExec() override;

        void customEvent(QEvent *pEvent) override;

        bool readScript(QString const &scriptpathname);
        bool runScript();
        void setStdOutStream(bool bStdOut) {m_bStdOutStream = bStdOut;}

        bool bCSVOutput() const {return m_Reader.m_bcsvPolarOutput;}

        void traceLog(QString const &strMsg);

        QString const &outputDirPath()               const {return m_OutputPath;}
        QString const &foilPolarBinOutputDirPath()   const {return m_Reader.binPolarDirPath();}
        QString const &xfoilPolarOutputDirPath()     const {return m_Reader.xfoilPolarDirPath();}

        QString projectFilePathName() const;

        bool outputPolarBin()   const {return m_Reader.m_bOutputPolarsBin;}
        bool outputPolarText()  const {return m_Reader.m_bOutputPolarsText;}
        bool makeProjectFile()  const {return m_Reader.m_bMakeProjectFile;}
        bool setLogFile();

        void closeLogFile();
        QString const &logFileName() const {return m_LogFileName;}

    private:
        Polar* makePolarFromXml(const QString &pathName);
        bool makeExportDirectories();
        bool makeFoils();
        void makeFoilAnalysisList();
        void runFoilAnalyses();

    signals:
        void msgUpdate(const QString &msg) const;
        void cancelTask() const;

    public slots:
        void onCancel();

    private:
        XFLScriptReader m_Reader;
        QFile *m_pXFile;
        QTextStream m_OutLogStream;
        QString m_LogFileName;
        QString m_OutputPath;

        QVector<FoilAnalysis> m_FoilExecList;

        MainFrame *m_pMainFrame;

        bool m_bCancel;
        bool m_bStdOutStream;

        int m_nTaskStarted, m_nTaskDone;
        int m_nThreads;

        QVector <Foil*>  m_oaFoil;

        static QString s_VersionName;
};



