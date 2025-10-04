/****************************************************************************

    XMLWPolarWriter Class
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

#include "xmlwpolarwriter.h"
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/wpolar.h>

XmlWPolarWriter::XmlWPolarWriter(QFile &XFile)
{
    setDevice(&XFile);
}

void XmlWPolarWriter::writeHeader()
{
    setAutoFormatting(true);

    writeStartDocument();
    writeDTD("<!DOCTYPE Plane_Polar>");
    writeStartElement("Plane_Polar");
    writeAttribute("version", "1.0");
    writeStartElement("Units");
    {
        writeTextElement("length_unit_to_meter", QString("%1").arg(1./Units::mtoUnit()));
        writeTextElement("area_unit_to_m2",      QString("%1").arg(1./Units::m2toUnit()));
        writeTextElement("mass_unit_to_kg",      QString("%1").arg(1./Units::kgtoUnit()));
        writeTextElement("speed_unit_to_ms",     QString("%1").arg(1./Units::mstoUnit()));
        writeTextElement("inertia_unit_to_kgm2", QString("%1").arg(1./Units::kgm2toUnit()));
    }
    writeEndElement();
}



void XmlWPolarWriter::writeXMLWPolar(WPolar *pWPolar)
{
    if(!pWPolar) return;
    QString strange;

    writeHeader();

    writeStartElement("Polar");
    {
        writeTextElement("Polar_Name", pWPolar->polarName());
        writeTextElement("Plane_Name", pWPolar->planeName());
        writeTextElement("Type",   WPolarType(pWPolar->polarType()));
        writeTextElement("Method", analysisMethod(pWPolar->analysisMethod()));
        writeTextElement("Ignore_Body_Panels", pWPolar->bIgnoreBodyPanels() ?  "true" : "false");
        writeTextElement("Use_VLM1",           pWPolar->bVLM1() ?              "true" : "false");
        writeTextElement("Viscous_Analysis",   pWPolar->bViscous() ? "true" : "false");
        writeTextElement("Thin_Surfaces",      pWPolar->bThinSurfaces() ?      "true" : "false");
        writeTextElement("Tilted_Analysis",    pWPolar->bTilted() ?            "true" : "false");
        writeTextElement("Ground_Effect",      pWPolar->bGround() ?            "true" : "false");
        writeTextElement("Ground_Height",      QString("%1").arg(pWPolar->groundHeight()*Units::mtoUnit(),11,'f',5));
        writeTextElement("Viscosity",          QString("%1").arg(pWPolar->viscosity(),11,'g',5));
        writeTextElement("Density",            QString("%1").arg(pWPolar->density(),11,'f',5));
        writeTextElement("Fixed_Velocity",     QString("%1").arg(pWPolar->velocity()*Units::mstoUnit(),11,'f',5));
        writeTextElement("Fixed_AOA",          QString("%1").arg(pWPolar->Alpha(),11,'f',5));
        writeTextElement("Fixed_Bank_Angle",   QString("%1").arg(pWPolar->Phi(),11,'f',5));
        writeTextElement("Fixed_SideSlip",     QString("%1").arg(pWPolar->Beta(),11,'f',5));
        writeTextElement("Reference_Dimensions",   referenceDimension(pWPolar->referenceDim()));
        writeTextElement("Reference_Area",         QString("%1").arg(pWPolar->referenceArea()*Units::m2toUnit(),11,'f',5));
        writeTextElement("Reference_Span_Length",  QString("%1").arg(pWPolar->referenceSpanLength()*Units::mtoUnit(),11,'f',5));
        writeTextElement("Reference_Chord_Length", QString("%1").arg(pWPolar->referenceChordLength()*Units::mtoUnit(),11,'f',5));

        writeStartElement("ExtraDrag");
        {
            for(int iex=0; iex<MAXEXTRADRAG; iex++)
            {
                writeTextElement(QString("ExtraDragCoef_%1").arg(iex+1), QString("%1").arg(pWPolar->m_ExtraDragCoef[iex], 11,'f',5));
                writeTextElement(QString("ExtraDragArea_%1").arg(iex+1), QString("%1").arg(pWPolar->m_ExtraDragArea[iex], 11,'f',5));
            }
        }
        writeEndElement();

        writeTextElement("Use_Plane_Inertia",  pWPolar->bAutoInertia() ?  "true" : "false");

        writeStartElement("Inertia_Value");
        {
            writeTextElement("Mass", QString("%1").arg(pWPolar->mass()*Units::kgtoUnit(),11,'f',5));
            strange = QString::asprintf("%11.5g %11.5g %11.5g", pWPolar->CoG().x*Units::mtoUnit(), pWPolar->CoG().y*Units::mtoUnit(), pWPolar->CoG().z*Units::mtoUnit());
            writeTextElement("CoG", strange);
            writeTextElement("CoG_Ixx", QString("%1").arg(pWPolar->CoGIxx()*Units::kgm2toUnit(), 11, 'f', 5));
            writeTextElement("CoG_Iyy", QString("%1").arg(pWPolar->CoGIyy()*Units::kgm2toUnit(), 11, 'f', 5));
            writeTextElement("CoG_Izz", QString("%1").arg(pWPolar->CoGIzz()*Units::kgm2toUnit(), 11, 'f', 5));
            writeTextElement("CoG_Ixz", QString("%1").arg(pWPolar->CoGIxz()*Units::kgm2toUnit(), 11, 'f', 5));
        }
        writeEndElement();

        writeStartElement("Inertia_gains");
        {
            writeTextElement("Mass_gain",  QString("%1").arg(pWPolar->m_inertiaGain[0]*Units::kgtoUnit(),   11,'f',5));
            writeTextElement("CoG_x_gain", QString("%1").arg(pWPolar->m_inertiaGain[1]*Units::mtoUnit(),    11,'f',5));
            writeTextElement("CoG_z_gain", QString("%1").arg(pWPolar->m_inertiaGain[2]*Units::mtoUnit(),    11,'f',5));
            writeTextElement("Ixx_gain",   QString("%1").arg(pWPolar->m_inertiaGain[3]*Units::kgm2toUnit(), 11,'f',5));
            writeTextElement("Iyy_gain",   QString("%1").arg(pWPolar->m_inertiaGain[4]*Units::kgm2toUnit(), 11,'f',5));
            writeTextElement("Izz_gain",   QString("%1").arg(pWPolar->m_inertiaGain[5]*Units::kgm2toUnit(), 11,'f',5));
            writeTextElement("Ixz_gain",   QString("%1").arg(pWPolar->m_inertiaGain[6]*Units::kgm2toUnit(), 11,'f',5));
        }
        writeEndElement();


        writeStartElement("Angle_gains");
        {
            for(int iag=0; iag<pWPolar->m_nControls; iag++)
            {
                writeTextElement(QString::fromUtf8("Gain_%1").arg(iag+1), QString("%1").arg(pWPolar->m_ControlGain[iag], 11,'f',5));
            }
        }
        writeEndElement();
    }
    writeEndElement();
    writeEndDocument();
}


