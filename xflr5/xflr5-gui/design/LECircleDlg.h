/****************************************************************************

	LECircleDlg Class
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


#ifndef LECIRCLEDLG_H
#define LECIRCLEDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <misc/text/DoubleEdit.h>


class LECircleDlg : public QDialog
{
	Q_OBJECT
	friend class QAFoil;


public:
    LECircleDlg(QWidget *pParent);

	void SetupLayout();
	void InitDialog();

private slots:
	void OnOK();


private:
	void keyPressEvent(QKeyEvent *event);


private:
	QCheckBox *m_pctrlShow;
	DoubleEdit *m_pctrlRadius;
	QPushButton *OKButton, *CancelButton;

	double m_Radius;
	bool m_bShowRadius;
	void* m_pAFoil;
};

#endif // LECIRCLEDLG_H
