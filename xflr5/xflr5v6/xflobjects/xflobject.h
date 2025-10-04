/****************************************************************************

    XflObject Class
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

#include <QColor>
#include <xflcore/linestyle.h>


/**
 * @brief The XflObject class the base class for Xfl type objects
 */

class XflObject
{
    public:
        XflObject() = default;
        virtual ~XflObject() = default;

    public:
        virtual QString name() const {return m_Name;}
        virtual void setName(QString const & name) {m_Name=name;}

        QColor const &lineColor() const {return m_theStyle.m_Color;}
        void setLineColor(QColor clr) {m_theStyle.m_Color=clr;}

        Line::enumLineStipple lineStipple() const {return m_theStyle.m_Stipple;}
        void setLineStipple(Line::enumLineStipple iStipple) {m_theStyle.m_Stipple=iStipple;}

        int lineWidth() const {return m_theStyle.m_Width;}
        void setLineWidth(int iWidth) {m_theStyle.m_Width=iWidth;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible = bVisible;}

        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle iPointStyle) {m_theStyle.m_Symbol=iPointStyle;}

        void setTheStyle(LineStyle const &ls) {m_theStyle=ls;}

        void setTheStyle(Line::enumLineStipple stipple, int w, const QColor &clr, Line::enumPointStyle pointstyle)
        {
            m_theStyle.m_Stipple = stipple;
            m_theStyle.m_Width = w;
            m_theStyle.m_Color = clr;
            m_theStyle.m_Symbol = pointstyle;
        }
        LineStyle const &theStyle() const {return m_theStyle;}
        LineStyle &theStyle() {return m_theStyle;}

        int width() const {return m_theStyle.m_Width;}


        QColor const &color() const {return m_theStyle.m_Color;}
        void setColor(QColor const &clr) {m_theStyle.m_Color=clr;}
        void setColor(int r, int g, int b, int a=255) {m_theStyle.m_Color = {r,g,b,a};}

        int red() const {return m_theStyle.m_Color.red();}
        int green() const {return m_theStyle.m_Color.green();}
        int blue() const {return m_theStyle.m_Color.blue();}
        int alphaChannel() const {return m_theStyle.m_Color.alpha();}
        Line::enumLineStipple polarStyle() const     {return m_theStyle.m_Stipple;}

        void setStipple(int n) {m_theStyle.m_Stipple = LineStyle::convertLineStyle(n);} // conversion
        void setStipple2(Line::enumLineStipple s) {m_theStyle.m_Stipple=s;}
        void setWidth(int w) {m_theStyle.m_Width=w;}


    protected:
        LineStyle m_theStyle;
        QString m_Name;

};

