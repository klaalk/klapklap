//
// Created by Klaus on 21/05/2019.
//

#ifndef SERVER_KK_CONSTANTS_H
#define SERVER_KK_CONSTANTS_H

#define TIMEOUT_VALUE 5000

/// Payload format
#define PAYLOAD_HEAD_LENGTH 8
#define PAYLOAD_REQUEST_LENGTH 4
#define PAYLOAD_RESULT_LENGTH 3
#define PAYLOAD_FORMAT_LENGTH 3
#define PAYLOAD_FORMAT "%1"
#define PAYLOAD_END "000"
#define PAYLOAD_EMPTY_BODY {}
#define FILENAME_SEPARATOR "/"
#define FILENAME_MAX_LENGTH 30
#define DATE_TIME_FORMAT "dd.MM.yyyy hh:mm"

/// Payload requests
#define LOGIN "LOGN"
#define LOGOUT "LOUT"
#define SIGNUP "SGNP"
#define SIGNOUT "SOUT"
#define UPDATE_USER "UPDU"
#define GET_FILES "GETF"
#define OPEN_FILE "OPNF"
#define SHAREFILE "SHRF"
#define SAVE_FILE "SAVF"
#define LOAD_FILE "LODF"
#define QUIT_FILE "QUIF"

#define CRDT "CRDT"
#define CHAT "CHAT"
#define ALIGNMENT_CHANGE "AGNC"
#define SET_PARTECIPANTS "STPR"
#define ADDED_PARTECIPANT "ADPR"
#define REMOVED_PARTECIPANT "RMPR"
#define CHARFORMAT_CHANGE "CHCF"

/// Payload CRDT Action
#define CRDT_INSERT "INSERT"
#define CRDT_DELETE "DELETE"

/// Payload results
#define NONE "000"
#define SUCCESS "200"
#define BAD_REQUEST "400"
#define INTERNAL_SERVER_ERROR "500"
#define INVALID_CREDENTIALS "401"

/// DB errors
#define EMPTY_USER_INFO nullptr

#define DB_USER_FOUND 1
#define DB_USER_FILES_FOUND 2

#define DB_LOGIN_SUCCESS 20
#define DB_LOGIN_FAILED -20

#define DB_SIGNUP_SUCCESS 40
#define DB_SIGNUP_FAILED -40

#define DB_UPDATE_USER_SUCCESS 50
#define DB_UPDATE_USER_FAILED -50

#define DB_PASSWORD_UPDATED 30
#define DB_PASSWORD_NOT_UPDATED -30

#define DB_SHARE_FILE_SUCCESS 6
#define DB_SHARE_FILE_FAILED -6

#define DB_INSERT_FILE_SUCCESS 5
#define DB_INSERT_FILE_FAILED -5

#define DB_ERR_NOT_OPEN_CONNECTION -1
#define DB_ERR_USER_NOT_FOUND -2
#define DB_ERR_INSERT_EMAIL -3
#define DB_ERR_INSERT_USERNAME -4
#define DB_ERR_MULTIPLE_SHARE_FILE -7
#define DB_ERR_SHARE_FILE_USERS -8
#define DB_ERR_USER_FILES -9

#define DB_FILE_EXIST -10
#define DB_FILE_NOT_EXIST -11


/// Sender email errors
#define SEND_EMAIL_SUCCESS 50
#define SEND_EMAIL_NOT_SUCCESS -50

/// Client States
#define NOT_CONNECTED "SocketNotConnected"
#define CONNECTED "SocketConnected"
#define CONNECTED_NOT_LOGGED "ConnectedNotLogged"
#define CONNECTED_NOT_SIGNED "ConnectedNotSigned"
#define CONNECTED_AND_LOGGED "ConnectedAndLogged"
#define CONNECTED_NOT_OPENFILE "ConnectedNotOpened"
#define CONNECTED_AND_OPENED "ConnectedAndOpened"

// Participant States
#define PARTICIPANT_ONLINE "Online"
#define PARTICIPANT_OFFLINE "Offline"

/// Timeout Modal Types
#define CONNECTION_TIMEOUT "ConnectionTimeout"
#define LOGIN_TIMEOUT "LoginTimeout"
#define SIGNUP_TIMEOUT "SignupTimeout"
#define OPENFILE_TIMEOUT "OpenfileTimeout"

/// Success Modal Types
#define GENERIC_SUCCESS "GenericSuccess"

/// Error Modal Types
#define LOGIN_ERROR "LogginError"
#define SIGNUP_ERROR "LogginError"
#define UPDATE_USER_ERROR "UpdateUserError"
#define OPENFILE_ERROR "OpenfileError"
#define CRDT_ERROR "CrdtError"
#define CRDT_ILLEGAL "CrdtIllegal"
#define CHAT_ERROR "ChatError"
#define GENERIC_ERROR "GenericError"
#define SERVER_ERROR "ServerError"
#define INPUT_ERROR "InputOnClientError"

/// File system costants
#define LOG_FILE "log"
#define FILE_SYSTEM_USER "root"

#define SERVER_ROOT QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/KKServer"
#define LOG_ROOT SERVER_ROOT + "/log/"
#define APPLICATION_ROOT SERVER_ROOT + "/application/"

/// File system errors
#define FILE_SYSTEM_CREATE_ERROR nullptr

/// Regular Expressions
#define EMAIL_REGEX QRegularExpression(R"(^\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,4}\b)")
#define PASSWORD_REGEX QRegularExpression("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$")
#define USERNAME_REGEX QRegularExpression("^[A-Za-z0-9]+(?:[ _-][A-Za-z0-9]+)*$")
#define NAME_REGEX QRegularExpression("^[a-zA-Z]+(([',. -][a-zA-Z ])?[a-zA-Z]*)*$")
#define SURNAME_REGEX QRegularExpression("^[a-zA-Z]+(([',. -][a-zA-Z ])?[a-zA-Z]*)*$")

#endif //SERVER_KK_CONSTANTS_H
