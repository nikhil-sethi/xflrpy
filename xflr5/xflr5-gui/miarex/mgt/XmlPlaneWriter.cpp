/****************************************************************************

	XmlPlaneWriter Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#include "XmlPlaneWriter.h"
#include <globals.h>
#include <misc/options/Units.h>


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


void XMLPlaneWriter::writeXMLPlane(Plane *m_pPlane)
{
	if(!m_pPlane) return;

	writeHeader();

	writeStartElement("Plane");
	{
		writeTextElement("Name", m_pPlane->planeName());
		writeTextElement("Description", m_pPlane->planeDescription());
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
				writeStartElement("wing");
				{
					writeTextElement("Name", m_pPlane->wing(iw)->wingName());
					writeTextElement("Type",   wingType(m_pPlane->wing(iw)->wingType()));
					writeColor(m_pPlane->wing(iw)->wingColor());
					writeTextElement("Description", m_pPlane->wing(iw)->WingDescription());
					writeTextElement("Position",QString("%1, %2, %3").arg(m_pPlane->WingLE(iw).x*Units::mtoUnit(), 11,'g',5)
																	 .arg(m_pPlane->WingLE(iw).y*Units::mtoUnit(), 11,'g',5)
																	 .arg(m_pPlane->WingLE(iw).z*Units::mtoUnit(), 11,'g',5));
					writeTextElement("Tilt_angle",  QString("%1").arg(m_pPlane->WingTiltAngle(iw),7,'f',3));
					writeTextElement("Symetric",    m_pPlane->wing(iw)->isSymetric()  ? "true" : "false");
					writeTextElement("isFin",       m_pPlane->wing(iw)->isFin()       ? "true" : "false");
					writeTextElement("isDoubleFin", m_pPlane->wing(iw)->isDoubleFin() ? "true" : "false");
					writeTextElement("isSymFin",    m_pPlane->wing(iw)->isSymFin()    ? "true" : "false");
					writeStartElement("Inertia");
					{
						writeTextElement("Volume_Mass", QString("%1").arg(m_pPlane->wing(iw)->volumeMass(),7,'f',3));
						for(int ipm=0; ipm<m_pPlane->wing(iw)->m_PointMass.size(); ipm++)
						{
							writePointMass(m_pPlane->wing(iw)->m_PointMass.at(ipm), Units::kgtoUnit(), Units::mtoUnit());
						}
					}
					writeEndElement();
					writeStartElement("Sections");
					{
						for(int ips=0; ips<m_pPlane->wing(iw)->m_WingSection.size(); ips++)
						{
							WingSection *ws = m_pPlane->wing(iw)->m_WingSection.at(ips);
							writeStartElement("Section");
							{
								writeTextElement("y_position", QString("%1").arg(ws->m_YPosition*Units::mtoUnit(), 7, 'f', 3));
								writeTextElement("Chord", QString("%1").arg(ws->m_Chord*Units::mtoUnit(), 7, 'f', 3));
								writeTextElement("xOffset", QString("%1").arg(ws->m_Offset*Units::mtoUnit(), 7, 'f', 3));
								writeTextElement("Dihedral", QString("%1").arg(ws->m_Dihedral, 7, 'f', 3));
								writeTextElement("Twist", QString("%1").arg(ws->m_Twist, 7, 'f', 3));
								writeTextElement("x_number_of_panels", QString("%1").arg(ws->m_NXPanels));
								writeTextElement("x_panel_distribution", distributionType(ws->m_XPanelDist));
								writeTextElement("y_number_of_panels", QString("%1").arg(ws->m_NYPanels));
								writeTextElement("y_panel_distribution", distributionType(ws->m_YPanelDist));
								writeTextElement("Left_Side_FoilName", ws->m_LeftFoilName);
								writeTextElement("Right_Side_FoilName", ws->m_RightFoilName);
							}
							writeEndElement();
						}
					}
					writeEndElement();
				}
				writeEndElement();
			}
		}
	}
	writeEndElement();


	writeEndDocument();
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




void XMLPlaneWriter::writePointMass(PointMass *ppm, double massUnit, double lengthUnit)
{
	writeStartElement("Point_Mass");
	{
		writeTextElement("Tag", ppm->tag());
		writeTextElement("Mass", QString("%1").arg(ppm->mass()*massUnit,7,'f',3));
		writeTextElement("coordinates",QString("%1, %2, %3").arg(ppm->position().x*lengthUnit, 11,'g',5).arg(ppm->position().y*lengthUnit, 11,'g',5).arg(ppm->position().z*lengthUnit, 11,'g',5));

//		writeTextElement("x", QString("%1").arg(ppm->m_Position.x*lengthUnit,7,'f',3));
//		writeTextElement("y", QString("%1").arg(ppm->m_Position.y*lengthUnit,7,'f',3));
//		writeTextElement("z", QString("%1").arg(ppm->m_Position.z*lengthUnit,7,'f',3));
	}
	writeEndElement();
}




void XMLPlaneWriter::writeBody(Body *pBody, Vector3d position, double lengthUnit, double massUnit)
{
	NURBSSurface *pSurface = pBody->splineSurface();
	writeStartElement("body");
	{
		writeTextElement("Name", pBody->bodyName());
		writeColor(pBody->bodyColor());
		writeTextElement("Description", pBody->bodyDescription());
		writeTextElement("Position",QString("%1, %2, %3").arg(position.x*lengthUnit, 11,'g',5)
														 .arg(position.y*lengthUnit, 11,'g',5)
														 .arg(position.z*lengthUnit, 11,'g',5));

		writeTextElement("Type", pBody->bodyType()==XFLR5::BODYPANELTYPE ? "FLATPANELS" : "NURBS");
		if(pBody->bodyType()==XFLR5::BODYSPLINETYPE && pSurface)
		{
			writeTextElement("x_degree", QString ("%1").arg(pSurface->m_iuDegree));
			writeTextElement("hoop_degree", QString ("%1").arg(pSurface->m_ivDegree));
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
				if(pBody->bodyType()==XFLR5::BODYPANELTYPE)
				{
					writeTextElement("x_panels", QString("%1").arg(pBody->m_xPanels.at(iFrame)));
//					writeTextElement("h_panels", QString("%1").arg(pBody->m_xPanels.at(iFrame)));
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

