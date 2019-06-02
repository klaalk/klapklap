//
// Created by Augens on 06/05/2019.
//

#ifndef CLIENT_CHAT_CLIENT_H
#define CLIENT_CHAT_CLIENT_H

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslError>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>

QT_FORWARD_DECLARE_CLASS(QWebSocket)

#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/constants/kk_constants.h"
#include "../view/mainwindow.h"

class kk_client : public QObject
{
    Q_OBJECT
public:
    explicit kk_client(const QUrl &url, QObject *parent = nullptr);

public slots:
    void sendLoginRequest(QString email, QString password);
    void sendOpenFileRequest(QString fileName);

private slots:
    void onConnected();
    void onMessageReceived(QString message);
    void onSslErrors(const QList<QSslError> &errors);
    void closeConnection();
private:
    QWebSocket socket_;
    MainWindow view_;
};
#endif //CLIENT_CHAT_CLIENT_H
