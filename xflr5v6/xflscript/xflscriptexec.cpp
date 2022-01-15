/****************************************************************************

    xflScriptExec Class
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

#include <iostream>

#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QThreadPool>
#include <QDebug>

#include "xflscriptexec.h"
#include <xflobjects/xml/xmlplanereader.h>
#include <xflobjects/xml/xmlwpolarreader.h>

#include <xdirect/analysis/xfoiltask.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/xml/xmlpolarreader.h>
#include <xflanalysis/plane_analysis/planetask.h>
#include <xflcore/gui_params.h>
#include <xflcore/xflcore.h>
#include <xflcore/xflevents.h>
#include <xflobjects/objects_global.h>

QString XflScriptExec::s_VersionName;


XflScriptExec::XflScriptExec(MainFrame *pMainFrame)
{
    m_pXFile = nullptr;
    m_pMainFrame = pMainFrame;

    m_bStdOutStream = false;
    m_bCancel = false;

    m_nTaskStarted = m_nTaskDone =0;
    m_nThreads = 1;
}


XflScriptExec::~XflScriptExec()
{
    if(m_pXFile)
    {
        m_pXFile->close();
        delete m_pXFile;
    }
    m_FoilExecList.clear();
}


void XflScriptExec::makeFoilAnalysisList()
{
    m_FoilExecList.clear();

    // build from the xml files
    QStringList filters = {"*.xml"};
    QStringList xmlanalyseslist = m_Reader.m_PolarList;
    if(m_Reader.bRunAllAnalyses())
    {
        xmlanalyseslist = xfl::findFiles(m_Reader.xmlAnalysisDirPath(), filters, false);
    }

    for(int ip=0; ip<m_oaFoil.count(); ip++)
    {
        Foil *pFoil = m_oaFoil.at(ip);
        if(!pFoil) continue;

        for(int ip=0; ip<xmlanalyseslist.count(); ip++)
        {
            Polar *pPolar = makePolarFromXml(xmlanalyseslist.at(ip));
            if(pPolar)
            {
                pPolar->setFoilName(pFoil->name());
                pPolar->setTheStyle(pFoil->theStyle());
                pPolar->setVisible(true);
                pPolar->setAutoPolarName();

                if(pPolar->m_Reynolds<0.1)
                {
                    traceLog("   skipping polar "+pPolar->polarName()+": Reynolds number is null\n");
                    Objects2d::deletePolar(pPolar);
                    continue;
                }

                FoilAnalysis FoilAnalysis;
                switch(pPolar->polarType())
                {
                    case xfl::FIXEDSPEEDPOLAR:
                    case xfl::FIXEDLIFTPOLAR:
                    {
                        FoilAnalysis.vMin = m_Reader.m_aoaMin;
                        FoilAnalysis.vMax = m_Reader.m_aoaMax;
                        FoilAnalysis.vInc = m_Reader.m_aoaInc;
                        break;
                    }
                    case xfl::FIXEDAOAPOLAR:
                    {
                        FoilAnalysis.vMin = m_Reader.m_ReMin;
                        FoilAnalysis.vMax = m_Reader.m_ReMax;
                        FoilAnalysis.vInc = m_Reader.m_ReInc;
                        break;
                    }
                    default: break;
                }

                FoilAnalysis.pFoil = pFoil;
                FoilAnalysis.pPolar = pPolar;
                m_FoilExecList.append(FoilAnalysis);
                traceLog("   added analysis for foil "+pFoil->name()+" and "+pPolar->polarName() +"\n");
            }
        }
    }

    //build the batch range

    for(int ip=0; ip<m_oaFoil.count(); ip++)
    {
        Foil *pFoil = m_oaFoil.at(ip);
        if(!pFoil) continue;

        for(int ip=0; ip<m_Reader.m_Reynolds.count(); ip++)
        {
            Polar *pPolar = nullptr;

            FoilAnalysis FoilAnalysis;
            switch(m_Reader.m_PolarType)
            {
                case xfl::FIXEDSPEEDPOLAR:
                case xfl::FIXEDLIFTPOLAR:
                {
                    pPolar = Objects2d::createPolar(pFoil, m_Reader.m_PolarType, m_Reader.m_Reynolds.at(ip), m_Reader.m_Mach.at(ip),
                                                           m_Reader.m_NCrit.at(ip), m_Reader.m_XtrTop, m_Reader.m_XtrBot);

                    FoilAnalysis.vMin = m_Reader.m_aoaMin;
                    FoilAnalysis.vMax = m_Reader.m_aoaMax;
                    FoilAnalysis.vInc = m_Reader.m_aoaInc;
                    break;
                }
                case xfl::FIXEDAOAPOLAR:
                {
                    pPolar = Objects2d::createPolar(pFoil, m_Reader.m_PolarType, m_Reader.m_Alpha.at(ip), m_Reader.m_Mach.at(ip),
                                                           m_Reader.m_NCrit.at(ip), m_Reader.m_XtrTop, m_Reader.m_XtrBot);

                    FoilAnalysis.vMin = m_Reader.m_ReMin;
                    FoilAnalysis.vMax = m_Reader.m_ReMax;
                    FoilAnalysis.vInc = m_Reader.m_ReInc;
                    break;
                }
                default: break;
            }

            if(pPolar && pPolar->m_Reynolds<0.1)
            {
                traceLog("   skipping polar "+pPolar->polarName()+": Reynolds number is null\n");
                Objects2d::deletePolar(pPolar);
                continue;
            }

            if(pFoil && pPolar)
            {
                FoilAnalysis.pFoil = pFoil;
                FoilAnalysis.pPolar = pPolar;
                m_FoilExecList.append(FoilAnalysis);
                traceLog("   added analysis for foil "+pFoil->name()+" and "+pPolar->polarName() +"\n");
            }
        }
    }
}


bool XflScriptExec::makeFoils()
{
    traceLog("Reading foil files\n");
    QStringList filters =  {"*.dat"};
    for(int ifo=0; ifo<m_Reader.m_FoilList.count(); ifo++)
    {
        QFile datFile;
        QString datPathName;

        if (!xfl::findFile(m_Reader.m_FoilList.at(ifo), m_Reader.datFoilDirPath(), filters, true, datPathName))
        {
            QString strange = "   ...failed to find the file "+m_Reader.m_FoilList.at(ifo);
            traceLog(strange+"\n");
            return false;
        }

        datFile.setFileName(datPathName);
        if (!datFile.open(QIODevice::ReadOnly))
        {
            QString strange = "   ...Could not open the file "+m_Reader.m_FoilList.at(ifo);
            traceLog(strange+"\n");
            return false;
        }
        else
        {
            Foil *pFoil = xfl::readFoilFile(datFile);
            if(pFoil)
            {
                pFoil->setLineWidth(2);

                Objects2d::insertThisFoil(pFoil);
                m_oaFoil.append(pFoil);
                traceLog("   added foil: "+pFoil->name()+"\n");
            }
            else
            {
                traceLog("   ...failed to add the foil from "+m_Reader.m_FoilList.at(ifo)+"\n");
                return false;
            }
        }
    }
    traceLog("\n");

    return true;
}


Polar* XflScriptExec::makePolarFromXml(QString const &fileName)
{
    QString pathName = m_Reader.xmlAnalysisDirPath()+QDir::separator()+fileName;
    QFile xmlFile(pathName);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not read the file"+pathName;
        traceLog(strange);
        return nullptr;
    }
    else
    {
        Polar *pPolar = new Polar;
        XmlPolarReader xpReader(xmlFile, pPolar);
        xpReader.readXMLPolarFile();
        Objects2d::appendPolar(pPolar);
        return pPolar;
    }
}


bool XflScriptExec::setLogFile()
{
    m_LogFileName = m_OutputPath + "/script_"+QTime::currentTime().toString("hhmmss")+".log";

    m_pXFile = new QFile(m_LogFileName);
    if(!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        traceLog("Could not create the log file... aborting\n");
        return false;
    }
    m_OutLogStream.setDevice(m_pXFile);
    m_OutLogStream << "\n";
    m_OutLogStream << s_VersionName;
    m_OutLogStream << "\n";
    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
    m_OutLogStream << str<<"\n";

    m_OutLogStream.flush();
    return true;
}


void XflScriptExec::traceLog(const QString &strMsg)
{
    if(m_bStdOutStream)
    {
        std::cout<<strMsg.toStdString();
    }

    if(m_OutLogStream.device())
    {
        m_OutLogStream << strMsg;
        m_OutLogStream.flush();
    }
    emit msgUpdate(strMsg);
}


bool XflScriptExec::readScript(const QString &scriptpathname)
{
    QFile xmlFile(scriptpathname);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not read the file"+scriptpathname;
        traceLog(strange);
        return false;
    }

    m_Reader.setDevice(&xmlFile);
    m_Reader.readScript();
    if(m_Reader.hasError())
    {
        QString strange;
        strange = QString::asprintf("\nline %d column %d", int(m_Reader.lineNumber()), int(m_Reader.columnNumber()));
        QString errorMsg = m_Reader.errorString() + strange;
        traceLog(errorMsg);
    }
    else
    {
    }
    return true;
}


QString XflScriptExec::projectFilePathName() const
{
    QString projectFileName = m_Reader.projectFileName();
    QFileInfo fi(projectFileName);

    return m_OutputPath + QDir::separator() + fi.fileName();
}


void XflScriptExec::closeLogFile()
{
    m_OutLogStream.setDevice(nullptr);
    if(m_pXFile)
    {
        m_pXFile->close(); delete m_pXFile; m_pXFile=nullptr;
    }
}


void XflScriptExec::onCancel()
{
    m_bCancel=true;
    XFoilTask::cancelTask();

    traceLog("\n_____________Analysis cancellation request received_____________\n\n");
    emit cancelTask();
}


bool XflScriptExec::makeExportDirectories()
{
    bool bOK = true;
    QString projectFileName = m_Reader.projectFileName();
    QFileInfo fi(projectFileName);

    m_OutputPath = m_Reader.outputDirPath();
    m_OutputPath += QDir::separator() + fi.baseName();


    QDir outputDir(m_OutputPath);
    if(!outputDir.exists())
    {
        if(!outputDir.mkpath(m_OutputPath))
        {
            traceLog("Could not make the directory: "+m_OutputPath+"\n");
            bOK = false;
        }
    }

    QString FoilPolarsBinPath = m_Reader.binPolarDirPath();
    QDir FoilPolarsBinDir(FoilPolarsBinPath);
    if(!FoilPolarsBinDir.exists())
    {
        if(!FoilPolarsBinDir.mkpath(FoilPolarsBinPath))
        {
            traceLog("Could not make the directory: "+FoilPolarsBinPath+"\n");
            bOK = false;
        }
    }

    QString XFoilPolarsTextPath = m_Reader.xfoilPolarDirPath();
    QDir exportFoilPolarsDir(XFoilPolarsTextPath);
    if(!exportFoilPolarsDir.exists())
    {
        if(!exportFoilPolarsDir.mkpath(XFoilPolarsTextPath))
        {
            traceLog("Could not make the directory: "+XFoilPolarsTextPath+"\n");
            bOK = false;
        }
    }

    return bOK;
}


bool XflScriptExec::runScript()
{
    XFoilTask::setCancelled(false);

    if(!makeExportDirectories())
    {
        traceLog("Error making directories ...aborting\n");
        return false;
    }

    if(!setLogFile()) return false;

    if(m_bCancel) return false;

    makeFoils();
    if(m_bCancel) return false;

    makeFoilAnalysisList();
    if(m_bCancel) return false;

    if(m_FoilExecList.size())
    {
        runFoilAnalyses();
    }
    else traceLog("No Foil analysis requested\n\n");


    traceLog("Finished script successfully\n");
    return true;
}


void XflScriptExec::runFoilAnalyses()
{
    QString strong;

    //	QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms
    traceLog("\n\n");

    strong = "_____Starting foil analyses_____\n\n";
    traceLog(strong);

    m_nThreads = m_Reader.m_nMaxThreads;
    QThreadPool::globalInstance()->setMaxThreadCount(m_nThreads);
    strong = QString::asprintf("Running with %d thread(s)\n", m_Reader.m_nMaxThreads);
    traceLog(strong+"\n");

    //build an array of all analysis pairs to run
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    strong = QString::asprintf("Found %d (foil, polar) pairs to analyze.\n",m_FoilExecList.size());
    traceLog(strong+"\n");

    XFoilTask::s_bCancel = false;

    for(int i=0; i<m_FoilExecList.size(); i++)
    {
        XFoilTask *pXFoilTask = new XFoilTask(this);

        //take the last analysis in the array
         FoilAnalysis &Analysis = m_FoilExecList[i];

        //initiate the task
        pXFoilTask->initializeTask(Analysis, true, true, false);
        if(Analysis.pPolar->polarType()<xfl::FIXEDAOAPOLAR)
            pXFoilTask->setSequence(true, Analysis.vMin, Analysis.vMax, Analysis.vInc);
        else if(Analysis.pPolar->isFixedaoaPolar())
            pXFoilTask->setReRange(Analysis.vMin, Analysis.vMax, Analysis.vInc);

        //launch it
        m_nTaskStarted++;
        strong = "Starting "+Analysis.pFoil->name()+" / "+Analysis.pPolar->polarName()+"\n";
        traceLog(strong);

        QThreadPool::globalInstance()->start(pXFoilTask);
    }

    QThreadPool::globalInstance()->waitForDone();

    // leave things as they were
    XFoil::s_bCancel = false;
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    if(m_bCancel) strong = "\n_____Foil analysis cancelled_____\n";
    else          strong = "\n_____Foil analysis completed_____\n";
    traceLog(strong);
}


/** Useless: thread does not exit the runFoilAnalyses() method until all tasks are done*/
void XflScriptExec::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == XFOIL_END_TASK_EVENT)
    {
        // Now we can safely do something with our Qt objects.
        m_nTaskDone++; //one down, more to go

        XFoilTaskEvent *pXFEvent = dynamic_cast<XFoilTaskEvent*>(pEvent);
        QString str = "   ...Finished "+ (pXFEvent->foil())->name()+" / "
                +(pXFEvent->polar())->polarName()+"\n";

        traceLog(str);
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
//        Objects2d::addOpPoint(pOppEvent->foilPtr(), pOppEvent->polarPtr(), pOppEvent->oppPtr(), false);
        if(OpPoint::bStoreOpp()) Objects2d::insertOpPoint(pOppEvent->theOpPoint()); // OpPoint data is added to the polar data on the fly in the XFoilTask
        else                      delete pOppEvent->theOpPoint();
    }
}


