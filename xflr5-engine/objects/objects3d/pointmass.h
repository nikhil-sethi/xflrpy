/****************************************************************************

    PointMass Class
    Copyright (C) 2013 Andre Deperrois 

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


#ifndef POINTMASS_H
#define POINTMASS_H

#include <QString>
#include <objects/objects3d/vector3d.h>




class XFLR5ENGINELIBSHARED_EXPORT PointMass
{
public:
    /** The public constructor */
    PointMass()
    {
        m_Mass = 0.0;
    }

    /** Overloaded public constructor */
    PointMass(PointMass *pPtMass)
    {
        m_Mass = pPtMass->m_Mass;
        m_Position = pPtMass->m_Position;
        m_Tag = pPtMass->m_Tag;
    }

    /** Overloaded public constructor */
    PointMass(double const &m, Vector3d const &p, QString const &tag)
    {
        m_Mass = m;
        m_Position = p;
        m_Tag = tag;
    }

    /** Returns the the value of the mass */
    double &mass()      {return m_Mass;}
    
    /** Returns the the position of the mass */
    Vector3d &position() {return m_Position;}
    
    /** Returns the the tag of the mass */
    QString &tag()      {return m_Tag;}

private:
    double m_Mass;          /**< the value of the point mass, in kg */
    Vector3d m_Position;      /**< the absolute position of the point mass */
    QString m_Tag;           /**< the description of the point mass */

};

#endif // POINTMASS_H
