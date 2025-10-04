/****************************************************************************

    Reference Foil Class
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


#include <QTextStream>
#include <QRandomGenerator>

#include "foil.h"
#include <xflgeom/geom2d/spline.h>
#include <xflobjects/objects2d/polar.h>
#include <xflcore/constants.h>



/**
 * The public constructor.
 */
Foil::Foil()
{
    m_theStyle.m_Symbol = Line::NOSYMBOL; //no points to start with
    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 2;
    m_theStyle.m_bIsVisible    = true;

    m_theStyle.m_Color.setHsv(QRandomGenerator::global()->bounded(360),
               QRandomGenerator::global()->bounded(55)+30,
               QRandomGenerator::global()->bounded(55)+150);

    m_iHighLight = -1;

    m_bCenterLine = false;

    m_fCamber     = 0.0;
    m_fXCamber    = 0.0;
    m_fThickness  = 0.0;
    m_fXThickness = 0.0;

    m_n = 0;
    memset(m_x, 0, sizeof(m_x));
    memset(m_y, 0, sizeof(m_y));
    memset(m_nx, 0, sizeof(m_nx));
    memset(m_ny, 0, sizeof(m_ny));

    m_nb = 0;
    memset(m_xb, 0, sizeof(m_xb));
    memset(m_yb, 0, sizeof(m_yb));


    m_iInt = 0;
    m_iExt = 0;
    m_iBaseExt = 0;
    m_iBaseInt = 0;

    m_TEGap  = 0.0;

    memset(m_rpExtrados, 0, sizeof(m_rpExtrados));
    memset(m_rpIntrados, 0, sizeof(m_rpIntrados));
    memset(m_rpMid, 0, sizeof(m_rpMid));

    memset(m_BaseExtrados, 0, sizeof(m_BaseExtrados));
    memset(m_BaseIntrados, 0, sizeof(m_BaseIntrados));

    m_bTEFlap     = false;
    m_TEFlapAngle = 0.0;
    m_TEXHinge    = 70.0;
    m_TEYHinge    = 50.0;

    m_bLEFlap     = false;
    m_LEFlapAngle = 0.0;
    m_LEXHinge    = 20.0;
    m_LEYHinge    = 50.0;
}


/**
* Constructs the position of the foil's mid camber line.
* Calculates the foil's thickness and camber, if requested.
*@param bParams true if the max thickness and camber properties shold be calculated
*/
void Foil::compMidLine(bool bParams)
{
    double xt=0, yex=0, yin=0, step=0, nx=0, ny=0;

    if(bParams)
    {
        m_fThickness  = 0.0;
        m_fCamber     = 0.0;
        m_fXCamber    = 0.0;
        m_fXThickness = 0.0;
    }

    step = (m_rpExtrados[m_iExt].x-m_rpExtrados[0].x)/double(MIDPOINTCOUNT-1);

    for (int l=0; l<MIDPOINTCOUNT; l++)
    {
        xt = m_rpExtrados[0].x + l*step;
        getUpperY(double(l)*step, yex, nx, ny);
        getLowerY(double(l)*step, yin, nx, ny);

        m_rpMid[l].x = xt;
        m_rpMid[l].y = (yex+yin)/2.0;

        if(bParams)
        {
            if(fabs(yex-yin)>m_fThickness)
            {
                m_fThickness  = fabs(yex-yin);
                m_fXThickness = xt;
            }
            if(fabs(m_rpMid[l].y)>fabs(m_fCamber))
            {
                m_fCamber  = m_rpMid[l].y;
                m_fXCamber = xt;
            }
        }
    }
}


/**
* Copies the data from an existing foil and maps it to this foil's variables.
* @param pSrcFoil a pointer to the reference foil from which the data wil be copied
*/
void Foil::copyFoil(const Foil *pSrcFoil, bool bMetaData)
{
    if(bMetaData)
    {
        m_Name    = pSrcFoil->name();
        m_bCenterLine = pSrcFoil->m_bCenterLine;
        m_theStyle    = pSrcFoil->theStyle();
    }

    memcpy(m_x, pSrcFoil->m_x,  sizeof(pSrcFoil->m_x));
    memcpy(m_y, pSrcFoil->m_y,  sizeof(pSrcFoil->m_y));
    memcpy(m_xb,pSrcFoil->m_xb, sizeof(pSrcFoil->m_xb));
    memcpy(m_yb,pSrcFoil->m_yb, sizeof(pSrcFoil->m_yb));
    memcpy(m_nx,pSrcFoil->m_nx, sizeof(pSrcFoil->m_nx));
    memcpy(m_ny,pSrcFoil->m_ny, sizeof(pSrcFoil->m_ny));
    memcpy(m_rpMid,        pSrcFoil->m_rpMid,        sizeof(m_rpMid));
    memcpy(m_rpBaseMid,    pSrcFoil->m_rpBaseMid,    sizeof(m_rpBaseMid));
    memcpy(m_rpExtrados,   pSrcFoil->m_rpExtrados,   sizeof(m_rpExtrados));
    memcpy(m_rpIntrados,   pSrcFoil->m_rpIntrados,   sizeof(m_rpIntrados));
    memcpy(m_BaseExtrados, pSrcFoil->m_BaseExtrados, sizeof(m_BaseExtrados));
    memcpy(m_BaseIntrados, pSrcFoil->m_BaseIntrados, sizeof(m_BaseIntrados));
    m_iExt = pSrcFoil->m_iExt;
    m_iInt = pSrcFoil->m_iInt;
    m_iBaseExt = pSrcFoil->m_iBaseExt;
    m_iBaseInt = pSrcFoil->m_iBaseInt;
    m_TEGap  = pSrcFoil->m_TEGap;
    m_TE.x = pSrcFoil->m_TE.x;
    m_TE.y = pSrcFoil->m_TE.y;
    m_LE.x = pSrcFoil->m_LE.x;
    m_LE.y = pSrcFoil->m_LE.y;

    m_fThickness  = pSrcFoil->thickness();
    m_fXThickness = pSrcFoil->xThickness();
    m_fCamber     = pSrcFoil->camber();
    m_fXCamber    = pSrcFoil->xCamber();

    m_n  = pSrcFoil->m_n;
    m_nb = pSrcFoil->m_nb;

    m_bLEFlap     = pSrcFoil->m_bLEFlap;
    m_LEFlapAngle = pSrcFoil->m_LEFlapAngle;
    m_LEXHinge    = pSrcFoil->m_LEXHinge;
    m_LEYHinge    = pSrcFoil->m_LEYHinge;

    m_bTEFlap     = pSrcFoil->m_bTEFlap;
    m_TEFlapAngle = pSrcFoil->m_TEFlapAngle;
    m_TEXHinge    = pSrcFoil->m_TEXHinge;
    m_TEYHinge    = pSrcFoil->m_TEYHinge;
}


/**
* Derotates the fol's geometry i.e. aligns the mid-line with the x-axis.
* @return the angle, in degrees, by which the foil has been de-rotated
*/
double Foil::deRotate()
{
    // first translate the leading edge to the origin point
    for (int i=0; i<m_nb; i++)
    {
        m_xb[i] -= m_LE.x;
        m_yb[i] -= m_LE.y;
    }

    for (int i=0; i<m_n; i++)
    {
        m_x[i] -= m_LE.x;
        m_y[i] -= m_LE.y;
    }

    m_LE.set(0.0,0.0);
    m_TE.x  -= m_LE.x;
    m_TE.y  -= m_LE.y;

    // then find current angle

    double angle = atan2(m_TE.y-m_LE.y, m_TE.x-m_LE.x);// xle=tle=0;

    //rotate about the L.E.
    double cosa = cos(-angle);
    double sina = sin(-angle);


    double xr=0, yr=0;
    for (int i=0; i<m_nb; i++)
    {
        xr = m_xb[i]*cosa - m_yb[i]*sina;
        yr = m_xb[i]*sina + m_yb[i]*cosa;
        m_xb[i] = xr;
        m_yb[i] = yr;
    }

    for (int i=0; i<m_n; i++)
    {
        xr = m_x[i]*cosa - m_y[i]*sina;
        yr = m_x[i]*sina + m_y[i]*cosa;
        m_x[i] = xr;
        m_y[i] = yr;
    }

    xr = m_TE.x*cosa - m_TE.y*sina;
    yr = m_TE.x*sina + m_TE.y*cosa;
    m_TE.x = xr;
    m_TE.y = yr;

    initFoil();

    return angle*180.0/PI;
}



/**
 * Exports the foil geometry to a text .dat file.
 * @param out the QtextStream to which the output will be directed
 * @return true if the operation has been successful, false otherwise
 */
bool Foil::exportFoil(QTextStream &out) const
{
    QString strOut;

    out << m_Name +"\n";

    for (int i=0; i< m_n; i++)
    {
        // jojo-Patch increase precision from 5 to 7 (2020/01)
        strOut = QString("%1    %2\n").arg(m_x[i],10,'f',7).arg(m_y[i],10,'f',7);
        // strOut = QString("%1    %2\n").arg(x[i],8,'f',5).arg(y[i],8,'f',5);
        out << strOut;
    }

    return true;
}


/**
 * Returns the area defined by the foil's contour, in normalized units.
 * @return the foil's internal area
 */
double Foil::area() const
{
    double area = 0.0;
    for (int i=0; i<m_nb-1; i++)
    {
        area +=  qAbs((m_yb[i+1]+m_yb[i])/2.0 * (m_xb[i+1]-m_xb[i]));
    }
    return area;
}



/**
 * Returns the y-position on the lower surface, at a specified chord position.
 * @param &x the chordwise position
 * @return the y-position
 */
double Foil::baseLowerY(double x) const
{
    x = m_BaseIntrados[0].x + x*(m_BaseIntrados[m_iBaseInt].x-m_BaseIntrados[0].x);//in case there is a flap which reduces the length

    for (int i=0; i<m_iBaseInt; i++)
    {
        if (m_BaseIntrados[i].x <m_BaseIntrados[i+1].x  &&  m_BaseIntrados[i].x <= x && x<=m_BaseIntrados[i+1].x )
        {
            double y = (m_BaseIntrados[i].y   +(m_BaseIntrados[i+1].y-m_BaseIntrados[i].y)
                      /(m_BaseIntrados[i+1].x - m_BaseIntrados[i].x) * (x-m_BaseIntrados[i].x));
            return y;
        }
    }
    return 0.0;
}



/**
 * Returns the y-position on the upper surface, at a specified chord position
 * @param &x the chordwise position
 * @return the y-position
 */
double Foil::baseUpperY(double x) const
{
    x = m_BaseExtrados[0].x + x*(m_BaseExtrados[m_iBaseExt].x-m_BaseExtrados[0].x);//in case there is a flap which reduces the length

    for (int i=0; i<m_iBaseExt; i++)
    {
        if (m_BaseExtrados[i].x <m_BaseExtrados[i+1].x  &&  m_BaseExtrados[i].x <= x && x<=m_BaseExtrados[i+1].x )
        {
            double y = (m_BaseExtrados[i].y   +(m_BaseExtrados[i+1].y-m_BaseExtrados[i].y)
                      /(m_BaseExtrados[i+1].x - m_BaseExtrados[i].x) * (x-m_BaseExtrados[i].x));
            return y;
        }
    }
    return 0.0;
}


/**
 * Returns the slope in radians on the lower surface, at a specified chord position.
 * @param &x the chordwise position
 * @return the slope in radians
 */
double Foil::bottomSlope(double x) const
{
    for (int i=0; i<m_iInt; i++)
    {
        if ((m_rpIntrados[i].x <= x) && (x < m_rpIntrados[i+1].x))
        {
            double dx = m_rpIntrados[i+1].x-m_rpIntrados[i].x;
            double dy = m_rpIntrados[i+1].y-m_rpIntrados[i].y;
            return -atan2(dy,dx);
        }
    }
    return 0.0;
}


/**
 * Returns the slope in radians on the upperer surface, at a specified chord position.
 * @param &x the chordwise position
 * @return the slope in radians
 */
double Foil::topSlope(double x) const
{
    for (int i=0; i<m_iExt; i++)
    {
        if ((m_rpExtrados[i].x <= x) && (x < m_rpExtrados[i+1].x))
        {
            double dx = m_rpExtrados[i+1].x-m_rpExtrados[i].x;
            double dy = m_rpExtrados[i+1].y-m_rpExtrados[i].y;
            return -atan2(dy,dx);
        }
    }
    return 0.0;
}


/**
 * Returns the camber value at a specified chord position, in normalized units.
 * @param &x the chordwise position
 * @return the camber value
 */
double Foil::camber(double x) const
{
    for (int i=0; i<MIDPOINTCOUNT-1; i++)
    {
        if ((m_rpMid[i].x <= x) && (x < m_rpMid[i+1].x))
        {
            return (m_rpMid[i+1].y+m_rpMid[i].y)/2.0;
        }
    }
    return 0.0;
}


/**
 * Returns the camber angle in degrees at a specified chord position.
 * @param &x the chordwise position
 * @return the camber angle, in degrees
 */
double Foil::camberSlope(double x) const
{
    //returns the camber slope at position x
    for (int i=0; i<MIDPOINTCOUNT-1; i++)
    {
        if ((m_rpMid[i].x <= x) && (x < m_rpMid[i+1].x))
        {
            double dx = m_rpMid[i+1].x-m_rpMid[i].x;
            double dy = m_rpMid[i+1].y-m_rpMid[i].y;
            return atan2(dy,dx);
        }
    }
    if(x>=1.0)
    {
            double dx = m_rpMid[MIDPOINTCOUNT-1].x-m_rpMid[MIDPOINTCOUNT-2].x;
            double dy = m_rpMid[MIDPOINTCOUNT-1].y-m_rpMid[MIDPOINTCOUNT-2].y;
            return atan2(dy,dx);
    }
    return 0.0;
}


/**
 * Returns the foil's length.
 * @return the foil's length, in relative units
*/
double Foil::length() const
{
    return qMax(m_rpExtrados[m_iExt].x, m_rpExtrados[m_iInt].x);
}


/**
* Returns the y-coordinate on the current foil's mid line at the x position.
* @param x the chordwise position
* @return the position on the mid line
*/
Vector2d Foil::midYRel(double sRel) const
{
    if(sRel>=1.0)      return m_rpMid[MIDPOINTCOUNT-1];
    else if(sRel<=0.0) return m_rpMid[0];

    Vector2d midY;
    sRel *= (MIDPOINTCOUNT-1);
    int iRel = int(sRel);
    double frac = sRel-iRel;
    midY.x = m_rpMid[iRel].x * (1.0-frac) + m_rpMid[iRel+1].x * frac;
    midY.y = m_rpMid[iRel].y * (1.0-frac) + m_rpMid[iRel+1].y * frac;    
    return midY;
}


/**
* Returns the y-coordinate on the current foil's upper surface at the x position.
* @param x the chordwise position
* @return the position on the upper surface
*/
Vector2d Foil::upperYRel(double xRel, double &normx, double &normy) const
{
    double x = m_rpExtrados[0].x + xRel*(m_rpExtrados[m_iExt].x-m_rpExtrados[0].x);

    if(x<=m_rpExtrados[0].x)
    {
        normx = -1.0;
        normy = 0.0;
        return m_rpExtrados[0];
    }

    for (int i=0; i<m_iExt; i++)
    {
        if (m_rpExtrados[i].x < m_rpExtrados[i+1].x && m_rpExtrados[i].x <= x && x<=m_rpExtrados[i+1].x )
        {
            double nabs = sqrt((m_rpExtrados[i+1].x-m_rpExtrados[i].x) * (m_rpExtrados[i+1].x-m_rpExtrados[i].x)
                             + (m_rpExtrados[i+1].y-m_rpExtrados[i].y) * (m_rpExtrados[i+1].y-m_rpExtrados[i].y));
            normx = (-m_rpExtrados[i+1].y + m_rpExtrados[i].y)/nabs;
            normy = ( m_rpExtrados[i+1].x - m_rpExtrados[i].x)/nabs;
            return (m_rpExtrados[i] + (m_rpExtrados[i+1]-m_rpExtrados[i])
                                     /(m_rpExtrados[i+1].x-m_rpExtrados[i].x) * (x-m_rpExtrados[i].x));
        }
    }
    normx = 1.0;
    normy = 0.0;
    return m_rpExtrados[m_iExt];
}



/**
* Returns the y-coordinate on the current foil's lower surface at the x position.
* @param x the chordwise position
* @return the position on the upper surface
*/
Vector2d Foil::lowerYRel(double xRel, double &normx, double &normy) const
{
    double x = m_rpIntrados[0].x + xRel*(m_rpIntrados[m_iInt].x-m_rpIntrados[0].x);

    if(x<=m_rpIntrados[0].x)
    {
        normx = -1.0;
        normy = 0.0;
        return m_rpIntrados[0];
    }

    for (int i=0; i<m_iInt; i++)
    {
        if (m_rpIntrados[i].x<m_rpIntrados[i+1].x && m_rpIntrados[i].x<= x && x<=m_rpIntrados[i+1].x )
        {
            double nabs = sqrt((m_rpIntrados[i+1].x-m_rpIntrados[i].x) * (m_rpIntrados[i+1].x-m_rpIntrados[i].x)
                             + (m_rpIntrados[i+1].y-m_rpIntrados[i].y) * (m_rpIntrados[i+1].y-m_rpIntrados[i].y));
            normx = ( m_rpIntrados[i+1].y - m_rpIntrados[i].y)/nabs;
            normy = (-m_rpIntrados[i+1].x + m_rpIntrados[i].x)/nabs;

            return (m_rpIntrados[i] + (m_rpIntrados[i+1]-m_rpIntrados[i])
                                     /(m_rpIntrados[i+1].x-m_rpIntrados[i].x) * (x-m_rpIntrados[i].x));
        }
    }

    normx = 1.0;
    normy = 0.0;
    return m_rpIntrados[m_iExt];
}

/**
* Returns the y-coordinate and the normal to the surface on the foil's lower surface at the x position.
* @param x the chordwise position
* @param &y a reference to variable holding the position on the lower surface
* @param &normx a reference to variable holding the x-component of the normal to the surface
* @param &normy a reference to variable holding the y-component of the normal to the surface
*/
void Foil::getLowerY(double x, double &y, double &normx, double &normy) const
{
    double nabs;
    x = m_rpIntrados[0].x + x*(m_rpIntrados[m_iInt].x-m_rpIntrados[0].x);

    if(x<=m_rpIntrados[0].x)
    {
        normx = -1.0;
        normy = 0.0;
        y = m_rpIntrados[0].y;
        return;
    }

    for (int i=0; i<m_iInt; i++)
    {
        if (m_rpIntrados[i].x <m_rpIntrados[i+1].x  &&  m_rpIntrados[i].x <= x && x<=m_rpIntrados[i+1].x )
        {
            y = (m_rpIntrados[i].y     + (m_rpIntrados[i+1].y-m_rpIntrados[i].y)
                                     /(m_rpIntrados[i+1].x-m_rpIntrados[i].x) * (x-m_rpIntrados[i].x));

            nabs = sqrt(  (m_rpIntrados[i+1].x-m_rpIntrados[i].x) * (m_rpIntrados[i+1].x-m_rpIntrados[i].x)
                      + (m_rpIntrados[i+1].y-m_rpIntrados[i].y) * (m_rpIntrados[i+1].y-m_rpIntrados[i].y));
            normx = ( m_rpIntrados[i+1].y - m_rpIntrados[i].y)/nabs;
            normy = (-m_rpIntrados[i+1].x + m_rpIntrados[i].x)/nabs;
            return;
        }
    }

    y = m_rpIntrados[m_iInt].y;
    nabs = sqrt((m_rpIntrados[m_iInt].x-m_rpIntrados[m_iInt-1].x) * (m_rpIntrados[m_iInt].x-m_rpIntrados[m_iInt-1].x)
              + (m_rpIntrados[m_iInt].y-m_rpIntrados[m_iInt-1].y) * (m_rpIntrados[m_iInt].y-m_rpIntrados[m_iInt-1].y));
    normx = (-m_rpIntrados[m_iInt].y + m_rpIntrados[m_iInt-1].y)/nabs;
    normy = ( m_rpIntrados[m_iInt].x - m_rpIntrados[m_iInt-1].x)/nabs;
}



/**
* Returns the y-coordinate and the normal to the surface on the foil's upper surface at the x position.
*@param x the chordwise position
*@param &y a reference to variable holding the position on the surface
*@param &normx a reference to variable holding the x-component of the normal to the surface
*@param &normy a reference to variable holding the y-component of the normal to the surface
*/
void Foil::getUpperY(double x, double &y, double &normx, double &normy) const
{
    double nabs;
    x = m_rpExtrados[0].x + x*(m_rpExtrados[m_iExt].x-m_rpExtrados[0].x);

    if(x<=m_rpIntrados[0].x)
    {
        normx = -1.0;
        normy =  0.0;
        y = m_rpExtrados[0].y;
        return;
    }

    for (int i=0; i<m_iExt; i++)
    {
        if (m_rpExtrados[i].x <m_rpExtrados[i+1].x  &&  m_rpExtrados[i].x <= x && x<=m_rpExtrados[i+1].x )
        {
            y = (m_rpExtrados[i].y     + (m_rpExtrados[i+1].y-m_rpExtrados[i].y)
                                    / (m_rpExtrados[i+1].x-m_rpExtrados[i].x) * (x-m_rpExtrados[i].x));

            nabs = sqrt((m_rpExtrados[i+1].x-m_rpExtrados[i].x) * (m_rpExtrados[i+1].x-m_rpExtrados[i].x)
                      + (m_rpExtrados[i+1].y-m_rpExtrados[i].y) * (m_rpExtrados[i+1].y-m_rpExtrados[i].y));
            normx = (-m_rpExtrados[i+1].y + m_rpExtrados[i].y)/nabs;
            normy = ( m_rpExtrados[i+1].x - m_rpExtrados[i].x)/nabs;
            return;
        }
    }

    y = m_rpExtrados[m_iExt].y;
    nabs = sqrt((m_rpExtrados[m_iExt].x-m_rpExtrados[m_iExt-1].x) * (m_rpExtrados[m_iExt].x-m_rpExtrados[m_iExt-1].x)
              + (m_rpExtrados[m_iExt].y-m_rpExtrados[m_iExt-1].y) * (m_rpExtrados[m_iExt].y-m_rpExtrados[m_iExt-1].y));
    normx = (-m_rpExtrados[m_iExt].y + m_rpExtrados[m_iExt-1].y)/nabs;
    normy = ( m_rpExtrados[m_iExt].x - m_rpExtrados[m_iExt-1].x)/nabs;

}


/**
* Initializes the foil geometry, constructs the upper and lower points, and applies the flap deflection if requested.
*/
bool Foil::initFoil()
{
    // at this point, coordinates have been loaded
    // so has been the number of points defining the foil
    bool bNotFound = true;
    int k = 0;

    //first time is to calculate the base foil's thickness and camber

    if(m_nb<=0)
    {
        return false;
    }

    while (k<m_nb)
    {
        if (m_xb[k+1] < m_xb[k])
        {
            k++;
        }
        else 
        {
            if(bNotFound)
            {
                m_iBaseExt = k;
                m_BaseExtrados[k].x = m_xb[k];
                m_BaseExtrados[k].y = m_yb[k];
                bNotFound = false;
            }
            m_BaseIntrados[k-m_iBaseExt].x = m_xb[k]; /** @todo m_iBaseInt ? */
            m_BaseIntrados[k-m_iBaseExt].y = m_yb[k];
            k++;
        }
    }
    m_iBaseInt = m_nb-m_iBaseExt-1;
    m_BaseIntrados[m_nb-m_iBaseExt-1].x = m_xb[m_nb-1];
    m_BaseIntrados[m_nb-m_iBaseExt-1].y = m_yb[m_nb-1];
    for (k=0; k<=m_iBaseExt;k++)
    {
        m_BaseExtrados[k].x = m_xb[m_iBaseExt-k];
        m_BaseExtrados[k].y = m_yb[m_iBaseExt-k];
    }

    memcpy(m_rpExtrados, m_BaseExtrados, sizeof(m_rpExtrados));
    memcpy(m_rpIntrados, m_BaseIntrados, sizeof(m_rpIntrados));
    m_iExt = m_iBaseExt;
    m_iInt = m_iBaseInt;

    compMidLine(true);
    memcpy(m_rpBaseMid, m_rpMid, sizeof(m_rpBaseMid));


    m_TEGap = m_BaseExtrados[m_iBaseExt].y-m_BaseIntrados[m_iBaseInt].y;

    m_LE.x = (m_BaseIntrados[0].x+m_BaseExtrados[0].x)/2.0;
    m_LE.y = (m_BaseIntrados[0].y+m_BaseExtrados[0].y)/2.0;
    
    m_TE.x = (m_BaseIntrados[m_iBaseInt].x+m_BaseExtrados[m_iBaseExt].x)/2.0;
    m_TE.y = (m_BaseIntrados[m_iBaseInt].y+m_BaseExtrados[m_iBaseExt].y)/2.0;


    //the second time is to get the mid line of the current foil
    //i.e. with flaps eventually
    //used for the VLM analysis
    k=0;
    bNotFound = true;
    while (k<m_n)
    {
        if (m_x[k+1] < m_x[k])
        {
            k++;
        }
        else {
            if(bNotFound)
            {
                m_iExt = k;
                m_rpExtrados[k].x = m_x[k];
                m_rpExtrados[k].y = m_y[k];
                bNotFound = false;
            }
            m_rpIntrados[k-m_iExt].x = m_x[k];
            m_rpIntrados[k-m_iExt].y = m_y[k];
            k++;
        }
    }
    m_iInt = m_n-m_iExt-1;
    m_rpIntrados[m_n-m_iExt-1].x = m_x[m_n-1];
    m_rpIntrados[m_n-m_iExt-1].y = m_y[m_n-1];
    for (k=0; k<=m_iExt;k++)
    {
        m_rpExtrados[k].x = m_x[m_iExt-k];
        m_rpExtrados[k].y = m_y[m_iExt-k];
    }


    compMidLine(false);
    return true;
}


/**
*ABCD are assumed to lie in the xy plane
*@return true and intersection point M if AB and CD intersect inside, false and intersection point M if AB and CD intersect outside
*/
bool Foil::intersect(Vector2d const &A, Vector2d const &B, Vector2d const &C, Vector2d const &D, Vector2d *M) const
{
    double Det, Det1, Det2, t, u;
    Vector2d AB, CD;

    M->set(0,0);
    AB.set(B.x-A.x, B.y-A.y);
    CD.set(D.x-C.x, D.y-C.y);

    //Cramer's rule

    Det  = -AB.x * CD.y + CD.x * AB.y;
    if(Det==0.0)
    {
        //vectors are parallel, no intersection
        return false;
    }
    Det1 = -(C.x-A.x)*CD.y + (C.y-A.y)*CD.x;
    Det2 = -(C.x-A.x)*AB.y + (C.y-A.y)*AB.x;

    t = Det1/Det;
    u = Det2/Det;

    M->x = A.x + t*AB.x;
    M->y = A.y + t*AB.y;

    if (0.0<=t && t<=1.0 && 0.0<=u && u<=1.0) return true;//M is between A and B
    else                                      return false;//M is outside
}


/**
*Returns the index of foil's point which coincides with the input point, if any, otherwise returns -10.
*@param &Real the Vector2d which defines the input point
*/
int Foil::isPoint(Vector2d const &Real) const
{
    for (int k=0; k<m_n; k++)
    {
        if(qAbs(Real.x-m_x[k])<0.005 && qAbs(Real.y-m_y[k])<0.005) return k;
    }
    return -10;
}

/**
* Normalizes the base foil's lengh to unity.
* The current foil's length is modified by the same ratio.
* @return the foil's former length
*/
double Foil::normalizeGeometry()
{
    double xmin = 1.0;
    double xmax = 0.0;

    for (int i=0; i<m_nb; i++)
    {
        xmin = qMin(xmin, m_xb[i]);
        xmax = qMax(xmax, m_xb[i]);
    }
    double length = xmax - xmin;

    //reset origin
    for (int i=0; i<m_nb; i++) m_xb[i] -= xmin;

    //set length to 1. and cancel y offset
    for(int i=0; i<m_nb; i++)
    {
        m_xb[i] = m_xb[i]/length;
        m_yb[i] = m_yb[i]/length;
    }
    double yTrans = m_yb[0];
    for(int i=0; i<m_nb; i++)    m_yb[i] -= yTrans;

    //reset origin
    for (int i=0; i<m_n; i++)
    {
        m_x[i] -= xmin;
    }

    //set length to 1. and cancel y offset
    for(int i=0; i<m_n; i++)
    {
        m_x[i] = m_x[i]/length;
        m_y[i] = m_y[i]/length;
    }
    yTrans = m_y[0];
    for(int i=0; i<m_n; i++)    m_y[i] -= yTrans;

    return length;
}


/**
 *     Reset the foil to a default Naca 009 geometry.
 */
void Foil::setNaca009()
{    
    m_x[0]  = 1.00000    ; m_y[0]  = 0.00000;
    m_x[1]  = 0.99572    ; m_y[1]  = 0.00057;
    m_x[2]  = 0.98296    ; m_y[2]  = 0.00218;
    m_x[3]  = 0.96194    ; m_y[3]  = 0.00463;
    m_x[4]  = 0.93301    ; m_y[4]  = 0.00770;
    m_x[5]  = 0.89668    ; m_y[5]  = 0.01127;
    m_x[6]  = 0.85355    ; m_y[6]  = 0.01522;
    m_x[7]  = 0.80438    ; m_y[7]  = 0.01945;
    m_x[8]  = 0.75000    ; m_y[8]  = 0.02384;
    m_x[9]  = 0.69134    ; m_y[9]  = 0.02823;
    m_x[10] = 0.62941    ; m_y[10] = 0.03247;
    m_x[11] = 0.56526    ; m_y[11] = 0.03638;
    m_x[12] = 0.50000    ; m_y[12] = 0.03978;
    m_x[13] = 0.43474    ; m_y[13] = 0.04248;
    m_x[14] = 0.37059    ; m_y[14] = 0.04431;
    m_x[15] = 0.33928    ; m_y[15] = 0.04484;
    m_x[16] = 0.30866    ; m_y[16] = 0.04509;
    m_x[17] = 0.27886    ; m_y[17] = 0.04504;
    m_x[18] = 0.25000    ; m_y[18] = 0.04466;
    m_x[19] = 0.22221    ; m_y[19] = 0.04397;
    m_x[20] = 0.19562    ; m_y[20] = 0.04295;
    m_x[21] = 0.17033    ; m_y[21] = 0.04161;
    m_x[22] = 0.14645    ; m_y[22] = 0.03994;
    m_x[23] = 0.12408    ; m_y[23] = 0.03795;
    m_x[24] = 0.10332    ; m_y[24] = 0.03564;
    m_x[25] = 0.08427    ; m_y[25] = 0.03305;
    m_x[26] = 0.06699    ; m_y[26] = 0.03023;
    m_x[27] = 0.05156    ; m_y[27] = 0.02720;
    m_x[28] = 0.03806    ; m_y[28] = 0.02395;
    m_x[29] = 0.02653    ; m_y[29] = 0.02039;
    m_x[30] = 0.01704    ; m_y[30] = 0.01646;
    m_x[31] = 0.00961    ; m_y[31] = 0.01214;
    m_x[32] = 0.00428    ; m_y[32] = 0.00767;
    m_x[33] = 0.00107    ; m_y[33] = 0.00349;
    m_x[34] = 0.00000    ; m_y[34] = 0.00000;
    for (int i=0; i<34; i++){
        m_x[i+35] =  m_x[33-i];
        m_y[i+35] = -m_y[33-i];
    }
    m_n = 69;
    m_nb = 69;
    memcpy(m_xb,m_x, sizeof(m_x));
    initFoil();
}


/**
* Sets the properties for the trailing edge flap.
* @param bFlap true if a flap is applied to the trailing edge
* @param xhinge the relative x-position of the flap's hinge
* @param yhinge the relative y-position of the flap's hinge
* @param angle the flap angle in degrees
*/
void  Foil::setTEFlapData(bool bFlap, double xhinge, double yhinge, double angle)
{
    m_bTEFlap     = bFlap;
    m_TEXHinge    = xhinge;
    m_TEYHinge    = yhinge;
    m_TEFlapAngle = angle;
}


/**
* Sets the properties for the leading edge flap.
* @param bFlap true if a flap is applied to the leading edge
* @param xhinge the relative x-position of the flap's hinge
* @param yhinge the relative y-position of the flap's hinge
* @param angle the flap angle in degrees
*/
void  Foil::setLEFlapData(bool bFlap, double xhinge, double yhinge, double angle)
{
    m_bLEFlap     = bFlap;
    m_LEXHinge    = xhinge;
    m_LEYHinge    = yhinge;
    m_LEFlapAngle = angle;
}


/**
 * Modifies the geometry of the current foil by setting the leading edge flap.
 * The specification for the flap is assumed to have been set previously
 */
void Foil::setLEFlap()
{
    int j=0, k=0;

    double xh=0, yh=0, dx=0, dy=0;
    Vector2d M;
    bool bIntersect=false;

    double cosa = cos(m_LEFlapAngle*PI/180.0);
    double sina = sin(m_LEFlapAngle*PI/180.0);
    //first convert xhinge and yhinge in absolute coordinates
    xh = m_LEXHinge/100.0;
    double ymin = baseLowerY(xh);
    double ymax = baseUpperY(xh);
    yh = ymin + m_LEYHinge/100.0 * (ymax-ymin);

    // insert a breakpoint at xhinge location, if there isn't one already
    int iUpperh = 0;
    int iLowerh = 0;
    for (int i=0; i<m_iExt; i++)
    {
        if(qAbs(m_rpExtrados[i].x-xh)<0.001)
        {
            //then no need to add an extra point, just break
            iUpperh = i;
            break;
        }
        else if(m_rpExtrados[i].x>xh)
        {
            //insert one
            for(j=m_iExt+1; j>i; j--)
            {
                m_rpExtrados[j].x = m_rpExtrados[j-1].x;
                m_rpExtrados[j].y = m_rpExtrados[j-1].y;
            }

            m_rpExtrados[i].x = xh;
            m_rpExtrados[i].y = ymax;
            iUpperh = i;
            m_iExt+=1;
            break;
        }
    }

    for (int i=0; i<m_iInt; i++)
    {
        if(qAbs(m_rpIntrados[i].x-xh)<0.001)
        {
            //then no need to add an Intra point, just break
            iLowerh = i;
            break;
        }
        else if(m_rpIntrados[i].x>xh)
        {//insert one
            for(j=m_iInt+1; j>i; j--)
            {
                m_rpIntrados[j].x = m_rpIntrados[j-1].x;
                m_rpIntrados[j].y = m_rpIntrados[j-1].y;
            }

            m_rpIntrados[i].x = xh;
            m_rpIntrados[i].y = ymin;
            iLowerh = i;
            m_iInt+=1;
            break;
        }
    }

    // rotate all points upstream of xh
    if(m_LEFlapAngle>0.0)
    {
        //insert an extra point on intrados
        for (int i=m_iInt+1; i>iLowerh; i--)
        {
            m_rpIntrados[i] = m_rpIntrados[i-1];
        }
        m_rpIntrados[iLowerh] = m_rpIntrados[iLowerh+1];
        iLowerh++;
        m_iInt++;

        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_rpIntrados[iLowerh-1].x += 30.0*(m_rpIntrados[iLowerh-1].x-m_rpIntrados[iLowerh-2].x);
        m_rpIntrados[iLowerh-1].y += 30.0*(m_rpIntrados[iLowerh-1].y-m_rpIntrados[iLowerh-2].y);
        m_rpIntrados[iLowerh].x   += 30.0*(m_rpIntrados[iLowerh].x   - m_rpIntrados[iLowerh+1].x);
        m_rpIntrados[iLowerh].y   += 30.0*(m_rpIntrados[iLowerh].y   - m_rpIntrados[iLowerh+1].y);
    }
    if(m_LEFlapAngle<0.0)
    {
        //insert an extra point on extrados
        for (int i=m_iExt+1; i>iUpperh; i--)
        {
            m_rpExtrados[i] = m_rpExtrados[i-1];
        }
        m_rpExtrados[iUpperh] = m_rpExtrados[iUpperh+1];
        iUpperh++;
        m_iExt++;

        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_rpExtrados[iUpperh-1].x += 30.0 * (m_rpExtrados[iUpperh-1].x - m_rpExtrados[iUpperh-2].x);
        m_rpExtrados[iUpperh-1].y += 30.0 * (m_rpExtrados[iUpperh-1].y - m_rpExtrados[iUpperh-2].y);
        m_rpExtrados[iUpperh].x   += 30.0 * (m_rpExtrados[iUpperh].x   - m_rpExtrados[iUpperh+1].x);
        m_rpExtrados[iUpperh].y   += 30.0 * (m_rpExtrados[iUpperh].y   - m_rpExtrados[iUpperh+1].y);
    }
    for (int i=0; i<iUpperh; i++)
    {
        dx = m_rpExtrados[i].x-xh;
        dy = m_rpExtrados[i].y-yh;
        m_rpExtrados[i].x = xh + cosa * dx - sina * dy;
        m_rpExtrados[i].y = yh + sina * dx + cosa * dy;
    }
    for (int i=0; i<iLowerh; i++)
    {
        dx = m_rpIntrados[i].x-xh;
        dy = m_rpIntrados[i].y-yh;
        m_rpIntrados[i].x = xh + cosa * dx - sina * dy;
        m_rpIntrados[i].y = yh + sina * dx + cosa * dy;
    }

    Spline LinkSpline;
    LinkSpline.m_iRes = 4;
    LinkSpline.m_iDegree = 2;
    LinkSpline.m_CtrlPt.clear();

    if(m_LEFlapAngle<0.0)
    {

        //define a 3 ctrl-pt spline to smooth the connection between foil and flap on bottom side
        intersect(m_rpIntrados[iLowerh-2], m_rpIntrados[iLowerh-1],
                  m_rpIntrados[iLowerh],   m_rpIntrados[iLowerh+1], &M);
        //sanity check
        if(M.x <= m_rpIntrados[iLowerh-1].x || M.x >= m_rpIntrados[iLowerh].x)
            M = (m_rpIntrados[iLowerh-1] + m_rpIntrados[iLowerh])/2.0;

        LinkSpline.insertPoint(m_rpIntrados[iLowerh-1].x,m_rpIntrados[iLowerh-1].y);
        LinkSpline.insertPoint(M.x, M.y);
        LinkSpline.insertPoint(m_rpIntrados[iLowerh].x,m_rpIntrados[iLowerh].y);
        LinkSpline.splineKnots();
        LinkSpline.splineCurve();
        //retrieve point 1 and 2 and insert them
        for (int i=m_iInt; i>=iLowerh; i--)
        {
            m_rpIntrados[i+2].x = m_rpIntrados[i].x;
            m_rpIntrados[i+2].y = m_rpIntrados[i].y;
        }

        m_rpIntrados[iLowerh+1].x = LinkSpline.m_Output[2].x;
        m_rpIntrados[iLowerh+1].y = LinkSpline.m_Output[2].y;
        m_rpIntrados[iLowerh].x   = LinkSpline.m_Output[1].x;
        m_rpIntrados[iLowerh].y   = LinkSpline.m_Output[1].y;

        m_iInt+=2;
    }
    if(m_LEFlapAngle>0.0)
    {

        //define a 3 ctrl-pt spline to smooth the connection between foil and flap on bottom side
        intersect(m_rpExtrados[iUpperh-2], m_rpExtrados[iUpperh-1],
                  m_rpExtrados[iUpperh],   m_rpExtrados[iUpperh+1], &M);

        //sanity check
        if(M.x <= m_rpExtrados[iUpperh-1].x || M.x >= m_rpExtrados[iUpperh].x)
            M = (m_rpExtrados[iUpperh-1] + m_rpExtrados[iUpperh])/2.0;

        LinkSpline.insertPoint(m_rpExtrados[iUpperh-1].x,m_rpExtrados[iUpperh-1].y);
        LinkSpline.insertPoint(M.x, M.y);
        LinkSpline.insertPoint(m_rpExtrados[iUpperh].x,m_rpExtrados[iUpperh].y);
        LinkSpline.splineKnots();
        LinkSpline.splineCurve();
        //retrieve point 1 and 2 and insert them
        for (int i=m_iExt; i>=iUpperh; i--)
        {
            m_rpExtrados[i+2].x = m_rpExtrados[i].x;
            m_rpExtrados[i+2].y = m_rpExtrados[i].y;
        }

        m_rpExtrados[iUpperh+1].x = LinkSpline.m_Output[2].x;
        m_rpExtrados[iUpperh+1].y = LinkSpline.m_Output[2].y;
        m_rpExtrados[iUpperh].x   = LinkSpline.m_Output[1].x;
        m_rpExtrados[iUpperh].y   = LinkSpline.m_Output[1].y;

        m_iExt+=2;
    }

    // trim upper surface first
    int i1 = iUpperh;
    int i2 = iUpperh-1;

    bIntersect = false;
    for (j=i2-1; j>0; j--)
    {
        for (k=i1;k<m_iExt; k++)
        {
            if(intersect(m_rpExtrados[j], m_rpExtrados[j+1],
                         m_rpExtrados[k], m_rpExtrados[k+1], &M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_rpExtrados[j+1].x = M.x;
        m_rpExtrados[j+1].y = M.y;
        int p=1;
        for (int l=k+1;l<=m_iExt; l++){
            m_rpExtrados[j+1+p]  = m_rpExtrados[l];
            p++;
        }
        m_iExt = j+p;
    }

    // trim lower surface next
    i1 = iLowerh;
    i2 = iLowerh-1;

    bIntersect = false;
    for (j=i2-1; j>0; j--)
    {
        for (k=i1;k<m_iInt; k++)
        {
            if(intersect(m_rpIntrados[j], m_rpIntrados[j+1],
                         m_rpIntrados[k], m_rpIntrados[k+1], &M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_rpIntrados[j+1].x = M.x;
        m_rpIntrados[j+1].y = M.y;
        int p=1;
        for (int l=k+1; l<=m_iInt; l++)
        {
            m_rpIntrados[j+1+p]  = m_rpIntrados[l];
            p++;
        }
        m_iInt = j+p;
    }
}


/**
 * Modifies the geometry of the current foil by setting the trailing edge flap.
 * The specification for the flap is assumed to have been set previously
 */
void Foil::setTEFlap()
{
    int j{0};
    double xh{0}, yh{0}, dx{0}, dy{0};
    Vector2d M;
    bool bIntersect{false};

    double cosa = cos(m_TEFlapAngle*PI/180.0);
    double sina = sin(m_TEFlapAngle*PI/180.0);
    //first convert xhinge and yhinge in absolute coordinates
    xh = m_TEXHinge/100.0;
    double ymin = baseLowerY(xh);
    double ymax = baseUpperY(xh);
    yh = ymin + m_TEYHinge/100.0 * (ymax-ymin);

    // insert a breakpoint at xhinge location, if there isn't one already
    int iUpperh = 0;
    int iLowerh = 0;
    for (int i=0; i<m_iExt; i++)
    {
        if(qAbs(m_rpExtrados[i].x-xh)<0.001)
        {
            //then no need to add an extra point, just break
            iUpperh = i;
            break;
        }
        else if(m_rpExtrados[i].x>xh)
        {
            for(j=m_iExt+1; j>i; j--)
            {
                m_rpExtrados[j].x = m_rpExtrados[j-1].x;
                m_rpExtrados[j].y = m_rpExtrados[j-1].y;
            }
            m_rpExtrados[i].x = xh;
            m_rpExtrados[i].y = ymax;
            iUpperh = i;
            m_iExt++;
            break;
        }
    }

    for (int i=0; i<m_iInt; i++)
    {
                    if(qAbs(m_rpIntrados[i].x-xh)<0.001)
        {
            //then no need to add an Intra point, just break
            iLowerh = i;
            break;
        }
        else if(m_rpIntrados[i].x>xh)
        {
            for(j=m_iInt+1; j>i; j--)
            {
                m_rpIntrados[j].x = m_rpIntrados[j-1].x;
                m_rpIntrados[j].y = m_rpIntrados[j-1].y;
            }
            m_rpIntrados[i].x = xh;
            m_rpIntrados[i].y = ymin;
            iLowerh = i;
            m_iInt++;
            break;
        }
    }

    // rotate all points downstream of xh
    if(m_TEFlapAngle>0.0)
    {
        //insert an extra point on intrados
        for (int i=m_iInt+1; i>iLowerh; i--)
        {
            m_rpIntrados[i] = m_rpIntrados[i-1];
        }
        m_iInt++;
        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_rpIntrados[iLowerh+1].x += 30.0*(m_rpIntrados[iLowerh+1].x - m_rpIntrados[iLowerh+2].x);
        m_rpIntrados[iLowerh+1].y += 30.0*(m_rpIntrados[iLowerh+1].y - m_rpIntrados[iLowerh+2].y);
        m_rpIntrados[iLowerh].x   += 30.0*(m_rpIntrados[iLowerh].x   - m_rpIntrados[iLowerh-1].x);
        m_rpIntrados[iLowerh].y   += 30.0*(m_rpIntrados[iLowerh].y   - m_rpIntrados[iLowerh-1].y);
    }
    if(m_TEFlapAngle<0.0)
    {
        //insert an extra point on extrados
        for (int i=m_iExt+1; i>iUpperh; i--)
        {
            m_rpExtrados[i] = m_rpExtrados[i-1];
        }
        m_iExt++;

        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_rpExtrados[iUpperh+1].x += 30.0*(m_rpExtrados[iUpperh+1].x-m_rpExtrados[iUpperh+2].x);
        m_rpExtrados[iUpperh+1].y += 30.0*(m_rpExtrados[iUpperh+1].y-m_rpExtrados[iUpperh+2].y);
        m_rpExtrados[iUpperh].x   += 30.0 * (m_rpExtrados[iUpperh].x-m_rpExtrados[iUpperh-1].x);
        m_rpExtrados[iUpperh].y   += 30.0 * (m_rpExtrados[iUpperh].y-m_rpExtrados[iUpperh-1].y);
    }
    for (int i=iUpperh+1; i<=m_iExt; i++)
    {
        dx = m_rpExtrados[i].x-xh;
        dy = m_rpExtrados[i].y-yh;
        m_rpExtrados[i].x = xh + cosa * dx + sina * dy;
        m_rpExtrados[i].y = yh - sina * dx + cosa * dy;
    }
    for (int i=iLowerh+1; i<=m_iInt; i++)
    {
        dx = m_rpIntrados[i].x-xh;
        dy = m_rpIntrados[i].y-yh;
        m_rpIntrados[i].x = xh + cosa * dx + sina * dy;
        m_rpIntrados[i].y = yh - sina * dx + cosa * dy;
    }

    Spline LinkSpline;
    LinkSpline.m_iRes = 4;
    LinkSpline.m_iDegree = 2;
    LinkSpline.m_CtrlPt.clear();

    if(m_TEFlapAngle<0.0)
    {
        //define a 3 ctrl-pt spline to smooth the connection between foil and flap on bottom side
        intersect(m_rpIntrados[iLowerh-1], m_rpIntrados[iLowerh],
                  m_rpIntrados[iLowerh+1], m_rpIntrados[iLowerh+2], &M);

        //sanity check
        if(M.x <= m_rpIntrados[iLowerh].x || M.x >= m_rpIntrados[iLowerh+1].x)
            M = (m_rpIntrados[iLowerh] + m_rpIntrados[iLowerh+1])/2.0;

        LinkSpline.insertPoint(m_rpIntrados[iLowerh].x,m_rpIntrados[iLowerh].y);
        LinkSpline.insertPoint(M.x, M.y);
        LinkSpline.insertPoint(m_rpIntrados[iLowerh+1].x,m_rpIntrados[iLowerh+1].y);
        LinkSpline.splineKnots();
        LinkSpline.splineCurve();
        //retrieve point 1 and 2 and insert them
        for (int i=m_iInt; i>=iLowerh+1; i--)
        {
            m_rpIntrados[i+2].x = m_rpIntrados[i].x;
            m_rpIntrados[i+2].y = m_rpIntrados[i].y;
        }

        m_rpIntrados[iLowerh+2].x = LinkSpline.m_Output[2].x;
        m_rpIntrados[iLowerh+2].y = LinkSpline.m_Output[2].y;
        m_rpIntrados[iLowerh+1].x = LinkSpline.m_Output[1].x;
        m_rpIntrados[iLowerh+1].y = LinkSpline.m_Output[1].y;

        m_iInt+=2;
    }
    else if(m_TEFlapAngle>0.0)
    {
        //define a 3 ctrl-pt spline to smooth the connection between foil and flap on top side
        intersect(m_rpExtrados[iUpperh-1], m_rpExtrados[iUpperh],
                  m_rpExtrados[iUpperh+1], m_rpExtrados[iUpperh+2], &M);

        //sanity check
        if(M.x <= m_rpExtrados[iUpperh].x || M.x >= m_rpExtrados[iUpperh+1].x)
            M = (m_rpExtrados[iUpperh] + m_rpExtrados[iUpperh+1])/2.0;

        LinkSpline.insertPoint(m_rpExtrados[iUpperh].x,m_rpExtrados[iUpperh].y);
        LinkSpline.insertPoint(M.x, M.y);
        LinkSpline.insertPoint(m_rpExtrados[iUpperh+1].x,m_rpExtrados[iUpperh+1].y);
        LinkSpline.splineKnots();
        LinkSpline.splineCurve();

        //retrieve point 1 and 2 and insert them
        for (int i=m_iExt; i>=iUpperh+1; i--)
        {
            m_rpExtrados[i+2].x = m_rpExtrados[i].x;
            m_rpExtrados[i+2].y = m_rpExtrados[i].y;
        }

        m_rpExtrados[iUpperh+2].x = LinkSpline.m_Output[2].x;
        m_rpExtrados[iUpperh+2].y = LinkSpline.m_Output[2].y;
        m_rpExtrados[iUpperh+1].x = LinkSpline.m_Output[1].x;
        m_rpExtrados[iUpperh+1].y = LinkSpline.m_Output[1].y;

        m_iExt+=2;
    }

    // trim upper surface first
    int i1 = iUpperh;
    int i2 = iUpperh+1;

    bIntersect = false;

    int k=0;
    for (j=i2; j<m_iExt; j++)
    {
        for (k=i1; k>0; k--)
        {
            if(intersect(m_rpExtrados[j], m_rpExtrados[j+1], m_rpExtrados[k], m_rpExtrados[k-1], &M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_rpExtrados[k] = M;
        int p=1;
        for (int l=j+1; l<=m_iExt; l++)
        {
            m_rpExtrados[k+p]  = m_rpExtrados[l];
            p++;
        }
        m_iExt = k+p-1;
    }
    // trim lower surface next

    i1 = iLowerh;
    i2 = iLowerh+1;

    bIntersect = false;
    for (j=i2; j<m_iInt; j++)
    {
        for (k=i1; k>0; k--)
        {
            if(intersect(m_rpIntrados[j], m_rpIntrados[j+1], m_rpIntrados[k], m_rpIntrados[k-1], &M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_rpIntrados[k] = M;
        int p=1;
        for (int l=j+1; l<=m_iInt; l++)
        {
            m_rpIntrados[k+p]  = m_rpIntrados[l];
            p++;
        }
        m_iInt = k+p-1;
    }
}

/**
 * Creates the leading and trailing edge flaps on the current geometry.
 */
void Foil::setFlap()
{
    //copy the base foil to the current foil
    memcpy(m_rpExtrados, m_BaseExtrados, sizeof(m_rpExtrados));
    memcpy(m_rpIntrados, m_BaseIntrados, sizeof(m_rpIntrados));

    m_iExt = m_iBaseExt;
    m_iInt = m_iBaseInt;

    if(m_bLEFlap) setLEFlap();
    if(m_bTEFlap) setTEFlap();

    //And finally rebuild the current foil
    for (int i=m_iExt; i>=0; i--)
    {
        m_x[m_iExt-i] = m_rpExtrados[i].x;
        m_y[m_iExt-i] = m_rpExtrados[i].y;
    }
    for (int i=1; i<=m_iInt; i++)
    {
        m_x[m_iExt+i] = m_rpIntrados[i].x;
        m_y[m_iExt+i] = m_rpIntrados[i].y;
    }
    
    m_n = m_iExt + m_iInt + 1;

    if(m_bTEFlap)
    {
        //rotate the mid line
        int im = 0;
        //restore the mid line from the base flap
        memcpy(m_rpMid, m_rpBaseMid, sizeof(m_rpMid));

        //convert xhinge and yhinge in absolute coordinates
        double xh = m_TEXHinge/100.0;
        double ymin = baseLowerY(xh);
        double ymax = baseUpperY(xh);
        double yh = ymin + m_TEYHinge/100.0 * (ymax-ymin);

        Vector2d hinge(xh, yh);

        while(im<MIDPOINTCOUNT)
        {
            if(m_rpMid[im].x>=hinge.x)
            {
                // rotate around XHinge
                m_rpMid[im].rotateZ(hinge, -m_TEFlapAngle);
            }

            im++;
        }
    }
}


/** For debug purposes only */
void Foil::displayCoords(bool bBaseCoords) const
{
    if(bBaseCoords)
    {
        for(int i=0; i<m_nb; i++)
        {
            qDebug(" %13.5f   %13.5f", m_xb[i], m_yb[i]);
        }
    }
    else
    {
        for(int i=0; i<m_n; i++)
        {
            qDebug(" %13.5f   %13.5f", m_x[i], m_y[i]);
        }
    }
}


/**
 * Set the graphics style to the edit mode which is the grey line with the dots.
 */
void Foil::setEditStyle()
{
    setPointStyle(Line::LITTLECIRCLE);
    setVisible(true);
    setColor(160,160,160);
    setLineStipple(Line::DASH);
    setLineWidth(1);
    setVisible(true);
}


QString Foil::properties() const
{
    QString props, str1;
    props = m_Name + "\n";

    str1 = QString(QObject::tr("Thickness         = %1")).arg(thickness()*100.0, 6, 'f', 2);
    props += str1 + "%\n";

    str1 = QString(QObject::tr("Max. Thick.pos.   = %1")).arg(xThickness()*100.0, 6, 'f', 2);
    props += str1 + "%\n";

    str1 = QString(QObject::tr("Max. Camber       = %1")).arg( camber()*100.0, 6, 'f', 2);
    props += str1 + "%\n";

    str1 = QString(QObject::tr("Max. Camber pos.  = %1")).arg(xCamber()*100.0, 6, 'f', 2);
    props += str1 + "%\n";

    str1 = QString(QObject::tr("Number of Panels  =  %1")).arg( m_n);
    props += str1;

    return props;
}
