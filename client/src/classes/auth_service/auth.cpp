//
// Created by jsnow on 23/05/19.
//

#include "auth.h"


bool auth::auth_register(std::string username, std::string password){
    crypto psw_cr;
    int psw_len = std::strlen(password.c_str());
    std::string cifred = psw_cr._encrypt(password,&psw_len);

//    TODO: Send registration request (psw+len)

    return false;
}

bool auth::auth_login(std::string username, std::string password){
    crypto psw_cr;
    int psw_len = std::strlen(password.c_str());
    std::string cifred = psw_cr._encrypt(password,&psw_len);

//    TODO: Send login request (psw+len)


    return false;
}