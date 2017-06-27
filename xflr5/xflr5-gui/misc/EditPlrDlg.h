/****************************************************************************

	EditPlrDlg Class
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

#ifndef EDITPLRDLG_H
#define EDITPLRDLG_H

#include <QDialog>
#include <QPushButton>
//#include <QListWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <misc/FloatEditDelegate.h>

#include <objects2d/Polar.h>
#include <objects3d/WPolar.h>


class EditPlrDlg : public QDialog
{
	Q_OBJECT

	friend class QXDirect;
	friend class QMiarex;

public:
	EditPlrDlg(QWidget *pParent=NULL);
	~EditPlrDlg();


	void initDialog(void *pXDirect, Polar *pPolar, void *pMiarex, WPolar *pWPolar);

private slots:
	void onDeletePoint();
	void onDeleteAllPoints();


private:
	void setupLayout();
	void fillPolarData();
	void fillWPolarData();
	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent*event);
	void hideEvent(QHideEvent *event);
	void showEvent(QShowEvent *event);

private:
	QPushButton *m_pctrlDeletePoint, *m_pctrlDeleteAllPoints;
	QPushButton *OKButton, *CancelButton;

	Polar *m_pPolar;
	WPolar *m_pWPolar;

	QTableView *m_pctrlPointTable;
	QStandardItemModel *m_pPointModel;
	FloatEditDelegate *m_pFloatDelegate;

	void *m_pXDirect;
	void *m_pMiarex;

	int  *m_precision;

	static QSize  s_WindowSize;
	static bool s_bWindowMaximized;

public:
	static QPoint s_Position;
};

#endif // EDITPLRDLG_H
