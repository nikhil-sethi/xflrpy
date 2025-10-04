/****************************************************************************

    Core functions

    Copyright (C) André Deperrois

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


#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QNetworkInterface>
#include <QPen>
#include <QRandomGenerator>
#include <QTextStream>


#include <xflcore/core_enums.h>
#include <xflcore/gui_params.h>
#include <xflcore/linestyle.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>


bool xfl::g_bLocalize = false;
int xfl::s_SymbolSize = 3;

QString xfl::s_LastDirName = QDir::homePath();
QString xfl::s_xmlDirName  = QDir::homePath();
QString xfl::s_plrDirName  = QDir::homePath();


QVector <QColor> xfl::s_ColorList = {{ 85, 170, 255},
                                     {215,  75,  65},
                                     {125, 195, 105},
                                     {215, 135, 135},
                                     { 85,  85, 170},
                                     {255, 70,  200},
                                     {165,  55, 255},
                                     { 70, 125, 255},
                                     {153,  85,  36},
                                     {215, 215,  75}};


/**
* Returns the red component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the red component of the color
*/
float xfl::GLGetRed(float tau)
{
    if(tau>2.0f/3.0f)      return 1.0f;
    else if(tau>1.0f/3.0f) return (3.0f*(tau-1.0f/3.0f));
    else                   return 0.0;
}


/**
* Returns the green component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the green component of the color
*/
float xfl::GLGetGreen(float tau)
{
    if(tau<0.f || tau>1.0f)     return 0.0f;
    else if(tau<1.0f/4.0f)     return (4.0f*tau);
    else if(tau>3.0f/4.0f)     return (1.0f-4.0f*(tau-3.0f/4.0f));
    else                    return 1.0f;
}


/**
* Returns the blue component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the blue component of the color
*/
float xfl::GLGetBlue(float tau)
{
    if(tau>2.0f/3.0f)      return 0.0;
    else if(tau>1.0f/3.0f) return (1.0f-3.0f*(tau-1.0f/3.0f));
    else                   return 1.0;
}





QColor xfl::randomColor(bool bLightColor)
{
    QColor clr;

    int h = QRandomGenerator::global()->bounded(360);
    int s = QRandomGenerator::global()->bounded(155)+100;
    int v = QRandomGenerator::global()->bounded(80)+120;
    if(bLightColor) v += 55;

    clr.setHsv(h,s,v,255);

    return clr;
}


xfl::enumPanelDistribution xfl::distributionType(QString const &strDist)
{
    if(strDist.compare("COSINE",           Qt::CaseInsensitive)==0) return xfl::COSINE;
    else if(strDist.compare("UNIFORM",     Qt::CaseInsensitive)==0) return xfl::UNIFORM;
    else if(strDist.compare("SINE",        Qt::CaseInsensitive)==0) return xfl::SINE;
    else if(strDist.compare("INVERSESINE", Qt::CaseInsensitive)==0) return xfl::INVERSESINE;
    else return xfl::UNIFORM;
}


QString xfl::distributionType(xfl::enumPanelDistribution dist)
{
    switch(dist)
    {
        case xfl::COSINE: return "COSINE";
        case xfl::UNIFORM: return "UNIFORM";
        case xfl::SINE: return "SINE";
        case xfl::INVERSESINE: return "INVERSE SINE";
    }
    return QString();
}



xfl::enumBodyLineType xfl::bodyPanelType(QString const &strPanelType)
{
    if(strPanelType.compare("FLATPANELS", Qt::CaseInsensitive)==0) return xfl::BODYPANELTYPE;
    else                                                           return xfl::BODYSPLINETYPE;
}

QString xfl::bodyPanelType(xfl::enumBodyLineType panelType)
{
    switch(panelType)
    {
        case xfl::BODYPANELTYPE:  return "FLATPANELS";
        case xfl::BODYSPLINETYPE: return "NURBS";
    }
    return QString();
}


xfl::enumPolarType xfl::polarType(const QString &strPolarType)
{
    if     (strPolarType.compare("FIXEDSPEEDPOLAR",   Qt::CaseInsensitive)==0) return xfl::FIXEDSPEEDPOLAR;
    else if(strPolarType.compare("FIXEDLIFTPOLAR",    Qt::CaseInsensitive)==0) return xfl::FIXEDLIFTPOLAR;
    else if(strPolarType.compare("RUBBERCHORDPOLAR",  Qt::CaseInsensitive)==0) return xfl::RUBBERCHORDPOLAR;
    else if(strPolarType.compare("FIXEDAOAPOLAR",     Qt::CaseInsensitive)==0) return xfl::FIXEDAOAPOLAR;
    else return xfl::FIXEDSPEEDPOLAR;
}

QString xfl::polarType(xfl::enumPolarType polarType)
{
    switch(polarType)
    {
        case xfl::FIXEDSPEEDPOLAR:  return "FIXEDSPEEDPOLAR";
        case xfl::FIXEDLIFTPOLAR:   return "FIXEDLIFTPOLAR";
        case xfl::RUBBERCHORDPOLAR: return "RUBBERCHORDPOLAR";
        case xfl::FIXEDAOAPOLAR:    return "FIXEDAOAPOLAR";
        default: return "";
    }
}



xfl::enumPolarType xfl::WPolarType(const QString &strPolarType)
{
    if     (strPolarType.compare("FIXEDSPEEDPOLAR", Qt::CaseInsensitive)==0) return xfl::FIXEDSPEEDPOLAR;
    else if(strPolarType.compare("FIXEDLIFTPOLAR",  Qt::CaseInsensitive)==0) return xfl::FIXEDLIFTPOLAR;
    else if(strPolarType.compare("FIXEDAOAPOLAR",   Qt::CaseInsensitive)==0) return xfl::FIXEDAOAPOLAR;
    else if(strPolarType.compare("STABILITYPOLAR",  Qt::CaseInsensitive)==0) return xfl::STABILITYPOLAR;
    else if(strPolarType.compare("BETAPOLAR",       Qt::CaseInsensitive)==0) return xfl::BETAPOLAR;
    else return xfl::FIXEDSPEEDPOLAR;
}

QString xfl::WPolarType(xfl::enumPolarType polarType)
{
    switch(polarType)
    {
        case xfl::FIXEDSPEEDPOLAR:  return "FIXEDSPEEDPOLAR";
        case xfl::FIXEDLIFTPOLAR:   return "FIXEDLIFTPOLAR";
        case xfl::FIXEDAOAPOLAR:    return "FIXEDAOAPOLAR";
        case xfl::STABILITYPOLAR:   return "STABILITYPOLAR";
        case xfl::BETAPOLAR:        return "BETAPOLAR";
        default: return "";
    }
}


xfl::enumAnalysisMethod xfl::analysisMethod(const QString &strAnalysisMethod)
{
    if     (strAnalysisMethod.compare("LLTMETHOD",   Qt::CaseInsensitive)==0) return xfl::LLTMETHOD;
    else if(strAnalysisMethod.compare("VLMMETHOD",   Qt::CaseInsensitive)==0) return xfl::VLMMETHOD;
    else if(strAnalysisMethod.compare("PANELMETHOD", Qt::CaseInsensitive)==0) return xfl::PANEL4METHOD;
    else return xfl::VLMMETHOD;
}


QString xfl::analysisMethod(xfl::enumAnalysisMethod analysisMethod)
{
    switch(analysisMethod)
    {
        case xfl::LLTMETHOD:   return "LLTMETHOD";
        case xfl::VLMMETHOD:   return "VLMMETHOD";
        case xfl::PANEL4METHOD: return "PANELMETHOD";
        default: return "";
    }
}



xfl::enumBC xfl::boundaryCondition(const QString &strBC)
{
    if   (strBC.compare("DIRICHLET", Qt::CaseInsensitive)==0) return xfl::DIRICHLET;
    else                                                      return xfl::NEUMANN;
}

QString xfl::boundaryCondition(xfl::enumBC boundaryCondition)
{
    switch(boundaryCondition)
    {
        case xfl::DIRICHLET: return "DIRICHLET";
        case xfl::NEUMANN:   return "NEUMANN";
    }
    return "DIRICHLET";
}


QString xfl::referenceDimension(xfl::enumRefDimension refDimension)
{
    switch(refDimension)
    {
        case xfl::PLANFORMREFDIM:  return "PLANFORMREFDIM";
        case xfl::PROJECTEDREFDIM: return "PROJECTEDREFDIM";
        case xfl::MANUALREFDIM:    return "MANUALREFDIM";
    }
    return QString();
}


xfl::enumRefDimension xfl::referenceDimension(const QString &strRefDimension)
{
    if     (strRefDimension.compare("PLANFORMREFDIM",  Qt::CaseInsensitive)==0) return xfl::PLANFORMREFDIM;
    else if(strRefDimension.compare("PROJECTEDREFDIM", Qt::CaseInsensitive)==0) return xfl::PROJECTEDREFDIM;
    else if(strRefDimension.compare("MANUALREFDIM",    Qt::CaseInsensitive)==0) return xfl::MANUALREFDIM;
    else return xfl::PLANFORMREFDIM;
}






xfl::enumWingType xfl::wingType(const QString &strWingType)
{
    if     (strWingType.compare("MAINWING",   Qt::CaseInsensitive)==0) return xfl::MAINWING;
    else if(strWingType.compare("SECONDWING", Qt::CaseInsensitive)==0) return xfl::SECONDWING;
    else if(strWingType.compare("ELEVATOR",   Qt::CaseInsensitive)==0) return xfl::ELEVATOR;
    else if(strWingType.compare("FIN",        Qt::CaseInsensitive)==0) return xfl::FIN;
    else                                                               return xfl::OTHERWING;
}

QString xfl::wingType(xfl::enumWingType wingType)
{
    switch(wingType)
    {
        case xfl::MAINWING:   return "MAINWING";
        case xfl::SECONDWING: return "SECONDWING";
        case xfl::ELEVATOR:   return "ELEVATOR";
        case xfl::FIN:        return "FIN";
        case xfl::OTHERWING:  return "OTHERWING";
    }
    return "OTHERWING";
}

/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
* @param ar the binary datastream
* @param r the red component
* @param g the green component
* @param b the blue component
* @param a the alpha component
*/
void xfl::readQColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
    uchar byte=0;

    ar>>byte;//probably a format identificator
    ar>>byte>>byte;
    a = int(byte);
    ar>>byte>>byte;
    r = int(byte);
    ar>>byte>>byte;
    g = int(byte);
    ar>>byte>>byte;
    b = int(byte);
    ar>>byte>>byte; //
}


/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
* @param ar the binary datastream
* @param r the red component
* @param g the green component
* @param b the blue component
* @param a the alpha component
*/
void xfl::writeQColor(QDataStream &ar, int r, int g, int b, int a)
{
    uchar byte;

    byte = 1;
    ar<<byte;
    byte = a & 0xFF;
    ar << byte<<byte;
    byte = r & 0xFF;
    ar << byte<<byte;
    byte = g & 0xFF;
    ar << byte<<byte;
    byte = b & 0xFF;
    ar << byte<<byte;
    byte = 0;
    ar << byte<<byte;
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void xfl::readCOLORREF(QDataStream &ar, int &r, int &g, int &b)
{
    qint32 colorref;

    ar >> colorref;
    b = int(colorref/256/256);
    colorref -= b*256*256;
    g = int(colorref/256);
    r = colorref - g*256;
}

/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void xfl::readColor(QDataStream &ar, int &r, int &g, int &b)
{
    qint32 colorref;

    ar >> colorref;
    b = colorref/256/256;
    colorref -= b*256*256;
    g = colorref/256;
    r = colorref - g*256;
}


/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component

*/
void xfl::writeColor(QDataStream &ar, int r, int g, int b)
{
    qint32 colorref;

    colorref = b*256*256+g*256+r;
    ar << colorref;
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void xfl::readColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
    uchar byte=0;

    ar>>byte;//probably a format identificator
    ar>>byte>>byte;
    a = int(byte);
    ar>>byte>>byte;
    r = int(byte);
    ar>>byte>>byte;
    g = int(byte);
    ar>>byte>>byte;
    b = int(byte);
    ar>>byte>>byte; //
}

/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void xfl::writeColor(QDataStream &ar, int r, int g, int b, int a)
{
    uchar byte;

    byte = 1;
    ar<<byte;
    byte = a & 0xFF;
    ar << byte<<byte;
    byte = r & 0xFF;
    ar << byte<<byte;
    byte = g & 0xFF;
    ar << byte<<byte;
    byte = b & 0xFF;
    ar << byte<<byte;
    byte = 0;
    ar << byte<<byte;
}


QString xfl::versionName(bool bFull)
{
    QString VName;
    VName = QString::asprintf("v%d.%02d", MAJOR_VERSION, MINOR_VERSION);
    if(bFull) VName = "xflr5 "+VName;
    return VName;
}


 /**
 * Takes a double number holding the value of a Reynolds number and returns a string.
 *@param str the return string  with the formatted number
 *@param f the Reynolds number to be formatted
 */
 void xfl::ReynoldsFormat(QString &str, double f)
 {
     f = (int(f/1000.0))*1000.0;

     int exp = int(log10(f));
     int r = exp%3;
     int q = (exp-r)/3;

     QString strong;
     strong = QString("%1").arg(f,0,'f',0);

     int l = strong.length();

     for (int i=0; i<q; i++){
         strong.insert(l-3*(i+1)-i," ");
         l++;
     }

     for (int i=strong.length(); i<9; i++){
         strong = " "+strong;
     }

     str = strong;
 }


/**
* Returns a double number as its root and its base 10 exponent
* @param f the double number to reformat; is returned as f = f/pow(10.0,exp);
* @param exp the base 10 exponent of f.
*/
void xfl::expFormat(double &f, int &exp)
{
    if (f==0.0)
    {
        exp = 0;
        f = 0.0;
        return;
    }
    double f1 = fabs(f);
    if(f1<1)
        exp = int(log10(f1)-1);
    else
        exp = int(log10(f1));

    f = f/pow(10.0,exp);

    if(fabs(f-10.0)<0.00001)
    {
        f = +1.0;
        exp++;
    }
    else if(fabs(f+10.0)<0.00001)
    {
        f = -1.0;
        exp++;
    }
}



/** from Qt examples WordCount
 * startDir = QDir::home().absolutePath()
 * filters = QStringList() << "*.cpp" << "*.h" ;
*/
QStringList xfl::findFiles(const QString &startDir, QStringList const &filters, bool bRecursive)
{
    QStringList names;
    QDir dir(startDir);

    foreach (QString file, dir.entryList(filters, QDir::Files))
    {
        names += startDir + '/' + file;
    }

    if(bRecursive)
    {
        foreach (QString subdir, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            names += findFiles(startDir + '/' + subdir, filters, bRecursive);
        }
    }

    return names;
}


bool xfl::findFile(QString const &filename, QString const &startDir, QStringList const &filters, bool bRecursive, QString &filePathName)
{
    QDir dir(startDir);

    foreach (QString file, dir.entryList(filters, QDir::Files))
    {
        if(file.compare(filename, Qt::CaseInsensitive)==0)
        {
            filePathName = startDir + '/' + file;
            return true;
        }
    }

    if(bRecursive)
    {
        foreach (QString subdir, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            if(findFile(filename, startDir + '/' + subdir, filters, bRecursive, filePathName))
                return true;
        }
    }

    return false;
}



void xfl::printDouble(QString msg, double d0, double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, double d9)
{
    QString strong, str;
    strong = msg;
    str = QString::asprintf("  %13.7g", d0);
    strong += str;
    if(d1>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d1);
        strong += str;
    }
    if(d2>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d2);
        strong += str;
    }
    if(d3>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d3);
        strong += str;
    }
    if(d4>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d4);
        strong += str;
    }
    if(d5>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d5);
        strong += str;
    }
    if(d6>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d6);
        strong += str;
    }
    if(d7>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d7);
        strong += str;
    }
    if(d8>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d8);
        strong += str;
    }
    if(d9>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d9);
        strong += str;
    }

    qDebug("%s", strong.toStdString().c_str());
}


void xfl::printDouble(double d0, double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, double d9)
{
    QString strong, str;
    strong = QString::asprintf("  %13.7g", d0);
    if(d1>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d1);
        strong += str;
    }
    if(d2>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d2);
        strong += str;
    }
    if(d3>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d3);
        strong += str;
    }
    if(d4>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d4);
        strong += str;
    }
    if(d5>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d5);
        strong += str;
    }
    if(d6>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d6);
        strong += str;
    }
    if(d7>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d7);
        strong += str;
    }
    if(d8>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d8);
        strong += str;
    }
    if(d9>-1.0e50)
    {
        str = QString::asprintf("  %13.7g", d9);
        strong += str;
    }

    qDebug("%s", strong.toStdString().c_str());
}


/**
* Reads a sequence of characters from a binary stream and returns a QString. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString read from the stream
*/
void xfl::readString(QDataStream &ar, QString &strong)
{
    qint8 qi(0), ch(0);
    char c(0);

    ar >> qi;
    strong.clear();
    for(int j=0; j<qi;j++)
    {
        strong += " ";
        ar >> ch;
        c = char(ch);
        strong[j] = c;
    }
}


/**
* Writes a sequence of characters from a QStrinf to a binary stream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString to output to the stream
*/
void xfl::writeString(QDataStream &ar, QString const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}



/**
* Reads a sequence of characters from a binary stream and returns a QString. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString read from the stream
*/
void xfl::readCString(QDataStream &ar, QString &strong)
{
    qint8 qi, ch;
    char c;

    ar >> qi;
    strong.clear();
    for(int j=0; j<qi;j++)
    {
        strong += " ";
        ar >> ch;
        c = char(ch);
        strong[j] = c;
    }
}


/**
* Writes a sequence of characters from a QString to a binary stream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString to output to the stream
*/
void xfl::writeCString(QDataStream &ar, QString const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}


bool xfl::stringToBool(QString str)
{
    return str.trimmed().compare("true", Qt::CaseInsensitive)==0 ? true : false;
}


QString xfl::boolToString(bool b)
{
    return b ? "true" : "false";
}


/**
* Extracts three double values from a QString, and returns the number of extracted values.
*/
int xfl::readValues(QString const &theline, double &x, double &y, double &z)
{
    int res=0;

    QString line, str;
    bool bOK=false;

    line = theline.simplified();
    int pos = line.indexOf(" ");

    if(pos>0)
    {
        str = line.left(pos);
        line = line.right(line.length()-pos);
    }
    else
    {
        str = line;
        line = "";
    }
    x = str.toDouble(&bOK);
    if(bOK) res++;
    else
    {
        y=z=0.0;
        return res;
    }

    line = line.trimmed();
    pos = line.indexOf(" ");
    if(pos>0)
    {
        str = line.left(pos);
        line = line.right(line.length()-pos);
    }
    else
    {
        str = line;
        line = "";
    }
    y = str.toDouble(&bOK);
    if(bOK) res++;
    else
    {
        z=0.0;
        return res;
    }

    line = line.trimmed();
    if(!line.isEmpty())
    {
        z = line.toDouble(&bOK);
        if(bOK) res++;
    }
    else z=0.0;

    return res;
}


void xfl::readFloat(QDataStream &inStream, float &f)
{
    char buffer[4];
    inStream.readRawData(buffer, 4);
    memcpy(&f, buffer, sizeof(float));
}


void xfl::writeFloat(QDataStream &outStream, float f)
{
    char buffer[4];
    memcpy(buffer, &f, sizeof(float));
    outStream.writeRawData(buffer, 4);
}


/**
 * Reads one line from an AVL-format text file
 */
bool xfl::readAVLString(QTextStream &in, int &Line, QString &strong)
{
    bool isCommentLine = true;
    int pos=0;
    if(in.atEnd()) return false;

    while(isCommentLine && !in.atEnd())
    {
        isCommentLine = false;

        strong = in.readLine();

        strong = strong.trimmed();
        pos = strong.indexOf("#",0);
        if(pos>=0) strong = strong.left(pos);
        pos = strong.indexOf("!",0);
        if(pos>=0) strong = strong.left(pos);

        if(strong.isEmpty()) isCommentLine = true;

        Line++;
    }

    return true;
}

bool xfl::readXmlFile(QIODevice &, QSettings::SettingsMap &) {return true;}
bool xfl::writeXmlFile(QIODevice &, const QSettings::SettingsMap &) {return true;}



void xfl::listSysInfo(QString &info)
{
    info.clear();
    QString prefix = "   ";
    info += "System info:";
    info += prefix + "bootUniqueId:            " + QSysInfo::bootUniqueId();
    info += prefix + "buildAbi:                " + QSysInfo::buildAbi(); // Application Binary Interface
    info += prefix + "buildCpuArchitecture:    " + QSysInfo::buildCpuArchitecture();
    info += prefix + "currentCpuArchitecture:  " + QSysInfo::currentCpuArchitecture();
    info += prefix + "kernelType:              " + QSysInfo::kernelType();
    info += prefix + "kernelVersion:           " + QSysInfo::kernelVersion();
    info += prefix + "machineHostName:         " + QSysInfo::machineHostName();
    info += prefix + "machineUniqueId:         " + QSysInfo::machineUniqueId();
    info += prefix + "prettyProductName:       " + QSysInfo::prettyProductName();
    info += prefix + "productType:             " + QSysInfo::productType();
    info += prefix + "productVersion:          " + QSysInfo::productVersion();
    info += "\n";


    info += "MAC adresses:";
    foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the first non-loopback MAC Address
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack))
            info += prefix + netInterface.hardwareAddress();
    }
    info += "\n";
}


void xfl::getNetworkError(QNetworkReply::NetworkError neterror, QString &errorstring)
{
    switch(neterror)
    {
        case QNetworkReply::NoError:
            errorstring = "no error condition.";
            break;
        case QNetworkReply::ConnectionRefusedError:
            errorstring = "The remote server refused the connection (the server is not accepting requests)";
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorstring = "The remote server closed the connection prematurely, before the entire reply was received and processed";
            break;
        case QNetworkReply::HostNotFoundError:
            errorstring = "The remote host name was not found (invalid hostname)";
            break;
        case QNetworkReply::TimeoutError:
            errorstring = "The connection to the remote server timed out";
            break;
        case QNetworkReply::OperationCanceledError:
            errorstring = "The operation was canceled via calls to abort() or close() before it was finished.";
            break;
        case QNetworkReply::SslHandshakeFailedError:
            errorstring = "The SSL/TLS handshake failed and the encrypted channel could not be established. The sslErrors() signal should have been emitted.";
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
            errorstring = "The connection was broken due to disconnection from the network, however the system has initiated roaming to another access point."
                  " The request should be resubmitted and will be processed as soon as the connection is re-established.";
            break;
        case QNetworkReply::NetworkSessionFailedError:
            errorstring = "The connection was broken due to disconnection from the network or failure to start the network.";
            break;
        case QNetworkReply::BackgroundRequestNotAllowedError:
            errorstring = "The background request is not currently allowed due to platform policy.";
            break;
        case QNetworkReply::TooManyRedirectsError:
            errorstring = "While following redirects, the maximum limit was reached. "
                  "The limit is by default set to 50 or as set by QNetworkRequest::setMaxRedirectsAllowed(). "
                  "(This value was introduced in 5.6.)";
            break;
        case QNetworkReply::InsecureRedirectError:
            errorstring = "While following redirects, the network access API detected a redirect from a encrypted protocol (https) "
                  "to an unencrypted one (http). (This value was introduced in 5.6.)";
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
            errorstring = "The connection to the proxy server was refused (the proxy server is not accepting requests)";
            break;
        case QNetworkReply::ProxyConnectionClosedError:
            errorstring = "The proxy server closed the connection prematurely, before the entire reply was received and processed";
            break;
        case QNetworkReply::ProxyNotFoundError:
            errorstring = "The proxy host name was not found (invalid proxy hostname)";
            break;
        case QNetworkReply::ProxyTimeoutError:
            errorstring = "The connection to the proxy timed out or the proxy did not reply in time to the request sent";
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
            errorstring = "The proxy requires authentication in order to honour the request but did not accept any credentials "
                  "offered (if any)";
            break;
        case QNetworkReply::ContentAccessDenied:
            errorstring = "The access to the remote content was denied (similar to HTTP error 403)";
            break;
        case QNetworkReply::ContentOperationNotPermittedError:
            errorstring = "The operation requested on the remote content is not permitted";
            break;
        case QNetworkReply::ContentNotFoundError:
            errorstring = "The remote content was not found at the server (similar to HTTP error 404)";
            break;
        case QNetworkReply::AuthenticationRequiredError:
            errorstring = "The remote server requires authentication to serve the content but the credentials provided "
                  "were not accepted (if any)";
            break;
        case QNetworkReply::ContentReSendError:
            errorstring = "The request needed to be sent again, but this failed for example because the upload data "
                  "could not be read a second time.";
            break;
        case QNetworkReply::ContentConflictError:
            errorstring = "The request could not be completed due to a conflict with the current state of the resource.";
            break;
        case QNetworkReply::ContentGoneError:
            errorstring = "The requested resource is no longer available at the server.";
            break;
        case QNetworkReply::InternalServerError:
            errorstring = "The server encountered an unexpected condition which prevented it from fulfilling the request.";
            break;
        case QNetworkReply::OperationNotImplementedError:
            errorstring = "The server does not support the functionality required to fulfill the request.";
            break;
        case QNetworkReply::ServiceUnavailableError:
            errorstring = "The server is unable to handle the request at this time.";
            break;
        case QNetworkReply::ProtocolUnknownError:
            errorstring = "The Network Access API cannot honor the request because the protocol is not known";
            break;
        case QNetworkReply::ProtocolInvalidOperationError:
            errorstring = "The requested operation is invalid for this protocol";
            break;
        case QNetworkReply::UnknownNetworkError:
            errorstring = "An unknown network-related error was detected";
            break;
        case QNetworkReply::UnknownProxyError:
            errorstring = "An unknown proxy-related error was detected";
            break;
        case QNetworkReply::UnknownContentError:
            errorstring = "An unknown error related to the remote content was detected";
            break;
        case QNetworkReply::ProtocolFailure:
            errorstring = "A breakdown in protocol was detected (parsing error, invalid or unexpected responses, etc.)";
            break;
        case QNetworkReply::UnknownServerError:
            errorstring = "An unknown error related to the server response was detected";
            break;
    }
}



QList<QStandardItem*> xfl::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
    for(int ii=0; ii<rowItems.size(); ii++)
        rowItems.at(ii)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem*> xfl::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::BOOLVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> xfl::prepareIntRow(const QString &object, const QString &field, const int &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::INTEGER, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> xfl::prepareDoubleRow(const QString &object, const QString &field, const double &value, const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> xfl::prepareDoubleRow(const QString &field, const double &value1, const double &value2, const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.append(new QStandardItem);
    rowItems.at(1)->setData(value1, Qt::DisplayRole);
    rowItems.at(2)->setData(value2, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}



void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, QColor const &linecolor, QPoint const &pt)
{
    xfl::drawSymbol(painter, ptstyle, backcolor, linecolor, double(pt.x()), double(pt.y()));
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, QColor const &linecolor, QPointF const & ptf)
{
    xfl::drawSymbol(painter, ptstyle, backcolor, linecolor, ptf.x(), ptf.y());
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, QColor const &linecolor, double x, double y)
{
    painter.save();
    painter.setBackgroundMode(Qt::TransparentMode);

    QPen Pointer(painter.pen());

    painter.setPen(Pointer);

    QColor bck(backcolor);
    bck.setAlpha(255);

    switch(ptstyle)
    {
        case Line::NOSYMBOL: break;
        case Line::LITTLECIRCLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize);
            painter.drawEllipse(QPointF(x,y), ptSide*1.2, ptSide*1.2);
            break;
        }
        case Line::LITTLECIRCLE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize);
            painter.drawEllipse(QPointF(x,y), ptSide*1.2, ptSide*1.2);
            break;
        }
        case Line::BIGCIRCLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize)*1.75;
            painter.drawEllipse(QPointF(x,y), ptSide, ptSide);
            break;
        }
        case Line::BIGCIRCLE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize)*1.75;
            painter.drawEllipse(QPointF(x,y), ptSide, ptSide);
            break;
        }
        case Line::LITTLESQUARE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize)*1.1;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::LITTLESQUARE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize)*1.1;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::BIGSQUARE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize)*1.7;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::BIGSQUARE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(s_SymbolSize)*1.7;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::TRIANGLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(s_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,        y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(s_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,        y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_INV:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(s_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,        y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_INV_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(s_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,        y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::LITTLECROSS:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            int ptSide = s_SymbolSize;
            painter.drawLine(int(x)-ptSide, int(y)-ptSide, int(x)+ptSide, int(y)+ptSide);
            painter.drawLine(int(x)-ptSide, int(y)+ptSide, int(x)+ptSide, int(y)-ptSide);
            break;
        }
        case Line::BIGCROSS:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            int ptSide = int(2.0*double(s_SymbolSize)*0.85);
            painter.drawLine(int(x)-ptSide, int(y)-ptSide, int(x)+ptSide, int(y)+ptSide);
            painter.drawLine(int(x)-ptSide, int(y)+ptSide, int(x)+ptSide, int(y)-ptSide);
            break;
        }
//        default: break;
    }
    painter.restore();
}




/*
Qt::PenStyle xfl::getStyle(int s)
{
    if     (s==0) return Qt::SolidLine;
    else if(s==1) return Qt::DashLine;
    else if(s==2) return Qt::DotLine;
    else if(s==3) return Qt::DashDotLine;
    else if(s==4) return Qt::DashDotDotLine;
    return Qt::NoPen;
}
*/

