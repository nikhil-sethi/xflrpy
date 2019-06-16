/****************************************************************************

	InertiaDlg Class
    Copyright (C) 2009-2019 Andre Deperrois

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

#ifndef INERTIADLG_H
#define INERTIADLG_H

#include <QDialog>
#include <QLabel>
#include <QStackedWidget>
#include <QRadioButton>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>


#include <objects/objects3d/vector3d.h>

class FloatEditDelegate;
class DoubleEdit;
class Plane;
class Wing;
class Body;
class PointMass;

class InertiaDlg : public QDialog
{
	Q_OBJECT
	friend class GL3dWingDlg;
	friend class GL3dBodyDlg;
	friend class EditBodyDlg;
	friend class PlaneDlg;
	friend class Miarex;
	friend class MainFrame;

public:
    InertiaDlg(QWidget *pParent);
	~InertiaDlg();

	void initDialog();

private slots:
	void onOK();
	void onBodyInertia();
	void onWingInertia();
	void onWing2Inertia();
	void onStabInertia();
	void onFinInertia();
	void onCellChanged(QWidget *);
	void onExportToAVL();
	void onInsertMassRow();
	void onDeleteMassRow();
	void onVolumeMass();
    void onButton(QAbstractButton *pButton);

private:
	void contextMenuEvent(QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *pEvent);
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);

	void fillMassModel();
	void clearPointMasses();
	void computeInertia();
	void computeBodyAxisInertia();
	void setupLayout();
	void readData();

	//layout widget variables
	QStackedWidget *m_pctrlTopStack;
	QPushButton *m_pctrlWingInertia, *m_pctrlWing2Inertia, *m_pctrlStabInertia, *m_pctrlFinInertia, *m_pctrlBodyInertia;

	QLabel *m_pctrlMassUnit, *m_pctrlMassUnit2;
	QLabel *m_pctrlLengthUnit10, *m_pctrlLengthUnit11, *m_pctrlLengthUnit12;
	QLabel *m_pctrlLengthUnit20, *m_pctrlLengthUnit21, *m_pctrlLengthUnit22;
	QLabel *m_pctrlVolumeMassLabel, *m_pctrlTotalMassLabel;
	QLabel *m_pctrlInertiaUnit1, *m_pctrlInertiaUnit2, *m_pctrlInertiaUnit3, *m_pctrlInertiaUnit4;
	QLabel *m_pctrlInertiaUnit10, *m_pctrlInertiaUnit20, *m_pctrlInertiaUnit30, *m_pctrlInertiaUnit40;
	QTableView *m_pctrlMassTable;
	QStandardItemModel *m_pMassModel;
	FloatEditDelegate *m_pFloatDelegate;
	DoubleEdit *m_pctrlCoGIxx, *m_pctrlCoGIyy, *m_pctrlCoGIzz, *m_pctrlCoGIxz;
	DoubleEdit *m_pctrlXCoG,*m_pctrlYCoG,*m_pctrlZCoG;
	DoubleEdit *m_pctrlVolumeMass;

	DoubleEdit *m_pctrlTotalIxx, *m_pctrlTotalIyy, *m_pctrlTotalIzz, *m_pctrlTotalIxz;
	DoubleEdit *m_pctrlXTotalCoG,*m_pctrlYTotalCoG,*m_pctrlZTotalCoG;
	DoubleEdit *m_pctrlTotalMass;

    QPushButton *m_pctrlExportToAVL;
    QDialogButtonBox *m_pButtonBox;

	QMenu *m_pContextMenu;
	QAction *m_pInsertMassRow, *m_pDeleteMassRow;

	int  *m_precision;

	//member variables
	Body *m_pBody;
	Wing *m_pWing;
	Plane *m_pPlane;

	double m_VolumeMass;
	Vector3d m_VolumeCoG;

	double m_CoGIxx, m_CoGIyy, m_CoGIzz, m_CoGIxz;

	QList<PointMass*> m_PointMass;

	bool m_bChanged;


public:
    static QByteArray s_Geometry;
};

#endif // INERTIADLG_H
