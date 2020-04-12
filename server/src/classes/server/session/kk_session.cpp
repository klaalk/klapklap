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
    : QObject(parent), db(db), files(files_),logFile(logFile), fileSystem(filesys), user(KKUserPtr(new KKUser())) {
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
    fileSystem->writeFile(logFile, "Client "+id+", "
                          +descriptor->peerAddress().toString()+", "
                          +QString::number(descriptor->peerPort())+" connected at "
                          + descriptor->localAddress().toString() +", "
                          +QString::number(descriptor->localPort()) );
}

void KKSession::sendResponse(QString type, QString result, QStringList values) {
    KKPayload res(type, result, values);
    fileSystem->writeFile(logFile, "Server send (" + res.encode() +")");
    socket->sendTextMessage(res.encode());
}

void KKSession::handleRequest(QString message) {
    fileSystem->writeFile(logFile, "Client send " + message);
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
        else if(req.getRequestType() == "sharefile") {
            handleShareFileRequest(req);
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
        else if(req.getRequestType()==ALIG){
            handleAlignChangeRequest(req);
        }
        else if(req.getRequestType()==CHANGECHARFORMAT){
            handleFormatChangeRequest(req);
        }
    }
}

void KKSession::handleLoginRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    fileSystem->writeFile(logFile, "Client info " + id +", " + socket->peerAddress().toString()+", " + QString::number(socket->peerPort()));
    KKTask *mytask = new KKTask([=]() {
        int result = DB_LOGIN_SUCCESS;
        result = db->loginUser(_body[0],_body[1], user);
        if(result == DB_LOGIN_SUCCESS) {
            QStringList* output = new QStringList();
            output->append(user->getName());
            output->append(user->getSurname());
            output->append(user->getEmail());
            output->append(user->getPassword());
            output->append(user->getUsername());
            //          TODO: inserire immagine
            //            output->append(user->image);
            output->append(user->getRegistrationDate());
            result = db->getUserFile(user, output);
            this->sendResponse(LOGIN, SUCCESS, *output);
        } else if (result == DB_LOGIN_FAILED) {
            this->sendResponse(LOGIN, BAD_REQUEST, {"Credenziali non valide"});
        } else if (result == DB_ERR_USER_NOT_FOUND) {
            this->sendResponse(LOGIN, BAD_REQUEST, {"Account non esistente"});
        } else {
            this->sendResponse(LOGIN, INTERNAL_SERVER_ERROR, {"Errore interno. Non è stato possibile effettuare il login!"});
        }
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleSignupRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    KKTask *mytask = new KKTask([=]() {
        int result = db->signupUser(_body[0],_body[1],_body[0],_body[2], _body[3]);
        if(result == DB_SIGNUP_SUCCESS) {
            int emailResult = smtp->sendSignupEmail(_body[0], _body[0],_body[2], _body[3]);
            if (emailResult == SEND_EMAIL_NOT_SUCCESS) {
                fileSystem->writeFile(logFile, "Non è stato possibile inivare l'email a " + _body[0]);
            }
            this->sendResponse(SIGNUP, SUCCESS, {"Registrazione effettuata con successo"});
        } else if (result == DB_ERR_INSERT_EMAIL || result == DB_ERR_INSERT_USERNAME) {
            this->sendResponse(SIGNUP, BAD_REQUEST, {"Non e' stato possibile procedere con la registrazione. Username e/o Email esistenti!"});
        } else {
            this->sendResponse(SIGNUP, INTERNAL_SERVER_ERROR, {"Errore interno. Non e' stato possibile effettuare la registrazione!"});
        }
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleGetFilesRequest()
{
    QStringList* output = new QStringList();
    db->getUserFile(user, output);
    this->sendResponse(GETFILES, SUCCESS, *output);
}

void KKSession::handleOpenFileRequest(KKPayload request) {
    QString message= "Non è stato inserito nessuno nome file";
    QString result = BAD_REQUEST;
    QString fileName;
    if (request.getBodyList().size() > 0) {
        fileName = request.getBodyList()[0];
        result = INTERNAL_SERVER_ERROR;
        message= "Non è stato possibile aggiungere il file nel database";

        int dbFileInsert = DB_INSERT_FILE_SUCCESS;
        int dbFileExist;

        auto search = files->find(fileName);

        if (search != files->end()) {
            file = files->value(fileName);
            file->join(sharedFromThis());
            result = SUCCESS;
            message = "File esistente, sei stato aggiunto correttamente";

        } else {
            //        Controllo se il file esiste nel DB
            dbFileExist = db->existFilename(fileName);
            if(dbFileExist==DB_FILE_NOT_EXIST){
                file = fileSystem->createFile(id, fileName);
                if(file != FILE_SYSTEM_CREATE_ERROR)
                    message = "File creato";
                else
                    message = "File non creato";
            } else {
                dbFileExist = db->existFilenameByUserId(fileName, user->getId());
                file = fileSystem->openFile(fileName);
                message= "File esistente";
            }


            fileName = file->getFilename();
            if(dbFileExist == DB_FILE_NOT_EXIST && file != FILE_SYSTEM_CREATE_ERROR ) {
                dbFileInsert = db->addUserFile(fileName, APPLICATION_ROOT + fileName, user);
                int emailResult = smtp->sendAddUserFileEmail(user, fileName);
                if (emailResult == SEND_EMAIL_NOT_SUCCESS) {
                    fileSystem->writeFile(logFile, "Non è stato possibile inivare l'email a " + user->getEmail());
                }
            }

            if(dbFileInsert == DB_INSERT_FILE_SUCCESS && file != FILE_SYSTEM_CREATE_ERROR){
                file->join(sharedFromThis());
                files->insert(fileName, file);
                result = SUCCESS;
                message+=", sei stato aggiunto correttamente";
            }else{
                message+=", si verificato un errore";
            }

        }
    }

    fileSystem->writeFile(logFile, fileName + ": " + message);

    // invio al client la risposta della request.
    sendResponse(OPENFILE, result, {message});

    if (result == SUCCESS) {
        // Mi aggiorno con gli ultimi messaggi mandati.
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

void KKSession::handleShareFileRequest(KKPayload request) {
    qDebug() << "TODO: Share file - " << request.getData();
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
        fileSystem->writeFile(file,_body[2]);
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
    fileSystem->writeFile(logFile, "Client: "+id+", "+socket->peerName()+", "
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
