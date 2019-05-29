//
// Created by jsnow on 11/05/19.
//
//

#include "kk_db.h"

#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change



struct var {
    std::string id;
    std::string name;
    std::string surname;
    std::string email;
    std::string username;
    std::string image; //cosa è questo? quanto fa questo?
    std::string reg_date;
    std::string password;
    int psw_len;

};

kk_db::kk_db(sql::Driver *driver) : driver(driver) {
}

sql::Statement *kk_db::connect(void) {
    con = driver->connect(HOST, USR, PSW);
    con->setSchema("KLAPKLAP_DB");
    return con->createStatement();
}

void kk_db::close(void) {
    con->close();
}

///DEPRECATED
bool kk_db::db_query(std::string query, int n_col) {
    sql::Statement *stmt = connect();
    sql::ResultSet *res = stmt->executeQuery(query);

    while (res->next()) {
        cout << "MySQL says: ";
        for (int i = 1; i <= n_col; ++i)
            cout << res->getString(i) + " ";
        cout << endl;
    }
    close();
    return true;
}

///DEPRECATED
sql::ResultSet *kk_db::db_query(std::string query) {
    sql::Statement *stmt = connect();
    sql::ResultSet *res = stmt->executeQuery(query);
    close();
    return res;
}

////
//// -----> NOTA: QUANDO NON ESEGUI QUERY DEVI FARE EXECUTE E BASTAAAAAAA!
////


user_info *kk_db::db_getUserInfo(std::string username) {

    sql::Statement *stmt = connect();
    auto userInfo = new user_info;
    sql::ResultSet *res = stmt->executeQuery(
            "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD`,`PSWLEN` FROM `USERS` WHERE `USERNAME`='" +
            username + "';");
    res->next();
    userInfo->id = res->getString(1);
    userInfo->name = res->getString(2);
    userInfo->surname = res->getString(3);
    userInfo->email = res->getString(4);
    userInfo->image = res->getString(5);
    userInfo->reg_date = res->getString(6);
    userInfo->password = res->getString(7);
    userInfo->psw_len = res->getInt(8);
    close();
    return userInfo;
}

int kk_db::db_insert_user(std::string username, std::string password,int pass_len, std::string email, std::string name,
                                 std::string surname) {
    int errCode = 0;
    std::ostringstream _string;
    sql::ResultSet *res;
    sql::Statement *stmt = connect();
    kk_smtp sender;
    QString mex,dest_name=QString::fromStdString(name) + " " + QString::fromStdString(surname);
    std::string _psw_len=std::to_string(pass_len);

    _string << "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`PSWLEN`,`EMAIL`,`NAME`,`SURNAME`) "
            << "VALUES('" + username + "','" + password + "','" + _psw_len + "','" + email + "','" + name + "','" + surname +
               "');";
    try {
        stmt->execute(_string.str());
        _string.str("");
        mex = sender.QSMTP_message_builder("Welcome in KlapKlap Soft :)", dest_name,
                                           QString::fromStdString(username) + " complete your registration now!", "Activate Now",
                                          "http://www.facebook.it");
        sender.QSMTP_send_message(mex,dest_name ,QString::fromStdString(email), "KlapKlap Registration");
        close();
    } catch (sql::SQLException &e) {
        std::string str(e.what());
        close();
        if (str.find("EMAIL") != string::npos)
            return -1;
        else if (str.find("USERNAME") != string::npos)
            return -2;
    }
    return errCode;
}

int kk_db::db_insert_file(std::string username, std::string filename, std::string path) {
    kk_smtp sender;
    std::string name, surname, email;
    std::ostringstream _string;

    sql::Statement *stmt = connect();
    sql::ResultSet *res = stmt->executeQuery(
            "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL` FROM `USERS` WHERE `USERNAME`='" + username + "';");

    res->next();
    _string << "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) "
            << "VALUES('" + res->getString(1) + "','" + filename + "','" + path + "');";

    name = res->getString(2);
    surname = res->getString(3);
    email = res->getString(4);

    QString mex,dest_name=QString::fromStdString(name) + " " + QString::fromStdString(surname);

    mex = sender.QSMTP_message_builder("New File added: " + QString::fromStdString(filename), "Owner: " + dest_name, QString::fromStdString(username) + "",
                                      "Share now!", "http://www.facebook.it");

    try {
        stmt->execute(_string.str());
        close();
        sender.QSMTP_send_message(mex,dest_name, QString::fromStdString(email), "KlapKlap File_Add");
    } catch (sql::SQLException &e) {
        close();
        cout << e.what() << endl;
        return -1;
    }
    return 0;
}

int kk_db::db_share_file(std::string username_from, std::string username_to, std::string filename) {


    kk_smtp sender;
    QString mex;
    sql::ResultSet *res;
    std::ostringstream _string;
    auto user1 = new user_info;
    auto user2 = new user_info;


    user1 = db_getUserInfo(username_from);
    user2 = db_getUserInfo(username_to);

//    controllo se non ho gia invitato l'user2
    sql::Statement *stmt = connect();
    res = stmt->executeQuery(
            "SELECT COUNT(`ID`) FROM `FILES_OWNERS` WHERE `ID`='" + user2->id + "' AND `FILENAME`='" + filename + "';");
    res->next();
    if (res->getInt(1) > 0)
        return -1;

    _string << "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) "
            << "VALUES('" + user2->id + "','" + filename + "','./" + filename + "');";


    mex = sender.QSMTP_message_builder("New file shared: " + QString::fromStdString(filename), "",
                                      "Sender: " + QString::fromStdString(user1->name + " " + user1->surname), "Open now!",
                                      "http://www.facebook.it");

    try {
        stmt->execute(_string.str());
        close();
        sender.QSMTP_send_message(mex,QString::fromStdString(user2->name + " " + user2->surname) ,QString::fromStdString(user2->email), "KlapKlap Invite");
    } catch (sql::SQLException &e) {
        close();
        cout << e.what() << endl;
        return -1;
    }
    return 0;
}

//int db_reset_psw_ask(std::string username){}



bool kk_db::db_login(std::string username, std::string password, int psw_len) {
    user_info *user;
    user = db_getUserInfo(username);
    crypto solver;

    std::string key1=password;
    std::string key2=user->password;
    int lKey1=psw_len;
    int lkey2=user->psw_len;

    if(lKey1!=lkey2)
        return false;

    return solver._isEqual(key1,key2,lKey1,lkey2);
}

//TODO: delete user

