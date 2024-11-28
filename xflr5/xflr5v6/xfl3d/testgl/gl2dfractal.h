/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QOpenGLShaderProgram>
#include <QRadioButton>
#include <QCheckBox>
#include <QSettings>

#include <xfl3d/testgl/gl2dview.h>

#include <QLabel>
#include <QSlider>


class IntEdit;
class DoubleEdit;

class gl2dFractal : public gl2dView
{
    Q_OBJECT
    public:
        gl2dFractal(QWidget *pParent = nullptr);

        QPointF defaultOffset() override {return QPointF(+0.5*float(width()),0.0f);}

        void initializeGL() override;
        void paintGL()  override;

        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onMode();

    private:
        QRadioButton *m_prbMandelbrot, *m_prbJulia;
        IntEdit *m_pieMaxIter;
        DoubleEdit *m_pdeMaxLength;
        QLabel *m_plabScale;
        QCheckBox *m_pchShowSeed;
        QSlider *m_pslTau;

        QOpenGLBuffer m_vboRoots;
        QOpenGLBuffer m_vboSegs;

        QOpenGLShaderProgram m_shadFrac;
        // shader uniforms
        int m_locJulia;
        int m_locParamX;
        int m_locParamY;
        int m_locIters;
        int m_locHue;
        int m_locLength;
        int m_locViewTrans;
        int m_locViewScale;
        int m_locViewRatio;

        //shader attributes
        int m_attrVertexPosition;

        bool m_bResetRoots;
        int m_iHoveredRoot;
        int m_iSelectedRoot;
        float m_amp0, m_phi0; /** The seed's initial position */

        static int s_Hue;
        static int s_MaxIter;
        static float s_MaxLength;
        static QVector2D s_Seed;
};
