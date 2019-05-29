//
// Created by jsnow on 23/05/19.
//

#ifndef SERVER_AUTH_H
#define SERVER_AUTH_H
#include "../../../../libs/src/classes_include.h"
#include "../../../../libs/src/classes/crypto/crypto.h"

class auth {
private:
    std::string username;
public:
    bool auth_register(std::string username, std::string password);
    bool auth_login(std::string username, std::string password);

};


#endif //SERVER_AUTH_H
