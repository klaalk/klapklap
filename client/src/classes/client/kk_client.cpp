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
    if (crdt != nullptr) delete crdt;
    crdt = new KKCrdt(user->getUsername().toStdString(), casuale);

    if (chat != nullptr) delete chat;
    chat = new ChatDialog();

    if (editor != nullptr) delete editor;
    editor = new KKEditor();


    // Gestisco le richieste dell'editor
    connect(editor, &KKEditor::insertTextToCRDT, this, &KKClient::onInsertTextCrdt);
    connect(editor, &KKEditor::removeTextFromCRDT, this, &KKClient::onRemoveTextCrdt);
    connect(editor, &KKEditor::saveCRDTtoFile, this, &KKClient::onSaveCrdtToFile);
    connect(editor, &KKEditor::alignChange, this, &KKClient::onAlignmentChange);
    connect(editor, &KKEditor::alignmentNotifyEvent, this, &KKClient::notifyAlignment);
    connect(editor,&KKEditor::charFormatChange, this, &KKClient::onCharFormatChanged);
    connect(editor, &KKEditor::updateSiteIdsPositions, this, &KKClient::onUpdateSiteIdsPositions);
    connect(editor, &KKEditor::openFileDialog, this, &KKClient::onOpenFileDialog);
    connect(editor, &KKEditor::editorClosed, this, &KKClient::onEditorClosed);

    editor->setChatDialog(chat);
    editor->setMySiteId(user->getUsername());
    editor->setCurrentFileName(currentfile_);
    editor->hide();

    // Gestisco le richieste della chat
    connect(chat, &ChatDialog::sendMessageEvent, this, &KKClient::sendMessageRequest);
    connect(chat, &ChatDialog::siteIdClicked, this, &KKClient::onSiteIdClicked);

    chat->setNickName(user->getAlias());
    chat->hide();

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
        chat->appendMessage(bodyList[0], bodyList[1]);

    } else if(response.getRequestType() == SET_PARTECIPANTS) {
        QStringList participants = response.getBodyList();
        chat->setParticipants(participants);
        editor->setParticipantAlias(participants);

    } else if(response.getRequestType() == ADDED_PARTECIPANT) {
        QStringList params = response.getBodyList();
        chat->addParticipant(params.at(0), params.at(1), params.at(2));
        editor->addParticipant(params.at(0), params.at(1));

    } else if(response.getRequestType() == REMOVED_PARTECIPANT) {
        QStringList params = response.getBodyList();
        chat->removeParticipant(params.at(0), params.at(1));
        editor->removeParticipant(params.at(0));

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

    if (user != nullptr) delete user;
    user = new KKUser();

    user->setName(params.at(0));
    user->setSurname(params.at(1));
    user->setEmail(params.at(2));
    user->setUsername(params.at(4));
    user->setAlias(params.at(5));
    user->setRegistrationDate(params.at(6));
    user->setImage(params.at(7));

    openFile_.clear();
    openFile_.setUser(user);
    openFile_.setUserFiles(params.mid(8, params.size()));

    access_.hide();
    openFile_.show();
    logger("[handleLoginResponse] - Site id: " + user->getUsername());
}

void KKClient::handleLogoutResponse(KKPayload res) {
    Q_UNUSED(res)
    state_= CONNECTED_NOT_LOGGED;

    currentfileValid_ = false;
    openFile_.hide();

    if (editor != nullptr)
        editor->hide();

    if (chat != nullptr)
        chat->hide();

    access_.showLoader(false);
    access_.showLogin();
    access_.show();
}

void KKClient::handleUpdateUserResponse()
{
    if (user != nullptr) {
        user->setAlias(openFile_.getAlias());
        user->setName(openFile_.getName());
        user->setSurname(openFile_.getSurname());
        user->setImage(openFile_.getAvatar());
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

    state_= CONNECTED_AND_OPENED;
    currentfileValid_ = true;
    initEditor();
    editor->setLink(res.getBodyList()[1]);
    openFile_.hide();
    editor->show();
    chat->show();
}

void KKClient::handleLoadFileResponse(KKPayload res) {
    QStringList bodyList = res.getBodyList();
    if (bodyList.isEmpty())
        return;

    crdt->decodeCrdt(bodyList);
    editor->loadCrdt(crdt->getText(), crdt->getLinesAlignment());
}

void KKClient::handleQuitFileResponse()
{
    if (editor != nullptr)
        editor->hide();

    if (chat != nullptr)
        chat->hide();

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

    unsigned long remotePos = isInsert ? crdt->remoteInsert(char_) : crdt->remoteDelete(char_);
    siteId = isInsert ? siteId : bodyList_[1];
    editor->applyRemoteChanges(bodyList_[0], siteId, text, static_cast<int>(remotePos), char_->getKKCharFont(), char_->getKKCharColor());
    editor->applySiteIdsPositions(siteId, findPositions(siteId));
}

void KKClient::handleAlignmentChange(KKPayload response){

    QStringList bodyList = response.getBodyList();
    QString alignment=bodyList[0];
    QString startAlignLine=bodyList[1];
    QString endAlignLine=bodyList[2];
    int alignPos;

    for(unsigned long i = startAlignLine.toULong(); i <= endAlignLine.toULong(); i++){ //per ogni riga si crea la posizione globale dell'inizio della riga e chiama la alignmentRemoteChange
        if(crdt->checkLine(i) || (crdt->isTextEmpty() && i==0)){ //controlla che la riga esista){ //controlla che la riga esista
            crdt->setLineAlignment(i, alignment.toInt());
            alignPos = static_cast<int>(crdt->calculateGlobalPosition(KKPosition(i,0)));
            editor->applyRemoteAlignmentChange(alignment.toInt(), alignPos);
        } else {
            break;
        }
    }
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
    unsigned long remotePos = crdt->remoteFormatChange(char_,fontStr,colorStr);
    editor->applyRemoteFormatChange(static_cast <int>(remotePos),fontStr,colorStr);
    //editor_->applySiteIdsPositions(siteId, findPositions(siteId));
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
        chat->close();
        editor->close();
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
    bool result = sendRequest(UPDATE_USER, NONE, {user->getUsername(), name, surname, alias, avatar});
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
    QString identifier = (user!=nullptr) ? user->getUsername() : "unknown";
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
        openFile_.setUser(user);

    } else if (modalType == CRDT_ERROR || modalType == CHAT_ERROR || modalType == INPUT_ERROR) {
        modal_.hide();
        editor->close();

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
    crdt->calculateLineCol(position, &line, &col);
    KKCharPtr char_ = crdt->localInsert(value, KKPosition(line, col), font_, color_);
    QString ids = QString::fromStdString(char_->getIdentifiersString());
    sendCrdtRequest({CRDT_INSERT, QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids , font_, color_});
}

void KKClient::onRemoveTextCrdt(unsigned long start, unsigned long end) {
    unsigned long startLine, endLine, startCol, endCol;
    crdt->calculateLineCol(start, &startLine, &startCol);
    crdt->calculateLineCol(end, &endLine, &endCol);
    list<KKCharPtr> deletedChars = crdt->localDelete(KKPosition(startLine, startCol), KKPosition(endLine, endCol));

    QTextCharFormat format = editor->getTextEdit()->textCursor().charFormat();
    QString font_= format.font().toString();
    QString color_= format.foreground().color().name();

    std::for_each(deletedChars.begin(), deletedChars.end(),[&](const KKCharPtr& char_){
        QString ids = QString::fromStdString(char_->getIdentifiersString());
        sendCrdtRequest({CRDT_DELETE, user->getUsername(), QString::fromStdString(char_->getSiteId()), QString(char_->getValue()), ids, font_, color_});
    });
}

void KKClient::onSaveCrdtToFile() {
    if(currentfileValid_) {
        sendRequest(SAVE_FILE, NONE, {crdt->getSiteId(), currentfile_});
    }
}

void KKClient::onOpenFileDialog() {
    sendGetFilesRequest();
}

QSharedPointer<QList<int>> KKClient::findPositions(const QString& siteId){
    QSharedPointer<QList<int>> myList = QSharedPointer<QList<int>>(new QList<int>());
    int global = 0;
    for(const list<KKCharPtr>& linea: crdt->getText()){
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
    editor->applySiteIdClicked(siteId);
}

void KKClient::onUpdateSiteIdsPositions(const QString &siteId)
{
    editor->applySiteIdsPositions(siteId, findPositions(siteId));
}

void KKClient::onAlignmentChange(int alignment, int alignStart, int alignEnd){
    unsigned long startAlignLine, endAlignLine,startAlignCol, endAlignCol; //le colonne non serviranno
    crdt->calculateLineCol(static_cast<unsigned long>(alignStart), &startAlignLine, &startAlignCol);
    crdt->calculateLineCol(static_cast<unsigned long>(alignEnd), &endAlignLine, &endAlignCol);

    for(unsigned long i=startAlignLine;i<=endAlignLine;i++){
        // Aggiorno il crdt con gli allineamenti
        crdt->setLineAlignment(static_cast<unsigned long>(i), alignment);
    }
    sendRequest(ALIGNMENT_CHANGE,NONE,{QString::number(alignment),QString::number(static_cast<int>(startAlignLine)),QString::number(static_cast<int>(endAlignLine))});
}


void KKClient::onCharFormatChanged(unsigned long pos, QString font_, QString color_){
    unsigned long line, col;
    crdt->calculateLineCol(pos, &line, &col);
    KKCharPtr _char = crdt->changeSingleKKCharFormat(KKPosition(line, col), font_, color_);
    if (_char != nullptr) {
        QString ids = QString::fromStdString(_char->getIdentifiersString());
        sendRequest(CHARFORMAT_CHANGE, NONE, {user->getUsername(), QString::fromStdString(_char->getSiteId()), QString(_char->getValue()), ids, font_, color_});
    }
}

void KKClient::notifyAlignment(int alignStart, int alignEnd){ //prende l'allineamento delle righe da start a end dal textedit e manda un messaggio di cambio allineamento per quelle righr
    unsigned long startAlignLine, endAlignLine,startAlignCol, endAlignCol; //le colonne non serviranno
    unsigned long pos;
    int alignment;
    crdt->calculateLineCol(static_cast<unsigned long>(alignStart), &startAlignLine, &startAlignCol);
    crdt->calculateLineCol(static_cast<unsigned long>(alignEnd), &endAlignLine, &endAlignCol);

    for(unsigned long i=startAlignLine;i<=endAlignLine;i++){
        pos = crdt->calculateGlobalPosition(KKPosition(i,0));
        alignment = editor->getCurrentAlignment(static_cast<int>(pos));

        // Controlla che la riga esista
        if(crdt->checkLine(i) || (crdt->isTextEmpty() && i==0)){
            crdt->setLineAlignment(static_cast<unsigned long>(i), alignment);
            sendRequest(ALIGNMENT_CHANGE,NONE,{QString::number(alignment), QString::number(static_cast<int>(i)), QString::number(static_cast<int>(i))});
        }
    }
}
