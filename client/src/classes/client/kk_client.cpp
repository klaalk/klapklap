//
// Created by Klaus on 06/05/2019.
//

#include "kk_client.h"

KKClient::KKClient(const QUrl &url, QObject *parent)
    : QObject(parent) {
    connect(&socket_, &QWebSocket::connected, this, &KKClient::handleOpenedConnection);
//    connect(&socket_, &QWebSocket::error, this, &KKClient::handleErrorConnection);
    connect(&socket_, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &KKClient::handleSslErrors);
    socket_.open(QUrl(url));
    //Editor setup
}

void KKClient::sendLoginRequest(QString email, QString password) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    sendRequest("login", "req", email + "_" + psw);
}

void KKClient::sendSignupRequest(QString email, QString password, QString name, QString surname) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    sendRequest("signup", "req", email + "_" + psw + "_" + name + "_" + surname);
}

void KKClient::sendOpenFileRequest(QString fileName) {
    sendRequest("openfile", "req", fileName);
}

void KKClient::sendMessageRequest(QString message) {
    sendRequest("chat", "req", message);
}

void KKClient::sendCrdtRequest(QString crdtType, QString crdt) {
    sendRequest("crdt", "req", crdtType + "_" + crdt);
}

void KKClient::sendRequest(QString type, QString result, QString body) {
    KKPayload req(type, result, body);
    qDebug() << "Send: " << req.encodeHeader();
    socket_.sendTextMessage(req.encodeHeader());
}

void KKClient::handleOpenedConnection() {
    qDebug() << "WebSocket connected";
    // Gestisco la lettura dei messaggi.
    connect(&socket_, &QWebSocket::textMessageReceived, this, &KKClient::handleResponse);
    connect(&login_, &LoginWindow::loginBtnClicked, this, &KKClient::sendLoginRequest);
    connect(&login_, &LoginWindow::signupBtnClicked, this, &KKClient::sendSignupRequest);
    connect(&openFile_, &OpenFileDialog::openFileRequest, this, &KKClient::sendOpenFileRequest);
    login_.show();
}

void KKClient::handleErrorConnection(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket not connected";
    qDebug() << error;
    socket_.close();
}

void KKClient::handleResponse(QString message) {
    qDebug() << "Message received:" << message;
    KKPayload res(message);
    res.decodeHeader();
    if(res.getType() == "login" && res.getResultType() == "ok") {
       login_.hide();
        QStringList files = res.getBody().split("_");
        for(QString s : files) {
            if(s!="")
                openFile_.addFile(s);
        }
       openFile_.show();
    } else if(res.getType() == "openfile" && res.getResultType() == "ok") {
       crdt_ = new KKCrdt(email_.toStdString(), casuale);
       connect(&editor_, &TextEdit::insertTextToCRDT, this, &KKClient::onInsertTextCRDT);
       connect(&editor_, &TextEdit::removeTextFromCRDT, this, &KKClient::onRemoveTextCRDT);
       connect(&chat_, &ChatDialog::sendMessageEvent, this, &KKClient::sendMessageRequest);
       connect(&chat_, &ChatDialog::siteIdClicked, this, &KKClient::onSiteIdClicked);
       openFile_.hide();
       editor_.show();
       chat_.show();
       chat_.setNickName(email_);


    } else if(res.getType() == "crdt" && res.getResultType() == "ok"){
        QStringList bodyList_ = res.getBody().split("_");
        KKCharPtr char_ = KKCharPtr(new KKChar(*bodyList_[2].toLatin1().data(), bodyList_[1].toStdString()));

        for(int i = 3; i<bodyList_.size(); i++){
            unsigned long digit = bodyList_[i].toULong();
            KKIdentifierPtr ptr = KKIdentifierPtr(new KKIdentifier(digit, bodyList_[1].toStdString()));
            char_->pushIdentifier(ptr);
        }

        unsigned long remotePos = bodyList_[0] == "insert" ? crdt_->remoteInsert(char_) : crdt_->remoteDelete(char_);
        crdt_->print();
        editor_.applyRemoteChanges(bodyList_[0], bodyList_[1], bodyList_[2], static_cast<int>(remotePos));
    } else if(res.getType() == "chat" && res.getResultType() == "ok") {
        QStringList res_ = res.getBody().split('_');
        chat_.appendMessage(res_[0], res_[1]);
     } else if(res.getType() == "addedpartecipant" && res.getResultType() == "ok") {
        chat_.addParticipant(res.getBody());
     } else if(res.getType() == "removedpartecipant" && res.getResultType() == "ok") {
        chat_.removeParticipant(res.getBody());
     }
}

void KKClient::handleSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors);
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    socket_.ignoreSslErrors();    
}

void KKClient::handleClosedConnection() {
    qApp->quit();
}

void KKClient::onInsertTextCRDT(QString diffText, int position) {
//    std::thread t([=](){
//        mtxCrdt_.lock();
        QByteArray ba = diffText.toLocal8Bit();
        QString siteId;
        char *c_str = ba.data();
        unsigned long line, col;
        for(int i = 0; *c_str != '\0'; c_str++, i++) {
            crdt_->calculateLineCol(position + i, &line, &col);
            KKCharPtr char_= crdt_->localInsert(*c_str, KKPosition(line, col));
            crdt_->print();
            QString ids = QString::fromStdString(char_->getIdentifiersString());
            sendCrdtRequest("insert", QString::fromStdString(char_->getSiteId())+ "_" + QString(char_->getValue())+ ids);
        }


//        mtxCrdt_.unlock();
//    });
//    t.detach();
}

void KKClient::onRemoveTextCRDT(int start, int end) {
    unsigned long startLine, endLine, startCol, endCol;
    crdt_->calculateLineCol(start, &startLine, &startCol);
    crdt_->calculateLineCol(end, &endLine, &endCol);
//    std::thread t([=](){
//        mtxCrdt_.lock();
//        qDebug() << startLine << " " << startCol << " - " << endLine << " " << endCol;   
        list<KKCharPtr> deletedChars = crdt_->localDelete(KKPosition(static_cast<unsigned long>(startLine),static_cast<unsigned long>(startCol)),
                            KKPosition(static_cast<unsigned long>(endLine), static_cast<unsigned long>(endCol)));
        crdt_->print();
//        mtxCrdt_.unlock();
        std::for_each(deletedChars.begin(), deletedChars.end(),[&](KKCharPtr char_){
            QString ids = QString::fromStdString(char_->getIdentifiersString());
            sendCrdtRequest("delete",QString::fromStdString(char_->getSiteId())+ "_" + QString(char_->getValue())+ ids);
        });
//    });
//    t.detach();
}

void KKClient::onSiteIdClicked(QString siteId){
    QSharedPointer<QList<int>> myList=QSharedPointer<QList<int>>(new QList<int>());
    int global = 0;
    for(list<KKCharPtr> linea: crdt_->text){
        for(KKCharPtr carattere: linea){
            if(carattere->getSiteId()==siteId.toStdString()){
                myList->push_back(global);
            }
            global++;
        }
    }
    editor_.updateSiteIdsMap(siteId, myList);
}

