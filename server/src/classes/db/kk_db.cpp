#include "kk_db.h"
#include <QtSql>
#include <QDebug>

#define  HOST "127.0.0.1"
#define  PORT 3306
#define  USR  "root"
#define  DBN  "klapklap"
#define  PSW  ""

#define INSERT_USER "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`ALIAS`,`NAME`,`SURNAME`, `IMAGE`, `REGISTRATION_DATE`) VALUES (?, ?, ?, ?, ?, ?, ?, CURRENT_TIME())"
#define UPDATE_USER_QRY "UPDATE `USERS` SET `ALIAS`=?,`NAME`=?,`SURNAME`=?,`IMAGE`=? WHERE `USERNAME` = ?"
#define GET_USER_BY_USERNAME "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`USERNAME`,`ALIAS`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD` FROM `USERS` WHERE `USERNAME`= ?"
#define CHECK_USER_BY_EMAIL "SELECT COUNT(*) FROM `USERS` WHERE `EMAIL`= ?"
#define CHECK_USER_BY_USERNAME "SELECT COUNT(*) FROM `USERS` WHERE `USERNAME`= ?"

#define INSERT_FILE "INSERT INTO `FILES` (`FILENAME`, `HASHNAME`, `USERNAME`, `CREATION_DATE`) VALUES (?, ?, ?, CURRENT_TIME())"
#define INSERT_FILE_OWNER "INSERT INTO `FILES_OWNERS` (`USERNAME`, `HASHNAME`, `JOIN_DATE`) VALUES (?, ?, CURRENT_TIME())"
#define GET_USER_FILE_BY_EMAIL "SELECT `HASHNAME`, `JOIN_DATE` FROM `FILES_OWNERS` WHERE `USERNAME`= ?"
#define SELECT_FILE_EMAILS "SELECT `USERNAME` FROM `FILES_OWNERS` WHERE `HASHNAME`= ?"
#define COUNT_FILE_PER_EMAIL "SELECT COUNT(*) FROM `FILES_OWNERS` WHERE `HASHNAME`= ? AND `USERNAME` = ?"
#define COUNT_FILE_PER_NAME "SELECT COUNT(*) FROM `FILES` WHERE `FILENAME`= ?"


KKDataBase::KKDataBase():
    crypter(KKCryptPtr(new KKCrypt(Q_UINT64_C(0x0c2ad4a4acb9f023))))
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(HOST);
    db.setPort(PORT);
    db.setUserName(USR);
    db.setPassword(PSW);
    db.setDatabaseName(DBN);

}

KKDataBase::~KKDataBase(){
    db.close();
}

int KKDataBase::signupUser(QString username, QString password, QString email, QString name, QString surname, QString image) {
    int resCode = DB_SIGNUP_FAILED;

    if (existUserByUsername(username) == DB_USER_FOUND)
        resCode = DB_ERR_INSERT_USERNAME;
    else if (existUserByEmail(email) == DB_USER_FOUND)
        resCode = DB_ERR_INSERT_EMAIL;
    else {
        if(!db.open()) {
            resCode = DB_ERR_NOT_OPEN_CONNECTION;
        } else  {
            try {
                QSqlQuery query;
                query.prepare(INSERT_USER);
                query.addBindValue(username);
                query.addBindValue(password);
                query.addBindValue(email);
                query.addBindValue(username);
                query.addBindValue(name);
                query.addBindValue(surname);
                query.addBindValue(image);
                query.exec();
                db.close();
                resCode = DB_SIGNUP_SUCCESS;
            } catch (QException &e) {
                QString _str(e.what());
                qDebug() << "Errore inserimento user: " << _str;
                db.close();
                if (_str.contains("EMAIL"))
                    resCode = DB_ERR_INSERT_EMAIL;
                else if (_str.contains("USERNAME"))
                    resCode = DB_ERR_INSERT_USERNAME;
            }
        }
    }
    return resCode;
}

int KKDataBase::loginUser(QString username, QString password, KKUserPtr user) {
    int resCode = getUser(username, user);
    if(resCode == DB_USER_FOUND) {
        QString userPsw = crypter->decryptToString(user->getPassword());
        QString inputPsw = crypter->decryptToString(password);
        if  (userPsw == inputPsw)
            resCode = DB_LOGIN_SUCCESS;
        else
            resCode = DB_LOGIN_FAILED;
    }
    return resCode;
}

int KKDataBase::getUser(QString username, KKUserPtr userInfo) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(GET_USER_BY_USERNAME);
            query.addBindValue(username);
            query.exec();
            query.next();

            userInfo->setId(query.value(0).toString());
            userInfo->setName(query.value(1).toString());
            userInfo->setSurname(query.value(2).toString());
            userInfo->setEmail(query.value(3).toString());
            userInfo->setUsername(query.value(4).toString());
            userInfo->setAlias(query.value(5).toString());
            userInfo->setImage(query.value(6).toString());
            userInfo->setRegistrationDate(query.value(7).toString());
            userInfo->setPassword(query.value(8).toString());

            db.close();
            resCode = DB_USER_FOUND;
        } catch (QException e) {
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::updateUser(QString username, QString name, QString surname, QString alias, QString avatar)
{
    int resCode = DB_UPDATE_USER_FAILED;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else  {
        try {
            QSqlQuery query;
            query.prepare(UPDATE_USER_QRY);
            query.addBindValue(alias);
            query.addBindValue(name);
            query.addBindValue(surname);
            query.addBindValue(avatar);
            query.addBindValue(username);
            query.exec();
            db.close();
            resCode = DB_UPDATE_USER_SUCCESS;
        } catch (QException &e) {
            QString _str(e.what());
            qDebug() << "Errore aggiornamento user: " << _str;
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::addUserFile(QString filename, QString username) {
    int resCode = DB_INSERT_FILE_FAILED;

    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(INSERT_FILE_OWNER);
            query.addBindValue(username);
            query.addBindValue(filename);
            query.exec();
            db.close();
            resCode = DB_INSERT_FILE_SUCCESS;
        } catch (QException &e) {
            db.close();
            qDebug() << e.what();
        }
    }

    return resCode;
}

int  KKDataBase::getUserFile(KKUserPtr user, QStringList* files){
    int resCode = DB_ERR_USER_FILES;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(GET_USER_FILE_BY_EMAIL);
            query.addBindValue(user->getUsername());
            query.exec();

            if (files == nullptr) {
                files = new QStringList();
            }

            while(query.next()){
                (*files).push_back(query.value(0).toString() + FILENAME_SEPARATOR + query.value(1).toString());
            }

            db.close();
            resCode = DB_USER_FILES_FOUND;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::addFile(QString filename, QString hashname, QString username)
{
    int resCode = DB_INSERT_FILE_FAILED;

    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(INSERT_FILE);
            query.addBindValue(filename);
            query.addBindValue(hashname);
            query.addBindValue(username);
            query.exec();
            db.close();
            resCode = DB_INSERT_FILE_SUCCESS;
        } catch (QException &e) {
            db.close();
            qDebug() << e.what();
        }
    }

    return resCode;
}

int KKDataBase::existUserByEmail(QString email) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(CHECK_USER_BY_EMAIL);
            query.addBindValue(email);
            query.exec();
            db.close();
            query.next();
            if (query.value(0).toInt() > 0)
                resCode = DB_USER_FOUND;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::existUserByUsername(QString username) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(CHECK_USER_BY_USERNAME);
            query.addBindValue(username);
            query.exec();
            db.close();
            query.next();
            if (query.value(0).toInt() > 0)
                resCode = DB_USER_FOUND;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}


int KKDataBase::existFileByName(QString filename)
{
    int resCode = DB_FILE_NOT_EXIST;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(COUNT_FILE_PER_NAME);
            query.addBindValue(filename);
            query.exec();
            db.close();
            query.next();
            if (query.value(0).toInt() > 0)
                resCode = DB_FILE_EXIST;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::existFileByHash(QString hash, QStringList* users)
{
    int resCode = DB_FILE_NOT_EXIST;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        users = (users == nullptr) ? new QStringList() : users;
        try {
            QSqlQuery query(db);
            query.prepare(SELECT_FILE_EMAILS);
            query.addBindValue(hash);
            query.exec();
            db.close();
            while (query.next()) {
                *users << query.value(0).toString();
            }
            if (users->length() > 0)
                resCode = DB_FILE_EXIST;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }

    }
    return resCode;
}

int KKDataBase::existFileByUsername(QString filename, QString username)
{
    int resCode = DB_FILE_NOT_EXIST;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(COUNT_FILE_PER_EMAIL);
            query.addBindValue(filename);
            query.addBindValue(username);
            query.exec();
            db.close();
            query.next();
            if (query.value(0).toInt() > 0)
                resCode = DB_FILE_EXIST;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

/// FIXME: Metodi implementati parzialmente
//TODO: delete user
int  KKDataBase::resetPassword(QString username){
    KKUserPtr user = KKUserPtr(new KKUser);
    int resCode = getUser(username, user);
    if (resCode == DB_USER_FOUND) {
        QString tempPsw;
        KKCrypt casual_psw(Q_UINT64_C(0x1c3ad5a6acb0f134));
        casual_psw.random_psw(tempPsw);
        //update with temporary password
        resCode = updatePassword(username, tempPsw);
        if(resCode == DB_PASSWORD_UPDATED) {
//            KKSmtp sender;
//            QString mex = sender.messageBuilder("Reset password for user: " +username, "",
//                                                "Your temporary password is:",
//                                                tempPsw,
//                                                "null");
//            sender.sendMessage(mex, user->name + " " + user->surname,
//                               user->email, "KlapKlap Reset Password");
        }
    }
    return resCode;
}

int KKDataBase::updatePassword(QString username, QString password) {
    int resCode = DB_PASSWORD_NOT_UPDATED;
    if(!db.open()) {
        qDebug("Errore in fase di apertura della connessione con il Database");
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery res = db.exec("UPDATE `USERS` SET `PASSWORD`='"+password+"' WHERE `USERNAME`='" + username + "';");
            db.close();
            resCode = DB_PASSWORD_UPDATED;
        } catch (QException &e) {
            db.close();
            qDebug() << e.what();
        }
    }
    return resCode;
}


int KKDataBase::insertUserImage(QString username, QString image_path){
    QFile file(image_path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QByteArray inByteArray = file.readAll();
    db.open();
    QSqlQuery query = QSqlQuery( db );
    try {
        query.prepare( "UPDATE `USERS` SET `IMAGE` = :imageData WHERE `USERNAME`='" + username + "';" );
        query.bindValue( ":imageData", inByteArray );
        query.exec();
        return true;
    } catch (QException &e) {
        QString _str(e.what());
        qDebug() << "Error inserting image into table:\n" << query.lastError();
        db.close();
        return false;
    }
}
