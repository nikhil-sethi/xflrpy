/****************************************************************************

    LLTAnalysisDlg Class
    Copyright (C) 2009-2016 Andre Deperrois

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

#ifndef LLTANALYSISDLG_H
#define LLTANALYSISDLG_H

#include <QDialog>
#include <QTimer>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QFile>
#include <QTextEdit>
#include <QPoint>



class Graph;
class GraphWidget;
class WPolar;
class Wing;
class LLTAnalysis;
class PlaneAnalysisTask;

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
    ~LLTAnalysisDlg();

    void initDialog();
    void setTask(PlaneAnalysisTask *pTask){m_pTheTask = pTask;}
    void deleteTask();
    void analyze();
    void cleanUp();
    static void setPosition(QPoint pos) {s_Position = pos;}

    Graph* iterGraph(){return m_pIterGraph;}

private slots:
    void onCancelAnalysis();
    void onProgress();
    void onLogFile();
    void onMessage(QString msg);

private:
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void customEvent(QEvent * event);

    bool AlphaLoop();
    bool QInfLoop();
    void setupLayout();
    void updateView();
    void updateOutput(QString &strong);

    PlaneAnalysisTask *m_pTheTask; /**< a pointer to the one and only instance of the PlaneAnalysisTask class */

    static QPoint s_Position;   /**< the position on the client area of the dialog's topleft corner */
    static QSize  s_WindowSize;
    static bool s_bWindowMaximized;

    bool m_bCancel;             /**< true if the user has cancelled the analysis */
    bool m_bFinished;           /**< true if the analysis is completed, false if it is running */
    Graph *m_pIterGraph;         /**< A pointer to the QGraph object where the progress of the iterations are displayed */
    QPoint m_LegendPlace;       /**< The position where the legend should be diplayed in the output graph */
    QRect m_ViscRect;           /**< The rectangle in the client area where the graph is displayed */

    QString m_strOut;

    QTimer m_Timer;

    //GUI widget variables
    QPushButton *m_pctrlCancel;
    GraphWidget * m_pGraphWidget;
    QTextEdit *m_pctrlTextOutput;
    QCheckBox * m_pctrlLogFile;
};

#endif
