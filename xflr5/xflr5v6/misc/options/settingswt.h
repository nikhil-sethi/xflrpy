/****************************************************************************

    Settings Class
    Copyright (C) 2018 André Deperrois

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



#pragma once

#include <QDir>
#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSettings>

#include <xflcore/core_enums.h>
#include <xflgraph/graph.h>

class MainFrame;
class TextClrBtn;
class ColorBtn;



class Settings : public QWidget
{
    Q_OBJECT

    friend class MainFrame;

    public:
        Settings(QWidget *pParent);
        void initWidget();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setStyleName(QString const &name){s_StyleName=name;}
        static QString const &styleName()  {return s_StyleName;}

        static void setColorList();

        static void setStyleSheetOverride(bool bSheet){s_bStyleSheet=bSheet;}
        static bool bStyleSheeOverride() {return s_bStyleSheet;}


    private slots:
        void onAlignChildrenStyle();
        void onBackgroundColor2d();
        void onGraphSettings();
        void onReverseZoom();
        void onStyleChanged(const QString &StyleName);
        void onTextColor();
        void onTableFont();
        void onTextFont();
        void onTreeFont();
        void onTheme();
        void onStyleSheet(bool bSheet);

    private:
        void setupLayout();
        void setButtonFonts();

    private:
        ColorBtn *m_pcbBackColor;
        TextClrBtn *m_ptcbTextClr;
        QPushButton *m_ppbTextFont, *m_ppbTableFont, *m_ppbTreeFont;
        QPushButton *m_ppbGraphSettings;

        QCheckBox *m_pchReverseZoom;
        QCheckBox *m_pchAlignChildrenStyle;

        QComboBox *m_pcbStyles;
        QCheckBox *m_pchStyleSheetOverride;

        QRadioButton *m_prbDark, *m_prbLight, *m_prbCustom;

        QDir m_StyleSheetDir;
        Graph m_MemGraph;
        bool m_bIsGraphModified;

    public:
        //settings variables used throughout the program
        static QString s_StyleName, s_StyleSheetName;

        static bool s_bStyleSheet;

        static xfl::enumTextFileType s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
        static Graph s_RefGraph;//Reference setttings
        static QStringList s_colorList;
        static QStringList s_colorNames;
        static bool s_bDontUseNativeDlg;
};


