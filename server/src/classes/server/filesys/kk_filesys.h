//
// Created by michele on 09/06/2019.
//

#ifndef KK_FILESYS_H
#define KK_FILESYS_H

#include <set>
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <QSharedPointer>
#include <QWebSocket>
#include <QFile>

#include "../../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../db/kk_db.h"

#define FILE_PATH "./filefolder/"

class KKFileSystem {
public:
    KKFileSystem(KKDataBasePtr db): db(db){}
    KKFileSystem(){}
    QString createFile(QString username, QString filename);
    bool openFile(QString username, QString filename);
    bool sendFile(QString filename);
    bool writeFile(QString filename, QString toPrint);
    QString readFile(QString filename);

private:
    KKDataBasePtr db;
    QString logFileName;
};

typedef QSharedPointer<KKFileSystem> KKFileSystemPtr;

#endif //KK_FILESYS_H
