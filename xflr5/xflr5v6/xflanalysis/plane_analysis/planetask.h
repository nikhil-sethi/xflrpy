/****************************************************************************

    PlaneTask Class

    Copyright (C) André Deperrois

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


#include <QEvent>
#include <QTextStream>

#include <xflanalysis/plane_analysis/lltanalysis.h>
#include <xflanalysis/plane_analysis/panelanalysis.h>

class Plane;
class WPolar;
class PlaneOpp;
class Surface;
class Panel;
class Vector3d;


/**
 * @struct A set of a plane, a polar and a range to analyze, used by a thread to perform one analysis.
 */
struct PlaneAnalysis
{
    Plane *pPlane=nullptr;            /**< a pointer to the Foil to be analyzed by the thread */
    WPolar *pWPolar=nullptr;          /**< a pointer to the polar to be analyzed by the thread */
    double vMin=0, vMax=0, vInc=0;
};


class PlaneTask : public QObject
{
    Q_OBJECT
    friend class MainFrame;
    friend class Miarex;
    friend class PanelAnalysisDlg;
    friend class LLTAnalysisDlg;
    friend class gl3dMiarexView;
    friend class gl3dView;

    public:
        PlaneTask();
        ~PlaneTask();

        void initializeTask(Plane *pPlane, WPolar *pWPolar, double vMin, double vMax, double VInc, bool bSequence = true);
        void initializeTask(PlaneAnalysis *pAnalysis);

        bool   allocatePanelArrays(int &memsize);
        int    calculateMatSize();
        int    createBodyElements(Plane *pCurPlane);
        bool   createWakeElems(int PanelIndex, const Plane *pPlane, const WPolar *pWPolar);
        int    createSurfaceElements(const Plane *pPlane, const WPolar *pWPolar, Surface *pSurface);
        bool   initializePanels();
        void   insertPOpp(PlaneOpp *pPOpp);
        int    isNode(Vector3d &Pt);
        int    isWakeNode(Vector3d &Pt);
        void   joinSurfaces(WPolar*pWPolar, Surface *pLeftSurf, Surface *pRightSurf, int pl, int pr);
        void   releasePanelMemory();
        void   setStaticPointers();
        void   stitchSurfaces();
        void   setLLTAnalysis(LLTAnalysis &LLTAnalysis)       {m_ptheLLTAnalysis   = &LLTAnalysis;}
        void   setPanelAnalysis(PanelAnalysis &panelAnalysis) {m_pthePanelAnalysis = &panelAnalysis;}

        bool isFinished(){return m_bIsFinished;}

        WPolar *  setWPolarObject(Plane *pCurPlane, WPolar *pCurWPolar);
        Plane *   setPlaneObject(Plane *pPlane);

        void LLTAnalyze();
        void PanelAnalyze();

        static void cancelTask(){s_bCancel=true;}

        PanelAnalysis *m_pthePanelAnalysis;
        LLTAnalysis *m_ptheLLTAnalysis;

        bool isLLTTask() const;
        bool isPanelTask() const;

        int nNodes() const {return m_Node.size();}
        int matSize() const {return m_Panel.size();}

    public slots:
        void run();

    signals:
        void taskFinished();

    private:
        Plane *m_pPlane;
        WPolar *m_pWPolar;


        //data arrays
        QVector<Vector3d> m_Node;              /**< the node array for the currently loaded UFO*/
        QVector<Vector3d> m_MemNode;           /**< used if the analysis should be performed on the tilted geometry */
        QVector<Vector3d> m_WakeNode;          /**< the current wake node array */
        QVector<Vector3d> m_RefWakeNode;       /**< the reference wake node array if wake needs to be reset */
        QVector<Vector3d> m_TempWakeNode;      /**< a temporary array to hold the calculations of wake roll-up */

        QVector<Panel> m_Panel;               /**< the panel array for the currently loaded UFO */
        QVector<Panel> m_MemPanel;            /**< used if the analysis should be performed on the tilted geometry */
        QVector<Panel> m_WakePanel;           /**< the reference current wake panel array */
        QVector<Panel> m_RefWakePanel;        /**< the reference wake panel array if wake= new Vector3d needs to be reset */

        int m_WakeSize;                    /**< the size of the wake matrix, if a wake is included in the analysis */
        int m_NWakeColumn;                 /**< the number of wake columns */

        int m_MaxPanelSize;                  /**< the maximum matrix size consistent <ith the current memory allocation */

        QVector<Surface *> m_SurfaceList;        /**< An array holding the pointers to the wings Surface objects */

        double m_vMin, m_vMax, m_vInc;
        bool m_bSequence;
        bool m_bIsFinished;       /**< true if the calculation is over */
        static bool s_bCancel;    /**< true if all analysis should be cancelled */

};






