//
// Created by Augens on 21/05/2019.
//

#ifndef SERVER_KK_CONSTANTS_H
#define SERVER_KK_CONSTANTS_H
enum kk_payload_type {
    error,
    login,
    logout,
    signup,
    openfile,
    crdt,
    chat,
};

enum kk_payload_result_type {
    OK = 200,
    NOT_FOUND = 404,
    ERR_SIZE = 406,
    KO = 500,
};
#endif //SERVER_KK_CONSTANTS_H
