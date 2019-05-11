//
// Created by jsnow on 11/05/19.
//
//

#include "db_connector.h"
#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change

db_connector::db_connector(sql::Driver *driver): driver(driver){}

sql::Statement * db_connector::connect(void) {
    con = driver->connect(HOST, USR, PSW);
    con->setSchema("KLAPKLAP_DB");
    return con->createStatement();
}

void db_connector::close(void) {
    con->close();
}

  bool db_connector::db_query(std::string query){
    sql::Statement *stmt = connect();
    sql::ResultSet * res = stmt->executeQuery(query);

    while (res->next()) {
        cout << "MySQL says: ";
        /* Access column data by numeric offset, 1 is the first column */
        cout << res->getString(1)
//        + " " << res->getString(2) + " "<<res->getString(3) + " "<<res->getString(4) + " "<< res->getString(5) + " " << res->getString(6) + " "<<res->getString(7) + " "<<res->getString(8) + " "
        <<endl;
    }
    close();
}