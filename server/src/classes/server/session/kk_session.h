//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SESSION_H
#define KK_SESSION_H

#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../../libs/src/constants/kk_constants.h"
#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>
#include <QDebug>
#include <QThreadPool>
#include <QtCore/QByteArray>

#include "../participant/kk_participant.h"
#include "../file/kk_file.h"
#include "../task/kk_task.h"
#include "../room/kk_room.h"
#include "../../db/kk_db.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class kk_session : public QObject, public kk_participant, public QEnableSharedFromThis<kk_session> {
    Q_OBJECT
public:
    kk_session(kk_db_ptr db, map_files_ptr files_, QObject *parent = 0);
    ~kk_session();
    void deliver(kk_payload_ptr msg);
    void sendResponse(QString type, QString result, QString body);
    void setSocket(QWebSocket* Descriptor);
public slots:
    void handleRequest(QString message);
    void handleBinaryRequests(QByteArray message);
    void handleDisconnection();
private:
    QWebSocket*  session_socket_;
    kk_db_ptr db_;
    map_files_ptr files_;
    kk_file_ptr actual_file_;
};

typedef QSharedPointer<kk_session> kk_session_ptr;

#endif //KK_SESSION_H
