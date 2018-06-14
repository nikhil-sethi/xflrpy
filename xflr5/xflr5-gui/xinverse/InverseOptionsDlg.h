/****************************************************************************

	InverseOptionsDlg  Classes
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

#ifndef INVERSEOPTIONSDLG_H
#define INVERSEOPTIONSDLG_H

#include <misc/line/LineBtn.h>
#include <QDialog>


class InverseOptionsDlg:public QDialog
{
	Q_OBJECT
	
	friend class XInverse;

public:
    InverseOptionsDlg(QWidget *pParent);

private slots:
	void onRefStyle();
	void onModStyle();
	void onSplineStyle();
	void onReflectedStyle();

private:
	void setupLayout();
	void initDialog();

	LineBtn *m_pctrlRefFoil, *m_pctrlModFoil, *m_pctrlSpline, *m_pctrlReflected;

	void * m_pXInverse;
};

#endif // INVERSEOPTIONSDLG_H
