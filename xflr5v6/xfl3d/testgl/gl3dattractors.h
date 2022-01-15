/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/linestyle.h>

class IntEdit;
class DoubleEdit;
class LineBtn;

class gl3dAttractors : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dAttractors(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

        void keyPressEvent(QKeyEvent *pEvent) override;
//        void showEvent(QShowEvent *pEvent) override;

        double f(double x, double y, double z) const;
        double g(double x, double y, double z) const;
        double h(double x, double y, double z) const;

    private slots:
        void moveThem();
        void onRandomSeed();
        void onLineStyle(LineStyle);
        void onAttractor();

    private:
        QTimer m_Timer;
        IntEdit *m_pieNTrace, *m_pieTailSize;
        QCheckBox *m_pchLeadingSphere, *m_pchDynColor;
        LineBtn *m_plbStyle;
        QSlider *m_pslSpeed;

        QVector<QRadioButton*> m_prbAttractors;

        QVector<QVector<Vector3d>> m_Trace;
        QVector<QVector<double>> m_Velocity;
        double m_MaxVelocity;
        QOpenGLBuffer m_vboTrace;

        bool m_bResetAttractor;

        int m_iLead;

        static int s_NTrace;
        static int s_TailSize;
        static int s_iAttractor;
        static LineStyle s_ls;
        static bool s_bDynColor;
};

