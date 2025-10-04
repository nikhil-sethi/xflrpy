/****************************************************************************

    gl3dTestView Class
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

#include <xfl3d/views/gl3dview.h>
#include <xflgeom/geom3d/vector3d.h>

class gl3dTestGLView : public gl3dView
{
    Q_OBJECT

    public:
        gl3dTestGLView(QWidget *pParent = nullptr);
        bool intersectTheObject(Vector3d const &,  Vector3d const &, Vector3d &) override {return false;}


    protected:
        void glRenderView() override;
        void glMake3dObjects() override {}
        void showEvent(QShowEvent *pEvent) override;

    signals:
        void ready() const;


    protected:
        bool m_bInitialized;

};

