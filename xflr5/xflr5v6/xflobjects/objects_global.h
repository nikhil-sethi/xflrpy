/****************************************************************************

    Objects_global Class
    Copyright (C) 2017 André Deperrois 

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

#pragma once

#include <xflanalysis/analysis3d_params.h>
#include <xflgeom/geom3d/vector3d.h>
#include <complex>

#include <QDataStream>
#include <QTextStream>
#include <QPainter>
#include <QFile>

class Foil;
class Polar;
class WPolar;
class Plane;

namespace xfl
{
    QColor getObjectColor(int type);

    bool intersect(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &Normal,
                   Vector3d const &A,  Vector3d const &U,  Vector3d &I, double &dist);

    Foil *readFoilFile(QFile &xFoilFile);
    Foil *readPolarFile(QFile &plrFile, QVector<Polar*> &polarList);

    void drawFoil(QPainter &painter, const Foil *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset);
    void drawMidLine(QPainter &painter, Foil const*pFoil, double scalex, double scaley, QPointF const &Offset);
    void drawFoilPoints(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley, QPointF const &Offset, QColor const &backColor);

    void setAutoWPolarName(WPolar * pWPolar, Plane *pPlane);
    void setRandomFoilColor(Foil *pFoil, bool bLightTheme);
    bool serializeFoil(Foil*pFoil, QDataStream &ar, bool bIsStoring);
    bool serializePolar(Polar *pPolar, QDataStream &ar, bool bIsStoring);


}
