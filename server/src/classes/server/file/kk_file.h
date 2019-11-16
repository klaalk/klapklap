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


class KKFile {
public:
    KKFile();
    ~KKFile();
    void join(QSharedPointer<KKParticipant> participant);

    void leave(QSharedPointer<KKParticipant> participant);

    void deliver(QString type, QString result, QStringList values, QString myNick);

    KKVectorPayloadPtr getRecentMessages();
private:
    std::set<QSharedPointer<KKParticipant>> participants;
    enum {
        MaxRecentMessages = 100
    };
    KKVectorPayloadPtr recentMessages;
};

typedef QSharedPointer<KKFile> KKFilePtr;
typedef QSharedPointer<QMap<QString, KKFilePtr>> KKMapFilePtr;
#endif //KK_FILE_H
