/****************************************************************************

	Objects_global Class
	Copyright (C) 2017 Andre Deperrois adeperrois@xflr5.com

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

#ifndef OBJECTS_GLOBAL_H
#define OBJECTS_GLOBAL_H

#endif // OBJECTS_GLOBAL_H

#include "engine_params.h"
#include <objects2d/Vector3d.h>
#include <complex>

#include <QDataStream>
#include <QTextStream>
#include <QColor>

void readCString(QDataStream &ar, QString &strong);
void writeCString(QDataStream &ar, QString const &strong);
bool ReadAVLString(QTextStream &in, int &Line, QString &strong);


void readCOLORREF(QDataStream &ar, int &r, int &g, int &b);

void readqColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeqColor(QDataStream &ar, int r, int g, int b, int a);

void readFloat(QDataStream &inStream, float &f);
void writeFloat(QDataStream &outStream, float f);

void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &dsi);

bool Intersect(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &Normal,
                      Vector3d const &A,  Vector3d const &U,  Vector3d &I, double &dist);
