//
// Created by Klaus on 26/05/2019.
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTextCursor>
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class myCursor{
public:
    int line;
    int col;
    myCursor(int line, int col): line(line), col(col){}
    void setCol(int col) {
        this->col = col;
    }
    void setLine(int line) {
        this->line = line;
    }
};
class TextEdit : public QMainWindow
{
Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);

    bool load(const QString &f);
    void myInsertText(QString text, QString name, int line, int position);
    void moveMyCursor(int targetCol, int targetLine, int line, QTextCursor *curs);
signals:
    void textInserted(QString text, int startLine, int startCol, int endLine, int endCol);

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

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    int lastLength = 0, curLinePos = 0, pos=0, lastPos=0;
    QString lastText="",diffText="",fileName;
    QToolBar *tb;
    QTextEdit *textEdit;
    QMap <QString,myCursor*> myCursors;

};
typedef QSharedPointer<TextEdit> textedit_ptr;
#endif // TEXTEDIT_H
