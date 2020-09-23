#include "kk_textedit.h"

#include <QKeyEvent>
#include <QtDebug>

KKTextEdit::KKTextEdit(QWidget *parent): QTextEdit(parent)
{
}

void KKTextEdit::keyReleaseEvent(QKeyEvent *e)
{
    if (e->text() != "" && keyCounter == 1) {
        end = textCursor().position();

        if (wasSelected && !textCursor().hasSelection()) {
            if (selectionStart > selectionEnd) {
                int tmp = selectionStart;
                selectionStart = selectionEnd;
                selectionEnd = tmp;
            }

            QString diff = toPlainText().mid(selectionStart, end - selectionStart);
            QString lastDiff = lastText.mid(selectionStart, selectionEnd-selectionStart);
            if (diff.isEmpty()) {
                qDebug() << "TEXT: " << e->text() << "DELETE: "<< lastDiff << " - START " << selectionStart << " END " << selectionEnd;
                emit textChangedEvent(DELETE, lastDiff, selectionStart, selectionEnd);
            } else {
                if (lastDiff.isEmpty()) {
                    qDebug() << "TEXT: " << e->text() << "INSERT: " << diff << " START " << selectionStart << " END " << selectionStart + diff.length();
                    emit textChangedEvent(INSERT, diff, selectionStart, selectionStart + diff.length());
                } else {
                    qDebug() << "TEXT: " << e->text() << "DELETE: "<< lastDiff << " - START " << selectionStart << " END " << selectionEnd;
                    emit textChangedEvent(DELETE, lastDiff, selectionStart, selectionEnd);

                    qDebug() << "TEXT: " << e->text() << "INSERT: " << diff << " - START " << selectionStart << " END " << selectionStart + diff.length();
                    emit textChangedEvent(INSERT, diff, selectionStart, selectionStart + diff.length());
                }
            }

        } else {
            if (start != end) {
                if (start > end) {
                    QString lastDiff = lastText.mid(end, start - end);
                    qDebug() << "TEXT: " << e->text() << "DELETE: " << lastDiff << " - START " << end << " END " << start;
                    emit textChangedEvent(DELETE, lastDiff, end, start);

                } else {
                    QString diff = toPlainText().mid(start, end-start);
                    if (diff.isEmpty()) {
                        QString lastDiff = lastText.mid(start, end - start);
                        qDebug() << "TEXT: " << e->text() << "DELETE: " << lastDiff << " - START " << end << " END " << start;
                        emit textChangedEvent(DELETE, lastDiff, end, start);
                    } else {
                        qDebug() << "TEXT: " << e->text() << "INSERT: "<< diff << " - START " << start << " END " << end;
                        emit textChangedEvent(INSERT, diff, start, end);
                    }
                }
            } else if (start == end && start >= 0 && end >= 0) {
                int length = toPlainText().length() - lastText.length();
                if (length < 0) {
                    length = -length;
                    QString lastDiff = lastText.mid(start, length);
                    qDebug() << "TEXT: " << e->text() << "DELETE: " << lastDiff << " - START " << start << " END " << start + length;
                    emit textChangedEvent(DELETE, lastDiff, start, start + length);
                }
            }

            lastPos = textCursor().position();
        }
        keyCounter--;
    }
    QTextEdit::keyPressEvent(e);
}

void KKTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->text() != "" && keyCounter == 0) {
        if (e->text() == "\u001A") {
            start = lastPos;
        } else {
            if (textCursor().hasSelection()) {
                selectionStart = textCursor().selectionStart();
                selectionEnd = textCursor().selectionEnd();
                wasSelected = true;
            } else {
                wasSelected = false;
            }
            start = textCursor().position();
        }
        keyCounter++;
        qDebug() << keyCounter;
    }
    lastText = toPlainText();
    QTextEdit::keyPressEvent(e);
}

void KKTextEdit::mousePressEvent(QMouseEvent *e)
{
    if (keyCounter == 0)
        start = textCursor().position();
    lastText = toPlainText();
    QTextEdit::mousePressEvent(e);
}

void KKTextEdit::wheelEvent(QWheelEvent *e)
{
    emit wheelEventTriggered();
    QTextEdit::wheelEvent(e);
}
