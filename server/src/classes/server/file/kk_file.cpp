//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

#include <QVariant>

KKFile::KKFile(QObject *parent): QObject(parent) {
    recentMessages = KKVectorPayloadPtr(new QVector<KKPayloadPtr>());
    participants = KKMapParticipantPtr(new QMap<QString, KKParticipantPtr>());
    crdt = KKCrdtPtr(new KKCrdt("file", casuale));

    // Set autosave timer
    timer = QSharedPointer<QTimer>(new QTimer());
    timer->start(10000);
    connect(timer.get(), &QTimer::timeout, this, &KKFile::handleTimeout);
}

KKFile::~KKFile() {
    timer->stop();
    flushCrdtText();
    KKLogger::log("File deconstructed", hash);
}

void KKFile::join(KKParticipantPtr participant) {
    participants->insert(participant->id, participant);
}

void KKFile::leave(KKParticipantPtr participant) {
    participants->remove(participant->id);
}

int KKFile::deliver(QString type, QString result, QStringList message, QString username) {
    KKPayloadPtr data = KKPayloadPtr(new KKPayload(type, result, message));

    if (type == CHAT || type == REMOVED_PARTECIPANT || type == ADDED_PARTECIPANT) {
        recentMessages->push_back(data);
    }

    while (recentMessages->size() > MaxRecentMessages)
        recentMessages->pop_front();

    if (!participants->isEmpty()) {
        std::for_each(participants->begin(), participants->end(),[&](QSharedPointer<KKParticipant> p){
            if(p->id != username) {
                p->deliver(data);
            }
        });
    }
    return 0;
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

void KKFile::addUser(QString user)
{
    if (!user.isEmpty()) {
        users.push_back(user);
    }
}

void KKFile::setUsers(QStringList users_)
{
    users = users_;
}

QStringList KKFile::getUsers()
{
    return users;
}

int KKFile::applyRemoteTextChangeSafe(QStringList body){
    try {
        // Ottengo i campi della risposta
        QString operation = body.takeFirst();
        QString siteId = body.takeFirst();
        body.pop_front();

        if (operation == CRDT_ALIGNM) {
            while (!body.isEmpty()) {
                // Per ogni riga si crea la posizione globale dell'inizio della riga e chiama la alignmentRemoteChange
                int alignment = body.takeFirst().toInt();
                unsigned long startLine = body.takeFirst().toULong();
                unsigned long endLine = body.takeFirst().toULong();

                for(unsigned long i = startLine; i <= endLine; i++) {
                    if(!crdt->remoteAlignmentChange(i, alignment))
                        break;
                }
            }

        } else {
            for (QString crdtChar : body) {
                KKCharPtr charPtr = crdt->decodeCrdtChar(crdtChar);
                if (operation == CRDT_INSERT) {
                    crdt->remoteInsert(charPtr);
                } else if (operation == CRDT_DELETE) {
                    crdt->remoteDelete(charPtr);
                } else if (operation == CRDT_FORMAT) {
                    crdt->remoteFormatChange(charPtr);
                }
            }
        }
        return 200;
    } catch (QException e) {
        KKLogger::log(e.what(), "applyRemoteChangeSafe");
        qDebug() << "ERROR :" << e.what();
        return -200;
    }
}

void KKFile::flushCrdtText()
{
    bool result = file.get()->open(QIODevice::WriteOnly | QIODevice::Text);
    if(result){
        QTextStream stream(file.get());
        // Scrivo il crdt
        QStringList crdtText = crdt->encodeCrdt();
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
    return crdt->encodeCrdt();
}

QStringList KKFile::getParticipants()
{
    // Rispondo con la list di tutti partecipanti al file (attivi o non attivi)
    QStringList participants_;

    for(QString user : users) {
        QString id = user.split(":").at(0);
        QString state = participants->find(id) != participants->end() ? PARTICIPANT_ONLINE : PARTICIPANT_OFFLINE;
        participants_.push_back(user + ":" + state);
    }

    return participants_;
}

bool KKFile::partecipantExist(QString username){
    return participants->contains(username);
}

int KKFile::getPartecipantsNumber(){
    return participants->size();
}

KKVectorPayloadPtr KKFile::getRecentMessages()
{
    return recentMessages;
}

int KKFile::getParticipantCounter() const
{
    return participants->size();
}

void KKFile::handleTimeout() {
    if (participants->size() > 0) {
        flushCrdtText();
    }
}

void KKFile::initCrdtText()
{
    QStringList text;
    if(file->open(QFile::ReadOnly)) {
        QTextStream stream(file.get());
        QString line = stream.readAll();
        line.remove('\r');

        int start = 0;
        int nextFieldLength = 0;
        do {
            nextFieldLength = line.midRef(start, 3).toInt();
            start += 3;
            text.push_back(line.mid(start, nextFieldLength));
            start += nextFieldLength;
        } while (start < line.size()-1);


        if(!text.isEmpty()) {
            crdt->decodeCrdt(text);
        }
        file.get()->close();
    }
}

