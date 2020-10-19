//
// Created by Klaus Cuko on 11/05/19.
//

#ifndef SERVER_KK_DB_H
#define SERVER_KK_DB_H

#include <QtSql>
#include <QSqlDatabase>

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../../../libs/src/classes/user/kk_user.h"
#include "../../../../libs/src/classes/logger/kk_logger.h"
#include "../../../../libs/src/constants/kk_constants.h"



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

    /// file
    int addFile(QString filename, QString hashname, QString username);
    int addShareFile(QString filename, QString username);
    int deleteShareFile(QString filename, QString username);
    int deleteFile(QString hashname);
    int getUserFiles(QString username, QStringList* files);
    int getShareFileUsers(QString hash, QStringList* users);

    // checks
    int existUserByUsername(QString username);
    int existUserByEmail(QString email);
    int existFileByUsername(QString filename, QString username);
    int existShareFileByUsername(QString filename, QString username);
    int existUserByHashName(QString hash);

private:
    void logger(QString message);
};

typedef QSharedPointer<KKDataBase> KKDataBasePtr;
#endif //SERVER_KK_DB_H
