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
#include <QDir>
#include <QStandardPaths>

#include "../../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../../../../libs/src/constants/kk_constants.h"

#include <classes/server/file/kk_file.h>

class KKFileSystem {
public:
    KKFileSystem();
    ~KKFileSystem();

    KKFilePtr createFile(QString username, QString filename);
    KKFilePtr openFile(QString filename, QString rootPath=APPLICATION_ROOT);
    QString readFile(QString filename);
    bool writeFile(KKFilePtr file, QString toPrint);

private:
    KKCryptPtr crypter;

};

typedef QSharedPointer<KKFileSystem> KKFileSystemPtr;



#endif //KK_FILESYS_H
