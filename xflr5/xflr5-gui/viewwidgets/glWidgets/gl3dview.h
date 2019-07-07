/****************************************************************************

    gl3dView Class
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



#ifndef GL3DVIEW_H
#define GL3DVIEW_H



#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTimer>

#include <globals/gui_enums.h>
#include <viewwidgets/glWidgets/arcball.h>


#define PIf 3.141592654f

#define MAXCPCOLORS    21

class Miarex;
class MainFrame;
class Plane;
class Body;
class Wing;
class WingOpp;
class WPolar;
class PlaneOpp;
class Panel;
class GLLightDlg;
class PointMass;



class gl3dView : public QOpenGLWidget
{
    friend class MainFrame;
    friend class Miarex;
    friend class GL3dWingDlg;
    friend class GL3dBodyDlg;
    friend class EditBodyDlg;
    friend class EditPlaneDlg;
    friend class GLLightDlg;

    Q_OBJECT
public:
    gl3dView(QWidget *pParent = nullptr);
    ~gl3dView();

signals:
    void viewModified();

protected:
    //OVERLOADS
    void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
    void keyReleaseEvent(QKeyEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    void on3DIso();
    void on3DFlip();
    void on3DTop();
    void on3DLeft();
    void on3DFront();
    virtual void on3DReset() = 0;
    void onAxes(bool bChecked);
    void onClipPlane(int pos);
    void onSurfaces(bool bChecked);
    void onPanels(bool bChecked);
    void onOutline(bool bChecked);
    void onFoilNames(bool bChecked);
    void onShowMasses(bool bChecked);
    void onResetIncrement();
    void onRotationIncrement();
    void onTranslationIncrement();

public:
    void setScale(double refLength);
    void glSetupLight();
    bool bUsing120StyleShaders() const {return m_bUse120StyleShaders;}

protected:
    void getTextureFile(QString planeName, QString surfaceName, QImage & textureImage);
    void glDrawMasses(Plane *pPlane);
    void getGLError();
    void glInverseMatrix();
    void glMakeUnitSphere();
    void glMakeArcPoint();
    void glMakeArcBall();
    void glMakeBody3DFlatPanels(Body *pBody);
    void glMakeBodySplines(Body *pBody);
    void glMakeWingGeometry(int iWing, Wing *pWing, Body *pBody);
    void glMakeWingEditMesh(QOpenGLBuffer &vbo, Wing *pWing);
    void glMakeBodyFrameHighlight(Body *pBody, Vector3d bodyPos, int iFrame);
    void glMakeEditBodyMesh(Body *pBody, Vector3d BodyPosition);
    void glRenderText(int x, int y, const QString & str, QColor textColor = QColor(Qt::white));
    void glRenderText(double x, double y, double z, const QString & str, QColor textColor = QColor(Qt::white));
    void glMakeAxis();

    virtual void glRenderView() = 0;
    virtual void set3DRotationCenter(QPoint point) = 0;
    virtual void paintOverlay() {}

    void paintGL3();
    void paintFoilNames(Wing *pWingPtr);
    void paintMasses(double volumeMass, Vector3d pos, QString tag, const QVector<PointMass *> &ptMasses);
    void paintArcBall();
    void paintAxes();
    void paintWingMesh(Wing *pWing);
    void paintEditWingMesh(QOpenGLBuffer &vbo);
    void paintSectionHighlight();
    void paintBody(Body *pBody);
    void paintEditBodyMesh(Body *pBody);
    void paintWing(int iWing, Wing *pWing);
    void paintSphere(Vector3d place, double radius, QColor sphereColor, bool bLight=true);
    void printFormat(const QSurfaceFormat &format);
    void reset3DRotationCenter();
    void set3DScale(double length=-1.0);
    void setSpanStations(Plane *pPlane, WPolar *pWPolar, PlaneOpp *pPOpp);
    void startResetTimer(double length);
    void startRotationTimer();
    void startTranslationTimer(Vector3d PP);
    void screenToViewport(QPoint const &point, Vector3d &real);
    void viewportToScreen(Vector3d const &real, QPoint &point);
    void viewportToWorld(Vector3d vp, Vector3d &w);
    QVector4D worldToViewport(Vector3d v);
    QPoint worldToScreen(Vector3d v);
    QPoint worldToScreen(QVector4D v4);

    QOpenGLShaderProgram m_ShaderProgramLine,  m_ShaderProgramGradient;
    QOpenGLShaderProgram m_ShaderProgramSurface, m_ShaderProgramTexture;

    QOpenGLBuffer m_vboArcBall, m_vboArcPoint, m_vboBody;
    QOpenGLBuffer m_vboWingSurface[MAXWINGS], m_vboWingOutline[MAXWINGS];
    QOpenGLBuffer m_vboEditWingMesh[MAXWINGS];
    QOpenGLBuffer m_vboEditBodyMesh;
    QOpenGLBuffer m_vboHighlight;
    QOpenGLBuffer m_vboSphere;
    QOpenGLBuffer m_vboAxis;


    QOpenGLTexture     *m_pLeftBodyTexture, *m_pRightBodyTexture;
    QOpenGLTexture     *m_pWingTopLeftTexture[MAXWINGS], *m_pWingTopRightTexture[MAXWINGS], *m_pWingBotLeftTexture[MAXWINGS], *m_pWingBotRightTexture[MAXWINGS];

    int m_VertexLocationGradient, m_ColorLocationGradient;
    int m_pvmMatrixLocationGradient;

    int m_VertexLocationLine, m_ColorLocationLine;
    int m_pvmMatrixLocationLine, m_vMatrixLocationLine, m_mMatrixLocationLine;
    int m_ClipPlaneLocationLine;

    int m_VertexLocationSurface, m_NormalLocationSurface;
    int m_LightLocationSurface, m_SurfaceLocationSurface, m_ColorLocationSurface;

    int m_ClipPlaneLocationSurface;
    int m_vMatrixLocationSurface, m_mMatrixLocationSurface, m_pvmMatrixLocationSurface;
    int m_EyePosLocationSurface, m_LightPosLocationSurface;
    int m_LightColorLocationSurface, m_LightAmbientLocationSurface, m_LightDiffuseLocationSurface, m_LightSpecularLocationSurface;
    int m_AttenuationConstantSurface, m_AttenuationLinearSurface, m_AttenuationQuadraticSurface;
    int m_MaterialShininessSurface;


    int m_VertexLocationTexture, m_NormalLocationTexture, m_UVLocationTexture;
    int m_LightLocationTexture;
    int m_ClipPlaneLocationTexture;
    int m_vMatrixLocationTexture, m_mMatrixLocationTexture, m_pvmMatrixLocationTexture;
    int m_EyePosLocationTexture, m_LightPosLocationTexture;
    int m_LightColorLocationTexture, m_LightAmbientLocationTexture, m_LightDiffuseLocationTexture, m_LightSpecularLocationTexture;
    int m_AttenuationConstantTexture, m_AttenuationLinearTexture, m_AttenuationQuadraticTexture;
    int m_MaterialShininessTexture;


    bool m_bArcball;            //true if the arcball is to be displayed
    bool m_bCrossPoint;            //true if the control point on the arcball is to be displayed
    ArcBall m_ArcBall;
    unsigned short *m_SphereIndicesArray, *m_WingMeshIndicesArray;
    QVector<ushort> m_BodyIndicesArray;
    QVector<ushort> m_WingIndicesArray[MAXWINGS];

    double m_glScaled, m_glScaledRef;

    static Miarex *s_pMiarex;     /**< A void pointer to the instance of the QMiarex widget.*/
    static MainFrame *s_pMainFrame;  /**< A void pointer to the instance of the MainFrame widget.*/

    bool m_bOutline;                   /**< true if the surface outlines are to be displayed in the 3D view*/
    bool m_bSurfaces;                  /**< true if the surfaces are to be displayed in the 3D view*/
    bool m_bVLMPanels;                 /**< true if the panels are to be displayed in the 3D view*/
    bool m_bAxes;                      /**< true if the axes are to be displayed in the 3D view*/
    bool m_bShowMasses;                /**< true if the point masses are to be displayed on the openGL 3D view */
    bool m_bFoilNames;                 /**< true if the foil names are to be displayed on the openGL 3D view */


    static GLLightDlg* s_pglLightDlg;

    QRectF m_GLViewRect;    /**< The OpenGl viewport.*/

    QTimer *m_pTransitionTimer;

    bool m_bTrans;
    bool m_bDragPoint;


    float m_ClipPlanePos;      /**< the z-position of the clip plane in viewport coordinates */
    double MatIn[4][4], MatOut[4][4];

    bool m_bUse120StyleShaders;

    QMatrix4x4 m_orthoMatrix;
    QMatrix4x4 m_pvmMatrix;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_modelMatrix;

    QPoint m_ptPopUp;
    QPoint m_LastPoint;

    Vector3d m_transIncrement;
    double m_glScaleIncrement;

    Vector3d m_RealPopUp;
    Vector3d m_glViewportTrans;// the translation vector in gl viewport coordinates
    Vector3d m_glRotCenter;    // the center of rotation in object coordinates... is also the opposite of the translation vector

    QPixmap m_PixTextOverlay;

    int m_iBodyElems;
    int m_iWingElems[MAXWINGS], m_iWingMeshElems;

    int m_Ny[MAXWINGS];
    int m_nHighlightLines, m_HighlightLineSize;

    int m_iBodyMeshLines;
    int m_iWingOutlinePoints[MAXWINGS];
    int m_iMomentPoints;

    float ab_old[16], ab_new[16];
    int m_iTransitionInc;

    QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/

};

void GLLineStipple(int style);


#endif // GL3DVIEW_H
