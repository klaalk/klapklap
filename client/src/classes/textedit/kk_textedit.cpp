
#include "kk_textedit.h"

#include <QScrollBar>

KKTextEdit::KKTextEdit(QWidget *parent): QTextEdit(parent)
{
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &KKTextEdit::wheelEventTriggered);
    connect(this, &QTextEdit::textChanged, this, &KKTextEdit::handleTextChange);
}

// Evento che inizia quando premo un tasto
void KKTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (cursorCounter > 0)
        restoreCursorPosition();

    if (e->text() == "\u001A") {
        textUndo();
        QTextEdit::keyPressEvent(e);
        return;
    }

    if (e->text() == "\u0019") {
        textRedo();
        QTextEdit::keyPressEvent(e);
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

    if (textChanged) {
        sendDiffText();
    }

    textChanged = false;

}

void KKTextEdit::mousePressEvent(QMouseEvent *e)
{
    start = textCursor().position();
    lastText = toPlainText();
    QTextEdit::mousePressEvent(e);
}

void KKTextEdit::wheelEvent(QWheelEvent *e)
{
    emit wheelEventTriggered();
    QTextEdit::wheelEvent(e);
}

void KKTextEdit::lockCursor()
{
    if (cursorCounter == 0) {
        localCursorPosition = textCursor().position();
    }
    cursorCounter++;
}

void KKTextEdit::unlockCursor()
{
    cursorCounter--;
    if (cursorCounter == 0)
        restoreCursorPosition();
}

QTextCursor KKTextEdit::cursorIn(int position)
{
    QTextCursor tmp = textCursor();
    tmp.setPosition(position);
    return tmp;
}

void KKTextEdit::setCursorPosition(int position)
{
    QTextCursor tmp = textCursor();
    tmp.setPosition(position);
    setTextCursor(tmp);
}

void KKTextEdit::restoreCursorPosition()
{
    QTextCursor tmp = textCursor();
    tmp.setPosition(localCursorPosition);
    setTextCursor(tmp);
}

int KKTextEdit::cursorPosition()
{
    return textCursor().position();
}

int KKTextEdit::getLocalCursorPosition()
{
    return localCursorPosition;
}

void KKTextEdit::handleTextChange()
{
    textChanged = true;
}

void KKTextEdit::textUndo() {
    // Ho premuto ctrl+z
    // Inizializzo la prima volta all'apertura dell'editor con la posizione finale del testo
    if (lastPos == -1)
        lastPos = toPlainText().length();

    // Vuol dire che devo partire dall'ultima posizione in cui è cambiato il testo
    start = lastPos;
    lastText = toPlainText();

    undo();

    if (textChanged)
        sendDiffText();
}

void KKTextEdit::textRedo()
{
    // Vuol dire che devo partire dall'ultima posizione in cui è cambiato il testo
    start = lastPos;
    lastText = toPlainText();

    redo();

    if (textChanged)
        sendDiffText();
}

void KKTextEdit::textCopy()
{
    if (cursorCounter > 0)
        restoreCursorPosition();

    copy();
}

void KKTextEdit::textPaste()
{
    if (cursorCounter > 0)
        restoreCursorPosition();

    start = textCursor().position();
    lastText = toPlainText();

    paste();

    if (textChanged)
        sendDiffText();
}

void KKTextEdit::textCut()
{
    if (cursorCounter > 0)
        restoreCursorPosition();

    start = textCursor().position();
    lastText = toPlainText();

    cut();

    if (textChanged)
        sendDiffText();
}

void KKTextEdit::sendDiffText()
{
    // Prelevo la posizione corrente del cursore
    end = textCursor().position();
    localCursorPosition = end;
    // Se il testo era selezionato e ora non lo è più
    if (wasSelected && !textCursor().hasSelection()) {
        // Allora guardo inizio e fine della slezione facendo in modo che inizio sia sempre "minore" di fine
        if (selectionStart > selectionEnd) {
            int tmp = selectionStart;
            selectionStart = selectionEnd;
            selectionEnd = tmp;
        }
        // Prendo dal testo precedente ciò che avevo selezionato
        QString lastDiff = lastText.mid(selectionStart, selectionEnd-selectionStart);

        // Prendo dal testo attuale ciò che c'è da dove era iniziata la selezione a dove si trova ora il cursore
        QString diff = toPlainText().mid(selectionStart, end - selectionStart);

        if (diff.isEmpty()) {
            // Se ora non c'è niente vuol dire che ho selezionato e cancellato
            qDebug() << "DELETE: "<< lastDiff << " - START " << selectionStart << " END " << selectionEnd;

            emit textChangedEvent(CRDT_DELETE, lastDiff, selectionStart, selectionEnd);
            emit alignmentNotifyEvent(selectionStart, selectionStart);

        } else {
            // Altrimenti ho cancellato e inserito oppure solo inserito
            if (lastDiff.isEmpty()) {
                // Quindi se la selezione del testo precedente è vuoto vuol dire che ho solo inserito (DIFFICILE CADERE IN QUESTA SITUAZIONE)
                qDebug() << "INSERT: " << diff << " START " << selectionStart << " END " << selectionStart + diff.length();
                emit textChangedEvent(CRDT_INSERT, diff, selectionStart, selectionStart + diff.length());
                emit alignmentNotifyEvent(selectionStart, selectionStart + diff.length());

            } else {
                // Mentre se la selezione del testo precedente non è vuoto allora ho cancellato
                qDebug() << "DELETE: "<< lastDiff << " - START " << selectionStart << " END " << selectionEnd;
                emit textChangedEvent(CRDT_DELETE, lastDiff, selectionStart, selectionEnd);

                // E inserito
                qDebug() << "INSERT: " << diff << " - START " << selectionStart << " END " << selectionStart + diff.length();
                emit textChangedEvent(CRDT_INSERT, diff, selectionStart, selectionStart + diff.length());
                emit alignmentNotifyEvent(selectionStart, selectionStart + diff.length());
            }
        }

        // Mi salvo l'ultima posizione del cursore in cui è cambiato il testo (serve per il CTRL + Z)
        lastPos = textCursor().position();

    } else {
        // Non ho selezionato nulla ma il cursore ha cambiato posizione da quando ho premuto un tasto (o una combinazione di tasti)
        // a quando ho rilasciato i tasti
        // Vuol dire che il testo è cambiato
        if (start != end) {
            if (start > end) {
                // Se prima il cursore (start) era più avanti di ora (end) allora ho cancellato (in INDIETRO) da end a start
                QString lastDiff = lastText.mid(end, start - end);
                qDebug() << "DELETE: " << lastDiff << " - START " << end << " END " << start;
                emit textChangedEvent(CRDT_DELETE, lastDiff, end, start);

            } else {
                // Altrimenti ho inserito oppure cancellato in AVANTI
                QString diff = toPlainText().mid(start, end-start);
                if (diff.isEmpty()) {
                    // Se il testo attuale è vuoto da start a end allora ho cancellato in AVANTI
                    QString lastDiff = lastText.mid(start, end - start);
                    qDebug() << "DELETE: " << lastDiff << " - START " << end << " END " << start;
                    emit textChangedEvent(CRDT_DELETE, lastDiff, end, start);

                    if(lastDiff.length()>1)
                        emit alignmentNotifyEvent(end, end);

                } else {
                    // Altrimenti ho inserito in AVANTI
                    qDebug() << "INSERT: "<< diff << " - START " << start << " END " << end;
                    emit textChangedEvent(CRDT_INSERT, diff, start, end);

                    if(diff.length()>1)
                       emit alignmentNotifyEvent(start, end);

                }
            }

            // Mi salvo l'ultima posizione del cursore in cui è cambiato il testo (serve per il CTRL + Z)
            lastPos = textCursor().position();

        } else if (start == end && start >= 0 && end >= 0) {
            // Caso in cui ho premuto CANC e quindi cancellato in avanti
            int length = toPlainText().length() - lastText.length();
            if (length < 0) {
                length = -length;
                QString lastDiff = lastText.mid(start, length);
                qDebug() << "DELETE: " << lastDiff << " - START " << start << " END " << start + length;
                emit textChangedEvent(CRDT_DELETE, lastDiff, start, start + length);
                // Mi salvo l'ultima posizione del cursore in cui è cambiato il testo (serve per il CTRL + Z)
                lastPos = textCursor().position();
            }
        }
    }
}

