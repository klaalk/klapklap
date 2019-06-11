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
#define FILE_PATH "./filefolder/"
#include "../../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../db/kk_db.h"

class kk_filesys {

public:
    kk_filesys(){db=kk_db_ptr(new kk_db());}
    bool kk_CreateFile(QString username, QString filename);
    bool kk_OpenFile(QString username, QString filename);
    bool kk_SendFile(QString filename);

private:
    kk_db_ptr db;

};

#endif //KK_FILESYS_H
