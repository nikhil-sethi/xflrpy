/****************************************************************************

    gl3dBodyView Class
    Copyright (C) 2016-2019 Andre Deperrois

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

#ifndef GL3DBODYVIEW_H
#define GL3DBODYVIEW_H

#include <gl3dview.h>

class Body;
class GL3dBodyDlg;

class gl3dBodyView : public gl3dView
{
public:
    gl3dBodyView(QWidget *pParent = nullptr);
    void setBody(Body* pBody){m_pBody = pBody;}
    void resetGLBody() {m_bResetglBody = true;}

private:
    void glRenderView();
    void contextMenuEvent (QContextMenuEvent * event);
    void paintGL();
    void paintOverlay();
    void set3DRotationCenter(QPoint point);

    void glMake3DObjects();

public slots:
    void on3DReset();

public:
    Body *m_pBody;

    bool m_bResetglFrameHighlight;
    bool m_bResetglBody;

//    GL3dBodyDlg *m_pglBodyDlg;
};

#endif // GL3DBODYVIEW_H
