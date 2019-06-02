//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SESSION_H
#define KK_SESSION_H

#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../../libs/src/constants/kk_constants.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include "../partecipant/kk_partecipant.h"
#include "../file/kk_file.h"
#include "../room/kk_room.h"
#include "../../db/kk_db.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class kk_session : public QObject  {
        Q_OBJECT
public:
    kk_session(std::shared_ptr<kk_db> db, QWebSocket*  pSocket);

private slots:
    void handleRequests(QString message);
    void handleBinaryRequests(QByteArray message);
    void socketDisconnected();

private:
    void handle_request();
    QWebSocket*  pSocket_;
    std::shared_ptr<kk_db> db_;
    std::shared_ptr<kk_file> actual_file_;
};

typedef std::shared_ptr<kk_session> kk_session_ptr;

#endif //KK_SESSION_H
