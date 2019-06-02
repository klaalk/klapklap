//
// Created by Klaus on 06/05/2019.
//

#include "kk_server.h"
#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>

QT_USE_NAMESPACE

kk_server::kk_server(quint16 port, QObject *parent):
    QObject(parent), m_pWebSocketServer(nullptr) {
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("SSL Echo Server"),
                                              QWebSocketServer::SecureMode,
                                              this);
    QSslConfiguration sslConfiguration;
    QFile certFile(QStringLiteral(":/localhost.cert"));
    QFile keyFile(QStringLiteral(":/localhost.key"));

    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);

    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();

    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

    m_pWebSocketServer->setSslConfiguration(sslConfiguration);
    db_ = std::shared_ptr<kk_db>(new kk_db());

    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        qDebug() << "SSL Echo Server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,&kk_server::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors, this, &kk_server::onSslErrors);
    }

}
kk_server::~kk_server()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void kk_server::onNewConnection() {
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    qDebug() << "Client connected";

    connect(pSocket, &QWebSocket::textMessageReceived, this, &kk_server::handleRequests);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &kk_server::handleBinaryRequests);
    connect(pSocket, &QWebSocket::disconnected, this, &kk_server::socketDisconnected);
    m_clients << pSocket;
}

void kk_server::handleRequests(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        qDebug() << "Client send:" << message;
        kk_payload req(message);
        req.decode_header();
        if(req.type() == "login") {
            QStringList body = req.body().split("_");
            bool result = db_->db_login(body.at(0), body.at(1));
            QString resultType = result ? "ok" : "ko";
            QString message = result ? "Account loggato" : "Account non loggato";
            kk_payload res("login", resultType, message);
            pClient->sendTextMessage(res.encode_header());
        } else if(req.type() == "signup") {

        } else if(req.type() == "openfile") {

        } else if(req.type() == "sharefile") {

        } else if(req.type() == "crdt") {

        } else if(req.type() == "chat") {

        }

    }
}

void kk_server::handleBinaryRequests(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        qDebug() << "Client send binary:" << message;
//        pClient->sendBinaryMessage(message);
    }
}

void kk_server::socketDisconnected()
{
    qDebug() << "Client disconnected";
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
//        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void kk_server::onSslErrors(const QList<QSslError> &)
{
    qDebug() << "Ssl errors occurred";
}
