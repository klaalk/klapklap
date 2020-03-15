//
// Created by jsnow on 11/05/19.
//
//
#include "kk_db.h"
#include <QtSql>
#include <QDebug>

#define  HOST "127.0.0.1"
#define  USR  "server"
#define  DBN  "KLAPKLAP_DB"
#define  PSW  "password"
#define  PORT 8889

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

KKDataBase::KKDataBase(){
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("130.192.163.109");
    db.setPort(3000);
    db.setUserName(USR);
    db.setPassword(PSW);
    db.setDatabaseName(DBN);
}

KKDataBase::~KKDataBase(){
    db.close();
}

UserInfo *KKDataBase::getUserInfo(QString username) {
    if(!db.open()) {
        qDebug("DB not opened");
        return nullptr;
    }
    auto userInfo = new UserInfo;
    try {
        QSqlQuery res = db.exec(
                    "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`REGISTRATION_DATE`,`PASSWORD` FROM `USERS` WHERE `USERNAME`='" +
                    username + "';");
        while(res.next()) {
            userInfo->id = res.value(0).toString();
            userInfo->name = res.value(1).toString();
            userInfo->surname = res.value(2).toString();
            userInfo->email = res.value(3).toString();
//          userInfo->image = res.value(5).toString();
            userInfo->registrationDate = res.value(4).toString();
            userInfo->password = res.value(5).toString();
        }
        db.close();
        return userInfo;
    } catch (QException e) {
        db.close();
        return nullptr;
    }
}

int KKDataBase::insertUserInfo(QString username, QString password, QString email, QString name, QString surname) {
    KKSmtp sender;
    QString destName = name + " " + surname;
    QString queryStr = "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`) VALUES('" +
            username + "','" + password + "','" + email + "','" + name + "','" + surname + "');";
    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }
    try {
        QSqlQuery res = db.exec(queryStr);
        QString mex = sender.messageBuilder("Welcome to KlapKlap Soft :)",
                                    destName,
                                    username + "\nYour registration is now complete!",
                                    "You are signed-up!",
                                    "blank"
                                    );
        sender.sendMessage(mex, destName, email, "KlapKlap Registration");
        db.close();
        return 0;
    } catch (QException &e) {
        QString _str(e.what());
        db.close();
        if (_str.contains("EMAIL"))
            return -1;
        else if (_str.contains("USERNAME"))
            return -2;
    }
    return 0;
}

int KKDataBase::insertUserFile(QString username, QString filename, QString path) {
    KKSmtp sender;
    QString name, surname, email;
    QString _queryStr;

    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }

    QSqlQuery res = db.exec("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL` FROM `USERS` WHERE `USERNAME`='" + username + "';");

    res.next();

    _queryStr = "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) VALUES('" + res.value(0).toString() + "','" + filename + "','" + path + "');";

    name = res.value(1).toString();
    surname = res.value(2).toString();
    email = res.value(3).toString();

    QString mex, dest_name =name + " " + surname;

    mex = sender.messageBuilder("New File added: " + filename, "Owner: " + dest_name, username + "", "Share now!", "http://www.facebook.it");

    try {
        db.exec(_queryStr);
        db.close();
        sender.sendMessage(mex, dest_name, email, "KlapKlap File_Add");
    } catch (QException &e) {
        db.close();
        qDebug() << e.what();
        return -1;
    }
    return 0;
}

int KKDataBase::shareUserFile(QString fromUser, QString toUser, QString filename) {


    KKSmtp sender;
    QString mex;
    QString _queryStr;
    auto user1 = new UserInfo;
    auto user2 = new UserInfo;


    user1 = getUserInfo(fromUser);
    user2 = getUserInfo(toUser);

    //    controllo se non ho gia invitato l'user2
    if(!db.open()) {
        qDebug("DB not opened");
    }
    QSqlQuery res = db.exec("SELECT COUNT(`ID`) FROM `FILES_OWNERS` WHERE `ID`='" + user2->id + "' AND `FILENAME`='" + filename + "';");
    res.next();

    if (res.value(0).toInt() > 0)
        return -1;

    _queryStr = "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) VALUES('" + user2->id + "','" + filename + "','./" + filename + "');";


    mex = sender.messageBuilder("New file shared: " + filename, "",
                                       "Sender: " + user1->name + " " + user1->surname,
                                       "Open now!",
                                       "http://www.facebook.it");

    try {
        db.exec(_queryStr);
        db.close();
        sender.sendMessage(mex, user2->name + " " + user2->surname,
                                  user2->email, "KlapKlap Invite");
    } catch (QException &e) {
        db.close();
        qDebug() << e.what();
        return -1;
    }
    return 0;
}

bool KKDataBase::login(QString username, QString password) {
    UserInfo *user;
    user = getUserInfo(username);
    if(user== nullptr)
        return false;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    return solver.decryptToString(password) == solver.decryptToString(user->password);
}

//TODO: delete user

int  KKDataBase::resetPassword(QString username){
    auto user1 = new UserInfo;
    user1 = getUserInfo(username);
    QString mex;
    KKSmtp sender;
    SimpleCrypt casual_psw(Q_UINT64_C(0x1c3ad5a6acb0f134));
    QString cos;

    mex = sender.messageBuilder("Reset password for user: " +username, "",
                                       "Your temporary password is:",
                                       casual_psw.random_psw(cos),
                                       "null");
    sender.sendMessage(mex, user1->name + " " + user1->surname,
                              user1->email, "KlapKlap Reset Password");

    //update with temporary password

    updatePassword(username,cos);
    return 0;
}

int KKDataBase::updatePassword(QString username, QString password){
    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }
    try {
        QSqlQuery res = db.exec("UPDATE `USERS` SET `PASSWORD`='"+password+"' WHERE `USERNAME`='" + username + "';");
        db.close();
        return 0;
    } catch (QException &e) {
        db.close();
        qDebug() << e.what();
        return -1;
    }
}

QStringList  KKDataBase::getUserFile(QString username){
    auto user1 = new UserInfo;
    user1 = getUserInfo(username);
    QStringList tmp;
    QString _queryStr;

    if(!db.open()) {
        qDebug("DB not opened");
        return tmp;
    }

    try {
        QSqlQuery res = db.exec("SELECT `FILENAME` FROM `FILES_OWNERS` WHERE `ID`='" + user1->id + "';");
        db.close();
        while(res.next()){
            tmp.push_back( res.value(0).toString());
        }
        return tmp;
    } catch (QException &e) {
        QString _str(e.what());
        db.close();
        return tmp;
    }

}

bool KKDataBase::checkUserInfo(QString username) {
    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }
    try {
        QSqlQuery res = db.exec("SELECT COUNT(*) FROM `USERS` WHERE `EMAIL`='" + username + "';");
        db.close();
        res.next();
        if (res.value(0).toInt() > 0)
            return true;
        return false;
    } catch (QException &e) {
        QString _str(e.what());
        db.close();
        return false;
    }
}

bool KKDataBase::insertUserImage(QString username, QString image_path){
     QFile file(image_path);
     if (!file.open(QIODevice::ReadOnly))
         return false;
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

