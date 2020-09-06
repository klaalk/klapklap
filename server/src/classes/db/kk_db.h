//
// Created by jsnow on 11/05/19.
//

#ifndef SERVER_KK_DB_H
#define SERVER_KK_DB_H

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../../../libs/src/constants/kk_constants.h"

#include <QtSql>
#include <QSqlDatabase>
#include <classes/user/kk_user.h>


class KKDataBase {
private:
    QSqlDatabase db;
    KKCryptPtr crypter;
public:

    ///Costruttore
    explicit KKDataBase();
    ~KKDataBase();

    /// utente.
    int signupUser(QString username, QString password, QString email, QString name, QString surname, QString image);
    int loginUser(QString username, QString password, KKUserPtr user);
    int getUser(QString username, KKUserPtr user);
    int updateUser(QString username, QString name, QString surname, QString alias, QString avatar);
    int existUserByUsername(QString username);
    int existUserByEmail(QString email);

    /// file
    int addUserFile(QString filename, QString username);
    int getUserFile(KKUserPtr user, QStringList* files);
    int existFileByName(QString filename);
    int existFileByHash(QString hash, QStringList* users);
    int existFileByUsername(QString filename, QString username);

    int addFile(QString filename, QString hashname, QString username);
    int insertUserImage(QString username, QString image_path);
    int resetPassword(QString username);
    int updatePassword(QString username, QString new_psw);
};

typedef QSharedPointer<KKDataBase> KKDataBasePtr;
#endif //SERVER_KK_DB_H
