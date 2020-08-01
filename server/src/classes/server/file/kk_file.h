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
#include <QFile>


#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../participant/kk_participant.h"


class KKFile {
public:
    KKFile();
    ~KKFile();
    void join(KKParticipantPtr participant);
    void leave(KKParticipantPtr participant);
    void deliver(QString type, QString result, QStringList values, QString myNick);

    void setFile(QSharedPointer<QFile> file);
    QSharedPointer<QFile> getFile();

    void setFilename(QString filename);
    QString getFilename();

    void setParticipants(QStringList* users);
    KKMapParticipantPtr getParticipants();

    KKVectorPayloadPtr getRecentMessages();

private:
    enum { MaxRecentMessages = 100 };
    KKMapParticipantPtr participants;
    KKVectorPayloadPtr recentMessages;
    KKVectorPayloadPtr crdtMessages;
    QVector<long long> crdtIndexMessages;
    QSharedPointer<QFile> file;
    QString filename;
    long long messageIndex;
};

typedef QSharedPointer<KKFile> KKFilePtr;
typedef QSharedPointer<QMap<QString, KKFilePtr>> KKMapFilePtr;
#endif //KK_FILE_H
