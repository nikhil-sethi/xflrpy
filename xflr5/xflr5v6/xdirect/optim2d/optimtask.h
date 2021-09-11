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

#pragma once


#include <QWidget>
#include <QEvent>


#include <xflcore/xflevents.h>
#include <xdirect/optim2d/particle.h>
#include <xdirect/optim2d/optstructures.h>
#include <xflcore/core_enums.h>

#define DELTAVAR 0.0001     // minimum difference between varmax and varmin

#include "particle.h"
#include "optstructures.h"

/**
 * @brief Abstract base class for optimization MOPSO and GA tasks.
 * Run the task using either a worker thread or a timer https://doc.qt.io/qt-5/thread-basics.html
 */

class OptimTask : public QObject
{
    Q_OBJECT

    public:
        OptimTask();

        void setParent(QWidget *pParent) {m_pParent=pParent;}

        virtual void setDimension(int n) {m_Variable.resize(n);}
        virtual void makeSwarm() = 0;


        int swarmSize() const {return m_Swarm.size();}

        int nVariables() const {return m_Variable.size();}
        void setVariables(QVector<OptVariable> const &var) {m_Variable=var;}
        void setVariable(int iDim, const OptVariable&var) {m_Variable[iDim] = var;}
        OptVariable const &variable(int iDim) const {return m_Variable.at(iDim);}
        int nActiveVariables() const;

        Particle const &particle(int ip) const {return m_Swarm.at(ip);}

        void restartIterations() {m_Iter=0;}

        void listPopulation() const;

        void setAnalysisStatus(xfl::enumAnalysisStatus status) {m_Status=status;}
        void cancelAnalyis() {m_Status = xfl::CANCELLED;}
        bool isCancelled() const {return m_Status==xfl::CANCELLED;}
        bool isRunning()   const {return m_Status==xfl::RUNNING;}
        bool isPending()   const {return m_Status==xfl::PENDING;}
        bool isFinished()  const {return m_Status==xfl::FINISHED || m_Status==xfl::CANCELLED;}

        static void setMultithreaded(bool b) {s_bMultiThreaded=b;}

    protected:
        void checkBounds(Particle &particle) const;
        virtual void makeRandomParticle(Particle *pParticle) const = 0;

        void outputMsg(QString const &msg) const;

        virtual double error(Particle const *pParticle, int iObjective) const = 0;
        virtual void calcFitness(Particle *pParticle) const = 0;

        virtual void postIterEvent(int iBest) = 0;
        void postOptEndEvent();

    protected slots:
        virtual void onIteration() = 0;


    protected:
        QVector<Particle> m_Swarm; // the swarm

        int m_Iter;
        QObject *m_pParent;
        xfl::enumAnalysisStatus m_Status;

        // size = dim
        QVector<OptVariable> m_Variable;


    public:
        static int  s_PopSize;
        static int  s_MaxIter;
        static bool s_bMultiThreaded;

};


class OptimEvent : public QEvent
{
    public:
        OptimEvent(QEvent::Type type, int iter, int ibest, Particle const &p): QEvent(type)
        {
            m_Iter  = iter;
            m_iBest = ibest;
            m_Particle = p;
        }

        Particle const &particle() const {return m_Particle;}
        int iter() const {return m_Iter;}
        int iBest() const {return m_iBest;}

    private:
        Particle m_Particle;
        int m_Iter=0;
        int m_iBest = 0;
};






