//
// Created by Klaus on 06/05/2019.
//

#include "kk_session.h"

#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>


#define DEBUG

KKSession::KKSession(KKDataBasePtr db, KKFileSystemPtr filesys, KKMapFilePtr files_, QObject*  parent)
    : QObject(parent), db(db), files(files_), fileSystem(filesys) {
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
    qDebug() << "Client connected at " << descriptor;
    fileSystem->writeFile("log", "Client (\""+id+"\" "
                                      +descriptor->peerAddress().toString()+":"
                                      +QString::number(descriptor->peerPort())+" ) connected at "
                                      + descriptor->localAddress().toString() +":"
                                      +QString::number(descriptor->localPort()) );
}


void KKSession::sendResponse(QString type, QString result, QStringList values) {
    KKPayload res(type, result, values);
    qDebug() << "Server send: " << res.encode();
    fileSystem->writeFile("log", "Server send: " + res.encode());
    socket->sendTextMessage(res.encode());
}

void KKSession::handleRequest(QString message) {
    qDebug() << "Client send:" << message;
    fileSystem->writeFile("log", "Client send: " + message);
    if (socket){
        KKPayload req(message);
        req.decode();
        qDebug() << req.getRequestType() ;
        if(req.getRequestType() == LOGIN) {
            handleLoginRequest(req);
        }
        else if(req.getRequestType() == SIGNUP) {
            handleSignupRequest(req);
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
    }
}

void KKSession::handleLoginRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
#ifndef ENV
    fileSystem->writeFile("log","Client info (\"" + id + "\" " + socket->peerAddress().toString()+":" + QString::number(socket->peerPort()) + ")");
#endif
    KKTask *mytask = new KKTask([=]() {
        bool result = true;
        #ifndef ENV
        result = db->login(_body[0],_body[1]);
        #endif
        if(result) {
            QStringList files;
            #ifndef ENV
            files = db->getUserFile(_body[0]);
            #endif
            this->sendResponse(LOGIN,SUCCESS, files);
        } else {
            this->sendResponse(LOGIN,FAILED, {"Invalid credentials"});
        }
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleSignupRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    KKTask *mytask = new KKTask([=]() {

        int result = db->insertUserInfo(_body[0],_body[1],_body[0],_body[2], _body[3]);
        if(result == 0) {
            this->sendResponse(SIGNUP, SUCCESS, {"Registrazione effettuata con successo"});
        } else {
            this->sendResponse(SIGNUP,FAILED, {"Invalid Parameters"});
        }

    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleSaveFileRequest(KKPayload request) {
    QStringList _body = request.getBodyList();
    id = _body[0];
    qDebug() << "salvo file\n"<< _body[2];
    KKTask *mytask = new KKTask([=]() {
////  Creo il file, supponendo che non esista, se esiste questo va evitato
//    QString filename = fileSystem->createFile(_body[1],_body[2]);
//  Il file viene sempre creato all'apertura, mi aspetto di ricevere il nome file completo jump+salt+filename
    fileSystem->writeFile(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/"+_body[1],_body[2]);
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleOpenFileRequest(KKPayload request) {
    QStringList list = request.getBodyList();
    QString fileName = list[0];
    QString completeFileName = fileName;
    QString message;
    QString result = SUCCESS;
    auto search = files->find(completeFileName);
    if (search != files->end()) {
#ifndef ENV
        // il file era già aperto ed è nella mappa globale
        fileSystem->openFile(id, completeFileName);
#endif
        file = files->value(completeFileName);
        file->join(sharedFromThis());
        message = "File esistente, sei stato aggiunto correttamente";
    } else {
#ifndef ENV
        // Apro il file. Con i dovuti controlli
        completeFileName = fileSystem->createFile(id, fileName);
#endif
        if(completeFileName != "ERR_CREATEFILE") {
            file = QSharedPointer<KKFile>(new KKFile());
            file->join(sharedFromThis());
            files->insert(completeFileName, file);
            auto search = files->find(completeFileName);
            if (search != files->end()) {
#ifndef ENV
                //sto aprendo un file condiviso/privato
                fileSystem->openFile(id, completeFileName);
#endif
                message = "File creato, sei stato aggiunto correttamente";
            } else {
                message = "Non è stato possibile creare il file";
                result = FAILED;
            }
        } else {
            message = "Non è stato possibile creare il file";
            result = FAILED;
        }
    }
#ifndef ENV
    fileSystem->writeFile("log", completeFileName + ": " + message);
    //mando al client la risposta della request.
#endif
    sendResponse(OPENFILE, result, {message});
    if(result == SUCCESS) {
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

void KKSession::handleBinaryRequests(QByteArray message) {
    if (socket) {
        qDebug() << "Client send binary:" << message;
    }
}

void KKSession::handleDisconnection() {
    qDebug() << "Client disconnected";
    fileSystem->writeFile("log", "Client ( \""+socket->peerName()+"\" "
                                      +socket->peerAddress().toString()+":"
                                      +QString::number(socket->peerPort())+" ) disconnected");

    if (socket)
    {
        if(file.get() != nullptr) {
            file->deliver(REMOVED_PARTECIPANT, SUCCESS, {id}, "All");
            file->leave(sharedFromThis());
        }
        socket->deleteLater();
    }
}
