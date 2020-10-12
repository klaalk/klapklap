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
    possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

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

    sessions = KKMapSessionPtr(new QMap<QString, KKSessionPtr>());
    files = KKMapFilePtr(new QMap<QString, KKFilePtr>());
    db = KKDataBasePtr(new KKDataBase());
    filesys = KKFileSystemPtr(new KKFileSystem(this));

    KKLogger::log(QString("Running Server (Version: %1.%2 Build: %3)")
                  .arg(QString::number(VERSION_MAJOR),
                       QString::number(VERSION_MINOR),
                       QString::number(VERSION_BUILD)), "SERVER");

    if (socket->listen(QHostAddress::Any, port)) {
        connect(socket, &QWebSocketServer::newConnection, this, &KKServer::onNewConnection);
        connect(socket, &QWebSocketServer::sslErrors, this, &KKServer::onSslErrors);
        KKLogger::log(QString("SSL Server listening on port %1").arg(QString::number(port)), "SERVER");
    }
}
KKServer::~KKServer()
{
    socket->close();
    files.clear();
    sessions.clear();

    delete files.get();
    delete sessions.get();
    delete filesys.get();
    delete files.get();
    delete db.get();
}

void KKServer::onNewConnection() {
    QWebSocket *pSocket = socket->nextPendingConnection();
    QString sessionId = generateSessionId();

    KKSessionPtr session = KKSessionPtr(new KKSession(db, filesys, files, sessionId, this));
    session->setSocket(QSharedPointer<QWebSocket>(pSocket));

    connect(session.get(), &KKSession::disconnected, this, &KKServer::onSessionDisconnected);
    sessions->insert(sessionId, session);
}

void KKServer::onSslErrors(const QList<QSslError> &)
{
    KKLogger::log("SSL errors occurred", "SERVER");
}

void KKServer::onSessionDisconnected(QString sessionId)
{
    if (sessions->remove(sessionId)) {
        KKLogger::log("Session closed succesfully", sessionId);
    }
}

QString KKServer::generateSessionId()
{
    QString randomString;
    for(int i=0; i<randomStringLength; ++i) {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}
