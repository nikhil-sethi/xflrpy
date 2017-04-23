/****************************************************************************

	DisplaySettingsDlg Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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

#include "Settings.h"
#include <mainframe.h>
#include <miarex/Miarex.h>
#include <xdirect/XDirect.h>
#include <xinverse/XInverse.h>
#include <graph/GraphDlg.h>
#include <QApplication>
#include <QGroupBox>
#include <QColorDialog>
#include <QFontDialog>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QtDebug>

bool Settings::s_bStyleSheets = true;
QString Settings::s_StyleName;
QString Settings::s_StyleSheetName;

QFont Settings::s_TextFont;
QFont Settings::s_TableFont;
QColor Settings::s_BackgroundColor = QColor(3, 9, 9);
QColor Settings::s_TextColor=QColor(221,221,221);
bool Settings::s_bReverseZoom = false;
QGraph Settings::s_RefGraph;
XFLR5::enumTextFileType Settings::s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
QString Settings::s_LastDirName = QDir::homePath();
QString Settings::s_xmlDirName = QDir::homePath();

SETTINGS::enumThemeType Settings::s_Theme = SETTINGS::DARKTHEME;


Settings::Settings(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("General Display Settings"));

	s_RefGraph.setGraphName("Reference_Graph");

	m_pMainFrame = pParent;
	m_bIsGraphModified = false;


#ifdef Q_OS_MAC
	m_StyleSheetDir.setPath(qApp->applicationDirPath());
#endif
#ifdef Q_OS_WIN
	m_StyleSheetDir.setPath(qApp->applicationDirPath());
#endif
#ifdef Q_OS_LINUX
	m_StyleSheetDir.setPath("/usr/share/xflr5");
#endif

	setupLayout();

	connect(m_pctrlStyles, SIGNAL(activated(const QString &)),this, SLOT(onStyleChanged(const QString &)));

	connect(m_pctrlBackColor, SIGNAL(clicked()),this, SLOT(onBackgroundColor2d()));
	connect(m_pctrlGraphSettings, SIGNAL(clicked()),this, SLOT(onGraphSettings()));
	connect(m_pctrlTextClr, SIGNAL(clickedTB()),this, SLOT(onTextColor()));
	connect(m_pctrlTextFont, SIGNAL(clicked()),this, SLOT(onTextFont()));
	connect(m_pctrlTableFont, SIGNAL(clicked()),this, SLOT(onTableFont()));

	connect(m_pctrlReverseZoom, SIGNAL(clicked()), this, SLOT(onReverseZoom()));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
}


void Settings::setupLayout()
{
	m_pctrlStyles = new QComboBox;

	QRegExp regExp("Q(.*)Style");
	QString defaultStyle = QApplication::style()->metaObject()->className();
	if (defaultStyle == QLatin1String("QMacStyle"))
		defaultStyle = QLatin1String("Macintosh (Aqua)");
	else if (defaultStyle == QLatin1String("OxygenStyle"))
		defaultStyle = QLatin1String("Oxygen");
	else if (regExp.exactMatch(defaultStyle))
		defaultStyle = regExp.cap(1);

	m_pctrlStyles->addItems(QStyleFactory::keys());
	m_pctrlStyles->setCurrentIndex(m_pctrlStyles->findText(defaultStyle));

	// add custom style sheets
	QString fileName = "*.qss";
	QStringList filesList = MainFrame::s_StylesheetDir.entryList(QStringList(fileName), QDir::Files | QDir::NoSymLinks);
	for(int is=0; is<filesList.count(); is++)
	{
		QString styleSheetName = filesList.at(is);
		int len = styleSheetName.length();
		styleSheetName = styleSheetName.left(len-4);
		m_pctrlStyles->addItem(styleSheetName);
	}

	QGroupBox *pWidgetStyleBox = new QGroupBox(tr("Widget Style"));
	{
		QHBoxLayout *pWidgetStyleLayout = new QHBoxLayout;
		pWidgetStyleLayout->addWidget(m_pctrlStyles);
		pWidgetStyleBox->setLayout(pWidgetStyleLayout);
	}

	QGroupBox * pThemeBox = new QGroupBox(tr("Display"));
	{
		QVBoxLayout *pThemeBoxLayout = new QVBoxLayout;
		{
			QHBoxLayout *pThemeLayout = new QHBoxLayout;
			{
				m_prbDark   = new QRadioButton(tr("Dark"));
				m_prbLight  = new QRadioButton(tr("Light"));
				m_prbCustom = new QRadioButton(tr("Custom"));
				pThemeLayout->addWidget(m_prbDark);
				pThemeLayout->addWidget(m_prbLight);
				pThemeLayout->addWidget(m_prbCustom);
				connect(m_prbDark, SIGNAL(clicked(bool)), this, SLOT(onTheme()));
				connect(m_prbLight, SIGNAL(clicked(bool)), this, SLOT(onTheme()));
				connect(m_prbCustom, SIGNAL(clicked(bool)), this, SLOT(onTheme()));
			}

			QGroupBox *pBackBox = new QGroupBox(tr("Background Colors"));
			{
				QHBoxLayout *pBackLayout = new QHBoxLayout;
				{
					m_pctrlBackColor      = new ColorButton(this);
					pBackLayout->addWidget(m_pctrlBackColor);
				}
				pBackBox->setLayout(pBackLayout);
			}

			QGroupBox *pFontBox = new QGroupBox(tr("Fonts"));
			{
				QGridLayout *pMainFontLayout = new QGridLayout;
				{
					QLabel *labMain = new QLabel(tr("Main display font"));
					m_pctrlTextFont = new QPushButton;
					m_pctrlTextClr  = new TextClrBtn(this);

					pMainFontLayout->addWidget(labMain,1,1);
					pMainFontLayout->addWidget(m_pctrlTextFont,1,2);
					pMainFontLayout->addWidget(m_pctrlTextClr,1,3);

					QLabel *labTable = new QLabel(tr("Table font"));
					m_pctrlTableFont = new QPushButton;

					pMainFontLayout->addWidget(labTable,2,1);
					pMainFontLayout->addWidget(m_pctrlTableFont,2,2);
				}
				pFontBox->setLayout(pMainFontLayout);
			}

			QGroupBox *pGraphBox = new QGroupBox(tr("Graph Settings"));
			{
				QVBoxLayout *pGraphLayout = new QVBoxLayout;
				{
					m_pctrlGraphSettings  = new QPushButton(tr("All Graph Settings"));
					m_pctrlGraphSettings->setMinimumWidth(120);
					pGraphLayout->addWidget(m_pctrlGraphSettings);
				}
				pGraphBox->setLayout(pGraphLayout);
			}

			pThemeBoxLayout->addLayout(pThemeLayout);
			pThemeBoxLayout->addWidget(pBackBox);
			pThemeBoxLayout->addWidget(pFontBox);
			pThemeBoxLayout->addWidget(pGraphBox);
		}
		pThemeBox->setLayout(pThemeBoxLayout);
	}



	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("Close"));
		OKButton->setAutoDefault(false);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(OKButton);
		pCommandButtons->addStretch(1);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		m_pctrlReverseZoom = new QCheckBox(tr("Reverse zoom direction using mouse wheel"));
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pWidgetStyleBox);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pThemeBox);
		pMainLayout->addWidget(m_pctrlReverseZoom);
		pMainLayout->addSpacing(20);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtons);
		pMainLayout->addStretch(1);
	}

	setLayout(pMainLayout);
}


void Settings::initDialog()
{
	m_prbCustom->setChecked(true);
	m_prbCustom->setEnabled(false);

	m_MemGraph.copySettings(&s_RefGraph);

	m_pctrlBackColor->setColor(s_BackgroundColor);

	m_pctrlTextClr->setTextColor(s_TextColor);
	m_pctrlTextClr->setBackgroundColor(s_BackgroundColor);

	QString textFontName = s_TextFont.family() + QString(" %1").arg(s_TextFont.pointSize());
	m_pctrlTextFont->setText(textFontName);
	m_pctrlTextFont->setFont(s_TextFont);
	m_pctrlTextClr->setFont(s_TextFont);
	m_pctrlTextClr->setText(QObject::tr("Text color"));

	QString tableFontName = s_TableFont.family() + QString(" %1").arg(s_TableFont.pointSize());
	m_pctrlTableFont->setText(tableFontName);
	m_pctrlTableFont->setFont(s_TableFont);

	if(m_pctrlStyles->findText(s_StyleName)>=0)
		m_pctrlStyles->setCurrentIndex(m_pctrlStyles->findText(s_StyleName));
	else if(m_pctrlStyles->findText(s_StyleSheetName)>=0)
		m_pctrlStyles->setCurrentIndex(m_pctrlStyles->findText(s_StyleSheetName));

	m_pctrlReverseZoom->setChecked(s_bReverseZoom);
}



void Settings::onStyleChanged(const QString &StyleName)
{
	//test for style sheet
	QString fileName = "*.qss";
	QStringList filesList = MainFrame::s_StylesheetDir.entryList(QStringList(fileName), QDir::Files | QDir::NoSymLinks);

	for(int is=0; is<filesList.count(); is++)
	{
		QString styleSheetName = filesList.at(is);
		int len = styleSheetName.length();
		styleSheetName = styleSheetName.left(len-4);
		if(styleSheetName.compare(StyleName)==0)
		{
			s_bStyleSheets = true;
			s_StyleSheetName = styleSheetName;
			s_StyleName.clear();
			QString styleSheet;
			MainFrame::readStyleSheet(styleSheetName, styleSheet);
			return;
		}
	}

	s_bStyleSheets = false;
	s_StyleSheetName.clear();
	qApp->setStyleSheet(styleSheet());
	qApp->setStyle(StyleName);
	s_StyleName = StyleName;
}


void Settings::onBackgroundColor2d()
{
	QColor Color = QColorDialog::getColor(s_BackgroundColor);
	if(Color.isValid()) s_BackgroundColor = Color;

	m_pctrlBackColor->setColor(s_BackgroundColor);

	m_pctrlTextClr->setBackgroundColor(s_BackgroundColor);
}




void Settings::reject()
{
	setAllGraphSettings(&m_MemGraph);

	QDialog::reject();
}



void Settings::onGraphSettings()
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	GraphDlg dlg(pMainFrame);

	dlg.setGraph(&s_RefGraph);

	dlg.setControls();

	if(dlg.exec() == QDialog::Accepted)
	{
		m_bIsGraphModified = true;
	}
}


void Settings::setAllGraphSettings(QGraph *pGraph)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QXDirect *pXDirect   = (QXDirect*)pMainFrame->m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)pMainFrame->m_pMiarex;
	QXInverse *pXInverse = (QXInverse*)pMainFrame->m_pXInverse;

	pXDirect->m_CpGraph.copySettings(pGraph);
	pXDirect->m_CpGraph.setInverted(true);

	for(int ig=0; ig<pXDirect->m_PlrGraph.count(); ig++)     pXDirect->PlrGraph(ig)->copySettings(pGraph);
	for(int ig=0; ig<pMiarex->m_WingGraph.count(); ig++)     pMiarex->m_WingGraph.at(ig)->copySettings(pGraph);
	for(int ig=0; ig<pMiarex->m_WPlrGraph.count(); ig++)     pMiarex->m_WPlrGraph.at(ig)->copySettings(pGraph);
	for(int ig=0; ig<pMiarex->m_StabPlrGraph.count(); ig++)  pMiarex->m_StabPlrGraph.at(ig)->copySettings(pGraph);
	for(int ig=0; ig<pMiarex->m_TimeGraph.count(); ig++)     pMiarex->m_TimeGraph.at(ig)->copySettings(pGraph);

	pMiarex->m_CpGraph.copySettings(pGraph);
	pMiarex->m_CpGraph.setInverted(true);

	pXInverse->m_QGraph.copySettings(pGraph);
	pXInverse->m_QGraph.setInverted(true);

}


void Settings::onTextColor()
{
	QColor Color = QColorDialog::getColor(s_TextColor);
	if(Color.isValid()) s_TextColor = Color;
	m_pctrlTextClr->setTextColor(s_TextColor);
}



void Settings::onTextFont()
{
	bool bOK;
	QFont TextFont;
	TextFont.setStyleHint(QFont::TypeWriter, QFont::OpenGLCompatible);

#ifdef Q_OS_MAC
	//20090604 Mac OS Native font dialog does not work well under QT 4.5.1
	//QFont font = QFontDialog::getFont(&ok, m_TextFont, this);
		//20110324 Works again under QT 4.6, though it loses focus if mouse is moved outside of it (QT bug?)
		//QFont font = QFontDialog::getFont(&bOK, m_TextFont, this, "",QFontDialog::DontUseNativeDialog);
       TextFont = QFontDialog::getFont(&bOK, s_TextFont, this);
#else
	TextFont = QFontDialog::getFont(&bOK, s_TextFont, this);
#endif

	if (bOK)
	{
		s_TextFont = TextFont;
		m_pctrlTextFont->setText(s_TextFont.family());
		m_pctrlTextFont->setFont(s_TextFont);
		m_pctrlTextClr->setFont(s_TextFont);
	}
}



void Settings::onTableFont()
{
	bool bOK;
	QFont TableFont;
//	TableFont.setStyleHint(QFont::TypeWriter, QFont::OpenGLCompatible);

#ifdef Q_OS_MAC
	//20090604 Mac OS Native font dialog does not work well under QT 4.5.1
	//QFont font = QFontDialog::getFont(&ok, m_TextFont, this);
		//20110324 Works again under QT 4.6, though it loses focus if mouse is moved outside of it (QT bug?)
		//QFont font = QFontDialog::getFont(&bOK, m_TextFont, this, "",QFontDialog::DontUseNativeDialog);
       TableFont = QFontDialog::getFont(&bOK, s_TableFont, this);
#else
	TableFont = QFontDialog::getFont(&bOK, s_TableFont, this);
#endif

	if (bOK)
	{
		s_TableFont = TableFont;
		m_pctrlTableFont->setText(s_TableFont.family());
		m_pctrlTableFont->setFont(s_TableFont);
	}
}



void Settings::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("global_settings");
	{
		pSettings->setValue("LastDirName", s_LastDirName);
		pSettings->setValue("XMLDirName", s_xmlDirName);

		pSettings->setValue("BackgroundColor", s_BackgroundColor);
		pSettings->setValue("TextColor", s_TextColor);

		pSettings->setValue("TextFontFamily", s_TextFont.family());
		pSettings->setValue("TextFontPointSize", s_TextFont.pointSize());
		pSettings->setValue("TextFontItalic", s_TextFont.italic());
		pSettings->setValue("TextFontBold", s_TextFont.bold());

		pSettings->setValue("TableFontFamily", s_TableFont.family());
		pSettings->setValue("TableFontPointSize", s_TableFont.pointSize());
		pSettings->setValue("TableFontItalic", s_TableFont.italic());
		pSettings->setValue("TableFontBold", s_TableFont.bold());

		pSettings->setValue("ReverseZoom", s_bReverseZoom);
		s_RefGraph.saveSettings(pSettings);
	}
	pSettings->endGroup();
}


void Settings::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("global_settings");
	{
		s_LastDirName = pSettings->value("LastDirName", QDir::homePath()).toString();
		s_xmlDirName = pSettings->value("XMLDirName", QDir::homePath()).toString();

		s_BackgroundColor = pSettings->value("BackgroundColor", QColor(5,11,13)).value<QColor>();

		s_TextColor = pSettings->value("TextColor", QColor(237,237,237)).value<QColor>();

		s_TextFont = QFont(pSettings->value("TextFontFamily", "Courier").toString());
		s_TextFont.setPointSize(pSettings->value("TextFontPointSize", 10).toInt());
		s_TextFont.setItalic(pSettings->value("TextFontItalic", false).toBool());
		s_TextFont.setBold(pSettings->value("TextFontBold", false).toBool());
		s_TextFont.setStyleStrategy(QFont::OpenGLCompatible);

		s_TableFont = QFont(pSettings->value("TableFontFamily", "Courier").toString());
		s_TableFont.setPointSize(pSettings->value("TableFontPointSize", 10).toInt());
		s_TableFont.setItalic(pSettings->value("TableFontItalic", false).toBool());
		s_TableFont.setBold(pSettings->value("TableFontBold", false).toBool());

		s_bReverseZoom   = pSettings->value("ReverseZoom", false).toBool();

		s_RefGraph.loadSettings(pSettings);
	}
	pSettings->endGroup();
}



void Settings::onReverseZoom()
{
	s_bReverseZoom = m_pctrlReverseZoom->isChecked();
}



void Settings::onTheme()
{
	if (m_prbDark->isChecked())
	{
		m_bIsGraphModified = true;
		s_Theme = SETTINGS::DARKTHEME;
		s_BackgroundColor = QColor(3, 9, 9);
		s_TextColor=QColor(221,221,221);
		s_RefGraph.setGraphDefaults(true);
	}
	else if(m_prbLight->isChecked())
	{
		m_bIsGraphModified = true;
		s_Theme = SETTINGS::LIGHTTHEME;
		s_BackgroundColor = QColor(241, 241, 241);
		s_TextColor=QColor(0,0,0);
		s_RefGraph.setGraphDefaults(false);
	}
	else
	{
		return;
	}
	m_pctrlBackColor->setColor(s_BackgroundColor);
	m_pctrlTextClr->setBackgroundColor(s_BackgroundColor);
	m_pctrlTextClr->setTextColor(s_TextColor);
}














