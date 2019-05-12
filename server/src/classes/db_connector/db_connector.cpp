//
// Created by jsnow on 11/05/19.
//
//

#include "db_connector.h"
#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change

db_connector::db_connector(sql::Driver *driver): driver(driver){
}

sql::Statement * db_connector::connect(void) {
    con = driver->connect(HOST, USR, PSW);
    con->setSchema("KLAPKLAP_DB");
    return con->createStatement();
}

void db_connector::close(void) {
    con->close();
}

bool db_connector::db_query(std::string query,int n_col){
    sql::Statement *stmt = connect();
    sql::ResultSet * res = stmt->executeQuery(query);

    while (res->next()) {
        cout << "MySQL says: ";
        for (int i = 1; i <= n_col; ++i)
            cout << res->getString(i) + " ";
        cout<<endl;
    }
    close();
    return true;
}

sql::ResultSet* db_connector::db_query(std::string query){
    sql::Statement *stmt = connect();
    sql::ResultSet * res = stmt->executeQuery(query);
    close();
    return res;
}