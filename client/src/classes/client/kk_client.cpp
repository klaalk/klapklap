//
// Created by Klaus on 06/05/2019.
//

#include "kk_client.h"


KKClient::KKClient(const QUrl &url, QObject *parent)
    : QObject(parent), url_(url) {

    // Gestisco l' apertura della connessione al socket
    connect(&socket_, &QWebSocket::connected, this, &KKClient::handleOpenedConnection);
    // Gestisco la lettura dei messaggi.
    connect(&socket_, &QWebSocket::textMessageReceived, this, &KKClient::handleResponse);
    // Gestisco eventuali errorri sulla connessione al socket
    connect(&socket_, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this, &KKClient::handleSslErrors);

    // Gestisco le due chiusuere della modale
    connect(&modal_, &ModalDialog::modalButtonClicked, this, &KKClient::handleModalButtonClick);
    connect(&modal_, &ModalDialog::modalClosed, this, &KKClient::handleModalClosed);

    // Gestisco le richieste di login o di registrazione
    connect(&login_, &AccessDialog::loginBtnClicked, this, &KKClient::sendLoginRequest);
    connect(&login_, &AccessDialog::signupBtnClicked, this, &KKClient::sendSignupRequest);

    // Gestisco le richieste di apertura file
    connect(&openFile_, &OpenFileDialog::openFileRequest, this, &KKClient::sendOpenFileRequest);

    // Gestisco le richieste dell'editor
    connect(&editor_, &TextEdit::insertTextToCRDT, this, &KKClient::onInsertTextCRDT);
    connect(&editor_, &TextEdit::removeTextFromCRDT, this, &KKClient::onRemoveTextCRDT);

    // Gestisco le richieste della chat
    connect(&chat_, &ChatDialog::sendMessageEvent, this, &KKClient::sendMessageRequest);
    connect(&chat_, &ChatDialog::siteIdClicked, this, &KKClient::onSiteIdClicked);

    // Gestisco il timeout
    connect(&timer_, &QTimer::timeout, this, &KKClient::handleTimeOutConnection);

    setInitState();
}

void KKClient::setInitState() {
    state_ = "NotConnected";
    timer_.start(5000);
    socket_.open(QUrl(url_));

    chat_.hide();
    editor_.hide();
    openFile_.hide();
    modal_.hide();

    login_.show();
    login_.showLoader(true);
}

void KKClient::sendLoginRequest(QString email, QString password) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    login_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(5000);
    sendRequest("login", "req", email + "_" + psw);
}

void KKClient::sendSignupRequest(QString email, QString password, QString name, QString surname, QString username) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    login_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(5000);
    sendRequest("signup", "req", email + "_" + psw + "_" + name + "_" + surname + "_" + username);
}

void KKClient::sendOpenFileRequest(QString fileName) {
    if (!timer_.isActive())
        timer_.start(5000);
    sendRequest("openfile", "req", fileName);
}

void KKClient::sendMessageRequest(QString message) {
    bool result = sendRequest("chat", "req", message);
    if (!result || !socket_.isValid()) {
        modal_.setModal("Attenzione! Sembra che tu non sia connesso alla rete.", "Riprova", "ChatError");
        modal_.show();
    }
}

void KKClient::sendCrdtRequest(QString crdtType, QString crdt) {
    bool result = sendRequest("crdt", "req", crdtType + "_" + crdt);
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile aggiornare il file dal server!", "Riprova", "CrdtError");
        modal_.show();
    }
}

bool KKClient::sendRequest(QString type, QString result, QString body) {
    KKPayload req(type, result, body);
    qDebug() << "[send] -" << req.encodeHeader();
    int size = static_cast<int>(socket_.sendTextMessage(req.encodeHeader()));
    return size = req.getTotalLength();
}

void KKClient::handleOpenedConnection() {
    qDebug() << "[websocket connected]";
    state_ = "ConnectedNotLogged";
    timer_.stop();
    login_.showLoader(false);
}

void KKClient::handleModalClosed(QString modalType) {
    QApplication::quit();
}

void KKClient::handleModalButtonClick(QString btnText, QString modalType) {
    if(modalType == "ConnectionTimeout") {
        setInitState();
    } else if (modalType == "LogginTimeout") {
        setInitState();
    } else if (modalType == "OpenfileTimeout") {
        setInitState();
    } else if (modalType == "CrdtError") {
        delete crdt_;
        editor_.resetState();
        chat_.resetState();
        setInitState();
    }
}

void KKClient::handleErrorConnection(QAbstractSocket::SocketError error) {
    qDebug() << "[websocket not connected]";
    qDebug() << error;
    socket_.close();
}

void KKClient::handleTimeOutConnection() {
    qDebug() << "[websocket timeout connection]";
    timer_.stop();
    if(state_ == "NotConnected") {
        modal_.setModal("Non è stato possibile connettersi al server!", "Riprova", "ConnectionTimeout");
        modal_.show();
    } else if(state_ == "ConnectedNotLogged") {
        modal_.setModal("Connessione al server non riuscita!", "Riprova", "LogginTimeout");
        modal_.show();
    } else if (state_ == "ConnectedAndLogged") {
        modal_.setModal("Non è stato possibile scaricare il file dal server!", "Riprova", "OpenfileTimeout");
        modal_.show();
    }
    socket_.close();
}

void KKClient::handleResponse(QString message) {
    timer_.stop();
    qDebug() << "[message received] -" << message;
    KKPayload res(message);
    res.decodeHeader();
    if(res.getType() == "login") {
        if (res.getResultType() == "ok")
            handleLoginResponse(res);
        // TODO: mostrare modale di errore
    } else if(res.getType() == "openfile") {
        if (res.getResultType() == "ok")
            handleOpenfileResponse();
        // TODO: mostrare modale di errore
    } else if(res.getType() == "crdt"){
        if (res.getResultType() == "ok")
            handleCrdtResponse(res);
        // TODO: mostrare modale di errore
    } else if(res.getType() == "chat" && res.getResultType() == "ok") {
        QStringList res_ = res.getBody().split('_');
        chat_.appendMessage(res_[0], res_[1]);
     } else if(res.getType() == "addedpartecipant" && res.getResultType() == "ok") {
        chat_.addParticipant(res.getBody());
     } else if(res.getType() == "removedpartecipant" && res.getResultType() == "ok") {
        chat_.removeParticipant(res.getBody());
     }
}

void KKClient::handleLoginResponse(KKPayload res) {
    state_="ConnectedAndLogged";
    QStringList files = res.getBody().split("_");
    for(QString s : files) {
        if(s!="")
            openFile_.addFile(s);
    }
    login_.hide();
    openFile_.show();
}

void KKClient::handleOpenfileResponse() {
    state_="ConnectedAndLoggedWithFile";
    crdt_ = new KKCrdt(email_.toStdString(), casuale);
    openFile_.hide();
    editor_.show();
    chat_.setNickName(email_);
    chat_.show();
}

void KKClient::handleCrdtResponse(KKPayload res) {
    // Ottengo i campi della risposta
    QStringList bodyList_ = res.getBody().split("_");

    int increment = bodyList_[0] == "insert" ? 0 : 1;
    QString siteId = bodyList_[1 + increment];
    QString text = bodyList_[2 + increment];

    KKCharPtr char_ = KKCharPtr(new KKChar(*text.toLatin1().data(), siteId.toStdString()));
    for(int i = 3 + increment; i<bodyList_.size(); i++){
        unsigned long digit = bodyList_[i].toULong();
        KKIdentifierPtr ptr = KKIdentifierPtr(new KKIdentifier(digit, siteId.toStdString()));
        char_->pushIdentifier(ptr);
    }
    unsigned long remotePos = bodyList_[0] == "insert" ? crdt_->remoteInsert(char_) : crdt_->remoteDelete(char_);
    QString labelName = bodyList_[0] == "insert" ? siteId : bodyList_[1];

    editor_.applyRemoteChanges(bodyList_[0], labelName, text, static_cast<int>(remotePos));
}

void KKClient::handleSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors);
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    socket_.ignoreSslErrors();    
}

void KKClient::onInsertTextCRDT(QString diffText, int position) {
    QByteArray ba = diffText.toLocal8Bit();
    QString siteId;
    char *c_str = ba.data();
    unsigned long line, col;
    for(int i = 0; *c_str != '\0'; c_str++, i++) {
        crdt_->calculateLineCol(static_cast<unsigned long>(position + i), &line, &col);
        KKCharPtr char_= crdt_->localInsert(*c_str, KKPosition(line, col));
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendCrdtRequest("insert", QString::fromStdString(char_->getSiteId())+ "_" + QString(char_->getValue())+ ids);
    }
}

void KKClient::onRemoveTextCRDT(int start, int end) {
    unsigned long startLine, endLine, startCol, endCol;
    crdt_->calculateLineCol(static_cast<unsigned long>(start), &startLine, &startCol);
    crdt_->calculateLineCol(static_cast<unsigned long>(end), &endLine, &endCol);
    list<KKCharPtr> deletedChars = crdt_->localDelete(KKPosition(static_cast<unsigned long>(startLine),static_cast<unsigned long>(startCol)),
                        KKPosition(static_cast<unsigned long>(endLine), static_cast<unsigned long>(endCol)));
    std::for_each(deletedChars.begin(), deletedChars.end(),[&](KKCharPtr char_){
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendCrdtRequest("delete", email_ + "_"+QString::fromStdString(char_->getSiteId())+ "_" + QString(char_->getValue())+ ids);
    });}

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

