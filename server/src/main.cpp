//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <string>

//#include <QtSql>
#include "./classes/smtp/kk_smtp.h"
#include "classes/server/kk_server.h"
#include "classes/db/kk_db.h"
/*
#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../libs/src/classes/crypt/kk_crypt.h"*/

#include <QApplication>
#include <QDebug>
#define STD_Q(x) QString::fromStdString(x)

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    kk_server server(3310);
    Q_UNUSED(server);

//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
//    kk_db my_conn;

//    my_conn.db_insert_user("klauscuko","ciao", "klaus.c96@gmail.com","klaus", "cuko");
    return a.exec();


}


