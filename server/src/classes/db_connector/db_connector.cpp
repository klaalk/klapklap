//
// Created by jsnow on 11/05/19.
//
//

#include "db_connector.h"
#define  HOST "tcp://130.192.163.109:3000"
#define  USR  "server"
#define  PSW  "password" //TODO:change

db_connector::db_connector(sql::Driver *driver): driver(driver){
}

sql::Statement * db_connector::connect(void) {
    con = driver->connect(HOST, USR, PSW);
    con->setSchema("KLAPKLAP_DB");
    return con->createStatement();
}

void db_connector::close(void) {
    con->close();
}

bool db_connector::db_query(std::string query,int n_col){
    sql::Statement *stmt = connect();
    sql::ResultSet * res = stmt->executeQuery(query);

    while (res->next()) {
        cout << "MySQL says: ";
        for (int i = 1; i <= n_col; ++i)
            cout << res->getString(i) + " ";
        cout<<endl;
    }
    close();
    return true;
}

sql::ResultSet* db_connector::db_query(std::string query){
    sql::Statement *stmt = connect();
    sql::ResultSet * res = stmt->executeQuery(query);
    close();
    return res;
}

int db_connector::db_insert_user(std::string username,std::string password, std::string email, std::string name, std::string surname){
    std::ostringstream _string,_date;
    string id,date;
    int ID;
    sql::ResultSet * res;
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    sql::Statement *stmt = connect();

//    Check univocita username
    _string << "SELECT COUNT(*) FROM USERS WHERE USERNAME='" + username + "';";
    res = stmt->executeQuery(_string.str());
    res->next();
    _string.str("");
    if(res->getInt(1))
        return -1;

//    Check univocita email
    _string << "SELECT COUNT(*) FROM USERS WHERE EMAIL='" + email + "';";
    res = stmt->executeQuery(_string.str());
    res->next();
    _string.str("");
    if(res->getInt(1))
        return -2;

//    Inserimento dati
    _string << "SELECT MAX(ID) FROM USERS;";
    res = stmt->executeQuery(_string.str());
    res->next();
    _string.str("");


//  Sto recuperando ed aggiornando l'ID
    if(res->getString(1) == "")
        ID=0;
    else
        ID=std::stoi(res->getString(1));
    ID++;

    id=std::to_string(ID);

//    Sto recuperando la data odierna
    _date << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' <<  now->tm_mday;


//  Sono pronto per eseguire la insert
    _string << "INSERT INTO `USERS` (`ID`,`USERNAME`,`PSW`,`EMAIL`,`NAME`,`SURNAME`,`IMAGE`,`REGISTRATION_DATE`) "
            << "VALUES('"+id+"','"+username+"','"+password+"','"+email+"','"+name+"','"+surname+"',"+"NULL,'"+_date.str()+"');";

    stmt->execute(_string.str());
    _string.str("");
    _date.str("");
    return 0;
    close();
}