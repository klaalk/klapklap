//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

KKFile::KKFile(){
    recentMessages = KKVectorPayloadPtr(new QVector<KKPayloadPtr>());
    crdtMessages = KKVectorPayloadPtr(new QVector<KKPayloadPtr>());
    participants = KKMapParticipantPtr(new QMap<QString, KKParticipantPtr>());
}

KKFile::~KKFile() {
    std::for_each(recentMessages->begin(), recentMessages->end(), [](KKPayloadPtr d){
        delete d.get();
    });
    std::for_each(crdtMessages->begin(), crdtMessages->end(), [](KKPayloadPtr d){
        delete d.get();
    });
    delete file.get();
    delete crdtMessages.get();
    delete recentMessages.get();
}

void KKFile::join(KKParticipantPtr participant) {
    participants->insert(participant->id, participant);
}

void KKFile::leave(KKParticipantPtr participant) {
    participants->insert(participant->id, nullptr);
}

void KKFile::deliver(QString type, QString result, QStringList message, QString myNick) {
    KKPayloadPtr data = QSharedPointer<KKPayload>(new KKPayload(type,result, message));
    recentMessages->push_back(data);

    if (type == CRDT) {
        crdtMessages->push_back(data);
        crdtIndexMessages.push_back(messageIndex);
    }

    messageIndex++;

    while (recentMessages->size() > MaxRecentMessages)
        recentMessages->pop_front();

    std::for_each(participants->begin(), participants->end(),[&](QSharedPointer<KKParticipant> p){
        if(p->id != myNick) {
            p->deliver(data);
        }
    });
}

void KKFile::setFile(QSharedPointer<QFile> file)
{
    this->file=file;
}

QSharedPointer<QFile> KKFile::getFile()
{
    return this->file;
}

void KKFile::setFilename(QString filename)
{
    this->filename=filename;
}

QString KKFile::getFilename()
{
    return this->filename;
}

KKMapParticipantPtr KKFile::getParticipants()
{
    return this->participants;
}

KKVectorPayloadPtr KKFile::getRecentMessages() {
    return recentMessages;
}

void KKFile::setOwners(QStringList *owners)
{
    this->owners = owners;
}

void KKFile::addOwner(QString owner)
{
    if (!owner.isEmpty()) {
        owners->push_back(owner);
    }
}

QStringList* KKFile::getOwners()
{
    return this->owners;
}


