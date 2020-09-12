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
#include <QtCore/QObject>
#include <QSharedPointer>
#include <QWebSocket>
#include <QFile>
#include <QTimer>

#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../../libs/src/classes/logger/kk_logger.h"
#include "../../../../../libs/src/classes/crdt/kk_crdt.h"

#include "../participant/kk_participant.h"

class KKFile : public QObject {
public:
    KKFile();
    ~KKFile();
    void join(KKParticipantPtr participant);
    void leave(KKParticipantPtr participant);
    void deliver(QString type, QString result, QStringList values, QString myNick);

    void setFile(QSharedPointer<QFile> file);
    QSharedPointer<QFile> getFile();

    void setHash(QString hash);
    QString getHash();

    KKVectorPayloadPtr getRecentMessages();
    KKMapParticipantPtr getParticipants();

    void addOwner(QString owner);
    void setOwners(QStringList* owners);
    QStringList* getOwners();

    void applyRemoteInsert(QStringList bodyList);
    void applyRemoteCharFormatChange(QStringList bodyList);
    void initCrdtText();
    void flushCrdtText();

    QStringList getCrdtText();

    int getParticipantCounter() const;
public slots:
    void handleTimeout();
private:
    enum { MaxRecentMessages = 100 };
    int participantCounter = 0;
    KKMapParticipantPtr participants;
    QStringList* owners;
    KKVectorPayloadPtr recentMessages;
    KKCrdtPtr crdt;
    QTimer* timer;
    bool flushCrdt = true;
    QSharedPointer<QFile> file;
    QString hash;
};

typedef QSharedPointer<KKFile> KKFilePtr;
typedef QSharedPointer<QMap<QString, KKFilePtr>> KKMapFilePtr;
#endif //KK_FILE_H
