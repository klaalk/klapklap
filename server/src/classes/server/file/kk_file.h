//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_FILE_H
#define KK_FILE_H

#include <set>
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <QSharedPointer>
#include <QWebSocket>


#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../participant/kk_participant.h"


class kk_file {
public:
    kk_file();
    ~kk_file();
    void join(QSharedPointer<kk_participant> participant);

    void leave(QSharedPointer<kk_participant> participant);

    void deliver(QString type, QString result, QString message, QString myNick);

    queue_payload_ptr getRecentMessages();
private:
    std::set<QSharedPointer<kk_participant>> participants_;
    enum {
        max_recent_msgs = 100
    };
    queue_payload_ptr recent_msgs_;
};

typedef QSharedPointer<kk_file> kk_file_ptr;
typedef QSharedPointer<QMap<QString, kk_file_ptr>> map_files_ptr;
#endif //KK_FILE_H
