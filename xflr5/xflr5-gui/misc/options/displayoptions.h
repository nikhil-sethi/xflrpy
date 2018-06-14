/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/



#pragma once

#include <QDir>
#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSettings>
#include <graph/graph.h>
#include <graph/graph.h>

#include <globals/gui_enums.h>

class TextClrBtn;
class ColorButton;

// first name space
namespace SETTINGS
{
	/** @enum The different types of polar available for 2D and 3D calculations. */
	typedef enum {LIGHTTHEME, DARKTHEME, CUSTOMTHEME} enumThemeType;
}


class Settings : public QWidget
{
	Q_OBJECT

	friend class MainFrame;
public:
	Settings(QWidget *pParent);
	void initWidget();
	static void loadSettings(QSettings *settings);
	static void saveSettings(QSettings *settings);

	static QColor &backgroundColor(){return s_BackgroundColor;}
	static QColor &textColor(){return s_TextColor;}
	static QString &lastDirName(){return s_LastDirName;}
	static QString &styleName(){return s_StyleName;}
	static QFont &textFont(){return s_TextFont;}
	static QFont &tableFont(){return s_TableFont;}
	static void setDefaultFonts();
	static void setColorList();
	static bool isLightTheme(){return s_Theme==SETTINGS::LIGHTTHEME;}

private slots:
	void onStyleChanged(const QString &StyleName);
	void onBackgroundColor2d();
	void onGraphSettings();
	void onTextColor();
	void onTextFont();
	void onTableFont();
	void onReverseZoom();
	void onTheme();

private:
	void reject();
	void setupLayout();
	void setAllGraphSettings(Graph *pGraph);

	ColorButton *m_pctrlBackColor;
	TextClrBtn *m_pctrlTextClr;
	QPushButton *m_pctrlTextFont, *m_pctrlTableFont;
	QPushButton *m_pctrlGraphSettings;

	QCheckBox *m_pctrlReverseZoom;

	QComboBox *m_pctrlStyles;


	QRadioButton *m_prbDark, *m_prbLight, *m_prbCustom;


	void *m_pMainFrame;
	QDir m_StyleSheetDir;
	Graph m_MemGraph;
	bool m_bIsGraphModified;

public:
	//settings variables used throughout the program
	static QString s_StyleName, s_StyleSheetName;
	static QFont s_TextFont, s_TableFont;
	static QColor s_BackgroundColor, s_BackgroundColor3d;
	static QColor s_TextColor;
	static bool s_bStyleSheets;
	static bool s_bReverseZoom;
	static XFLR5::enumTextFileType s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
	static Graph s_RefGraph;//Reference setttings
	static QString s_LastDirName, s_xmlDirName;
	static QStringList s_colorList;
	static QStringList s_colorNames;
	static SETTINGS::enumThemeType s_Theme;
};


