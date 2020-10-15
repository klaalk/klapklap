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
    : QObject(parent), db(db), files(files), fileSystem(fileSystem), user(KKUserPtr(new KKUser())), sessionId(sessionId) {

    QThreadPool::globalInstance()->setMaxThreadCount(5);
}

KKSession::~KKSession() {
    socketMutex.lock();
    if (!socket.isNull())
        socket->deleteLater();
    socketMutex.unlock();

    if (!user.isNull())
        user->deleteLater();

    logger("Session deconstructed");
}

void KKSession::deliver(KKPayload msg, bool log) {
    socketMutex.lock();
    QString data = msg.encode();
    if (log)
        logger(QString("[deliver] - %1 ").arg(data));
    socket->sendTextMessage(data);
    socketMutex.unlock();
}

QString KKSession::getSessionId() {
    return sessionId;
}

void KKSession::setSocket(QSharedPointer<QWebSocket> descriptor) {
    // make a new socket
    socketMutex.lock();
    socket = descriptor;
    connect(socket.get(), &QWebSocket::textMessageReceived, this, &KKSession::handleRequest);
    connect(socket.get(), &QWebSocket::binaryMessageReceived, this, &KKSession::handleBinaryRequests);
    connect(socket.get(), &QWebSocket::disconnected, this, &KKSession::handleDisconnection);
    logger(QString("[setSocket] - %1, %2 connected at %3, %4").arg(
               descriptor->peerAddress().toString(),
               QString::number(descriptor->peerPort()),
               descriptor->localAddress().toString(),
               QString::number(descriptor->localPort())));
    socketMutex.unlock();
}

void KKSession::sendResponse(QString type, QString result, QStringList values) {
    deliver(KKPayload(type, result, values), true);
}

void KKSession::handleRequest(QString message) {
    logger(QString("[handelRequest] - %1").arg(message));

    socketMutex.lock();
    if (socket) {
        socketMutex.unlock();
        KKPayload req(message);
        req.decode();
        if (req.getRequestType() == LOGIN) {
            handleLoginRequest(req);
        } else if(req.getRequestType() == SIGNUP) {
            handleSignupRequest(req);
        } else if(req.getRequestType() == LOGOUT){
            handleLogoutRequest(req);
        } else if(req.getRequestType() == UPDATE_USER) {
            handleUpdateUserRequest(req);
        } else if(req.getRequestType() == GET_FILES) {
            handleGetFilesRequest();
        } else if(req.getRequestType() == OPEN_FILE) {
            handleOpenFileRequest(req);
        } else if(req.getRequestType() == SAVE_FILE) {
            handleSaveFileRequest(req);
        } else if(req.getRequestType() == LOAD_FILE) {
            handleLoadFileRequest(req);
        } else if (req.getRequestType() == QUIT_FILE) {
            handleQuitFileRequest();
        } else if(req.getRequestType() == CRDT) {
            handleCrdtRequest(req);
        } else if(req.getRequestType() == CHAT) {
            handleChatRequest(req);
        }

    } else
        socketMutex.unlock();
}

void KKSession::handleBinaryRequests(QByteArray message) {
    socketMutex.lock();
    if (socket) {
        logger(QString("[handleBinaryRequests] - %1").arg(QVariant(message).toString()));
    }
    socketMutex.unlock();
}

void KKSession::handleDisconnection() {
    socketMutex.lock();
    logger(QString("[handleDisconnection] - Session %1, %2, %3 closing...")
           .arg(id, socket->peerAddress().toString(), QString::number(socket->peerPort())));

    socketMutex.unlock();
    disconnectFromFile();
    emit disconnected(sessionId);
}

void KKSession::handleLoginRequest(KKPayload request) {
    QStringList params = request.getBodyList();

    int result = DB_LOGIN_SUCCESS;
    result = db->loginUser(params[0], params[1], user);

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
            logger("Non è stato possibile recuperare i file associati a " + params[0]);
        }

        sendResponse(LOGIN, SUCCESS, *output);

    } else if (result == DB_LOGIN_FAILED) {
        sendResponse(LOGIN, BAD_REQUEST, {"Credenziali non valide"});

    } else if (result == DB_ERR_USER_NOT_FOUND) {
        sendResponse(LOGIN, BAD_REQUEST, {"Account non esistente"});

    } else {
        sendResponse(LOGIN, INTERNAL_SERVER_ERROR, {"Errore interno. Non è stato possibile effettuare il login!"});
    }
}

void KKSession::handleSignupRequest(KKPayload request) {
    QStringList params = request.getBodyList();
    int result = db->signupUser(params[4],params[1],params[0],params[2], params[3], params[5]);

    if(result == DB_SIGNUP_SUCCESS) {
        sendResponse(SIGNUP, SUCCESS, {"Registrazione effettuata con successo"});

    } else if (result == DB_ERR_INSERT_EMAIL || result == DB_ERR_INSERT_USERNAME) {
        sendResponse(SIGNUP, BAD_REQUEST, {"Errore nella richiesta, username e/o Email esistenti!"});

    } else {
        sendResponse(SIGNUP, INTERNAL_SERVER_ERROR, {"Errore interno. Non e' stato possibile effettuare la registrazione!"});
    }
}

void KKSession::handleLogoutRequest(KKPayload request) {
    Q_UNUSED(request)
    disconnectFromFile();
    sendResponse(LOGOUT, SUCCESS, {"Logut eseguito"});
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

        auto search = files->find(params.at(0));

        if (search != files->end()) {
            if (files->value(params.at(0))->partecipantExist(user->getUsername())) {
                sendResponse(OPEN_FILE, BAD_REQUEST, {"Errore in fase di richiesta: stai già partecipando al file"});
                return;
            }

            if (files->value(params.at(0))->getPartecipantsNumber()>=25) {
                sendResponse(OPEN_FILE, BAD_REQUEST, {"Errore in fase di richiesta: numero massimo di partecipanti attivi raggiunto"});
                return;
            }
        }

        disconnectFromFile();
        connectToFile(params.at(0));
    } else {
        sendResponse(OPEN_FILE, BAD_REQUEST, {"Errore in fase di richiesta: non è stato inserito nessun nome file"});
    }
}


void KKSession::handleSaveFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();

    KKTask *mytask = new KKTask([&]() {
        file->flushCrdtText();
    });

    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleLoadFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();

    KKTask *mytask = new KKTask([&]() {
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
    QStringList body = request.getBodyList();
    QString operation = body.at(0);
    file->produceMessages(request, operation == CRDT_ALIGNM ? "All" : user->getUsername());
}

void KKSession::handleChatRequest(KKPayload request) {
    file->produceMessages(request, "All");
}

void KKSession::connectToFile(QString filename)
{
    QStringList users;
    QString result = INTERNAL_SERVER_ERROR;

    auto search = files->find(filename);
    if (search != files->end()) {
        // Controllo se il file risulta tra quelli già aperti.
        file = files->value(filename);
    } else {
        // Controllo se il file esiste nel DB e recupero la lista di utenti associati a quel file
        if (db->getShareFileUsers(filename, &users) == DB_FILE_NOT_EXIST) {

            // File non esistente, controllo se l'utente ha già creato il file con lo stesso nome
            if (db->existFileByUsername(filename, user->getUsername()) == DB_FILE_NOT_EXIST) {
                file = fileSystem->createFile(filename, user->getUsername());

                if (file != FILE_SYSTEM_CREATE_ERROR) {
                    if (db->addFile(filename, file->getHash(), user->getUsername()) != DB_INSERT_FILE_SUCCESS) {
                        file = FILE_SYSTEM_CREATE_ERROR;
                        sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Errore nell'inseriemento del nuovo file nel database"});
                        return;
                    }
                } else {
                    sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Non è stato possibile creare il file"});
                    return;
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
            file->setUsers(users);
            files->insert(file->getHash(), file);
        }
    }

    if (file != FILE_SYSTEM_CREATE_ERROR) {

        if( db->existShareFileByUsername(file->getHash(), user->getUsername()) == DB_FILE_NOT_EXIST) {

            if (db->addShareFile(file->getHash(), user->getUsername()) == DB_INSERT_FILE_SUCCESS) {
                file->addUser(QString("%1:%2:%3").arg(user->getUsername(), user->getAlias(), user->getImage()));
                result = SUCCESS;
            } else sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Errore in fase di inserimento partecipante per il file richiesto"});

        } else result = SUCCESS;

    } else sendResponse(OPEN_FILE, INTERNAL_SERVER_ERROR, {"Errore nel filesystem durante l'apertura del nuovo file"});

    if (result == SUCCESS) {
        sendResponse(OPEN_FILE, SUCCESS, { "File aperto con successo, partecipazione confermata", file->getHash()});
        file->join(sharedFromThis());
        file->produceMessages(KKPayload(ADDED_PARTECIPANT, SUCCESS, {user->getUsername(), user->getAlias(), user->getImage()}), user->getUsername());
    }

}

void KKSession::disconnectFromFile()
{
    if(!file.isNull()) {
        file->produceMessages(KKPayload(REMOVED_PARTECIPANT, SUCCESS, {user->getUsername(), user->getAlias(), user->getImage()}), "All");
        file->leave(sharedFromThis());

        if (file->getPartecipantsNumber() < 1) {
            files->remove(file->getHash());
        }
    }
}

void KKSession::logger(QString message) {
    loggerMutex.lock();
    KKLogger::log(message, QString("%1 - %2").arg(sessionId, id));
    loggerMutex.unlock();
}

