/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QSettings>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>

#include <xflgeom/geom3d/vector3d.h>

class IntEdit;
class DoubleEdit;

class gl2dFractal : public QOpenGLWidget
{
    Q_OBJECT
    public:
        gl2dFractal(QWidget *pParent = nullptr);

        virtual QSize sizeHint() const override {return QSize(1500, 1100);}
        void initializeGL() override;
        void paintGL()  override;
        void showEvent(QShowEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setScaleFactor(double f) {s_ScaleFactor=float(f);}

    private:
        void screenToViewport(QPoint const &point, QVector2D &real) const;
        void makeQuad();
        void startDynamicTimer();
        void stopDynamicTimer();

    private slots:
        void onDynamicIncrement();

    private:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLBuffer m_vboQuad;

        QOpenGLShaderProgram m_shadFrac;

        // shader uniforms
        int m_locIters;
        int m_locLength;
        int m_locViewTrans;
        int m_locViewScale;
        int m_locViewRatio;

        //shader attributes
        int m_attrVertexPosition;

        //
        QPoint m_LastPoint, m_PressedPoint;
        float m_Scale;
        QPointF m_ptOffset;          /**< the foil's leading edge position in screen coordinates */

        QRectF m_rectMandelbrot;


        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer;

        QPointF m_Trans;
        bool m_bDynTranslation;

        bool m_bDynScaling;
        float m_ZoomFactor;

        IntEdit *m_pieMaxIter;
        DoubleEdit *m_pdeMaxLength;
        QLabel *m_plabScale;

        static float s_ScaleFactor;

        static int s_MaxIter;
        static float s_MaxLength;

};

