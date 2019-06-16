/****************************************************************************

	ManageFoilsDlg Class
	Copyright (C) 2009 Andre Deperrois 

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


#ifndef MANAGEFOILSDLG_H
#define MANAGEFOILSDLG_H

#include <QDialog>

#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>


class FoilTableDelegate;
class Foil;

class ManageFoilsDlg : public QDialog
{
	Q_OBJECT
	friend class XDirect;
	friend class MainFrame;

public:
	ManageFoilsDlg(QWidget *pParent=NULL);
	~ManageFoilsDlg();

	void initDialog(QString FoilName);

private slots:
	void onDelete();
	void onRename();
	void onExport();
	void onFoilClicked(const QModelIndex& index);
	void onDoubleClickTable(const QModelIndex &index);

private:
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void fillFoilTable();
	void fillTableRow(int row);

	void setupLayout();

private:
	QPushButton *CloseButton;
	QPushButton *m_pctrlRename, *m_pctrlDelete, *m_pctrlSelect, *m_pctrlExport;
	QTableView *m_pctrlFoilTable;
	QStandardItemModel *m_pFoilModel;
	FoilTableDelegate *m_pFoilDelegate;

	int m_iSelection;
	Foil *m_pFoil;
	int  *m_precision;
	bool m_bChanged;
};

#endif // MANAGEFOILSDLG_H
