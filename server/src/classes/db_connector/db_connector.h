//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_DB_CONNECTOR_H
#define SERVER_DB_CONNECTOR_H
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;



class db_connector {
private:
    sql::Driver *driver;
    sql::Connection *con;
    ///Open connection. Private.
    sql::Statement *connect(void);
    void close(void);
public:
    ///Costruttore
    explicit db_connector(sql::Driver *driver);

    ///Generic query exec
    bool db_query(std::string query);




};


#endif //SERVER_DB_CONNECTOR_H
