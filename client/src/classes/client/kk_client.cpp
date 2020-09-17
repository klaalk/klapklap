//
// Created by Klaus on 06/05/2019.
//

#include "kk_client.h"
#include <utility>

//#define test

KKClient::KKClient(QUrl url, QObject *parent)
    : QObject(parent), url_(std::move(url)) {


    // Gestisco l' apertura della connessione al socket
    connect(&socket_, &QWebSocket::connected, this, &KKClient::handleOpenedConnection);

    // Gestisco la lettura dei messaggi.
    connect(&socket_, &QWebSocket::textMessageReceived, this, &KKClient::handleResponse);

    // Gestisco eventuali errorri sulla connessione al socket
    connect(&socket_, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this, &KKClient::handleSslErrors);

    // Gestisco le due chiusuere della modale
    connect(&modal_, &ModalDialog::modalButtonClicked, this, &KKClient::handleModalButtonClick);
    connect(&modal_, &ModalDialog::modalClosed, this, &KKClient::handleModalClosed);

    // Gestisco le richieste di login o di registrazione o logout
    connect(&access_, &AccessDialog::loginBtnClicked, this, &KKClient::sendLoginRequest);
    connect(&access_, &AccessDialog::signupBtnClicked, this, &KKClient::sendSignupRequest);
    connect(&openFile_, &OpenFileDialog::logoutRequest, this, &KKClient::sendLogoutRequest);

    // Gestisco le richieste di apertura file
    connect(&openFile_, &OpenFileDialog::openFileRequest, this, &KKClient::sendOpenFileRequest);
    connect(&openFile_, &OpenFileDialog::updateAccountRequest, this, &KKClient::sendUpdateUserRequest);

    // Gestisco il timeout
    connect(&timer_, &QTimer::timeout, this, &KKClient::handleTimeOutConnection);

    QDirIterator it(":/images/avatars", QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        avatars.push_back(it.next());
    }

    setInitState();
}

/// INIT

void KKClient::setInitState() {
    state_ = NOT_CONNECTED;
    timer_.start(TIMEOUT_VALUE);
    socket_.open(QUrl(url_));
    initTextEdit();
    initChatDialog();

    openFile_.hide();
    modal_.hide();

    access_.show();
    access_.showLoader(true);
}

void KKClient::initTextEdit() {

    if (editor_ != nullptr) delete editor_;
    editor_ = new TextEdit();
    // Gestisco le richieste dell'editor
    connect(editor_, &TextEdit::insertTextToCRDT, this, &KKClient::onInsertTextCrdt);
    connect(editor_, &TextEdit::removeTextFromCRDT, this, &KKClient::onRemoveTextCrdt);
    connect(editor_, &TextEdit::saveCRDTtoFile, this, &KKClient::onSaveCrdtToFile);
    connect(editor_, &TextEdit::openFileDialog, this, &KKClient::sendGetFilesRequest);

    //xxx
    connect(editor_,&TextEdit::alignChange, this, &KKClient::onAlignmentChange);
    connect(editor_,&TextEdit::selectionFormatChanged, this, &KKClient::onSelectionFormatChange);
    connect(editor_,&TextEdit::charFormatChange, this, &KKClient::onCharFormatChanged);

    editor_->setMySiteId(mySiteId_);
    editor_->setCurrentFileName(currentfile_);
    editor_->close();
}

void KKClient::initChatDialog() {
    if (chat_ != nullptr) delete chat_;
    chat_ = new ChatDialog();
    // Gestisco le richieste della chat
    connect(chat_, &ChatDialog::sendMessageEvent, this, &KKClient::sendMessageRequest);
    connect(chat_, &ChatDialog::siteIdClicked, this, &KKClient::onSiteIdClicked);

    chat_->setNickName(mySiteId_);
    chat_->close();
}

/// HANDLING

void KKClient::handleOpenedConnection() {
    logger("[handleOpenedConnection] - Websocket connesso");
    state_ = CONNECTED;
    timer_.stop();
    access_.showLoader(false);
}

void KKClient::handleResponse(const QString& message) {
    timer_.stop();
    logger("[handleResponse] - Ricevuto: " + message);
    KKPayload res(message);
    res.decode();
    if (res.getResultType() == SUCCESS)
        handleSuccessResponse(res);
    else
        handleErrorResponse(res);
}

void KKClient::handleSuccessResponse(KKPayload response) {

    if (response.getRequestType() == LOGIN) {
        handleLoginResponse(response);

    } else if(response.getRequestType() == SIGNUP) {
        handleSignupResponse();

    } else if(response.getRequestType() == GETFILES) {
        handleGetFilesResponse(response);

    } else if(response.getRequestType() == OPENFILE) {
        handleOpenFileResponse(response);

    } else if(response.getRequestType() == LOADFILE) {
        handleLoadFileResponse(response);

    } else if(response.getRequestType() == CRDT) {
        handleCrdtResponse(response);

    } else if(response.getRequestType() == CHAT) {
        QStringList bodyList = response.getBodyList();
        chat_->appendMessage(bodyList[0], bodyList[1]);

    } else if(response.getRequestType() == ADDED_PARTECIPANT) {
        QStringList list = response.getBodyList();
        chat_->addParticipant(list[0]);
        qDebug() << "Added participant: " << list[0];

    } else if(response.getRequestType() == REMOVED_PARTECIPANT) {
        QStringList list = response.getBodyList();
        chat_->removeParticipant(list[0]);

    } else if(response.getRequestType() == ALIGNMENT_CHANGE){
        handleAlignmentChange(response);

    } else if(response.getRequestType() == CHARFORMAT_CHANGE){
        handleCharFormatChange(response);

    } else if (response.getRequestType() == LOGOUT ) {
        handleLogoutResponse(response);
    }else {
        modal_.setModal("L'operazione è andata a buon fine.", "Chiudi", GENERIC_SUCCESS);
        modal_.show();
    }
}

void KKClient::handleLoginResponse(KKPayload res) {
    state_= CONNECTED;
    // La risposta dovrebbe contenere le info dell'utente e poi i suoi file
    QStringList bodyList = res.getBodyList();
    mySiteId_ = bodyList.value(4);
    access_.hide();
#ifndef test
    openFile_.setUserInfo(bodyList.mid(0, 7));
    openFile_.setUserAvatar(bodyList.value(7));
    openFile_.setUserFiles(bodyList.mid(8, bodyList.size()));
    openFile_.show();
#else
    this->sendOpenFileRequest("testboh13.txt");
#endif
    logger("[handleLoginResponse] - Site id: " + mySiteId_);
}

void KKClient::handleLogoutResponse(KKPayload res) {
    Q_UNUSED(res)
    state_= CONNECTED_NOT_LOGGED;

    currentfileValid_ = false;
    openFile_.hide();
    editor_->hide();
    chat_->hide();

    access_.showLoader(false);
    access_.showLogin();
    access_.show();
}


void KKClient::handleSignupResponse() {
    state_ = CONNECTED;
    access_.showLoader(false);
    access_.showLogin();
}

void KKClient::handleGetFilesResponse(KKPayload res)
{
    openFile_.setUserFiles(res.getBodyList());
    openFile_.show();
}

void KKClient::handleOpenFileResponse(KKPayload response) {
    currentfileValid_ = true;
    state_= CONNECTED_AND_OPENED;


    if (crdt_ != nullptr) delete crdt_;
    crdt_ = new KKCrdt(mySiteId_.toStdString(), casuale);


    initTextEdit();

    initChatDialog();



    editor_->show();
    chat_->setParticipants(response.getBodyList());
    chat_->show();
    openFile_.hide();
}

void KKClient::handleLoadFileResponse(KKPayload response) {
    QStringList bodyList = response.getBodyList();
    if (bodyList.isEmpty())
        return;

    crdt_->loadCrdt(bodyList);
    //crdt_->print();
    editor_->loadCrdt(crdt_->text);
}

void KKClient::handleCrdtResponse(KKPayload response) {
    // Ottengo i campi della risposta
    QStringList bodyList_ = response.getBodyList();

    // Stampo i campi
    // for(const QString& l : bodyList_)
    //  qDebug() << l;

    bool isInsert = bodyList_[0] == CRDT_INSERT;
    int increment = isInsert ? 0 : 1;
    QString siteId = bodyList_[1 + increment];
    QString text = bodyList_[2 + increment];
    QStringList ids = bodyList_[3 + increment].split(" ");
    QString fontStr = bodyList_[4 + increment];
    QString colorStr = bodyList_[5 + increment];


    KKCharPtr char_ = KKCharPtr(new KKChar(*text.toLatin1().data(), siteId.toStdString()));
    char_->setKKCharFont(fontStr);
    char_->setKKCharColor(colorStr);

    for(int i = 0; i < ids.size() - 1; i++) {
        // size() - 1 per non considerare l'elemento vuoto della string list ids
        char_->pushIdentifier(KKIdentifierPtr(new KKIdentifier(ids[i].toULong(), siteId.toStdString())));
    }

    unsigned long remotePos = isInsert ? crdt_->remoteInsert(char_) : crdt_->remoteDelete(char_);
    QString labelName = isInsert ? siteId : bodyList_[1];

    editor_->applyRemoteChanges(bodyList_[0], labelName, text, static_cast<int>(remotePos), char_->getKKCharFont(), char_->getKKCharColor());

    if(editor_->clickedAny())
        editor_->updateSiteIdsMap(labelName, findPositions(labelName));
}

void KKClient::handleAlignmentChange(KKPayload response){

    QStringList bodyList = response.getBodyList();
    QString alignment=bodyList[0];
    editor_->alignmentRemoteChange(alignment);
}

void KKClient::handleCharFormatChange(KKPayload response){
    QStringList bodyList_ = response.getBodyList();
    QString siteId = bodyList_[1];
    QString text = bodyList_[2];
    QStringList ids = bodyList_[3].split(" ");
    QString fontStr = bodyList_[4];
    QString colorStr = bodyList_[5];

    KKCharPtr char_ = KKCharPtr(new KKChar(*text.toLatin1().data(), siteId.toStdString()));

    // size() - 1 per non considerare l'elemento vuoto della string list ids

    for(int i = 0; i < ids.size() - 1; i++){
        qDebug()<<"[handleCharFormatChange] siteId char:" << siteId;
        unsigned long digit = ids[i].toULong();
        KKIdentifierPtr ptr = KKIdentifierPtr(new KKIdentifier(digit, siteId.toStdString()));
        char_->pushIdentifier(ptr);
    }

    //crdt_->print();

    unsigned long remotePos = crdt_->remoteFormatChange(char_,fontStr,colorStr);

    qDebug()<<"[handleCharFormatChange] char_value="<<char_->getValue()<<"remotePos="<<remotePos<<fontStr<<colorStr;
    editor_->singleCharFormatChange(static_cast <int>(remotePos),fontStr,colorStr);
}

void KKClient::handleErrorResponse(KKPayload response){
    if (response.getResultType() == BAD_REQUEST) {
        handleClientErrorResponse(response);
    } else if (response.getResultType() == INTERNAL_SERVER_ERROR) {
        handleServerErrorResponse(response);

    } else {
        modal_.setModal("Errore generico. Non è stato possibile gestire l'errore.", "Chiudi", GENERIC_ERROR);
        modal_.show();
    }
}

void KKClient::handleClientErrorResponse(KKPayload response) {
    QString message, button, modal;

    if (state_ == CONNECTED_NOT_LOGGED) {
        message = "Hai inserito delle credenziali non valide.\nControlla che email e/o password siano corretti.";
        button = "Chiudi";
        modal = LOGIN_ERROR;

    } else if (state_ == CONNECTED_NOT_SIGNED) {
        message = "La registrazione non è andata a buon fine. Username e/o Email esistenti!";
        button = "Riprova";
        modal = SIGNUP_ERROR;

    } else if (state_ == CONNECTED_NOT_OPENFILE) {
        message = "Non è stato possibile scaricare il file dal server";
        button = "Chiudi";
        modal = OPENFILE_ERROR;

    } else if (state_ == CONNECTED_AND_OPENED) {
        message = "Non è stato possibile aggiornare il file dal server";
        button = "Chiudi";
        modal = CRDT_ERROR;

    } else {
        message = " Errore generico nella risposta del server";
        button = "Chiudi";
        modal = GENERIC_ERROR;
    }

    QString remoteMessage = response.getBodyList().at(0);
    message.append(".\n").append(remoteMessage);
    modal_.setModal(message, button, modal);
    modal_.show();
}

void KKClient::handleServerErrorResponse(KKPayload res) {
    Q_UNUSED(res)
    modal_.setModal("Errore interno al server, non è possibile procedere.", "Riprova", SERVER_ERROR);
    modal_.show();
}

void KKClient::handleTimeOutConnection() {
    logger("[handleTimeOutConnection] - Websocket time out connection");
    timer_.stop();
    modal_.setModal("Non è stato possibile connettersi al server.", "Riprova", CONNECTION_TIMEOUT);
    modal_.show();
    socket_.close();
}

void KKClient::handleErrorConnection(QAbstractSocket::SocketError error) {
    logger(&"[handleErrorConnection] - Websocket not connected: " [error]);
    socket_.close();
}

void KKClient::handleSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors)
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    socket_.ignoreSslErrors();
}

/// SENDING

void KKClient::sendLoginRequest(QString email, const QString& password) {
#ifdef test
    email="bot"+QString::number(rand()%100);
    QString _password="none";
    KKCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(_password);
    qDebug() << psw << " "<< email;
    access_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);
    bool sended = sendRequest(LOGIN,NONE,{std::move(email), psw});
    if (sended) {
        state_ = CONNECTED_NOT_LOGGED;
    }
#else
    KKCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    access_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);
    bool sended = sendRequest(LOGIN, NONE, {std::move(email), psw});
    if (sended) {
        state_ = CONNECTED_NOT_LOGGED;
    }
#endif
}

void KKClient::sendGetFilesRequest()
{
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);

    bool sended = sendRequest(GETFILES, NONE, PAYLOAD_EMPTY_BODY);
    if (sended) {
        state_ = CONNECTED_AND_LOGGED;
    }
}

void KKClient::sendSignupRequest(QString email, const QString& password, QString name, QString surname, QString username) {
    KKCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    access_.showLoader(true);
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);
    QString avatar = avatars.at(qrand() % (avatars.size()-1)).split(":/images/avatars/")[1];
    bool sended = sendRequest(SIGNUP, NONE, {email, psw, name, surname, username, avatar});

    if (sended) {
        state_ = CONNECTED_NOT_SIGNED;
    }
}

void KKClient::sendLogoutRequest() {
    bool sended = sendRequest(LOGOUT, NONE, PAYLOAD_EMPTY_BODY);

    if (sended) {
        state_ = CONNECTED_NOT_SIGNED;
    }
}

void KKClient::sendOpenFileRequest(const QString& link, const QString& fileName) {
    if (!timer_.isActive())
        timer_.start(TIMEOUT_VALUE);

    currentfile_ = fileName;
    currentfileValid_ = false;

    if (state_ == CONNECTED_AND_OPENED) {
        chat_->close();
        editor_->close();
    }

    bool sended = sendRequest(OPENFILE, NONE, {link});
    if (sended) {
        state_ = CONNECTED_NOT_OPENFILE;
    }

}

void KKClient::sendMessageRequest(QString username, QString message) {
    bool result = sendRequest(CHAT, NONE, {std::move(username), std::move(message)});
    if (!result || !socket_.isValid()) {
        modal_.setModal("Attenzione! Sembra che tu non sia connesso alla rete.", "Riprova", CHAT_ERROR);
        modal_.show();
    }
}

void KKClient::sendUpdateUserRequest(QString name, QString surname, QString alias, QString avatar)
{
    bool result = sendRequest(UPDATE_USER, NONE, {mySiteId_, name, surname, alias, avatar});
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile aggiornare l'account.", "Chiudi", GENERIC_ERROR);
        modal_.show();
    }
}

void KKClient::sendCrdtRequest(QStringList crdt) {
    bool result = sendRequest(CRDT, NONE, std::move(crdt));
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile aggiornare il file dal server!", "Riprova", CRDT_ERROR);
        modal_.show();
    }
}

bool KKClient::sendRequest(QString type, QString result, QStringList values) {
    KKPayload req(std::move(type), std::move(result), std::move(values));
    logger("[sendRequest] - Send: " + req.encode());
    int size = static_cast<int>(socket_.sendTextMessage(req.getData()));
    return size == req.getTotalLength();
}

void KKClient::logger(QString message)
{
    KKLogger::log(message, "CLIENT ["+mySiteId_+"]");
}

/// MODAL ACTIONS

void KKClient::handleModalButtonClick(const QString& btnText, const QString& modalType) {
    Q_UNUSED(btnText)

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
        editor_->resetState();
        chat_->resetState();
        setInitState();

    } else if (modalType == OPENFILE_ERROR) {
        modal_.hide();

    } else if (modalType == GENERIC_SUCCESS) {
        modal_.hide();

    } else if (modalType == GENERIC_ERROR) {
        modal_.hide();

    } else if (modalType == SERVER_ERROR) {
        modal_.hide();
        access_.showLoader(false);
    }
}

void KKClient::handleModalClosed(const QString& modalType) {
    Q_UNUSED(modalType)
    QApplication::quit();
}

/// CRDT ACTIONS

void KKClient::onInsertTextCrdt(const QString& diffText, int position) {

    QByteArray ba = diffText.toLocal8Bit();
    QString siteId=crdt_->getSiteId(),font_,color_;
    char *c_str = ba.data();
    unsigned long line; unsigned long col;

    for(int i = 0; *c_str != '\0'; c_str++, i++) {
        editor_->getCurrentFontAndColor(position+i,&font_,&color_);
        crdt_->calculateLineCol(static_cast<unsigned long>(position + i), &line, &col);
        KKCharPtr char_= crdt_->localInsert(*c_str, KKPosition(line, col), font_, color_);
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendCrdtRequest({ CRDT_INSERT, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids , font_, color_});

    }
    if(editor_->clickedAny())
        editor_->updateSiteIdsMap(siteId,findPositions(siteId));
        crdt_->print();
}

void KKClient::onRemoveTextCrdt(int start, int end) {
    unsigned long startLine; unsigned long endLine; unsigned long startCol; unsigned long endCol;
    crdt_->calculateLineCol(static_cast<unsigned long>(start), &startLine, &startCol);
    crdt_->calculateLineCol(static_cast<unsigned long>(end), &endLine, &endCol);
     qDebug() << "[onRemoveTextCrdt]";
    list<KKCharPtr> deletedChars = crdt_->localDelete(KKPosition(static_cast<unsigned long>(startLine),static_cast<unsigned long>(startCol)),
                                                      KKPosition(static_cast<unsigned long>(endLine), static_cast<unsigned long>(endCol)));

    QString font_= editor_->getTextEdit()->textCursor().charFormat().font().toString();
    QString color_= editor_->getTextEdit()->textCursor().charFormat().foreground().color().name();

    std::for_each(deletedChars.begin(), deletedChars.end(),[&](const KKCharPtr& char_){
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendCrdtRequest({ CRDT_DELETE, mySiteId_, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids, font_, color_});
    });

    if(editor_->clickedAny())
       editor_->updateSiteIdsMap(crdt_->getSiteId(),findPositions(crdt_->getSiteId()));
    crdt_->print();
}

void KKClient::onSaveCrdtToFile() {
    if(currentfileValid_) {
        sendRequest(SAVEFILE, NONE, {crdt_->getSiteId(), currentfile_});
    }
}

void KKClient::onOpenFileDialog() {
    sendGetFilesRequest();
}

QSharedPointer<QList<int>> KKClient::findPositions(const QString& siteId){
    QSharedPointer<QList<int>> myList=QSharedPointer<QList<int>>(new QList<int>());
    int global = 0;
    for(const list<KKCharPtr>& linea: crdt_->text){
        for(const KKCharPtr& carattere: linea){
            if(carattere->getSiteId()==siteId.toStdString()){
                myList->push_front(global);
            }
            global++;
        }
    }
    return myList;
}

void KKClient::onSiteIdClicked(const QString& siteId, bool logout){
    QSharedPointer<QList<int>> myList=findPositions(siteId);
    if(!logout){
        editor_->updateSiteIdsMap(siteId, myList);
        editor_->siteIdClicked(siteId);
    }else {
        if(editor_->clickedOne(siteId))
            editor_->siteIdClicked(siteId);
    }
}

void KKClient::onAlignmentChange(QString alignment){
    QStringList a = {alignment};
    sendRequest(ALIGNMENT_CHANGE,NONE,a);
}

void KKClient::onSelectionFormatChange(int selectionStart, int selectionEnd, QTextCharFormat format){
    //qDebug() << "[onSelectionFormatChanged]";

    unsigned long startLine; unsigned long endLine; unsigned long startCol; unsigned long endCol;
    crdt_->calculateLineCol(static_cast<unsigned long>(selectionStart), &startLine, &startCol);
    crdt_->calculateLineCol(static_cast<unsigned long>(selectionEnd), &endLine, &endCol);
    list<KKCharPtr> changedChars = crdt_->changeMultipleKKCharFormat(KKPosition(static_cast<unsigned long>(startLine),static_cast<unsigned long>(startCol)),
                                                      KKPosition(static_cast<unsigned long>(endLine), static_cast<unsigned long>(endCol)),format.font().toString(), format.foreground().color().name());

    std::for_each(changedChars.begin(), changedChars.end(),[&](const KKCharPtr& char_){
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendRequest(CHARFORMAT_CHANGE, NONE, {mySiteId_, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids,format.font().toString() ,  format.foreground().color().name()});
    });

}

void KKClient::onCharFormatChanged(int pos, QTextCharFormat format){
     //qDebug() << "[onCharFormatChanged]";
     unsigned long line, col;
     crdt_->calculateLineCol(static_cast<unsigned long>(pos),&line,&col);
     KKCharPtr _char = crdt_->changeSingleKKCharFormat(KKPosition(static_cast<unsigned long>(line),static_cast<unsigned long>(col)),format.font().toString(), format.foreground().color().name());

     QString ids = QString::fromStdString(_char->getIdentifiersString());
     sendRequest(CHARFORMAT_CHANGE, NONE, {mySiteId_, QString::fromStdString(_char->getSiteId()), QString(_char->getValue()), ids,format.font().toString() ,  format.foreground().color().name()});
 }


