/****************************************************************************

	WingSelDlg Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#ifndef WINGSELDLG_H
#define WINGSELDLG_H
#include <gui_params.h>
#include <objects3d/Plane.h>
#include <QDialog>
#include <QCheckBox>

class WingSelDlg : public QDialog
{
	Q_OBJECT

public:
	WingSelDlg(QWidget *pParent=NULL);

	void initDialog(Plane *pPlane);


private:
	void setupLayout();

private slots:
	void onOK();

private:
	Plane * m_pPlane;
	QCheckBox *m_pctrlWing[MAXWINGS];
	QCheckBox *m_pctrlBody;
};

#endif // WINGSELDLG_H
