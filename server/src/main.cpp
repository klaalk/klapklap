//
//  main.cpp
//  Server
// prova
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include "./classes/db_connector/db_connector.h"
#include "./classes/db_interface/db_interface.h"
#include "./classes/db_interface/HTTP_listener/HTTP_listener.h"
#include "../src/classes/SMTP_client/SMTP_client.h"
#include "./classes/crdt/crdt_server.h"

#include <QtSql>

#include <cstring>

#include "../../libs/src/classes/crypto/crypto.h"

int main(int argc, char *argv[]) {
//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
    sql::Driver *driver = get_driver_instance();
    db_connector my_conn(driver);
    crypto my_crypto;



    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("130.192.163.109");
    db.setPort(3000);
    db.setDatabaseName("KLAPKLAP_DB");
    db.setUserName("server");
    db.setPassword("password");
    bool ok = db.open();

    int value;
    QSqlQuery query1(db);
//    query1.setForwardOnly(true);
    query1.exec("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD`,`PSWLEN` FROM `USERS`;");
    while (query1.next()) {
        cout << query1.value(1).toInt() << endl;
    }

    return 0;


}


