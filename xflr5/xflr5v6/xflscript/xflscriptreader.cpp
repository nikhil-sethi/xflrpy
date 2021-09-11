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

#include <QDir>
#include <QDebug>
#include <QDateTime>

#include "xflscriptreader.h"
#include <xflcore/units.h>
#include <xflcore/xflcore.h>


XFLScriptReader::XFLScriptReader()
{
    m_OutputDirectoryPath.clear();
    m_ProjectFileName =  "script_"+QDateTime::currentDateTime().toString("yyMMdd_hhmmss")+".xfl";

    m_MaxXFoilIterations = 100;
    m_NFoilPanels = 100;
    m_bRepanelFoils = false;

    m_bcsvPolarOutput = true;
    m_XtrTop = m_XtrBot = 1.0;
    m_aoaMin = m_aoaMax = 0.0;
    m_aoaInc = 0.5;
    m_ReMin  = m_ReMax  = 0.0;
    m_ReInc  = 10000;
    m_ClMin  = m_ClMax  = 0.0;
    m_ClInc  = 0.1;
    m_bMakeOpps  = false;
    m_bAlphaSpec = true;
    m_bFromZero  = true;
    m_bRunAllFoilAnalyses = true;

    m_bOutputPolarsBin = m_bOutputPolarsText = m_bMakeProjectFile = false;
    m_bRecursiveDirScan = true;
    m_bMakeXfl = m_bMakePOpps = m_bMultiThreading = false;
    m_nMaxThreads = 1;
    m_ThreadPriority=QThread::NormalPriority;
}


bool XFLScriptReader::readScript()
{
    // level 0
    if (readNextStartElement())
    {
        if (name().compare(QString("xflscript"), Qt::CaseInsensitive)!=0 || attributes().value("version") < "1.0")
        {
            raiseError(QObject::tr("The file is not an xflr5 readable script"));
            return false;
        }
    }

    // level 1
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Metadata"), Qt::CaseInsensitive)==0)
        {
            readMetaData();
        }
        else if (name().compare(QString("foil_analysis"), Qt::CaseInsensitive)==0)
        {
            readFoilData();
        }
        else
            skipCurrentElement();
    }


    return(hasError());
}



bool XFLScriptReader::readMetaData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("make_project_file"), Qt::CaseInsensitive)==0)
        {
            m_bMakeProjectFile = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("project_file_name"), Qt::CaseInsensitive)==0)
        {
            m_ProjectFileName = readElementText().trimmed();
        }
        else if(name().compare(QString("Directories"), Qt::CaseInsensitive)==0)
        {
            readDirectoryData();
        }
        else if(name().compare(QString("polar_text_output_format"), Qt::CaseInsensitive)==0)
        {
            m_bcsvPolarOutput = readElementText().trimmed().compare(QString("csv"), Qt::CaseInsensitive)==0;
        }
        else if(name().compare(QString("MultiThreading"), Qt::CaseInsensitive)==0)
        {
            readThreadingOptions();
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XFLScriptReader::readDirectoryData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("output_dir"), Qt::CaseInsensitive)==0)
        {
            m_OutputDirPath = readElementText().trimmed();
            if(m_OutputDirPath.endsWith(QDir::separator())) m_OutputDirPath.remove(m_OutputDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if (name().compare(QString("foil_files_dir"), Qt::CaseInsensitive)==0)
        {
            m_datFoilDirPath = readElementText().trimmed();
            if(m_datFoilDirPath.endsWith(QDir::separator())) m_datFoilDirPath.remove(m_datFoilDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("foil_analysis_xml_dir"), Qt::CaseInsensitive)==0)
        {
            m_xmlAnalysisDirPath = readElementText().trimmed();
            if(m_xmlAnalysisDirPath.endsWith(QDir::separator())) m_xmlAnalysisDirPath.remove(m_xmlAnalysisDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("foil_polars_dir"), Qt::CaseInsensitive)==0)
        {
            m_PolarBinDirPath = readElementText().trimmed();
            if(m_PolarBinDirPath.endsWith(QDir::separator())) m_PolarBinDirPath.remove(m_PolarBinDirPath.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("xfoil_polars_dir"), Qt::CaseInsensitive)==0)
        {
            m_XFoilPolarsDir = readElementText().trimmed();
            if(m_XFoilPolarsDir.endsWith(QDir::separator())) m_XFoilPolarsDir.remove(m_XFoilPolarsDir.lastIndexOf(QDir::separator()), 1);
        }
        else if(name().compare(QString("recursive_scan"), Qt::CaseInsensitive)==0)
        {
            m_bRecursiveDirScan = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }

    return !hasError();
}

bool XFLScriptReader::readFoilData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Foil_Files"), Qt::CaseInsensitive)==0)
        {
            readFoils();
        }
        else if(name().compare(QString("Analysis_Files"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisFiles();
        }
        else if(name().compare(QString("Batch_Analysis_Data"), Qt::CaseInsensitive)==0)
        {
            readBatchAnalysisData();
        }
        else if(name().compare(QString("OpPoint_Range"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisRange();
        }
        else if(name().compare(QString("Output"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisOutput();
        }
        else if(name().compare(QString("Options"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysisOptions();
        }
        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XFLScriptReader::readFoils()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Foil_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_FoilList.append(readElementText());
        }
        else
            skipCurrentElement();
    }

    return true;
}


bool XFLScriptReader::readThreadingOptions()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Allow_Multithreading"), Qt::CaseInsensitive)==0)
        {
            m_bMultiThreading = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("Thread_Priority"), Qt::CaseInsensitive)==0)
        {
            QString priority = readElementText().trimmed();
            if     (priority.compare("Idle",         Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::IdlePriority;
            else if(priority.compare("Lowest",       Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::LowestPriority;
            else if(priority.compare("Low",          Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::LowPriority;
            else if(priority.compare("Normal",       Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::NormalPriority;
            else if(priority.compare("High",         Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::HighPriority;
            else if(priority.compare("Highest",      Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::HighestPriority;
            else if(priority.compare("TimeCritical", Qt::CaseInsensitive)==0) m_ThreadPriority=QThread::TimeCriticalPriority;
            else                                                              m_ThreadPriority=QThread::NormalPriority;
        }
        else if(name().compare(QString("max_threads"), Qt::CaseInsensitive)==0)
        {
            m_nMaxThreads = readElementText().trimmed().toInt();
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XFLScriptReader::readFoilAnalysisFiles()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Process_All_Files"), Qt::CaseInsensitive)==0)
        {
            m_bRunAllFoilAnalyses = xfl::stringToBool(readElementText().trimmed());
        }
        else if(name().compare(QString("Analysis_File_Name"), Qt::CaseInsensitive)==0)
        {
            m_XmlFoilAnalysisList.push_back(readElementText().trimmed());
        }

        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XFLScriptReader::readFoilAnalysisRange()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) m_aoaMin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) m_aoaMax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) m_aoaInc = alphaList.at(2).toDouble();
        }
        else if(name().compare(QString("Cl"), Qt::CaseInsensitive)==0)
        {
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) m_ClMin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) m_ClMax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) m_ClInc = alphaList.at(2).toDouble();
        }
        else if(name().compare(QString("Reynolds"), Qt::CaseInsensitive)==0)
        {
            QStringList alphaList = readElementText().simplified().split(",");
            if(alphaList.length()>0) m_ReMin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) m_ReMax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) m_ReInc = alphaList.at(2).toDouble();
        }
        else if(name().compare(QString("Spec_Alpha"), Qt::CaseInsensitive)==0)
        {
            m_bAlphaSpec  = xfl::stringToBool(readElementText().trimmed());
        }
        else if(name().compare(QString("From_Zero"), Qt::CaseInsensitive)==0)
        {
            m_bFromZero  = xfl::stringToBool(readElementText().trimmed());
        }
        else
            skipCurrentElement();
    }
    return(!hasError());
}


bool XFLScriptReader::readFoilAnalysisOptions()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Max_XFoil_Iterations"), Qt::CaseInsensitive)==0)
        {
            m_MaxXFoilIterations  = readElementText().trimmed().toInt();
        }
        else if(name().compare(QString("Repanel_Foils"), Qt::CaseInsensitive)==0)
        {
            m_bRepanelFoils  = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("Foil_Panels"), Qt::CaseInsensitive)==0)
        {
            m_NFoilPanels = readElementText().trimmed().toInt();
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XFLScriptReader::readFoilAnalysisOutput()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("make_polars_bin_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputPolarsBin = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_polars_text_file"), Qt::CaseInsensitive)==0)
        {
            m_bOutputPolarsText = xfl::stringToBool(readElementText());
        }
        else if(name().compare(QString("make_oppoints"), Qt::CaseInsensitive)==0)
        {
            m_bMakeOpps = xfl::stringToBool(readElementText());
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XFLScriptReader::readBatchAnalysisData()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Polar_Type"), Qt::CaseInsensitive)==0)
        {
            m_PolarType  = xfl::polarType(readElementText());
        }
        else if(name().compare(QString("Forced_Top_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrTop = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Forced_Bottom_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrBot = readElementText().trimmed().toDouble();
        }
        else if(name().compare(QString("Batch_Range"), Qt::CaseInsensitive)==0)
        {
            readFoilBatchRange();
        }
        else
            skipCurrentElement();
    }
    for(int ic=m_NCrit.size(); ic<m_Reynolds.size() ;ic++)
    {
        m_NCrit.push_back(9);
    }
    for(int ic=m_Mach.size(); ic<m_Reynolds.size() ;ic++)
    {
        m_Mach.push_back(0.0);
    }

    if(m_NCrit.size()>m_Reynolds.size()) m_NCrit.resize(m_Reynolds.size());
    if(m_Mach.size()>m_Reynolds.size())  m_Mach.resize(m_Reynolds.size());

    return !hasError();
}


bool XFLScriptReader::readFoilBatchRange()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            m_Alpha.clear();
            QStringList AoaList = readElementText().simplified().split(",");
            for(int ir=0; ir<AoaList.count(); ir++)
            {
                if(AoaList.at(ir).length()>0) m_Alpha.append(AoaList.at(ir).toDouble());
            }
        }
        else if(name().compare(QString("Reynolds"), Qt::CaseInsensitive)==0)
        {
            m_Reynolds.clear();
            QStringList ReList = readElementText().simplified().split(",");
            for(int ir=0; ir<ReList.count(); ir++)
            {
                if(ReList.at(ir).length()>0) m_Reynolds.append(ReList.at(ir).toDouble());
            }
        }
        else if(name().compare(QString("NCrit"), Qt::CaseInsensitive)==0)
        {
            m_NCrit.clear();
            QStringList NCritList = readElementText().simplified().split(",");
            for(int ic=0; ic<NCritList.count(); ic++)
            {
                if(NCritList.at(ic).length()>0) m_NCrit.append(NCritList.at(ic).toDouble());
            }
        }
        else if(name().compare(QString("Mach"), Qt::CaseInsensitive)==0)
        {
            m_Mach.clear();
            QStringList MachList = readElementText().simplified().split(",");
            for(int im=0; im<MachList.count(); im++)
            {
                if(MachList.at(im).length()>0) m_Mach.append(MachList.at(im).toDouble());
            }
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}




