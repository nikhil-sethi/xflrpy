/****************************************************************************

	STLExportDialog
	Copyright (C) 2016 Andre Deperrois adeperrois@xflr5.com

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

#ifndef STLEXPORTDIALOG_H
#define STLEXPORTDIALOG_H
#include <QDialog>
#include <QCheckBox>
#include <misc/text/IntEdit.h>
#include <QRadioButton>
#include <QLabel>

class STLExportDialog : public QDialog
{
	Q_OBJECT
public:
	STLExportDialog();
	void initDialog();

	void accept();

	static bool s_bBinary;
	static int s_iChordPanels;
	static int s_iSpanPanels;
	static int s_iObject;

private:
	void setupLayout();
	void readParams();
	void setLabels();

private slots:
	void onObjectSelection();

private:
	IntEdit *m_pctrlChordPanels, *m_pctrlSpanPanels;
	QRadioButton *m_pctrlBinary, *m_pctrlASCII;
	QRadioButton *m_prb[5];

	QLabel *m_pctrlChordLabel, *m_pctrlSpanLabel;
};

#endif // STLEXPORTDIALOG_H
