/****************************************************************************

    OptimEvent Class
    Copyright (C) 2021 André Deperrois

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

#include <xflcore/xflevents.h>
#include <xdirect/optim2d/particle.h>
/*
class OptimEvent : public QEvent
{
    public:
        OptimEvent(int iter, int ibest, QVector<Particle> const &lastswarm, QVector<Particle> const &lastpareto): QEvent(PSO_ITER_EVENT)
        {
            m_Iter  = iter;
            m_iBest = ibest;

            m_Swarm  = lastswarm;
            m_Pareto = lastpareto;
        }

        int iter() const {return m_Iter;}
        int iBest() const {return m_iBest;}
        QVector<Particle> const &swarm() const {return m_Swarm;}
        QVector<Particle> const &pareto() const {return m_Pareto;}

    private:
        QVector<Particle> m_Swarm;
        QVector<Particle> m_Pareto;
        int m_Iter=0;
        int m_iBest = 0;
};

*/
