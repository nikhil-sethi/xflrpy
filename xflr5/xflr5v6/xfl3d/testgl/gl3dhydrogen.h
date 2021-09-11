/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QEvent>

#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>

class DoubleEdit;
class IntEdit;

class gl3dHydrogen : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dHydrogen(QWidget *pParent = nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;
        void initializeGL() override;

        void customEvent(QEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void closeEvent(QCloseEvent *pEvent) override;

        void paintElectronInstances(QOpenGLBuffer &vboPosInstances, float radius, QColor const &clr, bool bTwoSided, bool bLight);

        double psi(double r, double theta, double phi) const;

        void collapseBlock(QWidget *pParent) const;

    private slots:
        void onCollapse();


    private:
        IntEdit *m_piel, *m_piem, *m_pien;
        IntEdit *m_pieNObs;
        DoubleEdit *m_pdeObsRad;

        QRadioButton *m_prbPtShader, *m_prbSurfShader;

        QCheckBox *m_pchBohr, *m_pchObsRad;
        QPushButton *m_ppbMake;

        QLabel *m_plabNObs;

        DoubleEdit *m_pdePtWidth;

        bool m_bResetPositions;
        QOpenGLBuffer m_vboObservations;

        QVector<Vector3d> m_Pts; // electron collapsed position
        QVector<float>m_State;      // value of the |psi|² at the collapsed position

        bool m_bCancel;
        bool m_bIsObserving;

        float m_StateMax;


        int m_BlockSize;

        int m_UpdateInterval;

        static int s_l, s_m, s_n;
        static int s_NObservations;
        static double s_ObsRadius;
        static double s_PtWidth;
};


const QEvent::Type HYDROGEN_EVENT         = static_cast<QEvent::Type>(QEvent::User + 200);

class HydrogenEvent : public QEvent
{
    public:
        HydrogenEvent(): QEvent(HYDROGEN_EVENT)
        {
        }

        void setNewPoints(QVector<Vector3d> const &points) {m_NewPoints=points;}
        void setNewStates(QVector<float> const &states)    {m_NewStates=states;}

        QVector<Vector3d> const & newPoints() const {return m_NewPoints;}
        QVector<float> const & newStates() const {return m_NewStates;}

    private:
        QVector<Vector3d>  m_NewPoints;
        QVector<float>     m_NewStates;
};
