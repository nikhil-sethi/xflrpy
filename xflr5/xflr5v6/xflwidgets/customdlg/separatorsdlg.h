/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>


class PlainTextOutput;


class SeparatorsDlg : public QDialog
{
    Q_OBJECT
    public:
        SeparatorsDlg(QWidget *pParent=nullptr);

        void hideEvent(QHideEvent *pEvent) override;

        static bool bWhiteSpace() {return s_bWhiteSpace;}
        static bool bTab()        {return s_bTab;}
        static bool bComma()      {return s_bComma;}
        static bool bSemiColon()  {return s_bSemiColon;}

        static void setWhiteSpace(bool b) {s_bWhiteSpace=b;}
        static void setTab(bool b)        {s_bTab=b;}
        static void setComma(bool b)      {s_bComma=b;}
        static void setSemiColon(bool b)  {s_bSemiColon=b;}

    private:
        void setupLayout();

    private:
        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchWhiteSpace;
        QCheckBox *m_pchTab, *m_pchComma, *m_pchSemiColon;

        static bool s_bWhiteSpace;
        static bool s_bTab;
        static bool s_bComma;
        static bool s_bSemiColon;
};



