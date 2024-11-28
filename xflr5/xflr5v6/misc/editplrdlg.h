/****************************************************************************

    EditPlrDlg Class
    Copyright (C) André Deperrois

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


#include <QDialog>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QSettings>

class XDirect;
class Miarex;
class Polar;
class WPolar;
class FloatEditDelegate;

class EditPlrDlg : public QDialog
{
    Q_OBJECT

    friend class XDirect;
    friend class Miarex;

    public:
        EditPlrDlg(QWidget *pParent=nullptr);
        ~EditPlrDlg();

        QSize sizeHint() const override {return QSize(1100,700);}

        void initDialog(XDirect *pXDirect, Polar *pPolar, Miarex *pMiarex, WPolar *pWPolar);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onButton(QAbstractButton *pButton);

    private:
        void deletePoint();
        void deleteAllPoints();
        void setupLayout();
        void fillPolarData();
        void fillWPolarData();
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent*pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private:
        QPushButton *m_ppbDeletePoint, *m_ppbDeleteAllPoints;


        Polar *m_pPolar;
        WPolar *m_pWPolar;

        QTableView *m_ptvPoints;
        QStandardItemModel *m_pPointModel;
        FloatEditDelegate *m_pFloatDelegate;
        QDialogButtonBox *m_pButtonBox;

        XDirect *m_pXDirect;
        Miarex *m_pMiarex;

        static QByteArray s_Geometry;
};


