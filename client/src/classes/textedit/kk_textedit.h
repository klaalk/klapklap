#ifndef KKTEXTEDIT_H
#define KKTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QtDebug>

#define DEL "DEL"
#define INS "INS"

class KKTextEdit : public QTextEdit
{
    Q_OBJECT
signals:
    void textChangedEvent(QString operation, QString diff, int start, int end);
    void wheelEventTriggered();
    void alignmentNotifyEvent(int startAling,int endAlign);
public:
    KKTextEdit(QWidget *parent = nullptr);
    void keyReleaseEvent(QKeyEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

    QTextCursor cursorIn(int position);
    void setCursorPosition(int position);
    void restoreCursorPosition();
    int cursorPosition();

public slots:
    void handleTextChange();
    void textUndo();
    void textRedo();
    void textCopy();
    void textPaste();
    void textCut();
private:
    void sendDiffText();
    int lastPos = -1;
    int start = -1;
    int end = -1;
    int selectionStart = -1;
    int selectionEnd = -1;
    int localCursorPosition = -1;

    /// Variabile che conta quanti tasti ho premuto
    int keyCounter = 0;
    bool wasSelected = false;
    bool textChanged = false;
    QString lastText;
};

#endif // KKTEXTEDIT_H
