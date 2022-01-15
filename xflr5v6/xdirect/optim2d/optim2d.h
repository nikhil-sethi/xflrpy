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
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>

#include <xflobjects/objects2d/foil.h>
#include <xdirect/optim2d/particle.h>
#include <xflgraph/graph.h>

class Foil;
class FoilWt;
class Polar;
class GraphWt;
class DoubleEdit;
class IntEdit;
class XFoilTask;
class GATask;
class MOPSOTask2d;


class Optim2d : public QDialog
{
    Q_OBJECT

    public:
        Optim2d(QWidget *pParent);
        ~Optim2d();

        void setFoil(const Foil *pFoil);

        bool isModified() const {return m_bModified;}

        QSize sizeHint() const override {return QSize(1300, 900);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        void setupLayout();
        void connectSignals();

        void readData();
        void outputText(QString const &msg) const;

        // common
        void makeRandomParticle(Particle *pParticle) const;

        void makePSOSwarm();
        void makeGAGen();

        void swarm();
        void evolution();
        void updateParetoGraph();

    private slots:
        void reject() override;
        void onButton(QAbstractButton *pButton);
        void onClose();

        void onAlgorithm();
        void onAnalyze();
        void onOptimize();
        void onMakeSwarm(bool bShow=true);
        void onStoreBestFoil();

    private:
        bool m_bSaved;
        bool m_bModified;

        Foil const *m_pFoil;
        Foil *m_pBestFoil; // used to animate the display

        Polar *m_pPolar;

        QVector<Foil*> m_TempFoils; /**< pointers to debug foils to delete on exit */

        int m_iLE;  /**< the index of the leading edge point for thee current aoa */


        MOPSOTask2d *m_pPSOTask;
        GATask *m_pGATask;

        //XFoil
        static double s_Alpha;
        static double s_Re;
        static double s_NCrit;
        static double s_XtrTop;
        static double s_XtrBot;

        //Target
        static double s_Cl;
        static double s_ClMaxError;
        static double s_Cd;
        static double s_CdMaxError;

        static double s_HHt2;     /**< t2 parameter of the HH functions */
        static int    s_HHn;      /**< number of HH functions to use */
        static double s_HHmax;    /**< the max amplitude of the HH functions */

        //Common
        static bool   s_bPSO;
        static int    s_Dt;

        // interface
        Graph m_ErrorGraph;
        GraphWt *m_pErrorGraphWt;
        Graph m_ParetoGraph;
        GraphWt *m_pParetoGraphWt;

        FoilWt *m_pFoilWt;

        // XFoil
        DoubleEdit *m_pdeAlpha, *m_pdeRe, *m_pdeNCrit, *m_pdeXtrTop, *m_pdeXtrBot;
        QPushButton *m_ppbAnalyze;

        // Optim targets
        DoubleEdit *m_pdeCl, *m_pdeClMaxError;
        DoubleEdit *m_pdeCd, *m_pdeCdMaxError;

        // Hicks-Henne
        IntEdit *m_pieNHH;
        DoubleEdit *m_pdeHHt2, *m_pdeHHmax;

        //PSO
        DoubleEdit *m_pdeInertiaWeight;
        DoubleEdit *m_pdeCognitiveWeight;
        DoubleEdit *m_pdeSocialWeight;

        //GA
        DoubleEdit *m_pdeProbXOver, *m_pdeProbMutation, *m_pdeSigmaMutation;

        //Common
        QStackedWidget *m_pswAlgo;
        IntEdit *m_piePopSize;
        IntEdit *m_pieMaxIter, *m_pieUpdateDt;
        QRadioButton *m_prbPSO, *m_prbGA;
        QCheckBox *m_pchMultithread;

        QPushButton * m_ppbSwarm, *m_ppbMakeSwarm, *m_ppbStoreBestFoil;

        QSplitter *m_pHSplitter, *m_pVSplitter;
        QPlainTextEdit *m_ppt;

        QTimer m_Timer;

        QDialogButtonBox *m_pButtonBox;

        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes, s_VSplitterSizes;
};


