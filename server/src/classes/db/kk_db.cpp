//
// Created by jsnow on 11/05/19.
//
//

#include "kk_db.h"

#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change
#define STD_Q(x) QString::fromStdString(x)


struct var {
    std::string id;
    std::string name;
    std::string surname;
    std::string email;
    std::string username;
    std::string image; //cosa è questo? quanto fa questo?
    std::string reg_date;
    std::string password;

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

    try {
        sql::ResultSet *res = stmt->executeQuery(
                "SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD`FROM `USERS` WHERE `USERNAME`='" +
                username + "';");
        res->next();
        userInfo->id = res->getString(1);
        userInfo->name = res->getString(2);
        userInfo->surname = res->getString(3);
        userInfo->email = res->getString(4);
        userInfo->image = res->getString(5);
        userInfo->reg_date = res->getString(6);
        userInfo->password = res->getString(7);
        close();
        return userInfo;

    } catch (sql::SQLException &e) {
        std::string str(e.what());
        cout << str << endl;
        close();
        return nullptr;
    }
}

int kk_db::db_insert_user(std::string username, std::string password ,std::string email, std::string name,
                          std::string surname) {
    int errCode = 0;
    std::ostringstream _string;
    sql::ResultSet *res;
    sql::Statement *stmt = connect();
    kk_smtp sender;
    QString mex, dest_name = STD_Q(name) + " " + STD_Q(surname);

    _string << "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`) "
            << "VALUES('" + username + "','" + password + "','" + email + "','" + name + "','" +
               surname +
               "');";
    try {
        stmt->execute(_string.str());
        _string.str("");
        mex = sender.QSMTP_message_builder("Welcome in KlapKlap Soft :)", dest_name,
                                           STD_Q(username) + " complete your registration now!",
                                           "Activate Now",
                                           "http://www.facebook.it");
        sender.QSMTP_send_message(mex, dest_name, STD_Q(email), "KlapKlap Registration");
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

    QString mex, dest_name = STD_Q(name) + " " + STD_Q(surname);

    mex = sender.QSMTP_message_builder("New File added: " + STD_Q(filename), "Owner: " + dest_name,
                                       STD_Q(username) + "",
                                       "Share now!", "http://www.facebook.it");

    try {
        stmt->execute(_string.str());
        close();
        sender.QSMTP_send_message(mex, dest_name, STD_Q(email), "KlapKlap File_Add");
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


    mex = sender.QSMTP_message_builder("New file shared: " + STD_Q(filename), "",
                                       "Sender: " + STD_Q(user1->name + " " + user1->surname),
                                       "Open now!",
                                       "http://www.facebook.it");

    try {
        stmt->execute(_string.str());
        close();
        sender.QSMTP_send_message(mex, STD_Q(user2->name + " " + user2->surname),
                                  STD_Q(user2->email), "KlapKlap Invite");
    } catch (sql::SQLException &e) {
        close();
        cout << e.what() << endl;
        return -1;
    }
    return 0;
}

bool kk_db::db_login(std::string username, QString password) {
    user_info *user;
    user = db_getUserInfo(username);
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));

    return solver.encryptToString(password) == solver.encryptToString(STD_Q(user->password));
}

//TODO: delete user

    int  kk_db::db_reset_psw(std::string username){

        auto user1 = new user_info;
        user1 = db_getUserInfo(username);
        QString mex;
        kk_smtp sender;
        SimpleCrypt casual_psw(Q_UINT64_C(0x1c3ad5a6acb0f134));
        QString cos;

        mex = sender.QSMTP_message_builder("Reset password for user: " + STD_Q(username), "",
                                           "Your temporary password is:",
                                           casual_psw.random_psw(cos),
                                           "null");
        sender.QSMTP_send_message(mex, STD_Q(user1->name + " " + user1->surname),
                                  STD_Q(user1->email), "KlapKlap Reset Password");

        //update with temporary password

    }

//    int db_reset_psw(std::string username, QString old_psw, QString new_psw){}
