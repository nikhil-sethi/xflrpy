/****************************************************************************

    Optimization structures Class
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
#include <QString>

namespace xfl
{
    typedef enum {MINIMIZE, EQUALIZE, MAXIMIZE} enumObjectiveType; // defines whether the objective is to minimize, maximize or make equal
}



struct OptObjective
{
    OptObjective() {}
    OptObjective(QString const &name, bool bActive, double target, double maxerror, xfl::enumObjectiveType type) :
        m_Name(name), m_bActive(bActive), m_Target(target), m_MaxError(maxerror), m_Type(type)
    {}

    QString m_Name;
    bool m_bActive = true;
    double m_Target = 0.0;
    double m_MaxError = 0.0;
    xfl::enumObjectiveType m_Type = xfl::EQUALIZE;
};


struct OptVariable
{
    OptVariable() {}
    OptVariable(QString const &name, double min, double max) :
        m_Name(name), m_Min(min), m_Max(max)
    {}

    QString m_Name;
    double m_Min = 0.0;
    double m_Max = 0.0;
};
