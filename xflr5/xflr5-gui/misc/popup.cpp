/****************************************************************************

    Popup Class
    Copyright (C) 2018 Andre Deperrois

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

#include <QVBoxLayout>
#include <QTimer>

#include "popup.h"


QPoint Popup::s_Position;
QSize  Popup::s_WindowSize = QSize(5,5);

Popup::Popup(QWidget *pParent) : QWidget(pParent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setupLayout();
}


Popup::Popup(QString const &message, QWidget *pParent) : QWidget(pParent)
{
	setupLayout();
	m_pMessage->setText(message);
}


void Popup::setupLayout()
{
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		m_pMessage = new QLabel;
        QFont fixedWidthFont("Courier");
        fixedWidthFont.setPointSize(14);
        m_pMessage->setFont(fixedWidthFont);
		m_pMessage->setStyleSheet("QLabel { color : red; background-color: black; "
                                  "padding: 5px}");

		m_pMessage->setText("A toast popup\n with useful information");
	}
	pMainLayout->addWidget(m_pMessage);
	setLayout(pMainLayout);
}

void Popup::setRed()
{
    m_pMessage->setStyleSheet("QLabel { color : red; background-color: black; "
                              "padding: 5px}");
}

void Popup::setGreen()
{
    m_pMessage->setStyleSheet("QLabel { color : green; background-color: black; "
                              "padding: 5px}");
}


void Popup::showEvent(QShowEvent *)
{
    move(s_Position);
/*    QTimer *pTimer = new QTimer;
    pTimer->setSingleShot(true);
    connect(pTimer,SIGNAL(timeout()),this,SLOT(close()));
    pTimer->start(11000); //hide once the user has read the popup*/

    QTimer::singleShot(11000, this, SLOT(close()));
}


void Popup::hideEvent(QHideEvent*)
{
	s_Position = pos();
	s_WindowSize = size();
}

void Popup::mousePressEvent(QMouseEvent *)
{
	close();
}

void Popup::setTextMessage(const QString &text)
{
	m_pMessage->setText(text);
}


void Popup::appendTextMessage(const QString &text)
{
	QString strange = m_pMessage->text();
	m_pMessage->setText(strange + "\n"+ text);
}


