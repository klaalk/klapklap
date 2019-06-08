//
// Created by Klaus on 06/05/2019.
//

#ifndef CLIENT_CHAT_CLIENT_H
#define CLIENT_CHAT_CLIENT_H

#include <memory>

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslError>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/constants/kk_constants.h"
#include "../view/mainwindow.h"
#include "../chat/chatdialog.h"

QT_FORWARD_DECLARE_CLASS(QWebSocket)

class kk_client : public QObject
{
    Q_OBJECT
public:
    explicit kk_client(const QUrl &url, QObject *parent = nullptr);

private slots:
    void handleConnection();
    void handleResponse(QString message);
    void handleSslErrors(const QList<QSslError> &errors);

    void sendLoginRequest(QString email, QString password);
    void sendOpenFileRequest(QString fileName);
    void sendMessageRequest(QString message);
    void closeConnection();
private:
    void sendRequest(QString type, QString result, QString body);

    QString email_;
    QWebSocket socket_;
    MainWindow view_;
    ChatDialog chat_;
};

typedef std::shared_ptr<kk_client> kk_client_ptr;


#endif //CLIENT_CHAT_CLIENT_H
