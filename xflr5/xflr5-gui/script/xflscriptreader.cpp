/****************************************************************************

    XFLScriptReader Class
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

#include <QDebug>

#include "xflscriptreader.h"
#include <misc/options/units.h>
#include <globals/globals.h>


XFLScriptReader::XFLScriptReader()
{
    m_planeList.clear();
    m_wPolarList.clear();
    m_OutputDirectoryPath.clear();
    m_projectFileName.clear();
    alphaMin = alphaMax = alphaInc = 0.0;
    qinfMin = qinfMax = qinfInc = 0.0;

    aoaMin = aoaMax = aoaInc = 0.0;
    VInfMin = VInfMax =VInfInc = 0.0;
    betaMin = betaMax = betaInc = 0.0;
    ctrlMin = ctrlMax = ctrlInc = 0.0;

    m_bMakeXfl = m_bMakePOpps = m_bMultiThreading = false;
    m_nMaxThreads = 1;
}


bool XFLScriptReader::readScript()
{
    // level 0
    if (readNextStartElement())
    {
        if (name().compare(QString("XFLR5_SCRIPT"), Qt::CaseInsensitive)!=0 || attributes().value("version") < "1.0")
        {
            raiseError(QObject::tr("The file is not an xflr5 readable script"));
            return false;
        }
    }

    // level 1
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("input"), Qt::CaseInsensitive)==0)
        {
            readInput();
        }
        else if (name().compare(QString("output"), Qt::CaseInsensitive)==0)
        {
            readOutput();
        }
        else if (name().compare(QString("options"), Qt::CaseInsensitive)==0)
        {
            readOptions();
        }
        else
            skipCurrentElement();
    }


    return(hasError());
}


bool XFLScriptReader::readInput()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Foils"), Qt::CaseInsensitive)==0)
        {
            readFoils();
        }
        else if(name().compare(QString("Foil_Polars"), Qt::CaseInsensitive)==0)
        {
            readFoilPolars();
        }
        else if(name().compare(QString("Planes"), Qt::CaseInsensitive)==0)
        {
            readPlanes();
        }
        else if(name().compare(QString("Plane_Polars"), Qt::CaseInsensitive)==0)
        {
            readPlanePolars();
        }
        else if(name().compare(QString("Foil_Analysis"), Qt::CaseInsensitive)==0)
        {
            readFoilAnalysis();
        }
        else if(name().compare(QString("Plane_Analysis"), Qt::CaseInsensitive)==0)
        {
            readPlaneAnalysis();
        }

        else
            skipCurrentElement();
    }
    return true;
}



bool XFLScriptReader::readOutput()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("Output_Directory"), Qt::CaseInsensitive)==0)
        {
            m_OutputDirectoryPath = readElementText();
        }
        else if(name().compare(QString("make_xfl_project_file"), Qt::CaseInsensitive)==0)
        {
            m_bMakeXfl = stringToBool(readElementText());
        }
        else if(name().compare(QString("project_file_name"), Qt::CaseInsensitive)==0)
        {
            m_projectFileName = readElementText();
        }
        else if(name().compare(QString("make_oppoints"), Qt::CaseInsensitive)==0)
        {
            m_bMakePOpps = stringToBool(readElementText());
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XFLScriptReader::readOptions()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        //level 2
        if(name().compare(QString("use_multithreading"), Qt::CaseInsensitive)==0)
        {
            m_bMultiThreading = stringToBool(readElementText());
        }
        else if(name().compare(QString("max_threads"), Qt::CaseInsensitive)==0)
        {
            m_nMaxThreads = readElementText().toInt();
        }
        else
            skipCurrentElement();
    }
    return true;
}


bool XFLScriptReader::readFoils()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Input_Directory_Foil_Files"), Qt::CaseInsensitive)==0)
        {
            m_InputFoilDirectoryPath = readElementText();
        }
        else if (name().compare(QString("Foil_Files"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Foil_File_Name"), Qt::CaseInsensitive)==0)
                {
                    m_foilList.append(readElementText());
                }
                else
                    skipCurrentElement();
            }
        }
    }
    return true;
}


bool XFLScriptReader::readFoilPolars()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Input_Directory_Foil_Polar_Files"), Qt::CaseInsensitive)==0)
        {
            m_InputPolarDirectoryPath = readElementText();
        }
        else if (name().compare(QString("Foil_Polar_Files"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Polar_File_Name"), Qt::CaseInsensitive)==0)
                {
                    m_polarList.append(readElementText());
                }
                else
                    skipCurrentElement();
            }
        }
    }
    return true;
}


bool XFLScriptReader::readPlanes()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Input_Directory_Plane_Files"), Qt::CaseInsensitive)==0)
        {
            m_InputPlaneDirectoryPath = readElementText();
        }
        else if (name().compare(QString("Plane_Files"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Plane_File_Name"), Qt::CaseInsensitive)==0)
                {
                    m_planeList.append(readElementText());
                }
                else
                    skipCurrentElement();
            }
        }
    }
    return true;
}



bool XFLScriptReader::readPlanePolars()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Input_Directory_Plane_Polar_Files"), Qt::CaseInsensitive)==0)
        {
            m_InputWPolarDirectoryPath = readElementText();
        }
        else if (name().compare(QString("Plane_Polar_Files"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Polar_File_Name"), Qt::CaseInsensitive)==0)
                {
                    m_wPolarList.append(readElementText());
                }
                else
                    skipCurrentElement();
            }
        }
    }
    return true;
}



bool XFLScriptReader::readPlaneAnalysis()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            QStringList alphaList = readElementText().split(",");
            if(alphaList.length()>0) aoaMin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) aoaMax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) aoaInc = alphaList.at(2).toDouble();
        }
        else if(name().compare(QString("Beta"), Qt::CaseInsensitive)==0)
        {
            QStringList betaList = readElementText().split(",");
            if(betaList.length()>0) betaMin = betaList.at(0).toDouble();
            if(betaList.length()>1) betaMax = betaList.at(1).toDouble();
            if(betaList.length()>2) betaInc = betaList.at(2).toDouble();
        }
        else if(name().compare(QString("Velocity"), Qt::CaseInsensitive)==0)
        {
            QStringList qinfList = readElementText().split(",");
            if(qinfList.length()>0) VInfMin = qinfList.at(0).toDouble();
            if(qinfList.length()>1) VInfMax = qinfList.at(1).toDouble();
            if(qinfList.length()>2) VInfInc = qinfList.at(2).toDouble();
        }
        else if(name().compare(QString("Control"), Qt::CaseInsensitive)==0)
        {
            QStringList ctrlList = readElementText().split(",");
            if(ctrlList.length()>0) ctrlMin = ctrlList.at(0).toDouble();
            if(ctrlList.length()>1) ctrlMax = ctrlList.at(1).toDouble();
            if(ctrlList.length()>2) ctrlInc = ctrlList.at(2).toDouble();
        }
        else
            skipCurrentElement();
    }

    return true;
}



bool XFLScriptReader::readFoilAnalysis()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().compare(QString("Alpha"), Qt::CaseInsensitive)==0)
        {
            QStringList alphaList = readElementText().split(" ");
            if(alphaList.length()>0) alphaMin = alphaList.at(0).toDouble();
            if(alphaList.length()>1) alphaMax = alphaList.at(1).toDouble();
            if(alphaList.length()>2) alphaInc = alphaList.at(2).toDouble();
        }
        else if(name().compare(QString("Forced_Top_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrTop = readElementText().toDouble();
        }
        else if(name().compare(QString("Forced_Bottom_Transition"), Qt::CaseInsensitive)==0)
        {
            m_XtrBot = readElementText().toDouble();
        }
        else if(name().compare(QString("Reynolds"), Qt::CaseInsensitive)==0)
        {
            QStringList ReList = readElementText().split(" ");
            for(int ir=0; ir<ReList.count();ir++)
            {
                if(ReList.at(ir).length()>0) m_Reynolds.append(ReList.at(ir).toDouble());
            }
        }
        else if(name().compare(QString("NCrit"), Qt::CaseInsensitive)==0)
        {
            QStringList NCritList = readElementText().split(" ");
            for(int ic=0; ic<NCritList.count();ic++)
            {
                if(NCritList.at(ic).length()>0) m_NCrit.append(NCritList.at(ic).toDouble());
            }
        }
        else if(name().compare(QString("Mach"), Qt::CaseInsensitive)==0)
        {
            QStringList MachList = readElementText().split(" ");
            for(int im=0; im<MachList.count();im++)
            {
                if(MachList.at(im).length()>0) m_Mach.append(MachList.at(im).toDouble());
            }
        }
        else
            skipCurrentElement();
    }

    return true;
}






















