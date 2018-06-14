/****************************************************************************

	AFoilGridDlg Class
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

#ifndef AFOILTABLEDLG_H
#define AFOILTABLEDLG_H

#include <QDialog>
#include <QPushButton>
#include <QCheckBox>



class AFoilTableDlg : public QDialog
{
	Q_OBJECT

	friend class AFoil;

public:
    AFoilTableDlg(QWidget *pParent);
	void initDialog();

private slots:
	void onOK();


private:
	void setupLayout();
	void keyPressEvent(QKeyEvent *event);


	QPushButton *OKButton, *CancelButton;
	QCheckBox *m_pctrlFoilName;
	QCheckBox *m_pctrlThickness;
	QCheckBox *m_pctrlThicknessAt;
	QCheckBox *m_pctrlCamber;
	QCheckBox *m_pctrlCamberAt;
	QCheckBox *m_pctrlPoints;
	QCheckBox *m_pctrlTEFlapAngle;
	QCheckBox *m_pctrlTEXHinge;
	QCheckBox *m_pctrlTEYHinge;
	QCheckBox *m_pctrlLEFlapAngle;
	QCheckBox *m_pctrlLEXHinge;
	QCheckBox *m_pctrlLEYHinge;

	bool m_bFoilName, m_bPoints;
	bool m_bThickness, m_bThicknessAt, m_bCamber, m_bCamberAt;
	bool m_bTEFlapAngle, m_bTEXHinge, m_bTEYHinge, m_bLEFlapAngle, m_bLEXHinge, m_bLEYHinge;
};

#endif // AFOILTableDLG_H
