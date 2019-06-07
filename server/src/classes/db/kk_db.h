//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_KK_DB_H
#define SERVER_KK_DB_H

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../smtp/kk_smtp.h"
#include <QtSql>
#include <QSqlDatabase>

typedef struct var user_info;


class kk_db {
private:
    QSqlDatabase db;
public:

    ///Costruttore
    explicit kk_db();

    ///Generic query exec
    bool db_query(QString query);


    ///inserimento utente. Ritorna 0 successo, -1 username fault, -2 email fault.
    int db_insert_user(QString username, QString password, QString email, QString name,
                       QString surname);

    ///inserimento utente. Ritorna 0 successo,
    int db_insert_file(QString username, QString filename, QString path);

    ///inserimento permessi (share file). Ritorna 0 successo,
    int db_share_file(QString username_from, QString username_to, QString filename);

    user_info *db_getUserInfo(QString username);

    bool db_login(QString username, QString password);

    ///reset password ask. invia una mail con password temporanea
    int db_reset_psw(QString username);

    ///reset password ask. cambia password
    int db_update_psw(QString username, QString new_psw);

};

typedef std::shared_ptr<kk_db> kk_db_ptr;
#endif //SERVER_KK_DB_H
