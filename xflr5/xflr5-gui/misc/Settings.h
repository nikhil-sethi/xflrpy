/****************************************************************************

	Settings Class
	Copyright (C) 2008-2017 Andre Deperrois adeperrois@xflr5.com

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
 


#ifndef XSETTINGS_H
#define XSETTINGS_H

#include <QDir>
#include <QDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <gui_enums.h>
#include "ColorButton.h"
#include "TextClrBtn.h"
#include <QGraph.h>

// first name space
namespace SETTINGS
{
	/** @enum The different types of polar available for 2D and 3D calculations. */
	typedef enum {LIGHTTHEME, DARKTHEME, CUSTOMTHEME} enumThemeType;
}


class Settings : public QDialog
{
	Q_OBJECT

	friend class MainFrame;
public:
	Settings(QWidget *pParent);
	void initDialog();
	static void loadSettings(QSettings *settings);
	static void saveSettings(QSettings *settings);

	static QColor &backgroundColor(){return s_BackgroundColor;}
	static QColor &textColor(){return s_TextColor;}
	static QString &lastDirName(){return s_LastDirName;}
	static QString &styleName(){return s_StyleName;}
	static QFont &textFont(){return s_TextFont;}
	static QFont &tableFont(){return s_TableFont;}
	static void setDefaultFonts();
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
	void setAllGraphSettings(QGraph *pGraph);

	ColorButton *m_pctrlBackColor;
	TextClrBtn *m_pctrlTextClr;
	QPushButton *m_pctrlTextFont, *m_pctrlTableFont;
	QPushButton *m_pctrlGraphSettings;
	QPushButton *OKButton;
	QCheckBox *m_pctrlReverseZoom;

	QComboBox *m_pctrlStyles;
	QPushButton *OK, *Cancel;

	QRadioButton *m_prbDark, *m_prbLight, *m_prbCustom;


	void *m_pMainFrame;
	QDir m_StyleSheetDir;
	QGraph m_MemGraph;
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
	static QGraph s_RefGraph;//Reference setttings
	static QString s_LastDirName, s_xmlDirName;

	static SETTINGS::enumThemeType s_Theme;
};


#endif // XSettings_H


