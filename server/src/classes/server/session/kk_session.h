//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SESSION_H
#define KK_SESSION_H

#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>
#include <QDebug>
#include <QThreadPool>
#include <QtCore/QByteArray>

#include <classes/user/kk_user.h>
#include <classes/db/kk_db.h>
#include <classes/server/file/kk_file.h>
#include <classes/server/filesys/kk_filesys.h>
#include <classes/server/task/kk_task.h>
#include <classes/smtp/kk_smtp.h>

#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../../libs/src/classes/logger/kk_logger.h"
#include "../../../../../libs/src/constants/kk_constants.h"

//#define ENV

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class KKSession : public QObject, public KKParticipant, public QEnableSharedFromThis<KKSession> {
    Q_OBJECT
signals:
    void disconnected(QString sessionId);
public:
    KKSession(KKDataBasePtr db, KKFileSystemPtr filesys, KKMapFilePtr files, QString sessiondId, QObject *parent = nullptr);
    ~KKSession();

    void deliver(KKPayloadPtr msg);
    void sendResponse(QString type, QString result, QStringList values);
    void setSocket(QSharedPointer<QWebSocket> Descriptor);

    QString getSessionId();

public slots:
    void handleRequest(QString message);
    void handleBinaryRequests(QByteArray message);
    void handleDisconnection();
private:
    void handleLoginRequest(KKPayload request);
    void handleSignupRequest(KKPayload request);
    void handleGetFilesRequest();
    void handleOpenFileRequest(KKPayload request);
    void handleUpdateUserRequest(KKPayload request);
    void handleChatRequest(KKPayload request);
    void handleCrdtRequest(KKPayload request);
    void handleSaveFileRequest(KKPayload request);
    void handleLoadFileRequest(KKPayload request);
    void handleAlignChangeRequest(KKPayload request);
    void handleFormatChangeRequest(KKPayload request);

    void logger(QString message);

    QSharedPointer<QWebSocket> socket;
    KKDataBasePtr db;
    KKMapFilePtr files;
    KKFilePtr file;
    KKFileSystemPtr fileSystem;
    KKUserPtr user;
    QString sessionId;
    int taskNumerator = 0;
};

typedef QSharedPointer<KKSession> KKSessionPtr;
typedef QSharedPointer<QMap<QString, KKSessionPtr>> KKMapSessionPtr;
#endif //KK_SESSION_H
