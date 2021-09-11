/****************************************************************************

    XmlPlaneWriter Class
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

#include "xmlplanewriter.h"
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xflobjects/objects3d/plane.h>


XMLPlaneWriter::XMLPlaneWriter(QFile &XFile)
{
    setDevice(&XFile);
}

void XMLPlaneWriter::writeHeader()
{
    setAutoFormatting(true);

    writeStartDocument();
    writeDTD("<!DOCTYPE explane>");
    writeStartElement("explane");
    writeAttribute("version", "1.0");


    writeStartElement("Units");
    {
        writeTextElement("length_unit_to_meter", QString("%1").arg(1./Units::mtoUnit()));
        writeTextElement("mass_unit_to_kg", QString("%1").arg(1./Units::kgtoUnit()));
    }
    writeEndElement();

}


void XMLPlaneWriter::writeXMLBody(Body *pBody)
{
    if(!pBody) return;

    writeHeader();

    Vector3d V;
    writeBody(pBody, V, Units::mtoUnit(), Units::kgtoUnit());


    writeEndDocument();
}


void XMLPlaneWriter::writeXMLWing(Wing &wing)
{
    writeHeader();

    Vector3d V;
    writeWing(wing, V, 0);


    writeEndDocument();
}


void XMLPlaneWriter::writeXMLPlane(Plane *m_pPlane)
{
    if(!m_pPlane) return;

    writeHeader();

    writeStartElement("Plane");
    {
        writeTextElement("Name", m_pPlane->name());
        writeTextElement("Description", m_pPlane->description());
        writeStartElement("Inertia");
        {
            for(int ipm=0; ipm<m_pPlane->m_PointMass.size(); ipm++)
            {
                writePointMass(m_pPlane->m_PointMass.at(ipm), Units::kgtoUnit(), Units::mtoUnit());
            }
        }
        writeEndElement();

        writeTextElement("has_body", m_pPlane->body() ? "true" : "false");
        if(m_pPlane->body())
        {
            Body *pBody = m_pPlane->body();
            writeBody(pBody, m_pPlane->bodyPos(), Units::mtoUnit(), Units::kgtoUnit());
        }

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(m_pPlane->wing(iw))
            {
                writeWing(*m_pPlane->wing(iw), m_pPlane->wingLE(iw), m_pPlane->wingTiltAngle(iw));
            }
        }
    }
    writeEndElement();


    writeEndDocument();
}



void XMLPlaneWriter::writeWing(Wing const &wing, Vector3d position, double Ry)
{
    writeStartElement("wing");
    {
        writeTextElement("Name", wing.name());
        writeTextElement("Type",   wingType(wing.wingType()));
        writeColor(wing.color());
        writeTextElement("Description", wing.wingDescription());
        writeTextElement("Position",QString("%1, %2, %3").arg(position.x*Units::mtoUnit(), 11,'g',5)
                         .arg(position.y*Units::mtoUnit(), 11,'g',5)
                         .arg(position.z*Units::mtoUnit(), 11,'g',5));
        writeTextElement("Tilt_angle",  QString("%1").arg(Ry,7,'f',3));
        writeTextElement("Symetric",    wing.isSymetric()  ? "true" : "false");
        writeTextElement("isFin",       wing.isFin()       ? "true" : "false");
        writeTextElement("isDoubleFin", wing.isDoubleFin() ? "true" : "false");
        writeTextElement("isSymFin",    wing.isSymFin()    ? "true" : "false");
        writeStartElement("Inertia");
        {
            writeTextElement("Volume_Mass", QString("%1").arg(wing.volumeMass(),7,'f',3));
            for(int ipm=0; ipm<wing.m_PointMass.size(); ipm++)
            {
                writePointMass(wing.m_PointMass.at(ipm), Units::kgtoUnit(), Units::mtoUnit());
            }
        }
        writeEndElement();
        writeStartElement("Sections");
        {
            for(int ips=0; ips<wing.m_Section.size(); ips++)
            {
                WingSection const&WS = wing.m_Section.at(ips);
                writeStartElement("Section");
                {
                    writeTextElement("y_position",           QString("%1").arg(WS.m_YPosition*Units::mtoUnit(), 7, 'f', 3));
                    writeTextElement("Chord",                QString("%1").arg(WS.m_Chord*Units::mtoUnit(), 7, 'f', 3));
                    writeTextElement("xOffset",              QString("%1").arg(WS.m_Offset*Units::mtoUnit(), 7, 'f', 3));
                    writeTextElement("Dihedral",             QString("%1").arg(WS.m_Dihedral, 7, 'f', 3));
                    writeTextElement("Twist",                QString("%1").arg(WS.m_Twist, 7, 'f', 3));
                    writeTextElement("x_number_of_panels",   QString("%1").arg(WS.m_NXPanels));
                    writeTextElement("x_panel_distribution", distributionType(WS.m_XPanelDist));
                    writeTextElement("y_number_of_panels",   QString("%1").arg(WS.m_NYPanels));
                    writeTextElement("y_panel_distribution", distributionType(WS.m_YPanelDist));
                    writeTextElement("Left_Side_FoilName",   WS.m_LeftFoilName);
                    writeTextElement("Right_Side_FoilName",  WS.m_RightFoilName);
                }
                writeEndElement();
            }
        }
        writeEndElement();
    }
    writeEndElement();

}


void XMLPlaneWriter::writeColor(QColor color)
{
    writeStartElement("Color");
    {
        writeTextElement("red",   QString("%1").arg(color.red()));
        writeTextElement("green", QString("%1").arg(color.green()));
        writeTextElement("blue",  QString("%1").arg(color.blue()));
        writeTextElement("alpha", QString("%1").arg(color.alpha()));
    }
    writeEndElement();
}


void XMLPlaneWriter::writePointMass(PointMass const *ppm, double massUnit, double lengthUnit)
{
    writeStartElement("Point_Mass");
    {
        writeTextElement("Tag", ppm->tag());
        writeTextElement("Mass", QString("%1").arg(ppm->mass()*massUnit,7,'f',3));
        writeTextElement("coordinates",QString("%1, %2, %3").arg(ppm->position().x*lengthUnit, 11,'g',5).arg(ppm->position().y*lengthUnit, 11,'g',5).arg(ppm->position().z*lengthUnit, 11,'g',5));

        //        writeTextElement("x", QString("%1").arg(ppm->m_Position.x*lengthUnit,7,'f',3));
        //        writeTextElement("y", QString("%1").arg(ppm->m_Position.y*lengthUnit,7,'f',3));
        //        writeTextElement("z", QString("%1").arg(ppm->m_Position.z*lengthUnit,7,'f',3));
    }
    writeEndElement();
}


void XMLPlaneWriter::writePointMass(PointMass const &pm, double massUnit, double lengthUnit)
{
    writeStartElement("Point_Mass");
    {
        writeTextElement("Tag", pm.tag());
        writeTextElement("Mass", QString("%1").arg(pm.mass()*massUnit,7,'f',3));
        writeTextElement("coordinates",QString("%1, %2, %3")
                         .arg(pm.position().x*lengthUnit, 11,'g',5)
                         .arg(pm.position().y*lengthUnit, 11,'g',5)
                         .arg(pm.position().z*lengthUnit, 11,'g',5));

        //        writeTextElement("x", QString("%1").arg(ppm->m_Position.x*lengthUnit,7,'f',3));
        //        writeTextElement("y", QString("%1").arg(ppm->m_Position.y*lengthUnit,7,'f',3));
        //        writeTextElement("z", QString("%1").arg(ppm->m_Position.z*lengthUnit,7,'f',3));
    }
    writeEndElement();
}


void XMLPlaneWriter::writeBody(Body *pBody, Vector3d position, double lengthUnit, double massUnit)
{
    NURBSSurface *pSurface = pBody->splineSurface();
    writeStartElement("body");
    {
        writeTextElement("Name", pBody->name());
        writeColor(pBody->color());
        writeTextElement("Description", pBody->description());
        writeTextElement("Position",QString("%1, %2, %3").arg(position.x*lengthUnit, 11,'g',5)
                         .arg(position.y*lengthUnit, 11,'g',5)
                         .arg(position.z*lengthUnit, 11,'g',5));

        writeTextElement("Type", pBody->bodyType()==xfl::BODYPANELTYPE ? "FLATPANELS" : "NURBS");
        if(pBody->bodyType()==xfl::BODYSPLINETYPE && pSurface)
        {
            writeTextElement("x_degree", QString ("%1").arg(pSurface->uDegree()));
            writeTextElement("hoop_degree", QString ("%1").arg(pSurface->vDegree()));
            writeTextElement("x_panels", QString ("%1").arg(pBody->m_nxPanels));
            writeTextElement("hoop_panels", QString ("%1").arg(pBody->m_nhPanels));
        }
        else
        {
            writeStartElement("Panel_Stripes");
            {
                for(int isl=0; isl<pBody->sideLineCount(); isl++)
                {
                    writeTextElement(QString("stripe_%1").arg(isl),  QString("%1").arg(pBody->m_hPanels.at(isl)));
                }
            }
            writeEndElement();
        }
        writeStartElement("Inertia");
        {
            writeTextElement("Volume_Mass", QString("%1").arg(pBody->volumeMass(),7,'f',3));
            for(int ipm=0; ipm<pBody->m_PointMass.size(); ipm++)
            {
                writePointMass(pBody->m_PointMass.at(ipm), massUnit, lengthUnit);
            }
        }
        writeEndElement();

        for(int iFrame=0; iFrame<pBody->splineSurface()->frameCount(); iFrame++)
        {
            Frame *pFrame = pBody->splineSurface()->frameAt(iFrame);
            writeStartElement("frame");
            {
                if(pBody->bodyType()==xfl::BODYPANELTYPE)
                {
                    writeTextElement("x_panels", QString("%1").arg(pBody->m_xPanels.at(iFrame)));
                    //                    writeTextElement("h_panels", QString("%1").arg(pBody->m_xPanels.at(iFrame)));
                }

                writeTextElement("Position",QString("%1, %2, %3").arg(pFrame->m_Position.x*lengthUnit, 11,'g',5)
                                 .arg(pFrame->m_Position.y*lengthUnit, 11,'g',5)
                                 .arg(pFrame->m_Position.z*lengthUnit, 11,'g',5));

                for(int iPt=0; iPt<pFrame->pointCount(); iPt++)
                {
                    Vector3d Pt(pFrame->point(iPt));
                    writeTextElement("point",QString("%1, %2, %3").arg(Pt.x*lengthUnit, 11,'g',5)
                                     .arg(Pt.y*lengthUnit, 11,'g',5)
                                     .arg(Pt.z*lengthUnit, 11,'g',5));
                }
            }
            writeEndElement();
        }
    }
    writeEndElement();
}

