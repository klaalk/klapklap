//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

#include <QThreadPool>
#include <QVariant>

KKFile::KKFile(QObject *parent): QObject(parent) {
    chatMessages = KKVectorPayloadPtr(new QVector<KKPayload>());
    messages = KKVectorPairPayloadPtr(new QVector<QPair<KKPayload, QString>>());
    participants = KKMapParticipantPtr(new QMap<QString, KKParticipantPtr>());
    crdt = KKCrdtPtr(new KKCrdt("file", casuale));

    messagesTask = new KKTask([&]() {
        consumeMessages();
    });
    messagesTask->setAutoDelete(true);

    // Set autosave timer
    timer = QSharedPointer<QTimer>(new QTimer());
    timer->start(10000);
    connect(timer.get(), &QTimer::timeout, this, &KKFile::handleTimeout);
    connect(messagesTask->thread(), &QThread::finished, this, &QObject::deleteLater);

}

KKFile::~KKFile() {
    timer->stop();
    flushCrdtText();
    crdt = nullptr;
    file = nullptr;
    timer = nullptr;
    KKLogger::log("File deconstructed", hash);
}

bool KKFile::contains(KKParticipantPtr participant) {
    bool contains = false;
    participantsMutex.lock();
    contains = participants->contains(participant->id);
    participantsMutex.unlock();
    return contains;
}

void KKFile::join(KKParticipantPtr participant) {
    participantsMutex.lock();
    // Lo inserisco tra i partecipanti del file
    participants->insert(participant->id, participant);

    // Invio gli ultimi messaggi della chat (al massimo 100)
    QVector<KKPayload> chatMessages = getChatMessages();
    if(chatMessages.length() > 0) {
        std::for_each(chatMessages.begin(), chatMessages.end(), [&](KKPayload chatMessage){
            participant->deliver(chatMessage, true);
        });
    }

    // Invio la lista dei partecipante con lo stato aggiornato
    participant->deliver(KKPayload(SET_PARTECIPANTS, SUCCESS, getParticipants()));

    // Invio il crdt
    participant->deliver(KKPayload(LOAD_FILE, SUCCESS, getCrdtText()));

    participantsMutex.unlock();
}

void KKFile::leave(KKParticipantPtr participant) {
    participantsMutex.lock();
    participants->remove(participant->id);
    participantsMutex.unlock();
}

int KKFile::deliverMessages(KKPayload data, QString username) {
    participantsMutex.lock();
    int result = 0;

    QString type = data.getRequestType();
    if (!participants->isEmpty()) {
        std::for_each(participants->begin(), participants->end(),
                      [&](QSharedPointer<KKParticipant> participant){
            if (participant->id != username) {
                participant->deliver(data);
            }
        });
    }

    if (type == CRDT) {
        result = changeCrdtText(data.getBodyList());
    }

    if (type == CHAT) {
        chatMutex.lock();
        chatMessages->push_back(data);

        while (chatMessages->size() > MaxRecentMessages)
            chatMessages->pop_front();

        chatMutex.unlock();
    }

    participantsMutex.unlock();
    return result;
}

void KKFile::consumeMessages()
{
    for (;;) {
        messagesMutex.lock();
        while (messages->isEmpty()) {
            messagesWait.wait(&messagesMutex);
        }
        QPair<KKPayload, QString> item = messages->takeFirst();
        messagesMutex.unlock();

        if (item.first.getRequestType() == CLOSE_FILE) {
            break;
        } else {
            KKPayload data = item.first;
            deliverMessages(item.first, item.second);
        }
    }
}

void KKFile::produceMessages(KKPayload action, QString username)
{
    messagesMutex.lock();
    {
        bool wasEmpty = messages->isEmpty();
        messages->push_back(QPair<KKPayload, QString>(action, username));
        if (wasEmpty) {
            messagesWait.wakeOne();
        }
    }
    messagesMutex.unlock();
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

void KKFile::initFile()
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

        file.get()->close();
    }
    QThreadPool::globalInstance()->start(messagesTask);
    KKLogger::log("File constructed", hash);
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
                    crdt->remoteAlignmentChange(i, alignment);
                }
            }
        } else {
            for (QString crdtChar : body) {
                KKCharPtr charPtr = crdt->decodeCrdtChar(crdtChar);
                bool value = false;
                if (operation == CRDT_INSERT) {
                    crdt->remoteInsert(charPtr);
                } else if (operation == CRDT_DELETE) {
                    crdt->remoteDelete(charPtr, &value);
                } else if (operation == CRDT_FORMAT) {
                    crdt->remoteFormatChange(charPtr);
                }
            }
        }
        crdtMutex.unlock();
        return 200;
    } catch (QException e) {
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

QVector<KKPayload> KKFile::getChatMessages()
{
    chatMutex.lock();
    QVector<KKPayload> chatMessages = QVector<KKPayload>(*this->chatMessages.get());
    chatMutex.unlock();
    return chatMessages;
}

void KKFile::handleTimeout() {
    if (participants->size() > 0) {
        flushCrdtText();
    }
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
