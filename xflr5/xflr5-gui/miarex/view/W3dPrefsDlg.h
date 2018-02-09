/****************************************************************************

	W3dPrefsDlg Class
    Copyright (C) 2009-2016 Andre Deperrois XFLR5@yahoo.com

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


#ifndef W3DPREFSDLG_H
#define W3DPREFSDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QSettings>
#include <misc/line/LineBtn.h>
#include <misc/color/ColorButton.h>
#include <misc/text/IntEdit.h>


class W3dPrefsDlg : public QDialog
{
	Q_OBJECT

	friend class QMiarex;
	friend class MainFrame;
	friend class GL3dBodyDlg;

public:
    W3dPrefsDlg(QWidget *pParent);
	void initDialog();

	static int chordwiseRes() {return s_iChordwiseRes;}
	static int bodyAxialRes() {return s_iBodyAxialRes;}
	static int bodyHoopRes() {return s_iBodyHoopRes;}

private slots:
	void on3DAxis();
	void onOutline();
	void onTopTrans();
	void onBotTrans();
	void onXCP();
	void onMoments();
	void onIDrag();
	void onVDrag();
	void onDownwash();
	void onWakePanels();
	void onStreamLines();
	void onVLMMesh();
	void onShowWake();
	void onMasses();
    void onResetDefaults();
	void onOK();

private:
	void setupLayout();
	void readSettings();
	static void saveSettings(QSettings *pSettings);
	static void loadSettings(QSettings *pSettings);
	static void resetDefaults();

	LineBtn *m_pctrlAxis, *m_pctrlOutline, *m_pctrlVLMMesh, *m_pctrlTopTrans, *m_pctrlBotTrans;
	LineBtn *m_pctrlLift, *m_pctrlMoments, *m_pctrlInducedDrag, *m_pctrlViscousDrag, *m_pctrlDownwash;
	LineBtn *m_pctrlStreamLines, *m_pctrlWakePanels;

	ColorButton *m_pctrlMassColor;

	QCheckBox*m_pctrlAnimateTransitions, *m_pctrlAutoAdjustScale;
	IntEdit *m_pctrlChordwiseRes, *m_pctrlBodyAxialRes, *m_pctrlBodyHoopRes;

public:
	static double s_MassRadius;
	static QColor s_MassColor;

	static int s_3DAxisStyle, s_3DAxisWidth;
	static QColor s_3DAxisColor;
	static int s_VLMStyle, s_VLMWidth;
	static QColor s_VLMColor;
	static int s_OutlineStyle, s_OutlineWidth;
	static QColor s_OutlineColor;
	static int s_XCPStyle, s_XCPWidth;
	static QColor s_XCPColor;
	static int s_MomentStyle, s_MomentWidth;
	static QColor s_MomentColor;
	static int s_IDragStyle, s_IDragWidth;
	static QColor s_IDragColor;
	static int s_VDragStyle, s_VDragWidth;
	static QColor s_VDragColor;

	static int s_TopStyle, s_TopWidth;
	static QColor s_TopColor;
	static int s_BotStyle, s_BotWidth;
	static QColor s_BotColor;

	static int s_DownwashStyle, s_DownwashWidth;
	static QColor s_DownwashColor;
	static int s_StreamLinesStyle, s_StreamLinesWidth;
	static QColor s_StreamLinesColor;
	static int s_WakeStyle, s_WakeWidth;
	static QColor s_WakeColor;

	static int s_CpStyle, s_CpWidth;
	static QColor s_CpColor;

	static bool s_bWakePanels;

	static bool s_bAutoAdjustScale;

	static int s_iChordwiseRes,s_iBodyAxialRes, s_iBodyHoopRes;
	static bool s_bAnimateTransitions;
};

#endif // W3DPREFSDLG_H

