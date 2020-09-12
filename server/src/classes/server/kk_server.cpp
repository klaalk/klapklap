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

    files = QSharedPointer<QMap<QString, KKFilePtr>>(new QMap<QString, KKFilePtr>());
    db = QSharedPointer<KKDataBase>(new KKDataBase());
    filesys = QSharedPointer<KKFileSystem>(new KKFileSystem());

    QString runInfo="";
    runInfo = "RUNNING (Version:" +  QString::number(VERSION_MAJOR) +  "." + QString::number(VERSION_MINOR) + " Build: "
            + QString::number(VERSION_BUILD)+")";

    KKLogger::log(runInfo, "Server");

    if (socket->listen(QHostAddress::Any, port)) {
        KKLogger::log("SSL Server listening on port " + QString::number(port), "SERVER");
        connect(socket, &QWebSocketServer::newConnection, this, &KKServer::onNewConnection);
        connect(socket, &QWebSocketServer::sslErrors, this, &KKServer::onSslErrors);
    }

}
KKServer::~KKServer()
{
    socket->close();
    qDeleteAll(clients.begin(), clients.end());
    std::for_each(files->begin(), files->end(),[](KKFilePtr e){
        delete e.get();
    });
    std::for_each(sessions.begin(), sessions.end(), [](KKSessionPtr p){
        delete p.get();
    });

    delete filesys.get();
    delete files.get();
    delete db.get();
}

void KKServer::onNewConnection() {
    QWebSocket *pSocket = socket->nextPendingConnection();
    QString sessionId = generateSessionId();
    KKSessionPtr session = QSharedPointer<KKSession>(new KKSession(db, filesys, files, sessionId, this));
    session->setSocket(pSocket);

    connect(session.get(), &KKSession::disconnected, this, &KKServer::onSessionDisconnected);
    sessions.insert(sessionId, session);

    clients << pSocket;
}

void KKServer::onSslErrors(const QList<QSslError> &)
{
    KKLogger::log("SSL errors occurred", "SERVER");
}

void KKServer::onSessionDisconnected(QString sessionId)
{
    KKSessionPtr session = sessions.value(sessionId);
    sessions.remove(sessionId);
    KKLogger::log("Session closed succesfully", sessionId);
}

QString KKServer::generateSessionId()
{
    QString randomString;
    for(int i=0; i<randomStringLength; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}
