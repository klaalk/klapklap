//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_DB_CONNECTOR_H
#define SERVER_DB_CONNECTOR_H

#include "../../../../libs/src/classes_include.h"
#include "../../../../libs/src/classes/crypto/crypto.h"
#include "../SMTP_client/SMTP_client.h"


typedef struct var user_info;


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

    ///inserimento utente. Ritorna 0 successo, -1 username fault, -2 email fault.
    int db_insert_user(std::string username, std::string password,int pass_len, std::string email, std::string name,
                       std::string surname);

    ///inserimento utente. Ritorna 0 successo,
    int db_insert_file(std::string username, std::string filename, std::string path);

    ///inserimento permessi (share file). Ritorna 0 successo,
    int db_share_file(std::string username_from, std::string username_to, std::string filename);

    user_info *db_getUserInfo(std::string username);

    bool db_login(std::string username, std::string password, int psw_len);
    ///reset password ask. Invia una main con hash key dell'user. Ritorna 0 successo,
//    int db_reset_psw_ask(std::string username);

//    ///reset password do. data la hash key permette il reset della password. Ritorna 0 successo, -1 se hash_key errata
//    int db_reset_psw(std::string username, std::string hash_key, std::string new_psw);






};


#endif //SERVER_DB_CONNECTOR_H
