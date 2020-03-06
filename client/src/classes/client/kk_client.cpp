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
    connect(&access_, &AccessDialog::loginBtnClicked, this, &KKClient::sendLoginRequest);
    connect(&access_, &AccessDialog::signupBtnClicked, this, &KKClient::sendSignupRequest);

    // Gestisco le richieste di apertura file
    connect(&openFile_, &OpenFileDialog::openFileRequest, this, &KKClient::sendOpenFileRequest);

    // Gestisco le richieste dell'editor
    connect(&editor_, &TextEdit::insertTextToCRDT, this, &KKClient::onInsertTextCRDT);
    connect(&editor_, &TextEdit::removeTextFromCRDT, this, &KKClient::onRemoveTextCRDT);
    connect(&editor_, &TextEdit::saveCRDTtoFile, this, &KKClient::onsaveCRDTtoFile);
    connect(&editor_, &TextEdit::loadCRDTtoFile, this, &KKClient::onloadCRDTtoFile);

    // Gestisco le richieste della chat
    connect(&chat_, &ChatDialog::sendMessageEvent, this, &KKClient::sendMessageRequest);
    connect(&chat_, &ChatDialog::siteIdClicked, this, &KKClient::onSiteIdClicked);

    // Gestisco il timeout
    connect(&timer_, &QTimer::timeout, this, &KKClient::handleTimeOutConnection);

    setInitState();
}

void KKClient::setInitState() {
    state_ = NOT_CONNECTED;
    timer_.start(TIMEOUT_VALUE);
    socket_.open(QUrl(url_));

    chat_.hide();
    editor_.hide();
    openFile_.hide();
    modal_.hide();

    access_.show();
    access_.showLoader(true);
}

void KKClient::sendLoginRequest(QString email, QString password) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    access_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);
    sendRequest(LOGIN, NONE, {email, psw});
}

void KKClient::sendSignupRequest(QString email, QString password, QString name, QString surname, QString username) {
    email_ = email;
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    access_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);
    sendRequest(SIGNUP, NONE, {email, psw, name, surname, username});
}

void KKClient::sendOpenFileRequest(QString fileName) {
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);
    currentfile=fileName;
    currentfileValid=false;
    sendRequest(OPENFILE, NONE, {fileName});
}

void KKClient::sendMessageRequest(QString username, QString message) {
    bool result = sendRequest(CHAT, NONE, {username, message});
    if (!result || !socket_.isValid()) {
        modal_.setModal("Attenzione! Sembra che tu non sia connesso alla rete.", "Riprova", CHAT_ERROR);
        modal_.show();
    }
}

void KKClient::sendCrdtRequest(QStringList crdt) {
    bool result = sendRequest(CRDT, NONE, crdt);
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile aggiornare il file dal server!", "Riprova", CRDT_ERROR);
        modal_.show();
    }
}

bool KKClient::sendRequest(QString type, QString result, QStringList values) {
    KKPayload req(type, result, values);
    qDebug() << "[send] -" << req.encode();
    int size = static_cast<int>(socket_.sendTextMessage(req.getData()));
    return size = req.getTotalLength();
}

void KKClient::handleOpenedConnection() {
    qDebug() << "[websocket connected]";
    state_ = CONNECTED_NOT_LOGGED;
    timer_.stop();
    access_.showLoader(false);
}

void KKClient::handleModalClosed(QString modalType) {
    QApplication::quit();
}

void KKClient::handleModalButtonClick(QString btnText, QString modalType) {
    if(modalType == CONNECTION_TIMEOUT) {
        setInitState();
    } else if (modalType == LOGIN_TIMEOUT) {
        setInitState();
    } else if (modalType == OPENFILE_TIMEOUT) {
        setInitState();
    } else  if (modalType == LOGIN_ERROR) {
        modal_.hide();
        access_.showLoader(false);
    } else if (modalType == CRDT_ERROR) {
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
    if(state_ == NOT_CONNECTED) {
        modal_.setModal("Non è stato possibile connettersi al server!", "Riprova", CONNECTION_TIMEOUT);
        modal_.show();
    } else if(state_ == CONNECTED_NOT_LOGGED) {
        modal_.setModal("Connessione al server non riuscita!", "Riprova", LOGIN_TIMEOUT);
        modal_.show();
    } else if (state_ == CONNECTED_AND_LOGGED) {
        modal_.setModal("Non è stato possibile scaricare il file dal server!", "Riprova", OPENFILE_TIMEOUT);
        modal_.show();
    }
    socket_.close();
}

void KKClient::handleErrorResponse() {
    if(state_ == CONNECTED_NOT_LOGGED) {
        modal_.setModal("Hai inserito delle credenziali non valide.\nControlla che email e/o password siano corretti.", "Chiudi", LOGIN_ERROR);
        modal_.show();
    } else if (state_ == CONNECTED_AND_LOGGED) {
        modal_.setModal("Non è stato possibile scaricare il file dal server!", "Chiudi", OPENFILE_ERROR);
        modal_.show();
    } else if (state_ == CONNECTED_AND_OPENED) {
        modal_.setModal("Non è stato possibile aggiornare il file dal server!", "Chiudi", CRDT_ERROR);
        modal_.show();
    }
}

void KKClient::handleResponse(QString message) {
    timer_.stop();
    qDebug() << "[message received] -" << message;
    KKPayload res(message);
    res.decode();
    // TODO: gestire response signup
    if(res.getRequestType() == LOGIN) {
        if (res.getResultType() == SUCCESS)
            handleLoginResponse(res);
        else
            handleErrorResponse();
    } else if(res.getRequestType() == SIGNUP) {
        if (res.getResultType() == SUCCESS)
            handleSignupResponse();
        else
            handleErrorResponse();
    } else if(res.getRequestType() == OPENFILE) {
        if (res.getResultType() == SUCCESS)
            handleOpenfileResponse();
        else
            handleErrorResponse();
    } else if(res.getRequestType() == CRDT) {
        if (res.getResultType() == SUCCESS)
            handleCrdtResponse(res);
        else
            handleErrorResponse();
    } else if(res.getRequestType() == CHAT && res.getResultType() == SUCCESS) {
        QStringList res_ = res.getBodyList();
        chat_.appendMessage(res_[0], res_[1]);
    } else if(res.getRequestType() == ADDED_PARTECIPANT && res.getResultType() == SUCCESS) {
        QStringList list = res.getBodyList();
        chat_.addParticipant(list[0]);
    } else if(res.getRequestType() == REMOVED_PARTECIPANT && res.getResultType() == SUCCESS) {
        QStringList list = res.getBodyList();
        chat_.removeParticipant(list[0]);
    } else if(res.getRequestType() == LOADFILE && res.getResultType() == SUCCESS) {
        QStringList bodyList_ = res.getBodyList();
        crdt_->loadCrdt(bodyList_[0].toStdString());
    } else {
        modal_.setModal("Errore generico", "Chiudi", GENERIC_ERROR);
        modal_.show();
    }
}

void KKClient::handleLoginResponse(KKPayload res) {
    state_= CONNECTED_AND_LOGGED;
    QStringList files = res.getBodyList();
    for(QString s : files) {
        if(s!="")
            openFile_.addFile(s);
    }
    access_.hide();
    openFile_.show();
}

void KKClient::handleSignupResponse() {
    access_.showLoader(false);
    access_.showLogin();
}

void KKClient::handleOpenfileResponse() {
    currentfileValid = true;
    state_= CONNECTED_AND_OPENED;
    crdt_ = new KKCrdt(email_.toStdString(), casuale);
    openFile_.hide();
    editor_.show();
    chat_.setNickName(email_);
    chat_.show();
}

void KKClient::handleCrdtResponse(KKPayload res) {
    // Ottengo i campi della risposta
    QStringList bodyList_ = res.getBodyList();
    for(QString l : bodyList_)
        qDebug() << l;

    int increment = bodyList_[0] == CRDT_INSERT ? 0 : 1;
    QString siteId = bodyList_[1 + increment];
    QString text = bodyList_[2 + increment];
    QStringList ids = bodyList_[3 + increment].split(" ");
    QString fontStr = bodyList_[4 + increment];


    KKCharPtr char_ = KKCharPtr(new KKChar(*text.toLatin1().data(), siteId.toStdString()));
    char_->setKKCharFont(fontStr);
    // size() - 1 per non considerare l'elemento vuoto della string list ids
    for(int i = 0; i < ids.size() - 1; i++){
        unsigned long digit = ids[i].toULong();
        KKIdentifierPtr ptr = KKIdentifierPtr(new KKIdentifier(digit, siteId.toStdString()));
        char_->pushIdentifier(ptr);
    }

    unsigned long remotePos = bodyList_[0] == CRDT_INSERT ? crdt_->remoteInsert(char_) : crdt_->remoteDelete(char_);
    QString labelName = bodyList_[0] == CRDT_INSERT ? siteId : bodyList_[1];
//xxx
    qDebug() << "FONTmandato:"<<char_->getKKCharFont();
    editor_.applyRemoteChanges(bodyList_[0], labelName, text, static_cast<int>(remotePos),char_->getKKCharFont());
}

void KKClient::handleSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors)
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
        QString font_;

        QString ids = QString::fromStdString(char_->getIdentifiersString());
        //xxx
        font_=editor_.getTextEdit()->textCursor().charFormat().font().toString();
        char_->setKKCharFont(font_); //prendo il font che sto usando e lo assegno alla mia KKChar
        sendCrdtRequest({ CRDT_INSERT, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids , font_});
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
        sendCrdtRequest({ CRDT_DELETE, email_, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids});
    });
}

void KKClient::onsaveCRDTtoFile() {
    QString message=crdt_->saveCrdt();
    QString username = crdt_->getSiteId();
    QString filename = currentfile;
    if(currentfileValid==true)
        sendRequest(SAVEFILE, NONE, {username,filename,message});
}

void KKClient::onloadCRDTtoFile() {
    QString username = crdt_->getSiteId();
    bool ok;
    QWidget tmp;
    QString filename = QInputDialog::getText(&tmp, tr("QInputDialog::getText()"),
                                            tr("User name:"), QLineEdit::Normal,
                                            QDir::home().dirName(), &ok);
    if (ok && !filename.isEmpty())
       sendRequest(LOADFILE, NONE, {username,filename});
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

