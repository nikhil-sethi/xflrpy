/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>
#include <QTimer>

#include "popup.h"


PopUp::PopUp(QWidget *pParent) : QWidget(pParent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setupLayout();

    m_Duration = 5000;
}


PopUp::PopUp(QString const &message, QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    m_plabMessage->setText(message);
}


void PopUp::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabMessage = new QLabel;

        m_plabMessage->setAutoFillBackground(true);
        m_plabMessage->setContentsMargins(5,5,5,5);
        m_plabMessage->setText("A toast popup\n with useful information");
    }
    pMainLayout->addWidget(m_plabMessage);
    setLayout(pMainLayout);
}


void PopUp::setFont(QFont const &fnt)
{
    QFont txtfnt(fnt);
//    txtfnt.setPointSize(14);
    txtfnt.setBold(true);
    m_plabMessage->setFont(txtfnt);
}


void PopUp::setRed()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::darkRed);
    palette.setColor(QPalette::Window, QColor(205,205,205));
    m_plabMessage->setPalette(palette);
    m_plabMessage->setAutoFillBackground(true);
}


void PopUp::setGreen()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::darkGreen);
    palette.setColor(QPalette::Window, QColor(205,205,205));
    m_plabMessage->setPalette(palette);
    m_plabMessage->setAutoFillBackground(true);
}


void PopUp::showEvent(QShowEvent *)
{
    QTimer::singleShot(m_Duration, this, SLOT(close()));
}


void PopUp::mousePressEvent(QMouseEvent *)
{
    close();
}


void PopUp::setTextMessage(const QString &text)
{
    m_plabMessage->setText(text);
}


void PopUp::appendTextMessage(const QString &text)
{
    QString strange = m_plabMessage->text();
    m_plabMessage->setText(strange + "\n"+ text);
}


