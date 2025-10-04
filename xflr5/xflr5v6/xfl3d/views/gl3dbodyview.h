/****************************************************************************

    gl3dBodyView Class
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

class Body;
class BodyDlg;

class gl3dBodyView : public gl3dXflView
{
    public:
        gl3dBodyView(QWidget *pParent = nullptr);
        void setBody(Body* pBody){m_pBody = pBody;}
        void resetGLBody() {m_bResetglBody = true;}

    private:
        void glRenderView() override;
        void contextMenuEvent (QContextMenuEvent * event) override;
        bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) override;

        void glMake3dObjects() override;
        void glMakePanels(QOpenGLBuffer &vbo, int nPanels, const Vector3d *pNode, const Panel *pPanel);


    private slots:
        void on3dReset() override;

    public:
        Body *m_pBody;

        bool m_bResetglFrameHighlight;
        bool m_bResetglBody;

};


