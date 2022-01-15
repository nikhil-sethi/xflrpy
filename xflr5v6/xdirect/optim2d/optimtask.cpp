/****************************************************************************

    OptimTask Class
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

#include <QApplication>
#include <QDebug>

#include "optimtask.h"

int  OptimTask::s_PopSize           = 31;
int  OptimTask::s_MaxIter           = 100;
bool OptimTask::s_bMultiThreaded    = true;


OptimTask::OptimTask()
{
    m_Iter = 0;
    m_Status = xfl::PENDING;
}


void OptimTask::outputMsg(QString const &msg) const
{
    MessageEvent * pMsgEvent = new MessageEvent(msg);
    qApp->postEvent(m_pParent, pMsgEvent);
}


void OptimTask::checkBounds(Particle &particle) const
{
    for(int i=0; i<particle.dimension(); i++)
    {
        particle.setPos(i, std::max(m_Variable.at(i).m_Min, particle.pos(i)));
        particle.setPos(i, std::min(m_Variable.at(i).m_Max, particle.pos(i)));
    }
}


void OptimTask::listPopulation() const
{
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle const &ind = m_Swarm.at(i);
        qDebug(" p[%d]: Cl=%7.3g  Err=%7.3g", i, ind.fitness(0), ind.error(0));
    }
    qDebug();
}


/** Posted when the iteration loop has ended */
void OptimTask::postOptEndEvent()
{
    QEvent *pOptimEvent = new QEvent(OPTIM_END_EVENT);
    qApp->postEvent(m_pParent, pOptimEvent);
}


int OptimTask::nActiveVariables() const
{
    int nActive = 0;
    for(int ivar=0; ivar<m_Variable.size(); ivar++)
    {
        OptVariable const &var = m_Variable.at(ivar);
        if(var.m_Max-var.m_Min>DELTAVAR) nActive++;
    }
    return nActive;
}



