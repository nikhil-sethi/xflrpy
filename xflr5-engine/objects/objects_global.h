/****************************************************************************

    Objects_global Class
    Copyright (C) 2017 Andre Deperrois 

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

#include <analysis3d/analysis3d_params.h>
#include <objects/objects3d/vector3d.h>
#include <complex>

#include <QDataStream>
#include <QTextStream>

void XFLR5ENGINELIBSHARED_EXPORT readCString(QDataStream &ar, QString &strong);
void writeCString(QDataStream &ar, QString const &strong);
bool XFLR5ENGINELIBSHARED_EXPORT ReadAVLString(QTextStream &in, int &Line, QString &strong);


void XFLR5ENGINELIBSHARED_EXPORT readCOLORREF(QDataStream &ar, int &r, int &g, int &b);

void readQColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeQColor(QDataStream &ar, int r, int g, int b, int a);

void XFLR5ENGINELIBSHARED_EXPORT readFloat(QDataStream &inStream, float &f);
void writeFloat(QDataStream &outStream, float f);

void XFLR5ENGINELIBSHARED_EXPORT modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &dsi);

bool XFLR5ENGINELIBSHARED_EXPORT Intersect(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &Normal,
                      Vector3d const &A,  Vector3d const &U,  Vector3d &I, double &dist);

