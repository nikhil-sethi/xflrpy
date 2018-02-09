/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>
#include <QLabel>

#include <misc/text/IntEdit.h>
#include <misc/text/TextClrBtn.h>
#include <misc/color/ColorButton.h>


class TranslatorDlg;
class SaveOptions;
class Units;
class Settings;

class PreferencesDlg : public QDialog
{
	friend class MainFrame;
	Q_OBJECT

public:
	PreferencesDlg(QWidget *pParent);

private:
	void setupLayout();

private slots:
	void onPage(int iRow);
	void onOK();

private:
	QListWidget *m_pTabWidget;
	QStackedWidget *m_pPageStack;

	SaveOptions *m_pSaveOptionsWidget;
	TranslatorDlg *m_pLanguageOptionsWidget;
	Units *m_pUnitsWidget;
	Settings *m_pDisplayOptionsWidget;

};


