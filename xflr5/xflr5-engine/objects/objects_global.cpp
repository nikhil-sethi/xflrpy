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

#include "objects_global.h"
#include <objects/objects2d/foil.h>
#include <objects/objects2d/polar.h>
/**
* Reads a sequence of characters from a binary stream and returns a QString. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString read from the stream
*/
void readCString(QDataStream &ar, QString &strong)
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
* Writes a sequence of characters from a QStrinf to a binary stream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString to output to the stream
*/
void writeCString(QDataStream &ar, QString const &strong)
{
    qint8 qi = strong.length();

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}

void readFloat(QDataStream &inStream, float &f)
{
    char buffer[4];
    inStream.readRawData(buffer, 4);
    memcpy(&f, buffer, sizeof(float));
}


void writeFloat(QDataStream &outStream, float f)
{
    char buffer[4];
    memcpy(buffer, &f, sizeof(float));
    outStream.writeRawData(buffer, 4);
}



/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
* @param ar the binary datastream
* @param r the red component
* @param g the green component
* @param b the blue component
* @param a the alpha component
*/
void readQColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
    uchar byte=0;

    ar>>byte;//probably a format identificator
    ar>>byte>>byte;
    a = (int)byte;
    ar>>byte>>byte;
    r = (int)byte;
    ar>>byte>>byte;
    g = (int)byte;
    ar>>byte>>byte;
    b = (int)byte;
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
void writeQColor(QDataStream &ar, int r, int g, int b, int a)
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
*Reads one line from an AVL-format text file
*@deprecated the option to map AVL data was too comlplex and has been disabled.
*/
bool ReadAVLString(QTextStream &in, int &Line, QString &strong)
{
    bool bComment = true;
    int pos;

    while(bComment && !in.atEnd())
    {
        bComment = false;

        strong = in.readLine();
        if(in.atEnd()) return false;

        strong = strong.trimmed();
        pos = strong.indexOf("#",0);
        if(pos>=0) strong = strong.left(pos);
        pos = strong.indexOf("!",0);
        if(pos>=0) strong = strong.left(pos);

        if(strong.isEmpty()) bComment = true;

        Line++;
    }

    if(in.atEnd())
    {
        return false;
    }
    return true;
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void readCOLORREF(QDataStream &ar, int &r, int &g, int &b)
{
    qint32 colorref;

    ar >> colorref;
    b = (int)(colorref/256/256);
    colorref -= b*256*256;
    g = (int)(colorref/256);
    r = colorref - g*256;
}


void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &zeta)
{
    omega1 = fabs(lambda.imag());

    if(omega1 > PRECISION)
    {
        omegaN = sqrt(lambda.real()*lambda.real()+omega1*omega1);
        zeta = -lambda.real()/omegaN;
    }
    else
    {
        omegaN = 0.0;
        zeta = 0.0;
    }

/*    double sum, prod, sigma1;
    sum  = lambda.real() * 2.0;                         // is a real number
    prod = lambda.real()*lambda.real() + lambda.imag()*lambda.imag();  // is a positive real number
    omegaN = fabs(lambda.imag());
    if(omegaN>PRECISION)    omega1 = sqrt(prod);
    else                    omega1 = 0.0;
    sigma1 = sum /2.0;
    if(omega1>PRECISION) dsi = -sigma1/omega1;
    else                 dsi = 0.0;
    qDebug("old   %13.7f  %13.7f  %13.7f", omegaN/2/PI, omega1/2/PI, dsi);*/
}




/**
* Returns the intersection of a ray with the object's panels
* The ray is defined by a mouse click and is perpendicular to the viewport
*    A is the ray's origin,
*    U is the ray's direction
*    LA, LB, TA, TB define a quadrangle in 3D space.
*    N is the normal to the quadrangle
*    I is the resulting intersection point of the ray and the quadrangle, if inside the quadrangle
*    dist = |AI|
*    The return value is true if the intersection inside the quadrangle, false otherwise
**/
bool Intersect(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &Normal,
               Vector3d const &A,  Vector3d const &U,  Vector3d &I, double &dist)
{
    Vector3d P, W, V, T;
    bool b1, b2, b3, b4;
    double r,s;

    r = (LA.x-A.x)*Normal.x + (LA.y-A.y)*Normal.y + (LA.z-A.z)*Normal.z ;
    s = U.x*Normal.x + U.y*Normal.y + U.z*Normal.z;

    dist = 10000.0;

    if(qAbs(s)>0.0)
    {
        dist = r/s;

        //inline operations to save time
        P.x = A.x + U.x * dist;
        P.y = A.y + U.y * dist;
        P.z = A.z + U.z * dist;

        // P is inside the panel if on left side of each panel side
        W.x = P.x  - TA.x;
        W.y = P.y  - TA.y;
        W.z = P.z  - TA.z;
        V.x = TB.x - TA.x;
        V.y = TB.y - TA.y;
        V.z = TB.z - TA.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        b1 = (T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0);

        W.x = P.x  - TB.x;
        W.y = P.y  - TB.y;
        W.z = P.z  - TB.z;
        V.x = LB.x - TB.x;
        V.y = LB.y - TB.y;
        V.z = LB.z - TB.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        b2 = (T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0);

        W.x = P.x  - LB.x;
        W.y = P.y  - LB.y;
        W.z = P.z  - LB.z;
        V.x = LA.x - LB.x;
        V.y = LA.y - LB.y;
        V.z = LA.z - LB.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        b3 = (T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0);

        W.x = P.x  - LA.x;
        W.y = P.y  - LA.y;
        W.z = P.z  - LA.z;
        V.x = TA.x - LA.x;
        V.y = TA.y - LA.y;
        V.z = TA.z - LA.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        b4 = (T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0);

        if(b1 && b2 && b3 && b4)
        {
            I.set(P.x, P.y, P.z);
            return true;
        }
    }
    return false;
}

