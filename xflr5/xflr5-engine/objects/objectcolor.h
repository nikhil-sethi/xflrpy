/****************************************************************************

    ObjectColor Class
    Copyright (C) 2018 Andre Deperrois

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



struct ObjectColor
{
    ObjectColor()
    {
        m_red = m_green = m_blue = m_alpha = 255;
    }

    ObjectColor(int r, int g, int b, int a=255)
    {
        setColor(r,g,b,a);
    }

    void setColor(int r, int g, int b, int a=255)
    {
        m_red=r;
        m_green=g;
        m_blue=b;
        m_alpha=a;
    }

    void setColorF(float r, float g, float b, float a=1.0f)
    {
        m_red=int(r*255.0f);
        m_green=int(g*255.0f);
        m_blue=int(b*255.0f);
        m_alpha=int(a*255.0f);
    }

    void setRed(int r) {m_red = r;}
    void setGreen(int g) {m_green = g;}
    void setBlue(int b) {m_blue = b;}
    void setAlpha(int a) {m_alpha = a;}

    int red() const {return m_red;}
    int green() const {return m_green;}
    int blue() const {return m_blue;}
    int alpha() const {return m_alpha;}

    void setAlphaF(float a) {m_alpha = int(a*255.0f);}

    float redf() const {return (float)m_red/255.0f;}
    float greenf() const {return (float)m_green/255.0f;}
    float bluef() const {return (float)m_blue/255.0f;}
    float alphaf() const {return (float)m_alpha/255.0f;}

private:
    int m_red;
    int m_green;
    int m_blue;
    int m_alpha;
};
