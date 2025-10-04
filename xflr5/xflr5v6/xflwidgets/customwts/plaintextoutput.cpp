/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include "plaintextoutput.h"


FontStruct PlainTextOutput::s_TableFontStruct;
QColor PlainTextOutput::s_TableColor;

PlainTextOutput::PlainTextOutput(QWidget *pWidget) : QPlainTextEdit(pWidget)
{
    m_nHChar = 15;
    m_nVChar = 5;

    setMinimumHeight(2*s_TableFontStruct.height());

    QPalette palette;
    palette.setColor(QPalette::WindowText, s_TableColor);
    palette.setColor(QPalette::Window, QColor(225,125,125,0));
    palette.setColor(QPalette::Base, QColor(125,155,225,25));

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);

    setBackgroundVisible(false);
    setPalette(palette);
    viewport()->setPalette(palette);
    viewport()->setAutoFillBackground(true);
}


void PlainTextOutput::showEvent(QShowEvent *)
{
    setFont(s_TableFontStruct.font());
}


void PlainTextOutput::setCharDimensions(int nHChar, int nVChar)
{
    m_nHChar = nHChar;
    m_nVChar = nVChar;
}


QSize PlainTextOutput::sizeHint() const
{
    return QSize(s_TableFontStruct.averageCharWidth()*m_nHChar, s_TableFontStruct.height()*m_nVChar);
}


void PlainTextOutput::onAppendThisPlainText(QString const &sometext)
{
    moveCursor(QTextCursor::End);
    insertPlainText(sometext);
    moveCursor(QTextCursor::End);
    ensureCursorVisible();
}

