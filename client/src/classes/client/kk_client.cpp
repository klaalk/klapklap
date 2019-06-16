//
// Created by Klaus on 06/05/2019.
//

#include "kk_client.h"

kk_client::kk_client(const QUrl &url, QObject *parent)
    : QObject(parent) {
    connect(&socket_, &QWebSocket::connected, this, &kk_client::handleOpenedConnection);
    connect(&socket_, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &kk_client::handleSslErrors);
    socket_.open(QUrl(url));
    //Editor setup
}

void kk_client::sendLoginRequest(QString email, QString password) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    sendRequest("login", "req", email + "_" + psw);
}

void kk_client::sendSignupRequest(QString email, QString password, QString name, QString surname) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    sendRequest("signup", "req", email + "_" + psw + "_" + name + "_" + surname);
}

void kk_client::sendOpenFileRequest(QString fileName) {
    sendRequest("openfile", "req", fileName);
}

void kk_client::sendMessageRequest(QString message) {
    sendRequest("chat", "req", message);
}

void kk_client::sendCrdtRequest(QString crdtType, QString crdt) {
    sendRequest("crdt", "req", crdtType + "_" + crdt);
}

void kk_client::sendRequest(QString type, QString result, QString body) {
    kk_payload req(type, result, body);
    qDebug() << "Send: " << req.encode_header();
    socket_.sendTextMessage(req.encode_header());
}

void kk_client::handleOpenedConnection() {
    qDebug() << "WebSocket connected";
    // Gestisco la lettura dei messaggi.
    connect(&socket_, &QWebSocket::textMessageReceived, this, &kk_client::handleResponse);
    connect(&login_, &LoginWindow::loginBtnClicked, this, &kk_client::sendLoginRequest);
    connect(&login_, &LoginWindow::signupBtnClicked, this, &kk_client::sendSignupRequest);
    login_.show();
}

void kk_client::handleResponse(QString message) {
    qDebug() << "Message received:" << message;
    kk_payload res(message);
    res.decode_header();
    if(res.type() == "login" && res.result_type() == "ok") {
       login_.hide();
       //TODO: aprire la window che gestisce i file.
       // ma per il momento faccio l'apertura automatica del file.
       sendOpenFileRequest("file1");
    } else if(res.type() == "openfile" && res.result_type() == "ok") {
       crdt_ = new kk_crdt(email_.toStdString(), casuale);
       connect(&editor_, &TextEdit::insertTextToCRDT, this, &kk_client::onInsertTextCRDT);
       connect(&editor_, &TextEdit::removeTextFromCRDT, this, &kk_client::onRemoveTextCRDT);
       editor_.show();
       chat_.show();
       chat_.setNickName(email_);
       connect(&chat_, &ChatDialog::sendMessageEvent, this, &kk_client::sendMessageRequest);
    } else if(res.type() == "crdt" && res.result_type() == "ok") {
        QStringList bodyList_ = res.body().split("_");
        kk_char_ptr char_ = kk_char_ptr(new kk_char(*bodyList_[2].toLatin1().data(), bodyList_[1].toStdString()));

        for(int i = 2; i<bodyList_.size(); i++){
            unsigned long digit = bodyList_[i].toULong();
            kk_identifier_ptr ptr = kk_identifier_ptr(new kk_identifier(digit, bodyList_[1].toStdString()));
            char_->push_identifier(ptr);
        }
        if(bodyList_[0] == "insert") {
            kk_pos remotePos = crdt_->remote_insert(char_);
            editor_.applyRemoteChanges(bodyList_[0], bodyList_[1], bodyList_[2], 0, remotePos.get_ch());
        } else if(bodyList_[0] == "delete") {
            kk_pos remotePos = crdt_->remote_delete(char_);
            editor_.applyRemoteChanges(bodyList_[0], bodyList_[1], bodyList_[2], 0, remotePos.get_ch());
        }

    } else if(res.type() == "chat" && res.result_type() == "ok") {
        QStringList res_ = res.body().split('_');
        chat_.appendMessage(res_[0], res_[1]);
     } else if(res.type() == "addedpartecipant" && res.result_type() == "ok") {
        chat_.newParticipant(res.body());
     } else if(res.type() == "removedpartecipant" && res.result_type() == "ok") {
        chat_.participantLeft(res.body());
     }
}

void kk_client::handleSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors);
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    socket_.ignoreSslErrors();
}

void kk_client::handleClosedConnection() {
    qApp->quit();
}

void kk_client::onInsertTextCRDT(QString diffText, int line, int col) {
    std::thread t([=](){
        QByteArray ba = diffText.toLocal8Bit();
        char *c_str = ba.data();
        mtxCrdt_.lock();
        for(int i = 0; *c_str != '\0'; c_str++, i++) {
            kk_char_ptr char_= crdt_->local_insert(*c_str, kk_pos(line, static_cast<unsigned long>(col + i)));
            QString ids = QString::fromStdString(char_->get_identifiers_string());
            sendCrdtRequest("insert", QString::fromStdString(char_->get_siteId())+ "_" + QString(char_->get_value())+ ids);
        }
        mtxCrdt_.unlock();
    });
    t.detach();
}

void kk_client::onRemoveTextCRDT(int startLine, int startCol, int endLine, int endCol) {
    std::thread t([=](){
        mtxCrdt_.lock();
        list<kk_char_ptr> deletedChars = crdt_->local_delete(kk_pos(static_cast<unsigned long>(startLine),static_cast<unsigned long>(startCol)),
                            kk_pos(static_cast<unsigned long>(endLine), static_cast<unsigned long>(endCol)));
        mtxCrdt_.unlock();
        std::for_each(deletedChars.begin(), deletedChars.end(),[&](kk_char_ptr char_){
            QString ids = QString::fromStdString(char_->get_identifiers_string());
            sendCrdtRequest("delete",QString::fromStdString(char_->get_siteId())+ "_" + QString(char_->get_value())+ ids);
        });
    });
    t.detach();
}
