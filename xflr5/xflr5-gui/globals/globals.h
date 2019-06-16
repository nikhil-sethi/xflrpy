/****************************************************************************

    Global functions

    Copyright (C) 2008-2017 Andre Deperrois

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

/**
* @file
* This file contains the declaration of methods used throughout the program and not specific to one application.
*/

#pragma once

#include <QString>
#include <QColor>
#include <QFile>
#include <QPainter>
#include <complex>


#include "analysis3d/analysis3d_enums.h"
#include <objects/objectcolor.h>


using namespace std;

class Foil;
class Polar;
class OpPoint;
class Plane;
class WPolar;

int readValues(QString line, double &x, double &y, double &z);

void ExpFormat(double &f, int &exp);
void ReynoldsFormat(QString &str, double f);


float GLGetRed(float tau);
float GLGetGreen(float tau);
float GLGetBlue(float tau);


QColor randomColor(bool bLightColor);

void Trace(int n);
void Trace(QString msg);
void Trace(QString msg, bool b);
void Trace(QString msg, int n);
void Trace(QString msg, double f);



XFLR5::enumPanelDistribution distributionType(QString strDist);
QString distributionType(XFLR5::enumPanelDistribution dist);

XFLR5::enumBodyLineType bodyPanelType(QString strPanelType);
QString bodyPanelType(XFLR5::enumBodyLineType panelType);


XFLR5::enumPolarType polarType(QString strPolarType);
QString polarType(XFLR5::enumPolarType polarType);

XFLR5::enumPolarType WPolarType(QString strPolarType);
QString WPolarType(XFLR5::enumPolarType polarType);


XFLR5::enumAnalysisMethod analysisMethod(QString strAnalysisMethod);
QString analysisMethod(XFLR5::enumAnalysisMethod analysisMethod);


XFLR5::enumBC boundaryCondition(QString strBC);
QString boundaryCondition(XFLR5::enumBC boundaryCondition);

Foil *readFoilFile(QFile &xFoilFile);
Foil *readPolarFile(QFile &plrFile, QList<Polar*> &polarList);

void drawFoil(QPainter &painter, Foil*pFoil, double const &alpha, double const &scalex, double const &scaley, QPointF const &Offset);
void drawMidLine(QPainter &painter, Foil*pFoil, double const &scalex, double const &scaley, QPointF const &Offset);
void drawPoints(QPainter &painter, Foil*pFoil, double alpha, double const &scalex, double const &scaley, QPointF const &Offset, QColor backColor);
void drawPoint(QPainter &painter, int pointStyle, QColor bkColor, QPoint pt);

bool stringToBool(QString str);
QString referenceDimension(XFLR5::enumRefDimension refDimension);
XFLR5::enumRefDimension referenceDimension(QString strRefDimension);

XFLR5::enumWingType wingType(QString strWingType);
QString wingType(XFLR5::enumWingType wingType);

void setAutoWPolarName(WPolar * pWPolar, Plane *pPlane);


void ReynoldsFormat(QString &str, double f);

QColor getColor(int r, int g, int b, int a=255);
QColor colour(OpPoint *pOpp);
QColor colour(Polar *pPolar);
QColor colour(Foil *pFoil);
void setRandomFoilColor(Foil *pFoil, bool bLightTheme);




void readString(QDataStream &ar, QString &strong);
void writeString(QDataStream &ar, QString const &strong);
void readColor(QDataStream &ar, int &r, int &g, int &b);
void writeColor(QDataStream &ar, int r, int g, int b);

void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeColor(QDataStream &ar, int r, int g, int b, int a);


bool serializeFoil(Foil*pFoil, QDataStream &ar, bool bIsStoring);
bool serializePolar(Polar *pPolar, QDataStream &ar, bool bIsStoring);

QColor color(ObjectColor clr);
