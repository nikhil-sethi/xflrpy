/****************************************************************************

	TwoDPanelDlg Class
	Copyright (C) 2008-2016 Andre Deperrois 

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


#ifndef TWODPANELDLG_H
#define TWODPANELDLG_H

#include <QDialog>
#include <QPushButton>

#include <misc/text/IntEdit.h>
#include <misc/text/DoubleEdit.h>


class TwoDPanelDlg : public QDialog
{
	Q_OBJECT
	friend class AFoil;
	friend class XDirect;

private slots:
	void onApply();
	void onOK();
	void onChanged();

public:
	TwoDPanelDlg(QWidget *pParent);

	static void *s_pXFoil;

	void initDialog();

private:
	void keyPressEvent(QKeyEvent *event);
	void setupLayout();
	void readParams();

	QPushButton *OKButton, *CancelButton, *ApplyButton;

	IntEdit  *m_pctrlNPanels;
	DoubleEdit *m_pctrlCVpar,  *m_pctrlCTErat, *m_pctrlCTRrat;
	DoubleEdit *m_pctrlXsRef1, *m_pctrlXsRef2, *m_pctrlXpRef1, *m_pctrlXpRef2;

	bool m_bApplied;
	bool m_bModified;

	int npan;
	double cvpar;
	double cterat;
	double ctrrat;
	double xsref1;
	double xsref2;
	double xpref1;
	double xpref2;

	void *m_pBufferFoil;
	void *m_pMemFoil;

	QWidget *m_pParent;
};

#endif // TWODPANELDLG_H

