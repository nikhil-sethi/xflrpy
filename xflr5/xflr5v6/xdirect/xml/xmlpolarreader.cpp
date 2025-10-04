/****************************************************************************

    XMLPolarReader Class
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

#include <QDebug>

#include <xflcore/xflcore.h>
#include "xmlpolarreader.h"
#include <xflobjects/objects2d/polar.h>

XmlPolarReader::XmlPolarReader(QFile &file, Polar *pPolar)
{
    m_pPolar = pPolar;
    setDevice(&file);

}


bool XmlPolarReader::readXMLPolarFile()
{
    if (readNextStartElement())
    {
        if (name() == "Foil_Polar" && attributes().value("version") == "1.0")
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("Polar"), Qt::CaseInsensitive)==0)
                {
                    readPolar(m_pPolar);
                }
                else
                    skipCurrentElement();
            }
        }
        else
            raiseError(QObject::tr("The file is not an xflr5 polar version 1.0 file."));
    }
    return(hasError());
}


void XmlPolarReader::readPolar(Polar *pPolar)
{

    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("polar_name"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setPolarName(readElementText());
        }
        else if (name().toString().compare(QString("foil_name"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setFoilName(readElementText());
        }
        else if (name().toString().compare(QString("type"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setPolarType(xfl::polarType(readElementText()));
        }
        else if (name().compare(QString("Fixed_Reynolds"),         Qt::CaseInsensitive)==0)
        {
            pPolar->setReynolds(readElementText().toDouble());
        }
        else if (name().compare(QString("Fixed_AOA"),         Qt::CaseInsensitive)==0)
        {
            pPolar->setAoa(readElementText().toDouble());
        }
        else if (name().toString().compare(QString("Forced_Top_Transition"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setXtrTop(readElementText().toDouble());
        }
        else if (name().toString().compare(QString("Forced_Bottom_Transition"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setXtrBot(readElementText().toDouble());
        }
        else if (name().toString().compare(QString("Reynolds_Type"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setReType(readElementText().toInt());
        }
        else if (name().toString().compare(QString("Mach_Type"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setMaType(readElementText().toInt());
        }
        else if (name().toString().compare(QString("NCrit"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setNCrit(readElementText().toDouble());
        }
        else
            skipCurrentElement();

    }

}

