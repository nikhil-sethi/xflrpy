/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QCheckBox>

class IntEdit;

class SaveOptions : public QWidget
{
	friend class MainFrame;
	Q_OBJECT
public:
	SaveOptions(QWidget *parent = NULL);

	void initWidget(bool bAutoLoadLast=false, bool bOpps=false, bool bWOpps = true, bool bAutoSave=true, int saveInterval=10);

public slots:
	void onOK();

private:
	void setupLayout();
	void readParams();

	bool m_bOpps, m_bWOpps, m_bAutoSave, m_bAutoLoadLast;
	int m_SaveInterval;
	IntEdit *m_pctrlInterval;
	QCheckBox *m_pctrlOpps, *m_pctrlWOpps;
	QCheckBox *m_pctrlAutoSave, *m_pctrlAutoLoadLast;
};

