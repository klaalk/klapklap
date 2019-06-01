//
// Created by Augens on 06/05/2019.
//

#include "kk_client.h"

#include <QtCore/QDebug>
#include <QtWebSockets/QWebSocket>
#include <QCoreApplication>

kk_client::kk_client(const QUrl &url, QObject *parent): QObject(parent)
{
//  connection_state = not_connected;
//  state = file_closed;
    connect(&m_webSocket, &QWebSocket::connected, this, &kk_client::onConnected);
    connect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &kk_client::onSslErrors);
    m_webSocket.open(QUrl(url));
}

void kk_client::sendLoginRequest(QString email, QString password) {
    kk_payload payload("login", "ok", email + "_" + password);
    m_webSocket.sendTextMessage(payload.encode_header());
}

void kk_client::onConnected()
{
    qDebug() << "WebSocket connected";
    // Gestisco la lettura dei messaggi.
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &kk_client::onMessageReceived);
}

void kk_client::onMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
    kk_payload response(message);
}

void kk_client::onSslErrors(const QList<QSslError> &errors)
{
    Q_UNUSED(errors);
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    m_webSocket.ignoreSslErrors();
}

void kk_client::closeConnection(){
    qApp->quit();
}
