//
// Created by Klaus on 06/05/2019.
//

#include "kk_session.h"

#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>


#define DEBUG

KKSession::KKSession(KKDataBasePtr db, KKFileSystemPtr filesys, KKMapFilePtr files_, KKFilePtr logFile, QObject*  parent)
    : QObject(parent), db(db), files(files_), logFile(logFile), fileSystem(filesys), user(KKUserPtr(new KKUser())) {
    QThreadPool::globalInstance()->setMaxThreadCount(5);

}

KKSession::~KKSession() {}

void KKSession::deliver(KKPayloadPtr msg) {
    socket->sendTextMessage(msg->encode());
}

void KKSession::setSocket(QWebSocket* descriptor) {
    // make a new socket
    socket = descriptor;
    connect(socket, &QWebSocket::textMessageReceived, this, &KKSession::handleRequest);
    connect(socket, &QWebSocket::binaryMessageReceived, this, &KKSession::handleBinaryRequests);
    connect(socket, &QWebSocket::disconnected, this, &KKSession::handleDisconnection);
    logger("Client " + id + ", " +descriptor->peerAddress().toString()+", "
                          + QString::number(descriptor->peerPort())+" connected at "
                          + descriptor->localAddress().toString() +", "
                          + QString::number(descriptor->localPort()));
}

QString KKSession::getSessionId() {
    return sessionId;
}

void KKSession::sendResponse(QString type, QString result, QStringList values) {
    KKPayload res(type, result, values);
    logger("Server send (" + res.encode() +")");
    socket->sendTextMessage(res.encode());
}

void KKSession::handleRequest(QString message) {
    logger("Client send " + message);
    if (socket){
        KKPayload req(message);
        req.decode();
        if(req.getRequestType() == LOGIN) {
            handleLoginRequest(req);
        }
        else if(req.getRequestType() == SIGNUP) {
            handleSignupRequest(req);
        }
        else if(req.getRequestType() == GETFILES) {
            handleGetFilesRequest();
        }
        else if(req.getRequestType() == OPENFILE) {
            handleOpenFileRequest(req);
        }
        else if(req.getRequestType() == SHAREFILE) {
//            handleShareFileRequest(req);
        }
        else if(req.getRequestType() == CRDT) {
            handleCrdtRequest(req);
        }
        else if(req.getRequestType() == CHAT) {
            handleChatRequest(req);
        }
        else if(req.getRequestType() == SAVEFILE) {
            handleSaveFileRequest(req);
        }
        else if(req.getRequestType() == LOADFILE) {
            handleLoadFileRequest(req);
        }
        else if(req.getRequestType()== ALIG){
            handleAlignChangeRequest(req);
        }
        else if(req.getRequestType() == CHANGECHARFORMAT){
            handleFormatChangeRequest(req);
        }
    }
}

void KKSession::handleLoginRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    logger("Client info " + id +", " + socket->peerAddress().toString()+", " + QString::number(socket->peerPort()));
    int result = DB_LOGIN_SUCCESS;
    result = db->loginUser(_body[0],_body[1], user);
    if(result == DB_LOGIN_SUCCESS) {
        QStringList* output = new QStringList();
        output->append(user->getName());
        output->append(user->getSurname());
        output->append(user->getEmail());
        output->append(user->getPassword());
        output->append(user->getUsername());
        output->append(user->getImage());
        output->append(user->getRegistrationDate());
        result = db->getUserFile(user, output);
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
    id = _body[0];
    int result = db->signupUser(_body[0],_body[1],_body[0],_body[2], _body[3]);
    if(result == DB_SIGNUP_SUCCESS) {
        int emailResult = smtp->sendSignupEmail(_body[0], _body[0],_body[2], _body[3]);
        if (emailResult == SEND_EMAIL_NOT_SUCCESS) {
            logger("Non è stato possibile inivare l'email a " + _body[0]);
        }
        this->sendResponse(SIGNUP, SUCCESS, {"Registrazione effettuata con successo"});
    } else if (result == DB_ERR_INSERT_EMAIL || result == DB_ERR_INSERT_USERNAME) {
        this->sendResponse(SIGNUP, BAD_REQUEST, {"Non e' stato possibile procedere con la registrazione. Username e/o Email esistenti!"});
    } else {
        this->sendResponse(SIGNUP, INTERNAL_SERVER_ERROR, {"Errore interno. Non e' stato possibile effettuare la registrazione!"});
    }
}

void KKSession::handleGetFilesRequest() {
    QStringList* output = new QStringList();
    db->getUserFile(user, output);
    this->sendResponse(GETFILES, SUCCESS, *output);
}

void KKSession::handleOpenFileRequest(KKPayload request) {
    QStringList* response = new QStringList();
    QStringList* ids = new QStringList();

    QString message, result;
    bool isActiveFile = false;

    if (request.getBodyList().size() > 0) {
        QString fileName = request.getBodyList()[0];
        auto search = files->find(fileName);
        if (search != files->end()) {
            // Controllo se il file risulta tra quelli già aperti.
            file = files->value(fileName);
            isActiveFile = true;

        } else {
            // Controllo se il file esiste nel DB e recupero la lista di utenti associati a quel file
            int dbFileExist = db->existFilename(fileName, ids);
            if (dbFileExist == DB_FILE_NOT_EXIST) {
                // File non esistente
                file = fileSystem->createFile(id, fileName);
            } else {
                // File esistente
                file = fileSystem->openFile(fileName);
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
            files->insert(file->getFilename(), file);
            file->setOwners(ids);
        }

        int dbFileExistByEmail = db->existFilenameByEmail(file->getFilename(), user->getEmail());
        if(dbFileExistByEmail == DB_FILE_NOT_EXIST) {
            int dbFileInsert = db->addUserFile(file->getFilename(), user->getEmail());

            if (dbFileInsert == DB_INSERT_FILE_SUCCESS) {
                result = SUCCESS;
                message = "File aperto con successo, sei stato aggiunto come partecipante";
                file->join(sharedFromThis());
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
        for(auto id : *file->getOwners()) {
            auto entry = participants->find(id);
            response->push_back(id + ":" + (entry != participants->end() ? PARTICIPANT_ONLINE : PARTICIPANT_OFFLINE));
        }

    } else {
        result = INTERNAL_SERVER_ERROR;
        message = "Errore nel filesystem durante l'apertura del nuovo file";
    }

    logger(message);
    response->push_front(message);
    sendResponse(OPENFILE, result, *response);

    if (result == SUCCESS) {
        // Aggiorno con gli ultimi messaggi mandati.
        KKVectorPayloadPtr queue = file->getRecentMessages();
        if(queue->length() > 0) {
            std::for_each(queue->begin(), queue->end(), [&](KKPayloadPtr d){
                socket->sendTextMessage(d->encode());
            });
        }
        // Dico a tutti che c'è un nuovo partecipante.
        file->deliver(ADDED_PARTECIPANT, SUCCESS, {id}, "All");
    }
}

void KKSession::handleChatRequest(KKPayload request) {
    file->deliver(CHAT, SUCCESS, request.getBodyList(), "All");
}

void KKSession::handleCrdtRequest(KKPayload request) {
    file->deliver(CRDT, SUCCESS, request.getBodyList(), id);
}

void KKSession::handleSaveFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    KKTask *mytask = new KKTask([=]() {
        ////  Creo il file, supponendo che non esista, se esiste questo va evitato
        //    QString filename = fileSystem->createFile(_body[1],_body[2]);
        //  Il file viene sempre creato all'apertura, mi aspetto di ricevere il nome file completo jump+salt+filename
        fileSystem->writeFile(file, _body[2]);
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleLoadFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    KKTask *mytask = new KKTask([=]() {
        ////  Apro il file, supponendo che esista, se non esiste questo va evitato
        //    QString filename = fileSystem->createFile(_body[1],_body[2]);
        //  Il file viene sempre creato all'apertura, mi aspetto di ricevere il nome file completo jump+salt+filename
        QString message = fileSystem->readFile(APPLICATION_ROOT+_body[1]);
        this->sendResponse(LOADFILE, SUCCESS, {message});
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleBinaryRequests(QByteArray message) {
    if (socket) {
        qDebug() << "Client send binary:" << message;
    }
}

void KKSession::handleDisconnection() {
    logger("Client: "+id+", "+socket->peerName()+", "
                          +socket->peerAddress().toString()+", "
                          +QString::number(socket->peerPort())+" disconnected");

    if (socket)
    {
        if(file.get() != nullptr) {
            file->deliver(REMOVED_PARTECIPANT, SUCCESS, {id}, "All");
            file->leave(sharedFromThis());
        }
        socket->deleteLater();
    }
}

void KKSession::handleAlignChangeRequest(KKPayload request){
    file->deliver(ALIG, SUCCESS, request.getBodyList(), id);
}

void KKSession::handleFormatChangeRequest(KKPayload request){
    file->deliver(CHANGECHARFORMAT,SUCCESS,request.getBodyList(),id);
}


void KKSession::logger(QString message) {
    fileSystem->writeFile(logFile, message, sessionId);
}

