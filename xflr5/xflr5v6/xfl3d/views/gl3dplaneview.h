/****************************************************************************

    gl3dPlaneView Class
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

#include <xfl3d/views/gl3dxflview.h>

class Plane;

class gl3dPlaneView : public gl3dXflView
{
    public:
        gl3dPlaneView(QWidget *pParent = nullptr);
        void setPlane(Plane*pPlane){m_pPlane = pPlane;}

        void resetPlane() {m_bResetglPlane=true;}
        void resetBody()  {m_bResetglBody=true;}

    private:
        void glRenderView() override;

        void paintOverlay() override;
        void resizeGL(int width, int height) override;
        void set3dRotationCenter(QPoint point);
        bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) override;

        void glMake3dObjects() override;

    public slots:


    private:
        Plane const* m_pPlane;

    public:
        bool m_bResetglSectionHighlight;
        bool m_bResetglPlane, m_bResetglBody;
};


