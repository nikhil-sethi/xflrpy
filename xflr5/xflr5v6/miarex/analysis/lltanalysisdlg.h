/****************************************************************************

    LLTAnalysisDlg Class
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


/**
 *@file
 *
 * This file contains the LLTAnalysisDlg class, which is used to perform LLT analysis
 *
 */

#pragma once

#include <QDialog>
#include <QTimer>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QFile>
#include <QTextEdit>
#include <QPoint>



class Graph;
class GraphWt;
class WPolar;
class Wing;
class LLTAnalysis;
class PlaneTask;

/**
 *@class LLTAnalysisDlg
 *@brief The class is used to launch the LLT and to manage the progress of the analysis.
 
 It successively :
  - creates a single instance of the LLTAnalysis object,
  - initializes the data,
  - launches the analysis
  - displays the progress,
  - stores the results in the OpPoint and Polar objects
  - updates the display in the Miarex view.

 The LLTAnalysis class performs the calculation of a signle OpPoint. The loop over a sequence of aoa, Cl, or Re values
 are performed in the LLAnalysisDlg Class.
*/
class LLTAnalysisDlg : public QDialog
{
    Q_OBJECT

    friend class Wing;

    public:
        LLTAnalysisDlg(QWidget *pParent);
        ~LLTAnalysisDlg() override;

        QSize sizeHint() const override {return QSize(900,700);}
        void initDialog();
        void setTask(PlaneTask *pTask) {m_pTheTask = pTask;}
        void deleteTask();
        void analyze();

        Graph* iterGraph() const {return m_pIterGraph;}

    private slots:
        void onCancelAnalysis();
        void onProgress();
        void onLogFile();
        void onMessage(QString msg);
        void onTaskFinished();

    signals:
        void lltAnalysisFinished() const;


    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent * pEvent) override;

        bool AlphaLoop();
        bool QInfLoop();
        void setupLayout();
        void updateView();
        void updateOutput(QString &strong);

    private:
        PlaneTask *m_pTheTask; /**< a pointer to the one and only instance of the PlaneTask class */

        bool m_bCancel;             /**< true if the user has cancelled the analysis */
        bool m_bFinished;           /**< true if the analysis is completed, false if it is running */
        Graph *m_pIterGraph;         /**< A pointer to the QGraph object where the progress of the iterations are displayed */
        QPoint m_LegendPlace;       /**< The position where the legend should be diplayed in the output graph */
        QRect m_ViscRect;           /**< The rectangle in the client area where the graph is displayed */

        QString m_strOut;

        QTimer m_Timer;

        //GUI widget variables
        QPushButton *m_ppbCancel;
        GraphWt * m_pGraphWidget;
        QTextEdit *m_pteTextOutput;
        QCheckBox * m_pchLogFile;

        static QByteArray s_Geometry;
};


