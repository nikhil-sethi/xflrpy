/****************************************************************************

    PanelAnalysis Class

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

#include <complex>

#include <QObject>
#include <QVector>

#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/panel.h>
#include <xflanalysis/analysis3d_params.h>

#define VLMMAXRHS 100

class Plane;
class WPolar;
class PlaneOpp;
class PlaneTaskEvent;
class Wing;
class Polar;
class Surface;

class PanelAnalysis : QObject
{
    Q_OBJECT

    friend class PanelAnalysisDlg;
    friend class Wing;
    friend class Objects3D;
    friend class Miarex;
    friend class PlaneTask;
    friend class XflScriptExec;

    public:
        PanelAnalysis();
        ~PanelAnalysis();

        bool initializeAnalysis();

        bool solveUnitRHS();

        bool loop();
        bool alphaLoop();
        bool QInfLoop();
        bool unitLoop();
        bool controlLoop();

        bool getZeroMomentAngle();

        void buildInfluenceMatrix();

        void computeAeroCoefs(double V0, double VDelta, int nrhs);
        void computeOnBodyCp(double V0, double VDelta, int nval);
        void computePlane(double Alpha, double QInf, int qrhs);
        void computeFarField(double QInf, double Alpha0, double AlphaDelta, int nval);
        void computeBalanceSpeeds(double Alpha, int q);
        void createDoubletStrength(double Alpha0, double AlphaDelta, int nval);
        void createSourceStrength(double Alpha0, double AlphaDelta, int nval);
        void createRHS(double *RHS, Vector3d VInf, double *VField=nullptr);
        void createUnitRHS();
        void createWakeContribution();
        void createWakeContribution(double *pWakeContrib, Vector3d const &WindDirection);
        void getDoubletInfluence(Vector3d const &C, const Panel *pPanel, Vector3d &V, double &phi, bool bWake=false, bool bAll=true) const;
        void getSourceInfluence(Vector3d const &C, Panel *pPanel, Vector3d &V, double &phi) const;
        void scaleResultstoSpeed(int nval);
        void sumPanelForces(double const *Cp, double Alpha, double &Lift, double &Drag);
        void VLMGetVortexInfluence(const Panel *pPanel, Vector3d const &C, Vector3d &V, bool bAll) const;
        void VLMCmn(Vector3d const &A, Vector3d const &B, Vector3d const &C, Vector3d &V, bool bAll) const;
        void VLMQmn(const Vector3d &LA, const Vector3d &LB, const Vector3d &TA, const Vector3d &TB, Vector3d const &C, Vector3d &V) const;

        void panelTrefftz(Wing *pWing, double QInf, double Alpha, const double *Mu, const double *Sigma, int pos, Vector3d &Force, double &WingIDrag,
                          const WPolar *pWPolar, const Panel *pWakePanel, const Vector3d *pWakeNode) const;
        void getDoubletDerivative(const int &p, double const*Mu, double &Cp, Vector3d &VLocal, double QInf, double Vx, double Vy, double Vz) const;
        void getVortexCp(int p, const double *Gamma, double *Cp, const Vector3d &VInf) const;


        void computeStabilityDerivatives();
        void computeStabilityInertia();
        bool computeTrimmedConditions();
        bool solveEigenvalues();
        void buildRotationMatrix();
        void buildStateMatrices();
        void computeControlDerivatives();
        void computeNDStabDerivatives();
        void forces(double *Mu, double *Sigma, double alpha, Vector3d Vinc, double *VInf, Vector3d &Force, Vector3d &Moment);
        double computeCm(double Alpha) const;

        bool allocateMatrix(int matSize, int &memsize);
        bool allocateRHS(int matSize, int &memsize);
        void releaseArrays();
        void rotateGeomY(double Alpha, Vector3d const &P, int NXWakePanels);
        void rotateGeomZ(double Beta, Vector3d const &P, int NXWakePanels);

        void traceLog(QString str) const;

        void setControlPositions(double t, int &NCtrls, QString &out, bool bBCOnly);

        void restorePanels();
        void setArrayPointers(Panel *pPanel, Panel *pMemPanel, Panel *pWakePanel, Panel *pRefWakePanel, Vector3d *pNode, Vector3d *pMemNode, Vector3d *pWakeNode, Vector3d *pRefWakeNode, Vector3d *pTempWakeNode);
        void setArraySize(int MatSize, int WakeSize, int nNodes, int nWakeNodes, int NWakeColumn);
        void setInertia(double ctrl, double alpha, double beta);
        void setObjectPointers(Plane *pPlane, QVector<Surface *> *pSurfaceList);
        void setRange(double vMin, double VMax, double vDelta, bool bSequence);
        void setWPolar(WPolar*pWPolar){m_pWPolar = pWPolar;}
        PlaneOpp* createPlaneOpp(double *Cp, const double *Gamma, const double *Sigma);

        void getSpeedVector(Vector3d const &C, const double *Mu, const double *Sigma, Vector3d &VT, bool bAll=true) const;
        void computePhillipsFormulae();

        void clearPOppList();

        static bool s_bCancel;      /**< true if the user has cancelled the analysis */
        static bool s_bWarning;     /**< true if one the OpPoints could not be properly interpolated */
        static void setMaxWakeIter(int nMaxWakeIter) {s_MaxWakeIter = nMaxWakeIter;}

    signals:
        void outputMsg(QString msg) const;

    public slots:
        void onCancel();

    private:
        static bool s_bTrefftz;     /**< /true if the forces should be evaluated in the far-field plane rather than by on-body summation of panel forces */
        static bool s_bKeepOutOpp;  /**< true if points with viscous interpolation issues should be stored nonetheless */

        int s_MaxRHSSize;    /**< the max number of RHS points, used for memeory allocation >*/
        int m_MaxMatSize;    /**< the size currently allocated for the influence matrix >*/

        static int s_MaxWakeIter;                 /**< wake roll-up iteration limit */

        double m_Progress;   /**< A measure of the progress of the analysis, used to provide feedback to the user */
        double m_TotalTime;     /**< the esimated total time of the analysis, used to set the progress bar. No specific unit. */

        bool m_bPointOut;           /**< true if an interpolation was outside the min or max Cl */
        bool m_bSequence;           /**< true if the calculation is should be performed for a range of aoa */

        int m_nRHS;                 /**< the number of RHS to calculate; cannot be greater than VLMMAXRHS */
        int m_nNodes;               /**< the number of nodes  */
        int m_MatSize;              /**< the number of panels. Is also the size of the linear problem */
        int m_nWakeNodes;           /**< the number of wake nodes */
        int m_WakeSize;                /**< the number of wake elements */
        int m_NWakeColumn;          /**< the number of wake columns, which is also the number of panels in the spanwise direction */


        double m_vMin;              /**< The minimum value of the analysis parameter*/
        double m_vMax;              /**< The max value of the analysis parameter*/
        double m_vDelta;            /**< The increment value of the analysis parameter*/

        double m_Alpha;             /**< The angle of attack of the current calculation in degree >*/
        double m_OpAlpha;           /**< The angle of attack of the current calculation used in Tilted analysis, in degree >*/
        double m_QInf;              /**< The freestream velocity of the current calculation in m/s >*/
        double m_OpBeta;            /**< The sideslip angle of the current calculation, in degrees >*/

        double m_CL;                /**< The lift coefficient */
        double m_CX;                /**< The drag coefficient */
        double m_CY;                /**< The side force coefficient */

        double m_InducedDrag;       /**< The UFO's induced drag coefficient */
        double m_ViscousDrag;       /**< The UFO's viscous drag coefficient */
        double m_VCm;               /**< The UFO's viscous pitching moment coefficient */
        double m_VYm;               /**< The UFO's viscous yawing moment coefficient */
        double m_ICm;               /**< The UFO's induced pitching moment coefficient */
        double m_IYm;               /**< The UFO's induced yawing moment coefficient */
        double m_GCm;               /**< The UFO's total pitching moment coefficient */
        double m_GRm;               /**< The UFO's total rolling moment coefficient */
        double m_GYm;               /**< The UFO's total yawing moment coefficient */

        Vector3d m_CP;               /**< The position of the center of pressure */

        double XNP;                 /**< Neutral point x-position resulting from stability analysis */
        double CXu, CZu, Cmu, CXq, CZq, Cmq, CXa, CZa, Cma; // Non dimensional stability derivatives
        double CYb, CYp, CYr, Clb, Clp, Clr, Cnb, Cnp, Cnr;
        double CXe, CYe, CZe, Cle, Cme, Cne;


        //analysis arrays
        double *m_RHS;           /**< RHS vector. Is declared as a common member variable to save memory allocation times */
        double *m_RHSRef;        /**< RHS vector. Is declared as a common member variable to save memory allocation times */
        double *m_SigmaRef;
        double *m_Sigma;         /**< The array of resulting source strengths of the analysis */
        double *m_Mu;            /**< The array of resulting doublet strengths, or vortex circulations if the panel is located on a thin surface */
        double *m_Cp;            /**< The array of pressure coef per panel */
        double *m_3DQInf;        /**< a pointer to the calculated balance speeds for each aoa in Type 2 and Type 7 analysis */


        double *m_aij;           /**< coefficient matrix for the panel analysis. Is declared as a common member variable to save memory allocation times*/
        double *m_aijWake;       /**< coefficient matrix. Is declared as a common member variable to save memory allocation times*/
        double *m_uRHS, *m_vRHS, *m_wRHS;
        double *m_pRHS, *m_qRHS, *m_rRHS;
        double *m_cRHS;
        double *m_uWake, *m_wWake;
        int *m_Index;               /**< a pointer to the array of indexes used in matrix LU decomposition */
        QVector<Vector3d> m_uVl, m_wVl;


        // pointers to the geometry input data
        // these arrays are defined in the QMiarex handling class,
        Panel *m_pPanel;            /**< the current working array of array of panels */
        Panel *m_pWakePanel;        /**< the current working array of wake panel array */
        Panel const *m_pRefWakePanel;     /**< a copy of the reference wake node array if wake needs to be reset */
        Panel const *m_pMemPanel;         /**< a copy of the reference panel array if the panels need to be restored, for instance after control surfaces have been rotated*/

        Vector3d *m_pNode;            /**< the working array of nodes  */
        Vector3d const *m_pMemNode;        /**< a copy of the reference node array, if the nodes need to be restored */
        Vector3d *m_pWakeNode;        /**< the current working wake node array */
        Vector3d const *m_pRefWakeNode;   /**< a copy of the reference wake node array if the flat wake geometry needs to be restored */
        Vector3d *m_pTempWakeNode;  /**< a temporary array to hold the calculations of wake roll-up */


        // pointers to the object input data
        Plane *m_pPlane;            /**< a pointer to the plane object, or NULL if the calculation is performed on a wing */
        WPolar *m_pWPolar;          /**< a pointer to the current WPolar object */

        //temp data
        int m_NSpanStations;


        //    Vector3d h, r0, r1, r2, Psi, t, Far;
        //    double r1v,r2v,ftmp, Omega;
        //    Vector3d *m_pR[5];

    public:
        double *m_Ai;     /**< The array of calculated induced angles */
        double *m_Cl;     /**< The array of calculated lift coefficients */
        double *m_ICd;    /**< The array of calculated induced angles */
        Vector3d *m_F;     /**< The array of calculated forces on each chordwise strip */
        Vector3d *m_Vd;    /**< The array of calculated downwash velocity at the T.E. of each chordwise strip */
        Vector3d m_WingForce[MAXWINGS*VLMMAXRHS];               /**< The array of calculated resulting forces acting on the Wing objects */
        double m_WingIDrag[MAXWINGS*VLMMAXRHS];                /**< The array of calculated resulting induced drag acting on the Wing objects */
        Wing * m_pWingList[MAXWINGS];                          /**< The array of pointers to the plane's Wing objects */

    public: //stability analysis method and variables

        int m_NCtrls;  /**< The total number of control surfaces */

        Vector3d Force0;  /** The calculated equilibrium force */
        Vector3d Moment0; /** The calculated equilibrium moment */

        // longitudinal stability derivatives
        double Xu, Xw, Zu, Zw, Xq, Zq, Mu, Mw, Mq;//first order
        double Zwp, Mwp;                          //second order derivatives, cannot be calculated by a panel method, set to zero.

        // latal stability derivatives
        double Yv, Yp, Yr, Lv, Lp, Lr, Nv, Np, Nr;//first order

        //stability control derivatives
        double Xde, Yde, Zde, Lde, Mde, Nde;

        double u0;              /**< steady state x-velocity, used in stability analysis */
        double Theta0;          /**< steady state pitch angle, used in stability analysis */
        double m_ALong[4][4];    /**< The longitudinal state matrix */
        double m_ALat[4][4];    /**< The lateral state matrix */
        double m_BLong[4];      /**< The longitudinal control vector */
        double m_BLat[4];       /**< The lateral control vector */
        double m_R[3][3];        /**< Rotation matrix, used as a temp variable in the calculations */


        double m_Ib[3][3];              /**< Inertia tensor in body (geometrical) axis */
        double m_Is[3][3];              /**< Inertia tensor in stability axis */

        std::complex<double> m_rLong[4];   /**< complex longitudinal eigenvalues resulting from the stability analysis*/
        std::complex<double> m_rLat[4];    /**< complex lateral eigenvalues resulting from the stability analysis*/
        std::complex<double> m_vLong[16];  /**< complex longitudinal eigenvectors resulting from the stability analysis*/
        std::complex<double> m_vLat[16];   /**< complex lateral eigenvectors resulting from the stability analysis*/

        std::complex<double> m_phiPH;      /**< complex phugoid eigenvalue computed using Phillip's formula */
        std::complex<double> m_phiDR;      /**< complex Dutch roll eigenvalue computed using Phillip's formula */

        double m_AlphaEq;             /**< the balance aoa, calculated in stability analysis */
        double m_Ctrl;                /**< the control parameter, which defines the position of the control surfaces */

        QVector<Surface*> *m_ppSurface;        /**< A pointer to the array of Surface objects */
        QVector<PlaneOpp*> m_PlaneOppList;


        bool m_bTrace;

        double m_Mass;         /** The value of the mass for the calculation. Is set from the mean value, the gain, and the control parameter */
        Vector3d m_CoG;         /** The value of the CoG for the calculation. Is set from the mean value, the gain, and the control parameter */
        double m_Inertia[4];   /** The value of the inertia tensor components for the calculation. Is set from the mean value, the gain, and the control parameter. */
};

