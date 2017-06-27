/****************************************************************************

	FlapDlg class
	Copyright (C) 2004-2009 Andre Deperrois adeperrois@xflr5.com

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

#ifndef FLAPDLG_H
#define FLAPDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <objects2d/Foil.h>
#include <misc/DoubleEdit.h>


class FlapDlg : public QDialog
{
	Q_OBJECT

public:
	FlapDlg(QWidget *pParent);

	friend class QAFoil;
	friend class QXDirect;


private:
	void EnableTEFlap(bool bEnable);
	void EnableLEFlap(bool bEnable);
	void ReadParams();
	void SetupLayout();

	DoubleEdit	*m_pctrlLEYHinge;
	DoubleEdit	*m_pctrlLEXHinge;
	DoubleEdit	*m_pctrlLEFlapAngle;
	DoubleEdit	*m_pctrlTEYHinge;
	DoubleEdit	*m_pctrlTEXHinge;
	DoubleEdit	*m_pctrlTEFlapAngle;

	QCheckBox *m_pctrlLEFlapCheck;
	QCheckBox *m_pctrlTEFlapCheck;
	QPushButton	*OKButton, *CancelButton, *ApplyButton;

protected:
	void keyPressEvent(QKeyEvent *event);
	void initDialog();

private slots:
	void OnApply();
	void OnLEFlapCheck(int);
	void OnTEFlapCheck(int);
	void OnChanged();
	virtual void OnOK();

private:
	bool m_bTEFlap;
	bool m_bLEFlap;
	bool m_bApplied;
	bool m_bModified;

	double m_LEXHinge, m_LEYHinge, m_LEFlapAngle;
	double m_TEXHinge, m_TEYHinge, m_TEFlapAngle;
	Foil *m_pMemFoil, *m_pBufferFoil;

	void *m_pXFoil;

	QWidget *m_pParent;

};

#endif // FLAPDLG_H
