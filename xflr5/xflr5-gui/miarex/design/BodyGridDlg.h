/****************************************************************************

	BodyGridDlg Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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


#ifndef BODYGRIDDLG_H
#define BODYGRIDDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

#include <misc/text/DoubleEdit.h>
#include <misc/line/LineBtn.h>


class BodyGridDlg : public QDialog
{
	Q_OBJECT

	friend class GL3dBodyDlg;
	friend class Body;
	friend class MainFrame;

public:
	BodyGridDlg(QWidget *pParent);
	static void loadSettings (QSettings *pSettings);
	static void saveSettings (QSettings *pSettings);

private slots:
	void onOK();
	void onGrid();
	void onGrid2();
	void onMinGrid();
	void onMinGrid2();
	void onLineStyle();
	void onLine2Style();
	void onMinLineStyle();
	void onMinLine2Style();

private:
	void setupLayout();
	void initDialog();
	void enableControls();

private:
	QCheckBox *m_pctrlScales;

	QCheckBox *m_pctrlGrid, *m_pctrlMinGrid, *m_pctrlGrid2, *m_pctrlMinGrid2;
	LineBtn *m_pctrlLine, *m_pctrlMinLine, *m_pctrlLine2, *m_pctrlMinLine2;
	DoubleEdit *m_pctrlUnit, *m_pctrlMinUnit, *m_pctrlUnit2, *m_pctrlMinUnit2;
	QLabel *m_pctrlLength1, *m_pctrlLength2, *m_pctrlLength3, *m_pctrlLength4;

	static bool s_bScale;

	static bool s_bGrid;
	static int s_Style, s_Width;
	static QColor s_Color;
	static double s_Unit;
	static bool s_bMinGrid;
	static int s_MinStyle, s_MinWidth;
	static QColor s_MinColor;
	static double s_MinorUnit;

	static bool s_bGrid2;
	static int s_Style2, s_Width2;
	static QColor s_Color2;
	static double s_Unit2;
	static bool s_bMinGrid2;
	static int s_MinStyle2, s_MinWidth2;
	static QColor s_MinColor2;
	static double s_MinorUnit2;
};

#endif // BODYGRIDDLG_H
