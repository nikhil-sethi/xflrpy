/****************************************************************************

    Xfl3d
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QElapsedTimer>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QTimer>

#include <xflcore/linestyle.h>
#include <xfl3d/views/shadloc.h>

#define MAXROOTS 5

class gl2dView : public QOpenGLWidget
{
    Q_OBJECT
    public:
        gl2dView(QWidget *pParent = nullptr);

        virtual QSize sizeHint() const override {return QSize(1500, 1100);}


    protected:
        void showEvent(QShowEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void resizeGL(int width, int height)  override;
        void initializeGL() override;

        void makeQuad();
        void startDynamicTimer();
        void stopDynamicTimer();

        void screenToViewport(QPoint const &point, QVector2D &real) const;
        void screenToWorld(QPoint const &screenpt, QVector2D &pt);

        void paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride);
        void paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh);
        void paintSegments(QOpenGLBuffer &vbo, QColor const &clr, float thickness, Line::enumLineStipple stip, bool bHigh);

        virtual QPointF defaultOffset() = 0;

    protected slots:
        void onDynamicIncrement();

    signals:
        void ready2d();

    protected:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLBuffer m_vboQuad;

        QOpenGLShaderProgram m_shadPoint;
        ShaderLocations m_locPoint;

        QOpenGLShaderProgram m_shadLine;
        ShaderLocations m_locLine;

        QPoint m_LastPoint, m_PressedPoint;
        float m_Scale;
        QPointF m_ptOffset;


        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer;

        QPointF m_Trans;
        bool m_bDynTranslation;

        bool m_bDynScaling;
        float m_ZoomFactor;

        QRectF m_rectView;
        QRectF m_GLViewRect;    /**< The OpenGl viewport.*/

        bool m_bInitialized;

        int m_nRoots;
};


