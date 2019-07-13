//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_KK_DB_H
#define SERVER_KK_DB_H

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../smtp/kk_smtp.h"
#include <QtSql>
#include <QSqlDatabase>

typedef struct record UserInfo;

class KKDataBase {
private:
    QSqlDatabase db;
public:

    ///Costruttore
    explicit KKDataBase();
    ~KKDataBase();

    ///Generic query exec
    bool db_query(QString query);


    ///inserimento utente. Ritorna 0 successo, -1 username fault, -2 email fault.
    int insertUserInfo(QString username, QString password, QString email, QString name,
                       QString surname);

    ///inserimento utente. Ritorna 0 successo,
    int insertUserFile(QString username, QString filename, QString path);

    ///inserimento permessi (share file). Ritorna 0 successo,
    int shareUserFile(QString username_from, QString username_to, QString filename);

    UserInfo *getUserInfo(QString username);

    bool login(QString username, QString password);

    ///reset password ask. invia una mail con password temporanea
    int resetPassword(QString username);

    ///reset password ask. cambia password
    int updatePassword(QString username, QString new_psw);

    ///retorna la lista dei file a cui user ha accesso
    QStringList getUserFile(QString username);

    ///controlla l'esistenza di un utente
    bool checkUserInfo(QString username);

    ///inserisce l'immagine dell'utente
    bool InsertUserImage(QString username, QString image_path);

};

typedef QSharedPointer<KKDataBase> kk_db_ptr;
#endif //SERVER_KK_DB_H
