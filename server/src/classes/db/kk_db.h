//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_KK_DB_H
#define SERVER_KK_DB_H

#include "../../../../libs/src/classes_include.h"
#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../smtp/kk_smtp.h"



typedef struct var user_info;


class kk_db {


private:
    sql::Driver *driver;
    sql::Connection *con;

    ///Open connection. Private.
    sql::Statement *connect(void);

    void close(void);

public:
    ///Costruttore
    explicit kk_db(sql::Driver *driver);

    ///Generic query exec
    bool db_query(std::string query, int col_n);

    sql::ResultSet *db_query(std::string query);

    ///inserimento utente. Ritorna 0 successo, -1 username fault, -2 email fault.
    int db_insert_user(std::string username, std::string password, std::string email, std::string name,
                       std::string surname);

    ///inserimento utente. Ritorna 0 successo,
    int db_insert_file(std::string username, std::string filename, std::string path);

    ///inserimento permessi (share file). Ritorna 0 successo,
    int db_share_file(std::string username_from, std::string username_to, std::string filename);

    user_info *db_getUserInfo(std::string username);

    bool db_login(std::string username, QString password);

    ///reset password ask. invia una mail con password temporanea
    int db_reset_psw(std::string username);

    ///reset password ask. cambia password
    int db_update_psw(std::string username, QString new_psw);









};


#endif //SERVER_KK_DB_H
