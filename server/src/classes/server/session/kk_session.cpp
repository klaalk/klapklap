//
// Created by Klaus on 06/05/2019.
//

#include "kk_session.h"

#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>


#define DEBUG

KKSession::KKSession(KKDataBasePtr db, KKFileSystemPtr fileSystem, KKMapFilePtr files, QString sessionId, QObject*  parent)
    : QObject(parent), db(db), files(files), fileSystem(fileSystem), user(KKUserPtr(new KKUser())) {

    QThreadPool::globalInstance()->setMaxThreadCount(5);
    this->sessionId = sessionId;
}

KKSession::~KKSession() {
    if (!socket.isNull())
        socket->deleteLater();

    if (!user.isNull())
        user->deleteLater();

    KKLogger::log("Session deconstructed", sessionId);
}

void KKSession::deliver(KKPayloadPtr msg) {
    socket->sendTextMessage(msg->encode());
}

QString KKSession::getSessionId() {
    return sessionId;
}

void KKSession::setSocket(QSharedPointer<QWebSocket> descriptor) {
    // make a new socket
    socket = descriptor;
    connect(socket.get(), &QWebSocket::textMessageReceived, this, &KKSession::handleRequest);
    connect(socket.get(), &QWebSocket::binaryMessageReceived, this, &KKSession::handleBinaryRequests);
    connect(socket.get(), &QWebSocket::disconnected, this, &KKSession::handleDisconnection);
    logger("Client info: " + descriptor->peerAddress().toString() + ", "
                           + QString::number(descriptor->peerPort()) + " connected at "
                           + descriptor->localAddress().toString() + ", "
                           + QString::number(descriptor->localPort()));
}

void KKSession::sendResponse(QString type, QString result, QStringList values) {
    KKPayload res(type, result, values);
    logger("Server send: " + res.encode());
    socket->sendTextMessage(res.encode());
}

void KKSession::handleRequest(QString message) {
    logger("Client send: " + message);
    if (socket){
        KKPayload req(message);
        req.decode();
        if(req.getRequestType() == LOGIN) {
            handleLoginRequest(req);
        }
        else if(req.getRequestType() == SIGNUP) {
            handleSignupRequest(req);
        }
        else if(req.getRequestType() == LOGOUT){
            handleLogoutRequest(req);
        }
        else if(req.getRequestType() == UPDATE_USER) {
            handleUpdateUserRequest(req);
        }
        else if(req.getRequestType() == GET_FILES) {
            handleGetFilesRequest();
        }
        else if(req.getRequestType() == OPEN_FILE) {
            handleOpenFileRequest(req);
        }
        else if(req.getRequestType() == SAVE_FILE) {
            handleSaveFileRequest(req);
        }
        else if(req.getRequestType() == LOAD_FILE) {
            handleLoadFileRequest(req);
        }
        else if(req.getRequestType() == CRDT) {
            handleCrdtRequest(req);
        }
        else if(req.getRequestType()== ALIGNMENT_CHANGE){
            handleAlignChangeRequest(req);
        }
        else if(req.getRequestType() == CHARFORMAT_CHANGE){
            handleFormatChangeRequest(req);
        }
        else if(req.getRequestType() == CHAT) {
            handleChatRequest(req);
        }
    }
}

void KKSession::handleBinaryRequests(QByteArray message) {
    if (socket) {
        KKLogger::log("Client send binary: " + message, sessionId);
    }
}

void KKSession::handleDisconnection() {
    logger("Session "
                    + id + ", "
                    + socket->peerAddress().toString() + ", "
                    + QString::number(socket->peerPort()) + " closing...");

    disconnectFromFile();
    emit disconnected(sessionId);
}

void KKSession::handleLoginRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    logger("Client username: " + _body[0]);

    int result = DB_LOGIN_SUCCESS;
    result = db->loginUser(_body[0], _body[1], user);

    if(result == DB_LOGIN_SUCCESS) {
        QStringList* output = new QStringList();
        id = user->getUsername();
        output->append(user->getName());
        output->append(user->getSurname());
        output->append(user->getEmail());
        output->append(user->getPassword());
        output->append(user->getUsername());
        output->append(user->getAlias());
        output->append(user->getRegistrationDate());
        output->append(user->getImage());

        result = db->getUserFiles(user, output);

        if (result != DB_USER_FILES_FOUND) {
            logger("Non è stato possibile recuperare i file associati a " + _body[0]);
        }

        this->sendResponse(LOGIN, SUCCESS, *output);

    } else if (result == DB_LOGIN_FAILED) {
        this->sendResponse(LOGIN, BAD_REQUEST, {"Credenziali non valide"});

    } else if (result == DB_ERR_USER_NOT_FOUND) {
        this->sendResponse(LOGIN, BAD_REQUEST, {"Account non esistente"});

    } else {
        this->sendResponse(LOGIN, INTERNAL_SERVER_ERROR, {"Errore interno. Non è stato possibile effettuare il login!"});
    }
}

void KKSession::handleSignupRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    int result = db->signupUser(_body[4],_body[1],_body[0],_body[2], _body[3], _body[5]);

    if(result == DB_SIGNUP_SUCCESS) {
        this->sendResponse(SIGNUP, SUCCESS, {"Registrazione effettuata con successo"});

    } else if (result == DB_ERR_INSERT_EMAIL || result == DB_ERR_INSERT_USERNAME) {
        this->sendResponse(SIGNUP, BAD_REQUEST, {"Errore nella richiesta, username e/o Email esistenti!"});

    } else {
        this->sendResponse(SIGNUP, INTERNAL_SERVER_ERROR, {"Errore interno. Non e' stato possibile effettuare la registrazione!"});
    }
}

void KKSession::handleLogoutRequest(KKPayload request) {
    Q_UNUSED(request)
    disconnectFromFile();
    this->sendResponse(LOGOUT, SUCCESS, {"Logut eseguito"});
}

void KKSession::handleUpdateUserRequest(KKPayload request)
{
    QStringList bodyReqeust = request.getBodyList();
    QString name = bodyReqeust.value(1);
    QString surname = bodyReqeust.value(2);
    QString alias = bodyReqeust.value(3);
    QString avatar = bodyReqeust.value(4);

    int result = db->updateUser(user->getUsername(), name, surname, alias, avatar);
    if (result == DB_UPDATE_USER_SUCCESS) {
        db->getUser(user->getUsername(), user);
        this->sendResponse(UPDATE_USER, SUCCESS, {"Aggiornamento effettuato con successo"});

    } else {
        logger("Errore durante l'aggiornamento user. Result code: " + QVariant(result).toString());
        this->sendResponse(UPDATE_USER, INTERNAL_SERVER_ERROR, {"Non è stato possibile procedere con l'aggiornamento"});

    }
}

void KKSession::handleGetFilesRequest() {
    QStringList* output = new QStringList();
    db->getUserFiles(user, output);
    this->sendResponse(GET_FILES, SUCCESS, *output);
}

void KKSession::handleOpenFileRequest(KKPayload request) {
    QStringList* response = new QStringList();
    QStringList* ids = new QStringList();

    QString message, result;
    bool isActiveFile = false;

    if (request.getBodyList().size() > 0) {
        QString filename = request.getBodyList()[0];
        auto search = files->find(filename);
        if (search != files->end()) {
            // Controllo se il file risulta tra quelli già aperti.
            file = files->value(filename);
            isActiveFile = true;

        } else {
            // Controllo se il file esiste nel DB e recupero la lista di utenti associati a quel file
            if (db->getFileUsers(filename, ids) == DB_FILE_NOT_EXIST) {

                // File non esistente, controllo se l'utente ha già creato il file con lo stesso nome
                if (db->existFileByUsername(filename, user->getUsername()) == DB_FILE_NOT_EXIST) {

                    file = fileSystem->createFile(filename, user->getUsername());

                    if (file != FILE_SYSTEM_CREATE_ERROR)
                        db->addFile(filename, file->getHash(), user->getUsername());

                } else {
                    message = "Errore in fase di richiesta: nome file già esistente";
                    result = BAD_REQUEST;
                }
            } else {
                // File esistente
                file = fileSystem->openFile(filename);
            }
            isActiveFile = false;
        }

    } else {
        message = "Errore in fase di richiesta: non è stato inserito nessun nome file";
        result = BAD_REQUEST;
    }

    if (file != FILE_SYSTEM_CREATE_ERROR) {

        if(!isActiveFile) {
            // Inserisco il file nella mappa dei file attivi
            files->insert(file->getHash(), file);
            file->setOwners(QSharedPointer<QStringList>(ids));
            file->initCrdtText();
        }

        if( db->existShareFileByUsername(file->getHash(), user->getUsername()) == DB_FILE_NOT_EXIST) {

            if (db->addShareFile(file->getHash(), user->getUsername()) == DB_INSERT_FILE_SUCCESS) {
                result = SUCCESS;
                message = "File aperto con successo, sei stato aggiunto come partecipante";
                file->join(sharedFromThis());
                file->addOwner(user->getUsername());
            } else {
                result = INTERNAL_SERVER_ERROR;
                message = "Errore in fase di inserimento partecipante per il file richiesto";
            }

        } else {
            result = SUCCESS;
            message = "File aperto con successo, partecipazione confermata";
            file->join(sharedFromThis());
        }
        // Rispondo con la list di tutti partecipanti al file (attivi o non attivi)
        KKMapParticipantPtr participants = file->getParticipants();
        for(QString id : *file->getOwners()) {
            auto entry = participants->find(id);
            response->push_back(id + ":" + (entry != participants->end() ? PARTICIPANT_ONLINE : PARTICIPANT_OFFLINE));
        }

    } else {
        if (result.isEmpty())
            result = INTERNAL_SERVER_ERROR;
        if (message.isEmpty())
            message = "Errore nel filesystem durante l'apertura del nuovo file";
    }

    logger(message);
    response->push_front(message);
    sendResponse(OPEN_FILE, result, *response);

    if (result == SUCCESS) {
        sendResponse(LOAD_FILE, SUCCESS, {file->getCrdtText()});
        // Aggiorno con gli ultimi messaggi mandati.
        KKVectorPayloadPtr queue = file->getRecentMessages();
        if(queue->length() > 0) {
            std::for_each(queue->begin(), queue->end(), [&](KKPayloadPtr d){
                socket->sendTextMessage(d->encode());
            });
        }

        // Dico a tutti che c'è un nuovo partecipante.
        file->deliver(ADDED_PARTECIPANT, SUCCESS, {user->getUsername()}, "All");
    }
}


void KKSession::handleSaveFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();

    KKTask *mytask = new KKTask([=]() {
        file->flushCrdtText();
    });

    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleLoadFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();

    KKTask *mytask = new KKTask([=]() {
        this->sendResponse(LOAD_FILE, SUCCESS, file->getCrdtText());
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleCrdtRequest(KKPayload request) {
    file->deliver(CRDT, SUCCESS, request.getBodyList(), user->getUsername());
}

void KKSession::handleAlignChangeRequest(KKPayload request){
    file->deliver(ALIGNMENT_CHANGE, SUCCESS, request.getBodyList(), user->getUsername());
}

void KKSession::handleFormatChangeRequest(KKPayload request){
    file->deliver(CHARFORMAT_CHANGE,SUCCESS,request.getBodyList(), user->getUsername());
}

void KKSession::handleChatRequest(KKPayload request) {
    file->deliver(CHAT, SUCCESS, request.getBodyList(), "All");
}

void KKSession::disconnectFromFile()
{
    if(!file.isNull()) {
        file->deliver(REMOVED_PARTECIPANT, SUCCESS, {user->getUsername()}, "All");
        file->leave(sharedFromThis());

        if (file->getParticipantCounter() < 1) {
            files->remove(file->getHash());
        }
    }
}

void KKSession::logger(QString message) {
     KKLogger::log(message, QString("%1 - %2").arg(sessionId, id));
}

