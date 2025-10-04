/****************************************************************************

    Frame Class
    Copyright (C) 2007-2016 André Deperrois 

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

#include "frame.h"

#include <xflanalysis/analysis3d_params.h>
#include <xflcore/constants.h>

int Frame::s_iHighlight = -1;
int Frame::s_iSelect = -1;


/**
* The public constructor
* @param nCtrlPts the number of points with which the Frame is initialized.
*/
Frame::Frame(int nCtrlPts)
{
    m_Position.set(0.0,0.0,0.0);
    m_CtrlPoint.clear();
    for(int ic=0; ic<nCtrlPts; ic++)
    {
        m_CtrlPoint.append(Vector3d(0.0,0.0,0.0));
    }
}


/**
*Identifies if an input point matches with one of the Frame's control points
*@param Point the input point
*@param ZoomFactor the scaling factor to be withdrawn from the Point prior to the comparison.
* @todo withdrawal to be performed from within the calling function.
*@return the index of the point in the array which matches with the input point
*/
int Frame::isPoint(const Vector3d &Point, const double &ZoomFactor) const
{
    int l;
    for(l=0; l<m_CtrlPoint.size(); l++)
    {
        if(sqrt(  (Point.x-m_CtrlPoint[l].x)*(Point.x-m_CtrlPoint[l].x)
                + (Point.y-m_CtrlPoint[l].y)*(Point.y-m_CtrlPoint[l].y)
                + (Point.z-m_CtrlPoint[l].z)*(Point.z-m_CtrlPoint[l].z))<0.005/ZoomFactor)
              return l;
//        if (qAbs(Point.x-m_CtrlPoint[l].y)<0.005/ZoomFactor && qAbs(Point.y-m_CtrlPoint[l].z)<0.005/ZoomFactor) return l;
    }
    return -10;
}


/**
 * Loads or Saves the data of this spline to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool Frame::serializeFrame(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;
    int k,n;
    float fx, fy, fz;

    if(bIsStoring)
    {
        ar << 1000;
        //1000 : first format
        ar << m_CtrlPoint.size();
        for(k=0; k<m_CtrlPoint.size(); k++)
        {
            ar << m_CtrlPoint[k].xf() << m_CtrlPoint[k].yf() << m_CtrlPoint[k].zf();
        }
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000 || ArchiveFormat>1100) return false;
        ar >> n;
        m_CtrlPoint.clear();
        for(k=0; k<n; k++)
        {
            ar >> fx;
            ar >> fy;
            ar >> fz;
            m_CtrlPoint.append(Vector3d(double(fx), double(fy), double(fz)));
        }
    }
    return true;
}


/**
* Removes a point from the array of control points.
*@param n the index of the control point to remove in the array
*@return true if the input index is within the array's boundaries, false otherwise
*/
bool Frame::removePoint(int n)
{
    if (n>=0 && n<m_CtrlPoint.size())
    {
        m_CtrlPoint.removeAt(n);
        return true;
    }
    return false;
}

/**
*Inserts a new point at a specified index in the array of control points.
* the point is inserted at a mid position between the two adjacent points, or positioned 1/5 of hte distance of the last two points in the array.
*@param n the index at which a new points will be inserted
*/
void Frame::insertPoint(int n)
{
    m_CtrlPoint.insert(n, Vector3d(0.0,0.0,0.0));
    if(n>0 && n<m_CtrlPoint.size()-1)
    {
        m_CtrlPoint[n] = (m_CtrlPoint[n+1] + m_CtrlPoint[n-1])/2.0;
    }
    else if(n==m_CtrlPoint.size()-1)
    {
        m_CtrlPoint[n] = m_CtrlPoint[n-1] + (m_CtrlPoint[n-1] - m_CtrlPoint.first())/5.0;
    }
    s_iSelect = n;
}

/**
*Inserts a new point at a specified index in the array of control points.
* @param n the index at which a new points will be inserted
* @param Pt the coordinates of the point to insert
*/
void Frame::insertPoint(int n, Vector3d const& Pt)
{
    m_CtrlPoint.insert(n, Pt);
    s_iSelect = n;
}


/**
* Inserts a new point at a position in crescending order on the specified axis.
* @param Real the coordinates of the point to insert
* @param iAxis the axis used as the index key
*/
int Frame::insertPoint(const Vector3d &Real, int iAxis)
{
    int k=0;
    if(iAxis==1)
    {
        if(Real.x>m_CtrlPoint.first().x)
        {
            for(k=0; k<m_CtrlPoint.size()-1; k++)
            {
                if(m_CtrlPoint[k].x<=Real.x && Real.x <m_CtrlPoint[k+1].x)
                {
                    break;
                }
            }
        }
        else k=-1;
    }

    else if(iAxis==2)
    {
        if(Real.y>m_CtrlPoint.first().y)
        {
            for(k=0; k<m_CtrlPoint.size()-1; k++)
            {
                if(m_CtrlPoint[k].y<=Real.y && Real.y <m_CtrlPoint[k+1].y)
                {
                    break;
                }
            }
        }
        else k=-1;
    }
    else if(iAxis==3)
    {
/*        if(Real.z>m_CtrlPoint.last().z)
        {
            for(k=0; k<m_CtrlPoint.size()-1; k++)
            {
                if(m_CtrlPoint[k].z>=Real.z && Real.z >m_CtrlPoint[k+1].z)
                {
                    break;
                }
            }
        }
        else k=-1;*/

        double areal = atan2(Real.y, Real.z) *180/PI;

        for(k=0; k<m_CtrlPoint.count(); k++)
        {
            double actrlPt = atan2(m_CtrlPoint.at(k).y, m_CtrlPoint.at(k).z) *180/PI;
            if(areal<=actrlPt) break;
        }
        k--;
    }

    m_CtrlPoint.insert(k+1, Real);
    s_iSelect = k+1;
    return k+1;
}


/**
 * Returns the Frame's height as the difference of the z-coordinate of the last and first control points.
 *@return the Frame's height
 */
double Frame::height() const
{
    return (m_CtrlPoint.last() - m_CtrlPoint.first()).norm();
/*    double hmin    =  10.0;
    double hmax = -10.0;
    for(int k=0; k<m_CtrlPoint.size(); k++)
    {
        if(m_CtrlPoint[k].z<hmin) hmin = m_CtrlPoint[k].z;
        if(m_CtrlPoint[k].z>hmax) hmax = m_CtrlPoint[k].z;
    }
    return qAbs(hmax-hmin);*/
}


/**
 * Returns the Frame's z-position as the highest and lowest z-values in the array of control points.
 *@return the Frame's z-position
 */
double Frame::zPos() const
{
    double hmin    =  10.0;
    double hmax = -10.0;
    for(int k=0; k<m_CtrlPoint.size(); k++)
    {
        if(m_CtrlPoint[k].z<hmin) hmin = m_CtrlPoint[k].z;
        if(m_CtrlPoint[k].z>hmax) hmax = m_CtrlPoint[k].z;
    }
    return (hmax+hmin)/2.0;
}


/**
 * Copies the data from an existing Frame
 * @param pFrame a pointer to the Frame object from which to copy the data
*/
void Frame::copyFrame(Frame *pFrame)
{
    m_Position = pFrame->m_Position;
    copyPoints(&pFrame->m_CtrlPoint);
}


/**
 * Copies the control point data from an existing list of points
 * @param pPointList a pointer to the list of points
*/
void Frame::copyPoints(QVector<Vector3d> *pPointList)
{
    m_CtrlPoint.clear();
    for(int ip=0; ip<pPointList->size(); ip++)
    {
        m_CtrlPoint.append(pPointList->at(ip));
    }
}


/**
* Appends a new point at the end of the current array
* @param Pt to point to append
*/
void Frame::appendPoint(Vector3d const& Pt)
{
    m_CtrlPoint.append(Pt);
}


/**
* Sets the Frame's absolute position
* @param Pos the new position
*/
void Frame::setPosition(Vector3d Pos)
{
    double zpos;
    if(m_CtrlPoint.isEmpty()) zpos = 0.0;
    else                      zpos = (m_CtrlPoint.first().z + m_CtrlPoint.last().z)/2.0;

    m_Position = Pos;
    for (int ic=0; ic<m_CtrlPoint.count(); ic++)
    {
        m_CtrlPoint[ic].x  = Pos.x;
        m_CtrlPoint[ic].z += Pos.z - zpos;
    }
}


/**
* Set the frame's position on the x-axis
*@param u the new x-position
*/
void Frame::setuPosition(double u)
{
    m_Position.x = u;
    for (int ic=0; ic<m_CtrlPoint.count(); ic++)
    {
        m_CtrlPoint[ic].x = u;
    }
}


/**
* Set the frame's position on the y-axis
*@param v the new y-position
*/
void Frame::setvPosition(double v)
{
    m_Position.y = v;
}

/**
* Set the frame's position on the z-axis
*@param v the new z-position
*/
void Frame::setwPosition(double w)
{
    m_Position.z = w;
}


/**
* Rotates the Control points by a specified angle about the Frame's Oy axis
*@param Angle the rotation angle in degrees
*/
void Frame::rotateFrameY(double Angle)
{
    if(!m_CtrlPoint.size()) return;

//    Vector3d RotationCenter = m_CtrlPoint.first();
    for(int ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic].rotateY(m_Position, Angle);
    }
}









