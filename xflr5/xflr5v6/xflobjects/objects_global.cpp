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

#include <QDir>

#include <xflobjects/objects_global.h>
#include <xflcore/constants.h>
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects3d/objects3d.h>




QColor xfl::getObjectColor(int type)
{
    //type
    // 0=Foil
    // 1=Polar
    // 2=Opp
    // 3=Wing (unused)
    // 4=WPolar
    // 5=WOpp
    // 6=POpp
    int i=0,j=0;
    bool bFound = false;
    switch (type)
    {
        case 0:
        {
            Foil *pFoil;
            for (j=0; j<s_ColorList.size(); j++)
            {
                for (i=0; i<Objects2d::foilCount(); i++)
                {
                    pFoil = Objects2d::foilAt(i);
                    bFound = false;
                    if(pFoil->color() == s_ColorList.at(j))
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound) return s_ColorList.at(j);
            }
            break;
        }
        case 1:
        {
            Polar *pPolar;
            for (j=0; j<s_ColorList.size(); j++)
            {
                for (i=0; i<Objects2d::polarCount(); i++)
                {
                    pPolar = Objects2d::polarAt(i);
                    bFound = false;
                    if(pPolar->color() == s_ColorList.at(j))
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound)
                    return s_ColorList.at(j);
            }
            break;
        }
        case 2:
        {
            OpPoint *pOpPoint;
            for (j=0; j<s_ColorList.size(); j++){
                for (i=0; i<Objects2d::oppCount(); i++)
                {
                    pOpPoint = Objects2d::oppAt(i);
                    bFound = false;
                    QColor clr = pOpPoint->color();
                    if(clr == s_ColorList.at(j))
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound) return s_ColorList.at(j);
            }
            break;
        }

        default:
        {
        }
    }
    return xfl::randomColor(false);
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
bool xfl::intersect(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &Normal,
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



Foil *xfl::readFoilFile(QFile &xFoilFile)
{
    QString strong;
    QString tempStr;
    QString FoilName;

    Foil* pFoil = nullptr;
    int pos=0;
    double x=0, y=0, z=0, area=0;
    double xp=0, yp=0;

    QTextStream in(&xFoilFile);

    QString fileName = xFoilFile.fileName();
    int idx = fileName.lastIndexOf(QDir::separator());
    if(idx>0)
    {
        fileName = fileName.right(fileName.length()-idx-1);
        if(fileName.endsWith(".dat", Qt::CaseInsensitive))
        {
            fileName = fileName.left(fileName.length()-4);
        }
    }

    pFoil = new Foil();
    if(!pFoil)    return nullptr;

    while(tempStr.length()==0 && !in.atEnd())
    {
        strong = in.readLine();
        pos = strong.indexOf("#",0);
        // ignore everything after # (including #)
        if(pos>0)strong.truncate(pos);
        tempStr = strong;
        tempStr.remove(" ");
        FoilName = strong;
    }

    if(!in.atEnd())
    {
        // FoilName contains the last comment

        if(xfl::readValues(strong,x,y,z)==2)
        {
            //there isn't a name on the first line, use the file's name
            FoilName = fileName;
            {
                pFoil->m_xb[0] = x;
                pFoil->m_yb[0] = y;
                pFoil->m_nb=1;
                xp = x;
                yp = y;
            }
        }
        else FoilName = strong;
        // remove fore and aft spaces
        FoilName = FoilName.trimmed();
    }

    bool bRead = true;
    xp=-9999.0;
    yp=-9999.0;
    do
    {
        strong = in.readLine();
        pos = strong.indexOf("#",0);
        // ignore everything after # (including #)
        if(pos>0)strong.truncate(pos);
        tempStr = strong;
        tempStr.remove(" ");
        if (!strong.isNull() && bRead && tempStr.length())
        {
            if(xfl::readValues(strong, x,y,z)==2)
            {
                //add values only if the point is not coincident with the previous one
                double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
                if(dist>0.000001)
                {
                    pFoil->m_xb[pFoil->m_nb] = x;
                    pFoil->m_yb[pFoil->m_nb] = y;
                    pFoil->m_nb++;
                    if(pFoil->m_nb>IQX)
                    {
                        delete pFoil;
                        return nullptr;
                    }
                    xp = x;
                    yp = y;
                }
            }
            else bRead = false;
        }
    }while (bRead && !strong.isNull());

    pFoil->setName(FoilName);

    // Check if the foil was written clockwise or counter-clockwise

    area = 0.0;
    for (int i=0; i<pFoil->m_nb; i++)
    {
        int ip = 0;
        if(i==pFoil->m_nb-1)  ip = 0;
        else                ip = i+1;
        area +=  0.5*(pFoil->m_yb[i]+pFoil->m_yb[ip])*(pFoil->m_xb[i]-pFoil->m_xb[ip]);
    }

    if(area < 0.0)
    {
        //reverse the points order
        double xtmp, ytmp;
        for (int i=0; i<pFoil->m_nb/2; i++)
        {
            xtmp         = pFoil->m_xb[i];
            ytmp         = pFoil->m_yb[i];
            pFoil->m_xb[i] = pFoil->m_xb[pFoil->m_nb-i-1];
            pFoil->m_yb[i] = pFoil->m_yb[pFoil->m_nb-i-1];
            pFoil->m_xb[pFoil->m_nb-i-1] = xtmp;
            pFoil->m_yb[pFoil->m_nb-i-1] = ytmp;
        }
    }

    memcpy(pFoil->m_x, pFoil->m_xb, sizeof(pFoil->m_xb));
    memcpy(pFoil->m_y, pFoil->m_yb, sizeof(pFoil->m_yb));
    pFoil->m_n = pFoil->m_nb;

    QColor clr = xfl::randomColor(false);
    pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
    pFoil->initFoil();

    return pFoil;
}


/**
 *Reads a Foil and its related Polar objects from a binary stream associated to a .plr file.
 * @param ar the binary stream
 * @return the pointer to the Foil object which has been created, or NULL if failure.
 */
Foil* xfl::readPolarFile(QFile &plrFile, QVector<Polar*> &polarList)
{
    Foil* pFoil = nullptr;
    Polar *pPolar = nullptr;
    Polar * pOldPolar = nullptr;
    int i=0, n=0, l=0;

    QDataStream ar(&plrFile);
    ar.setVersion(QDataStream::Qt_4_5);
    ar.setByteOrder(QDataStream::LittleEndian);

    ar >> n;

    if(n<100000)
    {
        //old format
        return nullptr;
    }
    else if (n >=100000)
    {
        //new format XFLR5 v1.99+
        //first read all available foils
        ar>>n;
        for (i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!serializeFoil(pFoil, ar, false))
            {
                delete pFoil;
                return nullptr;
            }
        }

        //next read all available polars

        ar>>n;
        for (i=0;i<n; i++)
        {
            pPolar = new Polar();

            if (!serializePolar(pPolar, ar, false))
            {
                delete pPolar;
                return nullptr;
            }
            for (l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);
                if (pOldPolar->foilName()  == pPolar->foilName() &&
                        pOldPolar->polarName() == pPolar->polarName())
                {
                    //just overwrite...
                    polarList.removeAt(l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.append(pPolar);
        }
    }
    return pFoil;
}


/**
 * Draws the foil in the client area.
 * @param painter a reference to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void xfl::drawFoil(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley, QPointF const &Offset)
{
    double xa(0), ya(0), sina(0), cosa(0);
    QPointF From, To;

    QPen FoilPen, HighPen;

    FoilPen.setColor(pFoil->color());
    FoilPen.setWidth(pFoil->lineWidth());
    FoilPen.setStyle(xfl::getStyle(pFoil->lineStipple()));
    painter.setPen(FoilPen);

    HighPen.setColor(QColor(255,0,0));

    cosa = cos(alpha*PI/180.0);
    sina = sin(alpha*PI/180.0);

    xa = (pFoil->m_x[0]-0.5)*cosa - pFoil->m_y[0]*sina + 0.5;
    ya = (pFoil->m_x[0]-0.5)*sina + pFoil->m_y[0]*cosa;
    From.rx() = ( xa*scalex + Offset.x());
    From.ry() = (-ya*scaley + Offset.y());

    for (int k=1; k<pFoil->m_n; k++)
    {
        xa = (pFoil->m_x[k]-0.5)*cosa - pFoil->m_y[k]*sina+ 0.5;
        ya = (pFoil->m_x[k]-0.5)*sina + pFoil->m_y[k]*cosa;
        To.rx() =  xa*scalex+Offset.x();
        To.ry() = -ya*scaley+Offset.y();

        painter.drawLine(From,To);

        From = To;
    }
}


/**
 * Draws the foil's mid line in the client area.
 * @param painter a refernce to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void xfl::drawMidLine(QPainter &painter, const Foil *pFoil, double scalex, double scaley, QPointF const &Offset)
{
    QPointF From, To;

    QPen FoilPen;

    FoilPen.setColor(pFoil->color());
    FoilPen.setWidth(pFoil->lineWidth());
    FoilPen.setStyle(Qt::DashLine);
    painter.setPen(FoilPen);


    From.rx() = ( pFoil->m_rpMid[0].x*scalex)  +Offset.x();
    From.ry() = (-pFoil->m_rpMid[0].y*scaley)  +Offset.y();


    for (int k=0; k<MIDPOINTCOUNT; k++)
    {
        To.rx() = ( pFoil->m_rpMid[k].x*scalex)+Offset.x();
        To.ry() = (-pFoil->m_rpMid[k].y*scaley)+Offset.y();

        painter.drawLine(From, To);
        From = To;
    }
}
void xfl::drawFoilPoints(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley,
                QPointF const &Offset, const QColor &backColor)
{
    QPen FoilPen, HighPen;
    FoilPen.setColor(pFoil->color());
    FoilPen.setWidth(pFoil->lineWidth());
    FoilPen.setStyle(Qt::SolidLine);
    painter.setPen(FoilPen);

    HighPen.setColor(QColor(255,0,0));


    double xa(0), ya(0);
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);

    for (int i=0; i<pFoil->m_n;i++)
    {
        xa = (pFoil->m_x[i]-0.5)*cosa - pFoil->m_y[i]*sina + 0.5;
        ya = (pFoil->m_x[i]-0.5)*sina + pFoil->m_y[i]*cosa;

        QPoint pt(int(xa*scalex + Offset.x()), int(-ya*scaley + Offset.y()));

        xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, pFoil->color(), pt);
    }

    if(pFoil->iHighLight()>=0)
    {
        HighPen.setWidth(2);
        painter.setPen(HighPen);

        int ih = pFoil->iHighLight();
        xa = (pFoil->m_x[ih]-0.5)*cosa - pFoil->m_y[ih]*sina + 0.5;
        ya = (pFoil->m_x[ih]-0.5)*sina + pFoil->m_y[ih]*cosa;

        QPoint pt(int(xa*scalex + Offset.x()), int(-ya*scaley + Offset.y()));

        xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, pFoil->color(), pt);
    }
}

void xfl::setAutoWPolarName(WPolar *pWPolar, Plane *pPlane)
{
    if(!pPlane) return;
    QString str, strong;
    QString strSpeedUnit;
    Units::getSpeedUnitLabel(strSpeedUnit);

    int i, nCtrl;


    Units::getSpeedUnitLabel(str);

    QString name;
    switch(pWPolar->polarType())
    {
        case xfl::FIXEDSPEEDPOLAR:
        {
            name = QString("T1-%1 ").arg(pWPolar->velocity() * Units::mstoUnit(),0,'f',1);
            name += strSpeedUnit;
            break;
        }
        case xfl::FIXEDLIFTPOLAR:
        {
            name = QString("T2");
            break;
        }
        case xfl::FIXEDAOAPOLAR:
        {
            name = QString(QString::fromUtf8("T4-%1°")).arg(pWPolar->Alpha(),0,'f',1);
            break;
        }
        case xfl::BETAPOLAR:
        {
            name = QString(QString::fromUtf8("T5-a%1°-%2"))
                    .arg(pWPolar->Alpha(),0,'f',1)
                    .arg(pWPolar->velocity() * Units::mstoUnit(),0,'f',1);
            name += strSpeedUnit;
            break;
        }
        case xfl::STABILITYPOLAR:
        {
            name = QString("T7");
            break;
        }
        default:
        {
            name = "Tx";
            break;
        }
    }

    switch(pWPolar->analysisMethod())
    {
        case xfl::LLTMETHOD:
        {
            name += "-LLT";
            break;
        }
        case xfl::VLMMETHOD:
        {
            if(pWPolar->bVLM1()) name += "-VLM1";
            else                 name += "-VLM2";
            break;
        }
        case xfl::PANEL4METHOD:
        {
            if(!pWPolar->bThinSurfaces()) name += "-Panel";
            else
            {
                if(pWPolar->bVLM1()) name += "-VLM1";
                else                 name += "-VLM2";
            }
            break;
        }
        default:
            break;
    }

    nCtrl = 0;



    if(pWPolar->isStabilityPolar())
    {
        if(!pPlane->isWing())
        {
            if(pWPolar->m_ControlGain.size()>0 && qAbs(pWPolar->m_ControlGain[0])>PRECISION)
            {
                strong = QString::fromUtf8("-Wing(g%1)")
                        .arg(pWPolar->m_ControlGain[0],0,'f',1);
                name += strong;
            }
            nCtrl++;
        }

        if(pPlane->stab())
        {
            if(pWPolar->m_ControlGain.size()>1 && qAbs(pWPolar->m_ControlGain[1])>PRECISION)
            {
                strong = QString::fromUtf8("-Elev(g%1)").arg(pWPolar->m_ControlGain[1],0,'f',1);
                name += strong;
            }
            nCtrl++;
        }

        for(i=0; i<pPlane->wing()->nFlaps(); i++)
        {
            if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
            {
                strong = QString::fromUtf8("-WF%1(g%2)")
                        .arg(i+1)
                        .arg(pWPolar->m_ControlGain[i+nCtrl],0,'f',1);
                name += strong;
            }
        }
        nCtrl += pPlane->wing()->nFlaps();

        if(pPlane->stab())
        {
            for(i=0; i<pPlane->stab()->nFlaps(); i++)
            {
                if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
                {
                    strong = QString::fromUtf8("-EF%1(g%2)").arg(i+1).arg(pWPolar->m_ControlGain[i+nCtrl]);
                    name += strong;
                }
            }
            nCtrl += pPlane->stab()->nFlaps();
        }

        if(pPlane->fin())
        {
            for(i=0; i<pPlane->fin()->nFlaps(); i++)
            {
                if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
                {
                    strong = QString::fromUtf8("-FF%1(g%2)").arg(i+1).arg(pWPolar->m_ControlGain[i+nCtrl]);
                    name += strong;
                }
            }
        }
    }


    if(qAbs(pWPolar->Beta()) > .001  && pWPolar->polarType()!=xfl::BETAPOLAR)
    {
        strong = QString(QString::fromUtf8("-b%1°")).arg(pWPolar->Beta(),0,'f',1);
        name += strong;
    }

    if(qAbs(pWPolar->Phi()) > .001)
    {
        strong = QString(QString::fromUtf8("-B%1°")).arg(pWPolar->Phi(),0,'f',1);
        name += strong;
    }

    if(!pWPolar->bAutoInertia())
    {
        strong = QString::asprintf("-%.1f", pWPolar->mass()*Units::kgtoUnit());
        if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[0])>PRECISION)
            str = QString::asprintf("/%0.2f", pWPolar->m_inertiaGain[0]*Units::kgtoUnit());
        else str.clear();
        name += strong + str + Units::massUnitLabel();

        strong = QString::asprintf("-x%.1f", pWPolar->CoG().x*Units::mtoUnit());
        if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[1])>PRECISION)
            str = QString::asprintf("/%0.2f", pWPolar->m_inertiaGain[1]*Units::mtoUnit());
        else str.clear();
        name += strong + str + Units::lengthUnitLabel();

        if(qAbs(pWPolar->CoG().z)>=.000001)
        {
            strong = QString::asprintf("-z%.1f", pWPolar->CoG().z*Units::mtoUnit());
            if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[2])>PRECISION)
                str = QString::asprintf("/%0.2f", pWPolar->m_inertiaGain[2]*Units::mtoUnit());
            else str.clear();
            name += strong + str + Units::lengthUnitLabel();
        }
    }

    if(!pWPolar->bViscous())
    {
        name += "-Inviscid";
    }
    /*    if(pWPolar->bIgnoreBodyPanels())
    {
        name += "-NoBodyPanels";
    }*/
    //    if(pWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM) name += "-proj_area";

    if(pWPolar->bTilted()) name += "-TG";

    for(int i=0; i<MAXEXTRADRAG; i++)
    {
        if(fabs(pWPolar->m_ExtraDragCoef[i])>PRECISION && fabs(pWPolar->m_ExtraDragArea[i])>PRECISION)
        {
            name+="+Drag";
            break;
        }
    }
    pWPolar->setPolarName(name);
}


void xfl::setRandomFoilColor(Foil *pFoil, bool bLightTheme)
{
    QColor clr = xfl::randomColor(!bLightTheme);
    pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
}


/**
 * Loads or Saves the data of this foil to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool xfl::serializeFoil(Foil *pFoil, QDataStream &ar, bool bIsStoring)
{
    // saves or loads the foil to the archive ar

    int ArchiveFormat = 1006;
    // 1006 : QFLR5 v0.02 : added Foil description
    // 1005 : added LE Flap data
    // 1004 : added Points and Centerline property
    // 1003 : added Visible property
    // 1002 : added color and style save
    // 1001 : initial format
    int p(0), j(0), n(0);
    float f(0), ff(0);
    QString strange;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        xfl::writeString(ar, pFoil->name());
        xfl::writeString(ar, pFoil->m_FoilDescription);
        ar << pFoil->theStyle().m_Stipple << pFoil->theStyle().m_Width;
        xfl::writeColor(ar, pFoil->theStyle().m_Color.red(), pFoil->theStyle().m_Color.green(), pFoil->theStyle().m_Color.blue());

        if (pFoil->theStyle().m_bIsVisible) ar << 1; else ar << 0;
        if (pFoil->theStyle().m_Symbol>0)   ar << 1; else ar << 0;//1004
        if (pFoil->m_bCenterLine)    ar << 1; else ar << 0;//1004
        if (pFoil->m_bLEFlap)        ar << 1; else ar << 0;
        ar << float(pFoil->m_LEFlapAngle) << float(pFoil->m_LEXHinge) << float(pFoil->m_LEYHinge);
        if (pFoil->m_bTEFlap)        ar << 1; else ar << 0;
        ar << float(pFoil->m_TEFlapAngle) << float(pFoil->m_TEXHinge) << float(pFoil->m_TEYHinge);
        ar << 1.f << 1.f << 9.f;//formerly transition parameters
        ar << pFoil->m_nb;
        for (j=0; j<pFoil->m_nb; j++)
        {
            ar << float(pFoil->m_xb[j]) << float(pFoil->m_yb[j]);
        }
        ar << pFoil->m_n;
        for (j=0; j<pFoil->m_n; j++)
        {
            ar << float(pFoil->m_x[j]) << float(pFoil->m_y[j]);
        }
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000||ArchiveFormat>1010)
            return false;

        xfl::readString(ar, strange); pFoil->setName(strange);

        if(ArchiveFormat>=1006)
        {
            xfl::readString(ar, pFoil->m_FoilDescription);
        }
        if(ArchiveFormat>=1002)
        {
            ar >> n;
            pFoil->theStyle().setStipple(n);
            ar >> pFoil->theStyle().m_Width;
            int r=0,g=0,b=0;
            xfl::readColor(ar, r, g, b);
            pFoil->setColor(r,g,b);
        }
        if(ArchiveFormat>=1003)
        {
            ar >> p;
            if(p) pFoil->theStyle().m_bIsVisible = true; else pFoil->theStyle().m_bIsVisible = false;
        }
        if(ArchiveFormat>=1004)
        {
            ar >> p;
            pFoil->theStyle().setPointStyle(p);
            ar >> p;
            if(p) pFoil->m_bCenterLine = true; else pFoil->m_bCenterLine = false;
        }

        if(ArchiveFormat>=1005)
        {
            ar >> p;
            if (p) pFoil->m_bLEFlap = true; else pFoil->m_bLEFlap = false;
            ar >> f; pFoil->m_LEFlapAngle = double(f);
            ar >> f; pFoil->m_LEXHinge = double(f);
            ar >> f; pFoil->m_LEYHinge = double(f);
        }
        ar >> p;
        if (p) pFoil->m_bTEFlap = true; else pFoil->m_bTEFlap = false;
        ar >> f; pFoil->m_TEFlapAngle = double(f);
        ar >> f; pFoil->m_TEXHinge = double(f);
        ar >> f; pFoil->m_TEYHinge = double(f);

        ar >> f >> f >> f; //formerly transition parameters
        ar >> pFoil->m_nb;
        if(pFoil->m_nb>IBX) return false;

        for (j=0; j<pFoil->m_nb; j++)
        {
            ar >> f >> ff;
            pFoil->m_xb[j]  = double(f);  pFoil->m_yb[j]=double(ff);
        }

        /** @todo remove. We don't need to save/load the current foil geom
         *  since we re-create later it using base geometry and flap data */
        if(ArchiveFormat>=1001)
        {
            ar >> pFoil->m_n;
            if(pFoil->m_n>IBX) return false;

            for (j=0; j<pFoil->m_n; j++)
            {
                ar >> f >> ff;
                //                pFoil->x[j]=f; pFoil->y[j]=ff;
            }
            /*            if(pFoil->nb==0 && pFoil->n!=0)
            {
                pFoil->nb = pFoil->n;
                memcpy(pFoil->xb, pFoil->x, sizeof(pFoil->xb));
                memcpy(pFoil->yb, pFoil->y, sizeof(pFoil->yb));
            }*/
        }
        else
        {
            /*            memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
            memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
            pFoil->n=pFoil->nb;*/
        }


        pFoil->initFoil();
        pFoil->setFlap();

        return true;
    }
}



/**
 * Loads or saves the data of this polar to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool xfl::serializePolar(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    int i(0), j(0), n(0), l(0), k(0);
    // identifies the format of the file
    // 1005: new linestyle format

    int ArchiveFormat=1005;
    float f(0);

    if(bIsStoring)
    {
        //write variables
        n = pPolar->m_Alpha.size();

        ar << ArchiveFormat; // identifies the format of the file
        // 1004 : added XCp
        // 1003 : re-instated NCrit, XtopTr and XBotTr with polar
        xfl::writeString(ar, pPolar->m_FoilName);
        xfl::writeString(ar, pPolar->name());

        if     (pPolar->m_PolarType==xfl::FIXEDSPEEDPOLAR)  ar<<1;
        else if(pPolar->m_PolarType==xfl::FIXEDLIFTPOLAR)   ar<<2;
        else if(pPolar->m_PolarType==xfl::RUBBERCHORDPOLAR) ar<<3;
        else if(pPolar->m_PolarType==xfl::FIXEDAOAPOLAR)    ar<<4;
        else                                                ar<<1;

        ar << pPolar->m_MaType << pPolar->m_ReType;
        ar << int(pPolar->m_Reynolds) << float(pPolar->m_Mach);
        ar << float(pPolar->m_ASpec);
        ar << n << float(pPolar->m_NCrit);
        ar << float(pPolar->m_XTop) << float(pPolar->m_XBot);
        /*
        writeColor(ar, pPolar->m_red, pPolar->m_green, pPolar->m_blue);
        ar << pPolar->m_Style << pPolar->m_Width;
        if (pPolar->m_bIsVisible)  ar<<1; else ar<<0;
        ar<<pPolar->m_Symbol;*/
        pPolar->theStyle().serializeXfl(ar, bIsStoring);

        for (i=0; i< pPolar->m_Alpha.size(); i++){
            ar << float(pPolar->m_Alpha[i]) << float(pPolar->m_Cd[i]) ;
            ar << float(pPolar->m_Cdp[i])   << float(pPolar->m_Cl[i]) << float(pPolar->m_Cm[i]);
            ar << float(pPolar->m_XTr1[i])  << float(pPolar->m_XTr2[i]);
            ar << float(pPolar->m_HMom[i])  << float(pPolar->m_Cpmn[i]);
            ar << float(pPolar->m_Re[i]);
            ar << float(pPolar->m_XCp[i]);
        }

        ar << pPolar->m_NCrit << pPolar->m_XTop << pPolar->m_XBot;

        return true;
    }
    else
    {
        //read variables
        QString strange;
        float Alpha(0), Cd(0), Cdp(0), Cl(0), Cm(0), XTr1(0), XTr2(0), HMom(0), Cpmn(0), Re(0), XCp(0);
        int iRe(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat <1001 || ArchiveFormat>1100)
        {
            return false;
        }

        xfl::readString(ar, pPolar->m_FoilName);
        xfl::readString(ar, strange); pPolar->setName(strange);

        if(pPolar->m_FoilName.isEmpty() || pPolar->name().isEmpty())
        {
            return false;
        }

        ar >>k;
        if     (k==1) pPolar->m_PolarType = xfl::FIXEDSPEEDPOLAR;
        else if(k==2) pPolar->m_PolarType = xfl::FIXEDLIFTPOLAR;
        else if(k==3) pPolar->m_PolarType = xfl::RUBBERCHORDPOLAR;
        else if(k==4) pPolar->m_PolarType = xfl::FIXEDAOAPOLAR;
        else          pPolar->m_PolarType = xfl::FIXEDSPEEDPOLAR;


        ar >> pPolar->m_MaType >> pPolar->m_ReType;

        if(pPolar->m_MaType!=1 && pPolar->m_MaType!=2 && pPolar->m_MaType!=3)
        {
            return false;
        }
        if(pPolar->m_ReType!=1 && pPolar->m_ReType!=2 && pPolar->m_ReType!=3)
        {
            return false;
        }

        ar >> iRe;
        pPolar->m_Reynolds = double(iRe);
        ar >> f; pPolar->m_Mach = double(f);

        ar >> f; pPolar->m_ASpec = double(f);

        ar >> n;
        ar >> f; pPolar->m_NCrit = double(f);
        ar >> f; pPolar->m_XTop = double(f);
        ar >> f; pPolar->m_XBot = double(f);

        if(ArchiveFormat<1005)
        {
            int r,g,b;
            xfl::readColor(ar, r,g,b);
            pPolar->setColor({r,g,b});
            ar >>n;
            pPolar->theStyle().setStipple(n);
            ar >> pPolar->theStyle().m_Width;
            if(ArchiveFormat>=1002)
            {
                ar >> l;
                if(l!=0 && l!=1 )
                {
                    return false;
                }
                if (l) pPolar->theStyle().m_bIsVisible =true; else pPolar->theStyle().m_bIsVisible = false;
            }
            ar >> l;  pPolar->theStyle().setPointStyle(l);
        }
        else pPolar->theStyle().serializeXfl(ar, bIsStoring);

        bool bExists=false;
        for (i=0; i< n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm;
            ar >> XTr1 >> XTr2;
            ar >> HMom >> Cpmn;

            if(ArchiveFormat >=4) ar >> Re;
            else                  Re = float(pPolar->m_Reynolds);

            if(ArchiveFormat>=1004) ar>> XCp;
            else                    XCp = 0.0;

            bExists = false;
            if(pPolar->m_PolarType!=xfl::FIXEDAOAPOLAR)
            {
                for (j=0; j<pPolar->m_Alpha.size(); j++)
                {
                    if(fabs(double(Alpha)-pPolar->m_Alpha[j])<0.001)
                    {
                        bExists = true;
                        break;
                    }
                }
            }
            else
            {
                for (j=0; j<pPolar->m_Re.size(); j++)
                {
                    if(fabs(double(Re)-pPolar->m_Re[j])<0.1)
                    {
                        bExists = true;
                        break;
                    }
                }
            }
            if(!bExists)
            {
                pPolar->addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(XTr1), double(XTr2), double(HMom), double(Cpmn), double(Re), double(XCp));
            }
        }
        if(ArchiveFormat>=1003)
            ar >>pPolar->m_NCrit >> pPolar->m_XTop >> pPolar->m_XBot;
    }
    return true;
}
