//
// Created by jsnow on 11/05/19.
//
//
#include "kk_db.h"
#include <QtSql>
#include <QDebug>

#define  HOST "130.192.163.103"
#define  PORT 3000
#define  USR  "server"
#define  DBN  "KLAPKLAP_DB"
#define  PSW  "michele" //TODO:change

#define INSERT_USER "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`) VALUES (?, ?, ?, ?, ?)"


KKDataBase::KKDataBase(){
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

int KKDataBase::getUserInfo(QString username, UserInfo* userInfo) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery res = db.exec("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`USERNAME`,`REGISTRATION_DATE`,`PASSWORD` FROM `USERS` WHERE `USERNAME`='" +username + "';");
            res.next();
            userInfo->id = res.value(0).toString();
            userInfo->name = res.value(1).toString();
            userInfo->surname = res.value(2).toString();
            userInfo->email = res.value(3).toString();
            userInfo->username = res.value(4).toString();
            userInfo->registrationDate = res.value(5).toString();
            userInfo->password = res.value(6).toString();
            // TODO: manca l'immagine
            db.close();
            resCode = DB_USER_FOUND;
        } catch (QException e) {
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::insertUserInfo(QString username, QString password, QString email, QString name, QString surname) {
    int resCode = DB_SIGNUP_FAILED;

    if (checkUserInfoByUsername(username) == DB_USER_FOUND)
        resCode = DB_ERR_INSERT_USERNAME;
    else if (checkUserInfoByEmail(email) == DB_USER_FOUND)
        resCode = DB_ERR_INSERT_EMAIL;
    else {
        if(!db.open()) {
            resCode = DB_ERR_NOT_OPEN_CONNECTION;
        } else  {
            try {
                QString queryStr = "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`)"
                                   " VALUES('" +username + "','" + password + "','" + email + "','" + name + "','" + surname + "');";
                QSqlQuery res = db.exec(queryStr);
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

int KKDataBase::sendInsertUserInfoEmail(QString username, QString email, QString name, QString surname) {
    KKSmtp sender;
    QString destName = name + " " + surname;
    QString mex = sender.messageBuilder("Welcome to KlapKlap Soft :)",
                                destName,
                                username + "\nYour registration is now complete!",
                                "You are signed-up!",
                                "blank"
                                );
    bool success = sender.sendMessage(mex, destName, email, "KlapKlap Registration");
    if (!success) {
        return SEND_EMAIL_NOT_SUCCESS;
    }
    return SEND_EMAIL_SUCCESS;
}

int KKDataBase::insertUserFile(QString username, QString filename, QString path, UserInfo* user) {
    Q_UNUSED(path)
    int resCode = getUserInfo(username, user);

    if (resCode == DB_USER_FOUND) {
        resCode = DB_INSERT_FILE_FAILED;

        if(!db.open()) {
            resCode = DB_ERR_NOT_OPEN_CONNECTION;
        } else {
            QString _queryStr = "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`, `CREATION_DATE`) VALUES ('" + user->id + "','" + filename + "', CURRENT_TIME());";
            try {
                db.exec(_queryStr);
                db.close();
                resCode = DB_INSERT_FILE_SUCCESS;
            } catch (QException &e) {
                db.close();
                qDebug() << e.what();
            }
        }
    }
    return resCode;
}

int KKDataBase::sendInsertUserFileEmail(QString username, QString email, QString name, QString surname, QString filename) {
    QString dest_name = name + " " + surname;
    KKSmtp sender;
    QString mex = sender.messageBuilder("New File added: " + filename, "Owner: " + dest_name, username + "", "Share now!", "http://www.facebook.it");
    bool success = sender.sendMessage(mex, dest_name, email, "KlapKlap File_Add");
    if (!success) {
        return SEND_EMAIL_NOT_SUCCESS;
    }
    return SEND_EMAIL_SUCCESS;
}

int KKDataBase::shareUserFile(QString fromUsername, QString toUsername, QString filename, UserInfo* fromUser, UserInfo* toUser) {
    int resCode = DB_SHARE_FILE_FAILED;
    int resFromUser = getUserInfo(fromUsername, fromUser);
    int resToUser = getUserInfo(toUsername, toUser);

    if (resFromUser == DB_USER_FOUND && resToUser == DB_USER_FOUND) {
        if(!db.open()) {
             resCode = DB_ERR_NOT_OPEN_CONNECTION;
         } else {
            QSqlQuery res = db.exec("SELECT COUNT(`ID`) FROM `FILES_OWNERS` WHERE `ID`='" + toUser->id + "' AND `FILENAME`='" + filename + "';");
            res.next();

            if (res.value(0).toInt() > 0)
                resCode = DB_ERR_MULTIPLE_SHARE_FILE;
            else {
                try {
                    QString _queryStr = "INSERT INTO `FILES_OWNERS` (`ID`, `FILENAME`, `CREATION_DATE`) VALUES ('" + toUser->id + "','" + filename + "', CURRENT_TIME());";
                    db.exec(_queryStr);
                    db.close();
                    resCode = DB_SHARE_FILE_SUCCESS;
                } catch (QException &e) {
                    db.close();
                    qDebug() << e.what();
                }
            }
        }
    } else {
        if (resFromUser < 1) {
            qDebug("Retrieve 'from user' failed");
        }
        if (resToUser < 1) {
            qDebug("Retrieve 'to user' failed");
        }
        resCode = DB_ERR_SHARE_FILE_USERS;
    }
    return resCode;
}

int KKDataBase::sendShareUserFileEmail(QString filename, UserInfo* fromUser, UserInfo* toUser) {
    KKSmtp sender;
    QString mex = sender.messageBuilder("New file shared: " + filename, "",
                                       "Sender: " + fromUser->name + " " + fromUser->surname,
                                       "Open now!",
                                       "http://www.facebook.it");
    bool success = sender.sendMessage(mex, toUser->name + " " + toUser->surname, toUser->email, "KlapKlap Invite");

    if (!success) {
        return SEND_EMAIL_NOT_SUCCESS;
    }
    return SEND_EMAIL_SUCCESS;
}

int KKDataBase::login(QString username, QString password, UserInfo *user) {
    int resCode = getUserInfo(username, user);
    if(resCode == DB_USER_FOUND) {
         SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
         QString userPsw = solver.decryptToString(user->password);
         QString inputPsw = solver.decryptToString(password);
         if  (userPsw == inputPsw)
            resCode = DB_LOGIN_SUCCESS;
         else
            resCode = DB_LOGIN_FAILED;
    }
    return resCode;
}

int  KKDataBase::getUserFile(UserInfo *user, QStringList* files){
    int resCode = DB_ERR_USER_FILES;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery res = db.exec("SELECT `FILENAME`, `CREATION_DATE` FROM `FILES_OWNERS` WHERE `ID`='" + user->id + "';");
            db.close();
            if (files == nullptr) {
                files = new QStringList();
            }
            while(res.next()){
                (*files).push_back(res.value(0).toString() + "#" + res.value(1).toString());
            }
            resCode = DB_USER_FILES_FOUND;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::checkUserInfoByEmail(QString email) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        qDebug("Errore in fase di apertura della connessione con il Database");
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery res = db.exec("SELECT COUNT(*) FROM `USERS` WHERE `EMAIL`='" + email + "';");
            db.close();
            res.next();
            if (res.value(0).toInt() > 0)
                resCode = DB_USER_FOUND;

        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::checkUserInfoByUsername(QString username) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        qDebug("Errore in fase di apertura della connessione con il Database");
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery res = db.exec("SELECT COUNT(*) FROM `USERS` WHERE `USERNAME`='" + username + "';");
            db.close();
            res.next();
            if (res.value(0).toInt() > 0)
                resCode = DB_USER_FOUND;
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
    UserInfo *user = new UserInfo;
    int resCode = getUserInfo(username, user);
    if (resCode == DB_USER_FOUND) {
        QString tempPsw;
        SimpleCrypt casual_psw(Q_UINT64_C(0x1c3ad5a6acb0f134));
        casual_psw.random_psw(tempPsw);
        //update with temporary password
        resCode = updatePassword(username, tempPsw);
        if(resCode == DB_PASSWORD_UPDATED) {
            KKSmtp sender;
            QString mex = sender.messageBuilder("Reset password for user: " +username, "",
                                               "Your temporary password is:",
                                               tempPsw,
                                               "null");
            sender.sendMessage(mex, user->name + " " + user->surname,
                                      user->email, "KlapKlap Reset Password");
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

