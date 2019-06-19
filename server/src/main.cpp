//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <QApplication>
#include <QDebug>

#include "classes/server/kk_server.h"

#include "classes/server/filesys/kk_filesys.h"
#include "classes/db/kk_db.h"

int main(int argc, char *argv[]) {

qDebug() << "touch ./" + QDateTime::currentDateTime().toString("dd.MM.yyyy_hh.mm.ss") + "_log.txt" << endl;
//    QApplication a(argc, argv);
//    kk_server server(3002);
//    Q_UNUSED(server);

//    kk_db_ptr temporary = kk_db_ptr(new kk_db());
//    kk_filesys filesys(temporary);

//    for (int i =0;i<5;i++) {
//        filesys.kk_CreateFile("grecomichele96","prova.txt");
//    }

//    filesys.kk_OpenFile("klaus.cuko","2qmWqBlQ9wQj99nsQzld_AwIJdRfcCrvykd9GnFiZIW5bTg==_prova.txt");

//    qDebug() << "done";

//    return a.exec();
}


