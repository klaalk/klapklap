
#include "kk_textedit.h"

#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextCharFormat>

KKTextEdit::KKTextEdit(QWidget *parent): QTextEdit(parent)
{
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &KKTextEdit::wheelEventTriggered);
    connect(this, &QTextEdit::textChanged, this, &KKTextEdit::handleTextChange);
}

// Evento che inizia quando premo un tasto
void KKTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->text() == "\u001A") {
        textUndo();
        return;
    }

    if (e->text() == "\u0019") {
        textRedo();
        return;
    }

    if (e->text() == "\u0016") {
        textPaste();
        return;
    }

    if (textCursor().hasSelection()) {
        selectionStart = textCursor().selectionStart();
        selectionEnd = textCursor().selectionEnd();
        wasSelected = true;
    } else {
        wasSelected = false;
    }
    start = textCursor().position();
    lastText = toPlainText();
    textChanged = false;

    QTextEdit::keyPressEvent(e);
    saveLocalCursor();

    if (textChanged) {
        calculateDiffText();
    }
    textChanged = false;
}

void KKTextEdit::mousePressEvent(QMouseEvent *e)
{
    start = textCursor().position();
    lastText = toPlainText();
    QTextEdit::mousePressEvent(e);
    saveLocalCursor();
}

void KKTextEdit::wheelEvent(QWheelEvent *e)
{
    emit wheelEventTriggered();
    QTextEdit::wheelEvent(e);
}

QTextCursor KKTextEdit::getCursor(int position)
{
    int maxPosition = toPlainText().length();
    position = position >= 0 ? position : 0;
    position = position <= maxPosition ? position : maxPosition;
    QTextCursor tmp = textCursor();
    tmp.setPosition(position);
    return tmp;
}

void KKTextEdit::setCursorPosition(int position)
{
    QTextCursor tmp = getCursor(position);
    setTextCursor(tmp);
}

void KKTextEdit::setLocalCursorPosition(int position)
{
    if (!localCursor.hasSelection()) {
        QTextCursor tmp = getCursor(position);
        localCursor = tmp;
    }
    setTextCursor(localCursor);
}

void KKTextEdit::restoreLocalCursor()
{
    setTextCursor(localCursor);
}

int KKTextEdit::cursorPosition()
{
    return textCursor().position();
}

int KKTextEdit::getLocalCursorPosition()
{
    return localCursor.position();
}

void KKTextEdit::saveLocalCursor()
{
    localCursor = textCursor();
}

void KKTextEdit::handleTextChange()
{
    textChanged = true;
    isUndoAvailable = true;
    isRedoAvailable = false;
    emit undoAvailable(isUndoAvailable);
    emit redoAvailable(isRedoAvailable);
}

void KKTextEdit::textUndo() {
    if (isUndoAvailable) {
        // Ho premuto ctrl+z
        // Inizializzo la prima volta all'apertura dell'editor con la posizione finale del testo
        if (lastPos == -1)
            lastPos = toPlainText().length();

        // Vuol dire che devo partire dall'ultima posizione in cui ?? cambiato il testo
        start = lastPos;
        lastText = toPlainText();

        undo();

        isUndoAvailable = false;
        isRedoAvailable = true;

        if (textChanged)
            calculateDiffText();

        emit undoAvailable(isUndoAvailable);
        emit redoAvailable(isRedoAvailable);
    }
}

void KKTextEdit::textRedo()
{
    if (isRedoAvailable) {
        // Vuol dire che devo partire dall'ultima posizione in cui ?? cambiato il testo
        start = lastPos;
        lastText = toPlainText();

        redo();

        isRedoAvailable = false;
        isUndoAvailable = true;

        if (textChanged)
            calculateDiffText();

        emit undoAvailable(isUndoAvailable);
        emit redoAvailable(isRedoAvailable);
    }
}

void KKTextEdit::textCopy()
{
    copy();
}

void KKTextEdit::textPaste()
{
    start = textCursor().position();
    lastText = toPlainText();

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasHtml() && !mimeData->formats().contains("application/vnd.oasis.opendocument.text")) {
        textCursor().insertText(mimeData->text());
    } else if (mimeData->hasText()) {
        paste();
    }

    if (textChanged)
        calculateDiffText();
}

void KKTextEdit::textCut()
{
    start = textCursor().position();
    lastText = toPlainText();

    cut();

    if (textChanged)
        calculateDiffText();
}

void KKTextEdit::calculateDiffText()
{
    // Prelevo la posizione corrente del cursore
    saveLocalCursor();
    end = getLocalCursorPosition();
    // Se il testo era selezionato e ora non lo ?? pi??
    if (wasSelected && !textCursor().hasSelection()) {
        // Allora guardo inizio e fine della slezione facendo in modo che inizio sia sempre "minore" di fine
        if (selectionStart > selectionEnd) {
            int tmp = selectionStart;
            selectionStart = selectionEnd;
            selectionEnd = tmp;
        }
        // Prendo dal testo precedente ci?? che avevo selezionato
        QString lastDiff = lastText.mid(selectionStart, selectionEnd-selectionStart);

        // Prendo dal testo attuale ci?? che c'?? da dove era iniziata la selezione a dove si trova ora il cursore
        QString diff = toPlainText().mid(selectionStart, end - selectionStart);

        if (diff.isEmpty()) {
            // Se ora non c'?? niente vuol dire che ho selezionato e cancellato
            sendDiffText(CRDT_DELETE, lastDiff, selectionStart, selectionEnd);
            emit alignmentNotifyEvent(selectionStart, selectionStart);

        } else {
            // Altrimenti ho cancellato e inserito oppure solo inserito
            if (lastDiff.isEmpty()) {
                // Quindi se la selezione del testo precedente ?? vuoto vuol dire che ho solo inserito (DIFFICILE CADERE IN QUESTA SITUAZIONE)
                sendDiffText(CRDT_INSERT, diff, selectionStart, selectionStart + diff.length());
                emit alignmentNotifyEvent(selectionStart, selectionStart + diff.length());

            } else {
                // Mentre se la selezione del testo precedente non ?? vuoto allora ho cancellato
                sendDiffText(CRDT_DELETE, lastDiff, selectionStart, selectionEnd);

                // E inserito
                sendDiffText(CRDT_INSERT, diff, selectionStart, selectionStart + diff.length());
                emit alignmentNotifyEvent(selectionStart, selectionStart + diff.length());
            }
        }

        // Mi salvo l'ultima posizione del cursore in cui ?? cambiato il testo (serve per il CTRL + Z)
        lastPos = textCursor().position();

    } else {
        // Non ho selezionato nulla ma il cursore ha cambiato posizione da quando ho premuto un tasto (o una combinazione di tasti)
        // a quando ho rilasciato i tasti
        // Vuol dire che il testo ?? cambiato
        if (start != end) {
            if (start > end) {
                // Se prima il cursore (start) era pi?? avanti di ora (end) allora ho cancellato (in INDIETRO) da end a start
                QString lastDiff = lastText.mid(end, start - end);
                sendDiffText(CRDT_DELETE, lastDiff, end, start);

            } else {
                // Altrimenti ho inserito oppure cancellato in AVANTI
                QString diff = toPlainText().mid(start, end-start);
                if (diff.isEmpty()) {
                    // Se il testo attuale ?? vuoto da start a end allora ho cancellato in AVANTI
                    QString lastDiff = lastText.mid(start, end - start);
                    sendDiffText(CRDT_DELETE, lastDiff, end, start);

                    if(lastDiff.length()>1)
                        emit alignmentNotifyEvent(end, end);

                } else {
                    // Altrimenti ho inserito in AVANTI
                    sendDiffText(CRDT_INSERT, diff, start, end);

                    if(diff.length()>1)
                       emit alignmentNotifyEvent(start, end);

                }
            }

            // Mi salvo l'ultima posizione del cursore in cui ?? cambiato il testo (serve per il CTRL + Z)
            lastPos = textCursor().position();

        } else if (start == end && start >= 0 && end >= 0) {
            // Caso in cui ho premuto CANC e quindi cancellato in avanti
            int length = toPlainText().length() - lastText.length();
            if (length < 0) {
                length = -length;
                QString lastDiff = lastText.mid(start, length);
                sendDiffText(CRDT_DELETE, lastDiff, start, start + length);
                // Mi salvo l'ultima posizione del cursore in cui ?? cambiato il testo (serve per il CTRL + Z)
                lastPos = textCursor().position();
            }
        }
    }
}

void KKTextEdit::sendDiffText(QString operation, QString text, int start, int end)
{
    QString printText = text;
    if (printText.size() > 50) {
        printText.truncate(50);
        printText.append("[...]");
    }
    KKLogger::log(QString("[sendDiffText] - %1 >%2< from >%3< to >%4<").arg(operation, printText, QString::number(start), QString::number(end)), "TEXTEDIT");

    emit textChangedEvent(operation, text, start, end);
}

bool KKTextEdit::getIsRedoAvailable() const
{
    return isRedoAvailable;
}

bool KKTextEdit::getIsUndoAvailable() const
{
    return isUndoAvailable;
}

