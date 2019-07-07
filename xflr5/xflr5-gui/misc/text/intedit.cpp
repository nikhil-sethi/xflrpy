/****************************************************************************

    IntEdit Class
    Copyright (C) 2013 Andre Deperrois

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

#include <misc/text/intedit.h>
#include <misc/options/settings.h>


IntEdit::IntEdit(QWidget *pParent)
{
    setParent(pParent);
    setAutoFillBackground(true);
    m_Value = 0.0;
    m_pDV = new QIntValidator(this);
    setValidator(m_pDV);
    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
}


IntEdit::IntEdit(int val, QWidget *pParent)
{
    setParent(pParent);
    setAutoFillBackground(true);
    m_Value = val;
    m_pDV = new QIntValidator(this);
    setValidator(m_pDV);
    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    formatValue();
}


void IntEdit::focusOutEvent (QFocusEvent *event)
{
    readValue();
    formatValue();
    //    emit(editingFinished()); //is emitted by call to base class
    QLineEdit::focusOutEvent(event);
}


int IntEdit::readValue()
{
    bool bOK;
    int val = locale().toInt(text().trimmed(), &bOK);
    //    int val = text().toInt(&bOK);
    if(bOK) m_Value = val;
    return m_Value;
}


void IntEdit::setValue(int val)
{
    m_Value = val;
    formatValue();
}


void IntEdit::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            readValue();
            formatValue();
            QLineEdit::keyPressEvent(event);

            break;
        }
        case Qt::Key_Escape:
        {
            formatValue();
            QLineEdit::keyPressEvent(event);
            break;
        }
        default:
        {
            QLineEdit::keyPressEvent(event);
            readValue();
            break;
        }
    }
}



void IntEdit::formatValue()
{
    setText(QString("%L1").arg(m_Value));
}



void IntEdit::setValueNoFormat(int val)
{
    m_Value = val;
}




QSize IntEdit::sizeHint() const
{
    QFontMetrics fm(Settings::s_TextFont);
    int w = 11 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}
