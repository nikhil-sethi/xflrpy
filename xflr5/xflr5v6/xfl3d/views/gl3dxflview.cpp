/****************************************************************************

    gl3dXflView Class
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

#include <QOpenGLFunctions>
#include <QDir>

#include "gl3dxflview.h"

#include <xfl3d/controls/w3dprefs.h>
#include <xflanalysis/plane_analysis/lltanalysis.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/body.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/pointmass.h>
#include <xflobjects/objects3d/surface.h>
#include <xflobjects/objects3d/wing.h>
#include <xflobjects/objects3d/wpolar.h>

Miarex *gl3dXflView::s_pMiarex(nullptr);
MainFrame *gl3dXflView::s_pMainFrame(nullptr);

gl3dXflView::gl3dXflView(QWidget *pParent) : gl3dView(pParent)
{
    m_bOutline    = true;
    m_bSurfaces   = true;
    m_bVLMPanels  = false;
    m_bAxes       = true;
    m_bShowMasses = false;
    m_bFoilNames  = false;

    m_nHighlightLines = m_HighlightLineSize = 0;
    memset(m_Ny, 0, sizeof(m_Ny));
}


gl3dXflView::~gl3dXflView()
{
    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboEditWingMesh[iWing].destroy();
    }

    m_vboBody.destroy();
    m_vboEditBodyMesh.destroy();

    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboWingSurface[iWing].destroy();
        m_vboWingOutline[iWing].destroy();
    }
}


void gl3dXflView::enterEvent(QEvent *pEvent)
{
    setFocus();
    gl3dView::enterEvent(pEvent);
}


void gl3dXflView::onSurfaces(bool bChecked)
{
    m_bSurfaces = bChecked;
    update();
}


void gl3dXflView::onOutline(bool bChecked)
{
    m_bOutline = bChecked;
    update();
}


void gl3dXflView::onPanels(bool bChecked)
{
    m_bVLMPanels = bChecked;
    update();
}


void gl3dXflView::onFoilNames(bool bChecked)
{
    m_bFoilNames = bChecked;
    update();
}


void gl3dXflView::onShowMasses(bool bChecked)
{
    m_bShowMasses = bChecked;
    update();
}


void gl3dXflView::glMakeFuseFlatPanels(Body const*pBody)
{
    Vector3d P1, P2, P3, P4, N, P1P3, P2P4, Tj, Tjp1;

    int buffersize = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
    buffersize *= 2;    //two triangles per quad
    buffersize *= 3;    //three vertices per triangle
    buffersize *= 8;    // 3 position + 3 normal + 2 UV components

    QVector<float>pBodyVertexArray(buffersize);

    int iv=0;

    float fnh = pBody->sideLineCount();
    float fLength = float(pBody->length());

    float tip = 0.0;
    if(pBody->frameCount()) tip = pBody->frameAt(0)->position().xf();

    //surfaces
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frameAt(j)->position().x,     0.0, 0.0);
            Tjp1.set(pBody->frameAt(j+1)->position().x, 0.0, 0.0);

            P1 = pBody->frameAt(j  )->ctrlPointAt(k);       P1.x = pBody->frameAt(j  )->position().x;
            P2 = pBody->frameAt(j+1)->ctrlPointAt(k);       P2.x = pBody->frameAt(j+1)->position().x;
            P3 = pBody->frameAt(j+1)->ctrlPointAt(k+1);     P3.x = pBody->frameAt(j+1)->position().x;
            P4 = pBody->frameAt(j  )->ctrlPointAt(k+1);     P4.x = pBody->frameAt(j  )->position().x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = (P1P3 * P2P4).normalized(); // flat shading

            // first triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            //second triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;
        }
    }
    Q_ASSERT(iv==buffersize);

    m_vboFuseRight.destroy();
    m_vboFuseRight.create();
    m_vboFuseRight.bind();
    m_vboFuseRight.allocate(pBodyVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_vboFuseRight.release();

    // left side
    iv=0;
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frameAt(j)->position().x,     0.0, 0.0);
            Tjp1.set(pBody->frameAt(j+1)->position().x, 0.0, 0.0);

            P1 = pBody->frameAt(j  )->ctrlPointAt(k);       P1.x = pBody->frameAt(j  )->position().x;
            P2 = pBody->frameAt(j+1)->ctrlPointAt(k);       P2.x = pBody->frameAt(j+1)->position().x;
            P3 = pBody->frameAt(j+1)->ctrlPointAt(k+1);     P3.x = pBody->frameAt(j+1)->position().x;
            P4 = pBody->frameAt(j  )->ctrlPointAt(k+1);     P4.x = pBody->frameAt(j  )->position().x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = P1P3 * P2P4;
            N.normalize();

            P1.y = -P1.y;
            P2.y = -P2.y;
            P3.y = -P3.y;
            P4.y = -P4.y;
            N.y = -N.y;

            //first triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            //second triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

        }
    }
    Q_ASSERT(iv==buffersize);

    m_vboFuseLeft.destroy();
    m_vboFuseLeft.create();
    m_vboFuseLeft.bind();
    m_vboFuseLeft.allocate(pBodyVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_vboFuseLeft.release();
}



void gl3dXflView::glMakeFuseFlatPanelsOutline(const Body *pBody)
{
    Vector3d P1, P2;

    int buffersize = 0;
    buffersize += pBody->frameCount()             //
                 * (pBody->sideLineCount()-1) *2  // number of segments/frame
                 * 2                              // 2 vertices
                 * 3;                             // 3 components


    buffersize += pBody->sideLineCount()          //
                 * (pBody->frameCount()-1) *2     // number of segments/frame
                 * 2                              // 2 vertices
                 * 3;                             // 3 components

    QVector<float>OutlineVertexArray(buffersize);

    int iv=0;
    for (int j=0; j<pBody->frameCount();j++)
    {
        for (int k=0; k<pBody->sideLineCount()-1; k++)
        {
            P1 = pBody->frameAt(j)->ctrlPointAt(k);       P1.x = pBody->frameAt(j)->position().x;
            P2 = pBody->frameAt(j)->ctrlPointAt(k+1);     P2.x = pBody->frameAt(j)->position().x;

            // right side segment
            OutlineVertexArray[iv++] = P1.xf();
            OutlineVertexArray[iv++] = P1.yf();
            OutlineVertexArray[iv++] = P1.zf();

            OutlineVertexArray[iv++] = P2.xf();
            OutlineVertexArray[iv++] = P2.yf();
            OutlineVertexArray[iv++] = P2.zf();

            // left side segment
            OutlineVertexArray[iv++] =  P1.xf();
            OutlineVertexArray[iv++] = -P1.yf();
            OutlineVertexArray[iv++] =  P1.zf();

            OutlineVertexArray[iv++] =  P2.xf();
            OutlineVertexArray[iv++] = -P2.yf();
            OutlineVertexArray[iv++] =  P2.zf();
        }
    }

    for (int k=0; k<pBody->sideLineCount(); k++)
    {
        for (int j=0; j<pBody->frameCount()-1; j++)
        {
            P1 = pBody->frameAt(j  )->ctrlPointAt(k);       P1.x = pBody->frameAt(j  )->position().x;
            P2 = pBody->frameAt(j+1)->ctrlPointAt(k);       P2.x = pBody->frameAt(j+1)->position().x;

            // right side segment
            OutlineVertexArray[iv++] = P1.xf();
            OutlineVertexArray[iv++] = P1.yf();
            OutlineVertexArray[iv++] = P1.zf();

            OutlineVertexArray[iv++] = P2.xf();
            OutlineVertexArray[iv++] = P2.yf();
            OutlineVertexArray[iv++] = P2.zf();

            // left side segment
            OutlineVertexArray[iv++] =  P1.xf();
            OutlineVertexArray[iv++] = -P1.yf();
            OutlineVertexArray[iv++] =  P1.zf();

            OutlineVertexArray[iv++] =  P2.xf();
            OutlineVertexArray[iv++] = -P2.yf();
            OutlineVertexArray[iv++] =  P2.zf();
        }
    }

    Q_ASSERT(iv==buffersize);

    m_vboFuseOutline.destroy();
    m_vboFuseOutline.create();
    m_vboFuseOutline.bind();
    m_vboFuseOutline.allocate(OutlineVertexArray.data(), buffersize * sizeof(GLfloat));
    m_vboFuseOutline.release();
}


void gl3dXflView::glMakeFuseSplines(Body const *pBody)
{
    int NXXXX = W3dPrefs::bodyAxialRes();
    int NHOOOP = W3dPrefs::bodyHoopRes();
    QVector<Vector3d> T((NXXXX+1)*(NHOOOP+1)), N((NXXXX+1)*(NHOOOP+1));
    Vector3d TALB, LATB;

    if(!pBody)return;

    Vector3d Point;
    Vector3d Normal;

    //vertices array size:
    // surface:
    //     (NX+1)*(NH+1) : from 0 to NX, and from 0 to NH
    //     x2 : 2 sides
    // outline:
    //     frameSize()*(NH+1)*2 : frames
    //     (NX+1) + (NX+1)      : top and bottom lines
    //
    // x8 : 3 vertices components, 3 normal components, 2 texture componenents
    int FuseVertexSize(0);

    int nTriangles =  NXXXX*NHOOOP*2;           // quads x2 triangles/quad
    FuseVertexSize  =  nTriangles
                       *3                        // 3 vertices/triangle
                       *8;                       // 3 vertex components, 3 normal components, 2 uv components

    QVector<float> FuseVertexArray(FuseVertexSize);

    int p = 0;
    double ud(0), vd(0);
    for (int k=0; k<=NXXXX; k++)
    {
        ud = double(k) / double(NXXXX);
        for (int l=0; l<=NHOOOP; l++)
        {
            vd = double(l) / double(NHOOOP);
            pBody->getPoint(ud,  vd, true, T[p]);
            pBody->nurbs().getNormal(ud, vd, N[p]);

            p++;
        }
    }

    int nla(0), nlb(0), nta(0), ntb(0);

    int iv=0;
    //right side first;
    p=0;
    for (int k=0; k<NXXXX; k++)
    {
        for (int l=0; l<NHOOOP; l++)
        {
            nta =  k   *(NHOOOP+1)+l;
            ntb =  k   *(NHOOOP+1)+l+1;
            nla = (k+1)*(NHOOOP+1)+l;
            nlb = (k+1)*(NHOOOP+1)+l+1;

            //first triangle
            FuseVertexArray[iv++] = T.at(nta).xf();
            FuseVertexArray[iv++] = T.at(nta).yf();
            FuseVertexArray[iv++] = T.at(nta).zf();
            FuseVertexArray[iv++] = N.at(nta).xf();
            FuseVertexArray[iv++] = N.at(nta).yf();
            FuseVertexArray[iv++] = N.at(nta).zf();
            FuseVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(nla).xf();
            FuseVertexArray[iv++] = T.at(nla).yf();
            FuseVertexArray[iv++] = T.at(nla).zf();
            FuseVertexArray[iv++] = N.at(nla).xf();
            FuseVertexArray[iv++] = N.at(nla).yf();
            FuseVertexArray[iv++] = N.at(nla).zf();
            FuseVertexArray[iv++] = float(NXXXX-(k+1))/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(nlb).xf();
            FuseVertexArray[iv++] = T.at(nlb).yf();
            FuseVertexArray[iv++] = T.at(nlb).zf();
            FuseVertexArray[iv++] = N.at(nlb).xf();
            FuseVertexArray[iv++] = N.at(nlb).yf();
            FuseVertexArray[iv++] = N.at(nlb).zf();
            FuseVertexArray[iv++] = float(NXXXX-(k+1))/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            //second triangle
            FuseVertexArray[iv++] = T.at(nta).xf();
            FuseVertexArray[iv++] = T.at(nta).yf();
            FuseVertexArray[iv++] = T.at(nta).zf();
            FuseVertexArray[iv++] = N.at(nta).xf();
            FuseVertexArray[iv++] = N.at(nta).yf();
            FuseVertexArray[iv++] = N.at(nta).zf();
            FuseVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(nlb).xf();
            FuseVertexArray[iv++] = T.at(nlb).yf();
            FuseVertexArray[iv++] = T.at(nlb).zf();
            FuseVertexArray[iv++] = N.at(nlb).xf();
            FuseVertexArray[iv++] = N.at(nlb).yf();
            FuseVertexArray[iv++] = N.at(nlb).zf();
            FuseVertexArray[iv++] = float(NXXXX-(k+1))/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(ntb).xf();
            FuseVertexArray[iv++] = T.at(ntb).yf();
            FuseVertexArray[iv++] = T.at(ntb).zf();
            FuseVertexArray[iv++] = N.at(ntb).xf();
            FuseVertexArray[iv++] = N.at(ntb).yf();
            FuseVertexArray[iv++] = N.at(ntb).zf();
            FuseVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            p++;
        }
    }

    Q_ASSERT(iv==FuseVertexSize);

    m_vboFuseRight.destroy();
    m_vboFuseRight.create();
    m_vboFuseRight.bind();
    m_vboFuseRight.allocate(FuseVertexArray.data(), FuseVertexSize * int(sizeof(GLfloat)));
    m_vboFuseRight.release();

    //left side next;
    iv=0;
    p=0;
    for (int k=0; k<NXXXX; k++)
    {
        for (int l=0; l<NHOOOP; l++)
        {
            nta =  k   *(NHOOOP+1)+l;
            ntb =  k   *(NHOOOP+1)+l+1;
            nla = (k+1)*(NHOOOP+1)+l;
            nlb = (k+1)*(NHOOOP+1)+l+1;

            //first triangle
            FuseVertexArray[iv++] =  T.at(nta).xf();
            FuseVertexArray[iv++] = -T.at(nta).yf();
            FuseVertexArray[iv++] =  T.at(nta).zf();
            FuseVertexArray[iv++] =  N.at(nta).xf();
            FuseVertexArray[iv++] = -N.at(nta).yf();
            FuseVertexArray[iv++] =  N.at(nta).zf();
            FuseVertexArray[iv++] = float(k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(nlb).xf();
            FuseVertexArray[iv++] = -T.at(nlb).yf();
            FuseVertexArray[iv++] =  T.at(nlb).zf();
            FuseVertexArray[iv++] =  N.at(nlb).xf();
            FuseVertexArray[iv++] = -N.at(nlb).yf();
            FuseVertexArray[iv++] =  N.at(nlb).zf();
            FuseVertexArray[iv++] = float(k+1)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(nla).xf();
            FuseVertexArray[iv++] = -T.at(nla).yf();
            FuseVertexArray[iv++] =  T.at(nla).zf();
            FuseVertexArray[iv++] =  N.at(nla).xf();
            FuseVertexArray[iv++] = -N.at(nla).yf();
            FuseVertexArray[iv++] =  N.at(nla).zf();
            FuseVertexArray[iv++] = float(k+1)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);


            //second triangle
            FuseVertexArray[iv++] =  T.at(nta).xf();
            FuseVertexArray[iv++] = -T.at(nta).yf();
            FuseVertexArray[iv++] =  T.at(nta).zf();
            FuseVertexArray[iv++] =  N.at(nta).xf();
            FuseVertexArray[iv++] = -N.at(nta).yf();
            FuseVertexArray[iv++] =  N.at(nta).zf();
            FuseVertexArray[iv++] = float(k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(ntb).xf();
            FuseVertexArray[iv++] = -T.at(ntb).yf();
            FuseVertexArray[iv++] =  T.at(ntb).zf();
            FuseVertexArray[iv++] =  N.at(ntb).xf();
            FuseVertexArray[iv++] = -N.at(ntb).yf();
            FuseVertexArray[iv++] =  N.at(ntb).zf();
            FuseVertexArray[iv++] = float(k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(nlb).xf();
            FuseVertexArray[iv++] = -T.at(nlb).yf();
            FuseVertexArray[iv++] =  T.at(nlb).zf();
            FuseVertexArray[iv++] =  N.at(nlb).xf();
            FuseVertexArray[iv++] = -N.at(nlb).yf();
            FuseVertexArray[iv++] =  N.at(nlb).zf();
            FuseVertexArray[iv++] = float(k+1)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            p++;
        }
    }

    Q_ASSERT(iv==FuseVertexSize);

    m_vboFuseLeft.destroy();
    m_vboFuseLeft.create();
    m_vboFuseLeft.bind();
    m_vboFuseLeft.allocate(FuseVertexArray.data(), FuseVertexSize * int(sizeof(GLfloat)));
    m_vboFuseLeft.release();
}


void gl3dXflView::glMakeFuseSplinesOutline(Body const*pBody)
{
    if(!pBody) return;

    int NXXXX = W3dPrefs::bodyAxialRes();
    int NHOOOP = W3dPrefs::bodyHoopRes();


    Vector3d Point;
    double hinc(0), u(0), v(0);

    //OUTLINE
    // outline:
    //     frameSize()*(NH+1)*2 : frames
    //     (NX+1) + (NX+1)      : top and bottom lines
    //
    int outlinesize =   pBody->frameCount()*(NHOOOP+1)*2 // frames
                      + (NXXXX+1)                       // top outline
                      + (NXXXX+1);                      // bot outline
    outlinesize *=3; // x3 vertices components

    std::vector<float> OutlineVertexArray(outlinesize);

    hinc=1./(double)NHOOOP;

    int iv=0;
    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pBody->frameCount(); iFr++)
    {
        u = pBody->getu(pBody->frameAt(iFr)->position().x);
        for (int j=0; j<=NHOOOP; j++)
        {
            v = (double)j*hinc;
            pBody->getPoint(u,v,true, Point);
            OutlineVertexArray[iv++] = Point.x;
            OutlineVertexArray[iv++] = Point.y;
            OutlineVertexArray[iv++] = Point.z;
        }

        for (int j=NHOOOP; j>=0; j--)
        {
            v = (double)j*hinc;
            pBody->getPoint(u,v,false, Point);
            OutlineVertexArray[iv++] = Point.x;
            OutlineVertexArray[iv++] = Point.y ;
            OutlineVertexArray[iv++] = Point.z;
        }
    }

    //top line: NX+1
    v = 0.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint((double)iu/(double)NXXXX,v, true, Point);
        OutlineVertexArray[iv++] = Point.x;
        OutlineVertexArray[iv++] = Point.y;
        OutlineVertexArray[iv++] = Point.z;
    }

    //bottom line: NX+1
    v = 1.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint((double)iu/(double)NXXXX,v, true, Point);
        OutlineVertexArray[iv++] = Point.x;
        OutlineVertexArray[iv++] = Point.y;
        OutlineVertexArray[iv++] = Point.z;
    }
    Q_ASSERT(iv==outlinesize);

    m_vboFuseOutline.destroy();
    m_vboFuseOutline.create();
    m_vboFuseOutline.bind();
    m_vboFuseOutline.allocate(OutlineVertexArray.data(), outlinesize * sizeof(GLfloat));
    m_vboFuseOutline.release();
}


void gl3dXflView::paintSectionHighlight()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(255,0,0));
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, 3);
        m_vboHighlight.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
            glLineWidth(5);

            int pos = 0;
            for(int iLines=0; iLines<m_nHighlightLines; iLines++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, m_HighlightLineSize);
                pos += m_HighlightLineSize;
            }

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        m_vboHighlight.release();
    }
    m_shadLine.release();
}


/** Default mesh, if no polar has been defined */
void gl3dXflView::paintEditWingMesh(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_VLMStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_VLMStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_VLMStyle.m_Width);

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            int nTriangles = vbo.size()/3/3/int(sizeof(float)); // three vertices and three components

            f->glLineWidth(W3dPrefs::s_VLMStyle.m_Width);
            int pos = 0;
            for(int p=0; p<nTriangles; p++)
            {
                f->glDrawArrays(GL_LINE_STRIP, pos, 3);
                pos +=3;
            }

            m_shadLine.setUniformValue(m_locLine.m_UniColor, DisplayOptions::backgroundColor());

            f->glEnable(GL_POLYGON_OFFSET_FILL);
            f->glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
            f->glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
            f->glDisable(GL_POLYGON_OFFSET_FILL);
        }
        vbo.release();
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, s_BackgroundColor);

        vbo.bind();
        {
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3);

            int nTriangles = vbo.size()/3/3/int(sizeof(float)); // three vertices and three components

            f->glEnable(GL_POLYGON_OFFSET_FILL);
            f->glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
            f->glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
            f->glDisable(GL_POLYGON_OFFSET_FILL);
        }
        vbo.release();
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
    }
    m_shadSurf.release();
    f->glDisable(GL_POLYGON_OFFSET_FILL);
}


void gl3dXflView::setSpanStations(Plane const *pPlane, WPolar const *pWPolar, PlaneOpp const*pPOpp)
{
    if(!pPlane || !pWPolar || !pPOpp) return;
    Wing const *pWing = nullptr;

    if(pWPolar->isLLTMethod())
    {
        if(pPOpp)
        {
            m_Ny[0] = pPOpp->m_pWOpp[0]->m_NStation-1;
        }
        else
        {
            m_Ny[0] = LLTAnalysis::nSpanStations();
        }

        m_Ny[1] = m_Ny[2] = m_Ny[3] = 0;
    }
    else
    {
        for(int iWing=0; iWing<MAXWINGS; iWing++)
        {
            pWing = pPlane->wingAt(iWing);
            if(pWing)
            {
                m_Ny[iWing]=0;
                for (int j=0; j<pWing->m_Surface.size(); j++)
                {
                    m_Ny[iWing] += pWing->surface(j)->nYPanels();
                }
            }
        }
    }
}


void gl3dXflView::paintFoilNames(Wing const *pWing)
{
    int j=0;
    Foil const *pFoil=nullptr;

    QColor clr(105,105,195);
    if(DisplayOptions::isLightTheme()) clr = clr.darker();
    else                               clr = clr.lighter();

    for(j=0; j<pWing->m_Surface.size(); j++)
    {
        pFoil = pWing->surface(j)->foilA();

        if(pFoil) glRenderText(pWing->surface(j)->m_TA.x, pWing->surface(j)->m_TA.y, pWing->surface(j)->m_TA.z,
                               pFoil->name(),
                               clr);
    }

    j = pWing->m_Surface.size()-1;
    pFoil = pWing->surface(j)->foilB();
    if(pFoil) glRenderText(pWing->surface(j)->m_TB.x, pWing->surface(j)->m_TB.y, pWing->surface(j)->m_TB.z,
                           pFoil->name(),
                           clr);
}


void gl3dXflView::paintMasses(double volumeMass, const Vector3d &pos, const QString &tag, const QVector<PointMass*> &ptMasses)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QColor massclr = W3dPrefs::s_MassColor;
    if(DisplayOptions::isLightTheme()) massclr = massclr.darker();
    else                               massclr = massclr.lighter();

    double delta = 0.02/m_glScalef;
    if(qAbs(volumeMass)>PRECISION)
    {
        glRenderText(pos.x, pos.y, pos.z + delta,
                     tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::massUnitLabel()+")",
                     massclr);
    }

    for(int im=0; im<ptMasses.size(); im++)
    {
        paintSphere(ptMasses[im]->position() +pos,
                    W3dPrefs::s_MassRadius/m_glScalef,
                    massclr,
                    true);
        glRenderText(ptMasses[im]->position().x + pos.x,
                     ptMasses[im]->position().y + pos.y,
                     ptMasses[im]->position().z + pos.z + delta,
                     ptMasses[im]->tag()+QString(" (%1").arg(ptMasses[im]->mass()*Units::kgtoUnit(), 0,'g',3)+Units::massUnitLabel()+")",
                     massclr);
    }
}


void gl3dXflView::paintMasses(double volumeMass, const Vector3d &pos, const QString &tag, const QVector<PointMass> &ptMasses)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QColor massclr = W3dPrefs::s_MassColor;
    if(DisplayOptions::isLightTheme()) massclr = massclr.darker();
    else                               massclr = massclr.lighter();

    double delta = 0.02/m_glScalef;
    if(qAbs(volumeMass)>PRECISION)
    {
        glRenderText(pos.x, pos.y, pos.z + delta,
                     tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::massUnitLabel()+")",
                     massclr);
    }

    for(int im=0; im<ptMasses.size(); im++)
    {
        PointMass const &pm = ptMasses.at(im);
        paintSphere(pm.position() +pos,
                    W3dPrefs::s_MassRadius/m_glScalef,
                    massclr,
                    true);
        glRenderText(pm.position().x + pos.x,
                     pm.position().y + pos.y,
                     pm.position().z + pos.z + delta,
                     pm.tag()+QString(" (%1").arg(pm.mass()*Units::kgtoUnit(), 0,'g',3)+Units::massUnitLabel()+")",
                     massclr);
    }
}


/**
 * Draws the point masses, the object masses, and the CG position in the OpenGL viewport
*/
void gl3dXflView::paintMasses(Plane const *pPlane)
{
    if(!pPlane) return;
    double delta = 0.02/m_glScalef;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pPlane->wingAt(iw))
        {
            paintMasses(pPlane->wingAt(iw)->m_VolumeMass, pPlane->wingLE(iw),
                        pPlane->wingAt(iw)->m_Name,   pPlane->wingAt(iw)->m_PointMass);
        }
    }

    paintMasses(0.0, Vector3d(0.0,0.0,0.0),"", pPlane->m_PointMass);


    if(pPlane->body())
    {
        Body const *pCurBody = pPlane->body();

        paintMasses(pCurBody->m_VolumeMass,
                    pPlane->bodyPos(),
                    pCurBody->m_Name,
                    pCurBody->m_PointMass);
    }

    QColor massclr = W3dPrefs::s_MassColor;
    if(DisplayOptions::isLightTheme()) massclr = massclr.darker();
    else                               massclr = massclr.lighter();

    //plot CG
    Vector3d Place(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z);
    paintSphere(Place, W3dPrefs::s_MassRadius*2.0/m_glScalef, massclr);

    glRenderText(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z + delta,
                 "CoG "+QString("%1").arg(pPlane->totalMass()*Units::kgtoUnit(), 7,'g',3)
                 +Units::massUnitLabel(), massclr);
}


/** Default mesh, if no polar has been defined */
void gl3dXflView::glMakeWingEditMesh(QOpenGLBuffer &vbo, Wing const *pWing)
{    //not necessarily the same Nx for all surfaces, so we need to count the quad panels
    int bufferSize = 0;
    for (int j=0; j<pWing->surfaceCount(); j++)
    {
        Surface const &surf = pWing->m_Surface.at(j);
        //tip patches
        if(surf.isTipLeft())  bufferSize += (surf.nXPanels());
        if(surf.isTipRight()) bufferSize += (surf.nXPanels());

        // top and bottom surfaces
        bufferSize += surf.nXPanels()*2 * (surf.nYPanels());
    }
    bufferSize *=2;    // 2 triangles/quad
    bufferSize *=3;    // 3 vertex for each triangle
    bufferSize *=3;    // 3 components for each node

    QVector<float> meshVertexArray(bufferSize);

    int iv=0;

    Vector3d A,B,C,D;

    //tip patches
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const &surf = pWing->m_Surface.at(j);
        if(surf.isTipLeft())
        {
            for (int l=0; l<surf.nXPanels(); l++)
            {
                surf.getPanel(0,l,xfl::TOPSURFACE);
                A = surf.TA;
                B = surf.LA;
                surf.getPanel(0,l,xfl::BOTSURFACE);
                C = surf.LA;
                D = surf.TA;

                //first triangle
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
                meshVertexArray[iv++] = B.xf();
                meshVertexArray[iv++] = B.yf();
                meshVertexArray[iv++] = B.zf();
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();

                //second triangle
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
                meshVertexArray[iv++] = D.xf();
                meshVertexArray[iv++] = D.yf();
                meshVertexArray[iv++] = D.zf();
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
            }
        }
        if(surf.isTipRight())
        {
            for (int l=0; l<surf.nXPanels(); l++)
            {
                surf.getPanel(surf.nYPanels()-1,l,xfl::TOPSURFACE);
                A = surf.TB;
                B = surf.LB;
                surf.getPanel(surf.nYPanels()-1,l,xfl::BOTSURFACE);
                C = surf.LB;
                D = surf.TB;

                //first triangle
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
                meshVertexArray[iv++] = B.xf();
                meshVertexArray[iv++] = B.yf();
                meshVertexArray[iv++] = B.zf();
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();

                //second triangle
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
                meshVertexArray[iv++] = D.xf();
                meshVertexArray[iv++] = D.yf();
                meshVertexArray[iv++] = D.zf();
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
            }
        }
    }

    //background surface
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const &surf = pWing->m_Surface.at(j);
        for(int k=0; k<surf.nYPanels(); k++)
        {
            for (int l=0; l<surf.nXPanels(); l++)
            {
                surf.getPanel(k,l,xfl::TOPSURFACE);

                // first triangle
                meshVertexArray[iv++] = surf.LA.xf();
                meshVertexArray[iv++] = surf.LA.yf();
                meshVertexArray[iv++] = surf.LA.zf();
                meshVertexArray[iv++] = surf.TA.xf();
                meshVertexArray[iv++] = surf.TA.yf();
                meshVertexArray[iv++] = surf.TA.zf();
                meshVertexArray[iv++] = surf.TB.xf();
                meshVertexArray[iv++] = surf.TB.yf();
                meshVertexArray[iv++] = surf.TB.zf();

                //second triangle
                meshVertexArray[iv++] = surf.TB.xf();
                meshVertexArray[iv++] = surf.TB.yf();
                meshVertexArray[iv++] = surf.TB.zf();
                meshVertexArray[iv++] = surf.LB.xf();
                meshVertexArray[iv++] = surf.LB.yf();
                meshVertexArray[iv++] = surf.LB.zf();
                meshVertexArray[iv++] = surf.LA.xf();
                meshVertexArray[iv++] = surf.LA.yf();
                meshVertexArray[iv++] = surf.LA.zf();
            }

            for (int l=0; l<surf.nXPanels(); l++)
            {
                surf.getPanel(k,l,xfl::BOTSURFACE);
                //first triangle
                meshVertexArray[iv++] = surf.TB.xf();
                meshVertexArray[iv++] = surf.TB.yf();
                meshVertexArray[iv++] = surf.TB.zf();
                meshVertexArray[iv++] = surf.TA.xf();
                meshVertexArray[iv++] = surf.TA.yf();
                meshVertexArray[iv++] = surf.TA.zf();
                meshVertexArray[iv++] = surf.LA.xf();
                meshVertexArray[iv++] = surf.LA.yf();
                meshVertexArray[iv++] = surf.LA.zf();

                //second triangle
                meshVertexArray[iv++] = surf.LA.xf();
                meshVertexArray[iv++] = surf.LA.yf();
                meshVertexArray[iv++] = surf.LA.zf();
                meshVertexArray[iv++] = surf.LB.xf();
                meshVertexArray[iv++] = surf.LB.yf();
                meshVertexArray[iv++] = surf.LB.zf();
                meshVertexArray[iv++] = surf.TB.xf();
                meshVertexArray[iv++] = surf.TB.yf();
                meshVertexArray[iv++] = surf.TB.zf();
            }
        }
    }


    Q_ASSERT(iv==bufferSize);


    Q_ASSERT(iv==bufferSize);

    //    m_iWingMeshElems = ii/3;
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dXflView::glMakeBodyFrameHighlight(const Body *pBody, const Vector3d &bodyPos, int iFrame)
{
    //    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefs::bodyHoopRes();

    Vector3d Point;
    if(iFrame<0) return;

    Frame const*pFrame = pBody->frameAt(iFrame);
    //    xinc = 0.1;
    double hinc = 1.0/double(NHOOOP-1);

    int bufferSize = 0;
    QVector<float>pHighlightVertexArray;

    m_nHighlightLines = 2; // left and right - could make one instead

    //create 3D Splines or Lines to overlay on the body
    int iv = 0;

    if(pBody->isFlatPanelType())
    {
        m_HighlightLineSize = pFrame->pointCount();
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
        pHighlightVertexArray.resize(bufferSize);
        for (int k=0; k<pFrame->pointCount();k++)
        {
            pHighlightVertexArray[iv++] = pFrame->m_Position.xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].yf();
            pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].zf()+bodyPos.zf();
        }

        for (int k=0; k<pFrame->pointCount();k++)
        {
            pHighlightVertexArray[iv++] =  pFrame->m_Position.xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = -pFrame->m_CtrlPoint[k].yf();
            pHighlightVertexArray[iv++] =  pFrame->m_CtrlPoint[k].zf()+bodyPos.zf();
        }
    }
    else if(pBody->isSplineType())
    {
        m_HighlightLineSize = NHOOOP;
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
        pHighlightVertexArray.resize(bufferSize);

        if(pBody->activeFrame())
        {
            double u = pBody->getu(pFrame->m_Position.x);
            double v = 0.0;
            for (int k=0; k<NHOOOP; k++)
            {
                pBody->getPoint(u,v,true, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v += hinc;
            }

            v = 1.0;
            for (int k=0; k<NHOOOP; k++)
            {
                pBody->getPoint(u,v,false, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v -= hinc;
            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    m_vboHighlight.destroy();
    m_vboHighlight.create();
    m_vboHighlight.bind();
    m_vboHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboHighlight.release();
}



/** Default mesh, if no polar has been defined */
void gl3dXflView::glMakeEditBodyMesh(Body *pBody, Vector3d const&pos)
{
    if(!pBody) return;
    QVector<Panel> panels;
    QVector<Vector3d> nodes;
    pBody->makePanels(0, pos, panels, nodes);
}


void gl3dXflView::paintNormals(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(135,105,35));

        if(m_bUse120StyleShaders) glLineWidth(2);
        else m_shadLine.setUniformValue(m_locLine.m_Thickness, 1);

        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));


            int nNormals = vbo.size()/2/3/int(sizeof(float)); //  (two vertices) x (x,y,z) = 6

            //    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_LINES, 0, nNormals*2);

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        vbo.release();
    }
    m_shadLine.release();
}


void gl3dXflView::paintMesh(QOpenGLBuffer &vbo, bool bBackGround)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_VLMStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_VLMStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_VLMStyle.m_Width);

        vbo.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));

            int nPanels = vbo.size()/3/6/int(sizeof(float)); // three vertices and 6 components

            glLineWidth(W3dPrefs::s_VLMStyle.m_Width);
            glEnable(GL_LINE_STIPPLE);
            GLLineStipple(W3dPrefs::s_VLMStyle.m_Stipple);
            int pos = 0;
            for(int p=0; p<nPanels*2; p++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, 3);
                pos +=3 ;
            }
            glDisable (GL_LINE_STIPPLE);

/*            m_shadLine.setUniformValue(m_locLine.m_UniColor, DisplayOptions::backgroundColor());

            if(bBackGround)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
                glDrawArrays(GL_TRIANGLES, 0, nPanels*3);
                glDisable(GL_POLYGON_OFFSET_FILL);

            }
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);*/
        }
        vbo.release();
    }
    m_shadLine.release();

    if(bBackGround)
    {
        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_UniColor, DisplayOptions::backgroundColor());
            m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
            m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);

            vbo.bind();
            {
                m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));

                int nPanels = vbo.size()/3/6/int(sizeof(float)); // three vertices and 6 components
                glDisable(GL_CULL_FACE);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
                glDrawArrays(GL_TRIANGLES, 0, nPanels*3);
                glDisable(GL_POLYGON_OFFSET_FILL);


                m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
            }
            vbo.release();
        }
        m_shadSurf.release();
    }
}


void gl3dXflView::glMakeWingOutline(Wing const *pWing, Body const *pBody, QOpenGLBuffer &vboOutline) const
{
    int CHORDPOINTS = W3dPrefs::chordwiseRes();
    QVector<Vector3d>NA(CHORDPOINTS), NB(CHORDPOINTS);
    QVector<Vector3d>PtBotLeft(CHORDPOINTS), PtBotRight(CHORDPOINTS);
    QVector<Vector3d>PtTopLeft(CHORDPOINTS), PtTopRight(CHORDPOINTS);

    Vector3d Pt, N;

    int stride = 3;
    int nSegs = pWing->surfaceCount()
                *(CHORDPOINTS-1)
                *2                 // top and bottom
                *2;                // left and right
    nSegs += pWing->surfaceCount()
             *2;                     //LE and TE


    //TE flap outline
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->surface(j)->m_pFoilA;
        Foil const *pFoilB = pWing->surface(j)->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
            nSegs +=2;
        }
    }
    //LE flap outline
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->surface(j)->m_pFoilA;
        Foil const *pFoilB = pWing->surface(j)->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
        {
            nSegs +=2;
        }
    }


    int buffersize = nSegs
                *2                   // 2 vertices / segment
                *stride;
    QVector<float>OutlineVA(buffersize);

    int iv=0; //index of outline vertex components


    //SURFACE
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const &surf = pWing->m_Surface.at(j);

        surf.getSidePoints(xfl::TOPSURFACE, pBody, PtTopLeft, PtTopRight, NA, NB, CHORDPOINTS);
        surf.getSidePoints(xfl::BOTSURFACE, pBody, PtBotLeft, PtBotRight, NA, NB, CHORDPOINTS);

        //OUTLINE
        for(int l=0; l<CHORDPOINTS-1; l++)
        {
            OutlineVA[iv++] = PtBotLeft.at(l).xf();
            OutlineVA[iv++] = PtBotLeft.at(l).yf();
            OutlineVA[iv++] = PtBotLeft.at(l).zf();
            OutlineVA[iv++] = PtBotLeft.at(l+1).xf();
            OutlineVA[iv++] = PtBotLeft.at(l+1).yf();
            OutlineVA[iv++] = PtBotLeft.at(l+1).zf();

            OutlineVA[iv++] = PtTopLeft.at(l).xf();
            OutlineVA[iv++] = PtTopLeft.at(l).yf();
            OutlineVA[iv++] = PtTopLeft.at(l).zf();
            OutlineVA[iv++] = PtTopLeft.at(l+1).xf();
            OutlineVA[iv++] = PtTopLeft.at(l+1).yf();
            OutlineVA[iv++] = PtTopLeft.at(l+1).zf();

            OutlineVA[iv++] = PtBotRight.at(l).xf();
            OutlineVA[iv++] = PtBotRight.at(l).yf();
            OutlineVA[iv++] = PtBotRight.at(l).zf();
            OutlineVA[iv++] = PtBotRight.at(l+1).xf();
            OutlineVA[iv++] = PtBotRight.at(l+1).yf();
            OutlineVA[iv++] = PtBotRight.at(l+1).zf();

            OutlineVA[iv++] = PtTopRight.at(l).xf();
            OutlineVA[iv++] = PtTopRight.at(l).yf();
            OutlineVA[iv++] = PtTopRight.at(l).zf();
            OutlineVA[iv++] = PtTopRight.at(l+1).xf();
            OutlineVA[iv++] = PtTopRight.at(l+1).yf();
            OutlineVA[iv++] = PtTopRight.at(l+1).zf();
        }
        //LE & TE
        surf.getSidePoint(0.0, false, xfl::TOPSURFACE, Pt, N);
        OutlineVA[iv++] = Pt.xf();
        OutlineVA[iv++] = Pt.yf();
        OutlineVA[iv++] = Pt.zf();
        surf.getSidePoint(0.0, true, xfl::TOPSURFACE, Pt, N);
        OutlineVA[iv++] = Pt.xf();
        OutlineVA[iv++] = Pt.yf();
        OutlineVA[iv++] = Pt.zf();

        surf.getSidePoint(1.0, false, xfl::TOPSURFACE, Pt, N);
        OutlineVA[iv++] = Pt.xf();
        OutlineVA[iv++] = Pt.yf();
        OutlineVA[iv++] = Pt.zf();
        surf.getSidePoint(1.0, true, xfl::TOPSURFACE, Pt, N);
        OutlineVA[iv++] = Pt.xf();
        OutlineVA[iv++] = Pt.yf();
        OutlineVA[iv++] = Pt.zf();


        Foil const *pFoilA = pWing->surface(j)->m_pFoilA;
        Foil const *pFoilB = pWing->surface(j)->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
            surf.getSurfacePoint(surf.m_pFoilA->m_TEXHinge/100.0,
                                   pFoilA->m_TEXHinge/100.0,
                                   0.0, xfl::TOPSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();

            surf.getSurfacePoint(surf.m_pFoilB->m_TEXHinge/100.0,
                                   pFoilB->m_TEXHinge/100.0,
                                   1.0, xfl::TOPSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();


            surf.getSurfacePoint(surf.m_pFoilA->m_TEXHinge/100.0,
                                   pFoilA->m_TEXHinge/100.0,
                                   0.0, xfl::BOTSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();


            surf.getSurfacePoint(surf.m_pFoilB->m_TEXHinge/100.0,
                                   pFoilB->m_TEXHinge/100.0,
                                   1.0, xfl::BOTSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();
        }
        if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
        {
            surf.getSurfacePoint(surf.m_pFoilA->m_LEXHinge/100.0,
                                   pFoilA->m_TEXHinge/100.0,
                                   0.0, xfl::TOPSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();

            surf.getSurfacePoint(surf.m_pFoilB->m_LEXHinge/100.0,
                                   pFoilB->m_TEXHinge/100.0,
                                   1.0, xfl::TOPSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();


            surf.getSurfacePoint(surf.m_pFoilA->m_LEXHinge/100.0,
                                   pFoilA->m_TEXHinge/100.0,
                                   0.0, xfl::BOTSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();


            surf.getSurfacePoint(surf.m_pFoilB->m_LEXHinge/100.0,
                                   pFoilB->m_TEXHinge/100.0,
                                   1.0, xfl::BOTSURFACE, Pt, N);
            OutlineVA[iv++] = Pt.xf();
            OutlineVA[iv++] = Pt.yf();
            OutlineVA[iv++] = Pt.zf();
        }
    }
    Q_ASSERT(iv==buffersize);


    vboOutline.destroy();
    vboOutline.create();
    vboOutline.bind();
    vboOutline.allocate(OutlineVA.data(), OutlineVA.size() * int(sizeof(GLfloat)));
    vboOutline.release();
}


void gl3dXflView::glMakeWingSurface(Wing const *pWing, Body const *pBody, QOpenGLBuffer &vboSurf) const
{
    int CHORDPOINTS = W3dPrefs::chordwiseRes();

    Vector3d N, Pt;
    QVector<Vector3d>NormalA(CHORDPOINTS);
    QVector<Vector3d>NormalB(CHORDPOINTS);
    QVector<Vector3d>PtBotLeft( CHORDPOINTS);
    QVector<Vector3d>PtBotRight(CHORDPOINTS);
    QVector<Vector3d>PtTopLeft( CHORDPOINTS);
    QVector<Vector3d>PtTopRight( CHORDPOINTS);

    QVector<double>leftV(CHORDPOINTS);
    QVector<double>rightV(CHORDPOINTS);

    double leftU(0), rightU(1);

    int stride = 8; //3 vertex components, 3 normal components, 2 texture components
    int nTriangles = pWing->m_Surface.count()
            *2                          // top and bottom
            *(CHORDPOINTS-1)            // quad count
            *2;                         // 2 triangles/quad
    //tip patches
    nTriangles += 2* (CHORDPOINTS-1) * 2; // four triangles are null, but who cares? Not OpenGL
    int buffersize = nTriangles
                    *3                          // 3 vertices /triangle
                    *stride;
    QVector<float>SurfaceVA(buffersize);


    N.set(0.0, 0.0, 0.0);
    int ivs=0; //index of surface vertex components

    //SURFACE
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const &surf = pWing->m_Surface.at(j);

        //top surface
        surf.getSidePoints(xfl::TOPSURFACE, pBody, PtTopLeft, PtTopRight, NormalA, NormalB, CHORDPOINTS);
        pWing->getTextureUV(j, leftV.data(), rightV.data(), leftU, rightU, CHORDPOINTS);

        for(int l=0; l<CHORDPOINTS-1; l++)
        {
            // first triangle
            SurfaceVA[ivs++] = PtTopLeft.at(l).xf();
            SurfaceVA[ivs++] = PtTopLeft.at(l).yf();
            SurfaceVA[ivs++] = PtTopLeft.at(l).zf();
            SurfaceVA[ivs++] = NormalA.at(l).xf();
            SurfaceVA[ivs++] = NormalA.at(l).yf();
            SurfaceVA[ivs++] = NormalA.at(l).zf();
            SurfaceVA[ivs++] = leftU;
            SurfaceVA[ivs++] = leftV.at(l);

            SurfaceVA[ivs++] = PtTopLeft.at(l+1).xf();
            SurfaceVA[ivs++] = PtTopLeft.at(l+1).yf();
            SurfaceVA[ivs++] = PtTopLeft.at(l+1).zf();
            SurfaceVA[ivs++] = NormalA.at(l+1).xf();
            SurfaceVA[ivs++] = NormalA.at(l+1).yf();
            SurfaceVA[ivs++] = NormalA.at(l+1).zf();
            SurfaceVA[ivs++] = leftU;
            SurfaceVA[ivs++] = leftV.at(l+1);

            SurfaceVA[ivs++] = PtTopRight.at(l).xf();
            SurfaceVA[ivs++] = PtTopRight.at(l).yf();
            SurfaceVA[ivs++] = PtTopRight.at(l).zf();
            SurfaceVA[ivs++] = NormalB.at(l).xf();
            SurfaceVA[ivs++] = NormalA.at(l).yf();
            SurfaceVA[ivs++] = NormalB.at(l).zf();
            SurfaceVA[ivs++] = rightU;
            SurfaceVA[ivs++] = rightV.at(l);

            // second triangle
            SurfaceVA[ivs++] = PtTopLeft.at(l+1).xf();
            SurfaceVA[ivs++] = PtTopLeft.at(l+1).yf();
            SurfaceVA[ivs++] = PtTopLeft.at(l+1).zf();
            SurfaceVA[ivs++] = NormalA.at(l+1).xf();
            SurfaceVA[ivs++] = NormalA.at(l+1).yf();
            SurfaceVA[ivs++] = NormalA.at(l+1).zf();
            SurfaceVA[ivs++] = leftU;
            SurfaceVA[ivs++] = leftV.at(l+1);

            SurfaceVA[ivs++] = PtTopRight.at(l+1).xf();
            SurfaceVA[ivs++] = PtTopRight.at(l+1).yf();
            SurfaceVA[ivs++] = PtTopRight.at(l+1).zf();
            SurfaceVA[ivs++] = NormalB.at(l+1).xf();
            SurfaceVA[ivs++] = NormalB.at(l+1).yf();
            SurfaceVA[ivs++] = NormalB.at(l+1).zf();
            SurfaceVA[ivs++] = rightU;
            SurfaceVA[ivs++] = rightV.at(l+1);

            SurfaceVA[ivs++] = PtTopRight.at(l).xf();
            SurfaceVA[ivs++] = PtTopRight.at(l).yf();
            SurfaceVA[ivs++] = PtTopRight.at(l).zf();
            SurfaceVA[ivs++] = NormalB.at(l).xf();
            SurfaceVA[ivs++] = NormalA.at(l).yf();
            SurfaceVA[ivs++] = NormalB.at(l).zf();
            SurfaceVA[ivs++] = rightU;
            SurfaceVA[ivs++] = rightV.at(l);
        }

        //bottom surface
        surf.getSidePoints(xfl::BOTSURFACE, pBody, PtBotLeft, PtBotRight, NormalA, NormalB, CHORDPOINTS);
        pWing->getTextureUV(j, leftV.data(), rightV.data(), leftU, rightU, CHORDPOINTS);

        for(int l=0; l<CHORDPOINTS-1; l++)
        {
            // first triangle
            SurfaceVA[ivs++] = PtBotLeft.at(l).xf();
            SurfaceVA[ivs++] = PtBotLeft.at(l).yf();
            SurfaceVA[ivs++] = PtBotLeft.at(l).zf();
            SurfaceVA[ivs++] = NormalA.at(l).xf();
            SurfaceVA[ivs++] = NormalA.at(l).yf();
            SurfaceVA[ivs++] = NormalA.at(l).zf();
            SurfaceVA[ivs++] = leftU;
            SurfaceVA[ivs++] = leftV.at(l);

            SurfaceVA[ivs++] = PtBotRight.at(l).xf();
            SurfaceVA[ivs++] = PtBotRight.at(l).yf();
            SurfaceVA[ivs++] = PtBotRight.at(l).zf();
            SurfaceVA[ivs++] = NormalB.at(l).xf();
            SurfaceVA[ivs++] = NormalA.at(l).yf();
            SurfaceVA[ivs++] = NormalB.at(l).zf();
            SurfaceVA[ivs++] = rightU;
            SurfaceVA[ivs++] = rightV.at(l);

            SurfaceVA[ivs++] = PtBotLeft.at(l+1).xf();
            SurfaceVA[ivs++] = PtBotLeft.at(l+1).yf();
            SurfaceVA[ivs++] = PtBotLeft.at(l+1).zf();
            SurfaceVA[ivs++] = NormalA.at(l+1).xf();
            SurfaceVA[ivs++] = NormalA.at(l+1).yf();
            SurfaceVA[ivs++] = NormalA.at(l+1).zf();
            SurfaceVA[ivs++] = leftU;
            SurfaceVA[ivs++] = leftV.at(l+1);

            // second triangle
            SurfaceVA[ivs++] = PtBotLeft.at(l+1).xf();
            SurfaceVA[ivs++] = PtBotLeft.at(l+1).yf();
            SurfaceVA[ivs++] = PtBotLeft.at(l+1).zf();
            SurfaceVA[ivs++] = NormalA.at(l+1).xf();
            SurfaceVA[ivs++] = NormalA.at(l+1).yf();
            SurfaceVA[ivs++] = NormalA.at(l+1).zf();
            SurfaceVA[ivs++] = leftU;
            SurfaceVA[ivs++] = leftV.at(l+1);

            SurfaceVA[ivs++] = PtBotRight.at(l).xf();
            SurfaceVA[ivs++] = PtBotRight.at(l).yf();
            SurfaceVA[ivs++] = PtBotRight.at(l).zf();
            SurfaceVA[ivs++] = NormalB.at(l).xf();
            SurfaceVA[ivs++] = NormalA.at(l).yf();
            SurfaceVA[ivs++] = NormalB.at(l).zf();
            SurfaceVA[ivs++] = rightU;
            SurfaceVA[ivs++] = rightV.at(l);

            SurfaceVA[ivs++] = PtBotRight.at(l+1).xf();
            SurfaceVA[ivs++] = PtBotRight.at(l+1).yf();
            SurfaceVA[ivs++] = PtBotRight.at(l+1).zf();
            SurfaceVA[ivs++] = NormalB.at(l+1).xf();
            SurfaceVA[ivs++] = NormalB.at(l+1).yf();
            SurfaceVA[ivs++] = NormalB.at(l+1).zf();
            SurfaceVA[ivs++] = rightU;
            SurfaceVA[ivs++] = rightV.at(l+1);
        }

        if(surf.isTipLeft())
        {
            for(int l=0; l<CHORDPOINTS-1; l++)
            {
                // first triangle
                SurfaceVA[ivs++] = PtBotLeft.at(l).xf();
                SurfaceVA[ivs++] = PtBotLeft.at(l).yf();
                SurfaceVA[ivs++] = PtBotLeft.at(l).zf();
                SurfaceVA[ivs++] = NormalA.at(l).xf();
                SurfaceVA[ivs++] = NormalA.at(l).yf();
                SurfaceVA[ivs++] = NormalA.at(l).zf();
                SurfaceVA[ivs++] = leftU;
                SurfaceVA[ivs++] = leftV.at(l);

                SurfaceVA[ivs++] = PtBotLeft.at(l+1).xf();
                SurfaceVA[ivs++] = PtBotLeft.at(l+1).yf();
                SurfaceVA[ivs++] = PtBotLeft.at(l+1).zf();
                SurfaceVA[ivs++] = NormalA.at(l).xf();
                SurfaceVA[ivs++] = NormalA.at(l).yf();
                SurfaceVA[ivs++] = NormalA.at(l).zf();
                SurfaceVA[ivs++] = leftU;
                SurfaceVA[ivs++] = leftV.at(l);

                SurfaceVA[ivs++] = PtTopLeft.at(l).xf();
                SurfaceVA[ivs++] = PtTopLeft.at(l).yf();
                SurfaceVA[ivs++] = PtTopLeft.at(l).zf();
                SurfaceVA[ivs++] = -NormalA.at(l).xf();
                SurfaceVA[ivs++] = -NormalA.at(l).yf();
                SurfaceVA[ivs++] = -NormalA.at(l).zf();
                SurfaceVA[ivs++] = leftU;
                SurfaceVA[ivs++] = leftV.at(l);

                // second triangle
                SurfaceVA[ivs++] = PtTopLeft.at(l).xf();
                SurfaceVA[ivs++] = PtTopLeft.at(l).yf();
                SurfaceVA[ivs++] = PtTopLeft.at(l).zf();
                SurfaceVA[ivs++] = -NormalA.at(l).xf();
                SurfaceVA[ivs++] = -NormalA.at(l).yf();
                SurfaceVA[ivs++] = -NormalA.at(l).zf();
                SurfaceVA[ivs++] = leftU;
                SurfaceVA[ivs++] = leftV.at(l);

                SurfaceVA[ivs++] = PtBotLeft.at(l+1).xf();
                SurfaceVA[ivs++] = PtBotLeft.at(l+1).yf();
                SurfaceVA[ivs++] = PtBotLeft.at(l+1).zf();
                SurfaceVA[ivs++] = NormalA.at(l).xf();
                SurfaceVA[ivs++] = NormalA.at(l).yf();
                SurfaceVA[ivs++] = NormalA.at(l).zf();
                SurfaceVA[ivs++] = leftU;
                SurfaceVA[ivs++] = leftV.at(l);

                SurfaceVA[ivs++] = PtTopLeft.at(l+1).xf();
                SurfaceVA[ivs++] = PtTopLeft.at(l+1).yf();
                SurfaceVA[ivs++] = PtTopLeft.at(l+1).zf();
                SurfaceVA[ivs++] = -NormalA.at(l).xf();
                SurfaceVA[ivs++] = -NormalA.at(l).yf();
                SurfaceVA[ivs++] = -NormalA.at(l).zf();
                SurfaceVA[ivs++] = leftU;
                SurfaceVA[ivs++] = leftV.at(l);
            }
        }

        if(surf.isTipRight())
        {
            for(int l=0; l<CHORDPOINTS-1; l++)
            {
                // first triangle
                SurfaceVA[ivs++] = PtBotRight.at(l).xf();
                SurfaceVA[ivs++] = PtBotRight.at(l).yf();
                SurfaceVA[ivs++] = PtBotRight.at(l).zf();
                SurfaceVA[ivs++] = NormalB.at(l).xf();
                SurfaceVA[ivs++] = NormalB.at(l).yf();
                SurfaceVA[ivs++] = NormalB.at(l).zf();
                SurfaceVA[ivs++] = rightU;
                SurfaceVA[ivs++] = rightV.at(l);

                SurfaceVA[ivs++] = PtTopRight.at(l).xf();
                SurfaceVA[ivs++] = PtTopRight.at(l).yf();
                SurfaceVA[ivs++] = PtTopRight.at(l).zf();
                SurfaceVA[ivs++] = -NormalB.at(l).xf();
                SurfaceVA[ivs++] = -NormalB.at(l).yf();
                SurfaceVA[ivs++] = -NormalB.at(l).zf();
                SurfaceVA[ivs++] = rightU;
                SurfaceVA[ivs++] = rightV.at(l);

                SurfaceVA[ivs++] = PtBotRight.at(l+1).xf();
                SurfaceVA[ivs++] = PtBotRight.at(l+1).yf();
                SurfaceVA[ivs++] = PtBotRight.at(l+1).zf();
                SurfaceVA[ivs++] = NormalB.at(l).xf();
                SurfaceVA[ivs++] = NormalB.at(l).yf();
                SurfaceVA[ivs++] = NormalB.at(l).zf();
                SurfaceVA[ivs++] = rightU;
                SurfaceVA[ivs++] = rightV.at(l);

                // second triangle
                SurfaceVA[ivs++] = PtTopRight.at(l).xf();
                SurfaceVA[ivs++] = PtTopRight.at(l).yf();
                SurfaceVA[ivs++] = PtTopRight.at(l).zf();
                SurfaceVA[ivs++] = -NormalB.at(l).xf();
                SurfaceVA[ivs++] = -NormalB.at(l).yf();
                SurfaceVA[ivs++] = -NormalB.at(l).zf();
                SurfaceVA[ivs++] = rightU;
                SurfaceVA[ivs++] = rightV.at(l);

                SurfaceVA[ivs++] = PtTopRight.at(l+1).xf();
                SurfaceVA[ivs++] = PtTopRight.at(l+1).yf();
                SurfaceVA[ivs++] = PtTopRight.at(l+1).zf();
                SurfaceVA[ivs++] = -NormalB.at(l).xf();
                SurfaceVA[ivs++] = -NormalB.at(l).yf();
                SurfaceVA[ivs++] = -NormalB.at(l).zf();
                SurfaceVA[ivs++] = rightU;
                SurfaceVA[ivs++] = rightV.at(l);

                SurfaceVA[ivs++] = PtBotRight.at(l+1).xf();
                SurfaceVA[ivs++] = PtBotRight.at(l+1).yf();
                SurfaceVA[ivs++] = PtBotRight.at(l+1).zf();
                SurfaceVA[ivs++] = NormalB.at(l).xf();
                SurfaceVA[ivs++] = NormalB.at(l).yf();
                SurfaceVA[ivs++] = NormalB.at(l).zf();
                SurfaceVA[ivs++] = rightU;
                SurfaceVA[ivs++] = rightV.at(l);
            }
        }

    }
    Q_ASSERT(ivs==buffersize);

    vboSurf.destroy();
    vboSurf.create();
    vboSurf.bind();
    vboSurf.allocate(SurfaceVA.data(), SurfaceVA.size() * int(sizeof(GLfloat)));
    vboSurf.release();
}


