//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

KKFile::KKFile(){
    recentMessages = QSharedPointer<QVector<KKPayloadPtr>>(new QVector<KKPayloadPtr>());
}

KKFile::~KKFile() {
    std::for_each(recentMessages->begin(), recentMessages->end(), [](KKPayloadPtr d){
        delete d.get();
    });

    delete recentMessages.get();
}
void KKFile::join(QSharedPointer<KKParticipant> participant) {
    participants.insert(participant);
}

void KKFile::leave(QSharedPointer<KKParticipant> participant) {
    participants.erase(participant);
}

void KKFile::deliver(QString type, QString result, QStringList message, QString myNick) {
    KKPayloadPtr data = QSharedPointer<KKPayload>(new KKPayload(type,result, message));
    recentMessages->push_back(data);

    while (recentMessages->size() > MaxRecentMessages)
        recentMessages->pop_front();

    std::for_each(participants.begin(), participants.end(),[&](QSharedPointer<KKParticipant> p){
        if(p->id != myNick) {
            p->deliver(data);
        }
    });
}

KKVectorPayloadPtr KKFile::getRecentMessages() {
    return recentMessages;
}
