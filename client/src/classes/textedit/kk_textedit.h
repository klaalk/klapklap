#ifndef KKTEXTEDIT_H
#define KKTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QtDebug>

#include "../../../../libs/src/constants/kk_constants.h"
#include "../../../../libs/src/classes/logger/kk_logger.h"


class KKTextEdit : public QTextEdit
{
    Q_OBJECT
signals:
    void textChangedEvent(QString operation, QString diff, int start, int end);
    void wheelEventTriggered();
    void alignmentNotifyEvent(int startAling,int endAlign);

public:
    KKTextEdit(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

    QTextCursor getCursor(int position);
    void setCursorPosition(int position);
    void setLocalCursorPosition(int position);
    void restoreCursorPosition();
    int cursorPosition();
    int getLocalCursorPosition();

    bool getIsUndoAvailable() const;
    bool getIsRedoAvailable() const;

public slots:
    void handleTextChange();
    void textUndo();
    void textRedo();
    void textCopy();
    void textPaste();
    void textCut();

private:
    void calculateDiffText();
    void sendDiffText(QString operation, QString text, int start, int end);
    int lastPos = -1;
    int start = -1;
    int end = -1;
    int selectionStart = -1;
    int selectionEnd = -1;
    int localCursorPosition = -1;

    bool wasSelected = false;
    bool textChanged = false;
    bool isUndoAvailable = false;
    bool isRedoAvailable = false;
    QString lastText;
};

#endif // KKTEXTEDIT_H
