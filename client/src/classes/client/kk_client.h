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
#include <QTimer>
#include <QTextCursor>
#include <QTextEdit>

#include "../../../../libs/src/constants/kk_constants.h"
#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/classes/logger/kk_logger.h"
#include "../../../../libs/src/classes/crdt/kk_crdt.h"
#include "../../../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../../../libs/src/classes/user/kk_user.h"
#include "../../../../libs/src/classes/task/kk_task.h"

#include "../access/accessdialog.h"
#include "../chat/kk_chat.h"
#include "../openfile/openfiledialog.h"
#include "../textedit/kk_editor.h"
#include "../modal/kk_modal.h"


QT_FORWARD_DECLARE_CLASS(QWebSocket)

class KKClient : public QObject
{
    Q_OBJECT
public:
    explicit KKClient(QUrl url, QObject *parent = nullptr);
    QSharedPointer<QList<int>> findPositions(const QString& siteId);

private slots:
    void handleOpenedConnection();
    void handleResponse(const QString& message);
    void handleTimeOutConnection();
    void handleErrorConnection(QAbstractSocket::SocketError error);
    void handleSslErrors(const QList<QSslError> &errors);
    void handleModalButtonClick(const QString& btnText, const QString& modalType);
    void handleModalClosed(const QString& modalType);
    void handleModalActions(const QString& modalType, bool closed);

    void sendSignupRequest(QString email, const QString& password, QString name, QString surname, QString username);
    void sendLoginRequest(QString email, const QString& password);
    void sendLogoutRequest();
    void sendGetFilesRequest();
    void sendOpenFileRequest(const QString& link, const QString& fileName);
    void sendLoadFileRequest(const QString& link);
    void sendQuitFileRequest();
    void sendCrdtRequest(QStringList crdt);
    void sendMessageRequest(QString username, QString message);
    void sendUpdateUserRequest(QString name, QString surname, QString alias, QString avatar);

    void onEditorClosed();
    void onInsertTextToCrdt(unsigned long position, QList<QChar>, QStringList fonts, QStringList colors);
    void onRemoveTextFromCrdt(unsigned long start, unsigned long end, QString value);
    void onCharFormatChanged(unsigned long start, unsigned long end, QString font, QString color);
    void onAlignmentChange(int alignment, int alignStart, int alignEnd);
    void onNotifyAlignment(int alignStart, int alignEnd);
    void onSaveCrdtToFile();
    void onOpenFileDialogOpened();
    void onOpenFileDialogClosed();
    void onSiteIdClicked(const QString& siteId);
    void logger(QString message);
    void printCrdt();
private:
    void initState();
    void initEditor();
    void handleSuccessResponse(KKPayload res);
    void handleLoginResponse(KKPayload res);
    void handleLogoutResponse(KKPayload res);
    void handleUpdateUserResponse();
    void handleSignupResponse();
    void handleGetFilesResponse(KKPayload res);
    void handleOpenFileResponse(KKPayload res);
    void handleLoadFileResponse(KKPayload res);
    void handleQuitFileResponse();
    void handleCrdtResponse(KKPayload res);
    void handleCrdtAlignmentResponse(QStringList ranges);
    void handleCrdtTextResponse(QString remoteSiteId, QString operation, QStringList chars);
    void handleErrorResponse(KKPayload res);
    void handleClientErrorResponse(KKPayload res);
    void handleServerErrorResponse(KKPayload res);
    bool sendRequest(QString type, QString result, QStringList values);

    QStringList avatars;

    QString state;
    QString filename;
    QString link;
    bool fileValid;

    QUrl url;
    QWebSocket socket;
    QTimer timer;
    AccessDialog access;
    OpenFileDialog openFile;
    KKModal modal;

    KKUser* user{};
    KKEditor* editor{};
    KKCrdt* crdt{};
    KKChat* chat{};
};

typedef std::shared_ptr<KKClient> KKClientPtr;

#endif //CLIENT_CHAT_CLIENT_H
