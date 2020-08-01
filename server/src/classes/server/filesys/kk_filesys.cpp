//
// Created by michele on 09/06/2019.
//

#include "kk_filesys.h"

KKFileSystem::KKFileSystem():
    crypter(KKCryptPtr(new KKCrypt(Q_UINT64_C(0x0c2ad4a4acb9f023))))
{
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

KKFilePtr KKFileSystem::createFile(QString username, QString filename){
    if(username == FILE_SYSTEM_USER && filename == LOG_FILE) {
        logFileName = QDateTime::currentDateTime().toString("dd.MM.yyyy") + "_log.txt";
        return openFile(logFileName, LOG_ROOT);
    }

    //FIXME: FILE DI TEST PER TEXT EDITOR. TODO: rimuovere una volta finito
    if (filename.startsWith("test")) {
        return openFile(filename);
    }

    // Serve per garantire l'univocità del path.
    QString jump;
    jump = crypter->random_psw(jump);
    QString _filename;

    do {
        _filename = crypter->encryptToString(jump + FILENAME_SEPARATOR + username + FILENAME_SEPARATOR + filename);
    }
    // Serve ad evitare che il carattere "/" dia problemi nei path
    while(crypter->containLetter('/', _filename));

    return openFile(_filename);
}

KKFilePtr KKFileSystem::openFile(QString filename, QString rootPath){
    KKFilePtr kkFile = QSharedPointer<KKFile>(new KKFile());
    QSharedPointer<QFile> file = QSharedPointer<QFile>(new QFile (rootPath + filename));
    kkFile->setFile(file);
    kkFile->setFilename(filename);
    return  kkFile;
}

bool KKFileSystem::writeFile(KKFilePtr file, QString toPrint) {
    return writeFile(file, toPrint, nullptr);
}

bool KKFileSystem::writeFile(KKFilePtr file, QString toPrint, QString sessionId) {
    QFile *tmp = file->getFile().get();
    bool result = tmp->open(QIODevice::ReadWrite | QIODevice::Text);
    if(result){
        if(file->getFilename() == logFileName) {
            toPrint.insert(0, QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss - [") + sessionId + "] - ");
            qDebug() << "[log] " + toPrint;
        }

        QTextStream stream(file->getFile().get());
        stream << toPrint << endl;
        tmp->close();
        return true;
    }else
        return false;
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












