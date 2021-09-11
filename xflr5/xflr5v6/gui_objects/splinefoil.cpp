/****************************************************************************

    Spline Foil Class
    Copyright (C) 2003-2016 André Deperrois

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


#include "splinefoil.h"
#include <xflobjects/objects2d/foil.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects_global.h>

/**
 * The public costructor.
 */
SplineFoil::SplineFoil()
{
    m_theStyle = {true, Line::SOLID, 1, QColor(119, 183, 83), Line::NOSYMBOL};

    m_OutPoints    = 0;

    m_bOutPoints   = false;
    m_bModified    = false;
    m_bCenterLine  = false;
    m_bForceCloseLE = true;
    m_bForceCloseTE = true;
    m_Intrados.setTheStyle(m_theStyle);
    m_Extrados.setTheStyle(m_theStyle);
    m_bSymetric = false;
}


/**
 * Overloaded constructor.
 */
SplineFoil::SplineFoil(SplineFoil *pSF)
{
    copy(pSF);
}


/**
 * Initializes the SplineFoil object with stock data.
 */
void SplineFoil::initSplineFoil()
{
    m_bModified   = false;
    m_Name = QObject::tr("Spline Foil");

    m_Extrados.setTheStyle(m_theStyle);
    m_Extrados.clearPoints();
    m_Extrados.insertPoint(0.0 , 0.0);
    m_Extrados.insertPoint(0.0 , 0.00774066);
    m_Extrados.insertPoint(0.0306026, 0.0343829);
    m_Extrados.insertPoint(0.289036 , 0.0504014);
    m_Extrados.insertPoint(0.576000,  0.0350933);
    m_Extrados.insertPoint(0.736139 , 0.0269428);
    m_Extrados.insertPoint(1. , 0.);

    m_Intrados.setTheStyle(m_theStyle);
    m_Intrados.clearPoints();
    m_Intrados.insertPoint(0. , 0.);
    m_Intrados.insertPoint(0. , -0.00774066);
    m_Intrados.insertPoint(0.0306026, -0.0343829);
    m_Intrados.insertPoint(0.289036 , -0.0504014);
    m_Intrados.insertPoint(0.576000,  -0.0350933);
    m_Intrados.insertPoint(0.736139 , -0.0269428);
    m_Intrados.insertPoint(1. , 0.);

    compMidLine();

    m_OutPoints = m_Extrados.m_iRes + m_Intrados.m_iRes;

}

/**
 * Calculates the SplineFoil's mid-camber line and stores the resutls in the memeber array.
 * @return
 */
void SplineFoil::compMidLine()
{
    double x, yex, yin;
    m_fThickness = 0.0;
    m_fCamber    = 0.0;
    m_fxCambMax  = 0.0;
    m_fxThickMax = 0.0;

    m_rpMid[0].x   = 0.0;
    m_rpMid[0].y   = 0.0;

    double step = 1.0/double(MIDPOINTCOUNT);

    for (int k=0; k<MIDPOINTCOUNT; k++)
    {
        x = k*step;
        yex = m_Extrados.getY(x);
        yin = m_Intrados.getY(x);
        m_rpMid[k].x = x;
        m_rpMid[k].y = (yex+yin)/2.0;
        if(qAbs(yex-yin)>m_fThickness)
        {
            m_fThickness = qAbs(yex-yin);
            m_fxThickMax = x;
        }
        if(qAbs(m_rpMid[k].y)>qAbs(m_fCamber))
        {
            m_fCamber = m_rpMid[k].y;
            m_fxCambMax = x;
        }
    }
    m_rpMid[MIDPOINTCOUNT-1].x = 1.0;
    m_rpMid[MIDPOINTCOUNT-1].y = 0.0;
}


/**
 * Initializes this SplineFoil object with the data from another.
 * @param pSF a pointer to the source SplineFoil object.
 */
void SplineFoil::copy(SplineFoil* pSF)
{
    m_theStyle  = pSF->theStyle();
    m_Extrados.copy(&pSF->m_Extrados);
    m_Intrados.copy(&pSF->m_Intrados);
    m_OutPoints  = pSF->m_OutPoints;
    m_fCamber    = pSF->m_fCamber;
    m_fThickness = pSF->m_fThickness;
    m_fxCambMax  = pSF->m_fxCambMax;
    m_fxThickMax = pSF->m_fxThickMax;
    m_bSymetric  = pSF->m_bSymetric;
}


/**
 * Exports the current SplineFoil to a Foil object.
 * @param pFoil a pointer to the existing Foil object to be loaded with the SplineFoil points.
 */
void SplineFoil::exportToBuffer(Foil *pFoil)
{
    if(!pFoil) return;

    int i;
    int j = m_Extrados.m_iRes;
    int k = m_Intrados.m_iRes;
    for (i=0; i<m_Extrados.m_iRes; i++)
    {
        pFoil->m_x[j-i-1] = m_Extrados.m_Output[i].x;
        pFoil->m_y[j-i-1] = m_Extrados.m_Output[i].y;
    }
    for (i=1; i<m_Intrados.m_iRes;i++)
    {
        pFoil->m_x[i+j-1] = m_Intrados.m_Output[i].x;
        pFoil->m_y[i+j-1] = m_Intrados.m_Output[i].y;
    }
    pFoil->m_n = j+k-1;
    memcpy(pFoil->m_xb, pFoil->m_x, sizeof(pFoil->m_x));
    memcpy(pFoil->m_yb, pFoil->m_y, sizeof(pFoil->m_y));
    pFoil->m_nb = pFoil->m_n;
    pFoil->setName(m_Name);
}

/**
 * Exports the SplineFoil's output points to a text file.
 * @param out the QTextStream to which the output is directed.
 */
void SplineFoil::exportToFile(QTextStream &out)
{
    m_Extrados.exportSpline(out, true);
    m_Intrados.exportSpline(out, false);
}


/**
 * Loads or saves the data of this SplineFoil to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool SplineFoil::serialize(QDataStream &ar, bool bIsStoring)
{
    float f(0),x(0),y(0);
    int ArchiveFormat(0),k(0),n(0);
    int r(0),g(0),b(0);

    if(bIsStoring)
    {
        //no storage in.wpa format anymore
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat < 100000 || ArchiveFormat > 110000) return false;
        xfl::readCString(ar, m_Name);
        m_Name = QObject::tr("Spline Foil");
        xfl::readCOLORREF(ar, r,g,b);
        ar >> n;
        m_theStyle.setStipple(n);
        ar >> m_theStyle.m_Width;

        m_Extrados.setTheStyle(m_theStyle);
        m_Intrados.setTheStyle(m_theStyle);

        ar >> n;// m_Extrados.m_iCtrlPoints;
        m_Extrados.m_CtrlPt.clear();
        ar >> m_Extrados.m_iDegree;

        for (k=0; k<n; k++)
        {
            ar >> x;
            ar >> y;
            m_Extrados.m_CtrlPt.append({double(x), double(y), 0.0});
        }
        if(ArchiveFormat<100306)
        {
            ar >> f;
            ar >> f;
        }

        ar >> n;// m_Intrados.m_iCtrlPoints;
        m_Intrados.m_CtrlPt.clear();
        ar >> m_Intrados.m_iDegree;

        for (k=0; k<n; k++)
        {
            ar >> x;
            ar >> y;
            m_Intrados.m_CtrlPt.append({double(x), double(y), 0.0});
        }
        if(ArchiveFormat<100306)
        {
            ar >> f;
            ar >> f;
        }
        ar >> m_Extrados.m_iRes >> m_Intrados.m_iRes;

        ar >> k;
        if(k!=0 && k!=1) return false;
        if(k) m_theStyle.m_bIsVisible = true; else m_theStyle.m_bIsVisible = false;

        ar >> k;
        if(k!=0 && k!=1) return false;

        if(k) m_bOutPoints = true; else m_bOutPoints = false;

        ar >> k;
        if(k!=0 && k!=1) return false;

        if(k) m_bCenterLine = true; else m_bCenterLine = false;


        m_Extrados.splineKnots();
        m_Extrados.splineCurve();
        m_Intrados.splineKnots();
        m_Intrados.splineCurve();

        updateSplineFoil();

    }
    m_bModified = false;
    return true;
}


/**
 * Loads or saves the data of this SplineFoil to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool SplineFoil::serializeXFL(QDataStream &ar, bool bIsStoring)
{
    int k(0), n(0);
    double dble(0), x(0), y(0);
    int ArchiveFormat=200002; // 200002: nes LineStyle format

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_Name;

        m_theStyle.serializeXfl(ar, bIsStoring);

        ar<<m_bCenterLine << m_bOutPoints;

        ar << m_Extrados.m_iDegree << m_Intrados.m_iDegree;
        ar << m_Extrados.m_iRes << m_Intrados.m_iRes;

        ar << m_Extrados.m_CtrlPt.size();
        for (k=0; k<m_Extrados.m_CtrlPt.size();k++)
        {
            ar << m_Extrados.m_CtrlPt[k].x << m_Extrados.m_CtrlPt[k].y;
        }

        ar << m_Intrados.m_CtrlPt.size();
        for (k=0; k<m_Intrados.m_CtrlPt.size();k++)
        {
            ar << m_Intrados.m_CtrlPt[k].x << m_Intrados.m_CtrlPt[k].y;
        }

        if(m_bForceCloseLE) k=1; else k=0;
        ar << k;
        if(m_bForceCloseTE) k=1; else k=0;
        ar << k;
        // space allocation for the future storage of more data, without need to change the format
        n=0;
        for (int i=0; i<8; i++) ar << n;
        dble=0;
        for (int i=0; i<10; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat < 200000 || ArchiveFormat > 210000) return false;

        ar >> m_Name;
        if(ArchiveFormat<200002)
        {
            ar >> m_theStyle.m_Color;
            ar >> n; m_theStyle.setStipple(n);
            ar >> m_theStyle.m_Width;
            ar >> m_theStyle.m_bIsVisible;
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        m_Intrados.setTheStyle(m_theStyle);
        m_Extrados.setTheStyle(m_theStyle);

        ar >> m_bCenterLine >> m_bOutPoints;

        ar >> m_Extrados.m_iDegree >> m_Intrados.m_iDegree;
        ar >> m_Extrados.m_iRes >> m_Intrados.m_iRes;

        m_Extrados.m_CtrlPt.clear();
        ar >> n;
        for (k=0; k<n;k++)
        {
            ar >> x >> y;
            m_Extrados.m_CtrlPt.append({x, y, 0.0});
        }

        m_Intrados.m_CtrlPt.clear();
        ar >> n;
        for (k=0; k<n;k++)
        {
            ar >> x >> y;
            m_Intrados.m_CtrlPt.append({x, y, 0.0});
        }

        ar >> k;
        if(k>0) m_bForceCloseLE = true; else m_bForceCloseLE=false;
        ar >> k;
        if(k>0) m_bForceCloseTE = true; else m_bForceCloseTE=false;
        // space allocation
        for (int i=0; i<8; i++) ar >> k;
        for (int i=0; i<10; i++) ar >> dble;

        m_Extrados.splineKnots();
        m_Extrados.splineCurve();
        m_Intrados.splineKnots();
        m_Intrados.splineCurve();

        updateSplineFoil();

    }
    m_bModified = false;
    return true;
}


/**
 * Updates the mid camber line and the number of points after a modification.
 */
void SplineFoil::updateSplineFoil()
{
    compMidLine();
    m_OutPoints = m_Extrados.m_iRes + m_Intrados.m_iRes;
}


/**
 * Draws the SplineFoil's control points.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale of the view in the x direction
 * @param scaley the scale of the view in the y direction
 * @param Offset the postion of the SplineFoil's leading edge point
 */
void SplineFoil::drawCtrlPoints(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    m_Extrados.drawCtrlPoints(painter, scalex, scaley, Offset);
    m_Intrados.drawCtrlPoints(painter, scalex, scaley, Offset);
}

/**
 * Draws the SplineFoil's output points.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale of the view in the x direction
 * @param scaley the scale of the view in the y direction
 * @param Offset the postion of the SplineFoil's leading edge point
 */
void SplineFoil::drawOutPoints(QPainter & painter, double scalex, double scaley, QPointF Offset)
{
    m_Extrados.drawOutputPoints(painter, scalex, scaley, Offset);
    m_Intrados.drawOutputPoints(painter, scalex, scaley, Offset);
}

/**
 * Draws the SplineFoil's curves.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale of the view in the x direction
 * @param scaley the scale of the view in the y direction
 * @param Offset the postion of the SplineFoil's leading edge point
 */
void SplineFoil::drawFoil(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    m_Extrados.drawSpline(painter, scalex, scaley, Offset);
    m_Intrados.drawSpline(painter, scalex, scaley, Offset);
}


/**
 * Draws the SplineFoil's mid camber line.
 * @param painter a reference to the QPainter object with which to draw
 * @param scalex the scale of the view in the x direction
 * @param scaley the scale of the view in the y direction
 * @param Offset the postion of the SplineFoil's leading edge point
 */
void SplineFoil::drawMidLine(QPainter &painter, double scalex, double scaley, QPointF Offset)
{
    painter.save();

    QPointF From, To;

    QPen MidPen(m_theStyle.m_Color);
    MidPen.setStyle(Qt::DashLine);
    MidPen.setWidth(1);
    painter.setPen(MidPen);

    From = QPointF(m_rpMid[0].x*scalex + Offset.x(), -m_rpMid[0].y*scaley + Offset.y());

    for (int k=1; k<MIDPOINTCOUNT; k+=10)
    {
        To.rx() =  m_rpMid[k].x*scalex + Offset.x();
        To.ry() = -m_rpMid[k].y*scaley + Offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    painter.drawLine(From, QPointF(m_rpMid[MIDPOINTCOUNT-1].x*scalex + Offset.x(), -m_rpMid[MIDPOINTCOUNT-1].y*scaley + Offset.y()));
    painter.restore();
}
