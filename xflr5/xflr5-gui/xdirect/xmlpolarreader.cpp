/****************************************************************************

	XMLPolarReader Class
	Copyright (C) 2016-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <QtDebug>
#include <globals.h>
#include "xmlpolarreader.h"

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
			pPolar->polarName() = readElementText();
		}
        else if (name().toString().compare(QString("foil_name"),Qt::CaseInsensitive) ==0)
		{
			pPolar->foilName() = readElementText();
		}
        else if (name().toString().compare(QString("type"),Qt::CaseInsensitive) ==0)
		{
			pPolar->polarType() = polarType(readElementText());
		}
        else if (name().compare(QString("Fixed_Reynolds"),         Qt::CaseInsensitive)==0)
		{
			pPolar->Reynolds() = readElementText().toDouble();
		}
        else if (name().compare(QString("Fixed_AOA"),         Qt::CaseInsensitive)==0)
		{
			pPolar->aoa() = readElementText().toDouble();
		}
        else if (name().toString().compare(QString("Forced_Top_Transition"),Qt::CaseInsensitive) ==0)
		{
			pPolar->XtrTop() = readElementText().toDouble();
		}
        else if (name().toString().compare(QString("Forced_Bottom_Transition"),Qt::CaseInsensitive) ==0)
		{
			pPolar->XtrBot() = readElementText().toDouble();
		}
        else if (name().toString().compare(QString("Reynolds_Type"),Qt::CaseInsensitive) ==0)
		{
			pPolar->ReType() = readElementText().toInt();
		}
        else if (name().toString().compare(QString("Mach_Type"),Qt::CaseInsensitive) ==0)
		{
			pPolar->MaType() = readElementText().toInt();
		}
        else if (name().toString().compare(QString("NCrit"),Qt::CaseInsensitive) ==0)
		{
			pPolar->NCrit() = readElementText().toDouble();
		}
		else
			skipCurrentElement();

	}

}

