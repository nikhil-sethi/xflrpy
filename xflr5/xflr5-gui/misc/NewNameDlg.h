/****************************************************************************

	NewNameDlg Classes
        Copyright (C) 2010 Andre Deperrois adeperrois@xflr5.com

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

#ifndef NEWNAMEDLG_H
#define NEWNAMEDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>


class NewNameDlg : public QDialog
{
	friend class StabViewDlg;
	Q_OBJECT
public:
	NewNameDlg(QWidget *pParent=NULL);
	void InitDialog();
	void keyPressEvent(QKeyEvent *event);
	
private:
	void SetupLayout();
	
private slots:
	void OnOK();
	
private:
	QPushButton *OKButton;
	QLineEdit *m_pctrlName;
	QString m_OldName;
	QString m_NewName;
	
};

#endif // NEWNAMEDLG_H
