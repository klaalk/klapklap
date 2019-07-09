//
// Created by Klaus on 06/05/2019.
//

#ifndef CLIENT_CHAT_CLIENT_H
#define CLIENT_CHAT_CLIENT_H

#include <memory>
#include <mutex>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslError>
#include <QCoreApplication>

#include "../../../../libs/src/constants/kk_constants.h"
#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/classes/crdt/kk_crdt.h"
#include "../../../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../../../libs/src/classes/crypt/kk_crypt.h"

#include "../login/login.h"
#include "../chat/chatdialog.h"
#include "../openfile/openfiledialog.h"
#include "../textedit/textedit.h"



QT_FORWARD_DECLARE_CLASS(QWebSocket)

class kk_client : public QObject
{
    Q_OBJECT
public:
    explicit kk_client(const QUrl &url, QObject *parent = nullptr);

private slots:
    void handleOpenedConnection();
    void handleResponse(QString message);
    void handleSslErrors(const QList<QSslError> &errors);
    void handleClosedConnection();

    void sendSignupRequest(QString email, QString password, QString name, QString surname);
    void sendLoginRequest(QString email, QString password);
    void sendOpenFileRequest(QString fileName);
    void sendCrdtRequest(QString crdtType, QString crdt);
    void sendMessageRequest(QString message);

    void onInsertTextCRDT(QString diffText, int position);
   void onRemoveTextCRDT(int start, int end);

private:
    void sendRequest(QString type, QString result, QString body);
    QString email_;
    QWebSocket socket_;
    LoginWindow login_;
    TextEdit editor_;
    ChatDialog chat_;
    OpenFileDialog openFile_;
    kk_crdt* crdt_;
    QByteArray bufferCrdt_;
    std::mutex mtxCrdt_;
};

typedef std::shared_ptr<kk_client> kk_client_ptr;


#endif //CLIENT_CHAT_CLIENT_H
