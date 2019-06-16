/****************************************************************************

    BatchThreadDlg Class
    Copyright (C) 2003-201* Andre Deperrois

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

/** @file
 * This file implements the multi-threaded batch foil analysis
*/

#ifndef BATCHTHREADDLG_H
#define BATCHTHREADDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QTextEdit>

#include <analysis3d/analysis3d_enums.h>

class Foil;
class Polar;
class IntEdit;
class DoubleEdit;
class XFoilTask;
class XFoilTaskEvent;
struct FoilAnalysis;
class XDirect;

/**
 * @brief This class implements an interface to perform a multi-threaded batch foil analysis.
 */
class BatchThreadDlg : public QDialog
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;
    friend class XFoilTask;

public:
    BatchThreadDlg(QWidget *pParent=nullptr);
    ~BatchThreadDlg();
    void initDialog();

private:

    void setupLayout();
    void cleanUp();
    Polar * createPolar(Foil *pFoil, double Re, double Mach, double NCrit);

    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void reject();


    void outputFoilList();
    void outputReList();
    void readParams();
    void setFileHeader();
    void setPlrName(Polar *pNewPolar);
    void startAnalysis();
    void startThread();
    void updateOutput(QString &str);
    void writeString(QString &strong);

protected:
    void customEvent(QEvent *event); // This overrides QObject::customEvent()

private:
    void handleXFoilTaskEvent(const XFoilTaskEvent *event);

private slots:
    void onSpecChanged();
    void onAnalyze();
    void onRange();
    void onAcl();
    void onEditReList();
    void onClose();
    void onFromZero(int);
    void onInitBL(int);
    void onFoilList();
    void onFoilSelectionType();
    void onAdvancedSettings();
    void onTimerEvent();
    void onUpdatePolarView();


private:
    QRadioButton * m_pctrlFoil1, * m_pctrlFoil2;
    QPushButton *m_pctrlFoilList;
    QRadioButton *m_rbRange1, *m_rbRange2;
    QRadioButton *m_pctrlAlpha, *m_pctrlCl;
    QPushButton *m_pctrlEditList;
    DoubleEdit *m_pctrlReMin, *m_pctrlReMax, *m_pctrlReDelta, *m_pctrlMach;
    DoubleEdit *m_pctrlSpecMin, *m_pctrlSpecMax, *m_pctrlSpecDelta;
    DoubleEdit *m_pctrlACrit, *m_pctrlXTopTr, *m_pctrlXBotTr;
    IntEdit *m_pctrlMaxThreads;
    QLabel *m_pctrlSpecVar;
    QLabel *m_pctrlMaType, *m_pctrlReType;
    QCheckBox *m_pctrlInitBL, *m_pctrlFromZero, *m_pctrlUpdatePolarView;

    QPushButton *m_pctrlClose, *m_pctrlAnalyze;
    QTextEdit *m_pctrlTextOutput;

    static XDirect* s_pXDirect;           /**< a void pointer to the unique instance of the QXDirect class */
    static bool s_bCurrentFoil;        /**< true if the analysis should be performed only for the current foil */
    static bool s_bUpdatePolarView;    /**< true if the polar graphs should be updated during the analysis */
    static QPoint s_Position;          /**< the position on the client area of the dialog's topleft corner */
    static int s_nThreads;             /**< the number of available threads */

    double m_ReMin;             /**< the min Re for a range analysis */
    double m_ReMax;             /**< the max Re for a range analysis */
    double m_ReInc;             /**< the incement Re for a range analysis */
    double m_AlphaMin;          /**< The starting aoa */
    double m_AlphaMax;          /**< The ending aoa */
    double m_AlphaInc;          /**< The aoa increment */
    double m_ClMin;             /**< The starting Cl coefficient  */
    double m_ClMax;             /**< The ending Cl coefficient */
    double m_ClInc;             /**< The Cl increment  */

    //	bool m_bOutput;             /**< true if the output should be displayed in the text widget */
    bool m_bAlpha;              /**< true if the analysis should be performed for a range of aoa rather than lift coefficient */
    bool m_bFromList;           /**< true if the analysis should be performed for a list of Re values rather than for a range */
    bool m_bFromZero;           /**< true if the iterations should start from aoa=0 rather than aoa=alpha_min */
    bool m_bInitBL;             /**< true if the boundary layer should be restored to the default value before each polar analysis */
    bool m_bCancel;             /**< true if the user has clicked the cancel button */
    bool m_bIsRunning;          /**< true until all the pairs of (foil, polar) have been calculated */

    XFLR5::enumPolarType m_PolarType;  /**< the type of analysis to perform */

    int m_nAnalysis;            /**< the number of analysis pairs to run */
    int m_nTaskStarted;         /**< the number of started tasks */
    int m_nTaskDone;            /**< the number of finished tasks */
    double m_Mach;              /**< the Mach number used if not from the list of Re numbers */
    double m_ACrit;             /**< the transition criterion used if not from the list of Re numbers */

    double m_XTop;            /**< the point of forced transition on the upper surface */
    double m_XBot;            /**< the point of forced transition on the lower surface */


    QList<FoilAnalysis *> m_AnalysisPair;  /**< the list of all analysis to be performed. Once performed, an analysis is removed from the list. */
    //	XFoilTask *m_pXFoilTask;           /**< the task for a thread */

    QFile *m_pXFile;                   /**< a pointer to the output log file */

    Foil *m_pCurFoil;                  /**< a pointer to the current Foil */

    QStringList m_FoilList;            /**< the list of foils to analyze */

    QTimer *m_pTimer;
};

#endif // BATCHTHREADDLG_H
