/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include "helpimgdlg.h"

#include <QVBoxLayout>
#include <QScrollArea>

QByteArray HelpImgDlg::s_Geometry;

HelpImgDlg::HelpImgDlg(const QString &imagepath, QWidget *pParent) : QDialog(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setModal(false);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHelpLab = new QLabel();
        m_pHelpLab->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
        m_Image.load(imagepath);
        m_pHelpLab->setPixmap(m_Image);

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }
        pMainLayout->addWidget(m_pHelpLab);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


QSize HelpImgDlg::sizeHint() const
{
    if(!m_Image.isNull())
    {
        QSize sz = m_Image.size();
        if(sz.width()>1000)
        {
            sz.scale(1000,800, Qt::KeepAspectRatio);
        }
        else if(sz.height()>800)
        {
            sz.scale(sz.width()*800/sz.height(), 800, Qt::KeepAspectRatio);
        }
        return sz;
    }
    return QSize(750,700);
}


void HelpImgDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_Geometry);
    QDialog::showEvent(pEvent);
}


void HelpImgDlg::hideEvent(QHideEvent *pEvent)
{
    s_Geometry = saveGeometry();
    QDialog::hideEvent(pEvent);
}


void HelpImgDlg::resizeEvent(QResizeEvent *pEvent)
{
    QDialog::resizeEvent(pEvent);
    m_pHelpLab->setPixmap(m_Image.scaled(m_pHelpLab->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

