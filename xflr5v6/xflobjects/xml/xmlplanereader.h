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

#ifndef XMLPLANEREADER_H
#define XMLPLANEREADER_H

#include <QXmlStreamReader>
#include <QFile>

class Plane;
class Body;
class Wing;
class Vector3d;
class PointMass;

class XMLPlaneReader : public QXmlStreamReader
{
public:
    XMLPlaneReader(QFile &file, Plane *pPlane);

    bool readXMLPlaneFile();

private:
    bool readPlane(Plane *pPlane, double lengthUnit, double massUnit);
    bool readBody(Body *pBody, Vector3d &position, double lengthUnit, double massUnit);
    bool readWing(Wing &newwing, Vector3d &position, double &tiltangle, double lengthUnit, double massUnit);
    bool readPointMass(PointMass *ppm, double massUnit, double lengthUnit);
    bool readColor(QColor &color);

    Plane *m_pPlane;
};

#endif // XMLPLANEREADER_H
