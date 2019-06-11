//
// Created by Klaus on 06/05/2019.
//

#include "kk_session.h"

#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>


#define DEBUG

kk_session::kk_session(kk_db_ptr db, map_files_ptr files_, QObject*  parent)
        : QObject(parent), db_(db), files_(files_) {
    QThreadPool::globalInstance()->setMaxThreadCount(5);
}

kk_session::~kk_session() {}

void kk_session::deliver(kk_payload_ptr msg) {
    session_socket_->sendTextMessage(msg->encode_header());
}

void kk_session::setSocket(QWebSocket* descriptor) {
    // make a new socket
    session_socket_ = descriptor;
    connect(session_socket_, &QWebSocket::textMessageReceived, this, &kk_session::handleRequest);
    connect(session_socket_, &QWebSocket::binaryMessageReceived, this, &kk_session::handleBinaryRequests);
    connect(session_socket_, &QWebSocket::disconnected, this, &kk_session::handleDisconnection);
    qDebug() << "Client connected at " << descriptor;
}


void kk_session::sendResponse(QString type, QString result, QString body) {
    kk_payload res(type, result, body);
    qDebug() << "Server send: " << res.encode_header();
    session_socket_->sendTextMessage(res.encode_header());
}

void kk_session::handleRequest(QString message) {
    qDebug() << "Client send:" << message;
    if (session_socket_){
        kk_payload req(message);
        req.decode_header();
        if(req.type() == "login") {
            QStringList _body = req.body().split("_");
            nick_ = _body[0];
            kk_task *mytask = new kk_task([=]() {
                bool result = db_->db_login(_body[0],_body[1]);
                if(result) {
                    QStringList q=db_->db_getUserFile(_body[0]);
                    QString message;
                    std::for_each(q.begin(), q.end(), [&](QString msg){
                        message += msg + "_";
                    });
                    this->sendResponse("login","ok", message);

                } else {
                    this->sendResponse("login","ko","Invalid credentials");
                }
            });

            mytask->setAutoDelete(true);
            // using queued connection
            qDebug() << "Starting a new task using a thread from the QThreadPool";
            QThreadPool::globalInstance()->start(mytask);

        } else if(req.type() == "signup") {
            QStringList _body = req.body().split("_");
            nick_ = _body[0];
            kk_task *mytask = new kk_task([=]() {

                bool result = db_->db_insert_user(_body[0],_body[1],_body[0],_body[2], _body[3]);
                if(result) {
                    this->sendResponse("signup","ok", "Succes");
                } else {
                    this->sendResponse("signup","ko","Invalid Parameters");
                }

            });

            mytask->setAutoDelete(true);
            // using queued connection
            qDebug() << "Starting a new task using a thread from the QThreadPool";
            QThreadPool::globalInstance()->start(mytask);
        } else if(req.type() == "openfile") {
            QString fileName = req.body();
            QString message;
            QString result = "ok";
            auto search = files_.get()->find(fileName);
            if (search != files_.get()->end()) {
                // il file era già aperto ed è nella mappa globale
                actual_file_ = files_.get()->value(fileName);
                actual_file_->join(sharedFromThis());
                message = "file esistente, sei stato aggiunto correttamente";
            } else {
                // Apro il file. Con i dovuti controlli
                // TODO: fare query per inserire file
                actual_file_ = QSharedPointer<kk_file>(new kk_file());
                actual_file_->join(sharedFromThis());
                files_.get()->insert(fileName, actual_file_);
                auto search = files_.get()->find(fileName);
                if (search != files_.get()->end()) {
                    qDebug() << "file creato, sei stato aggiunto correttamente";
                    message = "file creato, sei stato aggiunto correttamente";
                } else {
                     message = "non è stato possibile creare il file";
                     result = "ko";
                }
            }
            //mando al client la risposta della request.
            sendResponse("openfile", result, message);

            if(result == "ok") {
                // Mi aggiorno con gli ultimi messaggi mandati.
                queue_payload_ptr queue = actual_file_->getRecentMessages();
                if(queue->length() > 0) {
                    std::for_each(queue->begin(), queue->end(), [&](kk_payload_ptr d){
                        session_socket_->sendTextMessage(d->encode_header());
                    });
                }
                // Dico a tutti che c'è un nuovo partecipante.
                actual_file_->deliver("addedpartecipant", "ok", nick_);
            }
        } else if(req.type() == "sharefile") {

        } else if(req.type() == "crdt") {

        } else if(req.type() == "chat") {
            actual_file_->deliver("chat", "ok", req.body());
        }
    }
}

void kk_session::handleBinaryRequests(QByteArray message)
{
    if (session_socket_)
    {
        qDebug() << "Client send binary:" << message;
//        session_socket_->sendBinaryMessage(message);
    }
}

void kk_session::handleDisconnection()
{
    qDebug() << "Client disconnected";
    if (session_socket_)
    {
//        clients_.removeAll(pClient);
        if(actual_file_.get() != nullptr) {
            actual_file_->deliver("removedpartecipant", "ok", nick_);
            actual_file_->leave(sharedFromThis());
        }
        session_socket_->deleteLater();
    }
}
