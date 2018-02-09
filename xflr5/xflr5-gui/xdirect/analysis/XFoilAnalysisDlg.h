/****************************************************************************

    XFoilAnalysisDlg Class
	Copyright (C) 2008 Andre Deperrois adeperrois@xflr5.com

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
 * This file implements the interface for the analysis of the active Foil and Polar objects in QXDirect.
*/

#ifndef XFOILANALYSISDLG_H
#define XFOILANALYSISDLG_H

#include <QDialog>
#include <QEventLoop>

#include <QString>
#include <QFile>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QPoint>
#include <xdirect/analysis/XFoilTask.h>
#include <xdirect/analysis/xfoiltaskevent.h>

class GraphWidget;
class QGraph;

/**
* @class XFoilAnalysisDlg
* This class provides an interface to manage the analysis of the active Foil and Polar pair.
*/
class XFoilAnalysisDlg : public QDialog
{
	Q_OBJECT

	friend class MainFrame;
	friend class QXDirect;

public:
	XFoilAnalysisDlg(QWidget *pParent=NULL);
	~XFoilAnalysisDlg();

	void initDialog();

	void handleXFoilTaskEvent(const XFoilTaskEvent *event);
	void customEvent(QEvent * event);

private slots:
	void onCancelAnalysis();
	void onLogFile(bool bChecked);
	void onSkipPoint();
	void onProgress();

private:
	void showEvent(QShowEvent *event);

	void accept();
	void reject();

	void resetCurves();
	void setAlpha(double AlphaMin, double AlphaMax, double DeltaAlpha);
	void setCl(double ClMin, double ClMax, double DeltaCl);
	void setRe(double ReMin, double ReMax, double DeltaRe);
	void setFileHeader();
	void setupLayout();
	void analyze();


	//variables
	GraphWidget * m_pGraphWidget;
	QTextEdit *m_pctrlTextOutput;
	QPushButton* m_pctrlCancel, *m_pctrlSkip;
	QCheckBox* m_pctrlLogFile;

	static void* s_pXDirect;     /**< a void pointer to the instance of the QXDirect object >*/
	static QPoint s_Position;   /**< the position on the client area of he dialog's topleft corner */

	bool m_bAlpha;                 /**< true if the analysis should be performed for a range of aoa, false if for a range of licf coefficient.>*/
	bool m_bErrors;                /**< true if some points are unconverged. Used by the calling class to know if the window should be kept visible at the end of the analysis.>*/

	double m_ReMin, m_ReMax, m_ReDelta;  /**< The range of Re values to analyze>*/

	double m_AlphaMin, m_AlphaMax, m_AlphaDelta;  /**< The range of aoa for a Type 1/2/3 Polar >*/
	double m_ClMin, m_ClMax, m_ClDelta;           /**< The range of lift coefficient for a Type 1/2/3 Polar>*/

	QFile *m_pXFile;               /**< a pointer to the log file>*/
	QGraph *m_pRmsGraph;           /**< a pointer to the output graph >*/

	XFoilTask *m_pXFoilTask;       /**< A pointer to the instance of the XFoilTask associated to this analysis. >*/

};

#endif
