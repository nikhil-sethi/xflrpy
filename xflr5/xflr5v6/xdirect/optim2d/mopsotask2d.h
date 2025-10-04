/****************************************************************************

    MOPSOTask2d Class
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

#include "mopsotask.h"

class Foil;

class MOPSOTask2d : public MOPSOTask
{
    public:
        MOPSOTask2d();

        void setFoil(Foil const*pFoil, int iLE) {m_pFoil=pFoil; m_iLE=iLE;}
        void setPolar(Polar *pPolar) {m_pPolar=pPolar;}
        void setAlpha(double aoa) {m_Alpha=aoa;}

        void setHHParams(int n, double t2, double amp) {m_HHn=n; m_HHt2=t2; m_HHmax=amp;}

        void makeFoil(Particle const &particle, Foil *pFoil) const;

    private:
        void calcFitness(Particle *pParticle) const override;
        double error(Particle const *pParticle, int iObjective) const override;
        double HH(double x, double t1, double t2) const;

    private:
        Foil const *m_pFoil;

        Polar *m_pPolar;

        int m_iLE;  /**< the index of the leading edge point for thee current aoa */

        double m_HHt2;     /**< t2 parameter of the HH functions */
        int    m_HHn;      /**< number of HH functions to use */
        double m_HHmax;    /**< the max amplitude of the HH functions - unused and deined by the variable's amplitude*/
        double m_Alpha;

};


