/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once


#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

#include <xflwidgets/customwts/intedit.h>


class IntValuesDlg : public QDialog
{
    Q_OBJECT

    public:
        IntValuesDlg(QWidget*pParent, QVector<int> const &values, QStringList const &labels);

        void setValue(int iVal, int value) {if(iVal>=0 && iVal<m_pIntEdit.size()) m_pIntEdit[iVal]->setValue(value);}
        int value(int iVal) const {if(iVal>=0 && iVal<m_pIntEdit.size()) return m_pIntEdit[iVal]->value(); else return 0;}

        void setLabel(int iLabel, QString label);

    private:
        void showEvent(QShowEvent *);
        void setupLayout(int nValues);
        void keyPressEvent(QKeyEvent *pEvent);

    private:
        QVector<IntEdit *> m_pIntEdit;
        QVector<QLabel*> m_pLabel;

        QDialogButtonBox *m_pButtonBox;
};






