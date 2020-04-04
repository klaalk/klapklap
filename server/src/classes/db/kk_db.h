//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_KK_DB_H
#define SERVER_KK_DB_H

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../smtp/kk_smtp.h"
#include "../../../../libs/src/constants/kk_constants.h"
#include <QtSql>
#include <QSqlDatabase>



typedef struct record UserInfo;

struct record {
    QString id;
    QString name;
    QString surname;
    QString email;
    QString username;
    QString image;
    QString registrationDate;
    QString password;
};

class KKDataBase {
private:
    QSqlDatabase db;
public:

    ///Costruttore
    explicit KKDataBase();
    ~KKDataBase();

    ///Generic query exec
    bool db_query(QString query);


    ///inserimento utente.
    int insertUserInfo(QString username, QString password, QString email, QString name,
                       QString surname);
    int sendInsertUserInfoEmail(QString username, QString email, QString name, QString surname);

    ///inserimento utente.
    int insertUserFile(QString filename, QString path, UserInfo* user);
    int sendInsertUserFileEmail(QString username, QString email, QString name, QString surname, QString filename);

    ///inserimento permessi (share file). Ritorna 0 successo,
    int shareUserFile(QString fromUsername, QString toUsername, QString filename, UserInfo* fromUser, UserInfo* toUser);
    int sendShareUserFileEmail(QString filename, UserInfo* fromUser, UserInfo* toUser);

    int getUserInfo(QString username, UserInfo* userInfo);

    int login(QString username, QString password, UserInfo *user);

    ///reset password ask. invia una mail con password temporanea
    int resetPassword(QString username);

    ///reset password ask. cambia password
    int updatePassword(QString username, QString new_psw);

    ///retorna la lista dei file a cui user ha accesso
    int getUserFile(UserInfo *user, QStringList* files);

    ///controlla l'esistenza di un utente tramite email
    int checkUserInfoByEmail(QString email);

    ///controlla l'esistenza di un utente
    int checkUserInfoByUsername(QString username);

    ///inserisce l'immagine dell'utente
    int insertUserImage(QString username, QString image_path);

    int existFilename(QString filename);

    int existFilenameById(QString filename, QString userId);


};

typedef QSharedPointer<KKDataBase> KKDataBasePtr;
#endif //SERVER_KK_DB_H
