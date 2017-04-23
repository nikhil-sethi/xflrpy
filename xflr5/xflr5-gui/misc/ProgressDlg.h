/****************************************************************************

	ProgressDlg Class
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

#ifndef PROGRESSDLG_H
#define PROGRESSDLG_H

#include <QDialog>
#include <QProgressBar>
#include <QPushButton>

class ProgressDlg : public QDialog
{
	Q_OBJECT
	friend class QMiarex;
public:
    ProgressDlg(QWidget *pParent);
	void InitDialog(int min=0, int max=100);
	void SetValue(int value);
	bool IsCanceled();

private slots:
	void OnCancel();
private:
	void SetupLayout();

private :
	QPushButton *CancelButton;
	QProgressBar* m_pctrlProgress;
	int m_Min, m_Max;
	bool m_bCancel;
};

#endif // PROGRESSDLG_H
