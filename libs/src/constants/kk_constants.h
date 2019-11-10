//
// Created by Klaus on 21/05/2019.
//

#ifndef SERVER_KK_CONSTANTS_H
#define SERVER_KK_CONSTANTS_H

#define TIMEOUT_VALUE 5000

#define PAYLOAD_HEAD_LENGTH 8
#define PAYLOAD_REQUEST_LENGTH 4
#define PAYLOAD_RESULT_LENGTH 3
#define PAYLOAD_FORMAT_LENGTH 3
#define PAYLOAD_FORMAT "%1"
#define PAYLOAD_END "000"

///Payload requests
#define LOGIN "LOGN"
#define LOGOUT "LOUT"
#define SIGNUP "SGNP"
#define SIGNOUT "SOUT"
#define OPENFILE "OPNF"
#define CRDT "CRDT"
#define CHAT "CHAT"
#define ADDED_PARTECIPANT "ADPR"
#define REMOVED_PARTECIPANT "RMPR"

/// Payload results
#define NONE "000"
#define SUCCESS "200"
#define FAILED "400"
#define INTERNAL_ERROR "500"
#define INVALID_CREDENTIALS "401"

/// Client States
#define NOT_CONNECTED "NotConnected"
#define CONNECTED "Connected"
#define CONNECTED_NOT_LOGGED "ConnectedNotLogged"
#define CONNECTED_AND_LOGGED "ConnectedAndLogged"
#define CONNECTED_NOT_OPENED "ConnectedNotOpened"
#define CONNECTED_AND_OPENED "ConnectedAndOpened"

/// Timeout Modal Types
#define CONNECTION_TIMEOUT "ConnectionTimeout"
#define LOGIN_TIMEOUT "LoginTimeout"
#define SIGNUP_TIMEOUT "SignupTimeout"
#define OPENFILE_TIMEOUT "OpenfileTimeout"

/// Error Modal Types
#define LOGIN_ERROR "LogginError"
#define OPENFILE_ERROR "OpenfileError"
#define CRDT_ERROR "CrdtError"
#define CHAT_ERROR "ChatError"
#define GENERIC_ERROR "GenericError"

#define CRDT_INSERT "INSERT"
#define CRDT_DELETE "DELETE"

#endif //SERVER_KK_CONSTANTS_H
