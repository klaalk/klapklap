//
// Created by michele on 09/06/2019.
//

#include "kk_filesys.h"

KKFileSystem::KKFileSystem(KKDataBasePtr db): db(db){
    // Preparo le folders per il file system

    if (!QDir().exists(SERVER_ROOT)) {
        QDir().mkdir(SERVER_ROOT);
    }
    if (!QDir().exists(LOG_ROOT)) {
        QDir().mkdir(LOG_ROOT);
    }
    if (!QDir().exists(APPLICATION_ROOT)) {
        QDir().mkdir(APPLICATION_ROOT);
    }
}

KKFileSystem::~KKFileSystem() {}

QString KKFileSystem::createFile(QString username, QString filename){

    if(username == FILE_SYSTEM_USER && filename == LOG_FILE) {
        this->logFileName = LOG_ROOT + QDateTime::currentDateTime().toString("dd.MM.yyyy") + "_log.txt";
        QFile file(this->logFileName);
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        return this->logFileName;
    }

    //FIXME: FILE DI TEST PER TEXT EDITOR. TODO: rimuovere una volta finito
    if (filename.startsWith("test")) {
        return filename;
    }

    SimpleCrypt crypt(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString tmp;

    // Serve ad evitare che il carattere "/" dia problemi nei path
    do {
        tmp = crypt.encryptToString(username);
    } while(crypt.containLetter('/', tmp));

    // Serve per garantire l'univocità del path.
    QString jump;
    jump=crypt.random_psw(jump);

    QString _filename = jump + "@" + tmp + "@" + filename;
    QFile file(APPLICATION_ROOT + _filename);
    file.open(QIODevice::ReadWrite | QIODevice::Text);

    UserInfo *user = new UserInfo;
    int result = db->insertUserFile(username, _filename, APPLICATION_ROOT + _filename, user);

    if(result == DB_INSERT_FILE_SUCCESS) {
        db->sendInsertUserFileEmail(user->username, user->email, user->name, user->surname, _filename);
        return _filename;
    }

    return FILE_SYSTEM_CREATE_ERROR;
}

bool KKFileSystem::openFile(QString username, QString filename){

    //FIXME: FILE DI TEST PER TEXT EDITOR. TODO: rimuovere una volta finito
    if (filename.startsWith("test")) {
        UserInfo *user = new UserInfo;
        return db->insertUserFile(username, filename, APPLICATION_ROOT + filename, user);
    }

    SimpleCrypt crypt(Q_UINT64_C(0x0c2ad4a4acb9f023));
    // 0=random_jump, 1=crypted username 2=effective filename
    QStringList body = filename.split("@");
    QString _username = crypt.decryptToString(body[1]);

    if (db->checkUserInfoByUsername(_username) != DB_USER_FOUND) return false;

    //Gia presente nella tabella, avendo
    if(username==_username) return true;

    UserInfo *user = new UserInfo;
    // Sto aprendo un file al quale sono invitato, devo tenerne traccia sul db
    int result = db->insertUserFile(username, filename, APPLICATION_ROOT + filename, user);
    if (result == DB_INSERT_FILE_SUCCESS) {
        db->sendInsertUserFileEmail(user->username,user->email,user->name, user->surname, filename);
        return true;
    }
    return false;
}

bool KKFileSystem::sendFile(QString filename){

    QFile file(APPLICATION_ROOT +filename);
    file.open(QFile::ReadOnly);

    //@klaus
    //va inviato il file tramite la sock
    return true;
}

// In pratica, come dicevamo, creiamo un file con "jump_crypt(username)_filename.txt".
// dove jump è una sringa causale da (26+10)^20 combinazioni che ci garantisce univocità per
// il filename completo. Viene chiamata la open file che: se lo user non esiste da false,
// se user è proprietario del file da true, non c'e bisogno di fare altre operazioni,
// se l'user non è proprietario vuol dire che è stato invitato, quindi va aggiunto,
// il valore ritornato dipende da se riesce o meno la query nel db.
// SendFile prende il file e lo invia sulla sock che usiamo per comunicare
// con header "file_response" e payload "<binary file content>"

bool KKFileSystem::writeFile(QString filename, QString toPrint) {

    if(filename == LOG_FILE) {
        filename = this->logFileName;
        toPrint.insert(0, QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss - "));
        qDebug() << "[log] " + toPrint;
    }

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return false;
    }

    QTextStream stream(&file);
    stream << toPrint << endl;
    file.close();
    return true;
}


QString KKFileSystem::readFile(QString filename){
    QString text;
    QFile file(filename);
    if(!file.open(QFile::ReadOnly)) {
        return text;
    }

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        text+= stream.readLine();
        text+= " ";
    }

    file.close();
    return text;
}












