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
    QObject(parent), server_socket_(nullptr) {
    server_socket_ = new QWebSocketServer(QStringLiteral("SSL Echo Server"),
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

    server_socket_->setSslConfiguration(sslConfiguration);

    files_ = QSharedPointer<QMap<QString, kk_file_ptr>>(new QMap<QString, kk_file_ptr>());
    db_ = QSharedPointer<kk_db>(new kk_db());

    if (server_socket_->listen(QHostAddress::Any, port)) {
        qDebug() << "SSL Server listening on port" << port;
        connect(server_socket_, &QWebSocketServer::newConnection, this,&kk_server::onNewConnection);
        connect(server_socket_, &QWebSocketServer::sslErrors, this, &kk_server::onSslErrors);
    }

}
kk_server::~kk_server()
{
    server_socket_->close();
    qDeleteAll(clients_.begin(), clients_.end());

    delete db_.get();

//    std::for_each(files_->begin(), files_->end(),[](kk_file_ptr e){
//        delete e.get();
//    });

//    delete files_.get();

    std::for_each(sessions_.begin(), sessions_.end(), [](kk_session_ptr p){
        delete p.get();
    });
}

void kk_server::onNewConnection() {
    QWebSocket *pSocket = server_socket_->nextPendingConnection();
    qDebug() << "Client connected";
    kk_session_ptr client = QSharedPointer<kk_session>(new kk_session(db_, files_, this));
    client->setSocket(pSocket);
    sessions_ << client;
    clients_ << pSocket;
}

void kk_server::onSslErrors(const QList<QSslError> &)
{
    qDebug() << "Ssl errors occurred";
}
