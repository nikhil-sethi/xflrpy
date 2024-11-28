/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once


#include <QApplication>
#include <QPlainTextEdit>


#include <xflcore/fontstruct.h>

class PlainTextOutput : public QPlainTextEdit
{
    Q_OBJECT
    public:
        PlainTextOutput(QWidget *pWidget=nullptr);
        void setCharDimensions(int nHChar, int nVChar);

        static void setTableFontStruct(FontStruct const &fntstruct) {s_TableFontStruct=fntstruct;}
        static void setTableColor(QColor textcolor) {s_TableColor=textcolor;}

    public slots:
        void onAppendThisPlainText(QString const &sometext);

    protected:
        QSize sizeHint() const override;
        void showEvent(QShowEvent *pEvent) override;

    private:
        int m_nHChar;
        int m_nVChar;

        static FontStruct s_TableFontStruct;

        static QColor s_TableColor;
};

