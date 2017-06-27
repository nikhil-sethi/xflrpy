/****************************************************************************

	SaveOptionsDlg
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


#ifndef SAVEOPTIONSDLG_H
#define SAVEOPTIONSDLG_H

#include "IntEdit.h"

#include <QDialog>
#include <QCheckBox>


class SaveOptionsDlg : public QDialog
{
	Q_OBJECT
	friend class MainFrame;

public:
    SaveOptionsDlg(QWidget *pParent);
	void initDialog(bool bAutoLoadLast=false, bool bOpps=false, bool bWOpps = true, bool bAutoSave=true, int saveInterval=10);

private slots:
	void onOK();

private:
	void setupLayout();
	void readParams();

	IntEdit *m_pctrlInterval;
	bool m_bOpps, m_bWOpps, m_bAutoSave, m_bAutoLoadLast;
	int m_SaveInterval;
	QCheckBox *m_pctrlOpps, *m_pctrlWOpps;
	QCheckBox *m_pctrlAutoSave, *m_pctrlAutoLoadLast;
};

#endif // SAVEOPTIONSDLG_H
