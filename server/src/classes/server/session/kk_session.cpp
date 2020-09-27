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
    res.encode();
    QString body;
    for (QString elem : values) body.append(" ").append(elem);
    logger("Server send: " + res.getRequestType() + " " + res.getResultType() + body);
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
        else if (req.getRequestType() == QUIT_FILE) {
            handleQuitFileRequest();
        }
        else if(req.getRequestType() == CRDT) {
            handleCrdtRequest(req);
        }
        else if(req.getRequestType()== ALIGNMENT_CHANGE){
            handleAlignChangeRequest(req);
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

        result = db->getUserFiles(user->getUsername(), output);

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
    db->getUserFiles(user->getUsername(), output);
    this->sendResponse(GET_FILES, SUCCESS, *output);
}

void KKSession::handleOpenFileRequest(KKPayload request) {
    QStringList params = request.getBodyList();


    if (params.size() > 0) {
        disconnectFromFile();
        connectToFile(params.at(0));

    } else {
        sendResponse(OPEN_FILE, BAD_REQUEST, {"Errore in fase di richiesta: non è stato inserito nessun nome file"});
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
        sendResponse(LOAD_FILE, SUCCESS, file->getCrdtText());
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleQuitFileRequest()
{
    disconnectFromFile();
    sendResponse(QUIT_FILE, SUCCESS, {});
}
void KKSession::handleCrdtRequest(KKPayload request) {
    if(file->deliver(CRDT, SUCCESS, request.getBodyList(), user->getUsername())<0){
        disconnectFromFile();
        sendResponse(QUIT_FILE, INTERNAL_SERVER_ERROR, {});
    }
}

void KKSession::handleAlignChangeRequest(KKPayload request){
    if(file->deliver(ALIGNMENT_CHANGE, SUCCESS, request.getBodyList(), "All")<0){
        disconnectFromFile();
        sendResponse(QUIT_FILE, INTERNAL_SERVER_ERROR, {});
    }
}

void KKSession::handleChatRequest(KKPayload request) {
    file->deliver(CHAT, SUCCESS, request.getBodyList(), "All");
}

void KKSession::connectToFile(QString filename)
{
    QStringList users;
    bool sendFileInfo = false;

    auto search = files->find(filename);
    if (search != files->end()) {
        // Controllo se il file risulta tra quelli già aperti.
        file = files->value(filename);
        if(file->partecipantExist(user->getUsername())){
            sendResponse(OPEN_FILE, BAD_REQUEST, {"Errore in fase di richiesta: stai già partecipando al file"});
            return;
        }

    } else {
        // Controllo se il file esiste nel DB e recupero la lista di utenti associati a quel file
        if (db->getFileUsers(filename, &users) == DB_FILE_NOT_EXIST) {
            // File non esistente, controllo se l'utente ha già creato il file con lo stesso nome
            if (db->existFileByUsername(filename, user->getUsername()) == DB_FILE_NOT_EXIST) {
                file = fileSystem->createFile(filename, user->getUsername());

                if (file != FILE_SYSTEM_CREATE_ERROR) {
                    if (db->addFile(filename, file->getHash(), user->getUsername()) != DB_INSERT_FILE_SUCCESS) {
                        file = FILE_SYSTEM_CREATE_ERROR;
                        sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Errore nell'inseriemnto del nuovo file nel database"});
                        return;
                    }
                }

            } else{
                sendResponse(OPEN_FILE, BAD_REQUEST, {"Errore in fase di richiesta: nome file già esistente"});
                return;
            }

        } else
            file = fileSystem->openFile(filename);

        if (file != FILE_SYSTEM_CREATE_ERROR) {
            // Inserisco il file nella mappa dei file attivi
            file->initCrdtText();
            files->insert(file->getHash(), file);
            file->setUsers(users);
        }
    }

    if (file != FILE_SYSTEM_CREATE_ERROR) {

        if( db->existShareFileByUsername(file->getHash(), user->getUsername()) == DB_FILE_NOT_EXIST) {

            if (db->addShareFile(file->getHash(), user->getUsername()) == DB_INSERT_FILE_SUCCESS) {
                file->join(sharedFromThis());
                file->addUser(QString("%1:%2:%3").arg(user->getUsername(), user->getAlias(), user->getImage()));

                sendFileInfo = true;
                sendResponse(OPEN_FILE, SUCCESS, {"File aperto con successo, sei stato aggiunto come partecipante",file->getHash()});

            } else
                sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Errore in fase di inserimento partecipante per il file richiesto"});

        } else {
            file->join(sharedFromThis());
            sendFileInfo = true;
            sendResponse(OPEN_FILE, SUCCESS, { "File aperto con successo, partecipazione confermata", file->getHash()});
        }

    } else
        sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Errore nel filesystem durante l'apertura del nuovo file"});


    if (sendFileInfo) {
        sendResponse(SET_PARTECIPANTS, SUCCESS, {file->getParticipants()});
        sendResponse(LOAD_FILE, SUCCESS, {file->getCrdtText()});

        // Aggiorno con gli ultimi messaggi mandati.
        KKVectorPayloadPtr queue = file->getRecentMessages();
        if(queue->length() > 0) {
            std::for_each(queue->begin(), queue->end(), [&](KKPayloadPtr d){
                socket->sendTextMessage(d->encode());
            });
        }

        // Dico a tutti che c'è un nuovo partecipante.
        file->deliver(ADDED_PARTECIPANT, SUCCESS, {user->getUsername(), user->getAlias(), user->getImage()}, "All");
    }

}

void KKSession::disconnectFromFile()
{
    if(!file.isNull()) {
        file->deliver(REMOVED_PARTECIPANT, SUCCESS, {user->getUsername(), user->getAlias()}, "All");
        file->leave(sharedFromThis());

        if (file->getParticipantCounter() < 1) {
            files->remove(file->getHash());
        }
    }
}

void KKSession::logger(QString message) {
     KKLogger::log(message, QString("%1 - %2").arg(sessionId, id));
}

