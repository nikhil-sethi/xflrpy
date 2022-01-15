/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QLabel>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/linestyle.h>

class IntEdit;
class DoubleEdit;
class LineBtn;


class gl3dAttractor : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dAttractor(QWidget *pParent = nullptr);
        ~gl3dAttractor() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

        void keyPressEvent(QKeyEvent *pEvent) override;


    private slots:
        void moveIt();
        void onResetDefaults();
        void onRestart();
        void onLineStyle(LineStyle);

    private:
        double f(double x, double y, double z) const;
        double g(double x, double y, double z) const;
        double h(double x, double y, double z) const;


    private:
        bool m_bResetAttractor;
        int m_Counter;
        int m_iLead;

        QElapsedTimer m_LastTime;

        QTimer *m_pTimer;
        QVector<Vector3d> m_Trace;

        QOpenGLBuffer m_vboTrace;

        DoubleEdit *m_pdeSigma, *m_pdeRho, *m_pdeBeta;
        DoubleEdit *m_pdeX, *m_pdeY, *m_pdeZ;
        IntEdit *m_pieIntervalms, *m_pieMaxPts;
        DoubleEdit *m_pdeDt;
        LineBtn *m_plbStyle;

        static int s_RefreshInterval;
        static int s_MaxPts;
        static double s_dt;
        static double s_Sigma, s_Rho, s_Beta;
        static Vector3d s_P;
        static LineStyle s_ls;

};

