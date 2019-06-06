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
#include "classes/db/kk_db.h"

int main(int argc, char *argv[]) {


    QApplication a(argc, argv);
    kk_db prova;
    prova.db_insert_user("klaus","randomica","klaus.cuko@gmail.com","k","c");
//    kk_server server(3310);
//    Q_UNUSED(server);
    return a.exec();
}


