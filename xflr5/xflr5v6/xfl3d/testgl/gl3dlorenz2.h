/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QLabel>
#include <QStack>

#include <xfl3d/testgl/gl3dtestglview.h>


class IntEdit;
class DoubleEdit;


class gl3dLorenz2 : public gl3dTestGLView
{
    Q_OBJECT
    public:
        gl3dLorenz2(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;

    private slots:
        void onRestart();
        void onParticleSize();

    private:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLShaderProgram m_shadCompute;

        // CS uniforms
        int m_locRadius, m_locDt;

        // VS/FS uniforms
        int m_locPosition, m_locFillColor;

        QOpenGLBuffer m_ssbParticle;

        bool m_bResetParticles;

        QTimer m_Timer;

        DoubleEdit *m_pdeScatter;
        DoubleEdit *m_pdeX, *m_pdeY, *m_pdeZ;
        DoubleEdit *m_pdeDt;

        IntEdit *m_pieNGroups;
        QLabel *m_plabNMaxGroups;
        QLabel *m_plabNParticles;

        QLabel *m_plabFrameRate;

        DoubleEdit *m_pdeParticleSize;

        QStack<int> m_stackInterval;

        static double s_X0, s_Y0, s_Z0;
        static double s_Scatter;
        static double s_dt;
        static float s_Size;


        static int s_NGroups;
};


