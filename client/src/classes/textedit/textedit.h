//
// Created by Klaus on 26/05/2019.
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTextCursor>
#include <QLabel>

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;

class kk_cursor{
public:
    int globalPositon=0;
    kk_cursor(int position): globalPositon(position){}
    void setGlobalPositon(int position) {
        this->globalPositon = position;
    }
};

class TextEdit : public QMainWindow
{
Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    QTextEdit *textEdit;
    bool load(const QString &f);
    void insertRemoteText(QString name, QString text, int position);
    void movekk_cursor(int targetCol, int targetLine, int line, QTextCursor *curs);
signals:
    void diffTextChanged(QString text, int position);

public slots:
    void fileNew();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);
    void onTextChange();

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif
    bool blockCursor = false;
    int lastLength = 0, cursorPos=0, lastCursorPos=0;
    QMap <QString,kk_cursor*> cursors_;
    QMap <QString,QLabel*> labels_;
    QMap <QString,QLabel*> labels2_;
    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    QString lastText="",diffText="",fileName;
    QToolBar *tb;
};
typedef QSharedPointer<TextEdit> textedit_ptr;
#endif // TEXTEDIT_H
