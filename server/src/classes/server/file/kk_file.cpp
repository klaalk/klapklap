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
//    participants->insert(participant->id, nullptr);
    participants->remove(participant->id);
}

int KKFile::deliver(QString type, QString result, QStringList message, QString username) {
    KKPayloadPtr data = KKPayloadPtr(new KKPayload(type, result, message));
    int code=0; //TODO: check if is better 0

    if (type == CRDT) {
        code = applyRemoteInsertSafe(data->getBodyList());
    } else if (type == CHARFORMAT_CHANGE) {
        code = applyRemoteCharFormatChangeSafe(data->getBodyList());
    } else if(type==ALIGNMENT_CHANGE){
        code = applyRemoteAlignmentChangeSafe(data->getBodyList());
    } else if (type == CHAT || type == REMOVED_PARTECIPANT || type == ADDED_PARTECIPANT) {
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
    return code;
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

int KKFile::applyRemoteAlignmentChangeSafe(QStringList bodyList){
    try {
        applyRemoteAlignmentChange(bodyList);
        return 200;
    } catch (QException e) {
        KKLogger::log(e.what(),"applyRemoteInsertSafe");
        return -200;
    }
}



int KKFile::applyRemoteInsertSafe(QStringList bodyList){
    try {
        applyRemoteInsert(bodyList);
        return 200;
    } catch (QException e) {
        KKLogger::log(e.what(),"applyRemoteInsertSafe");
        return -200;
    }
}

int KKFile::applyRemoteCharFormatChangeSafe(QStringList bodyList){
    try {
        applyRemoteCharFormatChange(bodyList);
        return 200;
    } catch (QException e) {
       KKLogger::log(e.what(),"applyRemoteCharFormatChangeSafe");
       return -200;
    }

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

void KKFile::applyRemoteAlignmentChange(QStringList bodyList){
    QString alignment=bodyList[0];
    QString startAlignLine=bodyList[1];
    QString endAlignLine=bodyList[2];

    for(unsigned long i=startAlignLine.toULong();i<=endAlignLine.toULong();i++){ //per ogni riga si crea la posizione globale dell'inizio della riga e chiama la alignmentRemoteChange
        if (crdt->checkLine(i) || (crdt->isTextEmpty() && i==0)) { //controlla che la riga esista
            crdt->setLineAlignment(static_cast<long>(i),alignment.toULong());
        } else {
            break;
        }
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

