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

    kk_filesys log;
    QString run_info="";
    run_info = "RUNNING (Version:" +  QString::number(VERSION_MAJOR) +  "." + QString::number(VERSION_MINOR) + " Build: "
            + QString::number(VERSION_BUILD)+")";

    log.kk_CreateFile("root","log");
    log.kk_WriteFile("log",run_info);

    qDebug() << run_info;

    QApplication a(argc, argv);
    kk_server server(3002);
    Q_UNUSED(server);



    return a.exec();
}


