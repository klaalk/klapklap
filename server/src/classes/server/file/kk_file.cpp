//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

#include <QThreadPool>
#include <QVariant>

KKFile::KKFile(QObject *parent): QObject(parent) {
    QThreadPool::globalInstance()->setMaxThreadCount(5);

    chatMessages = KKVectorPayloadPtr(new QVector<KKPayload>());
    crdtMessages = KKVectorPairPayloadPtr(new QVector<QPair<KKPayload, QString>>());
    participants = KKMapParticipantPtr(new QMap<QString, KKParticipantPtr>());
    crdt = KKCrdtPtr(new KKCrdt("file", casuale));

    crdtMessagesTask = new KKTask([&]() {
        consumeCrdtMessages();
    });
    crdtMessagesTask->setAutoDelete(true);

    // Set autosave timer
    timer = QSharedPointer<QTimer>(new QTimer());
    timer->start(10000);
    connect(timer.get(), &QTimer::timeout, this, &KKFile::handleTimeout);
}

KKFile::~KKFile() {
    timer->stop();
    produceCrdtMessages(KKPayload(CLOSE_FILE, NONE, {CRDT_CLOSE}), "All");
    QThreadPool::globalInstance()->waitForDone();
    flushCrdtText();
    KKLogger::log("File deconstructed", hash);
}

void KKFile::join(KKParticipantPtr participant) {
    participantsMutex.lock();
    participant->deliver(KKPayload(OPEN_FILE, SUCCESS, { "File aperto con successo, partecipazione confermata", getHash()}));
    participant->deliver(KKPayload(SET_PARTECIPANTS, SUCCESS, getParticipants()));

    // Aggiorno con gli ultimi messaggi mandati.
    KKVectorPayloadPtr queue = getRecentMessages();
    if(queue->length() > 0) {
        std::for_each(queue->begin(), queue->end(), [&](KKPayload d){
           participant->deliver(d.encode());
        });
    }
    participant->deliver(KKPayload(LOAD_FILE, SUCCESS, getCrdtText()));
    participants->insert(participant->id, participant);
    participantsMutex.unlock();
    KKLogger::log("JOINED", "FILE");
}

void KKFile::leave(KKParticipantPtr participant) {
    participantsMutex.lock();
    participants->remove(participant->id);
    participantsMutex.unlock();
}

int KKFile::deliver(KKPayload data, QString username) {

    QString type = data.getRequestType();
     KKLogger::log("ABOUT TO DELIVER","FILE");
    if (type == CHAT || type == REMOVED_PARTECIPANT || type == ADDED_PARTECIPANT) {
        chatMessages->push_back(data);
    }

    while (chatMessages->size() > MaxRecentMessages)
        chatMessages->pop_front();

    participantsMutex.lock();
    if (!participants->isEmpty()) {
        std::for_each(participants->begin(), participants->end(),[&](QSharedPointer<KKParticipant> p){
            if(p->id != username) {
                p->deliver(data);
            }
        });
    }
    changeCrdtText(data.getBodyList());
    KKLogger::log("CHANGED", "FILE");
    participantsMutex.unlock();
    return 0;
}

void KKFile::consumeCrdtMessages()
{
    for (;;) {
        crdtMessagesMutex.lock();
        while (crdtMessages->isEmpty()) {
            KKLogger::log("WAIT", "FILE");
            crdtMessagesWait.wait(&crdtMessagesMutex);
        }
        QPair<KKPayload, QString> item = crdtMessages->takeFirst();
        crdtMessagesMutex.unlock();

        if (item.first.getRequestType() == CLOSE_FILE) {
            break;
        } else {
            deliver(item.first, item.second);
        }
    }
}

void KKFile::produceCrdtMessages(KKPayload action, QString username)
{
    crdtMessagesMutex.lock();
    {
        bool wasEmpty = crdtMessages->isEmpty();
        crdtMessages->push_back(QPair<KKPayload, QString>(action, username));
        if (wasEmpty) {
            crdtMessagesWait.wakeOne();
        }
    }
    crdtMessagesMutex.unlock();
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
            crdtMutex.lock();
            crdt->decodeCrdt(text);
            crdtMutex.unlock();
        }

        QThreadPool::globalInstance()->start(crdtMessagesTask);
        file.get()->close();
    }
}

void KKFile::flushCrdtText()
{
    bool result = file.get()->open(QIODevice::WriteOnly | QIODevice::Text);
    if(result){
        QTextStream stream(file.get());

        // Scrivo il crdt
        QStringList crdtText = getCrdtText();

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


int KKFile::changeCrdtText(QStringList body){
    crdtMutex.lock();
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
        crdtMutex.unlock();
        return 200;
    } catch (QException e) {
        KKLogger::log(e.what(), "applyRemoteChangeSafe");
        qDebug() << "ERROR :" << e.what();
        crdtMutex.unlock();
        return -200;
    }

}

QStringList KKFile::getCrdtText()
{
    crdtMutex.lock();
    QStringList crdtText = crdt->encodeCrdt();
    crdtMutex.unlock();
    return crdtText;
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

int KKFile::getPartecipantsNumber() {
    participantsMutex.lock();
    long size = participants->size();
    participantsMutex.unlock();
    return size;
}

bool KKFile::partecipantExist(QString username){
    participantsMutex.lock();
    bool contains = participants->contains(username);
    participantsMutex.unlock();
    return contains;
}

KKVectorPayloadPtr KKFile::getRecentMessages()
{
    return chatMessages;
}

void KKFile::handleTimeout() {
    if (participants->size() > 0) {
        flushCrdtText();
    }
}
