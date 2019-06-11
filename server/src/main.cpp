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

int main(int argc, char *argv[]) {


    QApplication a(argc, argv);
    kk_server server(3002);
    Q_UNUSED(server);


    kk_filesys filesys;
    for (int i =0;i<500;i++) {
        filesys.kk_CreateFile("grecomichele96","prova.txt");
    }

    qDebug() << "done";

    return a.exec();
}


