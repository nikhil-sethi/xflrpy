/****************************************************************************

    gl3dView Class
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

#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTimer>
#include <QElapsedTimer>

#include <xflcore/fontstruct.h>
#include <xflcore/core_enums.h>
#include <xflcore/linestyle.h>
#include <xfl3d/controls/arcball.h>
#include <xfl3d/controls/light.h>


#define PIf 3.141592654f


#define DEPTHFACTOR 0.0f
#define DEPTHUNITS 1.0f

#define MAXCPCOLORS    21

/** generic shader locations; not all locations are necessary nor defined for each shader */
struct ShaderLocations
{
    // Attribute data
    int m_attrVertex{-1}, m_attrNormal{-1};
    int m_attrColor{-1};
    int m_attrm_UV{-1};
    int m_attrUV{-1}; // vertex attribute array containing the texture's UV coordinates
    int m_attrOffset{-1};

    // Uniforms
    int m_vmMatrix{-1}, m_pvmMatrix{-1};
    int m_Scale{-1}; // only used if instancing is enabled

    int m_Light{-1};
    int m_UniColor{-1};
    int m_ClipPlane{-1};

    int m_TwoSided{-1};

    int m_HasUniColor{-1};
    int m_HasTexture{-1};    // uniform defining whether a texture is enabled or not
    int m_IsInstanced{-1};

    int m_Pattern{-1}, m_nPatterns{-1};
    int m_Thickness{-1}, m_Viewport{-1};

    int m_State{-1};
    int m_Shape{-1};

    int m_TexSampler{-1}; // the id of the sampler; defaults to 0
};



class GLLightDlg;


class gl3dView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    friend class GLLightDlg;

    Q_OBJECT
    public:
        gl3dView(QWidget *pParent = nullptr);
        ~gl3dView();

        void setReferenceLength(double length) {m_RefLength = length;}
        double referenceLength() const {return m_RefLength;}
        void reset3dScale();

        void setLightVisible(bool bShow) {m_bLightVisible=bShow;}

        void getMemoryStatus(int &total_mem_kb, int &cur_avail_mem_kb);

        static void setFontStruct(FontStruct const & fntstruct) {s_glFontStruct=fntstruct;}
        static void setTextColor(QColor const &textclr) {s_TextColor=textclr;}
        static void setBackColor(QColor const &backclr) {s_BackgroundColor=backclr;}

        static QColor textColor() {return s_TextColor;}
        static QColor backColor() {return s_BackgroundColor;}

        static bool isLightOn() {return s_Light.m_bIsLightOn;}
        static void setLightOn(bool bLight) {s_Light.m_bIsLightOn = bLight;}
        static void setLight(Light const l) {s_Light=l;}
        static Light const &light() {return s_Light;}
        static void setLightPos(double x, double y, double z) {s_Light.m_X=x; s_Light.m_Y=y; s_Light.m_Z=z;}
        static void setSpecular(double s) {s_Light.m_Specular=s;}

        static bool bSpinAnimation() {return s_bSpinAnimation;}
        static double spinDamping() {return s_SpinDamping;}

        static void setMultiSample(bool bEnable) {s_bMultiSample=bEnable;}
        static bool bMultiSample() {return s_bMultiSample;}

        static void setXflSurfaceFormat(QSurfaceFormat const &fmt) {s_GlSurfaceFormat = fmt;}
        static QSurfaceFormat const& defaultXflSurfaceFormat() {return s_GlSurfaceFormat;}

        static void setOGLVersion(int OGLMajor, int OGLMinor) {s_GlSurfaceFormat.setVersion(OGLMajor, OGLMinor);}
        static int oglMajor() {return s_GlSurfaceFormat.majorVersion();}
        static int oglMinor() {return s_GlSurfaceFormat.minorVersion();}

        static void setDefaultSamples(int nSamples) {s_GlSurfaceFormat.setSamples(nSamples);}
        static int defaultSamples() {return s_GlSurfaceFormat.samples();}

        static void setProfile(QSurfaceFormat::OpenGLContextProfile prof) {s_GlSurfaceFormat.setProfile(prof);}
        static QSurfaceFormat::OpenGLContextProfile defaultProfile() {return s_GlSurfaceFormat.profile();}

        static void setDeprecatedFuncs(bool bDeprecated) {s_GlSurfaceFormat.setOption(QSurfaceFormat::DeprecatedFunctions, bDeprecated);}
        static bool deprecatedFuncs() {return s_GlSurfaceFormat.testOption(QSurfaceFormat::DeprecatedFunctions);}

        static void setDebugContext(bool bDebug) {s_GlSurfaceFormat.setOption(QSurfaceFormat::DebugContext, bDebug);}
        static bool debugContext() {return s_GlSurfaceFormat.testOption(QSurfaceFormat::DebugContext);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    signals:
        void viewModified();

    protected:
        //OVERLOADS
        void initializeGL() override;
        virtual void paintGL() override;
        virtual void resizeGL(int width, int height) override;
        virtual void glMake3dObjects() = 0;

        void keyReleaseEvent(QKeyEvent *pEvent) override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        void mouseDoubleClickEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(913, 701);}

    protected slots:
        void on3dBot();
        void on3dFlip();
        void on3dFront();
        void on3dIso();
        void on3dLeft();
        void on3dRear();
        void on3dRight();
        void on3dTop();
        virtual void on3dReset();
        void onAxes(bool bChecked);
        void onClipPlane(int pos);
        void onClipScreenPlane(bool bClip);

        void onDynamicIncrement();
        void onResetIncrement();
        void onRotationIncrement();
        void onTranslationIncrement();

    public:
        void setScale(double refLength);
        void glSetupLight();
        bool bUsing120StyleShaders() const {return m_bUse120StyleShaders;}

    protected:
        void getGLError();
        void glMakeArcPoint(ArcBall const&arcball);
        void glMakeArcBall(ArcBall &arcball);
        void glMakeLightSource();
        void glMakeIcoSphere(int nSplits);

        void glRenderText(Vector3d const &pos, const QString & str, const QColor &textcolor, bool bBackground=false, bool bBold=false) {glRenderText(pos.x, pos.y, pos.z, str, textcolor, bBackground, bBold);}
        void glRenderText(int x, int y, const QString & str, const QColor &backclr, const QColor &textcolor = QColor(Qt::white), bool bBold=false);
        void glRenderText(double x, double y, double z, const QString & str, const QColor &textcolor, bool bBackground=false, bool bBold=false);

        void glMakeAxes();

        void set3dRotationCenter(const QPoint &point);
        virtual bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) = 0;
        virtual void glRenderView() = 0;
        virtual void paintOverlay();

        void paintGL3();
        void paintArcBall();
        void paintAxes();
        void paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride);
        void paintSphere(float xs, float ys, float zs, float radius, const QColor &color, bool bLight=true);
        void paintSphere(const Vector3d &place, float radius, const QColor &sphereColor, bool bLight=true);
        void paintSphereInstances(QOpenGLBuffer &vboPosInstances, float radius, QColor const &clr, bool bTwoSided, bool bLight);
        void paintTriangle(QOpenGLBuffer &vbo, bool bHighlight, bool bBackground, const QColor &clrBack);
        void paintTriangles3Vtx(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight);
        void paintTriangles3VtxTexture(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight, QOpenGLTexture *pTexture);
        void paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh = false);
        void paintSegments(QOpenGLBuffer &vbo, const QColor &clr, int thickness, Line::enumLineStipple stip=Line::SOLID, bool bHigh=false);
        void paintLineStrip(QOpenGLBuffer &vbo, LineStyle const &ls);
        void paintLineStrip(QOpenGLBuffer &vbo, const QColor &clr, int width, Line::enumLineStipple stipple=Line::SOLID);
        void paintBox(double x, double y, double z, double dx, double dy, double dz, QColor const &clr, bool bLight);
        void paintCube(double x, double y, double z, double side, QColor const &clr, bool bLight);
        void paintColourSegments(QOpenGLBuffer &vbo, LineStyle const &ls);
        void paintColourMap(QOpenGLBuffer &vbo, QMatrix4x4 const& modelmat = QMatrix4x4());
        void paintTriangleFan(QOpenGLBuffer &vbo, QColor const &clr, bool bLight, bool bCullFaces);

        void reset3dRotationCenter();
        void startResetTimer();
        void startRotationTimer();
        void startTranslationTimer(Vector3d PP);

       void setViewportTranslation();
       void startDynamicTimer();
       void stopDynamicTimer();

       void updateLightMatrix();

       void screenToViewport(QPoint const &point, int z, Vector3d &real) const;
       void screenToViewport(QPoint const &point, Vector3d &real) const;
       void screenToWorld(const QPoint &screenpt, int z, Vector3d &modelpt) const;
       void viewportToScreen(Vector3d const &real, QPoint &point) const;
       void viewportToWorld(Vector3d vp, Vector3d &w) const;

       QVector4D worldToViewport(Vector3d v) const;
       QPoint worldToScreen(const Vector3d &v, QVector4D &vScreen) const;
       QPoint worldToScreen(const QVector4D &v4, QVector4D &vScreen) const;

       float scale() const {return m_glScalef;}

    public:
        void printFormat(QSurfaceFormat const &ctxFormat, QString &log);


    protected:

        QOpenGLShaderProgram m_shadLine;
        QOpenGLShaderProgram m_shadSurf;
        QOpenGLShaderProgram m_shadPoint;

        ShaderLocations m_locSurf;
        ShaderLocations m_locLine;
        ShaderLocations m_locPoint;

        int m_uHasShadow;
        int m_uShadowLightViewMatrix;

        QOpenGLBuffer m_vboArcBall, m_vboArcPoint;
        QOpenGLBuffer m_vboIcoSphere, m_vboIcoSphereEdges;
        QOpenGLBuffer m_vboAxes;
        QOpenGLBuffer m_vboLightSource;
        QOpenGLBuffer m_vboCube, m_vboCubeEdges;

        double m_RefLength;

        bool m_bLightVisible;

        bool m_bAxes;                      /**< true if the axes are to be displayed in the 3D view*/
        bool m_bArcball;            //true if the arcball is to be displayed
        bool m_bCrossPoint;            //true if the control point on the arcball is to be displayed
        ArcBall m_ArcBall;

        double m_glScalef, m_glScalefRef;

        QRectF m_GLViewRect;    /**< The OpenGl viewport.*/

        QTimer m_TransitionTimer;

        bool m_bTrans;
        bool m_bDragPoint;


        float m_ClipPlanePos;      /**< the z-position of the clip plane in viewport coordinates */
        double m_MatOut[16];

        bool m_bUse120StyleShaders;

        QMatrix4x4 m_matProj, m_matView, m_matModel;

        QPoint m_LastPoint, m_PressedPoint;

        Vector3d m_TransIncrement;
        double m_glScaleIncrement;

        Vector3d m_RealPopUp;
        Vector3d m_glViewportTrans;// the translation vector in gl viewport coordinates
        Vector3d m_glRotCenter;    // the center of rotation in object coordinates... is also the opposite of the translation vector

        QPixmap m_PixTextOverlay;

        int m_iTransitionInc;

        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/

        bool m_bHasMouseMoved;
        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer; // used when the user has set a dynamic rotation or translation using the mouse
        Quaternion m_SpinInc;
        Vector3d m_Trans;
        bool m_bDynTranslation;
        bool m_bDynRotation;
        bool m_bDynScaling;
        float m_ZoomFactor;
        int m_iTimerInc;
        Quaternion m_QuatStart, m_QuatEnd;

        int ANIMATIONFRAMES;

        QMatrix4x4 m_LightViewMatrix;


        static bool s_bSpinAnimation;
        static double s_SpinDamping;

        static int s_AnimationTime;
        static bool s_bAnimateTransitions;  // ms

        static QColor s_TextColor;
        static QColor s_BackgroundColor;

        static Light s_Light;
        static FontStruct s_glFontStruct;


        static bool s_bMultiSample;
        static QSurfaceFormat s_GlSurfaceFormat;
};

GLushort GLStipple(Line::enumLineStipple stipple);
void GLLineStipple(Line::enumLineStipple stipple);


