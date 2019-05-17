//
// Created by jsnow on 11/05/19.
//
//

#include "db_connector.h"

#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change

db_connector::db_connector(sql::Driver *driver) : driver(driver) {
}

sql::Statement *db_connector::connect(void) {
    con = driver->connect(HOST, USR, PSW);
    con->setSchema("KLAPKLAP_DB");
    return con->createStatement();
}

void db_connector::close(void) {
    con->close();
}

///DEPRECATED
bool db_connector::db_query(std::string query, int n_col) {
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
sql::ResultSet *db_connector::db_query(std::string query) {
    sql::Statement *stmt = connect();
    sql::ResultSet *res = stmt->executeQuery(query);
    close();
    return res;
}

////
//// -----> NOTA: QUANDO NON ESEGUI QUERY DEVI FARE EXECUTE E BASTAAAAAAA!
////

int db_connector::db_insert_user(std::string username, std::string password, std::string email, std::string name,
                                 std::string surname) {
    int errCode = 0;
    std::ostringstream _string;
    sql::ResultSet *res;
    sql::Statement *stmt = connect();
    SMTP_client sender;
    std::string mex;

    _string << "INSERT INTO `USERS` (`USERNAME`,`PASSWORD`,`EMAIL`,`NAME`,`SURNAME`) "
            << "VALUES('" + username + "','" + password + "','" + email + "','" + name + "','" + surname +
               "');";

    try {
        stmt->execute(_string.str());
        _string.str("");
        mex=sender.SMTP_message_builder("Welcome in KlapKlap Soft :)",name+" "+surname,username+" complete your registration now!","Activate Now","http://www.facebook.it");
        sender.SMPT_sendmail(mex,email,"KlapKlap Registration");
        close();
    } catch (sql::SQLException &e) {
        std::string str(e.what());
        close();
        if(str.find("EMAIL")!=string::npos)
            return -1;
        else if(str.find("USERNAME")!=string::npos)
            return -2;
    }
    return errCode;
}

int db_connector::db_insert_file(std::string username, std::string filename,std::string path){
    SMTP_client sender;
    std::string mex,name,surname,email;
    std::ostringstream _string;

    sql::Statement *stmt = connect();
    sql::ResultSet *res=stmt->executeQuery("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL` FROM `USERS` WHERE `USERNAME`='"+username+"';");

    res->next();
    _string << "INSERT INTO `FILES_OWNERS` (`ID`,`FILENAME`,`PATH`) "
            << "VALUES('" + res->getString(1) + "','" + filename + "','" + path + "');";

    name=res->getString(2);
    surname=res->getString(3);
    email=res->getString(4);
    mex=sender.SMTP_message_builder("New File added: "+filename,"Owner: "+name+" "+surname,username+"","Share now!","http://www.facebook.it");

    try {
        stmt->execute(_string.str());
        close();
        sender.SMPT_sendmail(mex,email,"KlapKlap File_Add");
    } catch (sql::SQLException &e) {
        close();
        cout << e.what() << endl ;
        return -1;
    }
    return 0;
}

int db_share_file(std::string username, std::string filename){}

int db_reset_psw_ask(std::string username){}

int db_reset_psw_do(std::string username, std::string hash_key, std::string new_psw){}