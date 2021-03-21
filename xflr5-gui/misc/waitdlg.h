/****************************************************************************

    WaitDlg    Copyright (C) 2014-2017 Andre Deperrois

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
#include <QAbstractButton>


class IntEdit;

class WaitDlg : public QDialog
{
    Q_OBJECT
public:
    WaitDlg();
    bool isCancelled() const {return m_bCancel;}
    void setProgress(int p);


private slots:
    void onButton(QAbstractButton *);

private:
    bool m_bCancel;

    IntEdit *m_pProgress;
};

