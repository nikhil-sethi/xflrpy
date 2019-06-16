/****************************************************************************

    Settings Class
    Copyright (C) 2018 Andre Deperrois

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

#include "displayoptions.h"
#include <globals/mainframe.h>
#include <miarex/Miarex.h>
#include <xdirect/XDirect.h>
#include <xinverse/XInverse.h>
#include <graph/graphdlg.h>
#include <misc/text/TextClrBtn.h>
#include "color/ColorButton.h"

#include <QApplication>
#include <QGroupBox>
#include <QColorDialog>
#include <QFontDialog>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QFontDatabase>
#include <QtDebug>

bool Settings::s_bStyleSheets = true;
QString Settings::s_StyleName;
QString Settings::s_StyleSheetName;

QFont Settings::s_TextFont;
QFont Settings::s_TableFont;
QColor Settings::s_BackgroundColor = QColor(3, 9, 9);
QColor Settings::s_TextColor=QColor(221,221,221);
bool Settings::s_bReverseZoom = false;
Graph Settings::s_RefGraph;
XFLR5::enumTextFileType Settings::s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
QString Settings::s_LastDirName = QDir::homePath();
QString Settings::s_xmlDirName = QDir::homePath();
QStringList Settings::s_colorList;
QStringList Settings::s_colorNames;



SETTINGS::enumThemeType Settings::s_Theme = SETTINGS::DARKTHEME;


Settings::Settings(QWidget *pParent) : QWidget(pParent)
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
    m_StyleSheetDir.setPath("/usr/local/share/xflr5");
#endif

	setupLayout();

	connect(m_pctrlStyles, SIGNAL(activated(const QString &)),this, SLOT(onStyleChanged(const QString &)));

	connect(m_pctrlBackColor, SIGNAL(clicked()),this, SLOT(onBackgroundColor2d()));
	connect(m_pctrlGraphSettings, SIGNAL(clicked()),this, SLOT(onGraphSettings()));
	connect(m_pctrlTextClr, SIGNAL(clickedTB()),this, SLOT(onTextColor()));
	connect(m_pctrlTextFont, SIGNAL(clicked()),this, SLOT(onTextFont()));
	connect(m_pctrlTableFont, SIGNAL(clicked()),this, SLOT(onTableFont()));

	connect(m_pctrlReverseZoom, SIGNAL(clicked()), this, SLOT(onReverseZoom()));

}


void Settings::setDefaultFonts()
{
//	QFontDatabase fntDatabase;
//	QStringList fontFamiliyList = fntDatabase.families();
//	for (int i=0; i<fontFamiliyList.count(); i++) qDebug()<<fontFamiliyList.at(i);

	QFont generalFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
	QFont fixedFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	QFont titleFont(QFontDatabase::systemFont(QFontDatabase::TitleFont));
	QFont smallestReadableFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

	s_TextFont = QFont(fixedFont);
	s_TableFont = QFont(fixedFont);

//	qDebug()<<"done"<<s_TextFont.family()<<s_TextFont.pointSize();
//	qDebug()<<"______";
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
	}

	setLayout(pMainLayout);
}


void Settings::initWidget()
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


void Settings::setAllGraphSettings(Graph *pGraph)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	XDirect *pXDirect   = (XDirect*)pMainFrame->m_pXDirect;
	Miarex *pMiarex     = (Miarex*)pMainFrame->m_pMiarex;
	XInverse *pXInverse = (XInverse*)pMainFrame->m_pXInverse;

	pXDirect->CpGraph()->copySettings(pGraph);
	pXDirect->CpGraph()->setInverted(true);

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


	TextFont = QFontDialog::getFont(&bOK, s_TextFont, this);

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

	TableFont = QFontDialog::getFont(&bOK, s_TableFont, this);

	if (bOK)
	{
		s_TableFont = TableFont;
		m_pctrlTableFont->setText(s_TableFont.family());
		m_pctrlTableFont->setFont(s_TableFont);
	}
}



void Settings::saveSettings(QSettings &settings)
{
    settings.beginGroup("global_settings");
	{
        settings.setValue("LastDirName", s_LastDirName);
        settings.setValue("XMLDirName", s_xmlDirName);

        settings.setValue("BackgroundColor", s_BackgroundColor);
        settings.setValue("TextColor", s_TextColor);

        settings.setValue("TextFontFamily", s_TextFont.family());
        settings.setValue("TextFontPointSize", s_TextFont.pointSize());
        settings.setValue("TextFontItalic", s_TextFont.italic());
        settings.setValue("TextFontBold", s_TextFont.bold());

        settings.setValue("TableFontFamily", s_TableFont.family());
        settings.setValue("TableFontPointSize", s_TableFont.pointSize());
        settings.setValue("TableFontItalic", s_TableFont.italic());
        settings.setValue("TableFontBold", s_TableFont.bold());

        settings.setValue("ReverseZoom", s_bReverseZoom);

        if(isLightTheme()) settings.setValue("Theme",0);
        else               settings.setValue("Theme",1);

        s_RefGraph.saveSettings(settings);
	}
    settings.endGroup();
}


void Settings::loadSettings(QSettings &settings)
{
    settings.beginGroup("global_settings");
	{
        s_LastDirName = settings.value("LastDirName", QDir::homePath()).toString();
        s_xmlDirName = settings.value("XMLDirName", QDir::homePath()).toString();

        s_BackgroundColor = settings.value("BackgroundColor", QColor(5,11,13)).value<QColor>();

        s_TextColor = settings.value("TextColor", QColor(237,237,237)).value<QColor>();

        s_TextFont = QFont(settings.value("TextFontFamily", "Courier").toString());
        s_TextFont.setPointSize(settings.value("TextFontPointSize", 10).toInt());
        s_TextFont.setItalic(settings.value("TextFontItalic", false).toBool());
        s_TextFont.setBold(settings.value("TextFontBold", false).toBool());
		s_TextFont.setStyleStrategy(QFont::OpenGLCompatible);

        s_TableFont = QFont(settings.value("TableFontFamily", "Courier").toString());
        s_TableFont.setPointSize(settings.value("TableFontPointSize", 10).toInt());
        s_TableFont.setItalic(settings.value("TableFontItalic", false).toBool());
        s_TableFont.setBold(settings.value("TableFontBold", false).toBool());

        s_bReverseZoom   = settings.value("ReverseZoom", false).toBool();
        int iTheme = settings.value("Theme",1).toInt();
		if(iTheme==0) s_Theme = SETTINGS::LIGHTTHEME;
		else          s_Theme = SETTINGS::DARKTHEME;
        s_RefGraph.loadSettings(settings);
	}
    settings.endGroup();
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



void Settings::setColorList()
{
//	QStringList colorList = QColor::colorNames();
	s_colorList.clear();
	s_colorList.append("#000000");
	s_colorList.append("#0A0A0A");
	s_colorList.append("#404040");
	s_colorList.append("#696969");
	s_colorList.append("#808080");
	s_colorList.append("#C0C0C0");
	s_colorList.append("#FFFFFF");
	s_colorList.append("#F0FFF0");
	s_colorList.append("#E0FFFF");
	s_colorList.append("#F5DEB3");
	s_colorList.append("#FFE4C4");
	s_colorList.append("#FFDEAD");
	s_colorList.append("#C9C299");
	s_colorList.append("#D8BFD8");
	s_colorList.append("#DDA0DD");
	s_colorList.append("#FFB6C1");
	s_colorList.append("#FF69B4");
	s_colorList.append("#D87093");
	s_colorList.append("#EE82EE");
	s_colorList.append("#DA70D6");
	s_colorList.append("#FF00FF");
	s_colorList.append("#BA55D3");
	s_colorList.append("#9932CC");
	s_colorList.append("#8B008B");
	s_colorList.append("#800080");
	s_colorList.append("#663399");
	s_colorList.append("#8A2BE2");
	s_colorList.append("#483D8B");
	s_colorList.append("#6A5ACD");
	s_colorList.append("#008B8B");
	s_colorList.append("#40E0D0");
	s_colorList.append("#00FFFF");
	s_colorList.append("#AFEEEE");
	s_colorList.append("#B0E0E6");
	s_colorList.append("#87CEEB");
	s_colorList.append("#87CEFA");
	s_colorList.append("#00BFFF");
	s_colorList.append("#1E90FF");
	s_colorList.append("#6495ED");
	s_colorList.append("#4682B4");
	s_colorList.append("#4169E1");
	s_colorList.append("#0000FF");
	s_colorList.append("#0000CD");
	s_colorList.append("#00008B");
	s_colorList.append("#000080");
	s_colorList.append("#191970");
	s_colorList.append("#0A1A11");
	s_colorList.append("#08291A");
	s_colorList.append("#006400");
	s_colorList.append("#228B22");
	s_colorList.append("#2E8B57");
	s_colorList.append("#3CB371");
	s_colorList.append("#00FF00");
	s_colorList.append("#00FF7F");
	s_colorList.append("#90EE90");
	s_colorList.append("#20B2AA");
	s_colorList.append("#66CDAA");
	s_colorList.append("#98FB98");
	s_colorList.append("#ADFF2F");
	s_colorList.append("#9ACD32");
	s_colorList.append("#FFFF00");
	s_colorList.append("#FFD700");
	s_colorList.append("#F4A460");
	s_colorList.append("#FFA500");
	s_colorList.append("#F08080");
	s_colorList.append("#FF5F50");
	s_colorList.append("#CD5C5C");
	s_colorList.append("#C71585");
	s_colorList.append("#FF0000");
	s_colorList.append("#dc143c");
	s_colorList.append("#B22222");
	s_colorList.append("#A52A2A");
	s_colorList.append("#8B0000");
	s_colorList.append("#800000");
	s_colorList.append("#401010");

	// set corresponding names
	s_colorNames.clear();
	s_colorNames.append("black");
	s_colorNames.append("black_overlay");
	s_colorNames.append("darkgrey");
	s_colorNames.append("dimgrey");
	s_colorNames.append("grey");
	s_colorNames.append("silver");
	s_colorNames.append("white");
	s_colorNames.append("honeydew");
	s_colorNames.append("lightcyan");
	s_colorNames.append("wheat");
	s_colorNames.append("bisque");
	s_colorNames.append("navajowhite");
	s_colorNames.append("lemonchiffon3");
	s_colorNames.append("thistle");
	s_colorNames.append("plum");
	s_colorNames.append("lightpink");
	s_colorNames.append("hotpink");
	s_colorNames.append("palevioletred");
	s_colorNames.append("violet");
	s_colorNames.append("orchid");
	s_colorNames.append("magenta");
	s_colorNames.append("mediumorchid");
	s_colorNames.append("darkorchid");
	s_colorNames.append("darkmagenta");
	s_colorNames.append("purple");
	s_colorNames.append("rebeccapurple");
	s_colorNames.append("blueviolet");
	s_colorNames.append("darkslateblue");
	s_colorNames.append("slateblue");
	s_colorNames.append("darkcyan");
	s_colorNames.append("turquoise");
	s_colorNames.append("cyan");
	s_colorNames.append("paleturquoise");
	s_colorNames.append("powderblue");
	s_colorNames.append("skyblue");
	s_colorNames.append("lightskyblue");
	s_colorNames.append("deepskyblue");
	s_colorNames.append("dodgerblue");
	s_colorNames.append("cornflowerblue");
	s_colorNames.append("steelblue");
	s_colorNames.append("royalblue");
	s_colorNames.append("blue");
	s_colorNames.append("mediumblue");
	s_colorNames.append("darkblue");
	s_colorNames.append("navy");
	s_colorNames.append("midnightblue");
	s_colorNames.append("deepseagreen");
	s_colorNames.append("darkseagreen");
	s_colorNames.append("darkgreen");
	s_colorNames.append("forestgreen");
	s_colorNames.append("seagreen");
	s_colorNames.append("mediumseagreen");
	s_colorNames.append("green");
	s_colorNames.append("springgreen");
	s_colorNames.append("lightgreen");
	s_colorNames.append("lightseagreen");
	s_colorNames.append("mediumaquamarine");
	s_colorNames.append("palegreen");
	s_colorNames.append("greenyellow");
	s_colorNames.append("yellowgreen");
	s_colorNames.append("yellow");
	s_colorNames.append("gold");
	s_colorNames.append("sandybrown");
	s_colorNames.append("orange");
	s_colorNames.append("lightcoral");
	s_colorNames.append("coral");
	s_colorNames.append("indianred");
	s_colorNames.append("mediumvioletred");
	s_colorNames.append("red");
	s_colorNames.append("crimson");
	s_colorNames.append("firebrick");
	s_colorNames.append("brown");
	s_colorNames.append("darkred");
	s_colorNames.append("maroon");
	s_colorNames.append("darkmaroon");

}






