/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QColor>

#include <xflcore/fontstruct.h>


namespace DisplayOptions
{
    // Potential issues when declaring fonts as variables when linking statically to Qt
    // From the Qt dochttps://doc.qt.io/qt-5/qfont.html:
    //    "Note that a QGuiApplication instance must exist before a QFont can be used."
    //
    extern FontStruct s_TextFontStruct;
    extern FontStruct s_TableFontStruct;
    extern FontStruct s_TreeFontStruct;
    extern FontStruct s_ToolTipFontStruct;

    extern QColor s_BackgroundColor;
    extern QColor s_TextColor;

    extern double s_ScaleFactor;
    extern bool s_bAlignChildrenStyle;
    extern bool s_bShowMousePos;

    typedef enum {LIGHTTHEME, DARKTHEME, CUSTOMTHEME} enumThemeType;
    extern  enumThemeType s_Theme;

    inline void setAlignedChildrenStyle(bool bAlign) {s_bAlignChildrenStyle = bAlign;}
    inline bool isAlignedChildrenStyle()             {return s_bAlignChildrenStyle;}

    inline void showMousePos(bool bShow) {s_bShowMousePos=bShow;}
    inline bool bMousePos() {return s_bShowMousePos;}

    inline QColor backgroundColor() {return s_BackgroundColor;}
    inline void setBackgroundColor(QColor clr) {s_BackgroundColor=clr;}

    inline QColor const &textColor() {return s_TextColor;}
    inline void setTextColor(QColor clr) {s_TextColor=clr;}


    inline FontStruct const &textFontStruct()    {return s_TextFontStruct;}
    inline FontStruct const &tableFontStruct()   {return s_TableFontStruct;}
    inline FontStruct const &treeFontStruct()    {return s_TreeFontStruct;}
    inline FontStruct const &toolTipFontStruct() {return s_ToolTipFontStruct;}

    inline QFont textFont()    {return s_TextFontStruct.font();}
    inline QFont tableFont()   {return s_TableFontStruct.font();}
    inline QFont treeFont()    {return s_TreeFontStruct.font();}
    inline QFont toolTipFont() {return s_ToolTipFontStruct.font();}
    inline void setTextFont(   QFont const & fnt) {s_TextFontStruct    = {fnt.family(), fnt.pointSize(), fnt.weight(), fnt.italic(), QFont::Monospace};}
    inline void setTableFont(  QFont const & fnt) {s_TableFontStruct   = {fnt.family(), fnt.pointSize(), fnt.weight(), fnt.italic(), QFont::Monospace};}
    inline void setTreeFont(   QFont const & fnt) {s_TreeFontStruct    = {fnt.family(), fnt.pointSize(), fnt.weight(), fnt.italic(), QFont::SansSerif};}
    inline void setToolTipFont(QFont const & fnt) {s_ToolTipFontStruct = {fnt.family(), fnt.pointSize(), fnt.weight(), fnt.italic(), QFont::SansSerif};}


    inline void setTheme(DisplayOptions::enumThemeType themetype) {s_Theme=themetype;}
    inline enumThemeType theme() {return s_Theme;}
    inline bool isLightTheme()   {return s_Theme==DisplayOptions::LIGHTTHEME;}
    inline bool isDarkTheme()    {return s_Theme==DisplayOptions::DARKTHEME;}

    inline void setScaleFactor(double f) {s_ScaleFactor=f;}
    inline double scaleFactor() {return s_ScaleFactor;}
    inline bool bReverseZoom() {return s_ScaleFactor<0;}
};

