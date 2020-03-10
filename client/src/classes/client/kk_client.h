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
#include <QInputDialog>
#include <QDir>
#include <QMap>
#include<QTimer>
#include <QTextCursor>
#include <QTextEdit>

#include "../../../../libs/src/constants/kk_constants.h"
#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/classes/crdt/kk_crdt.h"
#include "../../../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../../../libs/src/classes/crypt/kk_crypt.h"

#include "../login/login.h"
#include "../chat/chatdialog.h"
#include "../openfile/openfiledialog.h"
#include "../textedit/textedit.h"
#include "../modal/modaldialog.h"


QT_FORWARD_DECLARE_CLASS(QWebSocket)

class KKClient : public QObject
{
    Q_OBJECT
public:
    explicit KKClient(const QUrl &url, QObject *parent = nullptr);


private slots:
    void handleOpenedConnection();
    void handleTimeOutConnection();
    void handleErrorConnection(QAbstractSocket::SocketError error);
    void handleSslErrors(const QList<QSslError> &errors);

    void handleResponse(QString message);

    void handleModalButtonClick(QString btnText, QString modalType);
    void handleModalClosed(QString modalType);

    void sendSignupRequest(QString email, QString password, QString name, QString surname, QString username);
    void sendLoginRequest(QString email, QString password);
    void sendOpenFileRequest(QString fileName);
    void sendCrdtRequest(QStringList crdt);
    void sendMessageRequest(QString username, QString message);

    void onInsertTextCRDT(QString diffText, int position);
    void onRemoveTextCRDT(int start, int end);
    void onsaveCRDTtoFile();
    void onloadCRDTtoFile();
    void onSiteIdClicked(QString siteId);
private:
    void setInitState();
    void handleLoginResponse(KKPayload res);
    void handleSignupResponse();
    void handleOpenfileResponse();
    void handleCrdtResponse(KKPayload res);
    void handleErrorResponse();
    bool sendRequest(QString type, QString result, QStringList body);

    QString email_;
    QString state_;
    QString currentfile;
    bool currentfileValid;


    QUrl url_;
    QWebSocket socket_;
    QTimer timer_;

    AccessDialog access_;
    TextEdit editor_;
    ChatDialog chat_;
    OpenFileDialog openFile_;
    ModalDialog modal_;

    KKCrdt* crdt_;

    QByteArray bufferCrdt_;
    std::mutex mtxCrdt_;
};

typedef std::shared_ptr<KKClient> KKClientPtr;

#endif //CLIENT_CHAT_CLIENT_H
