/****************************************************************************

    BatchThreadDlg Class
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


/** @file
 * This file implements the multi-threaded batch foil analysis
*/


#include "batchabstractdlg.h"
#include <xflcore/core_enums.h>


/**
 * @brief This class implements an interface to perform a multi-threaded batch foil analysis.
 */
class BatchThreadDlg : public BatchAbstractDlg
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;
    friend class XFoilTask;

    public:
        BatchThreadDlg(QWidget *pParent=nullptr);
        ~BatchThreadDlg();

    private:
        void cleanUp() override;
        void customEvent(QEvent *pEvent) override;
        void handleXFoilTaskEvent(const XFoilTaskEvent *pEvent);
        void setupLayout();
        void startAnalysis();
        void startThread();
        void updateOutput(const QString &str);

    private slots:
        void onAnalyze() override;
        void onTimerEvent();

    private:
        int m_nTaskStarted;         /**< the number of started tasks */
        int m_nTaskDone;            /**< the number of finished tasks */
        int m_nAnalysis;            /**< the number of analysis pairs to run */

        QTimer *m_pTimer;

        QVector<FoilAnalysis *> m_AnalysisPair;  /**< the list of all analysis to be performed. Once performed, an analysis is removed from the list. */

};

