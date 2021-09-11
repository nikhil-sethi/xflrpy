/****************************************************************************

    Particle Class
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


#include <QRandomGenerator>

#include "particle.h"
#include <xflcore/constants.h>


Particle::Particle()
{
    resizeArrays(0, 1, 1);
}


void Particle::resizeArrays(int ndim, int nobj, int nbest)
{
    m_nBest = nbest;

    m_Position.resize(ndim);
    m_Velocity.resize(ndim);

    m_Fitness.resize(nobj);
    m_Error.resize(nobj);
    m_Error.fill(LARGEVALUE);


    m_BestPosition.resize(m_nBest);
    m_BestError.resize(m_nBest);
    for(int i=0; i<m_nBest; i++)
    {
        m_BestPosition[i].resize(ndim);
        m_BestPosition[i].fill(0);

        m_BestError[i].resize(nobj);
        m_BestError[i].fill(LARGEVALUE);
    }
}


void Particle::initializeBest()
{
    for(int i=0; i<m_nBest; i++)
    {
        m_BestPosition[i] = m_Position;
        m_BestError[i].fill(LARGEVALUE);
    }
}


/** Stores the current best if it is non-dominated by the existing solutions */
void Particle::updateBest()
{
    for(int i=0; i<m_nBest; i++)
    {
        QVector<double> const &olderror = m_BestError.at(i);

        for(int j=0; j<m_Error.size(); j++)
        {
            if(olderror.at(j)>m_Error.at(j))
            {
                // this particle dominates the old personal best, so replace it
                m_BestError[i] = m_Error;
                m_BestPosition[i] = m_Position;
                return;
            }
        }
    }
}


bool Particle::dominates(Particle const* pOther) const
{
    for(int j=0; j<m_Error.size(); j++)
    {
        if(m_Error.at(j) > pOther->m_Error.at(j))
        {
            return false;
        }
    }
    return true;
}




