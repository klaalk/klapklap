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
    kk_filesys(kk_db_ptr db): db(db){}
    kk_filesys(){}
    bool kk_CreateFile(QString username, QString filename);
    bool kk_OpenFile(QString username, QString filename);
    bool kk_SendFile(QString filename);
    bool kk_WriteFile(QString filename, QString toPrint);
    QString kk_ReadFile(QString filename);


private:
    kk_db_ptr db;
    QString log_name;

};

#endif //KK_FILESYS_H
