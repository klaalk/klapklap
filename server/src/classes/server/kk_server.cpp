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

KKServer::KKServer(quint16 port, QObject *parent):
    QObject(parent), socket(nullptr) {
    socket = new QWebSocketServer(QStringLiteral("SSL Echo Server"), QWebSocketServer::SecureMode, this);

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

    socket->setSslConfiguration(sslConfiguration);

    files = QSharedPointer<QMap<QString, KKFilePtr>>(new QMap<QString, KKFilePtr>());
    db = QSharedPointer<KKDataBase>(new KKDataBase());
    filesys = QSharedPointer<KKFileSystem>(new KKFileSystem(db));

    QString run_info="";
    run_info = "RUNNING (Version:" +  QString::number(VERSION_MAJOR) +  "." + QString::number(VERSION_MINOR) + " Build: "
            + QString::number(VERSION_BUILD)+")";

    filesys->createFile(FILE_SYSTEM_USER, LOG_FILE);
    filesys->writeFile(LOG_FILE, run_info);

    if (socket->listen(QHostAddress::Any, port)) {
        filesys->writeFile(LOG_FILE, "SSL Server listening on port " + QString::number(port));
        connect(socket, &QWebSocketServer::newConnection, this,&KKServer::onNewConnection);
        connect(socket, &QWebSocketServer::sslErrors, this, &KKServer::onSslErrors);
    }

}
KKServer::~KKServer()
{
    socket->close();
    qDeleteAll(clients.begin(), clients.end());
    delete db.get();
    std::for_each(files->begin(), files->end(),[](KKFilePtr e){
        delete e.get();
    });
    delete files.get();
    std::for_each(sessions.begin(), sessions.end(), [](KKSessionPtr p){
        delete p.get();
    });
}

void KKServer::onNewConnection() {
    QWebSocket *pSocket = socket->nextPendingConnection();
    KKSessionPtr client = QSharedPointer<KKSession>(new KKSession(db, filesys, files, this));
    client->setSocket(pSocket);
    sessions << client;
    clients << pSocket;
}

void KKServer::onSslErrors(const QList<QSslError> &)
{
    filesys->writeFile(LOG_FILE,"SSL errors occurred");
}
