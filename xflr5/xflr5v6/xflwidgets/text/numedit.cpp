/****************************************************************************

    NumEdit Class
    Copyright (C) 2021 Andre Deperrois

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
#include <QClipboard>
#include <QMimeData>

#include "numedit.h"



NumEdit::NumEdit(QWidget *pWidget) : QLineEdit(pWidget)
{
}


QSize NumEdit::sizeHint() const
{
    QFont fnt;
    fnt.setPointSize(10);
    QFontMetrics fm(fnt);
    int w = 11 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void NumEdit::focusOutEvent (QFocusEvent *pEvent)
{
    readValue();
    formatValue();

    QLineEdit::focusOutEvent(pEvent);
}


/** Hides the base function */
void NumEdit::paste()
{
    QClipboard const *pClip = QApplication::clipboard();
    if (!pClip->mimeData()->hasText()) return; // can only paste text;

    QString eol = "\n";
    QStringList lines = pClip->text().split(eol);
    if(!lines.size()) return;

    clear();
    setText(lines.first());
}
