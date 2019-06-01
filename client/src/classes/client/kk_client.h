//
// Created by Augens on 06/05/2019.
//

#ifndef CLIENT_CHAT_CLIENT_H
#define CLIENT_CHAT_CLIENT_H


#include <cstdlib>
#include <deque>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslError>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/constants/kk_constants.h"

QT_FORWARD_DECLARE_CLASS(QWebSocket)

class kk_client : public QObject
{
    Q_OBJECT
public:
    explicit kk_client(const QUrl &url, QObject *parent = nullptr);
    void sendLoginRequest(QString email, QString password);

private Q_SLOTS:
    void onConnected();
    void onMessageReceived(QString message);
    void onSslErrors(const QList<QSslError> &errors);
    void closeConnection();
private:
    QWebSocket m_webSocket;
};


#endif //CLIENT_CHAT_CLIENT_H
