/****************************************************************************

    Core functions

    Copyright (C) 2008-2017 André Deperrois

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
#include <QTextStream>
#include <QStandardItem>
#include <QNetworkReply>
#include <QThread>


#include <xflcore/linestyle.h>
#include <xflcore/core_enums.h>


using namespace std;


namespace xfl
{
    extern bool g_bLocalize;

    extern int s_SymbolSize;
    extern QString s_LastDirName, s_xmlDirName, s_plrDirName;

    extern QVector <QColor> s_ColorList;

    QString versionName(bool bFull);

    inline QColor getColor(int index){return s_ColorList.at(index%s_ColorList.size());}

    inline void setLastDirName(QString dirname) {s_LastDirName=dirname;}
    inline QString const &lastDirName() {return s_LastDirName;}

    inline void setXmlDirName(QString dirname) {s_xmlDirName=dirname;}
    inline QString xmlDirName() {return s_xmlDirName;}

    inline void setPlrDirName(QString dirname) {s_plrDirName=dirname;}
    inline QString plrDirName() {return s_plrDirName;}

    float GLGetRed(float tau);
    float GLGetGreen(float tau);
    float GLGetBlue(float tau);


    QColor randomColor(bool bLightColor);
    inline QString colorNameARGB(QColor const &colour) {return QString::asprintf("rgba(%d,%d,%3d,%g)", colour.red(), colour.green(), colour.blue(), colour.alphaF());}


    xfl::enumPanelDistribution distributionType(QString const &strDist);
    QString distributionType(xfl::enumPanelDistribution dist);

    xfl::enumBodyLineType bodyPanelType(QString const &strPanelType);
    QString bodyPanelType(xfl::enumBodyLineType panelType);


    xfl::enumPolarType polarType(QString const &strPolarType);
    QString polarType(xfl::enumPolarType polarType);

    xfl::enumPolarType WPolarType(QString const &strPolarType);
    QString WPolarType(xfl::enumPolarType polarType);


    xfl::enumAnalysisMethod analysisMethod(QString const &strAnalysisMethod);
    QString analysisMethod(xfl::enumAnalysisMethod analysisMethod);


    xfl::enumBC boundaryCondition(QString const &strBC);
    QString boundaryCondition(xfl::enumBC boundaryCondition);


    QString referenceDimension(xfl::enumRefDimension refDimension);
    xfl::enumRefDimension referenceDimension(QString const &strRefDimension);

    xfl::enumWingType wingType(QString const &strWingType);
    QString wingType(xfl::enumWingType wingType);



    void ReynoldsFormat(QString &str, double f);



    void readColor(QDataStream &ar, int &r, int &g, int &b);
    void writeColor(QDataStream &ar, int r, int g, int b);

    void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
    void writeColor(QDataStream &ar, int r, int g, int b, int a);

    void readCOLORREF(QDataStream &ar, int &r, int &g, int &b);
    void readQColor(QDataStream &ar, int &r, int &g, int &b, int &a);
    void writeQColor(QDataStream &ar, int r, int g, int b, int a);


    int readValues(const QString &theline, double &x, double &y, double &z);
    void readFloat(QDataStream &inStream, float &f);
    void writeFloat(QDataStream &outStream, float f);
    bool readAVLString(QTextStream &in, int &Line, QString &strong);

    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPoint const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPointF const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, double x, double y);

//    Qt::PenStyle getStyle(int s);

    inline Qt::PenStyle getStyle(Line::enumLineStipple s)
    {
         switch(s)
         {
             default:
             case Line::SOLID:      return Qt::SolidLine;
             case Line::DASH:       return Qt::DashLine;
             case Line::DOT:        return Qt::DotLine;
             case Line::DASHDOT:    return Qt::DashDotLine;
             case Line::DASHDOTDOT: return Qt::DashDotDotLine;
             case Line::NOLINE:     return Qt::NoPen;
         }
    }


    void expFormat(double &f, int &exp);
    void ReynoldsFormat(QString &str, double f);

    QStringList findFiles(const QString &startDir, const QStringList &filters, bool bRecursive);
    bool findFile(QString const &filename, QString const &startDir, const QStringList &filters, bool bRecursive, QString &filePathName);

    void printDouble(QString msg, double d0, double d1=-2.0e50, double d2=-2.0e50, double d3=-2.0e50, double d4=-2.0e50, double d5=-2.0e50, double d6=-2.0e50, double d7=-2.0e50, double d8=-2.0e50, double d9=-2.0e50);
    void printDouble(double d0, double d1=-2.0e50, double d2=-2.0e50, double d3=-2.0e50, double d4=-2.0e50, double d5=-2.0e50, double d6=-2.0e50, double d7=-2.0e50, double d8=-2.0e50, double d9=-2.0e50);

    void writeCString(QDataStream &ar, QString const &strong);
    void readCString(QDataStream &ar, QString &strong);

    void readString(QDataStream &ar, QString &strong);
    void writeString(QDataStream &ar, QString const &strong);


    bool stringToBool(QString str);
    QString boolToString(bool b);

    // dummy function arguments used to create a QSettings xml file for the license
    bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map); // dummy argument function
    bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map); // dummy argument function

    void listSysInfo(QString &info);

    void getNetworkError(QNetworkReply::NetworkError neterror, QString &errorstring);


    QList<QStandardItem *> prepareRow(const QString &first, const QString &second=QString(), const QString &third=QString(),  const QString &fourth=QString());
    QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
    QList<QStandardItem *> prepareIntRow(const QString &first, const QString &second, const int &third);
    QList<QStandardItem *> prepareDoubleRow(const QString &first, const QString &second, const double &third,  const QString &fourth);
    QList<QStandardItem *> prepareDoubleRow(const QString &second, const double &value1, const double &value2, const QString &fourth);

    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPoint const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPointF const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, double x, double y);

    inline void setSymbolSize(int s) {s_SymbolSize=s;}
    inline int symbolSize() {return s_SymbolSize;}

    inline void setLocalized(bool bLocal) {g_bLocalize=bLocal;}
    inline bool isLocalized() {return g_bLocalize;}

}
