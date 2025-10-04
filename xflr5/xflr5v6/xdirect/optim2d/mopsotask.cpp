/****************************************************************************

    MOPSOTask Class
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
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include <QRandomGenerator>

#include "mopsotask.h"
//#include <xdirect/optim2d/optimevent.h>
#include <xflcore/constants.h>

#include <QtConcurrent/QtConcurrent>


#include "mopsotask.h"
#include <xflcore/constants.h>

int    MOPSOTask::s_ArchiveSize       = 10;
double MOPSOTask::s_InertiaWeight     = 0.3;
double MOPSOTask::s_CognitiveWeight   = 0.7;
double MOPSOTask::s_SocialWeight      = 0.7;
double MOPSOTask::s_ProbRegenerate    = 0.05;


MOPSOTask::MOPSOTask()
{
}


void MOPSOTask::restoreDefaults()
{
    s_PopSize           = 31;
    s_ArchiveSize       = 10;
    s_MaxIter           = 100;
    s_InertiaWeight     = 0.3;
    s_CognitiveWeight   = 0.7;
    s_SocialWeight      = 0.7;
    s_ProbRegenerate    = 0.05;
}


void MOPSOTask::makeSwarm()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Iter = 0;

    outputMsg("Making swarm...\n");
    m_Swarm.resize(s_PopSize);

    // no need to multithread, no fitness calculation
    for (int i=0; i<m_Swarm.size(); ++i)
    {
        Particle &particle = m_Swarm[i];
        makeRandomParticle(&particle);
    }

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
           Particle &particle = m_Swarm[isw];
           futureSync.addFuture(QtConcurrent::run(this, &MOPSOTask::calcFitness, &particle));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int i=0; i<m_Swarm.size(); i++)
            calcFitness(&m_Swarm[i]);
    }

    for(int i=0; i<m_Swarm.size(); i++)
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
            m_Swarm[i].setError(iobj, error(&m_Swarm.at(i), iobj));

    outputMsg(QString::asprintf("Made %d random particles\n", m_Swarm.size()));

    QApplication::restoreOverrideCursor();
}


void MOPSOTask::onSwarm()
{
    if(m_Swarm.size()==0 || m_Swarm.size()!=s_PopSize)
    {
        outputMsg("Swarm has not been created\n");
        postPSOEvent(-1); // notifiy finished
        moveToThread(QApplication::instance()->thread());
        return;
    }

    m_Status = xfl::RUNNING;
    m_Iter=0;

    do
    {
        onIteration();
    }
    while(m_Status==xfl::RUNNING);
}


void MOPSOTask::onIteration()
{
    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
           Particle &particle = m_Swarm[isw];
           futureSync.addFuture(QtConcurrent::run(this, &MOPSOTask::moveParticle, &particle));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
            Particle &particle = m_Swarm[isw];
            moveParticle(&particle);
        }
    }

    m_Iter++;

    makeParetoFront();

    // select the best first objective
    int iBest0 = 0; // pareto first, if non better other
    bool bIsConverged = true;
    Particle bestparticle;
    for(int i=0; i<m_Pareto.size(); i++)
    {
        Particle const &particle = m_Pareto.at(i);
        // track the first objective for user information
        if(particle.error(0)<m_Objective.at(0).m_MaxError)
        {
            iBest0 = i;
            bestparticle = particle;
        }

        // check if the particle meets all criteria
        bIsConverged = true;
        for(int io=0; io<particle.nObjectives(); io++)
        {
            if(particle.error(io)>m_Objective.at(io).m_MaxError)
            {
                bIsConverged = false;
                break;
            }
        }
        if(bIsConverged)            break;
    }

    postIterEvent(iBest0);

    if(m_Iter>=s_MaxIter || bIsConverged || m_Status==xfl::CANCELLED)
    {
        if     (bIsConverged)             outputMsg("   ---Converged---\n");
        else if(m_Status==xfl::CANCELLED) outputMsg("The task has been cancelled\n");
        else if(m_Iter>=s_MaxIter)        outputMsg("The maximum number of iterations has been reached\n");

        m_Status = xfl::FINISHED;

        postPSOEvent(iBest0); // tell the GUI that the task is done

        // this task may be resumed, so move it back to the main GUI thread
        moveToThread(QApplication::instance()->thread());
    }
}


void MOPSOTask::moveParticle(Particle *pParticle) const
{
    double newpos=0, vel=0;
    double r1=0, r2=0;

    int igbest=0, ipbest=0;

    // regenerate particles with random probability
    double regen = QRandomGenerator::global()->bounded(1.0);
    if (regen<s_ProbRegenerate)
    {
        makeRandomParticle(pParticle);
    }
    else
    {
        // select a random best in the pareto front
        igbest = QRandomGenerator::global()->bounded(m_Pareto.size());
        Particle const &globalbest = m_Pareto.at(igbest);

        // select a random best in the personal bests
        ipbest = QRandomGenerator::global()->bounded(pParticle->nBest());

        // update the velocity
        for(int j=0; j<pParticle->dimension(); j++)
        {
            r1 = QRandomGenerator::global()->bounded(1.0);
            r2 = QRandomGenerator::global()->bounded(1.0);

            vel = s_InertiaWeight * pParticle->vel(j) +
                  s_CognitiveWeight * r1 * (pParticle->bestPos(ipbest, j) - pParticle->pos(j)) +
                  s_SocialWeight    * r2 * (globalbest.pos(j)             - pParticle->pos(j));

            //test if the velocity moves the particle off-bound and if true bounce it in the opposite direction
            newpos = pParticle->pos(j) + vel;
            if(newpos<m_Variable.at(j).m_Min || newpos>m_Variable.at(j).m_Max)
                vel = -vel;

            pParticle->setVel(j, vel);

            // update the position
            pParticle->setPos(j, pParticle->pos(j) + pParticle->vel(j));
        }
    }

    checkBounds(*pParticle);

    calcFitness(pParticle); // note: do not parallelize in derived class
    for(int i=0; i<m_Objective.size(); i++)  pParticle->setError(i, error(pParticle, i));

    pParticle->updateBest();
}


/** Posted when an iteration has ended */
void MOPSOTask::postIterEvent(int iBest)
{
    OptimEvent *pIterEvent = new OptimEvent(OPTIM_ITER_EVENT, m_Iter, iBest, m_Pareto.at(iBest));
    qApp->postEvent(m_pParent, pIterEvent);
}


/** Posted when the iteration loop has ended */
void MOPSOTask::postPSOEvent(int iBest)
{
    OptimEvent *pPSOEvent = new OptimEvent(OPTIM_END_EVENT, m_Iter, iBest, m_Pareto.at(iBest));
    qApp->postEvent(m_pParent, pPSOEvent);
}

#define PARTICLEBESTSIZE 3
void MOPSOTask::makeRandomParticle(Particle *pParticle) const
{
    int nBest = std::min(m_Objective.size(), PARTICLEBESTSIZE);
    pParticle->resizeArrays(m_Variable.size(), m_Objective.size(), nBest);
    double pos=0, vel=0;
    double deltap = 0.0;
    double deltav = 0.0;

    for(int i=0; i<pParticle->dimension(); i++)
    {
        deltap = m_Variable.at(i).m_Max - m_Variable.at(i).m_Min;
        pos = m_Variable.at(i).m_Min + QRandomGenerator::global()->bounded(deltap);

        pParticle->setPos(i, pos);
        pParticle->initializeBest();

        deltav = deltap/2.0;
        vel = -deltav/2.0 + QRandomGenerator::global()->bounded(deltav);
        pParticle->setVel(i, vel);
    }
}


void MOPSOTask::updateErrors()
{
    double err=LARGEVALUE;
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle &particle = m_Swarm[j];
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
        {
            err = error(&particle, iobj);
            particle.setError(iobj, err);
        }
        particle.initializeBest();
    }

/*    for(int j=0; j<m_Pareto.size(); j++)
    {
        Particle &particle = m_Pareto[j];
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
        {
            err = error(&particle, iobj);
            particle.setError(iobj, err);
        }
//        particle.initializeBest();
    }*/
}


void MOPSOTask::makeParetoFront()
{
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle const &pj = m_Swarm.at(j);
        bool bIsDominated=false;
        for(int ip=0; ip<m_Pareto.size(); ip++)
        {
            Particle const &ppar = m_Pareto.at(ip);
            if(ppar.dominates(&pj))
            {
                // this particle does not belong to the Pareto front, discard it
                bIsDominated=true;
                break;
            }
        }

        if(!bIsDominated)
        {
            // this particle is worthy
            // check if it dominates any of the existing particles in the Pareto frontier
            for(int ip=m_Pareto.size()-1; ip>=0; ip--)
            {
                Particle const &ppar = m_Pareto.at(ip);
                if(pj.dominates(&ppar))
                {
                    m_Pareto.removeAt(ip);
                }
            }
            m_Pareto.append(pj);
        }
    }

    /** @todo target removal to ensure max dispersion of the Pareto front */
    while(m_Pareto.size()>s_ArchiveSize)
    {
        m_Pareto.removeAt(QRandomGenerator::global()->bounded(m_Pareto.size()));
    }
}
