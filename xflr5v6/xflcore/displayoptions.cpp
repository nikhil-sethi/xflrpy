/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/


#include "displayoptions.h"


QColor DisplayOptions::s_BackgroundColor(QColor(237, 237, 237));
QColor DisplayOptions::s_TextColor(Qt::black);


FontStruct DisplayOptions::s_TextFontStruct;
FontStruct DisplayOptions::s_TableFontStruct;
FontStruct DisplayOptions::s_TreeFontStruct;
FontStruct DisplayOptions::s_ToolTipFontStruct;

DisplayOptions::enumThemeType DisplayOptions::s_Theme(DisplayOptions::LIGHTTHEME);

double DisplayOptions::s_ScaleFactor(0.07);


bool DisplayOptions::s_bAlignChildrenStyle(true);
bool DisplayOptions::s_bShowMousePos(true);
