//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_DB_CONNECTOR_H
#define SERVER_DB_CONNECTOR_H

#include "../classes_include.h"

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
    bool db_query(std::string query, int col_n);
    sql::ResultSet *db_query(std::string query);



};


#endif //SERVER_DB_CONNECTOR_H
