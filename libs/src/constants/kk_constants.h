//
// Created by Klaus on 21/05/2019.
//

#ifndef SERVER_KK_CONSTANTS_H
#define SERVER_KK_CONSTANTS_H

#define TIMEOUT_VALUE 10000

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
#define DATE_TIME_FORMAT "dd.MM.yyyy HH:mm"

/// Payload requests
#define LOGIN "LOGN"
#define LOGOUT "LOUT"
#define SIGNUP "SGNP"
#define UPDATE_USER "UPDU"
#define GET_FILES "GETF"
#define OPEN_FILE "OPNF"
#define DELETE_FILE "DELF"
#define CLOSE_FILE "CLSF"
#define SHAREFILE "SHRF"
#define SAVE_FILE "SAVF"
#define LOAD_FILE "LODF"
#define QUIT_FILE "QUIF"
#define CRDT "CRDT"
#define CHAT "CHAT"
#define SET_PARTECIPANTS "STPR"
#define ADDED_PARTECIPANT "ADPR"
#define REMOVED_PARTECIPANT "RMPR"

/// Payload CRDT Action
#define CRDT_INSERT "INSERT"
#define CRDT_DELETE "DELETE"
#define CRDT_FORMAT "FORMAT"
#define CRDT_ALIGNM "ALIGNM"
#define CRDT_CLOSE "CLOSE"

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
#define DB_DEACTIVE_FILE_SUCCESS 16
#define DB_DEACTIVE_FILE_FAILED -16
#define DB_INSERT_FILE_SUCCESS 5
#define DB_INSERT_FILE_FAILED -5
#define DB_DELETE_FILE_SUCCESS 15
#define DB_DELETE_FILE_FAILED -15
#define DB_ACCESS_SHAREFILE_SUCCESS 21
#define DB_ACCESS_SHAREFILE_FAILED -21
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
#define FILE_ERROR "FileError"
#define GENERIC_ERROR "GenericError"
#define SERVER_ERROR "ServerError"
#define INPUT_ERROR "InputOnClientError"

/// Modal Buttons Action
#define BUTTON_CLOSE_ACTION "Chiudi"
#define BUTTON_RETRY_ACTION "Riprova"
#define BUTTON_CONTINUE_ACTION "Continua"

/// File system costants
#define LOG_FILE "log"
#define FILE_SYSTEM_USER "root"

/// Application roots
#define SERVER_ROOT QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/klapklap"
#define LOG_ROOT SERVER_ROOT + "/log/"
#define APPLICATION_ROOT SERVER_ROOT + "/application/"

/// File system errors
#define FILE_SYSTEM_CREATE_ERROR nullptr

/// Regular Expressions
#define EMAIL_REGEX QRegularExpression(R"(^\b[A-Za-z0-9._%+-]{1,30}@[A-Za-z0-9.-]{1,20}\.[A-Za-z]{2,4}\b)")
#define PASSWORD_REGEX QRegularExpression("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,20}$")
#define USERNAME_REGEX QRegularExpression("^[A-Za-z0-9]{1,20}(?:[ _-][A-Za-z0-9]+)*$")
#define NAME_REGEX QRegularExpression("^[a-zA-Z]+(([',. -][a-zA-Z ])?[a-zA-Z]*)*$")
#define SURNAME_REGEX QRegularExpression("^[a-zA-Z]+(([',. -][a-zA-Z ])?[a-zA-Z]*)*$")

/// Modal messages
#define MODAL_TIMEOUT "Server non raggiungibile\nSi prega di riprovare pi?? tardi"
#define MODAL_SUCCESS "L'operazione ?? andata a buon fine"
#define MODAL_GENERIC_ERROR "Errore interno al server\nNon ?? possibile procedere con la richiesta"
#define MODAL_NOT_LOGGED "Hai inserito delle credenziali non valide\nControlla che email e/o password siano corrette"
#define MODAL_NOT_SIGNED "La registrazione non ?? andata a buon fine\nUsername e/o Email esistenti"
#define MODAL_NOT_OPENFILE "Non ?? stato possibile scaricare il file dal server\nSi proceder?? con la chiusura del file"
#define MODAL_OPENED_FILE "Non ?? stato possibile aggiornare il file dal server\nSi proceder?? con la chiusura del file"
#define MODAL_LOGIN_ERROR "Errore generico nella risposta del server\nRiprovare dopo il login"
#define MODAL_UPDATE_USER "Non ?? stato possibile procedere con il salvataggio\nSi proceder?? con la chiusura del file"
#define MODAL_QUIT_FILE "Non ?? stato possibile effettuare la modifica\nSi proceder?? con la chiusura del file"
#define MODAL_UPDATE_USER_INFO "Le tue informazioni sono state aggiornate con successo"
#define MODAL_NETWORK_ERROR "Attenzione!\nSembra che tu non sia connesso alla rete"
#define MODAL_UPDATE_USER_FAIL "Non ?? stato possibile aggiornare l'account"
#define MODAL_QUIT_FILE_ERROR "Non ?? stato possibile chiudere il file"
#define MODAL_DELETE_FILE_ERROR "Non ?? stato possibile eliminare il file"
#define MODAL_UPDATE_FILE_ERROR "Non ?? stato possibile aggiornare il file dal server"
#define MODAL_CRDT_ERROR "Non ?? stato possibile effettuare l'operarzione\nIl file verr?? ricaricato con l'ultima versione"

#define DIM_ICN 36
#define DIM_ICN_GREEN 11
#ifdef Q_OS_MACOS
    #define DIM_ICN_TXT 12
#else
    #define DIM_ICN_TXT 10
#endif

#endif //SERVER_KK_CONSTANTS_H
