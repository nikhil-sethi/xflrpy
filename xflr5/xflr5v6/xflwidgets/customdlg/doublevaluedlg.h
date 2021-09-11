/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

#include <xflwidgets/customwts/doubleedit.h>


class DoubleValueDlg : public QDialog
{
    public:
        DoubleValueDlg(QWidget *pParent, QVector<double> values, QStringList const &leftlabels, QStringList const &rightlabels);

        void setValue(int iVal, int value) {if(iVal>=0 && iVal<m_pDoubleEdit.size()) m_pDoubleEdit[iVal]->setValue(value);}
        double value(int iVal) const{if(iVal>=0 && iVal<m_pDoubleEdit.size()) return m_pDoubleEdit[iVal]->value(); else return 0;}

    private:
        void keyPressEvent(QKeyEvent *pEvent);
        void showEvent(QShowEvent *);

    private:
        QVector<DoubleEdit *> m_pDoubleEdit;
        QDialogButtonBox *m_pButtonBox;
};






