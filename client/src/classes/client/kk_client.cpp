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

    initState();
}

/// INIT

void KKClient::initState() {
    state_ = NOT_CONNECTED;
    timer_.start(TIMEOUT_VALUE);
    socket_.open(QUrl(url_));
    openFile_.hide();
    modal_.hide();
    access_.show();
    access_.showLoader(true);
}

void KKClient::initEditor()
{
    logger("Inizializzazione editor...");
    if (crdt_ != nullptr) delete crdt_;
    crdt_ = new KKCrdt(user_->getUsername().toStdString(), casuale);

    if (chat_ != nullptr) delete chat_;
    chat_ = new ChatDialog();

    if (editor_ != nullptr) delete editor_;
    editor_ = new TextEdit();


    // Gestisco le richieste dell'editor
    connect(editor_, &TextEdit::insertTextToCRDT, this, &KKClient::onInsertTextCrdt);
    connect(editor_, &TextEdit::removeTextFromCRDT, this, &KKClient::onRemoveTextCrdt);
    connect(editor_, &TextEdit::saveCRDTtoFile, this, &KKClient::onSaveCrdtToFile);
    connect(editor_,&TextEdit::alignChange, this, &KKClient::onAlignmentChange);
    connect(editor_,&TextEdit::charFormatChange, this, &KKClient::onCharFormatChanged);
    connect(editor_, &TextEdit::updateSiteIdsPositions, this, &KKClient::onUpdateSiteIdsPositions);
    connect(editor_, &TextEdit::openFileDialog, this, &KKClient::onOpenFileDialog);
    connect(editor_, &TextEdit::editorClosed, this, &KKClient::onEditorClosed);

    editor_->setChatDialog(chat_);
    editor_->setMySiteId(user_->getUsername());
    editor_->setCurrentFileName(currentfile_);
    editor_->hide();

    // Gestisco le richieste della chat
    connect(chat_, &ChatDialog::sendMessageEvent, this, &KKClient::sendMessageRequest);
    connect(chat_, &ChatDialog::siteIdClicked, this, &KKClient::onSiteIdClicked);

    chat_->setNickName(user_->getAlias());
    chat_->hide();

    logger("Inizializzazione editor completata");
}

/// HANDLING

void KKClient::handleOpenedConnection() {
    logger("[handleOpenedConnection] - Websocket connesso");
    state_ = CONNECTED;
    timer_.stop();
    access_.showLoader(false);
}

void KKClient::handleTimeOutConnection() {
    logger("[handleTimeOutConnection] - Websocket time out connection");
    timer_.stop();
    modal_.setModal("Non è stato possibile connettersi al server", "Riprova", CONNECTION_TIMEOUT);
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

    } else if (response.getRequestType() == LOGOUT ) {
        handleLogoutResponse(response);

    } else if (response.getRequestType() == UPDATE_USER ) {
        handleUpdateUserResponse();

    } else if(response.getRequestType() == GET_FILES) {
        handleGetFilesResponse(response);

    } else if(response.getRequestType() == OPEN_FILE) {
        handleOpenFileResponse(response);

    } else if(response.getRequestType() == LOAD_FILE) {
        handleLoadFileResponse(response);

    } else if(response.getRequestType() == QUIT_FILE) {
        handleQuitFileResponse();

    } else if(response.getRequestType() == CRDT) {
        handleCrdtResponse(response);

    } else if(response.getRequestType() == CHAT) {
        QStringList bodyList = response.getBodyList();
        chat_->appendMessage(bodyList[0], bodyList[1]);

    } else if(response.getRequestType() == SET_PARTECIPANTS) {
        QStringList participants = response.getBodyList();
        chat_->setParticipants(participants);

    } else if(response.getRequestType() == ADDED_PARTECIPANT) {
        QStringList params = response.getBodyList();
        chat_->addParticipant(params.at(0), params.at(1), params.at(2));

    } else if(response.getRequestType() == REMOVED_PARTECIPANT) {
        QStringList params = response.getBodyList();
        chat_->removeParticipant(params.at(0), params.at(1));

    } else if(response.getRequestType() == ALIGNMENT_CHANGE){
        handleAlignmentChange(response);

    } else if(response.getRequestType() == CHARFORMAT_CHANGE){
        handleCharFormatChange(response);

    } else {
        modal_.setModal("L'operazione è andata a buon fine", "Chiudi", GENERIC_SUCCESS);
        modal_.show();
    }
}

void KKClient::handleErrorResponse(KKPayload response){
    if (response.getResultType() == BAD_REQUEST) {
        handleClientErrorResponse(response);
    } else if (response.getResultType() == INTERNAL_SERVER_ERROR) {
        handleServerErrorResponse(response);
    } else {
        modal_.setModal("Errore generico\nNon è stato possibile gestire l'errore conn il server", "Chiudi", GENERIC_ERROR);
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
    QString message, button, modal;

    if (res.getRequestType() == UPDATE_USER) {
        message = "Non è stato possibile procedere con il salvataggio!";
        button = "Chiudi";
        modal = UPDATE_USER_ERROR;
    }else if (res.getRequestType() == QUIT_FILE) {
        message = "Non è stato possibile effettuare la modifica, il file adesso verrà chiuso!";
        button = "Chiudi";
        modal = INPUT_ERROR;
    }else {
        message = "Errore interno al server. Non è possibile procedere con la richiesta!";
        button = "Riprova";
        modal = SERVER_ERROR;
    }
    modal_.setModal(message, button, modal);
    modal_.show();
}


void KKClient::handleLoginResponse(KKPayload res) {
    state_= CONNECTED;
    // La risposta dovrebbe contenere le info dell'utente e poi i suoi file
    QStringList params = res.getBodyList();

    if (user_ != nullptr) delete user_;
    user_ = new KKUser();

    user_->setName(params.at(0));
    user_->setSurname(params.at(1));
    user_->setEmail(params.at(2));
    user_->setUsername(params.at(4));
    user_->setAlias(params.at(5));
    user_->setRegistrationDate(params.at(6));
    user_->setImage(params.at(7));

    openFile_.clear();
    openFile_.setUser(user_);
    openFile_.setUserFiles(params.mid(8, params.size()));

    access_.hide();
    openFile_.show();
    logger("[handleLoginResponse] - Site id: " + user_->getUsername());
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

void KKClient::handleUpdateUserResponse()
{
    if (user_ != nullptr) {
        user_->setAlias(openFile_.getAlias());
        user_->setName(openFile_.getName());
        user_->setSurname(openFile_.getSurname());
        user_->setImage(openFile_.getAvatar());
    }
    modal_.setModal("Le tue informazioni sono state aggiornate con susccesso", "Chiudi", GENERIC_SUCCESS);
    modal_.show();
}


void KKClient::handleSignupResponse() {
    state_ = CONNECTED;
    access_.showLoader(false);
    access_.showLogin();
}

void KKClient::handleGetFilesResponse(KKPayload res)
{
    openFile_.clear();
    openFile_.setUserFiles(res.getBodyList());
    openFile_.show();
}

void KKClient::handleOpenFileResponse(KKPayload res) {
    Q_UNUSED(res)
    state_= CONNECTED_AND_OPENED;
    currentfileValid_ = true;
    initEditor();
    openFile_.hide();
    editor_->show();
    chat_->show();
}

void KKClient::handleLoadFileResponse(KKPayload res) {
    QStringList bodyList = res.getBodyList();
    if (bodyList.isEmpty())
        return;

    crdt_->loadCrdt(bodyList);
    editor_->loadCrdt(crdt_->text);
}

void KKClient::handleQuitFileResponse()
{
    editor_->hide();
    chat_->hide();
    sendGetFilesRequest();
}

void KKClient::handleCrdtResponse(KKPayload response) {
    // Ottengo i campi della risposta
    QStringList bodyList_ = response.getBodyList();

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
    siteId = isInsert ? siteId : bodyList_[1];
    editor_->applyRemoteChanges(bodyList_[0], siteId, text, static_cast<int>(remotePos), char_->getKKCharFont(), char_->getKKCharColor());
    editor_->applySiteIdsPositions(siteId, findPositions(siteId));
}

void KKClient::handleAlignmentChange(KKPayload response){

    QStringList bodyList = response.getBodyList();
    QString alignment=bodyList[0];
    editor_->applyRemoteAlignmentChange(alignment);

    QString siteId = crdt_->getSiteId();
    editor_->applySiteIdsPositions(siteId, findPositions(siteId));

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
        unsigned long digit = ids[i].toULong();
        KKIdentifierPtr ptr = KKIdentifierPtr(new KKIdentifier(digit, siteId.toStdString()));
        char_->pushIdentifier(ptr);
    }
    unsigned long remotePos = crdt_->remoteFormatChange(char_,fontStr,colorStr);
    editor_->applyRemoteFormatChange(static_cast <int>(remotePos),fontStr,colorStr);
    editor_->applySiteIdsPositions(siteId, findPositions(siteId));
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

    bool sended = sendRequest(GET_FILES, NONE, PAYLOAD_EMPTY_BODY);
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

    bool sended = sendRequest(OPEN_FILE, NONE, {link});
    if (sended) {
        state_ = CONNECTED_NOT_OPENFILE;
    }

}

void KKClient::sendMessageRequest(QString username, QString message) {
    bool result = sendRequest(CHAT, NONE, {std::move(username), std::move(message)});
    if (!result || !socket_.isValid()) {
        modal_.setModal("Attenzione!\nSembra che tu non sia connesso alla rete", "Riprova", CHAT_ERROR);
        modal_.show();
    }
}

void KKClient::sendUpdateUserRequest(QString name, QString surname, QString alias, QString avatar)
{
    bool result = sendRequest(UPDATE_USER, NONE, {user_->getUsername(), name, surname, alias, avatar});
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile aggiornare l'account", "Chiudi", GENERIC_ERROR);
        modal_.show();
    }
}

void KKClient::onEditorClosed()
{
    bool result = sendRequest(QUIT_FILE, NONE, {});
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile chiudere il file", "Chiudi", GENERIC_ERROR);
        modal_.show();
    }
}

void KKClient::sendCrdtRequest(QStringList crdt) {
    bool result = sendRequest(CRDT, NONE, std::move(crdt));
    if (!result || !socket_.isValid()) {
        modal_.setModal("Non è stato possibile aggiornare il file dal server", "Riprova", CRDT_ERROR);
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
    QString identifier = (user_!=nullptr) ? user_->getUsername() : "unknown";
    KKLogger::log(message, QString("CLIENT - %1").arg(identifier));
}

/// MODAL ACTIONS

void KKClient::handleModalButtonClick(const QString& btnText, const QString& modalType) {
    Q_UNUSED(btnText)

    if(modalType == CONNECTION_TIMEOUT) {
        initState();

    } else if (modalType == LOGIN_TIMEOUT) {
        initState();

    } else if (modalType == OPENFILE_TIMEOUT) {
        initState();

    } else  if (modalType == LOGIN_ERROR) {
        modal_.hide();
        access_.showLoader(false);

    } else if (modalType == UPDATE_USER_ERROR) {
        openFile_.setUser(user_);

    } else if (modalType == CRDT_ERROR || modalType == CHAT_ERROR || modalType == INPUT_ERROR) {
        modal_.hide();
        editor_->close();

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

void KKClient::onInsertTextCrdt(char value, unsigned long position, QString font_, QString color_) {
    unsigned long line, col;
    crdt_->calculateLineCol(position, &line, &col);
    KKCharPtr char_ = crdt_->localInsert(value, KKPosition(line, col), font_, color_);
    QString ids = QString::fromStdString(char_->getIdentifiersString());
    sendCrdtRequest({CRDT_INSERT, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids , font_, color_});
}

void KKClient::onRemoveTextCrdt(unsigned long start, unsigned long end) {
    unsigned long startLine, endLine, startCol, endCol;
    crdt_->calculateLineCol(start, &startLine, &startCol);
    crdt_->calculateLineCol(end, &endLine, &endCol);
    list<KKCharPtr> deletedChars = crdt_->localDelete(KKPosition(startLine, startCol), KKPosition(endLine, endCol));

    QTextCharFormat format = editor_->getTextEdit()->textCursor().charFormat();
    QString font_= format.font().toString();
    QString color_= format.foreground().color().name();

    std::for_each(deletedChars.begin(), deletedChars.end(),[&](const KKCharPtr& char_){
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendCrdtRequest({CRDT_DELETE, user_->getUsername(), QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids, font_, color_});
    });
}

void KKClient::onSaveCrdtToFile() {
    if(currentfileValid_) {
        sendRequest(SAVE_FILE, NONE, {crdt_->getSiteId(), currentfile_});
    }
}

void KKClient::onOpenFileDialog() {
    sendGetFilesRequest();
}

QSharedPointer<QList<int>> KKClient::findPositions(const QString& siteId){
    QSharedPointer<QList<int>> myList = QSharedPointer<QList<int>>(new QList<int>());
    int global = 0;
    for(const list<KKCharPtr>& linea: crdt_->text){
        for(const KKCharPtr& carattere: linea){
            if(carattere->getSiteId() == siteId.toStdString()){
                myList->push_front(global);
            }
            global++;
        }
    }
    return myList;
}

void KKClient::onSiteIdClicked(const QString& siteId)
{
    onUpdateSiteIdsPositions(siteId);
    editor_->applySiteIdClicked(siteId);
}

void KKClient::onUpdateSiteIdsPositions(const QString &siteId)
{
    editor_->applySiteIdsPositions(siteId, findPositions(siteId));
}

void KKClient::onAlignmentChange(QString alignment){
    QStringList a = {alignment};
    sendRequest(ALIGNMENT_CHANGE,NONE,a);
}

void KKClient::onCharFormatChanged(unsigned long pos, QString font_, QString color_){
     unsigned long line, col;
     crdt_->calculateLineCol(pos, &line, &col);
     KKCharPtr _char = crdt_->changeSingleKKCharFormat(KKPosition(line, col), font_, color_);
     if (_char != nullptr) {
         QString ids = QString::fromStdString(_char->getIdentifiersString());
         sendRequest(CHARFORMAT_CHANGE, NONE, {user_->getUsername(), QString::fromStdString(_char->getSiteId()), QString(_char->getValue()), ids, font_, color_});
     }
 }


