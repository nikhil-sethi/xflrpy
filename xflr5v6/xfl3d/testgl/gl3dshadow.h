/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QOpenGLTexture>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>

#define SHADOW_WIDTH 2048
#define SHADOW_HEIGHT 2048

class GLLightDlg;
class ExponentialSlider;
class gl3dShadow : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dShadow(QWidget *pParent=nullptr);
        ~gl3dShadow();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;

        void renderToShadowMap();
        void renderToScreen();

        void initDepthMap();
        void paintTrianglesToDepthMap(QOpenGLBuffer &vbo, const QMatrix4x4 &ModelMat, int stride);
        void paintTriangles3VtxShadow(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight, const QMatrix4x4 &modelmat, int stride);
        void paintTriangles3VtxTexture(QOpenGLBuffer &vbo, QOpenGLTexture *pTexture, bool bTwoSided, bool bLight);

    private slots:
        void onObjectPos();
        void onControls();
        void onLightSettings(bool bShow);

    private:
        ExponentialSlider *m_peslXObj, *m_peslYObj, *m_peslZObj;
        QCheckBox *m_pchLightDlg;

        GLLightDlg *m_pglLightDlg;

        QOpenGLBuffer m_vboSphere, m_vboSphereEdges;
        QOpenGLBuffer m_vboQuad;

        //shadow shader
        QOpenGLShaderProgram m_shadDepth;   /** the shader used to build the depth map */

        ShaderLocations m_locShadow;

        int m_uDepthLightViewMatrix;
        int m_attrDepthPos;
        uint m_fboDepthMap;
        uint m_texDepthMap;

        bool m_bResetObjects;
        Vector3d m_Object;

        QTimer m_CtrlTimer;
};

