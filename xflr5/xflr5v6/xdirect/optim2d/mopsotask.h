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

#pragma once

#include "optimtask.h"

#include <xflcore/xflevents.h>
#include <xdirect/optim2d/particle.h>
#include <xdirect/optim2d/optstructures.h>


class MOPSOTask : public OptimTask
{
    Q_OBJECT

        friend class Optim2d;
        friend class Optim3d;

    public:
        MOPSOTask();

        void setNObjectives(int nObj) {m_Objective.resize(nObj);}

        void makeSwarm() override;

        int nObjectives() const {return m_Objective.size();}
        OptObjective const & objective(int iobj) const {return m_Objective.at(iobj);}
        void setObjective(int iobj, const OptObjective &obj) {m_Objective[iobj] = obj;}
        void updateErrors();
        void clearPareto(){ m_Pareto.clear();}

        static void restoreDefaults();

    private:
        void makeParetoFront();
        void makeRandomParticle(Particle *pParticle) const override;
        void moveParticle(Particle *pParticle) const;

        void postIterEvent(int iBest) override;
        void postPSOEvent(int iBest);

        // redeclare for QtConcurrent
        virtual double error(Particle const *pParticle, int iObjective) const override = 0;
        virtual void calcFitness(Particle *pParticle) const override = 0;

    private slots:
        void onSwarm();
        void onIteration() override; // in case the iteration is triggered by a timer

    protected:

        QVector<Particle> m_Pareto; /**< the particles which make the Pareto front */


        // size = nObjectives
        QVector<OptObjective> m_Objective;


        static int    s_ArchiveSize;
        static double s_InertiaWeight;
        static double s_CognitiveWeight;
        static double s_SocialWeight;
        static double s_ProbRegenerate;
};

