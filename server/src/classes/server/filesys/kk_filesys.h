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
#include "../file/kk_file.h"


#define SERVER_ROOT QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/KKServer"
#define LOG_ROOT SERVER_ROOT + "/log/"
#define APPLICATION_ROOT SERVER_ROOT + "/application/"


class KKFileSystem {
public:
    KKFileSystem();
    ~KKFileSystem();

    KKFilePtr createFile(QString username, QString filename);
    KKFilePtr openFile(QString filename, QString rootPath=APPLICATION_ROOT);
    bool sendFile(QString filename);
    bool writeFile(KKFilePtr file, QString toPrint);
    QString readFile(QString filename);

private:
    QString logFileName;
};

typedef QSharedPointer<KKFileSystem> KKFileSystemPtr;



#endif //KK_FILESYS_H
