//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include "classes/db/kk_db.h"
#include <boost/chrono.hpp>
#include <boost/asio.hpp>
#include <stdlib.h>
#include <string>

//#include <QtSql>
#include "./classes/smtp/kk_smtp.h"
#include "classes/server/kk_server.h"
#include "classes/db/kk_db.h"
#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../libs/src/classes/crypt/kk_crypt.h"

#include <QApplication>
#include <QDebug>
#define STD_Q(x) QString::fromStdString(x)

int main(int argc, char *argv[]) {
//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
    sql::Driver *driver = get_driver_instance();
//    kk_db my_conn(driver);
//
//    SimpleCrypt test(Q_UINT64_C(0x0c2ad4a4acb9f023));
//    QString mammt = "passwordsicura";
//    my_conn.db_update_psw("michele6000",test.encryptToString(mammt));

//    kk_crdt *crdt = new kk_crdt("Canguro", casuale);
//    crdt->local_insert('c', kk_pos(0, 0));
//    crdt->local_insert('a', kk_pos(0, 1));
//    crdt->local_insert('o', kk_pos(0, 2));
//    crdt->local_insert('i', kk_pos(0, 3));
//    crdt->local_insert('a', kk_pos(0, 4));
//    crdt->local_insert('a', kk_pos(0, 5));
//    crdt->local_insert('a', kk_pos(0, 1));
//    crdt->print();



    try {
        boost::asio::io_service io_service;
        // SOLO UNA PORTA APERTA
        int port = 3002;
        tcp::endpoint endpoint(tcp::v4(), port);
        kk_server_ptr server(new kk_server(io_service, endpoint, driver));
        io_service.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;


}


