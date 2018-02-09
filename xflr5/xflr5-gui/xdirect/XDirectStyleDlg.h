/****************************************************************************

	QXDirectStyleDlg Class
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
#ifndef XDIRECTSTYLEDLG_H
#define XDIRECTSTYLEDLG_H

#include <QDialog>
#include <misc/line/LineBtn.h>


class XDirectStyleDlg : public QDialog
{
	Q_OBJECT
	friend class OpPointWidget;

public:
	XDirectStyleDlg(QWidget *pParent=NULL);

private slots:
	void onRestoreDefaults();
	void onNeutralStyle();
	void onBLStyle();
	void onPressureStyle();

private:
	void keyPressEvent(QKeyEvent *event);
	void setupLayout();

	void *m_pParent;
	LineBtn *m_pctrlBL, *m_pctrlPressure, *m_pctrlNeutral;
	QPushButton *OKButton;

	QColor m_crFoilColor, m_crBLColor, m_crPressureColor, m_crNeutralColor; //foil display parameters
	int m_iFoilStyle, m_iFoilWidth;
	int m_iBLStyle, m_iBLWidth;
	int m_iPressureStyle, m_iPressureWidth;
	int m_iNeutralStyle, m_iNeutralWidth;
};

#endif // XDIRECTSTYLEDLG_H
