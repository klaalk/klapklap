//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

KKFile::KKFile() {
    recentMessages = KKVectorPayloadPtr(new QVector<KKPayloadPtr>());
    participants = KKMapParticipantPtr(new QMap<QString, KKParticipantPtr>());
    crdt = KKCrdtPtr(new KKCrdt("file", casuale));
    timer = new QTimer();

    // Set autosave timer
    connect(timer, &QTimer::timeout, this, &KKFile::handleTimeout);
    timer->start(10000);
}

KKFile::~KKFile() {
    KKLogger::log("File deleting...", hash);
    std::for_each(recentMessages->begin(), recentMessages->end(), [](KKPayloadPtr d){
        if (!d.isNull())
            delete d.get();
    });
    if (!recentMessages.isNull()) {
        delete recentMessages.get();
        KKLogger::log("Recent messages deleted. File deleting...", hash);
    }

    if (file->isOpen())
        file->close();

    if (!file.isNull()) {
        KKLogger::log("Physical file deleted. File deleting...", hash);
        delete file.get();
    }

    if (!crdt.isNull())
        delete crdt.get();
    if (owners != nullptr)
        delete owners;
    if (timer != nullptr)
        delete timer;
    KKLogger::log("Deleted successfullty", hash);
}

void KKFile::join(KKParticipantPtr participant) {
    participants->insert(participant->id, participant);
    participantCounter++;
}

void KKFile::leave(KKParticipantPtr participant) {
//    participants->insert(participant->id, nullptr);
    participants->remove(participant->id);
    participantCounter--;
}

void KKFile::deliver(QString type, QString result, QStringList message, QString myNick) {
    KKPayloadPtr data = KKPayloadPtr(new KKPayload(type, result, message));

    if (type == CRDT) {
        applyRemoteInsert(data->getBodyList());
    } else if (type == CHARFORMAT_CHANGE) {
        applyRemoteCharFormatChange(data->getBodyList());
    } else if (type == CHAT || type == REMOVED_PARTECIPANT || type == ADDED_PARTECIPANT) {
        recentMessages->push_back(data);
    }

    while (recentMessages->size() > MaxRecentMessages)
        recentMessages->pop_front();

    if (!participants->isEmpty()) {
        std::for_each(participants->begin(), participants->end(),[&](QSharedPointer<KKParticipant> p){
            if(p->id != myNick) {
                p->deliver(data);
            }
        });
    }
}

void KKFile::setFile(QSharedPointer<QFile> file)
{
    this->file=file;
}

QSharedPointer<QFile> KKFile::getFile()
{
    return this->file;
}

void KKFile::setHash(QString hash)
{
    this->hash=hash;
}

QString KKFile::getHash()
{
    return this->hash;
}

KKMapParticipantPtr KKFile::getParticipants()
{
    return this->participants;
}

KKVectorPayloadPtr KKFile::getRecentMessages() {
    return recentMessages;
}

void KKFile::addOwner(QString owner)
{
    if (!owner.isEmpty()) {
        owners->push_back(owner);
    }
}

void KKFile::setOwners(QStringList *owners)
{
    this->owners = owners;
}

QStringList* KKFile::getOwners()
{
    return this->owners;
}

void KKFile::applyRemoteInsert(QStringList bodyList)
{
    // Ottengo i campi della risposta
    int increment = bodyList[0] == CRDT_INSERT ? 0 : 1;
    QString siteId = bodyList[1 + increment];
    QString text = bodyList[2 + increment];
    QStringList ids = bodyList[3 + increment].split(" ");
    QString fontStr = bodyList[4 + increment];
    QString colorStr = bodyList[5 + increment];


    KKCharPtr char_ = KKCharPtr(new KKChar(*text.toLatin1().data(), siteId.toStdString()));
    char_->setKKCharFont(fontStr);
    char_->setKKCharColor(colorStr);

    // size() - 1 per non considerare l'elemento vuoto della string list ids
    for(int i = 0; i < ids.size() - 1; i++) {
        char_->pushIdentifier(KKIdentifierPtr(new KKIdentifier(ids[i].toULong(), siteId.toStdString())));
    }

    (bodyList[0] == CRDT_INSERT) ? crdt->remoteInsert(char_) : crdt->remoteDelete(char_);
}

void KKFile::applyRemoteCharFormatChange(QStringList bodyList){
    QString siteId = bodyList[1];
    QString text = bodyList[2];
    QStringList ids = bodyList[3].split(" ");
    QString fontStr = bodyList[4];
    QString colorStr = bodyList[5];

    KKCharPtr char_ = KKCharPtr(new KKChar(*text.toLatin1().data(), siteId.toStdString()));
    for(int i = 0; i < ids.size() - 1; i++) {
         // size() - 1 per non considerare l'elemento vuoto della string list ids
        char_->pushIdentifier(KKIdentifierPtr(new KKIdentifier(ids[i].toULong(), siteId.toStdString())));
    }

    crdt->remoteFormatChange(char_,fontStr,colorStr);
}

void KKFile::flushCrdtText()
{
    QStringList crdtText = crdt->saveCrdt();
    if (crdtText.isEmpty()) {
        KKLogger::log("Nothing to flush, CRDT is empty", hash);
        return;
    }

    bool result = file.get()->open(QIODevice::WriteOnly | QIODevice::Text);
    if(result){
        QTextStream stream(file.get());
        for(QString crdtChar : crdtText) {
            stream << QString("%1").arg(crdtChar.length(), 3, 10, QChar('0')) + crdtChar;
        }
        stream << endl;
        file->close();
        KKLogger::log("Flushed succesfully", hash);
    } else {
        KKLogger::log("Error on opening the file", hash);
    }
}

QStringList KKFile::getCrdtText()
{
    return crdt->saveCrdt();
}

int KKFile::getParticipantCounter() const
{
    return participantCounter;
}

void KKFile::handleTimeout()
{
    if (flushCrdt)
        flushCrdtText();

    flushCrdt = participantCounter > 0;
}

void KKFile::initCrdtText()
{
    QStringList text;
    if(file->open(QFile::ReadOnly)) {
        QTextStream stream(file.get());
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            int start = 0;
            int nextFieldLength = 0;
            do {
                nextFieldLength = line.midRef(start, 3).toInt();
                start += 3;
                text.push_back(line.mid(start, nextFieldLength));
                start += nextFieldLength;
            } while (start < line.size());
        }
        if(!text.isEmpty()) {
            crdt->loadCrdt(text);
        }
        crdt->print();
        file.get()->close();
    }
}

