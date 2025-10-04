/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QDate>
#include <QLabel>

#include <xfl3d/controls/light.h>
#include <xfl3d/testgl/gl3dtestglview.h>
#include <xfl3d/testgl/spaceobject.h>
#include <xflgeom/geom3d/vector3d.h>

class IntEdit;
class DoubleEdit;


class gl3dSolarSys : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dSolarSys(QWidget *pParent = nullptr);
        ~gl3dSolarSys() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onMovePlanets();
        void onRestart();
        void onPlanetSize(int size);

        void onHalley(bool bShow);

    private:
        void hideEvent(QHideEvent *pEvent) override;
        void initializeGL() override;
        void glRenderView() override;
        void glMake3dObjects() override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void makePlanets();


    private:
        QDate m_Elapsed;
        bool m_bResetPlanet;
        bool m_bResetStars;
        bool m_bHalley;

        QVector<Planet> m_Planet;

        Planet m_Halley;

        QTimer m_Timer;

        DoubleEdit *m_pdeDt, *m_pdePlanetSize;
        QVector<Star> m_Stars;

        QLabel *m_plabDate;
        QLabel *m_plabHalley;

        QVector<QOpenGLBuffer> m_vboCircle;
        QOpenGLBuffer m_vboSaturnDisk;
        QOpenGLBuffer m_vboHalleyEllipse;
        QOpenGLBuffer m_vboStars;

        Light m_RefLight;

        static double s_dt;
        static double s_PlanetSize;
};


