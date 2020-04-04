//
// Created by michele on 09/06/2019.
//

#include "kk_filesys.h"

KKFileSystem::KKFileSystem(){
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
    return openFile(filename);


}

KKFilePtr KKFileSystem::openFile(QString filename, QString rootPath){
    KKFilePtr kkFile = QSharedPointer<KKFile>(new KKFile());
    QSharedPointer<QFile> file = QSharedPointer<QFile>(new QFile (rootPath + filename));
    kkFile->setFile(file);
    kkFile->setFilename(filename);
    return  kkFile;
}

bool KKFileSystem::sendFile(QString filename){

    QFile file(APPLICATION_ROOT +filename);
    file.open(QFile::ReadOnly);

    //@klaus
    //va inviato il file tramite la sock
    return true;
}


bool KKFileSystem::writeFile(KKFilePtr file, QString toPrint) {
    QFile *tmp = file->getFile().get();
    bool result = tmp->open(QIODevice::ReadWrite | QIODevice::Text);
    if(result){
        if(file->getFilename() == logFileName) {
            toPrint.insert(0, QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss - "));
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












