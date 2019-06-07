//
// Created by jsnow on 11/05/19.
//
//

#include "kk_db.h"
#include <QtSql>
#include <QDebug>
#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  DBN  "KLAPKLAP_DB"
#define  PSW  "passord" //TODO:change

#define STD_Q(x) QString::fromStdString(x)

struct var {
    QString id;
    QString name;
    QString surname;
    QString email;
    QString username;
    QString image; //cosa è questo? quanto fa questo?
    QString reg_date;
    QString password;

};

kk_db::kk_db(){
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("130.192.163.109");
    db.setPort(3000);
    db.setUserName(USR);
    db.setPassword(PSW);
    db.setDatabaseName(DBN);
}



user_info *kk_db::db_getUserInfo(QString username) {
    if(!db.open()) {
        qDebug("DB not opened");
        return nullptr;
    }
    auto userInfo = new user_info;
    try {
        QSqlQuery res = db.exec(
                "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD` FROM `USERS` WHERE `USERNAME`='" +
                username + "';");
        while(res.next()) {
            userInfo->id = res.value(1).toString();
            userInfo->name = res.value(2).toString();
            userInfo->surname = res.value(3).toString();
            userInfo->email = res.value(4).toString();
            userInfo->image = res.value(5).toString();
            userInfo->reg_date = res.value(6).toString();
            userInfo->password = res.value(7).toString();
        }
        db.close();
        return userInfo;
    } catch (QException e) {
        db.close();
        return nullptr;
    }
}

int kk_db::db_insert_user(QString username, QString password ,QString email, QString name,
                          QString surname) {
    int errCode = 0;
    QString _queryStr;
    kk_smtp sender;
    QString mex, dest_name = name + " " + surname;

    _queryStr = "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`) VALUES('" + username + "','" + password + "','" + email + "','" + name + "','" +
               surname +
               "');";

    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }

    try {
        QSqlQuery res = db.exec(_queryStr);
        mex = sender.QSMTP_message_builder("Welcome in KlapKlap Soft :)", dest_name,
                                           username + " complete your registration now!",
                                           "Activate Now",
                                           "http://www.facebook.it");
        sender.QSMTP_send_message(mex, dest_name, email, "KlapKlap Registration");
        db.close();
    } catch (QException &e) {
        QString _str(e.what());
        db.close();
        if (_str.contains("EMAIL"))
            return -1;
        else if (_str.contains("USERNAME"))
            return -2;
    }
    return errCode;
}

int kk_db::db_insert_file(QString username, QString filename, QString path) {
    kk_smtp sender;
    QString name, surname, email;
    QString _queryStr;

    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }

    QSqlQuery res = db.exec("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL` FROM `USERS` WHERE `USERNAME`='" + username + "';");

    res.next();

    _queryStr = "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) VALUES('" + res.value(1).toString() + "','" + filename + "','" + path + "');";

    name = res.value(2).toString();
    surname = res.value(3).toString();
    email = res.value(4).toString();

    QString mex, dest_name =name + " " + surname;

    mex = sender.QSMTP_message_builder("New File added: " + filename, "Owner: " + dest_name,
                                       username + "",
                                       "Share now!", "http://www.facebook.it");

    try {
        db.exec(_queryStr);
        db.close();
        sender.QSMTP_send_message(mex, dest_name, email, "KlapKlap File_Add");
    } catch (QException &e) {
        db.close();
        qDebug() << e.what();
        return -1;
    }
    return 0;
}

int kk_db::db_share_file(QString username_from, QString username_to, QString filename) {


    kk_smtp sender;
    QString mex;
    QString _queryStr;
    auto user1 = new user_info;
    auto user2 = new user_info;


    user1 = db_getUserInfo(username_from);
    user2 = db_getUserInfo(username_to);

//    controllo se non ho gia invitato l'user2
    if(!db.open()) {
        qDebug("DB not opened");
    }
    QSqlQuery res = db.exec("SELECT COUNT(`ID`) FROM `FILES_OWNERS` WHERE `ID`='" + user2->id + "' AND `FILENAME`='" + filename + "';");
    res.next();

    if (res.value(1).toInt() > 0)
        return -1;

    _queryStr = "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) VALUES('" + user2->id + "','" + filename + "','./" + filename + "');";


    mex = sender.QSMTP_message_builder("New file shared: " + filename, "",
                                       "Sender: " + user1->name + " " + user1->surname,
                                       "Open now!",
                                       "http://www.facebook.it");

    try {
        db.exec(_queryStr);
        db.close();
        sender.QSMTP_send_message(mex, user2->name + " " + user2->surname,
                                  user2->email, "KlapKlap Invite");
    } catch (QException &e) {
        db.close();
        qDebug() << e.what();
        return -1;
    }
    return 0;
}

bool kk_db::db_login(QString username, QString password) {
    user_info *user;
    user = db_getUserInfo(username);
    if(user== nullptr)
        return false;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    return password == solver.decryptToString(user->password);
}

//TODO: delete user

int  kk_db::db_reset_psw(QString username){
    auto user1 = new user_info;
    user1 = db_getUserInfo(username);
    QString mex;
    kk_smtp sender;
    SimpleCrypt casual_psw(Q_UINT64_C(0x1c3ad5a6acb0f134));
    QString cos;

    mex = sender.QSMTP_message_builder("Reset password for user: " +username, "",
                                       "Your temporary password is:",
                                       casual_psw.random_psw(cos),
                                       "null");
    sender.QSMTP_send_message(mex, user1->name + " " + user1->surname,
                              user1->email, "KlapKlap Reset Password");

    //update with temporary password

    db_update_psw(username,cos);
    return 0;
}

int kk_db::db_update_psw(QString username, QString new_psw){
    if(!db.open()) {
        qDebug("DB not opened");
        return -1;
    }
    try {
        QSqlQuery res = db.exec("UPDATE `USERS` SET `PASSWORD`='"+new_psw+"' WHERE `USERNAME`='" + username + "';");
        db.close();
        return 0;
    } catch (QException &e) {
        db.close();
        qDebug() << e.what();
        return -1;
    }
}
