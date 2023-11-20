/****************************************************************************

    Optim2d Class
    Copyright (C) 2021 André Deperrois

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


#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QStandardItemModel>

#include <xdirect/optim2d/particle.h>
#include <xdirect/optim2d/optimtask.h>
#include <xflgraph/graph.h>

class Foil;
class FoilWt;
class Polar;
class GraphWt;
class DoubleEdit;
class IntEdit;
class XFoilTask;
class MOPSOTask2d;
struct OptObjective;

class PlainTextOutput;
class CPTableView;
class ActionDelegate;
class ActionItemModel;

#define NOBJECTIVES 6

class Optim2d : public QDialog
{
    Q_OBJECT

    public:
        Optim2d(QWidget *pParent);
        ~Optim2d();

        void setFoil(Foil *pFoil);

        bool isModified() const {return m_bModified;}

        QSize sizeHint() const override {return QSize(1300, 900);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;

        void setupLayout();
        void connectSignals();

        void cancelTask();
        void enableControls(bool bEnable);
        void evolution();
        void fillObjectives();
        void makePSOSwarm();
        void makeRandomParticle(Particle *pParticle) const;
        void outputText(QString const &msg);
        void readData();
        void readObjectives();
        void runXFoil(const Foil *pFoil, double &Cl, double &Cd, double &ClCd, double &minCp , double &Cm, double &Cm0, bool &bConverged);
        void setPSOObjectives(MOPSOTask2d *pSOTask);
        void swarm();
        void updatePolar();
        void updateTaskParameters();
        void updateVariables(MOPSOTask2d *pPSOTask2d);

    private slots:
        Foil *onStoreBestFoil();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onContinueBest();
        void onMakeSwarm(bool bShow=true);
        void onObjTableClicked(QModelIndex index);
        void onPlotHH();
        void onResizeColumns();
        void onRunOptimizer();
        void onRunXFoil();
        void onTEFlapCheck();
        void reject() override;

        void onIterEvent(OptimEvent *result);

    private:
        bool m_bSaved;
        bool m_bModified;

        Foil *m_pFoil; // not const, potential need to adjust the TE hinge location
        Foil *m_pBestFoil; // used to animate the display
        Polar *m_pPolar;

        QVector<Foil*> m_TempFoils; /**< pointers to debug foils to delete on exit */

        int m_iLE;  /**< the index of the leading edge point for thee current aoa */


        bool m_bIsSwarmValid;
        double m_FlapAngle; // the optimized flap angle

        MOPSOTask2d *m_pPSOTask;

        //XFoil
        static double s_Alpha;
        static double s_Re, s_Mach;
        static double s_NCrit;
        static double s_XtrTop;
        static double s_XtrBot;

        //Flap
        static bool s_bTEFlap;
        static double s_FlapAngleMin, s_FlapAngleMax;
        static double s_XHinge,  s_YHinge;

        //objectives
        static bool s_bCl, s_bCd, s_bClCd, s_bCpmin,  s_bCm, s_bCm0;
        static int s_ClObjType, s_CdObjType, s_ClCdObjType, s_CpMinObjType, s_CmObjType, s_Cm0ObjType;
        static double s_Cl, s_Cd, s_ClCd, s_Cpmin,  s_Cm, s_Cm0;
        static double s_ClMaxError, s_CdMaxError, s_ClCdMaxError, s_CpMaxError, s_CmMaxError, s_Cm0MaxError;


        //Hicks-Henne
        static double s_HHt1;     /**< t1 parameter of the HH functions */
        static double s_HHt2;     /**< t2 parameter of the HH functions */
        static int    s_HHn;      /**< number of HH functions to use */
        static double s_HHmax;    /**< the max amplitude of the HH functions */


        // interface
        Graph m_ObjGraph[NOBJECTIVES];
        GraphWt *m_pObjGraphWt[NOBJECTIVES];

        FoilWt *m_pFoilWt;

        QVector<OptObjective> m_Objective;

        // XFoil
        QPushButton *m_ppbXFoilRun;
        DoubleEdit *m_pdeAlpha, *m_pdeRe, *m_pdeMach, *m_pdeNCrit, *m_pdeXtrTop, *m_pdeXtrBot;

        //T.E. flap
        QCheckBox *m_pchTEFlap;
        DoubleEdit *m_pdeTEYHinge, *m_pdeTEXHinge;
        DoubleEdit *m_pdeFlapAngleMin, *m_pdeFlapAngleMax;

        //Objectives
        CPTableView *m_pcptObjective;
        QStandardItemModel *m_pObjModel;
        ActionDelegate *m_pObjDelegate;

        // Hicks-Henne
        IntEdit *m_pieNHH;
        DoubleEdit *m_pdeHHt1, *m_pdeHHt2, *m_pdeHHmax;
        Graph m_HHGraph;
        GraphWt *m_pHHGraphWt;

        //PSO
        DoubleEdit *m_pdeInertiaWeight;
        DoubleEdit *m_pdeCognitiveWeight;
        DoubleEdit *m_pdeSocialWeight;
        DoubleEdit *m_pdeProbaRegen;

        IntEdit *m_piePopSize;
        IntEdit *m_pieMaxIter;
        QCheckBox *m_pchMultithread;

        QPushButton * m_ppbSwarm, *m_ppbMakeSwarm, *m_ppbStoreBestFoil, *m_ppbContinueBest;

        QSplitter *m_pLeftSplitter ;
        QSplitter *m_pHSplitter, *m_pVSplitter;
        PlainTextOutput *m_ppto;

        QDialogButtonBox *m_pButtonBox;

        static QByteArray s_Geometry;
        static QByteArray s_LeftSplitterSizes, s_HSplitterSizes, s_VSplitterSizes;
};


