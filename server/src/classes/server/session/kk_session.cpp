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
    socket->sendTextMessage(msg->encodeHeader());
}

void KKSession::setSocket(QWebSocket* descriptor) {
    // make a new socket
    socket = descriptor;
    connect(socket, &QWebSocket::textMessageReceived, this, &KKSession::handleRequest);
    connect(socket, &QWebSocket::binaryMessageReceived, this, &KKSession::handleBinaryRequests);
    connect(socket, &QWebSocket::disconnected, this, &KKSession::handleDisconnection);
    qDebug() << "Client connected at " << descriptor;
    fileSystem->writeFile("log","Client (\""+id+"\" "
                                      +descriptor->peerAddress().toString()+":"
                                      +QString::number(descriptor->peerPort())+" ) connected at "
                                      + descriptor->localAddress().toString() +":"
                                      +QString::number(descriptor->localPort()) );
}


void KKSession::sendResponse(QString type, QString result, QString body) {
    KKPayload res(type, result, body);
    qDebug() << "Server send: " << res.encodeHeader();
    fileSystem->writeFile("log", "Server send: " + res.encodeHeader());
    socket->sendTextMessage(res.encodeHeader());
}

void KKSession::handleRequest(QString message) {
    qDebug() << "Client send:" << message;
    fileSystem->writeFile("log", "Client send: " + message);
    if (socket){
        KKPayload req(message);
        req.decodeHeader();
        if(req.getType() == "login") {
            handleLoginRequest(req);
        }
        else if(req.getType() == "signup") {
            handleSignupRequest(req);
        }
        else if(req.getType() == "openfile") {
            handleOpenFileRequest(req);
        }
        else if(req.getType() == "sharefile") {
            handleShareFileRequest(req);
        }
        else if(req.getType() == "crdt") {
            handleCrdtRequest(req);
        }
        else if(req.getType() == "chat") {
            handleChatRequest(req);
        }
    }
}

void KKSession::handleLoginRequest(KKPayload request) {
    QStringList _body = request.getBody().split("_");
    id = _body[0];
    fileSystem->writeFile("log","Client info (\"" + id + "\" " + socket->peerAddress().toString()+":" + QString::number(socket->peerPort()) + ")");
    KKTask *mytask = new KKTask([=]() {
        bool result = db->login(_body[0],_body[1]);
        if(result) {
            QStringList q=db->getUserFile(_body[0]);
            QString message ="";
            std::for_each(q.begin(), q.end(), [&](QString msg){
                message += msg + "_";
            });
            this->sendResponse("login","ok", message);
        } else {
            this->sendResponse("login","ko","Invalid credentials");
        }
    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleSignupRequest(KKPayload request) {
    QStringList _body = request.getBody().split("_");
    id = _body[0];
    KKTask *mytask = new KKTask([=]() {

        int result = db->insertUserInfo(_body[0],_body[1],_body[0],_body[2], _body[3]);
        if(result == 0) {
            this->sendResponse("signup","ok", "Succes");
        } else {
            this->sendResponse("signup","ko","Invalid Parameters");
        }

    });
    mytask->setAutoDelete(true);
    QThreadPool::globalInstance()->start(mytask);
}

void KKSession::handleOpenFileRequest(KKPayload request) {
    QString fileName = request.getBody();
    QString completeFileName = fileName;
    QString message;
    QString result = "ok";
    auto search = files->find(completeFileName);
    if (search != files->end()) {
        // il file era già aperto ed è nella mappa globale
        fileSystem->openFile(id, completeFileName);
        file = files->value(completeFileName);
        file->join(sharedFromThis());
        message = "File esistente, sei stato aggiunto correttamente";
    } else {
        // Apro il file. Con i dovuti controlli
        completeFileName = fileSystem->createFile(id, fileName);
        if(completeFileName != "ERR_CREATEFILE") {
            file = QSharedPointer<KKFile>(new KKFile());
            file->join(sharedFromThis());
            files->insert(completeFileName, file);
            auto search = files->find(completeFileName);
            if (search != files->end()) {
                //sto aprendo un file condiviso/privato
                fileSystem->openFile(id, completeFileName);
                message = "File creato, sei stato aggiunto correttamente";
            } else {
                message = "Non è stato possibile creare il file";
                result = "ko";
            }
        } else {
            message = "Non è stato possibile creare il file";
            result = "ko";
        }
    }
    fileSystem->writeFile("log", completeFileName + ": " + message);
    //mando al client la risposta della request.
    sendResponse("openfile", result, message);
    if(result == "ok") {
        // Mi aggiorno con gli ultimi messaggi mandati.
        KKVectorPayloadPtr queue = file->getRecentMessages();
        if(queue->length() > 0) {
            std::for_each(queue->begin(), queue->end(), [&](KKPayloadPtr d){
                socket->sendTextMessage(d->encodeHeader());
            });
        }
        // Dico a tutti che c'è un nuovo partecipante.
        file->deliver("addedpartecipant", "ok", id, "All");
    }
}

void KKSession::handleShareFileRequest(KKPayload request) {
    qDebug() << "TODO: Share file - " << request.getBody();
}

void KKSession::handleChatRequest(KKPayload request) {
    file->deliver("chat", "ok", request.getBody(), "All");
}

void KKSession::handleCrdtRequest(KKPayload request) {
    file->deliver("crdt", "ok", request.getBody(), id);
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
            file->deliver("removedpartecipant", "ok", id, "All");
            file->leave(sharedFromThis());
        }
        socket->deleteLater();
    }
}
