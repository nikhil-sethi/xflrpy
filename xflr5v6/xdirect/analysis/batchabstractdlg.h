/****************************************************************************

    BatchThreadDlg Class
    Copyright (C) 2003-2016 André Deperrois

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

#include <QListWidget>
#include <QSettings>
#include <QDialog>
#include <QCheckBox>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QModelIndex>
#include <QSplitter>

#include <xflcore/core_enums.h>

class Foil;
class Polar;
class IntEdit;
class DoubleEdit;
class XFoilTask;
class XFoilTaskEvent;
struct FoilAnalysis;
class XDirect;
class CPTableView;
class ActionDelegate;
class ActionItemModel;

/**
 * @brief Abstract base class for BatchThreadDlg and BacthCtrlDlg
 */
class BatchAbstractDlg : public QDialog
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;
    friend class XFoilTask;

    public:
        BatchAbstractDlg(QWidget *pParent=nullptr);
        ~BatchAbstractDlg();

        virtual void initDialog();
        QSize sizeHint() const override {return QSize(1100,900);}

        static void initReList();
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void keyPressEvent(QKeyEvent  *pEvent) override;
        virtual void showEvent(QShowEvent *pEvent) override;
        virtual void hideEvent(QHideEvent *pEvent) override;
        void reject() override;
        void resizeEvent(QResizeEvent *pEvent) override;


        virtual void cleanUp();
        void connectBaseSignals();
        virtual void readParams();

        void makeCommonWidgets();
        void outputReList();
        void setFileHeader();
        void setPlrName(Polar *pNewPolar);
        void writeString(QString &strong);
        void fillReModel();
        void sortRe();

        void readFoils(QVector<Foil *> &foils);

    protected slots:
        virtual void onAnalyze() = 0;
        void onAcl();
        void onAdvancedSettings();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onInitBL(int);
        void onSpecChanged();
        void onUpdatePolarView();

        void onDelete();
        void onInsertBefore();
        void onInsertAfter();
        void onCellChanged(QModelIndex topLeft, QModelIndex botRight);
        void onReTableClicked(QModelIndex index);

    protected:
        QListWidget *m_plwNameList;

        QRadioButton *m_prbAlpha, *m_prbCl;

        DoubleEdit *m_pdeSpecMin, *m_pdeSpecMax, *m_pdeSpecDelta;
        DoubleEdit *m_pdeXTopTr, *m_pdeXBotTr;

        IntEdit *m_pieMaxThreads;
        QLabel *m_plabSpecVar;
        QLabel *m_plabMaType, *m_plabReType;
        QCheckBox *m_pchInitBL, *m_pchFromZero, *m_pchUpdatePolarView;

        QGroupBox *m_pRangeVarsGroupBox, *m_pTransVarsGroupBox;
        QFrame *m_pOptionsFrame;

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbAnalyze, *m_ppbAdvancedSettings;
        QPlainTextEdit *m_pteTextOutput;

        QSplitter *m_pVSplitter;

        CPTableView *m_pcptReTable;
        ActionItemModel *m_pReModel;
        ActionDelegate *m_pFloatDelegate;

        QAction *m_pInsertBeforeAct, *m_pInsertAfterAct, *m_pDeleteAct;

        bool m_bCancel;             /**< true if the user has clicked the cancel button */
        bool m_bIsRunning;          /**< true until all the pairs of (foil, polar) have been calculated */

        QFile *m_pXFile;                   /**< a pointer to the output log file */

        Foil *m_pFoil;                  /**< a pointer to the current Foil */

        static bool s_bAlpha;              /**< true if the analysis should be performed for a range of aoa rather than lift coefficient */
        static bool s_bFromZero;           /**< true if the iterations should start from aoa=0 rather than aoa=alpha_min */

        static QVector<double> s_ReList;        /**< the user-defined list of Re numbers, used for batch analysis */
        static QVector<double> s_MachList;      /**< the user-defined list of Mach numbers, used for batch analysis */
        static QVector<double> s_NCritList;     /**< the user-defined list of NCrit numbers, used for batch analysis */

        static double s_XTop;            /**< the point of forced transition on the upper surface */
        static double s_XBot;            /**< the point of forced transition on the lower surface */

        static xfl::enumPolarType s_PolarType;  /**< the type of analysis to perform */

        static double s_AlphaMin;          /**< The starting aoa */
        static double s_AlphaMax;          /**< The ending aoa */
        static double s_AlphaInc;          /**< The aoa increment */
        static double s_ClMin;             /**< The starting Cl coefficient  */
        static double s_ClMax;             /**< The ending Cl coefficient */
        static double s_ClInc;             /**< The Cl increment  */

        static bool s_bInitBL;             /**< true if the boundary layer should be restored to the default value before each polar analysis */

        static QByteArray s_Geometry;
        static XDirect* s_pXDirect;           /**< a void pointer to the unique instance of the QXDirect class */
        static bool s_bUpdatePolarView;    /**< true if the polar graphs should be updated during the analysis */
        static int s_nThreads;             /**< the number of available threads */

        static QByteArray s_VSplitterSizes;
};



