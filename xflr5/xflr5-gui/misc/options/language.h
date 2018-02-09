/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QStringList>


class TranslatorDlg : public QWidget
{
	Q_OBJECT

	friend class MainFrame;
public:
    TranslatorDlg(QWidget *pParent);

public slots:
	void onOK();
	void onLanguageSelected(QListWidgetItem *pItem);

private:
	void setupLayout();
	void initWidget();
	QStringList findQmFiles();
	QString languageName(const QString &qmFile);

	QListWidget *m_pctrlLanguageList;
	QMap<QString, QString> qmFileForLanguage;

	bool m_bChanged;

};

