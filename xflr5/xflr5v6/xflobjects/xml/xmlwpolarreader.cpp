/****************************************************************************

    XMLWPolarReader Class
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
#include <QStringList>
#include <xflcore/xflcore.h>
#include "xmlwpolarreader.h"
#include <xflobjects/objects3d/wpolar.h>

XmlWPolarReader::XmlWPolarReader(QFile &file, WPolar *pWPolar)
{
    m_pWPolar = pWPolar;
    setDevice(&file);
}

bool XmlWPolarReader::readXMLPolarFile()
{
    double lengthunit   = 1.0;
    double areaunit     = 1.0;
    double massunit     = 1.0;
    double velocityunit = 1.0;
    double inertiaunit  = 1.0;


    if (readNextStartElement())
    {
        if (name() == "Plane_Polar" && attributes().value("version") == "1.0")
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
                        else if (name().compare(QString("speed_unit_to_ms"),      Qt::CaseInsensitive)==0)
                        {
                            velocityunit = readElementText().toDouble();
                        }
                        else if (name().compare(QString("inertia_unit_to_kgm2"),  Qt::CaseInsensitive)==0)
                        {
                            inertiaunit = readElementText().toDouble();
                        }
                        else if (name().compare(QString("area_unit_to_m2"),  Qt::CaseInsensitive)==0)
                        {
                            areaunit = readElementText().toDouble();
                        }
                        else
                            skipCurrentElement();
                    }
                }
                else if (name().toString().compare(QString("Polar"), Qt::CaseInsensitive)==0)
                {
                    readWPolar(m_pWPolar, lengthunit, areaunit, massunit, velocityunit, inertiaunit);
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


void XmlWPolarReader::readWPolar(WPolar *pWPolar, double lengthunit, double areaunit, double massunit, double velocityunit, double inertiaunit)
{
    int iw=0;

    while(!atEnd() && !hasError() && readNextStartElement() && iw<MAXWINGS)
    {
        if (name().toString().compare(QString("polar_name"),Qt::CaseInsensitive) ==0)
        {
            pWPolar->setPolarName(readElementText());
        }
        else if (name().toString().compare(QString("plane_name"),Qt::CaseInsensitive) ==0)
        {
            pWPolar->setPlaneName(readElementText());
        }
        else if (name().toString().compare(QString("type"),Qt::CaseInsensitive) ==0)
        {
            pWPolar->setPolarType(xfl::WPolarType(readElementText()));
        }
        else if (name().toString().compare(QString("method"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setAnalysisMethod(xfl::analysisMethod(readElementText()));
        }
        else if (name().compare(QString("Ignore_Body_Panels"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setIgnoreBodyPanels(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Use_VLM1"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setVLM1(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Viscous_Analysis"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setViscous(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Thin_Surfaces"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setThinSurfaces(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Wake_Rollup"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setWakeRollUp(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Tilted_Analysis"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setTilted(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Ground_Effect"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setGroundEffect(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Ground_Height"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setGroundHeight(readElementText().toDouble()*lengthunit);
        }
        else if (name().compare(QString("Viscosity"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setViscosity(readElementText().toDouble());
        }
        else if (name().compare(QString("Density"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setDensity(readElementText().toDouble());
        }
        else if (name().compare(QString("Fixed_Velocity"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setVelocity(readElementText().toDouble()*velocityunit);
        }
        else if (name().compare(QString("Fixed_AOA"),         Qt::CaseInsensitive)==0)
        {
            pWPolar->setAlpha(readElementText().toDouble());
        }
        else if (name().compare(QString("Fixed_Bank_Angle"),       Qt::CaseInsensitive)==0)
        {
            pWPolar->setPhi(readElementText().toDouble());
        }
        else if (name().compare(QString("Fixed_SideSlip"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setBeta(readElementText().toDouble());
        }
        else if (name().compare(QString("Reference_Dimensions"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setReferenceDim(xfl::referenceDimension(readElementText()));
        }
        else if (name().compare(QString("Reference_Area"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setReferenceArea(readElementText().toDouble()*areaunit);
        }
        else if (name().compare(QString("Reference_Span_Length"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setReferenceSpanLength(readElementText().toDouble()*lengthunit);
        }
        else if (name().compare(QString("Reference_Chord_Length"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setReferenceChordLength(readElementText().toDouble()*lengthunit);
        }
        else if (name().compare(QString("ExtraDrag"),         Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("ExtraDragCoef_1"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragCoef[0] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragArea_1"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragArea[0] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragCoef_2"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragCoef[1] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragArea_2"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragArea[1] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragCoef_3"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragCoef[2] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragArea_3"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragArea[2] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragCoef_4"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragCoef[3] = readElementText().toDouble();
                }
                else if (name().compare(QString("ExtraDragArea_4"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_ExtraDragArea[3] = readElementText().toDouble();
                }
                else
                    skipCurrentElement();

            }
        }
        else if (name().compare(QString("Use_Plane_Inertia"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setAutoInertia(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Inertia_Value"),         Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Mass"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->setMass(readElementText().toDouble()*massunit);
                }
                else if (name().compare(QString("CoG"), Qt::CaseInsensitive)==0)
                {
                    QStringList coordList = readElementText().split(",");
                    if(coordList.length()>=3)
                    {
                        m_pWPolar->setCoGx(coordList.at(0).toDouble()*lengthunit);
                        m_pWPolar->setCoGz(coordList.at(2).toDouble()*lengthunit);
                    }
                }
                else if (name().compare(QString("CoG_Ixx"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->setCoGIxx(readElementText().toDouble()*inertiaunit);
                }
                else if (name().compare(QString("CoG_Iyy"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->setCoGIyy(readElementText().toDouble()*inertiaunit);
                }
                else if (name().compare(QString("CoG_Izz"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->setCoGIzz(readElementText().toDouble()*inertiaunit);
                }
                else if (name().compare(QString("CoG_Izz"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->setCoGIxz(readElementText().toDouble()*inertiaunit);
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().compare(QString("Inertia_gains"),         Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Mass_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[0] = readElementText().toDouble()*massunit;
                }
                else if (name().compare(QString("CoG_x_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[1] = readElementText().toDouble()*lengthunit;
                }
                else if (name().compare(QString("CoG_z_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[2] = readElementText().toDouble()*lengthunit;
                }
                else if (name().compare(QString("Ixx_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[3] = readElementText().toDouble()*inertiaunit;
                }
                else if (name().compare(QString("Iyy_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[4] = readElementText().toDouble()*inertiaunit;
                }
                else if (name().compare(QString("Izz_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[5] = readElementText().toDouble()*inertiaunit;
                }
                else if (name().compare(QString("Ixz_gain"), Qt::CaseInsensitive)==0)
                {
                    pWPolar->m_inertiaGain[6] = readElementText().toDouble()*inertiaunit;
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().compare(QString("Angle_gains"),         Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                int index = name().right(name().length()-5).toInt();
                pWPolar->m_ControlGain[index-1] = readElementText().toDouble();
            }
        }
        else
            skipCurrentElement();

    }
}







