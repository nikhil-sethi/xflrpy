/****************************************************************************

	FoilCoordDlg Class
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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


#ifndef FOILCOORDDLG_H
#define FOILCOORDDLG_H

#include <QDialog>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <misc/text/FloatEditDelegate.h>
#include <objects2d/Foil.h>


class FoilCoordDlg : public QDialog
{
	Q_OBJECT

	friend class QXDirect;
	friend class QAFoil;

public:
	FoilCoordDlg(QWidget *pParent);
	~FoilCoordDlg();

	void initDialog();

private slots:
	void onDeletePoint();
	void onInsertPoint();
	void onRestore();
	void onApply();
	void onCellChanged(QWidget *);
	void onItemClicked(QModelIndex);


private:
	void fillList();
	void setSelection(int sel);
	void setupLayout();
	void readSectionData(int sel, double &X, double &Y);
	void keyPressEvent(QKeyEvent *event);
	void showEvent(QShowEvent *event);
	void resizeEvent(QResizeEvent *event);

private:
	QPushButton *m_pctrlInsertPoint, *m_pctrlDeletePoint, *m_pctrlRestore, *m_pctrlApply;
	QPushButton *OKButton, *CancelButton;

	QTableView *m_pctrlCoordTable;
	QStandardItemModel *m_pCoordModel;
	FloatEditDelegate *m_pFloatDelegate;

	Foil * m_pMemFoil;
	Foil * m_pBufferFoil;

	int *m_precision;

	bool m_bApplied, m_bModified;
	QWidget *m_pParent;
};

#endif // FOILCOORDDLG_H
