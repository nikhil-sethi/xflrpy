/****************************************************************************

    Updater Class
    Copyright (C) 2018-2019 Andre Deperrois

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


#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>


#include "updater.h"
#include <globals/mainframe.h>
#include <globals/gui_params.h>
#include <globals/globals.h>


bool Updater::s_bAutoCheck = false;  // until crash bugs are fixed
int Updater::s_AvailableMajorVersion=-1;
int Updater::s_AvailableMinorVersion=-1;
QDate Updater::s_LastCheckDate;

MainFrame *Updater::s_pMainFrame = nullptr;


Updater::Updater(MainFrame *pMainFrame)
{
    Trace("Updater::Updater");
    s_pMainFrame = pMainFrame;
    m_Date.clear();
    m_Description.clear();

//    m_pNetworkReply = nullptr;

    m_pNetworkAcessManager = new QNetworkAccessManager;
    connect(m_pNetworkAcessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));

}


Updater::~Updater()
{
    if(m_pNetworkAcessManager)
        delete m_pNetworkAcessManager;
}


void Updater::checkForUpdates()
{
    Trace("Updater::checkForUpdates()");

    if(!m_pNetworkAcessManager)
    {
        Trace("Failed to access the network manager");
        return;
    }

    switch(m_pNetworkAcessManager->networkAccessible())
    {
        case QNetworkAccessManager::UnknownAccessibility:
            Trace("QNetworkAccessManager::Unknown Accessibility");
            return;
        case QNetworkAccessManager::NotAccessible:
            Trace("QNetworkAccessManager::Network Not Accessible");
            return;
        case QNetworkAccessManager::Accessible:
            Trace("QNetworkAccessManager::Accessible");
            break;
    }

    QUrl url("https://www.xflr5.com/rss/rssfeed.xml");

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "xfl_browser");

    QNetworkReply *pNetworkReply = m_pNetworkAcessManager->get(request);
    if(!pNetworkReply)
    {
        Trace("No net reply");
        return;
    }
    Trace("pNetworkReply->isOpen(): ",pNetworkReply->isOpen());
    Trace("pNetworkReply->isReadable(): ", pNetworkReply->isReadable());
    Trace("pNetworkReply->isWritable(): ", pNetworkReply->isWritable());

    connect(pNetworkReply, SIGNAL(readyRead()),                        this, SLOT(onReadyRead()));
    connect(pNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(pNetworkReply, SIGNAL(sslErrors(QVector<QSslError>)),        this, SLOT(slotSslErrors(QVector<QSslError>)));

}


void Updater::slotError(QNetworkReply::NetworkError neterror)
{
    switch(neterror)
    {
        case QNetworkReply::NoError:
            Trace("no error condition.");
            break;
        case QNetworkReply::ConnectionRefusedError:
            Trace("The remote server refused the connection (the server is not accepting requests)");
            break;
        case QNetworkReply::RemoteHostClosedError:
            Trace("The remote server closed the connection prematurely, before the entire reply was received and processed");
            break;
        case QNetworkReply::HostNotFoundError:
            Trace("The remote host name was not found (invalid hostname)");
            break;
        case QNetworkReply::TimeoutError:
            Trace("The connection to the remote server timed out");
            break;
        case QNetworkReply::OperationCanceledError:
            Trace("The operation was canceled via calls to abort() or close() before it was finished.");
            break;
        case QNetworkReply::SslHandshakeFailedError:
            Trace("The SSL/TLS handshake failed and the encrypted channel could not be established. The sslErrors() signal should have been emitted.");
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
            Trace("The connection was broken due to disconnection from the network, however the system has initiated roaming to another access point. The request should be resubmitted and will be processed as soon as the connection is re-established.");
            break;
        case QNetworkReply::NetworkSessionFailedError:
            Trace("The connection was broken due to disconnection from the network or failure to start the network.");
            break;
        case QNetworkReply::BackgroundRequestNotAllowedError:
            Trace("The background request is not currently allowed due to platform policy.");
            break;
        case QNetworkReply::TooManyRedirectsError:
            Trace("While following redirects, the maximum limit was reached. The limit is by default set to 50 or as set by QNetworkRequest::setMaxRedirectsAllowed(). (This value was introduced in 5.6.)");
            break;
        case QNetworkReply::InsecureRedirectError:
            Trace("While following redirects, the network access API detected a redirect from a encrypted protocol (https) to an unencrypted one (http). (This value was introduced in 5.6.)");
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
            Trace("The connection to the proxy server was refused (the proxy server is not accepting requests)");
            break;
        case QNetworkReply::ProxyConnectionClosedError:
            Trace("The proxy server closed the connection prematurely, before the entire reply was received and processed");
            break;
        case QNetworkReply::ProxyNotFoundError:
            Trace("The proxy host name was not found (invalid proxy hostname)");
            break;
        case QNetworkReply::ProxyTimeoutError:
            Trace("The connection to the proxy timed out or the proxy did not reply in time to the request sent");
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
            Trace("The proxy requires authentication in order to honour the request but did not accept any credentials offered (if any)");
            break;
        case QNetworkReply::ContentAccessDenied:
            Trace("The access to the remote content was denied (similar to HTTP error 403)");
            break;
        case QNetworkReply::ContentOperationNotPermittedError:
            Trace("The operation requested on the remote content is not permitted");
            break;
        case QNetworkReply::ContentNotFoundError:
            Trace("The remote content was not found at the server (similar to HTTP error 404)");
            break;
        case QNetworkReply::AuthenticationRequiredError:
            Trace("The remote server requires authentication to serve the content but the credentials provided were not accepted (if any)");
            break;
        case QNetworkReply::ContentReSendError:
            Trace("The request needed to be sent again, but this failed for example because the upload data could not be read a second time.");
            break;
        case QNetworkReply::ContentConflictError:
            Trace("The request could not be completed due to a conflict with the current state of the resource.");
            break;
        case QNetworkReply::ContentGoneError:
            Trace("The requested resource is no longer available at the server.");
            break;
        case QNetworkReply::InternalServerError:
            Trace("The server encountered an unexpected condition which prevented it from fulfilling the request.");
            break;
        case QNetworkReply::OperationNotImplementedError:
            Trace("The server does not support the functionality required to fulfill the request.");
            break;
        case QNetworkReply::ServiceUnavailableError:
            Trace("The server is unable to handle the request at this time.");
            break;
        case QNetworkReply::ProtocolUnknownError:
            Trace("The Network Access API cannot honor the request because the protocol is not known");
            break;
        case QNetworkReply::ProtocolInvalidOperationError:
            Trace("The requested operation is invalid for this protocol");
            break;
        case QNetworkReply::UnknownNetworkError:
            Trace("An unknown network-related error was detected");
            break;
        case QNetworkReply::UnknownProxyError:
            Trace("An unknown proxy-related error was detected");
            break;
        case QNetworkReply::UnknownContentError:
            Trace("An unknown error related to the remote content was detected");
            break;
        case QNetworkReply::ProtocolFailure:
            Trace("A breakdown in protocol was detected (parsing error, invalid or unexpected responses, etc.)");
            break;
        default:
        case QNetworkReply::UnknownServerError:
            Trace("An unknown error related to the server response was detected");
            break;
    }
}


void Updater::slotSslErrors(QVector<QSslError> sslerrors)
{
    Trace("Updater::slotSslErrors()");
    QString strange;
    for(int i=0; i<sslerrors.size(); i++)
    {
       strange.sprintf("SSl Error %d : ", sslerrors.at(i).error());
       Trace(strange+sslerrors.at(i).errorString());
    }
}


void Updater::onReadyRead()
{
    Trace("Updater::ReadyRead()");
}


void Updater::onReplyFinished(QNetworkReply* pNetReply)
{
    if(!pNetReply)
    {
        Trace("Null net reply - aborting");
        return;
    }
    Trace("Updater::onReplyFinished");

    QString str (pNetReply->readAll());

    // If we are redirected, try again. TODO: Limit redirection count.
    QVariant vt = pNetReply->attribute(QNetworkRequest::RedirectionTargetAttribute);


    if (!vt.isNull())
    {
        Trace("Redirected to:" + vt.toUrl().toString());
        pNetReply = m_pNetworkAcessManager->get(QNetworkRequest(vt.toUrl()));
    }
    else
    {
        // We have something.
        QDomDocument doc;
        QString error;
        if (!doc.setContent(str, false, &error))
        {
            Trace("Updater::onReplyFinished - error setting xml doc content");
        }
        else
        {
            QDomElement docElem = doc.documentElement();
            QDomNodeList nodeList = docElem.elementsByTagName("item");

            for (int i=0; i<nodeList.length(); i++)
            {
                QDomNode node = nodeList.item(i);
                QDomElement e = node.toElement();
                /*                QString strTitle =  e.elementsByTagName("title").item(0).firstChild().nodeValue();
                QString strLink = e.elementsByTagName("link").item(0).firstChild().nodeValue();
                QString strDescription = e.elementsByTagName("description").item(0).firstChild().nodeValue();
                qDebug()<<"    "<<strTitle;
                qDebug()<<"    "<<strLink;
                qDebug()<<"    "<<strDescription;*/
            }
            nodeList = docElem.elementsByTagName("xflversion");

            for (int i=0; i<nodeList.length(); i++)
            {
                QDomNode node = nodeList.item(i);
                QDomElement e = node.toElement();
                s_AvailableMajorVersion  = e.elementsByTagName("majorversion").item(0).firstChild().nodeValue().toInt();
                s_AvailableMinorVersion  = e.elementsByTagName("minorversion").item(0).firstChild().nodeValue().toInt();
                m_Date = e.elementsByTagName("date").item(0).firstChild().nodeValue();
                m_Description = e.elementsByTagName("description").item(0).firstChild().nodeValue();
            }

            s_LastCheckDate = QDate::currentDate();
        }
    }

    pNetReply->deleteLater();

    emit finishedUpdate();
}


/** downloads a file and prints it */
void Updater::onDownloadFinished(QNetworkReply *pNetworkReply)
{
    Trace("Updater::onDownloadFinished");
    if(pNetworkReply)
    {
/*
        if (pResponse->error() != QNetworkReply::NoError) return;
        QString contentType = pResponse->header(QNetworkRequest::ContentTypeHeader).toString();
              if (!contentType.contains("charset=utf-8")) {
              qWarning() << "Content charsets other than utf-8 are not implemented yet.";
              return;
          }*/
        QString html = QString::fromUtf8(pNetworkReply->readAll());
        Trace(html);
        pNetworkReply->deleteLater();
    }
}


bool Updater::hasUpdate()
{
    QString strange;
    strange.sprintf("Latest version %d.%d", Updater::majorVersion(), Updater::minorVersion());
    Trace(strange);

    if(s_AvailableMajorVersion>MAJOR_VERSION) return true;
    else if(s_AvailableMajorVersion==MAJOR_VERSION)
    {
        if(s_AvailableMinorVersion>MINOR_VERSION) return true;
    }
    return false;
}



void Updater::loadSettings(QSettings &settings)
{
    settings.beginGroup("Updater");
    {
        s_bAutoCheck = settings.value("AutoUpdateCheck", true).toBool();
        s_LastCheckDate = settings.value("LastCheckDate", false).toDate();
    }
    Trace("loading lastcheckdate: "+s_LastCheckDate.toString("yyyy.MM.dd"));
    settings.endGroup();

    s_bAutoCheck = false; // until crash bugs are fixed
}


void Updater::saveSettings(QSettings &settings)
{
    settings.beginGroup("Updater");
    {
        Trace("Saving lastcheckdate: "+s_LastCheckDate.toString("yyyy.MM.dd"));
        settings.setValue("AutoUpdateCheck", s_bAutoCheck);
        settings.setValue("LastCheckDate", s_LastCheckDate);
    }
    settings.endGroup();
}
