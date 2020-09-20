#include "kk_db.h"
#include <QtSql>
#include <QDebug>

#define  HOST "localhost"
#define  PORT 3306
#define  USR  "root"
#define  DBN  "klapklap"
#define  PSW  ""

#define INSERT_USER_QRY "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`ALIAS`,`NAME`,`SURNAME`, `IMAGE`, `REGISTRATION_DATE`) VALUES (?, ?, ?, ?, ?, ?, ?, CURRENT_TIME())"
#define UPDATE_USER_QRY "UPDATE `USERS` SET `ALIAS`=?,`NAME`=?,`SURNAME`=?,`IMAGE`=? WHERE `USERNAME` = ?"
#define GET_USER_BY_USERNAME_QRY "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`USERNAME`,`ALIAS`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD` FROM `USERS` WHERE `USERNAME`= ?"
#define COUNT_USER_PER_EMAIL_QRY "SELECT COUNT(*) FROM `USERS` WHERE `EMAIL`= ?"
#define COUNT_USER_PER_USERNAME_QRY "SELECT COUNT(*) FROM `USERS` WHERE `USERNAME`= ?"

#define INSERT_FILE_QRY "INSERT INTO `FILES` (`FILENAME`, `HASHNAME`, `USERNAME`, `CREATION_DATE`) VALUES (?, ?, ?, CURRENT_TIME())"
#define INSERT_SHAREFILE_QRY "INSERT INTO `FILES_OWNERS` (`USERNAME`, `HASHNAME`, `JOIN_DATE`) VALUES (?, ?, CURRENT_TIME())"
#define GET_USER_FILES_QRY "SELECT `FILES_OWNERS`.`HASHNAME`, `JOIN_DATE` FROM `FILES_OWNERS` JOIN `FILES` ON `FILES_OWNERS`.`HASHNAME` = `FILES`.`HASHNAME` WHERE `FILES_OWNERS`.`USERNAME`= ? ORDER BY `JOIN_DATE` DESC, `FILES`.`FILENAME`"
#define GET_FILE_USERS_QRY "SELECT `USERS`.`USERNAME`, `USERS`.`ALIAS`, `USERS`.`IMAGE` FROM `FILES_OWNERS` JOIN `USERS` ON `USERS`.`USERNAME` = `FILES_OWNERS`.`USERNAME` WHERE `HASHNAME`= ?"
#define COUNT_SHAREFILE_PER_USER_QRY "SELECT COUNT(*) FROM `FILES_OWNERS` WHERE `HASHNAME`= ? AND `USERNAME` = ?"
#define COUNT_FILE_PER_USER_QRY "SELECT COUNT(*) FROM `FILES` WHERE `FILENAME`= ? AND `USERNAME` = ?"


KKDataBase::KKDataBase():
    crypter(KKCryptPtr(new KKCrypt(Q_UINT64_C(0x0c2ad4a4acb9f023))))
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(HOST);
    db.setPort(PORT);
    db.setUserName(USR);
    db.setPassword(PSW);
    db.setDatabaseName(DBN);
#ifdef Q_OS_MACOS
    db.setConnectOptions("UNIX_SOCKET=/Applications/MAMP/tmp/mysql/mysql.sock");
#endif

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
                query.prepare(INSERT_USER_QRY);
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

int KKDataBase::getUser(QString username, KKUserPtr user) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(GET_USER_BY_USERNAME_QRY);
            query.addBindValue(username);
            query.exec();
            query.next();

            user->setId(query.value(0).toString());
            user->setName(query.value(1).toString());
            user->setSurname(query.value(2).toString());
            user->setEmail(query.value(3).toString());
            user->setUsername(query.value(4).toString());
            user->setAlias(query.value(5).toString());
            user->setImage(query.value(6).toString());
            user->setRegistrationDate(query.value(7).toString());
            user->setPassword(query.value(8).toString());

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


int KKDataBase::addFile(QString filename, QString hashname, QString username)
{
    int resCode = DB_INSERT_FILE_FAILED;

    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(INSERT_FILE_QRY);
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

int KKDataBase::addShareFile(QString filename, QString username) {
    int resCode = DB_INSERT_FILE_FAILED;

    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(INSERT_SHAREFILE_QRY);
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

int  KKDataBase::getUserFiles(QString username, QStringList* files){
    int resCode = DB_ERR_USER_FILES;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(GET_USER_FILES_QRY);
            query.addBindValue(username);
            query.exec();

            if (files == nullptr)
                files = new QStringList();

            while(query.next())
                (*files).push_back(query.value(0).toString() + FILENAME_SEPARATOR + query.value(1).toString());

            db.close();
            resCode = DB_USER_FILES_FOUND;
        } catch (QException &e) {
            QString _str(e.what());
            db.close();
        }
    }
    return resCode;
}

int KKDataBase::getFileUsers(QString hash, QStringList* users)
{
    int resCode = DB_FILE_NOT_EXIST;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        users = (users == nullptr) ? new QStringList() : users;
        try {
            QSqlQuery query(db);
            query.prepare(GET_FILE_USERS_QRY);
            query.addBindValue(hash);
            query.exec();
            db.close();
            while (query.next()) {
                QString user = query.value(0).toString()
                        + ":" + query.value(1).toString()
                        + ":" + query.value(2).toString();

                users->push_back(user);
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

int KKDataBase::existUserByEmail(QString email) {
    int resCode = DB_ERR_USER_NOT_FOUND;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(COUNT_USER_PER_EMAIL_QRY);
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
            query.prepare(COUNT_USER_PER_USERNAME_QRY);
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

int KKDataBase::existFileByUsername(QString filename, QString username)
{
    int resCode = DB_FILE_NOT_EXIST;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(COUNT_FILE_PER_USER_QRY);
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


int KKDataBase::existShareFileByUsername(QString filename, QString username)
{
    int resCode = DB_FILE_NOT_EXIST;
    if(!db.open()) {
        resCode = DB_ERR_NOT_OPEN_CONNECTION;
    } else {
        try {
            QSqlQuery query(db);
            query.prepare(COUNT_SHAREFILE_PER_USER_QRY);
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

