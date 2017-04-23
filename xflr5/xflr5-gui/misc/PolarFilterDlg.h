/****************************************************************************

	PolarFilterDlg Class
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


#ifndef POLARFILTERDLG_H
#define POLARFILTERDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QKeyEvent>

class PolarFilterDlg : public QDialog
{
	Q_OBJECT

	friend class QMiarex;
	friend class QXDirect;

public:
	PolarFilterDlg(QWidget *pParent=NULL);
	void InitDialog();

private slots:
	void OnOK();

private:
	void keyPressEvent(QKeyEvent *event);
	void SetupLayout();

	QPushButton *OKButton;
	QCheckBox *m_pctrlType1, *m_pctrlType2, *m_pctrlType3, *m_pctrlType4, *m_pctrlType7;
	bool m_bType1, m_bType2, m_bType3, m_bType4, m_bType7;
	bool m_bMiarex;
};


#endif // POLARFILTERDLG_H









