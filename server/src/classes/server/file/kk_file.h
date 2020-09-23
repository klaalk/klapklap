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

#include <QObject>
#include <QSharedPointer>
#include <QWebSocket>
#include <QException>
#include <QFile>
#include <QTimer>

#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../../libs/src/classes/logger/kk_logger.h"
#include "../../../../../libs/src/classes/crdt/kk_crdt.h"

#include "../participant/kk_participant.h"

class KKFile : public QObject, public QEnableSharedFromThis<KKFile> {
    Q_OBJECT
public:
    KKFile(QObject *parent = nullptr);
    ~KKFile();
    void join(KKParticipantPtr participant);
    void leave(KKParticipantPtr participant);
    int deliver(QString type, QString result, QStringList values, QString myNick);

    void setFile(QSharedPointer<QFile> file);
    QSharedPointer<QFile> getFile();

    void setHash(QString hash);
    QString getHash();

    void addUser(QString user);
    void setUsers(QStringList users);
    QStringList getUsers();

    void applyRemoteInsert(QStringList bodyList);
    void applyRemoteCharFormatChange(QStringList bodyList);
    int applyRemoteInsertSafe(QStringList bodyList);
    int applyRemoteCharFormatChangeSafe(QStringList bodyList);
    void applyRemoteAlignmentChange(QStringList bodylist);
    void initCrdtText();
    void flushCrdtText();

    QStringList getCrdtText();
    QStringList getParticipants();
    KKVectorPayloadPtr getRecentMessages();

    int getParticipantCounter() const;

public slots:
    void handleTimeout();

private:
    enum { MaxRecentMessages = 100 };

    KKMapParticipantPtr participants;
    KKVectorPayloadPtr recentMessages;
    KKCrdtPtr crdt;
    QStringList users;
    QSharedPointer<QFile> file;
    QSharedPointer<QTimer> timer;
    QString hash;
};

typedef QSharedPointer<KKFile> KKFilePtr;
typedef QSharedPointer<QMap<QString, KKFilePtr>> KKMapFilePtr;
#endif //KK_FILE_H
