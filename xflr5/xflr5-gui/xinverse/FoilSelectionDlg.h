/****************************************************************************

	FoilSelectionDlg Classes
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


#ifndef FOILSELECTIONDLG_H
#define FOILSELECTIONDLG_H

#include <QDialog>
#include <QListWidget>
#include <QList>
#include <QStringList>

class Foil;

class FoilSelectionDlg : public QDialog
{
	Q_OBJECT
	friend class XInverse;
	friend class BatchDlg;
	friend class BatchThreadDlg;

public:
    FoilSelectionDlg(QWidget *pParent);

private slots:
	void onOK();
	void onSelChangeList(QListWidgetItem *);
	void onDoubleClickList(QListWidgetItem *);
	void onSelectAll();

private:
	void setupLayout();
	void initDialog();

	QListWidget *m_pctrlNameList;
	QString m_FoilName;
	QStringList m_FoilList;
	QList <Foil*> *m_poaFoil;
};

#endif // FOILSELECTIONDLG_H
