//
// Created by Klaus on 06/05/2019.
//

#include "kk_client.h"

#include <utility>

KKClient::KKClient(QUrl url, QObject *parent)
    : QObject(parent), url(std::move(url)) {

    // Gestisco l' apertura della connessione al socket
    connect(&socket, &QWebSocket::connected, this, &KKClient::handleOpenedConnection);

    // Gestisco la lettura dei messaggi.
    connect(&socket, &QWebSocket::textMessageReceived, this, &KKClient::handleResponse);

    // Gestisco eventuali errorri sulla connessione al socket
    connect(&socket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this, &KKClient::handleSslErrors);

    // Gestisco le due chiusuere della modale
    connect(&modal, &KKModal::modalButtonClicked, this, &KKClient::handleModalButtonClick);
    connect(&modal, &KKModal::modalClosed, this, &KKClient::handleModalClosed);

    // Gestisco le richieste di login o di registrazione o logout
    connect(&access, &AccessDialog::loginBtnClicked, this, &KKClient::sendLoginRequest);
    connect(&access, &AccessDialog::signupBtnClicked, this, &KKClient::sendSignupRequest);

    // Gestisco le richieste di apertura file
    connect(&openFile, &OpenFileDialog::openFile, this, &KKClient::sendOpenFileRequest);
    connect(&openFile, &OpenFileDialog::deleteFile, this, &KKClient::sendDeleteFileRequest);
    connect(&openFile, &OpenFileDialog::updateAccount, this, &KKClient::sendUpdateUserRequest);
    connect(&openFile, &OpenFileDialog::logout, this, &KKClient::sendLogoutRequest);
    connect(&openFile, &OpenFileDialog::closed, this, &KKClient::onOpenFileDialogClosed);

    // Gestisco il timeout
    connect(&timer, &QTimer::timeout, this, &KKClient::handleTimeOutConnection);

    QDirIterator it(":/images/avatars", QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        avatars.push_back(it.next());
    }

    initState();
}

/// INIT

void KKClient::initState() {
    state = NOT_CONNECTED;
    timer.start(TIMEOUT_VALUE);
    socket.open(QUrl(url));
    openFile.hide();
    modal.hide();
    if (editor != nullptr)
        editor->hide();
    access.show();
    access.showLoader(true);
}

void KKClient::initEditor()
{
    logger("Inizializzazione editor...");
    if (crdt != nullptr) delete crdt;
    crdt = new KKCrdt(user->getUsername(), casuale);

    if (chat != nullptr) delete chat;
    chat = new KKChat();

    if (editor != nullptr) delete editor;
    editor = new KKEditor();


    // Gestisco le richieste dell'editor
    connect(editor, &KKEditor::insertTextToCrdt, this, &KKClient::onInsertTextToCrdt);
    connect(editor, &KKEditor::removeTextFromCrdt, this, &KKClient::onRemoveTextFromCrdt);
    connect(editor, &KKEditor::saveCrdtTtoFile, this, &KKClient::onSaveCrdtToFile);
    connect(editor, &KKEditor::alignChange, this, &KKClient::onAlignmentChange);
    connect(editor, &KKEditor::notifyAlignment, this, &KKClient::onNotifyAlignment);
    connect(editor, &KKEditor::charFormatChange, this, &KKClient::onCharFormatChanged);
    connect(editor, &KKEditor::openFileDialog, this, &KKClient::onOpenFileDialogOpened);
    connect(editor, &KKEditor::editorClosed, this, &KKClient::onEditorClosed);
    connect(editor, &KKEditor::printCrdt, this, &KKClient::printCrdt);
    editor->setChatDialog(chat);
    editor->setMySiteId(user->getUsername());
    editor->setCurrentFileName(filename);
    editor->hide();

    // Gestisco le richieste della chat
    connect(chat, &KKChat::sendMessageEvent, this, &KKClient::sendMessageRequest);
    connect(chat, &KKChat::siteIdClicked, this, &KKClient::onSiteIdClicked);

    chat->setNickName(user->getAlias());
    chat->hide();

    logger("Inizializzazione editor completata");
}

/// HANDLING

void KKClient::handleOpenedConnection() {
    logger("[handleOpenedConnection] - Websocket connesso");
    state = CONNECTED;
    timer.stop();
    access.showLoader(false);
}

void KKClient::handleTimeOutConnection() {
    logger("[handleTimeOutConnection] - Websocket time out connection");
    timer.stop();
    modal.setModal(MODAL_TIMEOUT, BUTTON_RETRY_ACTION, CONNECTION_TIMEOUT);
    modal.show();
    socket.close();
}

void KKClient::handleErrorConnection(QAbstractSocket::SocketError error) {
    logger(&"[handleErrorConnection] - Websocket not connected: " [error]);
    socket.close();
}

void KKClient::handleSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors)
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    socket.ignoreSslErrors();
}

void KKClient::handleResponse(const QString& message) {
    timer.stop();

    logger(QString("[handleResponse] - %1").arg(message));

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

    } else if(response.getRequestType() == DELETE_FILE) {
        sendGetFilesRequest();


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
        chat->removeParticipant(params.at(0), params.at(1), params.at(2));
        editor->removeParticipant(params.at(0));

    } else {
        modal.setModal(MODAL_SUCCESS, BUTTON_CLOSE_ACTION, GENERIC_SUCCESS);
        modal.show();
    }
}

void KKClient::handleErrorResponse(KKPayload response){
    if (response.getResultType() == BAD_REQUEST) {
        handleClientErrorResponse(response);
    } else if (response.getResultType() == INTERNAL_SERVER_ERROR) {
        handleServerErrorResponse(response);
    } else {
        modal.setModal(MODAL_GENERIC_ERROR, BUTTON_CLOSE_ACTION, GENERIC_ERROR);
        modal.show();
    }
}

void KKClient::handleClientErrorResponse(KKPayload response) {
    QString message, button, modalType;

    if (state == CONNECTED_NOT_LOGGED) {
        message = MODAL_NOT_LOGGED ;
        button = BUTTON_CLOSE_ACTION;
        modalType = LOGIN_ERROR;

    } else if (state == CONNECTED_NOT_SIGNED) {
        message = MODAL_NOT_SIGNED;
        button = BUTTON_RETRY_ACTION;
        modalType = SIGNUP_ERROR;

    } else if (state == CONNECTED_NOT_OPENFILE) {
        message = MODAL_NOT_OPENFILE;
        button = BUTTON_CLOSE_ACTION;
        modalType = OPENFILE_ERROR;

    } else if (state == CONNECTED_AND_OPENED) {
        message = MODAL_OPENED_FILE;
        button = BUTTON_CLOSE_ACTION;
        modalType = CRDT_ERROR;

    } else if (response.getRequestType() == DELETE_FILE) {
        message = MODAL_DELETE_FILE_ERROR;
        button = BUTTON_CLOSE_ACTION;
        modalType = FILE_ERROR;
    } else {
        message = MODAL_LOGIN_ERROR;
        button = BUTTON_CLOSE_ACTION;
        modalType = GENERIC_ERROR;
    }
    if (modalType == OPENFILE_ERROR) {
        QString remoteMessage = response.getBodyList().at(0);
        message.append("\n").append(remoteMessage);
    }
    if (modalType == FILE_ERROR) {
        QString remoteMessage = response.getBodyList().at(0);
        message = remoteMessage;
    }
    modal.setModal(message, button, modalType);
    modal.show();
}

void KKClient::handleServerErrorResponse(KKPayload res) {
    Q_UNUSED(res)

    QString message, button, modalType;
    if (res.getRequestType() == UPDATE_USER) {
        message = MODAL_UPDATE_USER;
        button = BUTTON_CLOSE_ACTION;
        modalType = UPDATE_USER_ERROR;

    } else if (res.getRequestType() == QUIT_FILE) {
        message = MODAL_QUIT_FILE;
        button = BUTTON_CLOSE_ACTION;
        modalType = INPUT_ERROR;
    } else {
        message = MODAL_GENERIC_ERROR;
        button = BUTTON_RETRY_ACTION;
        modalType = SERVER_ERROR;
    }

    modal.setModal(message, button, modalType);
    modal.show();
}


void KKClient::handleLoginResponse(KKPayload res) {
    state= CONNECTED;

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

    openFile.clear();
    openFile.setUser(user);
    openFile.setUserFiles(params.mid(8, params.size()));

    access.hide();
    openFile.show();
}

void KKClient::handleLogoutResponse(KKPayload res) {
    Q_UNUSED(res)
    state= CONNECTED_NOT_LOGGED;

    fileValid = false;
    openFile.hide();

    if (editor != nullptr)
        editor->hide();

    if (chat != nullptr)
        chat->hide();

    access.showLoader(false);
    access.showLogin();
    access.show();
}

void KKClient::handleUpdateUserResponse()
{
    if (user != nullptr) {
        user->setAlias(openFile.getAlias());
        user->setName(openFile.getName());
        user->setSurname(openFile.getSurname());
        user->setImage(openFile.getAvatar());
    }
    modal.setModal(MODAL_UPDATE_USER_INFO, BUTTON_CLOSE_ACTION, GENERIC_SUCCESS);
    modal.show();
}


void KKClient::handleSignupResponse() {
    state = CONNECTED;
    access.showLoader(false);
    access.showLogin();
}

void KKClient::handleGetFilesResponse(KKPayload res)
{
    openFile.clear();
    openFile.setUserFiles(res.getBodyList());
    openFile.show();
}

void KKClient::handleOpenFileResponse(KKPayload res) {
    state = CONNECTED_AND_OPENED;
    fileValid = true;
    editor->setLink(res.getBodyList()[1]);
}

void KKClient::handleLoadFileResponse(KKPayload res) {
    QStringList bodyList = res.getBodyList();
    if (bodyList.isEmpty())
        return;
    fileValid = true;
    crdt->decodeCrdt(bodyList);
    editor->load(crdt->getText(), crdt->getLinesAlignment());
    editor->loading(false);
    editor->clearUndoRedoStack();
    editor->updateLabels();
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
    // Se il file non è valido non faccio nessuna modifica
    if (!fileValid) return;

    // Ottengo i campi della risposta
    QStringList body = response.getBodyList();
    // Recupero le info generiche dell'operazione
    QString operation = body.takeFirst();
    QString remoteSiteId = body.takeFirst();
    body.takeFirst();

    if (operation == CRDT_ALIGNM) {
        handleCrdtAlignmentResponse(remoteSiteId, body);
    } else {
        handleCrdtTextResponse(remoteSiteId, operation, body);
    }

    if (user->getUsername() != remoteSiteId) {
        // Pulisco lo stack delle operazioni
        editor->clearUndoRedoStack();
    }
    editor->updateLabels();
}

void KKClient::handleCrdtAlignmentResponse(QString remoteSiteId, QStringList ranges)
{
    // Caso in cui sia una operazione di allineamento
    int currentPosition = -1;
    while(!ranges.isEmpty()) {
        // Recupero le info dell'allineamento
        int alignment = ranges.takeFirst().toInt();
        unsigned long startLine = ranges.takeFirst().toULong();
        unsigned long endLine = ranges.takeFirst().toULong();

        // Per ogni riga si crea la posizione globale dell'inizio della riga e chiama la alignmentRemoteChange
        for(unsigned long i = startLine; i <= endLine; i++) {
            // Controlla che la riga esista
            if (crdt->remoteAlignmentChange(i, alignment)) {
                currentPosition = crdt->calculateGlobalPosition(KKPosition(i, 0));
                editor->applyRemoteAlignmentChange(alignment, currentPosition);
            }
        }
    }

    // Aggiorno la posizione del cursore remoto
    if (currentPosition >= 0)
        editor->applyRemoteCursorChange(remoteSiteId, currentPosition);

    // Semplicemente riposiziono il cursore dov'era
    editor->updateLocalCursor();
}

void KKClient::handleCrdtTextResponse(QString remoteSiteId, QString operation, QStringList chars)
{
    // Posizione corrente
    int currentPosition = -1;
    // Memorizza di quanti caratteri è cambiato il testo
    int operationCounter = 0;

    // Ciclo carattere per carattere
    KKPosition crdtPosition(0, 0);

    while (!chars.isEmpty()) {

        QString crdtChar = operation == CRDT_DELETE ? chars.takeLast() : chars.takeFirst();
        KKCharPtr charPtr = crdt->decodeCrdtChar(crdtChar);

        if (operation == CRDT_FORMAT) {
            crdtPosition = crdt->remoteFormatChange(charPtr);
        }
        else if (operation == CRDT_INSERT) {
            crdtPosition = crdt->remoteInsert(charPtr);
            operationCounter++;
        }
        else if (operation == CRDT_DELETE) {
            crdtPosition = crdt->remoteDelete(charPtr);
            operationCounter--;
        }

        // Calcolo la global position
        currentPosition = crdt->calculateGlobalPosition(crdtPosition);
        editor->applyRemoteTextChange(operation, currentPosition, remoteSiteId, charPtr->getValue(), charPtr->getKKCharFont(), charPtr->getKKCharColor());
    }

    // Aggiorno la posizione del cursore remoto
    if (currentPosition >= 0) {
        if (operation == CRDT_INSERT) currentPosition += 1;
        editor->applyRemoteCursorChange(remoteSiteId, currentPosition);
    }

    if (operationCounter != 0) {
        // Shifto la posizione di tutti gli altri utenti
        int startPosition = operation == CRDT_INSERT ? currentPosition - operationCounter : currentPosition;
        editor->updateCursors(remoteSiteId, startPosition, operationCounter);

        // Shifto la posizione del cursore locale se necessario
        editor->updateLocalCursor(startPosition, operationCounter);
    }
}

/// SENDING

void KKClient::sendLoginRequest(QString email, const QString& password) {
    KKCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));

    QString psw = solver.encryptToString(password);
    access.showLoader(true);

    if (!timer.isActive())
        timer.start(TIMEOUT_VALUE);

    bool sended = sendRequest(LOGIN, NONE, {std::move(email), psw});

    if (sended) {
        state = CONNECTED_NOT_LOGGED;
    }
}

void KKClient::sendGetFilesRequest()
{
    if (!timer.isActive())
        timer.start(TIMEOUT_VALUE);

    bool sended = sendRequest(GET_FILES, NONE, PAYLOAD_EMPTY_BODY);
    if (sended) {
        state = CONNECTED_AND_LOGGED;
    }
}

void KKClient::sendSignupRequest(QString email, const QString& password, QString name, QString surname, QString username) {
    KKCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    access.showLoader(true);
    if (!timer.isActive())
        timer.start(TIMEOUT_VALUE);
    QString avatar = avatars.at(qrand() % (avatars.size()-1)).split(":/images/avatars/")[1];
    bool sended = sendRequest(SIGNUP, NONE, {email, psw, name, surname, username, avatar});

    if (sended) {
        state = CONNECTED_NOT_SIGNED;
    }
}

void KKClient::sendLogoutRequest() {
    if (!timer.isActive())
        timer.start(TIMEOUT_VALUE);

    bool sended = sendRequest(LOGOUT, NONE, PAYLOAD_EMPTY_BODY);
    if (sended) {
        state = CONNECTED_NOT_LOGGED;
    }
}

void KKClient::sendOpenFileRequest(const QString& hashname_, const QString& filename_) {
    fileValid = false;
    filename = filename_;
    hashname = hashname_;

    initEditor();
    openFile.hide();
    editor->loading(true);
    editor->show();
    chat->show();

    if (state == CONNECTED_AND_OPENED) {
        chat->close();
        editor->close();
    }

    if (!timer.isActive())
        timer.start(TIMEOUT_VALUE);
    bool sended = sendRequest(OPEN_FILE, NONE, {hashname, filename});
    if (sended) {
        state = CONNECTED_NOT_OPENFILE;
    }
}

void KKClient::sendDeleteFileRequest(const QString &hashname)
{
    bool result = sendRequest(DELETE_FILE, NONE, {hashname});
    if (!result || !socket.isValid()) {
        modal.setModal(MODAL_DELETE_FILE_ERROR, BUTTON_CLOSE_ACTION, FILE_ERROR);
        modal.show();
    }
}

void KKClient::sendLoadFileRequest(const QString &hashname)
{
    bool result = sendRequest(LOAD_FILE, NONE, {hashname});
    if (!result || !socket.isValid()) {
        modal.setModal("Attenzione!\nSembra che tu non sia connesso alla rete", BUTTON_RETRY_ACTION, CRDT_ERROR);
        modal.show();
    }
}

void KKClient::sendMessageRequest(QString username, QString message) {
    bool result = sendRequest(CHAT, SUCCESS, {std::move(username), std::move(message)});
    if (!result || !socket.isValid()) {
        modal.setModal(MODAL_NETWORK_ERROR, BUTTON_RETRY_ACTION, CHAT_ERROR);
        modal.show();
    }
}

void KKClient::sendUpdateUserRequest(QString name, QString surname, QString alias, QString avatar)
{
    bool result = sendRequest(UPDATE_USER, NONE, {user->getUsername(), name, surname, alias, avatar});
    if (!result || !socket.isValid()) {
        modal.setModal(MODAL_UPDATE_USER_FAIL, BUTTON_CLOSE_ACTION, GENERIC_ERROR);
        modal.show();
    }
}

void KKClient::onEditorClosed()
{
    sendQuitFileRequest();
}

void KKClient::sendQuitFileRequest() {
    bool result = sendRequest(QUIT_FILE, NONE, {});
    if (!result || !socket.isValid()) {
        modal.setModal(MODAL_QUIT_FILE_ERROR, BUTTON_CLOSE_ACTION, GENERIC_ERROR);
        modal.show();
    }
}

void KKClient::sendCrdtRequest(QStringList crdt) {
    bool result = sendRequest(CRDT, SUCCESS, std::move(crdt));
    if (!result || !socket.isValid()) {
        modal.setModal(MODAL_UPDATE_FILE_ERROR, BUTTON_RETRY_ACTION, CRDT_ERROR);
        modal.show();
    }
}

bool KKClient::sendRequest(QString type, QString result, QStringList values) {
    KKPayload req(std::move(type), std::move(result), std::move(values));

    QString message = req.encode();
    logger(QString("[sendRequest] - %1").arg(message));

    qint64 size = socket.sendTextMessage(req.getData());
    return size >= req.getTotalLength();
}

void KKClient::logger(QString message)
{
    QString identifier = (user!=nullptr) ? user->getUsername() : "unknown";
    KKLogger::log(message, QString("CLIENT - %1").arg(identifier));
}

void KKClient::printCrdt()
{
    if (crdt != nullptr) {
        crdt->printText();
    }
}

/// MODAL ACTIONS

void KKClient::handleModalButtonClick(const QString& btnText, const QString& modalType) {
    handleModalActions(modalType, btnText == BUTTON_CLOSE_ACTION);
}

void KKClient::handleModalClosed(const QString& modalType) {
    handleModalActions(modalType, true);
}

void KKClient::handleModalActions(const QString &modalType, bool closed)
{
    Q_UNUSED(closed)
    if (modalType == CONNECTION_TIMEOUT) {
        initState();
    } else  if (modalType == LOGIN_ERROR) {
        modal.hide();
        access.showLoader(false);

    } else if (modalType == UPDATE_USER_ERROR) {
        openFile.setUser(user);

    } else if (modalType == CRDT_ERROR
               || modalType == CHAT_ERROR
               || modalType == INPUT_ERROR) {
        modal.hide();
        sendQuitFileRequest();

    } else if (modalType == CRDT_ILLEGAL) {
        modal.hide();
        sendLoadFileRequest(hashname);

    } else if (modalType == OPENFILE_ERROR) {
        modal.hide();
        sendQuitFileRequest();

    } else if (modalType == GENERIC_SUCCESS) {
        modal.hide();

    } else if (modalType == FILE_ERROR) {
        modal.hide();

    }  else if (modalType == GENERIC_ERROR) {
        initState();

    } else if (modalType == SERVER_ERROR) {
        initState();
    }
}

/// CRDT ACTIONS

void KKClient::onInsertTextToCrdt(unsigned long start, QList<QChar> values, QStringList fonts, QStringList colors) {
    QStringList changes;

    unsigned long line, col;
    bool success = crdt->calculateLineCol(start, 0, &line, &col);
    bool correctInsert = true;

    if (success && crdt->checkPosition(line, col)) {
        changes.push_back(CRDT_INSERT);
        changes.push_back(user->getUsername());
        changes.push_back(QVariant(editor->getTextEdit()->cursorPosition()).toString());
        int i = 0;
        for(QChar value : values) {
            KKCharPtr charPtr = crdt->localInsert(value, KKPosition(line, col), fonts.at(i), colors.at(i));
            if (charPtr->getValue() == value) {
                changes.push_back(crdt->encodeCrdtChar(charPtr));
                if (value != '\n') {
                    col++;
                } else {
                    line++;
                    col=0;
                }
            } else {
                correctInsert = false;
                break;
            }
            i++;
        }
    }
    if (!changes.isEmpty() && correctInsert) {
        sendCrdtRequest(changes);
    } else {
        logger("Inserimento illegale per il CRDT");
        fileValid = false;
        modal.setModal(MODAL_CRDT_ERROR, BUTTON_CONTINUE_ACTION, CRDT_ILLEGAL);
        modal.show();
    }
}

void KKClient::onRemoveTextFromCrdt(unsigned long start, unsigned long end, QString value) {
    unsigned long startLine = 0, endLine = 0, startCol = 0, endCol = 0;
    list<KKCharPtr> deletedChars;
    QStringList changes;
    QString deletedValue;

    bool success = crdt->calculateLineCol(start, 0, &startLine, &startCol)
            && crdt->calculateLineCol(end, 0, &endLine, &endCol);

    if (success) {
        deletedChars = crdt->localDelete(KKPosition(startLine, startCol), KKPosition(endLine, endCol));
        if (deletedChars.size() > 0) {
            changes.push_back(CRDT_DELETE);
            changes.push_back(user->getUsername());
            changes.push_back(QVariant(editor->getTextEdit()->cursorPosition()).toString());

            for (KKCharPtr charPtr : deletedChars) {
                deletedValue.append(charPtr->getValue());
                changes.push_back(crdt->encodeCrdtChar(charPtr));
            }
        }
    }

    if (!changes.isEmpty() && value == deletedValue) {
        sendCrdtRequest(changes);
    } else {
        logger("Cancellazione illegale per il CRDT");
        fileValid = false;
        modal.setModal(MODAL_CRDT_ERROR, BUTTON_CONTINUE_ACTION, CRDT_ILLEGAL);
        modal.show();
    }
}


void KKClient::onCharFormatChanged(unsigned long start, unsigned long end, QString font, QString color){
    unsigned long line = 0, col = 0;
    QStringList changes;

    bool success = crdt->calculateLineCol(start, 0, &line, &col);
    if (success && crdt->checkPosition(line, col)) {
        changes.push_back(CRDT_FORMAT);
        changes.push_back(user->getUsername());
        changes.push_back(QVariant(editor->getTextEdit()->cursorPosition()).toString());

        for (unsigned long i = start; i < end; i++) {
            QChar value;
            KKCharPtr charPtr = crdt->localFormatChange(KKPosition(line, col), font, color, &value);
            if (charPtr != nullptr)
                changes.push_back(crdt->encodeCrdtChar(charPtr));
            if (value != '\n') {
                col++;
            } else {
                line++;
                col = 0;
            }
        }
    }

    if (!changes.isEmpty()) {
        sendCrdtRequest(changes);
    } else {
        logger("Cambio formato illegale per il CRDT");
        fileValid = false;
        modal.setModal(MODAL_CRDT_ERROR, BUTTON_CONTINUE_ACTION, CRDT_ILLEGAL);
        modal.show();
    }
}

void KKClient::onAlignmentChange(int alignment, int alignStart, int alignEnd){
    unsigned long startAlignLine = 0, endAlignLine = 0, startAlignCol = 0, endAlignCol = 0;
    bool success = crdt->calculateLineCol(static_cast<unsigned long>(alignStart), 0, &startAlignLine, &startAlignCol)
            && crdt->calculateLineCol(static_cast<unsigned long>(alignEnd), 0, &endAlignLine, &endAlignCol);

    QStringList changes;
    if (success && crdt->checkPosition(startAlignLine, startAlignCol)
            && crdt->checkPosition(endAlignLine, endAlignCol)) {
        changes.push_back(CRDT_ALIGNM);
        changes.push_back(user->getUsername());
        changes.push_back(QVariant(editor->getTextEdit()->cursorPosition()).toString());
        changes.push_back(QVariant(alignment).toString());
        changes.push_back(QVariant(static_cast<int>(startAlignLine)).toString());
        changes.push_back(QVariant(static_cast<int>(endAlignLine)).toString());
    }

    if (!changes.isEmpty()) {
        sendCrdtRequest(changes);
    } else {
        logger("Cambio allineamento illegale per il CRDT");
        fileValid = false;
        modal.setModal(MODAL_CRDT_ERROR, BUTTON_CONTINUE_ACTION, CRDT_ILLEGAL);
        modal.show();
    }
}

/// Prende l'allineamento delle righe da start a end dal textedit e manda un messaggio di cambio allineamento per quelle righe
void KKClient::onNotifyAlignment(int alignStart, int alignEnd){
    unsigned long startAlignLine = 0, endAlignLine = 0, startAlignCol = 0, endAlignCol = 0;

    bool success = crdt->calculateLineCol(static_cast<unsigned long>(alignStart), 0, &startAlignLine, &startAlignCol)
            && crdt->calculateLineCol(static_cast<unsigned long>(alignEnd), 0, &endAlignLine, &endAlignCol);

    QStringList changes;
    if (success && crdt->checkPosition(startAlignLine, startAlignCol) && crdt->checkPosition(endAlignLine, endAlignCol)) {

        int prevAlignment = -1;

        for(unsigned long i = startAlignLine; i <= endAlignLine; i++) {

            if (crdt->checkLine(i) || (crdt->isTextEmpty() && i==0)) {
                int pos = crdt->calculateGlobalPosition(KKPosition(i, 0));
                int alignment = editor->getCurrentAlignment(pos);

                if (prevAlignment != alignment) {
                    // Controlla che la riga esista
                    changes.push_back(QVariant(alignment).toString());
                    changes.push_back(QVariant(static_cast<int>(i)).toString());
                    changes.push_back(QVariant(static_cast<int>(i)).toString());
                } else {
                    changes.pop_back();
                    changes.push_back(QVariant(static_cast<int>(i)).toString());
                }

                prevAlignment = alignment;
            }
        }
        if (!changes.isEmpty()) {
            changes.push_front(QVariant(editor->getTextEdit()->cursorPosition()).toString());
            changes.push_front(user->getUsername());
            changes.push_front(CRDT_ALIGNM);
        }
    }
    if (!changes.isEmpty()) {
        sendCrdtRequest(changes);
    } else {
        logger("Modifica allineamento illegale per il CRDT");
        fileValid = false;
        modal.setModal(MODAL_CRDT_ERROR, BUTTON_CONTINUE_ACTION, CRDT_ILLEGAL);
        modal.show();
    }
}


void KKClient::onSaveCrdtToFile() {
    if (fileValid) {
        sendRequest(SAVE_FILE, NONE, {crdt->getSiteId(), filename});
    }
}

void KKClient::onOpenFileDialogOpened() {
    sendGetFilesRequest();
}

void KKClient::onOpenFileDialogClosed()
{
    if (editor != nullptr && editor->isVisible())
        openFile.hide();
    else {
        openFile.hide();
        access.show();
        access.showLoader(true);
        sendLogoutRequest();
    }
}

QSharedPointer<QList<int>> KKClient::findPositions(const QString& siteId){
    QSharedPointer<QList<int>> myList = QSharedPointer<QList<int>>(new QList<int>());
    int global = 0;
    for(const list<KKCharPtr>& linea: crdt->getText()){
        for(const KKCharPtr& carattere: linea) {
            if (carattere->getSiteId() == siteId) {
                myList->push_back(global);
            }
            global++;
        }
    }
    return myList;
}

void KKClient::onSiteIdClicked(const QString& siteId)
{
    editor->applySiteIdsPositions(siteId, findPositions(siteId));
    QBrush color = editor->applySiteIdClicked(siteId);
    chat->setParticipantChatBackgroundColor(color, siteId);
}
