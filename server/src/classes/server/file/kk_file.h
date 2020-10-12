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
#include "../../../../../libs/src/classes/task/kk_task.h"
#include "../participant/kk_participant.h"

class KKFile : public QObject, public QEnableSharedFromThis<KKFile> {
    Q_OBJECT
public:
    KKFile(QObject *parent = nullptr);
    ~KKFile();
    void join(KKParticipantPtr participant);
    void leave(KKParticipantPtr participant);
    int deliverMessages(KKPayload data, QString username);

    void consumeMessages();
    void produceMessages(KKPayload action, QString username);

    void setFile(QSharedPointer<QFile> file);
    QSharedPointer<QFile> getFile();

    void setHash(QString hash);
    QString getHash();

    void addUser(QString user);
    void setUsers(QStringList users);
    QStringList getUsers();

    void initCrdtText();
    void flushCrdtText();
    int changeCrdtText(QStringList bodyList);
    QStringList getCrdtText();

    QStringList getParticipants();
    int getPartecipantsNumber();
    bool partecipantExist(QString username);

    QVector<KKPayload> getChatMessages();
public slots:
    void handleTimeout();

private:
    enum { MaxRecentMessages = 100 };

    KKMapParticipantPtr participants;
    KKVectorPayloadPtr chatMessages;
    KKVectorPairPayloadPtr messages;
    QSharedPointer<QVector<QStringList>> crdtActions;

    KKCrdtPtr crdt;
    QStringList users;
    QSharedPointer<QFile> file;
    QSharedPointer<QTimer> timer;
    QString hash;

    QWaitCondition messagesWait;
    KKTask *messagesTask;
    QMutex messagesMutex;
    QMutex crdtMutex;
    QMutex chatMutex;
    QMutex participantsMutex;
};

typedef QSharedPointer<KKFile> KKFilePtr;
typedef QSharedPointer<QMap<QString, KKFilePtr>> KKMapFilePtr;
#endif //KK_FILE_H
