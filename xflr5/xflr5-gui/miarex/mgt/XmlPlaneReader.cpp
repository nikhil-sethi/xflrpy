/****************************************************************************

	XMLPlaneReader Class
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

#include "XmlPlaneReader.h"
#include <globals.h>
#include <QMessageBox>
#include <QtDebug>

XMLPlaneReader::XMLPlaneReader(QFile &file, Plane *pPlane)
{
	m_pPlane = pPlane;
	setDevice(&file);
}



bool XMLPlaneReader::readXMLPlaneFile()
{
	double lengthunit = 1.0;
	double massunit = 1.0;

	if (readNextStartElement())
	{
		if (name() == "explane" && attributes().value("version") == "1.0")
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().toString().compare("units", Qt::CaseInsensitive)==0)
				{
					while(!atEnd() && !hasError() && readNextStartElement() )
					{
						if (name().toString().compare("length_unit_to_meter",      Qt::CaseInsensitive)==0)
						{
							lengthunit = readElementText().toDouble();
						}
						else if (name().toString().compare("mass_unit_to_kg",      Qt::CaseInsensitive)==0)
						{
							massunit = readElementText().toDouble();
						}
						else
							skipCurrentElement();
					}
				}
				else if (name().toString().compare("plane", Qt::CaseInsensitive)==0)
				{
					readPlane(m_pPlane, lengthunit, massunit);
				}
				else if (name().toString().compare("body", Qt::CaseInsensitive)==0)
				{
//					if(m_pPlane->body()) delete m_pPlane->body();
//					m_pPlane->setBody(new Body);
					m_pPlane->hasBody() = true;
					readBody(m_pPlane->body(), m_pPlane->bodyPos(), lengthunit, massunit);
				}
			}
		}
		else
			raiseError(QObject::tr("The file is not an xflr5 plane version 1.0 file."));
	}
	return(hasError());
}




/** */
bool XMLPlaneReader::readPlane(Plane *pPlane, double lengthUnit, double massUnit)
{
	int iw=0;
	pPlane->hasElevator() = false;
	pPlane->hasSecondWing() = false;
	pPlane->hasFin() = false;

	while(!atEnd() && !hasError() && readNextStartElement() && iw<MAXWINGS)
	{
		if (name().toString().compare("name",Qt::CaseInsensitive) ==0)
		{
			pPlane->rPlaneName() = readElementText();
		}
		else if (name().toString().compare("has_body",Qt::CaseInsensitive) ==0)
		{
			pPlane->hasBody() = readElementText().compare("true", Qt::CaseInsensitive)==0;
		}
		else if (name().toString().compare("description", Qt::CaseInsensitive)==0)
		{
			pPlane->rPlaneDescription() = readElementText();
		}
		else if (name().toString().compare("inertia",         Qt::CaseInsensitive)==0)
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().toString().compare("point_mass", Qt::CaseInsensitive)==0)
				{
					PointMass* ppm = new PointMass;
					pPlane->m_PointMass.append(ppm);
					readPointMass(ppm, massUnit, lengthUnit);
				}
				else
					skipCurrentElement();
			}
		}
		else if (name().toString().compare("body", Qt::CaseInsensitive)==0)
		{
			pPlane->setBody(new Body);
			readBody(pPlane->body(), pPlane->bodyPos(), lengthUnit, massUnit);
		}
		else if (name().toString().compare("wing", Qt::CaseInsensitive)==0)
		{
			double xw=0.0, yw=0.0, zw=0.0, ta=0.0;
			Wing newWing;
			newWing.clearPointMasses();
			newWing.clearSurfaces();
			newWing.clearWingSections();
			{
				newWing.wingType() = XFLR5::OTHERWING;
				newWing.m_WingSection.clear();

				while(!atEnd() && !hasError() && readNextStartElement() )
				{
					if (name().toString().compare("name",                 Qt::CaseInsensitive)==0)
					{
						newWing.rWingName() = readElementText();
					}
					else if (name().toString().compare("type",        Qt::CaseInsensitive)==0)
					{
						newWing.wingType() = wingType(readElementText());
						if(newWing.wingType()==XFLR5::ELEVATOR)   pPlane->hasElevator() = true;
						else if(newWing.wingType()==XFLR5::SECONDWING) pPlane->hasSecondWing() = true;
						else if(newWing.wingType()==XFLR5::FIN)        pPlane->hasFin() = true;
					}
					else if (name().toString().compare("color",           Qt::CaseInsensitive)==0)
					{
						readColor(newWing.wingColor());
					}
					else if (name().toString().compare("description",     Qt::CaseInsensitive)==0)
					{
						newWing.rWingDescription() = readElementText();
					}
					else if (name().toString().compare("position",        Qt::CaseInsensitive)==0)
					{
						QStringList coordList = readElementText().split(",");
						if(coordList.length()>=3)
						{
							xw = coordList.at(0).toDouble()*lengthUnit;
							yw = coordList.at(1).toDouble()*lengthUnit;
							zw = coordList.at(2).toDouble()*lengthUnit;
						}
					}
					else if (name().toString().compare("tilt_angle",      Qt::CaseInsensitive)==0)
					{
						ta = readElementText().toDouble();
					}
					else if (name().toString().compare("Symetric",        Qt::CaseInsensitive)==0)
					{
						newWing.isSymetric() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().toString().compare("isFin",           Qt::CaseInsensitive)==0)
					{
						newWing.isFin() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().toString().compare("isDoubleFin",     Qt::CaseInsensitive)==0)
					{
						newWing.isDoubleFin() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().toString().compare("isSymFin",        Qt::CaseInsensitive)==0)
					{
						newWing.isSymFin() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().toString().compare("Inertia",         Qt::CaseInsensitive)==0)
					{
						while(!atEnd() && !hasError() && readNextStartElement() )
						{
							if (name().toString().compare("volume_mass", Qt::CaseInsensitive)==0)
							{
								newWing.volumeMass() = readElementText().toDouble();
							}
							else if (name().toString().compare("point_mass", Qt::CaseInsensitive)==0)
							{
								PointMass* ppm = new PointMass;
								newWing.m_PointMass.append(ppm);
								readPointMass(ppm, massUnit, lengthUnit);
							}
							else
								skipCurrentElement();
						}
					}
					else if (name().toString().compare("Sections",        Qt::CaseInsensitive)==0)
					{
						while(!atEnd() && !hasError() && readNextStartElement() )
						{
							if (name().toString().compare("Section",  Qt::CaseInsensitive)==0)
							{
								WingSection *pWingSec = new WingSection;
								newWing.m_WingSection.append(pWingSec);
								while(!atEnd() && !hasError() && readNextStartElement() )
								{
									if (name().toString().compare("x_number_of_panels", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_NXPanels = readElementText().toInt();
									}
									else if (name().toString().compare("y_number_of_panels", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_NYPanels = readElementText().toInt();
									}
									else if (name().toString().compare("x_panel_distribution", Qt::CaseInsensitive)==0)
									{
										QString strPanelDist = readElementText();
										pWingSec->m_XPanelDist = distributionType(strPanelDist);
									}
									else if (name().toString().compare("y_panel_distribution", Qt::CaseInsensitive)==0)
									{
										QString strPanelDist = readElementText();
										pWingSec->m_YPanelDist = distributionType(strPanelDist);
									}
									else if (name().toString().compare("Chord", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Chord = readElementText().toDouble()*lengthUnit;
									}
									else if (name().toString().compare("y_position", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_YPosition = readElementText().toDouble()*lengthUnit;
									}
									else if (name().toString().compare("xOffset", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Offset = readElementText().toDouble()*lengthUnit;
									}
									else if (name().toString().compare("Dihedral", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Dihedral = readElementText().toDouble();
									}
									else if (name().toString().compare("Twist", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Twist = readElementText().toDouble();
									}
									else if (name().toString().compare("Left_Side_FoilName", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_LeftFoilName = readElementText();
									}
									else if (name().toString().compare("Right_Side_FoilName", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_RightFoilName = readElementText();
									}
									else
										skipCurrentElement();
								}
							}
							else
								skipCurrentElement();
						}
					}
					else
						skipCurrentElement();
				}

				int iWing = 0;
				if(newWing.wingType()==XFLR5::OTHERWING)
				{
					if(newWing.isFin())
					{
						newWing.wingType() = XFLR5::FIN;
						pPlane->hasFin() = true;
						iWing = 3;
					}
					else if(iw==0)
					{
						newWing.wingType() = XFLR5::MAINWING;
						iWing = 0;
					}
					else if(iw==1)
					{
						newWing.wingType() = XFLR5::ELEVATOR;
						pPlane->hasElevator() = true;
						iWing = 2;
					}
				}
				else
				{
					if(newWing.wingType()==XFLR5::MAINWING) iWing = 0;
					else if(newWing.wingType()==XFLR5::SECONDWING)
					{
						iWing = 1;
						pPlane->hasSecondWing() = true;
					}
					else if(newWing.wingType()==XFLR5::ELEVATOR)
					{
						iWing = 2;
						pPlane->hasElevator() = true;
					}
					else if(newWing.wingType()==XFLR5::FIN)
					{
						iWing = 3;
						pPlane->hasFin() = true;
					}
				}

				if(!hasError())
				{
					pPlane->m_Wing[iWing].duplicate(&newWing);
					pPlane->WingLE(iWing).x      = xw;
					pPlane->WingLE(iWing).y      = yw;
					pPlane->WingLE(iWing).z      = zw;
					pPlane->WingTiltAngle(iWing) = ta;
				}
				iw++;
			}
		}
		else
			skipCurrentElement();
	}
	if(pPlane->fin() && pPlane->fin()->m_bDoubleFin) pPlane->m_bDoubleFin = true;
	if(pPlane->fin() && pPlane->fin()->m_bSymFin)    pPlane->m_bSymFin = true;

	return(hasError());
}



bool XMLPlaneReader::readPointMass(PointMass *ppm, double massUnit, double lengthUnit)
{
	while(!atEnd() && !hasError() && readNextStartElement() )
	{
		if (name().toString().compare("tag", Qt::CaseInsensitive)==0)       ppm->tag() = readElementText();
		else if (name().toString().compare("mass", Qt::CaseInsensitive)==0) ppm->mass() =  readElementText().toDouble()*massUnit;
		else if (name().toString().compare("coordinates", Qt::CaseInsensitive)==0)
		{
			QStringList coordList = readElementText().split(",");
			if(coordList.length()>=3)
			{
				ppm->position().x = coordList.at(0).toDouble()*lengthUnit;
				ppm->position().y = coordList.at(1).toDouble()*lengthUnit;
				ppm->position().z = coordList.at(2).toDouble()*lengthUnit;
			}
		}
		else skipCurrentElement();
	}
	return(hasError());
}




bool XMLPlaneReader::readBody(Body *pBody, Vector3d &position, double lengthUnit, double massUnit)
{
	pBody->splineSurface()->clearFrames();
	pBody->m_xPanels.clear();
	pBody->m_hPanels.clear();
	pBody->m_XPanelPos.clear();

	while(!atEnd() && !hasError() && readNextStartElement() )
	{
		if (name().toString().compare("name",Qt::CaseInsensitive) ==0)
		{
			pBody->bodyName() = readElementText();
		}
		else if (name().toString().compare("color", Qt::CaseInsensitive)==0)
		{
			readColor(pBody->bodyColor());
		}
		else if (name().toString().compare("description", Qt::CaseInsensitive)==0)
		{
			pBody->bodyDescription() = readElementText();
		}
		else if (name().toString().compare("Inertia", Qt::CaseInsensitive)==0)
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().toString().compare("volume_mass", Qt::CaseInsensitive)==0)
				{
					pBody->m_VolumeMass = readElementText().toDouble();
				}
				else if (name().toString().compare("point_mass", Qt::CaseInsensitive)==0)
				{
					PointMass* ppm = new PointMass;
					pBody->m_PointMass.append(ppm);
					readPointMass(ppm, massUnit, lengthUnit);
				}
				else
					skipCurrentElement();
			}
		}
		else if (name().toString().compare("position", Qt::CaseInsensitive)==0)
		{
			QStringList coordList = readElementText().split(",");
			if(coordList.length()>=3)
			{
				position.x = coordList.at(0).toDouble()*lengthUnit;
				position.z = coordList.at(2).toDouble()*lengthUnit;
			}
		}
		else if (name().toString().compare("type", Qt::CaseInsensitive)==0)
		{
			if(readElementText().compare("NURBS", Qt::CaseInsensitive)==0) pBody->bodyType()=XFLR5::BODYSPLINETYPE;
			else                                                           pBody->bodyType()=XFLR5::BODYPANELTYPE;
		}
		else if (name().toString().compare("x_degree",    Qt::CaseInsensitive)==0) pBody->splineSurface()->m_iuDegree = readElementText().toInt();
		else if (name().toString().compare("hoop_degree", Qt::CaseInsensitive)==0) pBody->splineSurface()->m_ivDegree = readElementText().toInt();
		else if (name().toString().compare("x_panels",    Qt::CaseInsensitive)==0) pBody->m_nxPanels = readElementText().toInt();
		else if (name().toString().compare("hoop_panels", Qt::CaseInsensitive)==0) pBody->m_nhPanels = readElementText().toInt();
		else if (name().toString().compare("Panel_Stripes", Qt::CaseInsensitive)==0)
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().contains("stripe", Qt::CaseInsensitive))
				{
					pBody->m_hPanels.append(readElementText().toInt());
				}
			}
		}
		//read frames
		else if (name().toString().compare("frame", Qt::CaseInsensitive)==0)
		{
			Frame *pFrame = pBody->splineSurface()->appendNewFrame();
			pBody->m_xPanels.append(1);
			pBody->m_XPanelPos.append(0.0);

			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if(name().toString().compare("x_panels", Qt::CaseInsensitive)==0) pBody->m_xPanels[pBody->frameCount()-1] = readElementText().toInt();
				if(name().toString().compare("h_panels", Qt::CaseInsensitive)==0) pBody->m_hPanels[pBody->frameCount()-1] = readElementText().toInt();
				else if (name().toString().compare("position", Qt::CaseInsensitive)==0)
				{
					QStringList coordList = readElementText().split(",");
					if(coordList.length()>=3)
					{
						pFrame->m_Position.x = coordList.at(0).toDouble()*lengthUnit;
						pFrame->m_Position.z = coordList.at(2).toDouble()*lengthUnit;
					}
					pBody->m_XPanelPos[pBody->frameCount()-1] = pFrame->m_Position.x;
				}
				else if (name().toString().compare("point", Qt::CaseInsensitive)==0)
				{
					Vector3d ctrlPt;
					QStringList coordList = readElementText().split(",");
					if(coordList.length()>=3)
					{
						ctrlPt.x = coordList.at(0).toDouble()*lengthUnit;
						ctrlPt.y = coordList.at(1).toDouble()*lengthUnit;
						ctrlPt.z = coordList.at(2).toDouble()*lengthUnit;
						pFrame->appendPoint(ctrlPt);
					}
				}
			}
		}
	}
	if(pBody->isSplineType())
	{
		for(int ifp=0; ifp<pBody->framePointCount(); ifp++)
			pBody->m_hPanels.append(1);
	}
	return(hasError());
}




bool XMLPlaneReader::readColor(QColor &color)
{
	color.setRgb(0,0,0,255);
	while(!atEnd() && !hasError() && readNextStartElement() )
	{
		if (name().toString().compare("red", Qt::CaseInsensitive)==0)         color.setRed(readElementText().toInt());
		else if (name().toString().compare("green", Qt::CaseInsensitive)==0)  color.setGreen(readElementText().toInt());
		else if (name().toString().compare("blue", Qt::CaseInsensitive)==0)   color.setBlue(readElementText().toInt());
		else if (name().toString().compare("alpha", Qt::CaseInsensitive)==0)  color.setAlpha(readElementText().toInt());
		else skipCurrentElement();
	}
	return(hasError());
}
