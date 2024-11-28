/****************************************************************************

    PointMass Class
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


#pragma once

#include <QString>
#include <xflgeom/geom3d/vector3d.h>


class PointMass
{
    public:
        /** The public constructor */
        PointMass() : m_Mass{0}
        {
        }
        /** Overloaded public constructor */
        PointMass(double m, Vector3d const &p, QString const &tag) : m_Mass{m}, m_Position{p}, m_Tag{tag}
        {
        }

        /** Returns the value of the mass */
        double mass() const  {return m_Mass;}
        void setMass(double mass){m_Mass = mass;}

        /** Returns the the position of the mass */
        const Vector3d & position() const {return m_Position;}
        void setPosition(Vector3d pos) {m_Position=pos;}
        void setXPosition(double x){m_Position.x=x;}
        void setYPosition(double y){m_Position.y=y;}
        void setZPosition(double z){m_Position.z=z;}


        /** Returns the the tag of the mass */
        const QString &tag() const {return m_Tag;}
        void setTag(QString const &tag) {m_Tag=tag;}

        bool isSame(PointMass const &pm, Vector3d const& translated) const
        {
            if(fabs(pm.mass()-m_Mass)>0.001) return false;
            if(!m_Position.isSame(pm.position()+translated, 0.001)) return false;
            return true;
        }

    private:
        double m_Mass;          /**< the value of the point mass, in kg */
        Vector3d m_Position;      /**< the absolute position of the point mass */
        QString m_Tag;           /**< the description of the point mass */

};
