//
// Created by jsnow on 11/05/19.
//
//

#include "db_connector.h"

#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change

db_connector::db_connector(sql::Driver *driver) : driver(driver) {
}

sql::Statement *db_connector::connect(void) {
    con = driver->connect(HOST, USR, PSW);
    con->setSchema("KLAPKLAP_DB");
    return con->createStatement();
}

void db_connector::close(void) {
    con->close();
}

///DEPRECATED
bool db_connector::db_query(std::string query, int n_col) {
    sql::Statement *stmt = connect();
    sql::ResultSet *res = stmt->executeQuery(query);

    while (res->next()) {
        cout << "MySQL says: ";
        for (int i = 1; i <= n_col; ++i)
            cout << res->getString(i) + " ";
        cout << endl;
    }
    close();
    return true;
}

///DEPRECATED
sql::ResultSet *db_connector::db_query(std::string query) {
    sql::Statement *stmt = connect();
    sql::ResultSet *res = stmt->executeQuery(query);
    close();
    return res;
}

////
//// -----> NOTA: QUANDO NON ESEGUI QUERY DEVI FARE EXECUTE E BASTAAAAAAA!
////

int db_connector::db_insert_user(std::string username, std::string password, std::string email, std::string name,
                                 std::string surname) {
    int errCode = 0;
    std::ostringstream _string;
    sql::ResultSet *res;
    sql::Statement *stmt = connect();

    _string << "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`) "
            << "VALUES('" + username + "','" + password + "','" + email + "','" + name + "','" + surname +
               "');";

    try {
        stmt->execute(_string.str());
        _string.str("");
        close();
    } catch (sql::SQLException &e) {
        std::string str(e.what());
        if(str.find("EMAIL")!=string::npos)
            return -1;
        else if(str.find("USERNAME")!=string::npos)
            return -2;

        close();
    }
    return errCode;
}

int db_connector::db_insert_file(std::string username, std::string filename,std::string path){
    std::ostringstream _string;
    sql::Statement *stmt = connect();
    sql::ResultSet *res=stmt->executeQuery("SELECT `ID` FROM `USERS` WHERE `USERNAME`='"+username+"';");
    res->next();

    _string << "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) "
            << "VALUES('" + res->getString(1) + "','" + filename + "','" + path + "');";

    try {
        stmt->execute(_string.str());
        close();
    } catch (sql::SQLException &e) {
        close();
        cout << e.what() << endl ;
        return -1;
    }
    return 0;
}