//
// Created by michele on 09/06/2019.
//

#include "kk_filesys.h"

KKFileSystem::KKFileSystem(QObject *parent):QObject(parent),
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

KKFilePtr KKFileSystem::createFile(QString filename, QString username){
    // Serve per garantire l'univocità del path.
    QString jump;
    jump = crypter->random_psw(jump);

    // Genero il nome file (containLetter('/', _filename) serve ad evitare che il carattere "/" dia problemi nei path)
    QString hash;
    do {
        hash = crypter->encryptToString(jump + FILENAME_SEPARATOR + username + FILENAME_SEPARATOR + filename);
    } while(crypter->containLetter('/', hash));
    KKLogger::log(QString("File >%1< con hash >%2< creato").arg(filename, hash), "FILE SYSTEM");
    return openFile(hash);
}

KKFilePtr KKFileSystem::openFile(QString hash, QString rootPath){
    KKFilePtr file = QSharedPointer<KKFile>(new KKFile(this));
    QSharedPointer<QFile> qfile = QSharedPointer<QFile>(new QFile (rootPath + hash));
    file->setFile(qfile);
    file->setHash(hash);
    KKLogger::log(QString("File con hash >%1< aperto").arg(hash), "FILE SYSTEM");

    return  file;
}

bool KKFileSystem::writeFile(KKFilePtr file, QString toPrint) {
    QFile *tmp = file->getFile().get();
    bool result = tmp->open(QIODevice::ReadWrite | QIODevice::Text);
    if(result){
        QTextStream stream(file->getFile().get());
        stream << toPrint << endl;
        tmp->close();
        return true;
    } else
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










