/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>

#include <objects3d/WPolar.h>
#include <objects3d/Plane.h>
#include <miarex/design/EditObjectDelegate.h>


class EditPolarDefDlg : public QDialog
{
	Q_OBJECT

	friend class Wing;
	friend class Plane;
	friend class QMiarex;
	friend class WPolar;

private slots:
	void onOK();
	void onItemChanged();

public:
	EditPolarDefDlg(QWidget *pParent=NULL);

	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void initDialog(Plane *pPlane, WPolar *pWPolar);
	void setupLayout();
	void showWPolar();
	void fillInertiaData(QList<QStandardItem*> inertiaFolder);
	void fillControlFields(QList<QStandardItem *> stabControlFolder);
	void readData();
	void readViewLevel(QModelIndex indexLevel);
	void readControlFields(QModelIndex indexLevel);

	QList<QStandardItem *> prepareRow(const QString &object, const QString &field="", const QString &value="",  const QString &unit="");
	QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
	QList<QStandardItem *> prepareIntRow(const QString &object, const QString &field, const int &value);
	QList<QStandardItem *> prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit);


	static QPoint s_Position;   /**< the position on the client area of the dialog's topleft corner */
	static QSize s_Size;	    /**< the window size in the client area */

private:
	WPolar * m_pWPolar;
	Plane * m_pPlane;
	QTreeView * m_pStruct;
	QStandardItemModel *m_pModel;
	EditObjectDelegate *m_pDelegate;


	QPushButton *pOKButton, *pCancelButton;

};

