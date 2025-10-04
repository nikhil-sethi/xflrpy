/****************************************************************************

    XMLPlaneReader Class
    Copyright (C) 2015 André Deperrois

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


#include <QMessageBox>
#include <QDebug>

#include "xmlplanereader.h"
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/plane.h>

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
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    while(!atEnd() && !hasError() && readNextStartElement() )
                    {
                        if (name().compare(QString("length_unit_to_meter"),      Qt::CaseInsensitive)==0)
                        {
                            lengthunit = readElementText().toDouble();
                        }
                        else if (name().compare(QString("mass_unit_to_kg"),      Qt::CaseInsensitive)==0)
                        {
                            massunit = readElementText().toDouble();
                        }
                        else
                            skipCurrentElement();
                    }
                }
                else if (name().toString().compare(QString("plane"), Qt::CaseInsensitive)==0)
                {
                    readPlane(m_pPlane, lengthunit, massunit);
                }
                else if (name().toString().compare(QString("body"), Qt::CaseInsensitive)==0)
                {
                    m_pPlane->setBody(true);
                    Vector3d pos;
                    readBody(m_pPlane->body(), pos, lengthunit, massunit);
                    m_pPlane->setBodyPos(pos);
                }
                else if (name().toString().compare(QString("wing"), Qt::CaseInsensitive)==0)
                {
                    Vector3d V;
                    double ry=0.0;
                    m_pPlane->wing(0)->clearWingSections();
                    m_pPlane->wing(0)->clearPointMasses();
                    readWing(*m_pPlane->wing(0), V, ry, lengthunit, massunit);
                }
            }
        }
        else
            raiseError(QObject::tr("The file is not an xflr5 plane version 1.0 file."));
    }
    return(hasError());
}


/** */
bool XMLPlaneReader::readPlane(Plane *pPlane, double lengthunit, double massunit)
{
    int iw=0;
    pPlane->setElevator(false);
    pPlane->setSecondWing(false);
    pPlane->setFin(false);

    while(!atEnd() && !hasError() && readNextStartElement() && iw<MAXWINGS)
    {
        if (name().toString().compare(QString("name"),Qt::CaseInsensitive) ==0)
        {
            pPlane->setName(readElementText());
        }
        else if (name().toString().compare(QString("has_body"),Qt::CaseInsensitive) ==0)
        {
            pPlane->setBody(readElementText().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().toString().compare(QString("description"), Qt::CaseInsensitive)==0)
        {
            pPlane->setDescription(readElementText());
        }
        else if (name().compare(QString("inertia"),         Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("point_mass"), Qt::CaseInsensitive)==0)
                {
                    PointMass pm;
                    readPointMass(&pm, massunit, lengthunit);
                    pPlane->m_PointMass.append(pm);
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().toString().compare(QString("body"), Qt::CaseInsensitive)==0)
        {
            pPlane->setBody(new Body);
            Vector3d pos;
            readBody(pPlane->body(), pos, lengthunit, massunit);
            pPlane->setBodyPos(pos);
        }
        else if (name().toString().compare(QString("wing"), Qt::CaseInsensitive)==0)
        {
            Wing newwing;
            newwing.clearPointMasses();
            newwing.clearSurfaces();
            newwing.clearWingSections();
            newwing.setWingType(xfl::OTHERWING);
            newwing.m_Section.clear();

            Vector3d pos;
            double tiltangle=0.0;
            readWing(newwing, pos, tiltangle, lengthunit, massunit);


            int iWing = 0;
            if(newwing.wingType()==xfl::OTHERWING)
            {
                if(newwing.isFin())
                {
                    newwing.setWingType(xfl::FIN);
                    pPlane->setFin(true);
                    iWing = 3;
                }
                else if(iw==0)
                {
                    newwing.setWingType(xfl::MAINWING);
                    iWing = 0;
                }
                else if(iw==1)
                {
                    newwing.setWingType(xfl::ELEVATOR);
                    pPlane->setElevator(true);
                    iWing = 2;
                }
            }
            else
            {
                if(newwing.wingType()==xfl::MAINWING) iWing = 0;
                else if(newwing.wingType()==xfl::SECONDWING)
                {
                    iWing = 1;
                    pPlane->setSecondWing(true);
                }
                else if(newwing.wingType()==xfl::ELEVATOR)
                {
                    iWing = 2;
                    pPlane->setElevator(true);
                }
                else if(newwing.wingType()==xfl::FIN)
                {
                    iWing = 3;
                    pPlane->setFin(true);
                }
            }

            if(!hasError())
            {
                pPlane->m_Wing[iWing].duplicate(&newwing);
                pPlane->setWingLE(iWing, pos);
                pPlane->setWingTiltAngle(iWing, tiltangle);
            }
            iw++;
        }
        else
            skipCurrentElement();
    }
    if(pPlane->fin() && pPlane->fin()->m_bDoubleFin) pPlane->setDoubleFin(true);
    if(pPlane->fin() && pPlane->fin()->m_bSymFin)    pPlane->setSymFin(true);

    return(hasError());
}


bool XMLPlaneReader::readWing(Wing &newwing, Vector3d &position, double &tiltangle, double lengthUnit, double massUnit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("name"),                 Qt::CaseInsensitive)==0)
        {
            newwing.setWingName(readElementText());
        }
        else if (name().compare(QString("type"),            Qt::CaseInsensitive)==0)
        {
            newwing.setWingType(xfl::wingType(readElementText()));
            if(m_pPlane)
            {
                if     (newwing.wingType()==xfl::ELEVATOR)   m_pPlane->setElevator(true);
                else if(newwing.wingType()==xfl::SECONDWING) m_pPlane->setSecondWing(true);
                else if(newwing.wingType()==xfl::FIN)        m_pPlane->setFin(true);
            }
        }
        else if (name().compare(QString("color"),           Qt::CaseInsensitive)==0)
        {
            QColor clr;
            readColor(clr);
            newwing.setColor(clr);
        }
        else if (name().compare(QString("description"),     Qt::CaseInsensitive)==0)
        {
            newwing.setWingDescription(readElementText());
        }
        else if (name().compare(QString("position"),        Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().split(",");
            if(coordList.length()>=3)
            {
                position.x = coordList.at(0).toDouble()*lengthUnit;
                position.y = coordList.at(1).toDouble()*lengthUnit;
                position.z = coordList.at(2).toDouble()*lengthUnit;
            }
        }
        else if (name().compare(QString("tilt_angle"),      Qt::CaseInsensitive)==0)
        {
            tiltangle = readElementText().toDouble();
        }
        else if (name().compare(QString("Symetric"),        Qt::CaseInsensitive)==0)
        {
            newwing.setSymetric(readElementText().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("isFin"),           Qt::CaseInsensitive)==0)
        {
            newwing.setFin(readElementText().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("isDoubleFin"),     Qt::CaseInsensitive)==0)
        {
            newwing.setDoubleFin(readElementText().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("isSymFin"),        Qt::CaseInsensitive)==0)
        {
            newwing.setSymFin(readElementText().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("Inertia"),         Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("volume_mass"), Qt::CaseInsensitive)==0)
                {
                    newwing.setVolumeMass(readElementText().toDouble());
                }
                else if (name().compare(QString("point_mass"), Qt::CaseInsensitive)==0)
                {
                    PointMass pm;
                    readPointMass(&pm, massUnit, lengthUnit);
                    newwing.m_PointMass.append(pm);
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().compare(QString("Sections"),        Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Section"),  Qt::CaseInsensitive)==0)
                {
                    newwing.m_Section.append(WingSection());
                    WingSection &WS = newwing.m_Section.last();
                    while(!atEnd() && !hasError() && readNextStartElement() )
                    {
                        if (name().compare(QString("x_number_of_panels"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_NXPanels = readElementText().toInt();
                        }
                        else if (name().compare(QString("y_number_of_panels"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_NYPanels = readElementText().toInt();
                        }
                        else if (name().compare(QString("x_panel_distribution"), Qt::CaseInsensitive)==0)
                        {
                            QString strPanelDist = readElementText();
                            WS.m_XPanelDist = xfl::distributionType(strPanelDist);
                        }
                        else if (name().compare(QString("y_panel_distribution"), Qt::CaseInsensitive)==0)
                        {
                            QString strPanelDist = readElementText();
                            WS.m_YPanelDist = xfl::distributionType(strPanelDist);
                        }
                        else if (name().compare(QString("Chord"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_Chord = readElementText().toDouble()*lengthUnit;
                        }
                        else if (name().compare(QString("y_position"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_YPosition = readElementText().toDouble()*lengthUnit;
                        }
                        else if (name().compare(QString("xOffset"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_Offset = readElementText().toDouble()*lengthUnit;
                        }
                        else if (name().compare(QString("Dihedral"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_Dihedral = readElementText().toDouble();
                        }
                        else if (name().compare(QString("Twist"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_Twist = readElementText().toDouble();
                        }
                        else if (name().compare(QString("Left_Side_FoilName"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_LeftFoilName = readElementText();
                        }
                        else if (name().compare(QString("Right_Side_FoilName"), Qt::CaseInsensitive)==0)
                        {
                            WS.m_RightFoilName = readElementText();
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
    return(hasError());
}


bool XMLPlaneReader::readPointMass(PointMass *ppm, double massUnit, double lengthUnit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("tag"), Qt::CaseInsensitive)==0)       ppm->setTag(readElementText());
        else if (name().compare(QString("mass"), Qt::CaseInsensitive)==0) ppm->setMass(readElementText().toDouble()*massUnit);
        else if (name().compare(QString("coordinates"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().split(",");
            if(coordList.length()>=3)
            {
                ppm->setXPosition(coordList.at(0).toDouble()*lengthUnit);
                ppm->setYPosition(coordList.at(1).toDouble()*lengthUnit);
                ppm->setZPosition(coordList.at(2).toDouble()*lengthUnit);
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
        if (name().toString().compare(QString("name"),Qt::CaseInsensitive) ==0)
        {
            pBody->setName(readElementText());
        }
        else if (name().toString().compare(QString("color"), Qt::CaseInsensitive)==0)
        {
            QColor clr;
            readColor(clr);
            pBody->setColor(clr);
        }
        else if (name().toString().compare(QString("description"), Qt::CaseInsensitive)==0)
        {
            pBody->setDescription(readElementText());
        }
        else if (name().compare(QString("Inertia"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("volume_mass"), Qt::CaseInsensitive)==0)
                {
                    pBody->m_VolumeMass = readElementText().toDouble();
                }
                else if (name().compare(QString("point_mass"), Qt::CaseInsensitive)==0)
                {
                    PointMass pm;
                    readPointMass(&pm, massUnit, lengthUnit);
                    pBody->m_PointMass.append(pm);
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().compare(QString("position"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().split(",");
            if(coordList.length()>=3)
            {
                position.x = coordList.at(0).toDouble()*lengthUnit;
                position.z = coordList.at(2).toDouble()*lengthUnit;
            }
        }
        else if (name().compare(QString("type"), Qt::CaseInsensitive)==0)
        {
            if(readElementText().compare(QString("NURBS"), Qt::CaseInsensitive)==0) pBody->bodyType()=xfl::BODYSPLINETYPE;
            else                                                           pBody->bodyType()=xfl::BODYPANELTYPE;
        }
        else if (name().compare(QString("x_degree"),    Qt::CaseInsensitive)==0) pBody->splineSurface()->setuDegree(readElementText().toInt());
        else if (name().compare(QString("hoop_degree"), Qt::CaseInsensitive)==0) pBody->splineSurface()->setvDegree(readElementText().toInt());
        else if (name().compare(QString("x_panels"),    Qt::CaseInsensitive)==0) pBody->m_nxPanels = readElementText().toInt();
        else if (name().compare(QString("hoop_panels"), Qt::CaseInsensitive)==0) pBody->m_nhPanels = readElementText().toInt();
        else if (name().compare(QString("Panel_Stripes"), Qt::CaseInsensitive)==0)
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
        else if (name().compare(QString("frame"), Qt::CaseInsensitive)==0)
        {
            Frame *pFrame = pBody->splineSurface()->appendNewFrame();
            pBody->m_xPanels.append(1);
            pBody->m_XPanelPos.append(0.0);

            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if(name().compare(QString("x_panels"), Qt::CaseInsensitive)==0) pBody->m_xPanels[pBody->frameCount()-1] = readElementText().toInt();
                if(name().compare(QString("h_panels"), Qt::CaseInsensitive)==0) pBody->m_hPanels[pBody->frameCount()-1] = readElementText().toInt();
                else if (name().compare(QString("position"), Qt::CaseInsensitive)==0)
                {
                    QStringList coordList = readElementText().split(",");
                    if(coordList.length()>=3)
                    {
                        pFrame->m_Position.x = coordList.at(0).toDouble()*lengthUnit;
                        pFrame->m_Position.z = coordList.at(2).toDouble()*lengthUnit;
                    }
                    pBody->m_XPanelPos[pBody->frameCount()-1] = pFrame->m_Position.x;
                }
                else if (name().compare(QString("point"), Qt::CaseInsensitive)==0)
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
        if (name().compare(QString("red"), Qt::CaseInsensitive)==0)         color.setRed(readElementText().toInt());
        else if (name().compare(QString("green"), Qt::CaseInsensitive)==0)  color.setGreen(readElementText().toInt());
        else if (name().compare(QString("blue"), Qt::CaseInsensitive)==0)   color.setBlue(readElementText().toInt());
        else if (name().compare(QString("alpha"), Qt::CaseInsensitive)==0)  color.setAlpha(readElementText().toInt());
        else skipCurrentElement();
    }
    return(hasError());
}
