/****************************************************************************

    XMLPolarWriter Class
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

#include "xmlpolarwriter.h"
#include <xflcore/xflcore.h>
#include <xflobjects/objects2d/polar.h>

XmlPolarWriter::XmlPolarWriter(QFile &XFile)
{
    setDevice(&XFile);
}


void XmlPolarWriter::writeXMLPolar(Polar *pPolar)
{
    if(!pPolar) return;
    QString strange;

    writeHeader();

    writeStartElement("Polar");
    {
        writeTextElement("Polar_Name", pPolar->polarName());
        writeTextElement("Foil_Name", pPolar->foilName());
        writeTextElement("Type",   polarType(pPolar->polarType()));
        writeTextElement("Fixed_Reynolds",           QString("%1").arg(pPolar->Reynolds(),11,'f',0));
        writeTextElement("Fixed_AOA",                QString("%1").arg(pPolar->aoa(),11,'f',3));
        writeTextElement("Mach",                     QString("%1").arg(pPolar->Mach(),7,'f', 2));
        writeTextElement("ReType",                   QString("%1").arg(pPolar->ReType()));
        writeTextElement("MaType",                   QString("%1").arg(pPolar->MaType()));
        writeTextElement("NCrit",                    QString("%1").arg(pPolar->NCrit(),7,'f', 1));
        writeTextElement("Forced_Top_Transition",    QString("%1").arg(pPolar->XtrTop(),7,'f', 2));
        writeTextElement("Forced_Bottom_Transition", QString("%1").arg(pPolar->XtrBot(),7,'f', 2));
    }
    writeEndElement();
    writeEndDocument();

}


void XmlPolarWriter::writeHeader()
{
    setAutoFormatting(true);

    writeStartDocument();
    writeDTD("<!DOCTYPE Foil_Polar>");
    writeStartElement("Foil_Polar");
    writeAttribute("version", "1.0");
}
