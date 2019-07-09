//
// Created by Klaus on 21/05/2019.
//

#ifndef SERVER_KK_CONSTANTS_H
#define SERVER_KK_CONSTANTS_H

enum class KKClientConnectionState {
    waiting,
    connected,
    not_connected,
    connection_failed,
    authentication_succed,
    authentication_failed,
};

enum class KKClientState {
    file_closed,
    file_opened
};

enum class KKPayloadType {
    error,
    login,
    logout,
    signup,
    openfile,
    crdt,
    chat,
};

enum class KKPayloadResultType {
    OK = 200,
    NOT_FOUND = 404,
    ERR_SIZE = 406,
    KO = 500,
};
#endif //SERVER_KK_CONSTANTS_H
