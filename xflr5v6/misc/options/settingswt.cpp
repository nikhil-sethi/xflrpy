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

#include <QApplication>
#include <QGroupBox>
#include <QColorDialog>
#include <QFontDialog>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QFontDatabase>
#include <QHBoxLayout>

#include "settingswt.h"
#include <globals/mainframe.h>
#include <miarex/miarex.h>
#include <xdirect/xdirect.h>
#include <xflwidgets/view/section2dwt.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflgraph/controls/graphdlg.h>
#include <xflwidgets/color/textclrbtn.h>
#include <xflwidgets/color/colorbtn.h>

bool Settings::s_bDontUseNativeDlg(true);
bool Settings::s_bStyleSheet(false);
QString Settings::s_StyleName;
QString Settings::s_StyleSheetName;


Graph Settings::s_RefGraph;
xfl::enumTextFileType Settings::s_ExportFileType;
QStringList Settings::s_colorList;
QStringList Settings::s_colorNames;


Settings::Settings(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle(tr("General Display Settings"));

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

    connect(m_pcbStyles,             SIGNAL(activated(QString)),         SLOT(onStyleChanged(QString)));

    connect(m_pcbBackColor,          SIGNAL(clicked()),                  SLOT(onBackgroundColor2d()));
    connect(m_ppbGraphSettings,      SIGNAL(clicked()),                  SLOT(onGraphSettings()));
    connect(m_ptcbTextClr,           SIGNAL(clickedTB()),                SLOT(onTextColor()));
    connect(m_ppbTextFont,           SIGNAL(clicked()),                  SLOT(onTextFont()));
    connect(m_ppbTableFont,          SIGNAL(clicked()),                  SLOT(onTableFont()));
    connect(m_ppbTreeFont,           SIGNAL(clicked()),                  SLOT(onTreeFont()));

    connect(m_pchReverseZoom,        SIGNAL(clicked()),                  SLOT(onReverseZoom()));
    connect(m_pchAlignChildrenStyle, SIGNAL(clicked()),                  SLOT(onAlignChildrenStyle()));
}


void Settings::setupLayout()
{
    QGroupBox *pWidgetStyleBox = new QGroupBox(tr("Widget Style"));
    {
        QVBoxLayout *pWidgetStyleLayout = new QVBoxLayout;
        {
            m_pcbStyles = new QComboBox;

            QRegExp regExp("Q(.*)Style");
            QString defaultStyle = QApplication::style()->metaObject()->className();
            if (defaultStyle == QLatin1String("QMacStyle"))
                defaultStyle = QLatin1String("Macintosh (Aqua)");
            else if (defaultStyle == QLatin1String("OxygenStyle"))
                defaultStyle = QLatin1String("Oxygen");
            else if (regExp.exactMatch(defaultStyle))
                defaultStyle = regExp.cap(1);

            m_pcbStyles->addItems(QStyleFactory::keys());
            m_pcbStyles->setCurrentIndex(m_pcbStyles->findText(defaultStyle));


            m_pchStyleSheetOverride = new QCheckBox("Application dark mode override");
            m_pchStyleSheetOverride->setToolTip("Set a dark mode for the application's buttons, menus, toolbars and other widgets.\n"
                                                "Customize by editing the text file xflr5_dark.qss located in the application's directory.\n"
                                                "Intended primarily for Windows OS which does not support dark mode for 3rd party apps.\n"
                                                "This option should be used together with the UI dark theme activated.");
            connect(m_pchStyleSheetOverride, SIGNAL(clicked(bool)), SLOT(onStyleSheet(bool)));

            pWidgetStyleLayout->addWidget(m_pcbStyles);
            pWidgetStyleLayout->addWidget(m_pchStyleSheetOverride);
        }
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
                    m_pcbBackColor      = new ColorBtn(this);
                    pBackLayout->addWidget(m_pcbBackColor);
                }
                pBackBox->setLayout(pBackLayout);
            }

            QGroupBox *pFontBox = new QGroupBox(tr("Fonts"));
            {
                QGridLayout *pMainFontLayout = new QGridLayout;
                {
                    QLabel *plabMain = new QLabel(tr("Main display font"));
                    m_ppbTextFont = new QPushButton;
                    m_ptcbTextClr  = new TextClrBtn(this);

                    QLabel *plabTable = new QLabel(tr("Table font"));
                    m_ppbTableFont = new QPushButton;

                    QLabel *plabTree = new QLabel(tr("Tree font"));
                    m_ppbTreeFont = new QPushButton;

                    pMainFontLayout->addWidget(plabMain,      1,1);
                    pMainFontLayout->addWidget(m_ppbTextFont, 1,2);
                    pMainFontLayout->addWidget(m_ptcbTextClr, 1,3);
                    pMainFontLayout->addWidget(plabTable,     2,1);
                    pMainFontLayout->addWidget(m_ppbTableFont,2,2);
                    pMainFontLayout->addWidget(plabTree,      3,1);
                    pMainFontLayout->addWidget(m_ppbTreeFont, 3,2);
                }
                pFontBox->setLayout(pMainFontLayout);
            }

            QGroupBox *pGraphBox = new QGroupBox(tr("Graph Settings"));
            {
                QVBoxLayout *pGraphLayout = new QVBoxLayout;
                {
                    m_ppbGraphSettings  = new QPushButton(tr("All Graph Settings"));
                    m_ppbGraphSettings->setMinimumWidth(120);
                    pGraphLayout->addWidget(m_ppbGraphSettings);
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
        m_pchReverseZoom = new QCheckBox(tr("Reverse zoom direction using mouse wheel"));
        m_pchAlignChildrenStyle = new QCheckBox(tr("Flow down changes made to the style of objects to their children"));
        QString tip = tr("If activated:\n"
                         "all changes made to the style of the polar objects will flow down to the operating points\n"
                         "all changes made to the style of the foil objects will flow down to the polars and to the operating points");
        m_pchAlignChildrenStyle->setToolTip(tip);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pWidgetStyleBox);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pThemeBox);
        pMainLayout->addWidget(m_pchReverseZoom);
        pMainLayout->addWidget(m_pchAlignChildrenStyle);
        pMainLayout->addSpacing(20);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}


void Settings::initWidget()
{
    m_prbCustom->setChecked(true);
    m_prbCustom->setEnabled(false);

    m_pchStyleSheetOverride->setChecked(s_bStyleSheet);

    m_MemGraph.copySettings(&s_RefGraph);

    m_pcbBackColor->setColor(DisplayOptions::s_BackgroundColor);

    m_ptcbTextClr->setTextColor(DisplayOptions::s_TextColor);
    m_ptcbTextClr->setBackgroundColor(DisplayOptions::s_BackgroundColor);

    QFont s_TextFont = DisplayOptions::textFont();
    QString textFontName = s_TextFont.family() + QString(" %1").arg(s_TextFont.pointSize());
    m_ppbTextFont->setText(textFontName);
    m_ppbTextFont->setFont(s_TextFont);
    m_ptcbTextClr->setFont(s_TextFont);
    m_ptcbTextClr->setText(QObject::tr("Text color"));

    QFont s_TableFont = DisplayOptions::tableFont();
    QString tableFontName = s_TableFont.family() + QString(" %1").arg(s_TableFont.pointSize());
    m_ppbTableFont->setText(tableFontName);
    m_ppbTableFont->setFont(s_TableFont);

    QFont s_TreeFont = DisplayOptions::treeFont();
    QString TreeFontName = s_TreeFont.family() + QString(" %1").arg(s_TreeFont.pointSize());
    m_ppbTreeFont->setText(TreeFontName);
    m_ppbTreeFont->setFont(s_TreeFont);

    if(m_pcbStyles->findText(s_StyleName)>=0)
        m_pcbStyles->setCurrentIndex(m_pcbStyles->findText(s_StyleName));
    else if(m_pcbStyles->findText(s_StyleSheetName)>=0)
        m_pcbStyles->setCurrentIndex(m_pcbStyles->findText(s_StyleSheetName));

    m_pchReverseZoom->setChecked(DisplayOptions::bReverseZoom());
    m_pchAlignChildrenStyle->setChecked(DisplayOptions::s_bAlignChildrenStyle);
}


void Settings::onStyleChanged(const QString &StyleName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    qApp->setStyle(StyleName);
    s_StyleName = StyleName;
    QApplication::restoreOverrideCursor();
}


void Settings::onBackgroundColor2d()
{
    QColor Color = QColorDialog::getColor(DisplayOptions::s_BackgroundColor);
    if(Color.isValid()) DisplayOptions::s_BackgroundColor = Color;

    m_pcbBackColor->setColor(DisplayOptions::s_BackgroundColor);

    m_ptcbTextClr->setBackgroundColor(DisplayOptions::s_BackgroundColor);
}


void Settings::onGraphSettings()
{
    GraphDlg dlg(this);

    dlg.setGraph(&s_RefGraph);

    dlg.setControls();

    if(dlg.exec() == QDialog::Accepted)
    {
        m_bIsGraphModified = true;
    }
}


void Settings::onTextColor()
{
    QColor Color = QColorDialog::getColor(DisplayOptions::s_TextColor);
    if(Color.isValid()) DisplayOptions::s_TextColor = Color;
    m_ptcbTextClr->setTextColor(DisplayOptions::s_TextColor);
}


void Settings::onTextFont()
{
    bool bOK(false);
    QFont TextFont;

    QFontDialog::FontDialogOptions dialogoptions = QFontDialog::MonospacedFonts;
#ifdef Q_OS_MAC
    if(s_bDontUseNativeDlg) dialogoptions |= QFontDialog::DontUseNativeDialog;
#endif

    TextFont = QFontDialog::getFont(&bOK, DisplayOptions::textFont(), this,
                                    QString("Display font"), dialogoptions);

    if (bOK)
    {
        DisplayOptions::setTextFont(TextFont);
        setButtonFonts();
    }
}


void Settings::onTableFont()
{
    QFontDialog::FontDialogOptions dialogoptions = QFontDialog::MonospacedFonts;
#ifdef Q_OS_MAC
    if(s_bDontUseNativeDlg) dialogoptions |= QFontDialog::DontUseNativeDialog;
#endif
    bool bOK(false);
    QFont TableFont = QFontDialog::getFont(&bOK, DisplayOptions::tableFont(), this,
                                           QString("Table font"), dialogoptions);

    if (bOK)
    {
        DisplayOptions::setTableFont(TableFont);
        setButtonFonts();
    }
}


void Settings::onTreeFont()
{
    QFontDialog::FontDialogOptions dialogoptions = QFontDialog::MonospacedFonts;
#ifdef Q_OS_MAC
    if(s_bDontUseNativeDlg) dialogoptions |= QFontDialog::DontUseNativeDialog;
#endif
    bool bOK(false);
    QFont TreeFont = QFontDialog::getFont(&bOK, DisplayOptions::treeFont(), this, QString("Tree font"), dialogoptions);

    if (bOK)
    {
        DisplayOptions::setTreeFont(TreeFont);
        setButtonFonts();
    }
}


void Settings::saveSettings(QSettings &settings)
{
    settings.beginGroup("global_settings");
    {
        settings.setValue("LastDirName", xfl::s_LastDirName);
        settings.setValue("XMLDirName",  xfl::s_xmlDirName);
        settings.setValue("PlrDirName",  xfl::s_plrDirName);

        settings.setValue("bStyleSheet", s_bStyleSheet);

        settings.setValue("BackgroundColor", DisplayOptions::s_BackgroundColor);
        settings.setValue("TextColor", DisplayOptions::s_TextColor);

        DisplayOptions::s_TextFontStruct.saveSettings(   settings, "TextFont");
        DisplayOptions::s_TableFontStruct.saveSettings(  settings, "TableFont");
        DisplayOptions::s_TreeFontStruct.saveSettings(   settings, "TreeFont");
        DisplayOptions::s_ToolTipFontStruct.saveSettings(settings, "ToolTipFont");

        settings.setValue("ScaleFactor", DisplayOptions::scaleFactor());

        settings.setValue("ShowMousePos", DisplayOptions::s_bShowMousePos);
        settings.setValue("AligneChildrenStyle", DisplayOptions::s_bAlignChildrenStyle);

        settings.setValue("ShowStyleSheets", s_bStyleSheet);
        settings.setValue("StyleSheetName", s_StyleSheetName);

        if(DisplayOptions::isLightTheme()) settings.setValue("Theme",0);
        else                               settings.setValue("Theme",1);

        s_RefGraph.setGraphName("Reference_Graph");
        s_RefGraph.saveSettings(settings);
    }
    settings.endGroup();
}


void Settings::loadSettings(QSettings &settings)
{
    settings.beginGroup("global_settings");
    {
        xfl::s_LastDirName = settings.value("LastDirName", QDir::homePath()).toString();
        xfl::s_xmlDirName  = settings.value("XMLDirName", QDir::homePath()).toString();
        xfl::s_plrDirName  = settings.value("PlrDirName", QDir::homePath()).toString();

        s_bStyleSheet = settings.value("bStyleSheet", false).toBool();

        DisplayOptions::s_BackgroundColor = settings.value("BackgroundColor", QColor(5,11,13)).value<QColor>();

        DisplayOptions::s_TextColor = settings.value("TextColor", QColor(237,237,237)).value<QColor>();

        DisplayOptions::s_TextFontStruct.loadSettings(   settings, "TextFont");
        DisplayOptions::s_TableFontStruct.loadSettings(  settings, "TableFont");
        DisplayOptions::s_TreeFontStruct.loadSettings(   settings, "TreeFont");
        DisplayOptions::s_ToolTipFontStruct.loadSettings(settings, "ToolTipFont");

        DisplayOptions::setScaleFactor(settings.value("ScaleFactor", DisplayOptions::scaleFactor()).toDouble());
        DisplayOptions::s_bAlignChildrenStyle = settings.value("AligneChildrenStyle", true).toBool();

        DisplayOptions::s_bShowMousePos = settings.value("ShowMousePos", true).toBool();

        s_bStyleSheet   = settings.value("ShowStyleSheets", false).toBool();
        s_StyleSheetName = settings.value("StyleSheetName", "xflr5_style").toString();

        int iTheme = settings.value("Theme",1).toInt();
        if(iTheme==0) DisplayOptions::setTheme(DisplayOptions::LIGHTTHEME);
        else          DisplayOptions::setTheme(DisplayOptions::DARKTHEME);

        s_RefGraph.setGraphName("Reference_Graph");
        s_RefGraph.loadSettings(settings);
    }
    settings.endGroup();
}



void Settings::onReverseZoom()
{
    if(m_pchReverseZoom->isChecked()) DisplayOptions::setScaleFactor(-0.07);
    else                              DisplayOptions::setScaleFactor( 0.07);
}


void Settings::onAlignChildrenStyle()
{
    DisplayOptions::s_bAlignChildrenStyle = m_pchAlignChildrenStyle->isChecked();
}


void Settings::onTheme()
{
    if (m_prbDark->isChecked())
    {
        m_bIsGraphModified = true;
        DisplayOptions::setTheme(DisplayOptions::DARKTHEME);
        DisplayOptions::s_BackgroundColor = QColor(3, 9, 9);
        DisplayOptions::s_TextColor=QColor(221,221,221);
        s_RefGraph.setGraphDefaults(true);
    }
    else if(m_prbLight->isChecked())
    {
        m_bIsGraphModified = true;
        DisplayOptions::setTheme(DisplayOptions::LIGHTTHEME);
        DisplayOptions::s_BackgroundColor = QColor(241, 241, 241);
        DisplayOptions::s_TextColor=QColor(0,0,0);
        s_RefGraph.setGraphDefaults(false);
    }
    else
    {
        return;
    }
    m_pcbBackColor->setColor(DisplayOptions::s_BackgroundColor);
    m_ptcbTextClr->setBackgroundColor(DisplayOptions::s_BackgroundColor);
    m_ptcbTextClr->setTextColor(DisplayOptions::s_TextColor);
}


void Settings::setColorList()
{
    //    QStringList colorList = QColor::colorNames();
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



void Settings::setButtonFonts()
{
    m_ptcbTextClr->setFont(DisplayOptions::textFont());

    m_ptcbTextClr->setTextColor(DisplayOptions::textColor());
    m_ptcbTextClr->setBackgroundColor(DisplayOptions::backgroundColor());


    m_ppbTextFont->setFont(DisplayOptions::textFont());
    m_ppbTextFont->setText(DisplayOptions::textFontStruct().family() + QString::asprintf(" %d",DisplayOptions::textFont().pointSize()));
    m_ptcbTextClr->setText(QObject::tr("Text color"));
    QString stylestring = QString::asprintf("font-family: %s; font-size: %dpt",
                                           DisplayOptions::textFont().family().toStdString().c_str(),
                                           DisplayOptions::textFont().pointSize());
    m_ppbTextFont->setStyleSheet(stylestring);
    stylestring = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                    DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                    DisplayOptions::textFont().family().toStdString().c_str(),
                                    DisplayOptions::textFont().pointSize());
    m_ptcbTextClr->setStyleSheet(stylestring);


    QString tableFontName = DisplayOptions::tableFont().family() + QString(" %1").arg(DisplayOptions::tableFont().pointSize());
    m_ppbTableFont->setText(tableFontName);
    stylestring = QString::asprintf("font-family: %s; font-size: %dpt",
                                    DisplayOptions::tableFont().family().toStdString().c_str(),
                                    DisplayOptions::tableFont().pointSize());
    m_ppbTableFont->setStyleSheet(stylestring);

    QString treeeFontName = DisplayOptions::treeFont().family() + QString(" %1").arg(DisplayOptions::treeFont().pointSize());
    m_ppbTreeFont->setText(treeeFontName);
    stylestring = QString::asprintf("font-family: %s; font-size: %dpt",
                                    DisplayOptions::treeFont().family().toStdString().c_str(),
                                    DisplayOptions::treeFont().pointSize());
    m_ppbTreeFont->setStyleSheet(stylestring);
}


void Settings::onStyleSheet(bool bSheet)
{
    s_bStyleSheet = bSheet;
    QFile stylefile;
    if(bSheet)
    {
        QString qssPathName =  qApp->applicationDirPath() + QDir::separator() +"/xflr5_dark.qss";
        QFileInfo fi(qssPathName);
        if(fi.exists())
            stylefile.setFileName(qssPathName);
        else
            stylefile.setFileName(QStringLiteral(":/qss/xflr5_dark.qss"));
        if (stylefile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            QString qsStylesheet = QString::fromLatin1(stylefile.readAll());
            qApp->setStyleSheet(qsStylesheet);
            stylefile.close();
            QApplication::restoreOverrideCursor();
        }
    }
    else
    {
        qApp->setStyleSheet(QString());
    }
}




